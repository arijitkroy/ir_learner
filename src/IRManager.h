#ifndef IRMANAGER_H
#define IRMANAGER_H

#include <Arduino.h>
#include "WiFiManager.h"
#include "Storage.h"
#include "IRReceiver.h"
#include "IRSender.h"
#include "API.h"
#include "Web.h"

class IRManager {
private:
    WiFiManager _wifi;
    IRReceiver _receiver;
    IRSender _sender;
    API* _api;
    Web* _web;

    uint32_t _lastUptimeUpdate;
    IRReceiver::State _lastRxState;

    void handleRxStateTransitions();

public:
    IRManager();
    ~IRManager();

    /**
     * @brief Bootstraps and initializes all hardware and software modules.
     */
    void begin();

    /**
     * @brief The main loop scheduler. Drives ticks for WiFi, Web, Receiver, and pending Reboots.
     */
    void loop();

    /**
     * @brief Logs system messages to Serial and broadcasts them dynamically over WebSockets.
     * @param level Log severity level ("info", "warn", "error").
     * @param msg Log message text.
     */
    void log(const String& level, const String& msg);

    /**
     * @brief Singleton-like access helper for global scope (e.g. interrupt logging).
     */
    static IRManager* getInstance();
};

#endif // IRMANAGER_H
