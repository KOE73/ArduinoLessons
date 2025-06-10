#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "esp_log.h"

#include "config.h"
#include "data.h"
#include "webserver_me.h"
#include "ntp_time.h"
#include "eeprom_store.h"

// Load data to LittleFS
// pio run --target uploadfs

static const char *TAG = "PUMP"; // тег для модуля

StateData CurrentState;

unsigned long lastFillPumpOffTime = 0;
bool fillPumpState = false;

void setupPins()
{
    pinMode(PIN_FILL_PUMP, OUTPUT);
    pinMode(PIN_IRR_PUMP, OUTPUT);
    pinMode(PIN_VALVE_1, OUTPUT);
    pinMode(PIN_VALVE_2, OUTPUT);
    pinMode(PIN_VALVE_3, OUTPUT);
    pinMode(PIN_FLOAT_SENSOR, INPUT);
}

void stateIn(StateData &state)
{
    state.in_IsFull = digitalRead(PIN_FLOAT_SENSOR);
}

void stateOut(StateData &state)
{
    digitalWrite(PIN_FILL_PUMP, state.out_FillPumpOn);
    digitalWrite(PIN_IRR_PUMP, state.out_IrrigationPumpOn);
    digitalWrite(PIN_VALVE_1, state.valveStateOn[0]);
    digitalWrite(PIN_VALVE_2, state.valveStateOn[1]);
    digitalWrite(PIN_VALVE_3, state.valveStateOn[2]);
}

// void controlFillPump()
//{
//     bool floatLow = digitalRead(PIN_FLOAT_SENSOR) == LOW;
//     bool anyValveActive = isValveScheduled(1) || isValveScheduled(2) || isValveScheduled(3);
//     bool canStart = (millis() - lastFillPumpOffTime) > FILL_PUMP_MIN_OFF_TIME * 1000;
//
//     if (floatLow && !anyValveActive && canStart)
//     {
//         digitalWrite(PIN_FILL_PUMP, HIGH);
//         fillPumpState = true;
//     }
//     else
//     {
//         if (fillPumpState)
//         {
//             fillPumpState = false;
//             lastFillPumpOffTime = millis();
//             saveLastFillPumpOffTime(lastFillPumpOffTime);
//         }
//         digitalWrite(PIN_FILL_PUMP, LOW);
//     }
// }

// void controlValvesAndIrrigationPump()
//{
//     bool pumpNeeded = false;
//
//     for (uint8_t i = 1; i <= 3; i++)
//     {
//         bool valveOn = getScheduleState(i);
//         digitalWrite(PIN_VALVE_1 + i - 1, valveOn ? HIGH : LOW);
//         if (valveOn)
//             pumpNeeded = true;
//     }
//
//     // Учитываем ручной режим
//     if (isManualIrrigationActive())
//         pumpNeeded = true;
//
//     digitalWrite(PIN_IRR_PUMP, pumpNeeded ? HIGH : LOW);
// }

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    ESP_LOGI(TAG, "📂 Содержимое каталога: %s", dirname);

    File root = fs.open(dirname);
    if (!root || !root.isDirectory())
    {
        ESP_LOGW(TAG, "❌ Не удалось открыть каталог: %s", dirname);
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            ESP_LOGI(TAG, "📁 [DIR]  %s", file.name());
            if (levels > 0)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            ESP_LOGI(TAG, "📄 [FILE] %s  (%d байт)", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void setup()
{
    // pinMode(LED_BUILTIN, OUTPUT);
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW);
    delay(200);
    digitalWrite(8, HIGH);

    Serial.begin(921600);
    Serial.println("BOOT"); // должно сразу показать что-то
    printf("SETUP x\n");

    // esp_log_level_set("*", ESP_LOG_INFO);  // установить глобально
    ESP_LOGI(TAG, "SETUP");

    setupPins();
    EEPROM.begin(EEPROM_SIZE);

    if (!LittleFS.begin(true, "/littlefs", 10, "littlefs"))
    {
        ESP_LOGE(TAG, "LittleFS монтирование не удалось");
        return;
    }

    listDir(LittleFS, "/", 1);

    File file = LittleFS.open("/index.html", "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Файл не найден");
        return;
    }

    // ESP_LOGI(TAG, "Содержимое index.html");
    // while (file.available())
    //{
    //     File file = LittleFS.open("/index.html", "r");
    //
    //    if (file)
    //    {
    //        uint8_t buf[64];
    //        size_t bytesRead = file.read(buf, sizeof(buf));
    //        ESP_LOGI(TAG, "Прочитано %u байт\n", bytesRead);
    //
    //        for (size_t i = 0; i < bytesRead; ++i)
    //        {
    //            //ESP_LOGI(TAG, buf[i]); // выводим содержимое
    //        }
    //
    //        file.close();
    //    }
    //}

    setupWiFi();
    setupWebServer();
    initTime();
}

void loop()
{
    auto ms = millis();
    if (ms % 1000 < 500)
        digitalWrite(8, LOW); // turn the LED off by making the voltage LOW
    else
        digitalWrite(8, HIGH); // turn the LED on (HIGH is the voltage level)

    if (ms % 1000 == 500)
        ESP_LOGI(TAG, "Connected! IP: %s", WiFi.localIP().toString());

    uint16_t nowMinutes = getMinutesSinceMidnight();

    // Считываем состояния входов
    stateIn(CurrentState);

    CurrentState.hasTime = getHasTime();

    // Обнуление выходов, последующая логика, если надо включит обратно
    CurrentState.allValvesOff();

    // Если времени нет, то всё выключаем и пропускаем основную логику
    if (!CurrentState.hasTime)
    {
        CurrentState.out_FillPumpOn = false;
        CurrentState.out_IrrigationPumpOn = false;
        goto endMainLogik;
    }

    // Получаем время
    getLocalTime(&CurrentState.timeInfo, 0);
    CurrentState.minutesSinceMidnight = getMinutesSinceMidnight();

    // Обрабатываем расписание
    for (auto &shedule : CurrentState.shedules)
    {
        if (shedule.isTimeInRange(CurrentState.minutesSinceMidnight))
        {
            std::copy(
                std::begin(shedule.valves),
                std::end(shedule.valves),
                std::begin(CurrentState.valveStateOn));
            CurrentState.out_IrrigationPumpOn = shedule.irrigationPump;
            CurrentState.out_FillPumpOn = shedule.fillPump;
        }
    }

endMainLogik:

    // Если полив, наполнение нельзя
    if (CurrentState.out_IrrigationPumpOn || CurrentState.anyValveOn())
        CurrentState.out_FillPumpOn = false;

    // Если сработал датчик, вырубаем наполнение
    if (CurrentState.in_IsFull)
        CurrentState.out_FillPumpOn = false;

endLogik:
    // Выводим состояние выходов
    stateOut(CurrentState);

    // checkSchedules(nowMinutes);
    // controlValvesAndIrrigationPump();
    // controlFillPump();

    handleWebRequests();

    // delay(1); // Основной цикл раз в 1 сек
}
