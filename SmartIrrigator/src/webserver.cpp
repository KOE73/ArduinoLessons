#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ESPAsyncWiFiManager.h>
#include "AsyncOTA.h"

#include "ArduinoJson.h"

// #include "webserver.h"
#include "config.h"
#include "data.h"
#include "ntp_time.h"

static const char *TAG = "WEB"; // тег для модуля

// WebServer server(SERVER_PORT);
AsyncWebServer server(SERVER_PORT);
DNSServer dns;

// const char *ssid = "Ufa100500";       // 🔧 сюда можно вставить сохранённые настройки
// const char *password = "A1s2d3f4g56"; // или подгружать из EEPROM
//  Можно передать NULL сюда, если хочешь использовать сохранённые
const char *ssid = nullptr;
const char *password = nullptr;

void setupWiFi()
{
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.setConnectTimeout(3); // Ждать Wi-Fi только 3 секунды
    wifiManager.setTimeout(180);      // AP доступен 3 минуты
    wifiManager.setDebugOutput(true);

    // Автоподключение или запуск конфигурационного портала
    if (!wifiManager.autoConnect(AP_SSID, AP_PASSWORD))
    {
        ESP_LOGW(TAG, "Не удалось подключиться к Wi-Fi или портал не завершён.");
    }
    else
    {
        ESP_LOGI(TAG, "Wi-Fi подключён! IP: %s", WiFi.localIP().toString().c_str());
    }

    // WiFi.mode(WIFI_STA);
    //
    // if (ssid && password)
    //    WiFi.begin(ssid, password);
    // else
    //    WiFi.begin(); // пробуем сохранённую сеть
    //
    // ESP_LOGI(TAG, "Connecting to WiFi");
    //
    // unsigned long startAttemptTime = millis();
    // while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT)
    //{
    //    ESP_LOGI(TAG, ".");
    //    delay(500);
    //}
    //
    // if (WiFi.status() == WL_CONNECTED)
    //{
    //    ESP_LOGI(TAG, "Connected! IP: %s", WiFi.localIP().toString().c_str());
    //}
    // else
    //{
    //    ESP_LOGW(TAG, "Failed to connect. Starting config portal...");
    //
    //    WiFiManager wm;
    //    WiFi.mode(WIFI_AP_STA); // включаем AP + STA
    //
    //    // стартуем конфигурационный портал без автоперезапуска
    //    if (wm.startConfigPortal(AP_SSID, AP_PASSWORD))
    //    {
    //        ESP_LOGI(TAG, "Connected via portal! IP: %s", WiFi.localIP().toString().c_str());
    //    }
    //    else
    //    {
    //        ESP_LOGW(TAG, "Config portal timeout or failed. Not connected.");
    //    }
    //}
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

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
    // Log when OTA has started
    ESP_LOGI(TAG, "OTA update started!");
    // <Add your own code here>
}

void onOTAProgress()
{
    // Log every 1 second
    if (millis() - ota_progress_millis > 1000)
    {
        ota_progress_millis = millis();
        ESP_LOGI(TAG, "OTA Progress ");
    }
}

void onOTAEnd()
{
    // Log when OTA has finished
    if (1)
    {
        ESP_LOGI(TAG, "OTA update finished successfully!");
    }
    else
    {
        ESP_LOGI(TAG, "There was an error during OTA update!");
    }
    // <Add your own code here>
}

void setupWebServer()
{
    ESP_LOGI(TAG, "setupWebServer");

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            ESP_LOGI(TAG, "GET /");
            request->send(LittleFS, "/index.html", "text/html");
        });

