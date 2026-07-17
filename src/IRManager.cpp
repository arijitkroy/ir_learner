#include "IRManager.h"
#include "Utils.h"

// Global volatile flags triggered from REST API to execute safe delayed actions
volatile bool rebootRequested = false;
volatile bool factoryResetRequested = false;

static IRManager* globalManagerInstance = nullptr;

void logSystemMessage(const String& level, const String& msg) {
    if (globalManagerInstance) {
        globalManagerInstance->log(level, msg);
    }
}

IRManager::IRManager() : _api(nullptr), _web(nullptr), _lastUptimeUpdate(0), _lastRxState(IRReceiver::STATE_IDLE) {
    globalManagerInstance = this;
}

IRManager::~IRManager() {
    if (_api) delete _api;
    if (_web) delete _web;
}

IRManager* IRManager::getInstance() {
    return globalManagerInstance;
}

void IRManager::begin() {
    Serial.begin(115200);
    // Wait for serial line to initialize on native USB core
    delay(500); 
    
    Serial.println("\n====================================");
    Serial.println("IR Remote Learning Web Server Booting");
    Serial.println("====================================");

    // 1. Initialize persistent storage
    bool storageOk = Storage::getInstance().begin();
    if (!storageOk) {
        Serial.println("[WARN] Storage CRC verification failed! Defaults loaded.");
    } else {
        Serial.println("[INFO] Storage database integrity verified (CRC OK).");
    }

    // 2. Initialize IR hardware modules
    _sender.begin();
    _receiver.begin();

    // 3. Initialize REST and Web Servers
    _api = new API(&_receiver, &_sender, &Storage::getInstance(), &_wifi);
    _web = new Web(_api);

    // 4. Connect to Network
    const SystemConfig& config = Storage::getInstance().getSystemConfig();
    _wifi.begin(config);

    // 5. Start Web Server
    _web->begin();

    _lastUptimeUpdate = millis();
    _lastRxState = _receiver.getState();
    
    log("info", "System initialized successfully. Uptime counter started.");
}

void IRManager::loop() {
    // Drive execution ticks for sub-modules
    _wifi.loop();
    _web->loop();
    _receiver.loop();

    // Monitor and handle receiver state transitions (asynchronous captures)
    handleRxStateTransitions();

    // Process pending system reset tasks (reboot / factory reset)
    if (rebootRequested) {
        log("warn", "Reboot requested. System resetting in 1 second...");
        delay(1000);
        NVIC_SystemReset();
    }

    if (factoryResetRequested) {
        log("error", "FATAL: Factory reset requested! Wiping non-volatile memory...");
        Storage::getInstance().factoryReset();
        delay(1000);
        NVIC_SystemReset();
    }

    // Periodically log diagnostics (every 30 seconds)
    if (millis() - _lastUptimeUpdate > 30000) {
        _lastUptimeUpdate = millis();
        log("info", "System heartbeat. Free RAM: " + String(Utils::getFreeRAM()) + " bytes.");
    }
}

