#include "Webservice.h"
#include <SPIFFS.h>

#include <ArduinoJson.h>
#include "wifi_credentials.h"

static const char *TAG = "Webservice";
static Webservice *_instance = nullptr;

static String processor(const String &var)
{
    return String(var == "STATE" ? "on" : "off");
}

static void onRootRequest(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html", false, processor);
    response->addHeader("Cache-Control", "public,max-age=0"); //
    request->send(response);
}

static void onEvent(AsyncWebSocket *server,
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
        _instance->handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

Webservice::Webservice(NonVolatileStorage *nonVolatileStorage, void (*updateTime)(long utc, const String timezone),
                       void (*cbDataReceived)(void)) : localIP(4, 3, 2, 1),
                                                       subnetMask(255, 255, 255, 0),
                                                       server(80),
                                                       ws("/ws"),
                                                       _nonVolatileStorage(nonVolatileStorage),
                                                       _updateTime(updateTime),
                                                       _cbDataReceived(cbDataReceived)
{
    _instance = this;
}

Webservice::~Webservice()
{
}

void Webservice::setup()
{
    if(isInitialized)
        return;
    if (!SPIFFS.begin())
    {
        ESP_LOGE(TAG, "Cannot mount SPIFFS volume...be sure to upload Filesystem Image before uploading the sketch");
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
                      { request->redirect("http://4.3.2.1"); }); //// a string version of the local IP with http, used for redirecting clients to your webpage

    server.on("/", HTTP_ANY, onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
    isInitialized = true;
}

void Webservice::loop()
{
    if (!isInitialized)
    {
        return;
    }
    dnsServer.processNextRequest();
    ws.cleanupClients();
}

void Webservice::notifyClients(String key, String status)
{
    if (!isInitialized || !ws.count())
    {
        return;
    }
    ESP_LOGI(TAG, "Notify: key: %s, status: %s", key.c_str(), status.c_str());
    const uint8_t size = JSON_OBJECT_SIZE(3);
    StaticJsonDocument<size> json;
    json["key"] = key.c_str();
    if (key.equals("feedback"))
    {
        json["status"] = status.c_str();
    }
    if (key.equals("battery"))
    {
        json["status"] = status.c_str();
    }

    char buffer[size + 10];
    size_t len = serializeJson(json, buffer);
    ESP_LOGI(TAG, "json: %s", buffer);
    ws.textAll(buffer, len);
}

void Webservice::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {

        const uint8_t size = JSON_OBJECT_SIZE(7);
        StaticJsonDocument<size> json;
        char buffer[len + 1];
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        Serial.println(buffer);
        DeserializationError err = deserializeJson(json, data);
        if (err)
        {
            ESP_LOGE(TAG, "deserializeJson() failed with code %s", err.c_str());
            notifyClients("feedback", "error");
            return;
        }

        // Update time
        long utc = json["UTCSeconds"];
        String timeZone = json["Timezone"];
        _updateTime(utc, timeZone);

        // Update location
        float latitude = json["Latitude"];
        float longitude = json["Longitude"];
        _nonVolatileStorage->setGeoLocation(latitude, longitude);

        // Update door control
        String doorControl = json["DoorControl"];
        _nonVolatileStorage->setDoorControl(doorControl);

        // Update automatic opening and closing time
        String fixOpeningTime = json["AutomaticOpeningTime"];
        String fixClosingTime = json["AutomaticClosingTime"];
        _nonVolatileStorage->setFixOpeningTime(fixOpeningTime);
        _nonVolatileStorage->setFixClosingTime(fixClosingTime);

        notifyClients("feedback", "Data received");
        _cbDataReceived();
    }
}