#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <WiFi.h>

#include <Wire.h>

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "config.h"
#include "data.h"
#include "webserver_me.h"
#include "ntp_time.h"

// Load data to LittleFS
// pio run --target uploadfs

static const char *TAG = "PUMP"; // тег для модуля

StateData CurrentState = {};
// Чтобы вебка не моргала
StateData WorkState;

void setupPins()
{
    pinMode(PIN_FILL_PUMP, OUTPUT);
    pinMode(PIN_IRR_PUMP, OUTPUT);
    pinMode(PIN_VALVE_1, OUTPUT);
    pinMode(PIN_VALVE_2, OUTPUT);
    pinMode(PIN_VALVE_3, OUTPUT);
    pinMode(PIN_FLOAT_SENSOR, INPUT);

    Wire.begin(PIN_SDA, PIN_SCL); // Инициализация I2C с указанием пинов SDA (20), SCL (21)
}

void stateIn(StateData &state)
{
    state.in_IsFull = digitalRead(PIN_FLOAT_SENSOR);
}

void stateOut(StateData &state)
{
    digitalWrite(PIN_FILL_PUMP, state.out_FillPumpOn);
    digitalWrite(PIN_IRR_PUMP, state.out_IrrigationPumpOn);
    digitalWrite(PIN_VALVE_1, state.out_ValveOn[0]);
    digitalWrite(PIN_VALVE_2, state.out_ValveOn[1]);
    digitalWrite(PIN_VALVE_3, state.out_ValveOn[2]);
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

void listNVS()
{
    esp_err_t err = nvs_flash_init(); // Можно убрать, если уже инициализирован
    if (err != ESP_OK && err != ESP_ERR_NVS_NO_FREE_PAGES && err != ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE(TAG, "Ошибка инициализации NVS: %s", esp_err_to_name(err));
        return;
    }

    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
    if (it == nullptr)
    {
        ESP_LOGI(TAG, "NVS пуст или не содержит данных.");
        return;
    }

    ESP_LOGI(TAG, "Содержимое NVS:");
    while (it != nullptr)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        const char *typeStr = "UNKNOWN";
        switch (info.type)
        {
        case NVS_TYPE_U8:
            typeStr = "U8";
            break;
        case NVS_TYPE_I8:
            typeStr = "I8";
            break;
        case NVS_TYPE_U16:
            typeStr = "U16";
            break;
        case NVS_TYPE_I16:
            typeStr = "I16";
            break;
        case NVS_TYPE_U32:
            typeStr = "U32";
            break;
        case NVS_TYPE_I32:
            typeStr = "I32";
            break;
        case NVS_TYPE_U64:
            typeStr = "U64";
            break;
        case NVS_TYPE_I64:
            typeStr = "I64";
            break;
        case NVS_TYPE_STR:
            typeStr = "STRING";
            break;
        case NVS_TYPE_BLOB:
            typeStr = "BLOB";
            break;
        }

        ESP_LOGI(TAG, "Namespace: %-15s Key: %-15s Type: %s", info.namespace_name, info.key, typeStr);

        it = nvs_entry_next(it);
    }

    ESP_LOGI(TAG, "Конец списка.");
}

