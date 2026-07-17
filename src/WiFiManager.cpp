#include "WiFiManager.h"

WiFiManager::WiFiManager() : _state(STATE_DISCONNECTED), _connectStartTime(0), _lastReconnectCheck(0) {
    memset(_apSSID, 0, sizeof(_apSSID));
}

WiFiManager::~WiFiManager() {
}

void WiFiManager::generateAPSSID() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(_apSSID, "IR_Learner_%02X%02X", mac[4], mac[5]);
}

void WiFiManager::startAP() {
    WiFi.disconnect();
    generateAPSSID();
    
    // Start local Access Point
    WiFi.beginAP(_apSSID);
    _state = STATE_AP_MODE;
    
    Serial.print("WiFi connection failed or unconfigured. Started AP: ");
    Serial.println(_apSSID);
    Serial.print("Access the web UI at http://192.168.4.1\n");
}

bool WiFiManager::begin(const SystemConfig& config) {
    _config = config;
    generateAPSSID();

    if (_config.wifiConfigured == 1 && strlen(_config.ssid) > 0) {
        _state = STATE_CONNECTING;
        
        // Apply static IP configuration if specified
        if (_config.ip[0] != 0 || _config.ip[1] != 0) {
            IPAddress local(_config.ip[0], _config.ip[1], _config.ip[2], _config.ip[3]);
            IPAddress gateway(_config.gateway[0], _config.gateway[1], _config.gateway[2], _config.gateway[3]);
            IPAddress subnet(_config.subnet[0], _config.subnet[1], _config.subnet[2], _config.subnet[3]);
            WiFi.config(local, gateway, subnet);
        }

        Serial.print("Connecting to WiFi: ");
        Serial.println(_config.ssid);
        
        WiFi.begin(_config.ssid, _config.password);
        _connectStartTime = millis();
        _lastReconnectCheck = millis();
        return true;
    } else {
        startAP();
        return false;
    }
}

void WiFiManager::loop() {
    switch (_state) {
        case STATE_CONNECTING: {
            if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
                _state = STATE_CONNECTED;
                Serial.print("WiFi Connected! IP Address: ");
                Serial.println(WiFi.localIP());
                
                // Initialize mDNS responder
                if (_mdns.begin(WiFi.localIP(), _config.mdnsName)) {
                    Serial.print("mDNS Responder started: http://");
                    Serial.print(_config.mdnsName);
                    Serial.println(".local");
                }
            } else if (millis() - _connectStartTime > 15000) {
                // Connection timeout (15 seconds), fall back to AP mode
                startAP();
            }
            break;
        }
        case STATE_CONNECTED: {
            // Keep mDNS active
            _mdns.loop();

            // Periodic connection health check (every 5 seconds)
            if (millis() - _lastReconnectCheck > 5000) {
                _lastReconnectCheck = millis();
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("WiFi connection lost! Attempting reconnect...");
                    _state = STATE_CONNECTING;
                    WiFi.begin(_config.ssid, _config.password);
                    _connectStartTime = millis();
                }
            }
            break;
        }
        case STATE_AP_MODE:
            // In AP mode, we just wait. If user submits configuration via API,
            // the server will save settings and reboot the device.
            break;
            
        case STATE_DISCONNECTED:
        default:
            break;
    }
}

String WiFiManager::getIP() const {
    if (_state == STATE_CONNECTED) {
        return WiFi.localIP().toString();
    } else if (_state == STATE_AP_MODE) {
        return "192.168.4.1";
    }
    return "0.0.0.0";
}

int32_t WiFiManager::getRSSI() const {
    if (_state == STATE_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiManager::forceAPMode() {
    startAP();
}
