#ifndef API_H
#define API_H

#include <Arduino.h>
#include "IRReceiver.h"
#include "IRSender.h"
#include "Storage.h"
#include "WiFiManager.h"

class API {
private:
    IRReceiver* _receiver;
    IRSender* _sender;
    Storage* _storage;
    WiFiManager* _wifi;

    // Helper methods to generate JSON responses
    String getStatusJSON();
    String getCodesJSON();
    String getLearnStatusJSON();
    
    // Command Handlers
    String handleSend(const String& body, int& statusCode);
    String handleDelete(const String& body, int& statusCode);
    String handleRename(const String& body, int& statusCode);
    String handleImport(const String& body, int& statusCode);
    String handleWiFi(const String& body, int& statusCode);
    String handleSaveCode(const String& body, int& statusCode);
    
public:
    API(IRReceiver* rx, IRSender* tx, Storage* storage, WiFiManager* wifi);

    /**
     * @brief Parses and routes REST API requests, executing commands and returning JSON.
     * @param method HTTP method (GET, POST).
     * @param uri Request URI.
     * @param body Request body payload (JSON).
     * @param statusCode Reference to output HTTP status code.
     * @return JSON response string.
     */
    String handleRequest(const String& method, const String& uri, const String& body, int& statusCode);
};

#endif // API_H
