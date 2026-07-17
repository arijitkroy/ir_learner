#ifndef SIMPLEMDNS_H
#define SIMPLEMDNS_H

#include <Arduino.h>
#include <WiFiUdp.h>

class SimpleMDNS {
private:
    WiFiUDP _udp;
    String _hostname;
    IPAddress _ip;
    bool _started;

public:
    SimpleMDNS();
    bool begin(IPAddress ip, const char* hostname);
    void loop();
};

#endif // SIMPLEMDNS_H
