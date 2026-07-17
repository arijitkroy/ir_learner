#include "Storage.h"
#include "Utils.h"
#include "config.h"
#include <EEPROM.h>

#define CONFIG_ADDR 6
#define HEADER_ADDR 136
#define OFFSET_TABLE_ADDR 144
#define DATA_START_ADDR 344

// Serialization helper
static uint16_t getSerializedRawSize(const uint16_t* raw, uint16_t length) {
    uint16_t size = 0;
    for (uint16_t i = 0; i < length; i++) {
        if (raw[i] / 10 < 255) {
            size += 1;
        } else {
            size += 3;
        }
    }
    return size;
}

static int serializeRaw(const uint16_t* raw, uint16_t length, uint8_t* dest) {
    int offset = 0;
    for (uint16_t i = 0; i < length; i++) {
        uint32_t val = raw[i];
        uint32_t scaled = val / 10;
        if (scaled < 255) {
            dest[offset++] = (uint8_t)scaled;
        } else {
            dest[offset++] = 255;
            dest[offset++] = (uint8_t)(val >> 8);
            dest[offset++] = (uint8_t)(val & 0xFF);
        }
    }
    return offset;
}

static int deserializeRaw(const uint8_t* src, uint16_t length, uint16_t* dest) {
    int offset = 0;
    for (uint16_t i = 0; i < length; i++) {
        if (src[offset] == 255) {
            dest[i] = ((uint16_t)src[offset + 1] << 8) | src[offset + 2];
            offset += 3;
        } else {
            dest[i] = (uint16_t)src[offset] * 10;
            offset += 1;
        }
    }
    return offset;
}

Storage::Storage() : _codeCount(0), _nextId(1), _isDirty(false) {
    memset(&_config, 0, sizeof(SystemConfig));
}

bool Storage::begin() {
    // FORCE CLEAR EEPROM ON BOOT (Uncomment the line below to force wipe for development)
    // factoryReset(); return true;

    // Check magic bytes
    if (EEPROM.read(0) != 0x49 || EEPROM.read(1) != 0x52 || 
        EEPROM.read(2) != 0x4C || EEPROM.read(3) != 0x31) {
        // No valid magic, factory reset
        factoryReset();
        return true;
    }

    if (!verifyCRC()) {
        // Corrupted database, but let's try to load config and headers anyway
        // or re-verify. For safety, we will report corruption.
        loadConfig();
        loadDatabase();
        return false;
    }

    loadConfig();
    loadDatabase();
    return true;
}

void Storage::loadConfig() {
    EEPROM.get(CONFIG_ADDR, _config);
}

void Storage::saveSystemConfig(const SystemConfig& config) {
    _config = config;
    EEPROM.put(CONFIG_ADDR, _config);
    updateCRC();
}

void Storage::loadDatabase() {
    EEPROM.get(HEADER_ADDR, _codeCount);
    EEPROM.get(HEADER_ADDR + 2, _nextId);

    if (_codeCount > MAX_CODES) {
        _codeCount = 0;
    }

    uint16_t offsetCount = 0;
    for (int i = 0; i < MAX_CODES && offsetCount < _codeCount; i++) {
        uint16_t addr = 0;
        EEPROM.get(OFFSET_TABLE_ADDR + i * 2, addr);
        if (addr != 0) {
            IRCodeHeader& header = _headers[offsetCount];
            // Read record header from EEPROM
            // Record header is 36 bytes: id(4) + name(21) + protocol(1) + address(2) + command(4) + bits(1) + freq(2) + rawLen(2)
            EEPROM.get(addr, header.id);
            for (int k = 0; k < 21; k++) {
                header.name[k] = EEPROM.read(addr + 4 + k);
            }
            EEPROM.get(addr + 25, header.protocol);
            EEPROM.get(addr + 26, header.address);
            EEPROM.get(addr + 28, header.command);
            EEPROM.get(addr + 32, header.bits);
            EEPROM.get(addr + 33, header.frequency);
            EEPROM.get(addr + 35, header.rawLength);
            header.eepromAddr = addr + 37; // Raw data starts after the 37 bytes record header

            offsetCount++;
        }
    }
    _codeCount = offsetCount;
}

int Storage::findHeaderIndexById(uint32_t id) const {
    for (int i = 0; i < _codeCount; i++) {
        if (_headers[i].id == id) return i;
    }
    return -1;
}