void listNVS2()
{
    esp_err_t err = nvs_flash_init(); // Можно убрать, если уже инициализирован
    if (err != ESP_OK && err != ESP_ERR_NVS_NO_FREE_PAGES && err != ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE(TAG, "Ошибка инициализации NVS: %s", esp_err_to_name(err));
        return;
    }

    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
    if (it == nullptr)
    {
        ESP_LOGI(TAG, "NVS пуст или не содержит данных.");
        return;
    }

    ESP_LOGI(TAG, "Содержимое NVS:");

    while (it != nullptr)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        const char *typeStr = "UNKNOWN";
        String valueStr = "?";

        // Открываем namespace для чтения
        nvs_handle_t handle;
        if (nvs_open(info.namespace_name, NVS_READONLY, &handle) != ESP_OK)
        {
            ESP_LOGW(TAG, "Не удалось открыть namespace %s", info.namespace_name);
            it = nvs_entry_next(it);
            continue;
        }

        // Пытаемся считать значение
        if (info.type == NVS_TYPE_I32)
        {
            int32_t val = 0;
            if (nvs_get_i32(handle, info.key, &val) == ESP_OK)
                valueStr = String(val);
            typeStr = "I32";
        }
        else if (info.type == NVS_TYPE_U32)
        {
            uint32_t val = 0;
            if (nvs_get_u32(handle, info.key, &val) == ESP_OK)
                valueStr = String(val);
            typeStr = "U32";
        }
        else if (info.type == NVS_TYPE_STR)
        {
            size_t len = 0;
            if (nvs_get_str(handle, info.key, NULL, &len) == ESP_OK && len > 0)
            {
                char *buf = (char *)malloc(len);
                if (nvs_get_str(handle, info.key, buf, &len) == ESP_OK)
                    valueStr = String(buf);
                free(buf);
            }
            typeStr = "STR";
        }
        else if (info.type == NVS_TYPE_U8)
        {
            uint8_t val = 0;
            if (nvs_get_u8(handle, info.key, &val) == ESP_OK)
                valueStr = String(val);
            typeStr = "U8";
        }
        else if (info.type == NVS_TYPE_BLOB)
        {
            size_t len = 0;
            if (nvs_get_blob(handle, info.key, NULL, &len) == ESP_OK && len > 0)
            {
                valueStr = "<BLOB, " + String(len) + " байт>";
            }
            typeStr = "BLOB";
        }

        ESP_LOGI(TAG, "Namespace: %-12s  Key: %-16s  Type: %-6s  Value: %s",
                 info.namespace_name, info.key, typeStr, valueStr.c_str());

        nvs_close(handle);
        it = nvs_entry_next(it);
    }

    ESP_LOGI(TAG, "Конец списка.");
}

void setup()
{
    // pinMode(LED_BUILTIN, OUTPUT);
    pinMode(8, OUTPUT);
    digitalWrite(8, LOW);
    delay(2000);
    digitalWrite(8, HIGH);

    Serial.begin(921600);
    Serial.println("BOOT"); // должно сразу показать что-то
    printf("SETUP x\n");

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

    setupWiFi();
    setupWebServer();
    initTime();

    //    // ---- Настройка CurrentState
    //    // Если это первый запуск, устанавливаем безопасное значение, чтобы насос можно было включать
    //    if (CurrentState.out_FillPumpOn_LastOffTime == 0)
    //    {
    //        CurrentState.out_FillPumpOn_LastOffTime = millis() - FILL_PUMP_MIN_OFF_TIME_MS - 1;
    //    }

    // 🕒 Инициализация таймеров
    CurrentState.timer_Fill_RestartBlock.PT_presetTime = FILL_PUMP_MIN_OFF_TIME_MS;
    CurrentState.timer_Fill_MaxRuntime.PT_presetTime = FILL_PUMP_MAX_RUNTIME_MS;
    CurrentState.timer_IsFull_Confirm.PT_presetTime = FULL_STABLE_TIME_MS;

    loadSchedulesFromNVS();
    loadSetupDataFromNVS();
    CurrentState.applySetupDataToTimers();

    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
    // setenv("TZ", "Asia/Yekaterinburg", 1);  // ВАЖНО: минус, потому что в TZ правилах обратное направление
    setenv("TZ", "YEKT-5", 1); // ВАЖНО: минус, потому что в TZ правилах обратное направление
    tzset();

    // listNVS2();
}