#pragma region /state
    // API для получения текущего состояния
    server.on(
        "/state",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;
            
            doc["hasTime"] = WorkState.hasTime;
            doc["hasNTP"] = WorkState.hasNTP;     
            doc["hasRTC"] = WorkState.hasRTC;     
            doc["hasTimeRTC"] = WorkState.hasTimeRTC;     
            doc["rtcTemperature"] = WorkState.rtcTemperature;     
            doc["rtcLostPower"] = WorkState.rtcLostPower;     

            
            doc["in_IsFull"] = WorkState.in_IsFull;
            doc["in_IsManualFill"] = WorkState.in_IsManualFill;
            doc["in_IsManualIrrigation"] = WorkState.in_IsManualIrrigation;

            doc["out_IrrigationPumpOn"] = WorkState.out_IrrigationPumpOn;
            doc["out_FillPumpOn"] = WorkState.out_FillPumpOn;     
            
            doc["in_IsFull_Confirmed"] = WorkState.in_IsFull_Confirmed;     
  
        
 
            JsonObject nowTimeInfo = doc["nowTimeInfo"].to<JsonObject>();
            nowTimeInfo["sec"] = WorkState.nowTimeInfo.tm_sec;
            nowTimeInfo["min"] = WorkState.nowTimeInfo.tm_min;
            nowTimeInfo["hour"] = WorkState.nowTimeInfo.tm_hour;
            nowTimeInfo["mday"] = WorkState.nowTimeInfo.tm_mday;
            nowTimeInfo["month"] = WorkState.nowTimeInfo.tm_mon;
            nowTimeInfo["year"] = WorkState.nowTimeInfo.tm_year;
            nowTimeInfo["wday"] = WorkState.nowTimeInfo.tm_wday;
            nowTimeInfo["yday"] = WorkState.nowTimeInfo.tm_yday;
            nowTimeInfo["isdst"] = WorkState.nowTimeInfo.tm_isdst;
            
            JsonObject rtcTimeInfo = doc["rtcTimeInfo"].to<JsonObject>();
            rtcTimeInfo["sec"] = WorkState.rtcTimeInfo.tm_sec;
            rtcTimeInfo["min"] = WorkState.rtcTimeInfo.tm_min;
            rtcTimeInfo["hour"] = WorkState.rtcTimeInfo.tm_hour;
            rtcTimeInfo["mday"] = WorkState.rtcTimeInfo.tm_mday;
            rtcTimeInfo["month"] = WorkState.rtcTimeInfo.tm_mon;
            rtcTimeInfo["year"] = WorkState.rtcTimeInfo.tm_year;
            rtcTimeInfo["wday"] = WorkState.rtcTimeInfo.tm_wday;
            rtcTimeInfo["yday"] = WorkState.rtcTimeInfo.tm_yday;
            rtcTimeInfo["isdst"] = WorkState.rtcTimeInfo.tm_isdst;
            
            if(WorkState.in_TimeManualOff != 0)
            {
                JsonObject in_TimeInfoManualOff = doc["in_TimeInfoManualOff"].to<JsonObject>();
                in_TimeInfoManualOff["sec"] = WorkState.in_TimeInfoManualOff.tm_sec;
                in_TimeInfoManualOff["min"] = WorkState.in_TimeInfoManualOff.tm_min;
                in_TimeInfoManualOff["hour"] = WorkState.in_TimeInfoManualOff.tm_hour;
                in_TimeInfoManualOff["mday"] = WorkState.in_TimeInfoManualOff.tm_mday;
                in_TimeInfoManualOff["month"] = WorkState.in_TimeInfoManualOff.tm_mon;
                in_TimeInfoManualOff["year"] = WorkState.in_TimeInfoManualOff.tm_year;
                in_TimeInfoManualOff["wday"] = WorkState.in_TimeInfoManualOff.tm_wday;
                in_TimeInfoManualOff["yday"] = WorkState.in_TimeInfoManualOff.tm_yday;
                in_TimeInfoManualOff["isdst"] = WorkState.in_TimeInfoManualOff.tm_isdst;
            }

            doc["minutesSinceMidnight"] = WorkState.minutesSinceMidnight;

            JsonArray valves = doc["out_ValveOn"].to<JsonArray>();
            JsonArray valvesManual = doc["in_IsManualValveOn"].to<JsonArray>();
            for (int i = 0; i < VALVE_COUNT; ++i)
            {
                valves.add(WorkState.out_ValveOn[i]);
                valvesManual.add(WorkState.in_IsManualValveOn[i]);
            }

            JsonArray schedules = doc["schedules"].to<JsonArray>();
            for (int i = 0; i < MAX_SCHEDULES; ++i)
            {
                auto &s = WorkState.shedules[i];
                JsonObject schedule = schedules.add<JsonObject>();
                schedule["index"] = i;
                schedule["enabled"] = s.enabled;
                schedule["startTime"] = s.startTime;
                schedule["endTime"] = s.endTime;
                schedule["irrigationPump"] = s.irrigationPump;
                schedule["fillPump"] = s.fillPump;

                JsonArray valves = schedule["valves"].to<JsonArray>();
                for (int j = 0; j < VALVE_COUNT; ++j)
                {
                    valves.add(s.valves[j]);
                }
            }

            String json;
            serializeJson(doc, json);
            request->send(200, "application/json", json); });

#pragma endregion / state

#pragma region /control
    // API для управления
    server.on(
        "/control",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            if (request->hasParam("target") && request->hasParam("value"))
            {
                String target = request->getParam("target")->value();
                auto value = request->getParam("value")->value().toInt();

                ESP_LOGI(TAG, "/control target=%s value=%i", target.c_str(), value);

                if (target == "irrigation") 
                {
                    CurrentState.setManualIrrigation((bool)value);
                }
                else if (target == "fill") 
                {
                    CurrentState.setManualFill((bool)value);
                }
                else if (target.startsWith("valve")) 
                {  
                    int index = target.substring(5).toInt();
                    if (index >= 0 && index < VALVE_COUNT) 
                    {
                        CurrentState.setManualValve(index, (bool)value);
                    }
                }
                else if (target == "manualOffAdjust") 
                {
                    int delta = value;  // уже число минут
                    CurrentState.adjustManualOffTime(delta);
                }             
                else if (target == "syncTimeFromNTP") 
                {
                    syncTimeFromNTP();
                }
                else                
                {
                    request->send(400, "text/plain", "Unknown target");
                    return;
                }

                request->send(200, "text/plain", "OK");
            } 
            else 
            {
                request->send(400, "text/plain", "Missing parameters");
            } });

