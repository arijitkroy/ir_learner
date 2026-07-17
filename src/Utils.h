#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

namespace Utils {

    /**
     * @brief Calculates a standard CRC16-CCITT checksum for data integrity verification.
     * @param data Pointer to the data array.
     * @param length Number of bytes to process.
     * @return CRC-16 checksum value.
     */
    uint16_t calculateCRC16(const uint8_t* data, size_t length);

    /**
     * @brief Computes the SHA-1 hash of a buffer. Useful for WebSocket handshakes.
     * @param input Pointer to input bytes.
     * @param inputLength Length of input.
     * @param outputHash Array of size 20 where the hash will be written.
     */
    void sha1(const uint8_t* input, size_t inputLength, uint8_t* outputHash);

    /**
     * @brief Encodes binary data into a Base64 string.
     * @param input Pointer to raw binary data.
     * @param inputLength Length of binary data.
     * @return Base64-encoded string.
     */
    String base64Encode(const uint8_t* input, size_t inputLength);

    /**
     * @brief Extracts a string value from a simple JSON string.
     * @param json The JSON string.
     * @param key The key to look for.
     * @return The string value associated with the key, or empty string if not found.
     */
    String getJSONString(const String& json, const String& key);

    /**
     * @brief Extracts an integer value from a simple JSON string.
     * @param json The JSON string.
     * @param key The key to look for.
     * @param defaultValue The value to return if the key is not found.
     * @return The integer value, or defaultValue.
     */
    int getJSONInt(const String& json, const String& key, int defaultValue = -1);

    /**
     * @brief Extracts a boolean value from a simple JSON string.
     * @param json The JSON string.
     * @param key The key to look for.
     * @param defaultValue The value to return if the key is not found.
     * @return The boolean value, or defaultValue.
     */
    bool getJSONBool(const String& json, const String& key, bool defaultValue = false);

    /**
     * @brief Determines the amount of free SRAM on the Renesas RA4M1.
     * @return Free RAM in bytes.
     */
    uint32_t getFreeRAM();

    /**
     * @brief Formats an uptime in seconds into a human-readable string (e.g. "2d 4h 12m 30s").
     * @param uptimeSecs Uptime in seconds.
     * @return Formatted string.
     */
    String formatUptime(uint32_t uptimeSecs);

} // namespace Utils

#endif // UTILS_H