const IRCodeHeader* Storage::getCodeHeaderByIndex(int index) const {
    if (index >= 0 && index < _codeCount) {
        return &_headers[index];
    }
    return nullptr;
}

const IRCodeHeader* Storage::getCodeHeaderById(uint32_t id) const {
    int idx = findHeaderIndexById(id);
    if (idx != -1) return &_headers[idx];
    return nullptr;
}

bool Storage::getCode(uint32_t id, IRCode& code) const {
    const IRCodeHeader* header = getCodeHeaderById(id);
    if (!header) return false;

    code.id = header->id;
    code.name = String(header->name);
    code.protocol = header->protocol;
    code.address = header->address;
    code.command = header->command;
    code.bits = header->bits;
    code.frequency = header->frequency;
    code.rawLength = header->rawLength;

    if (code.rawLength > 0) {
        // Read compressed bytes from EEPROM
        uint16_t compressedSize = getSerializedRawSize(nullptr, 0); // Not directly useful, read byte-by-byte
        // Read dynamically:
        uint16_t addr = header->eepromAddr;
        uint8_t* compressedBuf = (uint8_t*)malloc(code.rawLength * 3); // Max possible size
        if (!compressedBuf) return false;

        int byteOffset = 0;
        for (uint16_t i = 0; i < code.rawLength; i++) {
            uint8_t b = EEPROM.read(addr + byteOffset);
            compressedBuf[byteOffset++] = b;
            if (b == 255) {
                compressedBuf[byteOffset++] = EEPROM.read(addr + byteOffset);
                compressedBuf[byteOffset++] = EEPROM.read(addr + byteOffset);
            }
        }

        deserializeRaw(compressedBuf, code.rawLength, code.raw);
        free(compressedBuf);
    } else {
        memset(code.raw, 0, sizeof(code.raw));
    }
    return true;
}

uint16_t Storage::findFreeEEPROMSpace(uint16_t requiredSize) {
    // Find the highest address used in EEPROM by active records
    uint16_t highestAddr = DATA_START_ADDR;
    for (int i = 0; i < MAX_CODES; i++) {
        uint16_t addr = 0;
        EEPROM.get(OFFSET_TABLE_ADDR + i * 2, addr);
        if (addr != 0) {
            // Find record size
            uint16_t rawLen = 0;
            EEPROM.get(addr + 35, rawLen);
            uint16_t recordSize = 37;
            if (rawLen > 0) {
                // Read compressed size
                uint16_t rawAddr = addr + 37;
                int byteOffset = 0;
                for (uint16_t j = 0; j < rawLen; j++) {
                    uint8_t b = EEPROM.read(rawAddr + byteOffset);
                    byteOffset++;
                    if (b == 255) byteOffset += 2;
                }
                recordSize += byteOffset;
            }
            if (addr + recordSize > highestAddr) {
                highestAddr = addr + recordSize;
            }
        }
    }

    if (highestAddr + requiredSize <= EEPROM_SIZE) {
        return highestAddr;
    }
    return 0; // Out of space
}

