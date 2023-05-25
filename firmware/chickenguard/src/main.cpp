/**
 */
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi_credentials.h"
#include "pins.h"
#include <DNSServer.h>

static const char* TAG = "Main";

void onRootRequest(AsyncWebServerRequest *request);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    const IPAddress localIP(4, 3, 2, 1);      // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress subnetMask(255, 255, 255, 0); // no need to change: https://avinetworks.com/glossary/subnet-mask/
const String localIPURL = "http://4.3.2.1";   // a string version of the local IP with http, used for redirecting clients to your webpage

DNSServer dnsServer;
AsyncWebServer server(80); // HTTP port 80
AsyncWebSocket ws("/ws");

void setup()
{ 
    Serial.begin(115200);
    while(!Serial);
    delay(2000);
    ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);

    if (!SPIFFS.begin())
    {
        ESP_LOGE(TAG, "Cannot mount SPIFFS volume...");
        while (1)
            ;
    }
#ifdef WIFI_STATION
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    ESP_LOGI(TAG, "Trying to connect [%s] ", WiFi.macAddress().c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        ESP_LOGI(TAG, ".");
        delay(500);
    }
    ESP_LOGI(TAG, " %s\n", WiFi.localIP().toString().c_str());
#else
    WiFi.mode(WIFI_AP);                              // access point mode
    WiFi.softAPConfig(localIP, localIP, subnetMask); // IP address of the gateway should be the same as the local IP for captive portals
    WiFi.softAP(WIFI_SSID, WIFI_PASS);

    dnsServer.setTTL(300);             // set 5min client side cache for DNS
    dnsServer.start(53, "*", localIP); // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
#endif
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/favicon.ico", [](AsyncWebServerRequest *request)
              { request->send(404); }); // webpage icon

    // the catch all
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->redirect(localIPURL); });

    server.on("/", HTTP_ANY, onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
}

void loop()
{
    dnsServer.processNextRequest();
    ws.cleanupClients();
}

String processor(const String &var)
{
    return String(var == "STATE" ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html", false, processor);
    response->addHeader("Cache-Control", "public,max-age=604800"); // 
    request->send(response);
}

void notifyClients()
{
    const uint8_t size = JSON_OBJECT_SIZE(1);
    StaticJsonDocument<size> json;
    json["status"] = "off";

    char buffer[17];
    size_t len = serializeJson(json, buffer);
    ws.textAll(buffer, len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {

        const uint8_t size = JSON_OBJECT_SIZE(6);
        StaticJsonDocument<size> json;
        char buffer[len+1];
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        Serial.println(buffer);
        DeserializationError err = deserializeJson(json, data);
        if (err)
        {
            ESP_LOGE(TAG, "deserializeJson() failed with code %s", err.c_str());
             return;
        }

        long utc = json["UTCSeconds"];
        long latitude = json["Latitude"];
        long longitude = json["Longitude"];
        String doorControl = json["DoorControl"];
        String automaticOpeningTime = json["AutomaticOpeningTime"];
        String automaticClosingTime = json["AutomaticClosingTime"];
        ESP_LOGI(TAG, "%lu", utc);
        ESP_LOGI(TAG, "%lu", latitude);
        ESP_LOGI(TAG, "%lu",longitude);
        ESP_LOGI(TAG, "%s", doorControl);
        ESP_LOGI(TAG, "%s", automaticOpeningTime);
        ESP_LOGI(TAG, "%s", automaticClosingTime);
    }
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{

    switch (type)
    {
    case WS_EVT_CONNECT:
        ESP_LOGI(TAG, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        ESP_LOGI(TAG, "WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}