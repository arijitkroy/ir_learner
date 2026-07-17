#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

#define MAX_CODES 100
#define EEPROM_SIZE 8192

// IR Code Struct (as requested)
struct IRCode {
    uint32_t id;
    String name;
    uint8_t protocol;
    uint16_t address;
    uint32_t command;
    uint8_t bits;
    uint16_t frequency;
    uint16_t rawLength;
    uint16_t raw[400];
};

// Compact Header Struct to save RAM (36 bytes)
struct IRCodeHeader {
    uint32_t id;
    char name[21];       // Max 20 chars + null terminator
    uint8_t protocol;    // Protocol ID
    uint16_t address;    // Device Address
    uint32_t command;    // Decoded Command
    uint8_t bits;        // Number of bits
    uint16_t frequency;  // Carrier frequency (e.g. 38000)
    uint16_t rawLength;  // Length of raw timings (0 if protocol-based)
    uint16_t eepromAddr; // Address in EEPROM where raw delta array is stored
};

// WiFi Credentials & System Config
struct SystemConfig {
    uint8_t wifiConfigured; // Persisted: 0 = start setup AP, 1 = join saved network
    char ssid[33];          // Max 32 chars + null terminator
    char password[65];      // Max 64 chars + null terminator
    char mdnsName[21];      // Max 20 chars, default "irremote"
    uint8_t ip[4];          // Static IP (0.0.0.0 for DHCP)
    uint8_t gateway[4];     // Gateway IP
    uint8_t subnet[4];      // Subnet mask
};

class Storage {
private:
    SystemConfig _config;
    IRCodeHeader _headers[MAX_CODES];
    uint16_t _codeCount;
    uint32_t _nextId;
    bool _isDirty;

    Storage();
    void loadConfig();
    void saveConfig();
    void loadDatabase();
    int findHeaderIndexById(uint32_t id) const;
    uint16_t findFreeEEPROMSpace(uint16_t requiredSize);
    void defragment();

public:
    static Storage& getInstance() {
        static Storage instance;
        return instance;
    }

    /**
     * @brief Initializes the storage module and loads the database.
     * @return true if initialization succeeded, false if memory was corrupted.
     */
    bool begin();

    /**
     * @brief Saves the current configuration to EEPROM.
     */
    void saveSystemConfig(const SystemConfig& config);

    /**
     * @brief Retrieves the current system configuration.
     * @return The SystemConfig struct.
     */
    const SystemConfig& getSystemConfig() const { return _config; }

    /**
     * @brief Retrieves the number of stored codes.
     * @return Number of codes.
     */
    int getCodeCount() const { return _codeCount; }

    /**
     * @brief Gets a compact code header by index.
     * @param index Index in the header array (0 to count - 1).
     * @return Pointer to the header, or nullptr if out of bounds.
     */
    const IRCodeHeader* getCodeHeaderByIndex(int index) const;

    /**
     * @brief Gets a compact code header by unique ID.
     * @param id The code ID.
     * @return Pointer to the header, or nullptr if not found.
     */
    const IRCodeHeader* getCodeHeaderById(uint32_t id) const;

    /**
     * @brief Retrieves the full IR code (including raw timings) by ID.
     * @param id The code ID.
     * @param code Reference to the IRCode struct to fill.
     * @return true if successful, false if not found or corrupted.
     */
    bool getCode(uint32_t id, IRCode& code) const;

    /**
     * @brief Saves an IR code (creates new if id is 0, otherwise updates).
     * @param code The IRCode to save.
     * @return true if saved successfully, false if out of storage.
     */
    bool saveCode(IRCode& code);

    /**
     * @brief Deletes an IR code by ID.
     * @param id The code ID.
     * @return true if successful, false if not found.
     */
    bool deleteCode(uint32_t id);

    /**
     * @brief Duplicates an IR code.
     * @param id The ID of the code to duplicate.
     * @param newId Reference to store the new code ID.
     * @return true if successful.
     */
    bool duplicateCode(uint32_t id, uint32_t& newId);

    /**
     * @brief Renames an IR code by ID.
     * @param id The code ID.
     * @param newName The new name string.
     * @return true if successful.
     */
    bool renameCode(uint32_t id, const String& newName);

    /**
     * @brief Exports the entire database to a JSON string.
     * @return A JSON formatted string.
     */
    String exportDatabaseJSON() const;

    /**
     * @brief Imports the database from a JSON string.
     * @param json JSON string containing the database.
     * @return Number of codes imported, or -1 on parse failure.
     */
    int importDatabaseJSON(const String& json);

    /**
     * @brief Resets the EEPROM to factory defaults and reboots.
     */
    void factoryReset();

    /**
     * @brief Checks the database integrity using CRC16.
     * @return true if intact, false if corrupted.
     */
    bool verifyCRC() const;

    /**
     * @brief Updates the database CRC16 checksum in EEPROM.
     */
    void updateCRC();
};

#endif // STORAGE_H