void Storage::defragment() {
    uint16_t writeOffset = DATA_START_ADDR;
    uint16_t tempOffsets[MAX_CODES];
    memset(tempOffsets, 0, sizeof(tempOffsets));

    int activeCount = 0;
    for (int i = 0; i < MAX_CODES; i++) {
        uint16_t addr = 0;
        EEPROM.get(OFFSET_TABLE_ADDR + i * 2, addr);
        if (addr != 0) {
            // Read active record
            uint32_t id;
            char name[21];
            uint8_t protocol;
            uint16_t address;
            uint32_t command;
            uint8_t bits;
            uint16_t frequency;
            uint16_t rawLen;

            EEPROM.get(addr, id);
            for (int k = 0; k < 21; k++) name[k] = EEPROM.read(addr + 4 + k);
            EEPROM.get(addr + 25, protocol);
            EEPROM.get(addr + 26, address);
            EEPROM.get(addr + 28, command);
            EEPROM.get(addr + 32, bits);
            EEPROM.get(addr + 33, frequency);
            EEPROM.get(addr + 35, rawLen);

            // Read raw bytes if any
            uint8_t* rawData = nullptr;
            int rawBytesCount = 0;
            if (rawLen > 0) {
                rawData = (uint8_t*)malloc(rawLen * 3);
                if (rawData) {
                    uint16_t rawAddr = addr + 37;
                    while (rawBytesCount < rawLen * 3) {
                        uint8_t b = EEPROM.read(rawAddr + rawBytesCount);
                        rawData[rawBytesCount++] = b;
                        // check if completed
                        int decodedCount = 0;
                        int checkIdx = 0;
                        while (checkIdx < rawBytesCount) {
                            if (rawData[checkIdx] == 255) checkIdx += 3;
                            else checkIdx += 1;
                            decodedCount++;
                        }
                        if (decodedCount >= rawLen) break;
                    }
                }
            }

            // Write to new location
            EEPROM.put(writeOffset, id);
            for (int k = 0; k < 21; k++) EEPROM.write(writeOffset + 4 + k, name[k]);
            EEPROM.put(writeOffset + 25, protocol);
            EEPROM.put(writeOffset + 26, address);
            EEPROM.put(writeOffset + 28, command);
            EEPROM.put(writeOffset + 32, bits);
            EEPROM.put(writeOffset + 33, frequency);
            EEPROM.put(writeOffset + 35, rawLen);

            if (rawLen > 0 && rawData) {
                for (int k = 0; k < rawBytesCount; k++) {
                    EEPROM.write(writeOffset + 37 + k, rawData[k]);
                }
                free(rawData);
            }

            tempOffsets[activeCount] = writeOffset;
            
            // Update header RAM address
            int hIdx = findHeaderIndexById(id);
            if (hIdx != -1) {
                _headers[hIdx].eepromAddr = writeOffset + 37;
            }

            writeOffset += 37 + rawBytesCount;
            activeCount++;
        }
    }

    // Write offsets table back
    for (int i = 0; i < MAX_CODES; i++) {
        uint16_t val = (i < activeCount) ? tempOffsets[i] : 0;
        EEPROM.put(OFFSET_TABLE_ADDR + i * 2, val);
    }

    // Clear rest of EEPROM
    for (uint16_t i = writeOffset; i < EEPROM_SIZE; i++) {
        EEPROM.update(i, 0xFF);
    }
}

bool Storage::saveCode(IRCode& code) {
    if (code.id == 0) {
        code.id = _nextId++;
    } else {
        // Delete old entry first to prevent space leaks
        deleteCode(code.id);
    }

    if (_codeCount >= MAX_CODES) return false;

    // Calculate space needed
    uint16_t compressedRawSize = 0;
    uint8_t* compressedRaw = nullptr;
    if (code.rawLength > 0) {
        compressedRawSize = getSerializedRawSize(code.raw, code.rawLength);
        compressedRaw = (uint8_t*)malloc(compressedRawSize);
        if (compressedRaw) {
            serializeRaw(code.raw, code.rawLength, compressedRaw);
        } else {
            return false; // OOM
        }
    }

    uint16_t requiredSize = 37 + compressedRawSize;
    uint16_t writeAddr = findFreeEEPROMSpace(requiredSize);

    if (writeAddr == 0) {
        defragment();
        writeAddr = findFreeEEPROMSpace(requiredSize);
        if (writeAddr == 0) {
            if (compressedRaw) free(compressedRaw);
            return false; // Out of memory
        }
    }

    // Save record to EEPROM
    EEPROM.put(writeAddr, code.id);
    char nameBuf[21];
    memset(nameBuf, 0, sizeof(nameBuf));
    code.name.toCharArray(nameBuf, 21);
    for (int k = 0; k < 21; k++) {
        EEPROM.write(writeAddr + 4 + k, nameBuf[k]);
    }
    EEPROM.put(writeAddr + 25, code.protocol);
    EEPROM.put(writeAddr + 26, code.address);
    EEPROM.put(writeAddr + 28, code.command);
    EEPROM.put(writeAddr + 32, code.bits);
    EEPROM.put(writeAddr + 33, code.frequency);
    EEPROM.put(writeAddr + 35, code.rawLength);

    if (code.rawLength > 0 && compressedRaw) {
        for (int k = 0; k < compressedRawSize; k++) {
            EEPROM.write(writeAddr + 37 + k, compressedRaw[k]);
        }
        free(compressedRaw);
    }

    // Find free index in offset table
    int offsetIdx = -1;
    for (int i = 0; i < MAX_CODES; i++) {
        uint16_t addr = 0;
        EEPROM.get(OFFSET_TABLE_ADDR + i * 2, addr);
        if (addr == 0) {
            offsetIdx = i;
            break;
        }
    }

    if (offsetIdx != -1) {
        EEPROM.put(OFFSET_TABLE_ADDR + offsetIdx * 2, writeAddr);
    } else {
        return false;
    }

    // Update RAM headers
    IRCodeHeader& header = _headers[_codeCount];
    header.id = code.id;
    strncpy(header.name, nameBuf, 20);
    header.name[20] = '\0';
    header.protocol = code.protocol;
    header.address = code.address;
    header.command = code.command;
    header.bits = code.bits;
    header.frequency = code.frequency;
    header.rawLength = code.rawLength;
    header.eepromAddr = writeAddr + 37;

    _codeCount++;

    // Save counts
    EEPROM.put(HEADER_ADDR, _codeCount);
    EEPROM.put(HEADER_ADDR + 2, _nextId);

    updateCRC();
    return true;
}