void IRManager::handleRxStateTransitions() {
    IRReceiver::State currentState = _receiver.getState();
    if (currentState != _lastRxState) {
        
        if (currentState == IRReceiver::STATE_LEARNING) {
            log("info", "IR receiver is listening for signal...");
            _web->broadcast("{\"event\":\"log\",\"level\":\"info\",\"msg\":\"Receiver is listening...\"}");
        } 
        else if (currentState == IRReceiver::STATE_CAPTURING) {
            log("info", "IR signal edge detected. Capturing pulses...");
            _web->broadcast("{\"event\":\"log\",\"level\":\"info\",\"msg\":\"Capturing IR pulses...\"}");
        }
        else if (currentState == IRReceiver::STATE_FINISHED) {
            // Signal completed capturing
            log("info", "Capture completed. Analyzing signal...");
            
            DecodeResult result;
            bool success = _receiver.decode(result);
            
            if (success) {
                // Broadcast success event with decoded properties
                String eventMsg = "{\n";
                eventMsg += "  \"event\": \"learn\",\n";
                eventMsg += "  \"status\": \"success\",\n";
                eventMsg += "  \"protocol\": " + String(result.protocol) + ",\n";
                eventMsg += "  \"address\": " + String(result.address) + ",\n";
                eventMsg += "  \"command\": " + String(result.command) + ",\n";
                eventMsg += "  \"bits\": " + String(result.bits) + ",\n";
                eventMsg += "  \"quality\": " + String(result.quality) + "\n";
                eventMsg += "}";
                _web->broadcast(eventMsg);
                
                log("info", "Decoded protocol " + String(result.protocol) + " (Addr=" + String(result.address) + ", Cmd=" + String(result.command) + ")");
            } else {
                // Capture failed to match known protocol, classified as RAW
                String eventMsg = "{\n";
                eventMsg += "  \"event\": \"learn\",\n";
                eventMsg += "  \"status\": \"success\",\n";
                eventMsg += "  \"protocol\": 0,\n"; // RAW
                eventMsg += "  \"address\": 0,\n";
                eventMsg += "  \"command\": 0,\n";
                eventMsg += "  \"bits\": " + String(result.bits) + ",\n";
                eventMsg += "  \"pulses\": " + String(_receiver.getRawCount()) + ",\n";
                eventMsg += "  \"quality\": 50,\n";
                eventMsg += "  \"raw\": [";
                
                // Fetch and append raw timings to allow client to save RAW captures
                uint16_t pulsesCount = _receiver.getRawCount();
                if (pulsesCount > 0) {
                    uint16_t* rawBuf = (uint16_t*)malloc(pulsesCount * sizeof(uint16_t));
                    if (rawBuf) {
                        uint16_t len = _receiver.getRawTimings(rawBuf, pulsesCount);
                        for (uint16_t j = 0; j < len; j++) {
                            eventMsg += String(rawBuf[j]);
                            if (j < len - 1) eventMsg += ",";
                        }
                        free(rawBuf);
                    }
                }
                eventMsg += "]\n";
                eventMsg += "}";
                
                _web->broadcast(eventMsg);
                log("warn", "Unable to decode. Classified as RAW (" + String(_receiver.getRawCount()) + " pulses)");
            }
            
            _receiver.stopLearning();
        }
        else if (currentState == IRReceiver::STATE_OVERFLOW) {
            log("error", "IR receiver buffer overflow (>400 pulses)!");
            _web->broadcast("{\"event\":\"learn\",\"status\":\"fail\",\"msg\":\"Buffer overflow\"}");
            _receiver.stopLearning();
        }
        else if (currentState == IRReceiver::STATE_TIMEOUT) {
            log("warn", "IR receiver learning timed out waiting for signal.");
            _web->broadcast("{\"event\":\"learn\",\"status\":\"fail\",\"msg\":\"Timeout waiting for signal\"}");
            _receiver.stopLearning();
        }

        _lastRxState = currentState;
    }

    // Stream pulse counts in real-time during captures
    if (currentState == IRReceiver::STATE_CAPTURING) {
        static uint16_t lastPulseCount = 0;
        uint16_t currentPulses = _receiver.getRawCount();
        if (currentPulses != lastPulseCount) {
            _web->broadcast("{\"event\":\"pulse\",\"count\":" + String(currentPulses) + "}");
            lastPulseCount = currentPulses;
        }
    }
}

void IRManager::log(const String& level, const String& msg) {
    // Print standard serial output
    String lvl = level;
    lvl.toUpperCase();
    
    Serial.print("[");
    Serial.print(lvl);
    Serial.print("] ");
    Serial.println(msg);

    // Broadcast log message over WebSocket connections
    if (_web) {
        String wsLog = "{\"event\":\"log\",\"level\":\"" + level + "\",\"msg\":\"" + msg + "\"}";
        _web->broadcast(wsLog);
    }
}
