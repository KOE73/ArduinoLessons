#include <Preferences.h>
#include "data.h"

Preferences prefs;

void loadSchedulesFromNVS()
{
    prefs.begin("SmartIrrigator", true); // true = только чтение

    for (int i = 0; i < MAX_SCHEDULES; i++)
    {
        String key = "shedule." + String(i);

        if (prefs.isKey(key.c_str()))
        {
            ScheduleEntry entry;
            size_t read = prefs.getBytes(key.c_str(), &entry, sizeof(ScheduleEntry));

            if (read == sizeof(ScheduleEntry))
            {
                CurrentState.shedules[i] = entry;
                ESP_LOGI("LOAD", "Загружено расписание %s (%d байт)", key, read);
            }
            else
            {
                ESP_LOGW("LOAD", "Размер данных не совпадает для %s", key);
            }
        }
        else
        {
            ESP_LOGI("LOAD", "Ключ %s не найден", key);
        }
    }

    prefs.end();

    cleanExtraSchedules();
}

void cleanExtraSchedules()
{
    prefs.begin("SmartIrrigator", false); // false = режим записи

    // запасом до 100
    for (int i = MAX_SCHEDULES; i < 100; i++)
    {
        String key = "shedule." + String(i);

        if (prefs.isKey(key.c_str()))
        {
            prefs.remove(key.c_str());
            ESP_LOGI("CLEAN", "Удалён лишний ключ: %s", key);
        }
    }

    prefs.end();
}

void saveScheduleToNVS(int index)
{
    if (index < 0 || index >= MAX_SCHEDULES)
    {
        ESP_LOGW("SAVE", "Индекс вне диапазона: %d", index);
        return;
    }

    Preferences prefs;
    prefs.begin("SmartIrrigator", false); // false = режим записи

    String key = "shedule." + String(index);
    size_t written = prefs.putBytes(key.c_str(), &CurrentState.shedules[index], sizeof(ScheduleEntry));

    prefs.end();

    if (written == sizeof(ScheduleEntry))
    {
        ESP_LOGI("SAVE", "Расписание %d успешно сохранено (%d байт)", index, written);
    }
    else
    {
        ESP_LOGE("SAVE", "Ошибка сохранения расписания %d (записано %d байт)", index, written);
    }
}


void loadSetupDataFromNVS()
{
    Preferences prefs;
    prefs.begin("SmartIrrigator", true); // true = только чтение

    CurrentState.setupData.fillPumpMaxRuntimeMs = prefs.getULong("fillMaxRun", FILL_PUMP_MAX_RUNTIME_MS);
    CurrentState.setupData.fillPumpMinOffTime   = prefs.getULong("fillMinStop", FILL_PUMP_MIN_OFF_TIME_MS);
    CurrentState.setupData.fullStableTimeMs     = prefs.getULong("fullOkDelay", FULL_STABLE_TIME_MS);

    prefs.end();

    ESP_LOGI("SETUP", "Загружено из NVS: max=%lu, minOff=%lu, stable=%lu",
        CurrentState.setupData.fillPumpMaxRuntimeMs,
        CurrentState.setupData.fillPumpMinOffTime,
        CurrentState.setupData.fullStableTimeMs);
}

void saveSetupDataToNVS()
{
    Preferences prefs;
    prefs.begin("SmartIrrigator", false); // false = запись

    prefs.putULong("fillMaxRun", CurrentState.setupData.fillPumpMaxRuntimeMs);
    prefs.putULong("fillMinStop", CurrentState.setupData.fillPumpMinOffTime);
    prefs.putULong("fullOkDelay", CurrentState.setupData.fullStableTimeMs);

    prefs.end();

    ESP_LOGI("SETUP", "Сохранено в NVS");
}