bool Storage::deleteCode(uint32_t id) {
    int idx = findHeaderIndexById(id);
    if (idx == -1) return false;

    // Find in offset table
    uint16_t recordAddr = _headers[idx].eepromAddr - 37;
    for (int i = 0; i < MAX_CODES; i++) {
        uint16_t addr = 0;
        EEPROM.get(OFFSET_TABLE_ADDR + i * 2, addr);
        if (addr == recordAddr) {
            EEPROM.put(OFFSET_TABLE_ADDR + i * 2, (uint16_t)0);
            break;
        }
    }

    // Shift headers
    for (int i = idx; i < _codeCount - 1; i++) {
        _headers[i] = _headers[i + 1];
    }
    _codeCount--;

    EEPROM.put(HEADER_ADDR, _codeCount);
    updateCRC();
    return true;
}

bool Storage::duplicateCode(uint32_t id, uint32_t& newId) {
    IRCode code;
    if (!getCode(id, code)) return false;
    
    code.id = 0; // Save as new code
    code.name = code.name + " (Copy)";
    if (saveCode(code)) {
        newId = code.id;
        return true;
    }
    return false;
}

bool Storage::renameCode(uint32_t id, const String& newName) {
    int idx = findHeaderIndexById(id);
    if (idx == -1) return false;

    IRCode code;
    if (!getCode(id, code)) return false;

    code.name = newName;
    return saveCode(code);
}

void Storage::factoryReset() {
    // Clear entire EEPROM to 0
    for (uint16_t i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.update(i, 0x00);
    }

    // Set Magic
    EEPROM.write(0, 0x49); // 'I'
    EEPROM.write(1, 0x52); // 'R'
    EEPROM.write(2, 0x4C); // 'L'
    EEPROM.write(3, 0x31); // '1'

    // Seed persisted WiFi settings from config.h only on first initialization
    // or factory reset. Later web Settings changes remain in EEPROM.
    SystemConfig defaultConfig;
    memset(&defaultConfig, 0, sizeof(SystemConfig));
    
    if (strlen(DEFAULT_WIFI_SSID) > 0 && strcmp(DEFAULT_WIFI_SSID, "Your_WiFi_SSID") != 0) {
        defaultConfig.wifiConfigured = 1;
        strncpy(defaultConfig.ssid, DEFAULT_WIFI_SSID, sizeof(defaultConfig.ssid) - 1);
        strncpy(defaultConfig.password, DEFAULT_WIFI_PASS, sizeof(defaultConfig.password) - 1);
    } else {
        defaultConfig.wifiConfigured = 0;
    }
    
    strncpy(defaultConfig.mdnsName, DEFAULT_MDNS_NAME, sizeof(defaultConfig.mdnsName) - 1);

    EEPROM.put(CONFIG_ADDR, defaultConfig);
    _config = defaultConfig;

    _codeCount = 0;
    _nextId = 1;
    EEPROM.put(HEADER_ADDR, _codeCount);
    EEPROM.put(HEADER_ADDR + 2, _nextId);

    // Clear Offset Table
    for (int i = 0; i < MAX_CODES; i++) {
        EEPROM.put(OFFSET_TABLE_ADDR + i * 2, (uint16_t)0);
    }

    updateCRC();
}

