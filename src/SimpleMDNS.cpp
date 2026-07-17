#include "SimpleMDNS.h"

SimpleMDNS::SimpleMDNS() : _started(false) {}

bool SimpleMDNS::begin(IPAddress ip, const char* hostname) {
    _ip = ip;
    _hostname = String(hostname);
    _hostname.toLowerCase();
    
    // mDNS standard multicast group: 224.0.0.251, port 5353
    IPAddress group(224, 0, 0, 251);
    if (_udp.beginMulticast(group, 5353)) {
        _started = true;
        Serial.print("Simple mDNS responder started for hostname: ");
        Serial.print(_hostname);
        Serial.println(".local");
        return true;
    } else {
        Serial.println("Failed to start Simple mDNS responder.");
        return false;
    }
}

void SimpleMDNS::loop() {
    if (!_started) return;

    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) return;

    // Allocate buffer for packet
    uint8_t* buffer = (uint8_t*)malloc(packetSize);
    if (!buffer) return;

    _udp.read(buffer, packetSize);

    // Parse DNS packet header (needs to be at least 12 bytes)
    if (packetSize >= 12) {
        uint16_t flags = (buffer[2] << 8) | buffer[3];
        uint16_t questions = (buffer[4] << 8) | buffer[5];
        
        bool isQuery = !(flags & 0x8000); // QR bit = 0 is query
        
        if (isQuery && questions > 0) {
            // Find hostname in the questions section
            // Convert buffer to lowercase characters for matching
            String queryStr = "";
            for (int i = 12; i < packetSize; i++) {
                char c = (char)buffer[i];
                if (c >= 32 && c <= 126) {
                    queryStr += (char)tolower(c);
                } else {
                    queryStr += '.'; // demarcator
                }
            }

            String targetQuery = "." + _hostname + ".local.";
            
            if (queryStr.indexOf(targetQuery) != -1 || queryStr.indexOf("." + _hostname + ".") != -1) {
                // Construct mDNS response
                _udp.beginPacket(IPAddress(224, 0, 0, 251), 5353);
                
                // Header (12 bytes)
                uint8_t responseHeader[12] = {
                    0x00, 0x00, // Transaction ID (must be 0 for mDNS multicast)
                    0x84, 0x00, // Flags: Response, Authoritative Answer
                    0x00, 0x00, // Questions: 0
                    0x00, 0x01, // Answer RRs: 1
                    0x00, 0x00, // Authority RRs: 0
                    0x00, 0x00  // Additional RRs: 0
                };
                _udp.write(responseHeader, 12);
                
                // Write hostname query name formatted as DNS labels
                // For "irremote": [8] 'i' 'r' 'r' 'e' 'm' 'o' 't' 'e' [5] 'l' 'o' 'c' 'a' 'l' [0]
                _udp.write((uint8_t)_hostname.length());
                _udp.write((const uint8_t*)_hostname.c_str(), _hostname.length());
                
                _udp.write((uint8_t)5); // "local" length
                _udp.write((const uint8_t*)"local", 5);
                _udp.write((uint8_t)0); // terminating null label
                
                // Answer RR fields
                uint8_t rrFields[10] = {
                    0x00, 0x01, // Type: A (IPv4)
                    0x80, 0x01, // Class: IN (flush cache bit set)
                    0x00, 0x00, 0x00, 0x78, // TTL: 120 seconds
                    0x00, 0x04  // Data length: 4 bytes
                };
                _udp.write(rrFields, 10);
                
                // Write IP Address
                uint8_t ipBytes[4] = { _ip[0], _ip[1], _ip[2], _ip[3] };
                _udp.write(ipBytes, 4);
                
                _udp.endPacket();
                Serial.println("Simple mDNS: Sent response for " + _hostname + ".local");
            }
        }
    }

    free(buffer);
}
