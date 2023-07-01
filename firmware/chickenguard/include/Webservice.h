#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "NonVolatileStorage.h"

class Webservice
{
public:
    Webservice(NonVolatileStorage* nonVolatileStorage, void (*updateTime)(long utc, const String timezone), void (*cbDataReceived)(void));
    ~Webservice();
    void setup();
    void loop();
    void notifyClients(String key, String status);
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

private:
    const IPAddress localIP;    // the IP address the web server, Samsung requires the IP to be in public space
    const IPAddress subnetMask; // no need to change: https://avinetworks.com/glossary/subnet-mask/
    DNSServer dnsServer;
    AsyncWebServer server; // HTTP port 80
    AsyncWebSocket ws;
    bool isInitialized = false;
    NonVolatileStorage* _nonVolatileStorage;
    void (*_cbDataReceived)(void) = nullptr;
    void (*_updateTime)(long utc, const String timezone) = nullptr;
};