bool Storage::verifyCRC() const {
    uint16_t storedCRC = 0;
    EEPROM.get(4, storedCRC);

    // Calculate CRC of configuration, header, offsets and records
    // We read EEPROM from address 6 to EEPROM_SIZE - 1
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 6; i < EEPROM_SIZE; i++) {
        uint8_t val = EEPROM.read(i);
        crc ^= (uint16_t)val;
        for (uint8_t j = 8; j > 0; j--) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return (crc == storedCRC);
}

void Storage::updateCRC() {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 6; i < EEPROM_SIZE; i++) {
        uint8_t val = EEPROM.read(i);
        crc ^= (uint16_t)val;
        for (uint8_t j = 8; j > 0; j--) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    EEPROM.put(4, crc);
}

String Storage::exportDatabaseJSON() const {
    String json = "[\n";
    for (int i = 0; i < _codeCount; i++) {
        IRCode code;
        if (getCode(_headers[i].id, code)) {
            json += "  {\n";
            json += "    \"id\": " + String(code.id) + ",\n";
            json += "    \"name\": \"" + code.name + "\",\n";
            json += "    \"protocol\": " + String(code.protocol) + ",\n";
            json += "    \"address\": " + String(code.address) + ",\n";
            json += "    \"command\": " + String(code.command) + ",\n";
            json += "    \"bits\": " + String(code.bits) + ",\n";
            json += "    \"frequency\": " + String(code.frequency) + ",\n";
            json += "    \"rawLength\": " + String(code.rawLength) + ",\n";
            json += "    \"raw\": [";
            for (uint16_t j = 0; j < code.rawLength; j++) {
                json += String(code.raw[j]);
                if (j < code.rawLength - 1) json += ",";
            }
            json += "]\n";
            json += "  }";
            if (i < _codeCount - 1) json += ",\n";
        }
    }
    json += "\n]";
    return json;
}

int Storage::importDatabaseJSON(const String& json) {
    // Basic bracket structure checks
    int startArr = json.indexOf('[');
    int endArr = json.lastIndexOf(']');
    if (startArr == -1 || endArr == -1) return -1;

    // Factory reset the database first before import
    // Note: We keep WiFi settings, only reset codes!
    _codeCount = 0;
    EEPROM.put(HEADER_ADDR, _codeCount);
    for (int i = 0; i < MAX_CODES; i++) {
        EEPROM.put(OFFSET_TABLE_ADDR + i * 2, (uint16_t)0);
    }
    updateCRC();
    loadDatabase();

    int importedCount = 0;
    int searchIdx = startArr + 1;
    
    while (searchIdx < endArr) {
        int objStart = json.indexOf('{', searchIdx);
        if (objStart == -1 || objStart > endArr) break;
        int objEnd = json.indexOf('}', objStart);
        if (objEnd == -1) break;

        String recordJson = json.substring(objStart, objEnd + 1);

        IRCode code;
        code.id = 0; // save as new
        code.name = Utils::getJSONString(recordJson, "name");
        code.protocol = Utils::getJSONInt(recordJson, "protocol");
        code.address = Utils::getJSONInt(recordJson, "address");
        code.command = Utils::getJSONInt(recordJson, "command");
        code.bits = Utils::getJSONInt(recordJson, "bits");
        code.frequency = Utils::getJSONInt(recordJson, "frequency");
        code.rawLength = Utils::getJSONInt(recordJson, "rawLength");

        // Load raw timing array if any
        memset(code.raw, 0, sizeof(code.raw));
        if (code.rawLength > 0) {
            int rawKeyIndex = recordJson.indexOf("\"raw\"");
            int rawStart = recordJson.indexOf('[', rawKeyIndex);
            int rawEnd = recordJson.indexOf(']', rawStart);
            if (rawStart != -1 && rawEnd != -1) {
                String rawArrayStr = recordJson.substring(rawStart + 1, rawEnd);
                int parseIdx = 0;
                for (int j = 0; j < code.rawLength && parseIdx < rawArrayStr.length(); j++) {
                    int nextComma = rawArrayStr.indexOf(',', parseIdx);
                    if (nextComma == -1) {
                        code.raw[j] = rawArrayStr.substring(parseIdx).toInt();
                        break;
                    }
                    code.raw[j] = rawArrayStr.substring(parseIdx, nextComma).toInt();
                    parseIdx = nextComma + 1;
                }
            }
        }

        if (saveCode(code)) {
            importedCount++;
        }

        searchIdx = objEnd + 1;
    }

    return importedCount;
}
