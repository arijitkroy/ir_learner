#include "Web.h"
#include "WebFiles.h"
#include "Utils.h"

// Extern logging callback in IRManager to broadcast log messages over WebSockets
void logSystemMessage(const String& level, const String& msg);

Web::Web(API* api) : _server(80), _api(api) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        _wsClients[i] = WiFiClient();
        _wsGraceUntil[i] = 0;
    }
}

void Web::begin() {
    _server.begin();
    Serial.println("Web and WebSocket servers started on port 80");
}

void Web::loop() {
    // 1. Accept new client connections as local variable
    WiFiClient newClient = _server.accept();
    if (newClient) {
        handleHTTPRequest(newClient);
    }

    // 2. Service existing WebSocket clients
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (_wsClients[i]) {
            // Skip connected() check during grace period (3s after upgrade)
            if (millis() < _wsGraceUntil[i]) {
                // Still in grace period, only handle incoming data
                if (_wsClients[i].available() > 0) {
                    handleWebSocketClient(_wsClients[i]);
                }
                continue;
            }

            if (!_wsClients[i].connected()) {
                _wsClients[i].stop();
                _wsClients[i] = WiFiClient();
                _wsGraceUntil[i] = 0;
                Serial.print("WebSocket client ");
                Serial.print(i);
                Serial.println(" disconnected.");
                continue;
            }

            if (_wsClients[i].available() > 0) {
                handleWebSocketClient(_wsClients[i]);
            }
        }
    }
}

void Web::handleHTTPRequest(WiFiClient& client) {
    // Wait for data to arrive (up to 100ms)
    unsigned long timeout = millis() + 100;
    while (!client.available() && millis() < timeout) {
        delay(1);
    }

    String reqLine = "";
    if (client.available()) {
        reqLine = client.readStringUntil('\n');
    }
    reqLine.trim();

    if (reqLine.length() == 0) {
        client.stop();
        return;
    }

    // Parse method and URI
    int firstSpace = reqLine.indexOf(' ');
    int secondSpace = reqLine.indexOf(' ', firstSpace + 1);
    if (firstSpace == -1 || secondSpace == -1) {
        client.stop();
        return;
    }

    String method = reqLine.substring(0, firstSpace);
    String uri = reqLine.substring(firstSpace + 1, secondSpace);

    // Read remaining headers with timeout
    bool isWSUpgrade = false;
    String secWebSocketKey = "";
    int contentLength = 0;

    while (true) {
        // Wait for next header line (up to 200ms)
        unsigned long hdrTimeout = millis() + 200;
        while (!client.available() && millis() < hdrTimeout) {
            delay(1);
        }
        if (!client.available()) break;

        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) break; // Headers section ends with empty line

        int colon = line.indexOf(':');
        if (colon != -1) {
            String headerName = line.substring(0, colon);
            headerName.trim();
            String headerValue = line.substring(colon + 1);
            headerValue.trim();

            if (headerName.equalsIgnoreCase("Upgrade") && headerValue.equalsIgnoreCase("websocket")) {
                isWSUpgrade = true;
            } else if (headerName.equalsIgnoreCase("Sec-WebSocket-Key")) {
                secWebSocketKey = headerValue;
            } else if (headerName.equalsIgnoreCase("Content-Length")) {
                contentLength = headerValue.toInt();
            }
        }
    }

    // Handle WebSocket Handshake Upgrade (do NOT close the client - it stays open)
    if (isWSUpgrade && secWebSocketKey.length() > 0) {
        performWebSocketHandshake(client, secWebSocketKey);
        return;
    }

    // Handle REST API request or serve Static Files
    String body = "";
    if (contentLength > 0) {
        // Read body payload with timeout
        unsigned long bodyTimeout = millis() + 1000;
        for (int i = 0; i < contentLength && millis() < bodyTimeout; i++) {
            while (!client.available() && millis() < bodyTimeout) {
                delay(1);
            }
            if (client.available()) {
                body += (char)client.read();
            }
        }
    }

    if (method == "GET") {
        if (uri == "/" || uri == "/index.html") {
            serveStatic(client, INDEX_HTML, "text/html");
        } else if (uri == "/style.css") {
            serveStatic(client, STYLE_CSS, "text/css");
        } else if (uri == "/app.js") {
            serveStatic(client, APP_JS, "application/javascript");
        } else if (uri.startsWith("/api/")) {
            int statusCode = 200;
            String res = _api->handleRequest(method, uri, body, statusCode);
            sendHTTPError(client, statusCode, res);
        } else {
            sendHTTPError(client, 404, "{\"status\":\"error\",\"msg\":\"Not Found\"}");
        }
    } else if (method == "POST") {
        if (uri.startsWith("/api/")) {
            int statusCode = 200;
            String res = _api->handleRequest(method, uri, body, statusCode);
            sendHTTPError(client, statusCode, res);
        } else {
            sendHTTPError(client, 404, "{\"status\":\"error\",\"msg\":\"Not Found\"}");
        }
    } else {
        sendHTTPError(client, 405, "{\"status\":\"error\",\"msg\":\"Method Not Allowed\"}");
    }

    // Flush and close HTTP connection to free the TCP socket on the ESP32-S3
    delay(5);
    client.stop();
}

