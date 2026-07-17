#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "Storage.h"
#include "SimpleMDNS.h"

class WiFiManager {
public:
    enum State {
        STATE_DISCONNECTED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_AP_MODE
    };

private:
    State _state;
    SystemConfig _config;
    uint32_t _connectStartTime;
    uint32_t _lastReconnectCheck;
    char _apSSID[33];
    SimpleMDNS _mdns;

    void startAP();
    void generateAPSSID();

public:
    WiFiManager();
    ~WiFiManager();

    /**
     * @brief Initializes WiFi in STA mode if configured, or falls back to AP configuration portal.
     * @param config The system config containing credentials.
     * @return true if starting in STA mode, false if starting in AP mode.
     */
    bool begin(const SystemConfig& config);

    /**
     * @brief Periodically checks connection status, manages retries, and updates mDNS.
     */
    void loop();

    /**
     * @brief Checks if connected in Station mode.
     * @return True if connected.
     */
    bool isConnected() const { return _state == STATE_CONNECTED; }

    /**
     * @brief Checks if currently operating in local Access Point mode.
     * @return True if in AP mode.
     */
    bool isAPMode() const { return _state == STATE_AP_MODE; }

    /**
     * @brief Returns current IP address.
     * @return IP address as String.
     */
    String getIP() const;

    /**
     * @brief Returns RSSI signal strength in dBm.
     * @return RSSI value.
     */
    int32_t getRSSI() const;

    /**
     * @brief Gets the SSID of the local Access Point.
     * @return AP SSID.
     */
    String getAPSSID() const { return String(_apSSID); }

    /**
     * @brief Forces restart into AP Mode.
     */
    void forceAPMode();
};

#endif // WIFIMANAGER_H
