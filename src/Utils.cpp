#include "Utils.h"
#include <malloc.h>

extern "C" char* sbrk(int incr);

namespace Utils {

    uint16_t calculateCRC16(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint16_t)data[i];
            for (uint8_t j = 8; j > 0; j--) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001; // Polynomial 0x8005 reversed is 0xA001
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    void sha1(const uint8_t* input, size_t inputLength, uint8_t* outputHash) {
        // Standard lightweight SHA-1 implementation
        uint32_t h0 = 0x67452301;
        uint32_t h1 = 0xEFCDAB89;
        uint32_t h2 = 0x98BADCFE;
        uint32_t h3 = 0x10325476;
        uint32_t h4 = 0xC3D2E1F0;

        // Pad input
        uint64_t bitLength = (uint64_t)inputLength * 8;
        size_t paddedLength = inputLength + 1 + 8;
        if (paddedLength % 64 != 0) {
            paddedLength += 64 - (paddedLength % 64);
        }
        
        uint8_t* padded = (uint8_t*)malloc(paddedLength);
        if (!padded) return; // OOM check
        
        memcpy(padded, input, inputLength);
        padded[inputLength] = 0x80;
        memset(padded + inputLength + 1, 0, paddedLength - inputLength - 1 - 8);
        
        // Append length in bits as 64-bit big-endian integer
        for (int i = 0; i < 8; i++) {
            padded[paddedLength - 8 + i] = (uint8_t)(bitLength >> (56 - i * 8));
        }

        // Process in 512-bit chunks
        for (size_t chunk = 0; chunk < paddedLength; chunk += 64) {
            uint32_t w[80];
            for (int i = 0; i < 16; i++) {
                w[i] = ((uint32_t)padded[chunk + i * 4] << 24) |
                       ((uint32_t)padded[chunk + i * 4 + 1] << 16) |
                       ((uint32_t)padded[chunk + i * 4 + 2] << 8) |
                       ((uint32_t)padded[chunk + i * 4 + 3]);
            }
            for (int i = 16; i < 80; i++) {
                uint32_t val = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
                w[i] = (val << 1) | (val >> 31);
            }

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;

            for (int i = 0; i < 80; i++) {
                uint32_t f, k;
                if (i < 20) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                } else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                } else if (i < 60) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                } else {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
                e = d;
                d = c;
                c = (b << 30) | (b >> 2);
                b = a;
                a = temp;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
        }

        free(padded);

        outputHash[0] = (uint8_t)(h0 >> 24); outputHash[1] = (uint8_t)(h0 >> 16); outputHash[2] = (uint8_t)(h0 >> 8); outputHash[3] = (uint8_t)h0;
        outputHash[4] = (uint8_t)(h1 >> 24); outputHash[5] = (uint8_t)(h1 >> 16); outputHash[6] = (uint8_t)(h1 >> 8); outputHash[7] = (uint8_t)h1;
        outputHash[8] = (uint8_t)(h2 >> 24); outputHash[9] = (uint8_t)(h2 >> 16); outputHash[10] = (uint8_t)(h2 >> 8); outputHash[11] = (uint8_t)h2;
        outputHash[12] = (uint8_t)(h3 >> 24); outputHash[13] = (uint8_t)(h3 >> 16); outputHash[14] = (uint8_t)(h3 >> 8); outputHash[15] = (uint8_t)h3;
        outputHash[16] = (uint8_t)(h4 >> 24); outputHash[17] = (uint8_t)(h4 >> 16); outputHash[18] = (uint8_t)(h4 >> 8); outputHash[19] = (uint8_t)h4;
    }

    String base64Encode(const uint8_t* input, size_t inputLength) {
        static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        String encoded = "";
        encoded.reserve(((inputLength + 2) / 3) * 4);
        
        for (size_t i = 0; i < inputLength; i += 3) {
            uint32_t val = ((uint32_t)input[i] << 16);
            int count = 1;
            if (i + 1 < inputLength) {
                val |= ((uint32_t)input[i + 1] << 8);
                count++;
            }
            if (i + 2 < inputLength) {
                val |= (uint32_t)input[i + 2];
                count++;
            }
            
            encoded += alphabet[(val >> 18) & 0x3F];
            encoded += alphabet[(val >> 12) & 0x3F];
            encoded += (count > 1) ? alphabet[(val >> 6) & 0x3F] : '=';
            encoded += (count > 2) ? alphabet[val & 0x3F] : '=';
        }
        return encoded;
    }

    String getJSONString(const String& json, const String& key) {
        String searchKey = "\"" + key + "\"";
        int keyPos = json.indexOf(searchKey);
        if (keyPos == -1) return "";
        
        int colonPos = json.indexOf(':', keyPos + searchKey.length());
        if (colonPos == -1) return "";
        
        int startQuote = json.indexOf('\"', colonPos + 1);
        if (startQuote == -1) return "";
        
        int endQuote = json.indexOf('\"', startQuote + 1);
        if (endQuote == -1) return "";
        
        return json.substring(startQuote + 1, endQuote);
    }

    int getJSONInt(const String& json, const String& key, int defaultValue) {
        String searchKey = "\"" + key + "\"";
        int keyPos = json.indexOf(searchKey);
        if (keyPos == -1) return defaultValue;
        
        int colonPos = json.indexOf(':', keyPos + searchKey.length());
        if (colonPos == -1) return defaultValue;
        
        int valPos = colonPos + 1;
        while (valPos < json.length() && (isspace(json[valPos]) || json[valPos] == ':')) {
            valPos++;
        }
        
        if (valPos >= json.length()) return defaultValue;
        
        String valStr = "";
        if (json[valPos] == '-') {
            valStr += '-';
            valPos++;
        }
        while (valPos < json.length() && isDigit(json[valPos])) {
            valStr += json[valPos];
            valPos++;
        }
        
        if (valStr.length() == 0 || valStr == "-") return defaultValue;
        return valStr.toInt();
    }

    bool getJSONBool(const String& json, const String& key, bool defaultValue) {
        String searchKey = "\"" + key + "\"";
        int keyPos = json.indexOf(searchKey);
        if (keyPos == -1) return defaultValue;
        
        int colonPos = json.indexOf(':', keyPos + searchKey.length());
        if (colonPos == -1) return defaultValue;
        
        int valPos = colonPos + 1;
        while (valPos < json.length() && (isspace(json[valPos]) || json[valPos] == ':')) {
            valPos++;
        }
        
        if (valPos >= json.length()) return defaultValue;
        
        if (json.substring(valPos, valPos + 4) == "true") return true;
        if (json.substring(valPos, valPos + 5) == "false") return false;
        
        return defaultValue;
    }

    uint32_t getFreeRAM() {
        char top;
        return &top - reinterpret_cast<char*>(sbrk(0));
    }

    String formatUptime(uint32_t uptimeSecs) {
        uint32_t days = uptimeSecs / 86400;
        uint32_t hours = (uptimeSecs % 86400) / 3600;
        uint32_t mins = (uptimeSecs % 3600) / 60;
        uint32_t secs = uptimeSecs % 60;
        
        String res = "";
        if (days > 0) res += String(days) + "d ";
        if (days > 0 || hours > 0) res += String(hours) + "h ";
        if (days > 0 || hours > 0 || mins > 0) res += String(mins) + "m ";
        res += String(secs) + "s";
        return res;
    }

} // namespace Utils