void Web::performWebSocketHandshake(WiFiClient& client, const String& secWebSocketKey) {
    // 1. Append Magic Key String
    String combined = secWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // 2. Compute SHA-1
    uint8_t hash[20];
    Utils::sha1((const uint8_t*)combined.c_str(), combined.length(), hash);

    // 3. Base64 Encode
    String acceptKey = Utils::base64Encode(hash, 20);

    // 4. Send Handshake Response
    client.print("HTTP/1.1 101 Switching Protocols\r\n");
    client.print("Upgrade: websocket\r\n");
    client.print("Connection: Upgrade\r\n");
    client.print("Sec-WebSocket-Accept: ");
    client.print(acceptKey);
    client.print("\r\n\r\n");
    client.flush();

    // 5. Add to active WS client slots with grace period
    int slot = -1;
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (!_wsClients[i]) {
            _wsClients[i] = client;
            _wsGraceUntil[i] = millis() + 3000; // 3s grace: don't check connected() during this time
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        Serial.print("WebSocket client connected in slot ");
        Serial.println(slot);
    } else {
        Serial.println("WebSocket connection rejected: server full!");
        client.stop();
    }
}

void Web::serveStatic(WiFiClient& client, const char* content, const char* contentType) {
    client.print("HTTP/1.1 200 OK\r\n");
    client.print("Content-Type: ");
    client.print(contentType);
    client.print("\r\n");
    client.print("Cache-Control: max-age=86400\r\n");
    client.print("Connection: close\r\n");
    
    size_t len = strlen_P(content);
    client.print("Content-Length: ");
    client.print(len);
    client.print("\r\n\r\n");
    
    size_t bytesSent = 0;
    while (bytesSent < len) {
        size_t chunkSize = (len - bytesSent > 256) ? 256 : (len - bytesSent);
        uint8_t buffer[256];
        memcpy_P(buffer, content + bytesSent, chunkSize);
        client.write(buffer, chunkSize);
        bytesSent += chunkSize;
        delay(2); // Prevent SPI transmission buffer overflow on the ESP32-S3 coprocessor
    }
}

void Web::sendHTTPError(WiFiClient& client, int code, const String& msg) {
    client.print("HTTP/1.1 ");
    client.print(code);
    client.print(" ");
    if (code == 200) client.print("OK");
    else if (code == 400) client.print("Bad Request");
    else if (code == 404) client.print("Not Found");
    else if (code == 405) client.print("Method Not Allowed");
    else client.print("Internal Server Error");
    client.print("\r\n");
    
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: ");
    client.print(msg.length());
    client.print("\r\n");
    client.print("Access-Control-Allow-Origin: *\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(msg);
}

void Web::handleWebSocketClient(WiFiClient& client) {
    // Read standard WebSocket frame (RFC 6455)
    uint8_t b1 = client.read();
    uint8_t b2 = client.read();

    bool fin = b1 & 0x80;
    uint8_t opcode = b1 & 0x0F;
    bool masked = b2 & 0x80;
    uint64_t payloadLen = b2 & 0x7F;

    if (payloadLen == 126) {
        uint16_t extraLen = ((uint16_t)client.read() << 8) | client.read();
        payloadLen = extraLen;
    } else if (payloadLen == 127) {
        // We do not support extremely large payloads (> 65KB) for RAM constraints
        for (int i = 0; i < 8; i++) client.read();
        payloadLen = 0;
    }

    uint8_t maskKey[4] = {0};
    if (masked) {
        for (int i = 0; i < 4; i++) {
            maskKey[i] = client.read();
        }
    }

    if (opcode == 0x08) { // Connection Close
        client.stop();
        return;
    }

    // Read payload
    if (payloadLen > 0 && payloadLen < 1024) {
        uint8_t* payload = (uint8_t*)malloc(payloadLen + 1);
        if (payload) {
            for (uint64_t i = 0; i < payloadLen; i++) {
                payload[i] = client.read();
                if (masked) {
                    payload[i] ^= maskKey[i % 4];
                }
            }
            payload[payloadLen] = '\0';

            String msg = String((char*)payload);
            free(payload);

            if (opcode == 0x01) { // Text frame
                if (msg == "PING") {
                    // Send PONG
                    client.write(0x81); // FIN, Text
                    client.write(4);    // Length 4
                    client.print("PONG");
                }
            }
        }
    } else {
        // Flush buffer if payload is too large/corrupted
        for (uint64_t i = 0; i < payloadLen; i++) {
            client.read();
        }
    }
}

void Web::broadcast(const String& message) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (!_wsClients[i]) continue;
        // During grace period, trust the connection; after grace, check connected()
        if (millis() >= _wsGraceUntil[i] && !_wsClients[i].connected()) continue;

        _wsClients[i].write(0x81); // Text frame
        
        size_t payloadSize = message.length();
        if (payloadSize < 126) {
            _wsClients[i].write((uint8_t)payloadSize);
        } else {
            _wsClients[i].write(126);
            _wsClients[i].write((uint8_t)(payloadSize >> 8));
            _wsClients[i].write((uint8_t)(payloadSize & 0xFF));
        }
        _wsClients[i].print(message);
    }
}
