/**
 */
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi_credentials.h"
#include "pins.h"
#include <DNSServer.h>
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "DS1337.h"

static const char *TAG = "Main";

void onRootRequest(AsyncWebServerRequest *request);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

const IPAddress localIP(4, 3, 2, 1);          // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress subnetMask(255, 255, 255, 0); // no need to change: https://avinetworks.com/glossary/subnet-mask/
const String localIPURL = "http://4.3.2.1";   // a string version of the local IP with http, used for redirecting clients to your webpage

DNSServer dnsServer;
AsyncWebServer server(80); // HTTP port 80
AsyncWebSocket ws("/ws");
AsyncDelay timeShowDelay;
AsyncDelay ledBlinkDelay;
TimeControl timeControl;
NonVolatileStorage config;
DS1337 rtc(readBytes, writeBytes);

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn LED on, don't wait for timer to expire first.
    pinMode(EN_PWR, OUTPUT);
    digitalWrite(EN_PWR, HIGH); // take over power enable pin from momentary switch.turn LED on, don't wait for timer to expire first.
    Serial.begin(115200);
    while (!Serial)
        ;
    ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);

    if (!SPIFFS.begin())
    {
        ESP_LOGE(TAG, "Cannot mount SPIFFS volume...be sure to upload Filesystem Image before uploading the sketch");
        while (1)
            ;
    }
    config.restoreAll();
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
    timeShowDelay.start(5000, AsyncDelay::MILLIS);
    ledBlinkDelay.start(500, AsyncDelay::MILLIS);

    if (!i2c_hal_init(9, 8))
    {
        ESP_LOGE(TAG, "I2C init failed");
        while (1)
            ;
    }

    if (!detectI2cDevice(rtc.getI2cAddress()))
    {
        ESP_LOGE(TAG, "DS1337 not found");
        while (1)
            ;
    }
    ESP_LOGI(TAG, "DS1337 found");

    int32_t unixtime = 1687093084; // result of linux cli : date '+%s'
    timeval epoch = {unixtime, 0};
    settimeofday((const timeval *)&epoch, 0);
    time_t now;
    time(&now);
    if (!rtc.setTime(now))
    {
        ESP_LOGE(TAG, "Failed to set time");
        while (1)
            ;
    }
    rtc.enableSquareWave(false); // disable square wave to save power
}

void loop()
{
    dnsServer.processNextRequest();
    ws.cleanupClients();
    if (timeShowDelay.isExpired())
    {
        timeShowDelay.repeat();
        time_t now;
        char strftime_buf[64];
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Brussels is: %s", strftime_buf);
    }
    if (ledBlinkDelay.isExpired())
    {
        ledBlinkDelay.repeat();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}

String processor(const String &var)
{
    return String(var == "STATE" ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html", false, processor);
    response->addHeader("Cache-Control", "public,max-age=0"); //
    request->send(response);
}

void notifyClients(String status)
{
    const uint8_t size = JSON_OBJECT_SIZE(1);
    StaticJsonDocument<size> json;
    json["status"] = status.c_str();

    char buffer[size + 10];
    size_t len = serializeJson(json, buffer);
    ws.textAll(buffer, len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
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
            notifyClients("error");
            return;
        }

        // Update time
        long utc = json["UTCSeconds"];
        String timeZone = json["Timezone"];
        timeControl.updateMcuTime(utc, timeZone);

        // Update location
        float latitude = json["Latitude"];
        float longitude = json["Longitude"];
        config.setGeoLocation(latitude, longitude);

        // Update door control
        String doorControl = json["DoorControl"];
        config.setDoorControl(doorControl);

        // Update automatic opening and closing time
        String fixOpeningTime = json["AutomaticOpeningTime"];
        String fixClosingTime = json["AutomaticClosingTime"];
        config.setFixOpeningTime(fixOpeningTime);
        config.setFixClosingTime(fixClosingTime);

        notifyClients("Data received");
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