#include <WiFi.h>
// #include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "ArduinoJson.h"

// #include "webserver.h"
#include "config.h"
#include "data.h"

static const char *TAG = "WEB"; // тег для модуля

// WebServer server(SERVER_PORT);
AsyncWebServer server(SERVER_PORT);

const char *ssid = "Ufa100500";       // 🔧 сюда можно вставить сохранённые настройки
const char *password = "A1s2d3f4g56"; // или подгружать из EEPROM

void setupWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT)
    {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        ESP_LOGI(TAG, "Connected! IP: %s", WiFi.localIP().toString());
    }
    else
    {
        ESP_LOGW(TAG, "Failed to connect. Starting AP mode...");

        WiFi.softAP(AP_SSID, AP_PASSWORD);
        ESP_LOGI(TAG, "AP IP address: %s", WiFi.softAPIP().toString());
    }
}

void handleRoot()
{
    //   request->send(LittleFS, "/index.html", "text/html");
    // server.send(200, "text/html", "<h1>Irrigation System</h1><p><a href='/manual?on=1'>Pump ON</a> | <a href='/manual?on=0'>Pump OFF</a></p>");
}

void handleManualControl()
{
    // if (server.hasArg("on"))
    //{
    //     CurrentState.in_IsManualIrrigation = server.arg("on") == "1";
    // }
    // server.sendHeader("Location", "/");
    // server.send(302, "text/plain", "");
}

void onNotFound(AsyncWebServerRequest *request)
{
    String path = request->url();

    ESP_LOGI(TAG, "%s", path.c_str());

    if (LittleFS.exists(path))
    {
        String mime = "text/plain"; // задай MIME по расширению
        if (path.endsWith(".html"))
            mime = "text/html";
        else if (path.endsWith(".css"))
            mime = "text/css";
        else if (path.endsWith(".js"))
            mime = "application/javascript";
        request->send(LittleFS, path, mime);
    }
    else
    {
        request->send(404, "text/plain", "File not found");
    }
}

void setupWebServer()
{
    ESP_LOGI(TAG, "setupWebServer");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
            ESP_LOGI(TAG, "GET /");
            request->send(LittleFS, "/index.html", "text/html"); });

    // API для получения текущего состояния
    server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                    JsonDocument doc;

                    doc["out_IrrigationPumpOn"] = CurrentState.out_IrrigationPumpOn;
                    doc["out_FillPumpOn"] = CurrentState.out_FillPumpOn;
                    doc["in_IsFull"] = CurrentState.in_IsFull;
                    doc["hasTime"] = CurrentState.hasTime;

                    JsonObject timeInfo = doc["timeInfo"].to<JsonObject>();
                    timeInfo["sec"] = CurrentState.timeInfo.tm_sec;
                    timeInfo["min"] = CurrentState.timeInfo.tm_min;
                    timeInfo["hour"] = CurrentState.timeInfo.tm_hour;
                    timeInfo["mday"] = CurrentState.timeInfo.tm_mday;
                    timeInfo["month"] = CurrentState.timeInfo.tm_mon;
                    timeInfo["year"] = CurrentState.timeInfo.tm_year;
                    timeInfo["wday"] = CurrentState.timeInfo.tm_wday;
                    timeInfo["yday"] = CurrentState.timeInfo.tm_yday;
                    timeInfo["isdst"] = CurrentState.timeInfo.tm_isdst;

                    doc["minutesSinceMidnight"] = CurrentState.minutesSinceMidnight;

                    JsonArray schedules = doc["schedules"].to<JsonArray>();
                    for (int i = 0; i < MAX_SCHEDULES; ++i)
                    {
                        auto &s = CurrentState.shedules[i];
                        JsonObject schedule = schedules.createNestedObject();
                        schedule["index"] = i;
                        schedule["enabled"] = s.enabled;
                        schedule["startTime"] = s.startTime;
                        schedule["endTime"] = s.endTime;
                        schedule["irrigationPump"] = s.irrigationPump;
                        schedule["fillPump"] = s.fillPump;
                        
                        JsonArray valves = schedule.createNestedArray("valves");
                        for (int j = 0; j < VALVE_COUNT; ++j)
                        {
                            valves.add(s.valves[j]);
                        }
                    }

                    String json;
                    serializeJson(doc, json);
                    request->send(200, "application/json", json); });

    // API для управления
    server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (request->hasParam("target") && request->hasParam("value")) {
            String target = request->getParam("target")->value();
            bool value = request->getParam("value")->value().toInt();

            if (target == "irrigation") {
                CurrentState.in_IsManualIrrigation = value;
            } else if (target == "fill") {
                CurrentState.in_IsManualFill = value;
            } else {
                request->send(400, "text/plain", "Unknown target");
                return;
            }

            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Missing parameters");
        } });

    server.on(
        "/schedules", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            JsonArray schedules = doc.to<JsonArray>();
            for (int i = 0; i < MAX_SCHEDULES; ++i) {
                auto &s = CurrentState.shedules[i];
                JsonObject schedule = schedules.createNestedObject();
                schedule["index"] = i;
                schedule["enabled"] = s.enabled;
                schedule["startTime"] = s.startTime;
                schedule["endTime"] = s.endTime;
                schedule["irrigationPump"] = s.irrigationPump;
                schedule["fillPump"] = s.fillPump;
                JsonArray valves = schedule.createNestedArray("valves");
                for (int j = 0; j < VALVE_COUNT; ++j) {
                    valves.add(s.valves[j]);
                }
            }

            String json;
            serializeJson(doc, json);
            request->send(200, "application/json", json); });

    AsyncCallbackWebHandler *handler = new AsyncCallbackWebHandler();
    handler->setUri("/schedules");
    handler->setMethod(HTTP_POST);
    handler->onBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                    {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

            if (err)
            {
                request->send(400, "text/plain", "JSON parse error");
                return;
            }

            int indexVal = doc["index"];
            if (indexVal < 0 || indexVal >= MAX_SCHEDULES)
            {
                request->send(400, "text/plain", "Invalid index");
                return;
            }

            ScheduleEntry &s = CurrentState.shedules[indexVal];
            s.enabled = doc["enabled"];
            s.startTime = doc["startTime"];
            s.endTime = doc["endTime"];
            s.irrigationPump = doc["irrigationPump"];
            s.fillPump = doc["fillPump"];

            JsonArray valves = doc["valves"];
            for (int i = 0; i < VALVE_COUNT && i < valves.size(); ++i)
            {
                s.valves[i] = valves[i];
            }

            request->send(200, "text/plain", "OK"); });

    server.addHandler(handler);

    server.onNotFound(onNotFound);

    // server.on("/", handleRoot);
    // server.on("/manual", handleManualControl);
    server.begin();
}

void handleWebRequests()
{
    // server.handleClient();
}
