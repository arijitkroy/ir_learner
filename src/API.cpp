#include "API.h"
#include "Utils.h"

// External flags declared in IRManager / main sketch to trigger delayed operations
extern volatile bool rebootRequested;
extern volatile bool factoryResetRequested;

API::API(IRReceiver* rx, IRSender* tx, Storage* storage, WiFiManager* wifi)
    : _receiver(rx), _sender(tx), _storage(storage), _wifi(wifi) {}

String API::handleRequest(const String& method, const String& uri, const String& body, int& statusCode) {
    statusCode = 200;

    if (method == "GET" && uri == "/api/status") {
        return getStatusJSON();
    }
    else if (method == "GET" && uri == "/api/codes") {
        return getCodesJSON();
    }
    else if (method == "POST" && uri == "/api/codes") {
        return handleSaveCode(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/learn/start") {
        _receiver->startLearning();
        return "{\"status\":\"ok\",\"msg\":\"Learning started\"}";
    }
    else if (method == "POST" && uri == "/api/learn/stop") {
        _receiver->stopLearning();
        return "{\"status\":\"ok\",\"msg\":\"Learning stopped\"}";
    }
    else if (method == "GET" && uri == "/api/learn/status") {
        return getLearnStatusJSON();
    }
    else if (method == "POST" && uri == "/api/send") {
        return handleSend(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/delete") {
        return handleDelete(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/rename") {
        return handleRename(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/export") {
        return "{\"status\":\"ok\",\"database\":" + _storage->exportDatabaseJSON() + "}";
    }
    else if (method == "POST" && uri == "/api/import") {
        return handleImport(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/wifi") {
        return handleWiFi(body, statusCode);
    }
    else if (method == "POST" && uri == "/api/reboot") {
        rebootRequested = true;
        return "{\"status\":\"ok\",\"msg\":\"Rebooting device...\"}";
    }
    else if (method == "POST" && uri == "/api/factoryreset") {
        factoryResetRequested = true;
        return "{\"status\":\"ok\",\"msg\":\"Factory resetting and rebooting...\"}";
    }

    statusCode = 404;
    return "{\"status\":\"error\",\"msg\":\"Not Found\"}";
}

String API::getStatusJSON() {
    uint32_t freeRam = Utils::getFreeRAM();
    uint32_t uptime = millis() / 1000;
    
    // Hardcoded flash estimate for RA4M1 compilation sizes
    uint32_t flashUsage = 135400; 

    String stateStr = "IDLE";
    switch (_receiver->getState()) {
        case IRReceiver::STATE_LEARNING: stateStr = "LEARNING"; break;
        case IRReceiver::STATE_CAPTURING: stateStr = "CAPTURING"; break;
        case IRReceiver::STATE_FINISHED: stateStr = "FINISHED"; break;
        case IRReceiver::STATE_OVERFLOW: stateStr = "OVERFLOW"; break;
        case IRReceiver::STATE_TIMEOUT: stateStr = "TIMEOUT"; break;
        default: break;
    }

    String json = "{\n";
    json += "  \"rssi\": " + String(_wifi->getRSSI()) + ",\n";
    json += "  \"ip\": \"" + _wifi->getIP() + "\",\n";
    json += "  \"freeRam\": " + String(freeRam) + ",\n";
    json += "  \"flashUsage\": " + String(flashUsage) + ",\n";
    json += "  \"uptime\": " + String(uptime) + ",\n";
    json += "  \"state\": \"" + stateStr + "\",\n";
    json += "  \"interrupts\": " + String(_receiver->getInterruptCount()) + ",\n";
    json += "  \"wifiSSID\": \"" + String(_wifi->isAPMode() ? _wifi->getAPSSID() : _storage->getSystemConfig().ssid) + "\",\n";
    json += "  \"wifiAP\": " + String(_wifi->isAPMode() ? "true" : "false") + "\n";
    json += "}";
    return json;
}

String API::getCodesJSON() {
    String json = "[\n";
    int count = _storage->getCodeCount();
    for (int i = 0; i < count; i++) {
        const IRCodeHeader* header = _storage->getCodeHeaderByIndex(i);
        if (header) {
            json += "  {\n";
            json += "    \"id\": " + String(header->id) + ",\n";
            json += "    \"name\": \"" + String(header->name) + "\",\n";
            json += "    \"protocol\": " + String(header->protocol) + ",\n";
            json += "    \"address\": " + String(header->address) + ",\n";
            json += "    \"command\": " + String(header->command) + ",\n";
            json += "    \"bits\": " + String(header->bits) + ",\n";
            json += "    \"frequency\": " + String(header->frequency) + ",\n";
            json += "    \"rawLength\": " + String(header->rawLength) + "\n";
            json += "  }";
            if (i < count - 1) json += ",\n";
        }
    }
    json += "\n]";
    return json;
}

String API::getLearnStatusJSON() {
    DecodeResult result;
    bool success = _receiver->decode(result);

    String stateStr = "IDLE";
    switch (_receiver->getState()) {
        case IRReceiver::STATE_LEARNING: stateStr = "LEARNING"; break;
        case IRReceiver::STATE_CAPTURING: stateStr = "CAPTURING"; break;
        case IRReceiver::STATE_FINISHED: stateStr = "FINISHED"; break;
        case IRReceiver::STATE_OVERFLOW: stateStr = "OVERFLOW"; break;
        case IRReceiver::STATE_TIMEOUT: stateStr = "TIMEOUT"; break;
        default: break;
    }

    String json = "{\n";
    json += "  \"state\": \"" + stateStr + "\",\n";
    json += "  \"decoded\": " + String(success ? "true" : "false") + ",\n";
    json += "  \"protocol\": " + String(result.protocol) + ",\n";
    json += "  \"address\": " + String(result.address) + ",\n";
    json += "  \"command\": " + String(result.command) + ",\n";
    json += "  \"bits\": " + String(result.bits) + ",\n";
    json += "  \"pulses\": " + String(_receiver->getRawCount()) + ",\n";
    json += "  \"quality\": " + String(result.quality) + ",\n";
    json += "  \"raw\": [";
    uint16_t pulsesCount = _receiver->getRawCount();
    if (pulsesCount > 0) {
        uint16_t* rawBuf = (uint16_t*)malloc(pulsesCount * sizeof(uint16_t));
        if (rawBuf) {
            uint16_t len = _receiver->getRawTimings(rawBuf, pulsesCount);
            for (uint16_t j = 0; j < len; j++) {
                json += String(rawBuf[j]);
                if (j < len - 1) json += ",";
            }
            free(rawBuf);
        }
    }
    json += "]\n";
    json += "}";
    return json;
}

String API::handleSend(const String& body, int& statusCode) {
    int id = Utils::getJSONInt(body, "id");
    
    IRCode code;
    bool codeLoaded = false;
    
    if (id > 0) {
        // Send a stored command by ID
        codeLoaded = _storage->getCode(id, code);
        if (!codeLoaded) {
            statusCode = 404;
            return "{\"status\":\"error\",\"msg\":\"Command ID not found\"}";
        }
    } else {
        // Send a custom command from HTTP request body
        code.protocol = Utils::getJSONInt(body, "protocol", -1);
        if (code.protocol == -1) {
            statusCode = 400;
            return "{\"status\":\"error\",\"msg\":\"Missing id or protocol parameter\"}";
        }
        code.address = Utils::getJSONInt(body, "address", 0);
        code.command = Utils::getJSONInt(body, "command", 0);
        code.bits = Utils::getJSONInt(body, "bits", 0);
        code.frequency = Utils::getJSONInt(body, "frequency", 38000);
        code.rawLength = Utils::getJSONInt(body, "rawLength", 0);
        
        memset(code.raw, 0, sizeof(code.raw));
        if (code.rawLength > 0) {
            int rawStart = body.indexOf("\"raw\"");
            int openBracket = body.indexOf('[', rawStart);
            int closeBracket = body.indexOf(']', openBracket);
            if (openBracket != -1 && closeBracket != -1) {
                String rawArrayStr = body.substring(openBracket + 1, closeBracket);
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
        codeLoaded = true;
    }

    if (codeLoaded) {
        // Transmit command
        if (code.protocol == 0) { // Raw
            _sender->sendRaw(code.raw, code.rawLength, code.frequency);
        } else if (code.protocol == 1) { // NEC
            _sender->sendNEC(code.address, code.command, code.bits);
        } else if (code.protocol == 2) { // Sony
            _sender->sendSony(code.address, code.command, code.bits);
        } else if (code.protocol == 3) { // Samsung
            _sender->sendSamsung(code.address, code.command, code.bits);
        } else if (code.protocol == 4) { // RC5
            _sender->sendRC5(code.address, code.command, code.bits);
        } else if (code.protocol == 5) { // RC6
            _sender->sendRC6(code.address, code.command, code.bits);
        } else if (code.protocol == 6) { // Panasonic
            _sender->sendPanasonic(code.address, code.command, code.bits);
        } else if (code.protocol == 7) { // LG
            _sender->sendLG(code.address, code.command, code.bits);
        } else if (code.protocol == 8) { // JVC
            _sender->sendJVC(code.address, code.command, code.bits);
        } else {
            statusCode = 400;
            return "{\"status\":\"error\",\"msg\":\"Unknown protocol ID\"}";
        }
        return "{\"status\":\"ok\",\"msg\":\"Signal transmitted successfully\"}";
    }

    statusCode = 500;
    return "{\"status\":\"error\",\"msg\":\"Failed to send command\"}";
}

String API::handleDelete(const String& body, int& statusCode) {
    int id = Utils::getJSONInt(body, "id");
    if (id <= 0) {
        statusCode = 400;
        return "{\"status\":\"error\",\"msg\":\"Missing or invalid ID\"}";
    }

    if (_storage->deleteCode(id)) {
        return "{\"status\":\"ok\",\"msg\":\"Command deleted successfully\"}";
    }

    statusCode = 404;
    return "{\"status\":\"error\",\"msg\":\"Command not found\"}";
}

String API::handleRename(const String& body, int& statusCode) {
    int id = Utils::getJSONInt(body, "id");
    String newName = Utils::getJSONString(body, "name");
    
    if (id <= 0 || newName.length() == 0) {
        statusCode = 400;
        return "{\"status\":\"error\",\"msg\":\"Missing or invalid ID/name\"}";
    }

    if (_storage->renameCode(id, newName)) {
        return "{\"status\":\"ok\",\"msg\":\"Command renamed successfully\"}";
    }

    statusCode = 404;
    return "{\"status\":\"error\",\"msg\":\"Command not found\"}";
}

String API::handleImport(const String& body, int& statusCode) {
    int bracketIndex = body.indexOf('[');
    if (bracketIndex == -1) {
        statusCode = 400;
        return "{\"status\":\"error\",\"msg\":\"Invalid JSON backup payload\"}";
    }

    String backupData = body.substring(bracketIndex);
    int importedCount = _storage->importDatabaseJSON(backupData);
    
    if (importedCount >= 0) {
        return "{\"status\":\"ok\",\"msg\":\"Imported " + String(importedCount) + " commands successfully\",\"count\":" + String(importedCount) + "}";
    }

    statusCode = 400;
    return "{\"status\":\"error\",\"msg\":\"Failed to parse backup payload\"}";
}

String API::handleWiFi(const String& body, int& statusCode) {
    SystemConfig config = _storage->getSystemConfig();
    config.wifiConfigured = 1;
    
    String ssid = Utils::getJSONString(body, "ssid");
    String pass = Utils::getJSONString(body, "password");
    String mdns = Utils::getJSONString(body, "mdnsName");
    
    ssid.toCharArray(config.ssid, 33);
    pass.toCharArray(config.password, 65);
    mdns.toCharArray(config.mdnsName, 21);
    
    // Parse IP Address arrays
    auto parseIP = [](const String& json, const String& key, uint8_t* dest) {
        int keyPos = json.indexOf("\"" + key + "\"");
        if (keyPos == -1) return;
        int startBracket = json.indexOf('[', keyPos);
        int endBracket = json.indexOf(']', startBracket);
        if (startBracket == -1 || endBracket == -1) return;
        
        String arrStr = json.substring(startBracket + 1, endBracket);
        int parseIdx = 0;
        for (int i = 0; i < 4; i++) {
            int nextComma = arrStr.indexOf(',', parseIdx);
            if (nextComma == -1) {
                dest[i] = arrStr.substring(parseIdx).toInt();
                break;
            }
            dest[i] = arrStr.substring(parseIdx, nextComma).toInt();
            parseIdx = nextComma + 1;
        }
    };
    
    parseIP(body, "ip", config.ip);
    parseIP(body, "gateway", config.gateway);
    parseIP(body, "subnet", config.subnet);
    
    _storage->saveSystemConfig(config);
    rebootRequested = true;
    
    return "{\"status\":\"ok\",\"msg\":\"WiFi configuration saved. Rebooting controller...\"}";
}

String API::handleSaveCode(const String& body, int& statusCode) {
    IRCode code;
    code.id = 0; // Save as new
    code.name = Utils::getJSONString(body, "name");
    code.protocol = Utils::getJSONInt(body, "protocol", 0);
    code.address = Utils::getJSONInt(body, "address", 0);
    code.command = Utils::getJSONInt(body, "command", 0);
    code.bits = Utils::getJSONInt(body, "bits", 0);
    code.frequency = Utils::getJSONInt(body, "frequency", 38000);
    code.rawLength = Utils::getJSONInt(body, "rawLength", 0);

    memset(code.raw, 0, sizeof(code.raw));
    if (code.rawLength > 0) {
        int rawStart = body.indexOf("\"raw\"");
        int openBracket = body.indexOf('[', rawStart);
        int closeBracket = body.indexOf(']', openBracket);
        if (openBracket != -1 && closeBracket != -1) {
            String rawArrayStr = body.substring(openBracket + 1, closeBracket);
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

    if (_storage->saveCode(code)) {
        return "{\"status\":\"ok\",\"msg\":\"Command saved successfully\",\"id\":" + String(code.id) + "}";
    }

    statusCode = 500;
    return "{\"status\":\"error\",\"msg\":\"Failed to save command\"}";
}