#pragma endregion / control

#pragma region GET schedules
    server.on(
        "/schedules", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            JsonArray schedules = doc.to<JsonArray>();
            for (int i = 0; i < MAX_SCHEDULES; ++i) {
                auto &s = CurrentState.shedules[i];
                JsonObject schedule = schedules.add<JsonObject>();
                schedule["index"] = i;
                schedule["enabled"] = s.enabled;
                schedule["startTime"] = s.startTime;
                schedule["endTime"] = s.endTime;
                schedule["irrigationPump"] = s.irrigationPump;
                schedule["fillPump"] = s.fillPump;
                JsonArray valves = schedule["valves"].to<JsonArray>();
                for (int j = 0; j < VALVE_COUNT; ++j) {
                    valves.add(s.valves[j]);
                }
            }

            String json;
            serializeJson(doc, json);
            request->send(200, "application/json", json); });

#pragma endregion GET schedules

#pragma region POST schedules
    // Получаем новые настройки расписания
    AsyncCallbackWebHandler *handler = new AsyncCallbackWebHandler();
    handler->setUri("/schedules");
    handler->setMethod(HTTP_POST);
    handler->onRequest([](AsyncWebServerRequest *request) {});
    handler->onBody(
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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

            saveScheduleToNVS(indexVal);

            request->send(200, "text/plain", "OK");
        });

    server.addHandler(handler);
#pragma endregion / schedules update

#pragma region GET setup
    server.on(
        "/setup",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;
            doc["fillPumpMaxRuntimeMs"] = CurrentState.setupData.fillPumpMaxRuntimeMs / 1000;
            doc["fillPumpMinOffTime"] = CurrentState.setupData.fillPumpMinOffTime / 1000;
            doc["fullStableTimeMs"] = CurrentState.setupData.fullStableTimeMs / 1000;

            String json;
            serializeJson(doc, json);
            request->send(200, "application/json", json);
        });

#pragma endregion GET setup

#pragma region POST setup
    AsyncCallbackWebHandler *setupHandler = new AsyncCallbackWebHandler();
    setupHandler->setUri("/setup");
    setupHandler->setMethod(HTTP_POST);
    setupHandler->onRequest([](AsyncWebServerRequest *request) {});

    setupHandler->onBody(
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

            if (err)
            {
                request->send(400, "text/plain", "JSON parse error");
                return;
            }

            // Проверка наличия и установка параметров
            if (doc["fillPumpMaxRuntimeMs"].is<unsigned long>())
                CurrentState.setupData.fillPumpMaxRuntimeMs = doc["fillPumpMaxRuntimeMs"].as<unsigned long>() * 1000UL;

            if (doc["fillPumpMinOffTime"].is<unsigned long>())
                CurrentState.setupData.fillPumpMinOffTime = doc["fillPumpMinOffTime"].as<unsigned long>() * 1000UL;

            if (doc["fullStableTimeMs"].is<unsigned long>())
                CurrentState.setupData.fullStableTimeMs = doc["fullStableTimeMs"].as<unsigned long>() * 1000UL;

            // Сохраняем в NVS
            saveSetupDataToNVS();

            // Применяем параметры к таймерам
            CurrentState.applySetupDataToTimers();

            request->send(200, "text/plain", "Setup updated");
        });

    server.addHandler(setupHandler);

#pragma endregion POST setup

#pragma region /OTA dopnt work
    // OTA endpoint
    server.on(
        "/update",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            bool ok = !Update.hasError();
            request->send(200, "text/plain", ok ? "OK" : "FAIL");
            if (ok)
            {
                delay(100);
                ESP.restart();
            }
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            if (!index)
            {
                Update.begin(UPDATE_SIZE_UNKNOWN);
            }
            Update.write(data, len);
            if (final)
            {
                Update.end(true);
            }
        });
#pragma endregion / OTA dopnt work

    AsyncOTA.begin(&server); // Start ElegantOTA
    // ElegantOTA callbacks
    AsyncOTA.onOTAStart(onOTAStart);
    AsyncOTA.onOTAProgress(onOTAProgress);
    AsyncOTA.onOTAEnd(onOTAEnd);

    server.onNotFound(onNotFound);
    // server.on("/", handleRoot);
    // server.on("/manual", handleManualControl);
    server.begin();
}