void loop()
{
    auto nowMilis = millis();
    if (nowMilis % 1000 < 500)
        digitalWrite(8, LOW); // turn the LED off by making the voltage LOW
    else
        digitalWrite(8, HIGH); // turn the LED on (HIGH is the voltage level)

    if (nowMilis % 5000 == 0)
        ESP_LOGI(TAG, "Connected! IP: %s", WiFi.localIP().toString().c_str());

#pragma region Input
    // Считываем состояния входов
    stateIn(CurrentState);
#pragma endregion Input

    // Обнуление выходов, последующая логика, если надо включит обратно
    CurrentState.out_FillPumpOn = false;
    CurrentState.out_IrrigationPumpOn = false;
    CurrentState.allValvesOff();

#pragma region Время
    // Получаем время, бывает что не получается ! :-/
    tm timeInfo;
    CurrentState.hasTime = getLocalTime(&timeInfo, 100);

    CurrentState.hasNTP = sntpReady();
    CurrentState.hasRTC = rtcHasRTC();
    CurrentState.hasTimeRTC = rtcHasTime();
    CurrentState.rtcLostPower = rtcLostPower();
    CurrentState.rtcTemperature = rtcGetTemperature();
    getRTCTime(CurrentState.rtcTimeInfo);

    // Если времени нет, то всё выключаем и пропускаем основную логику
    if (!CurrentState.hasTime)
        goto endMainLogik;

    CurrentState.now = mktime(&timeInfo);
    CurrentState.nowTimeInfo = timeInfo;
    CurrentState.minutesSinceMidnight = getMinutesSinceMidnight(timeInfo);

#pragma endregion

#pragma region Ручное управление
Manual:
    // Если установленно время окончания ручного управления, то что-то хозяин включил, не будем ему мешать
    if (CurrentState.in_TimeManualOff != 0)
    {
        // Если ручное выключено или
        // Проверяем время и Сбрасываем все ручные состояния если время подошло
        if (!CurrentState.anyManuals() ||CurrentState.now >= CurrentState.in_TimeManualOff)
            CurrentState.allManualsOff();

        CurrentState.out_FillPumpOn = CurrentState.in_IsManualFill;
        CurrentState.out_IrrigationPumpOn = CurrentState.in_IsManualIrrigation;
        std::copy(std::begin(CurrentState.in_IsManualValveOn), std::end(CurrentState.in_IsManualValveOn), std::begin(CurrentState.out_ValveOn));

        // Пропускам расписание ибо нефмг поперек батьки
        goto endMainLogik;
    }
#pragma endregion

#pragma region Расписание
Shedule:
    // Обрабатываем расписание
    for (auto &shedule : CurrentState.shedules)
    {
        if (shedule.isTimeInRange(CurrentState.minutesSinceMidnight))
        {
            std::copy(
                std::begin(shedule.valves),
                std::end(shedule.valves),
                std::begin(CurrentState.out_ValveOn));
            CurrentState.out_IrrigationPumpOn = shedule.irrigationPump;
            CurrentState.out_FillPumpOn = shedule.fillPump;
        }
    }
#pragma endregion

endMainLogik:

    // Аля LAD
    // 1. Если идёт полив или открыты клапаны — наполнение запрещено
    if (CurrentState.out_IrrigationPumpOn || CurrentState.anyValveOn())
    {
        CurrentState.out_FillPumpOn = false;
    }

    // 2. Подтверждение стабильного уровня поплавка через таймер
    CurrentState.timer_IsFull_Confirm.update(CurrentState.in_IsFull, nowMilis);
    bool prevConfirmed = CurrentState.in_IsFull_Confirmed;
    CurrentState.in_IsFull_Confirmed = CurrentState.timer_IsFull_Confirm.Q_output;

    // 6. Ограничение по максимальному времени работы
    CurrentState.timer_Fill_MaxRuntime.update(CurrentState.out_FillPumpOn, nowMilis);

    // 3. Если уровень подтвердился или вышло время работы — сброс ручного режима // ???, запуск блокировки
    if ((CurrentState.in_IsFull_Confirmed) || CurrentState.timer_Fill_MaxRuntime.Q_output)
    {
        CurrentState.in_IsManualFill = false;
        CurrentState.out_FillPumpOn = false;

        // **** CurrentState.timer_Fill_RestartBlock.update(true, nowMilis);
        ESP_LOGI("LEVEL", "Бочка полная %s, таймер %s",
                 BOOL_STR(CurrentState.in_IsFull_Confirmed),
                 BOOL_STR(CurrentState.timer_Fill_MaxRuntime.Q_output));
    }

    // 4. При падении уровня — лог
    if (!CurrentState.in_IsFull_Confirmed && prevConfirmed)
    {
        ESP_LOGI("LEVEL", "⬇️ Уровень упал — сброс подтверждения");
    }

    //// 5. Блокировка повторного запуска
    // CurrentState.timer_Fill_RestartBlock.update(!CurrentState.out_FillPumpOn, nowMilis);
    // if (CurrentState.timer_Fill_RestartBlock.Q_output)
    //{
    //     CurrentState.out_FillPumpOn = false;
    // }

endLogik:
    // Выводим состояние выходов
    stateOut(CurrentState);

    // Полная копия для вебки, а то накладки случаются
    WorkState = CurrentState;
}
