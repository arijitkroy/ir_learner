#ifndef WEB_H
#define WEB_H

#include <Arduino.h>
#include <WiFiS3.h>
#include "API.h"

#define MAX_WS_CLIENTS 4

class Web {
private:
    WiFiServer _server;
    WiFiClient _wsClients[MAX_WS_CLIENTS];
    unsigned long _wsGraceUntil[MAX_WS_CLIENTS]; // Grace period: don't check connected() until this time
    API* _api;

    void handleHTTPRequest(WiFiClient& client);
    void handleWebSocketClient(WiFiClient& client);
    void performWebSocketHandshake(WiFiClient& client, const String& secWebSocketKey);
    void serveStatic(WiFiClient& client, const char* content, const char* contentType);
    void sendHTTPError(WiFiClient& client, int code, const String& msg);

public:
    Web(API* api);

    /**
     * @brief Launches the HTTP and WebSocket servers on port 80.
     */
    void begin();

    /**
     * @brief Periodically services incoming HTTP connections and WebSocket frames.
     */
    void loop();

    /**
     * @brief Broadcasts a JSON message to all connected WebSocket clients.
     * @param message The JSON text string.
     */
    void broadcast(const String& message);
};

#endif // WEB_H
