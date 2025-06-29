#ifndef DATA_H
#define DATA_H

#include <Arduino.h>
#include <algorithm>
#include <functional> // std::identity
#include "config.h"
#include "lad.h"

void loadSchedulesFromNVS();
void cleanExtraSchedules();
void saveScheduleToNVS(int index);

void loadSetupDataFromNVS();
void saveSetupDataToNVS();

// Элемент расписания
// Что включить
struct ScheduleEntry
{
    bool enabled;             // Признак работы расписания
    bool valves[VALVE_COUNT]; // Клапана
    bool irrigationPump;      // Насос полива
    bool fillPump;            // Насос заполнения
    uint16_t startTime;       // Время включения в формате "минут с полуночи" (0–1439)
    uint16_t endTime;         // Время выключения в том же формате

    bool isTimeInRange(uint16_t now)
    {
        if (startTime <= endTime)
            return now >= startTime && now < endTime;
        else
            return now >= startTime || now < endTime; // через полночь
    }
};

struct SetupData
{
    unsigned long fillPumpMaxRuntimeMs; // Максимальное время работы насоса
    unsigned long fillPumpMinOffTime;   // Минимальная пауза между включениями насоса
    unsigned long fullStableTimeMs;     // Вркмя стабилизации датчика уровня
};

struct StateData
{

#pragma region Time
    bool hasNTP;       // Признак наличия синхронизации по NTP
    bool hasRTC;       // Признак наличия RTC
    bool hasTimeRTC;   // Признак наличия времени в RTC
    bool rtcLostPower; //
    bool hasTime;      // Время известно. Т.е. соеденились с NTP. Без этого все остальное не имет смысла

    time_t now;                    // Текущее время в минутах
    tm nowTimeInfo;                // Время в разделенном формате
    tm rtcTimeInfo;                // Время напрямую из RTC в разделенном формате
    uint16_t minutesSinceMidnight; // Минут с полуночи
    time_t in_TimeManualOff;       // Во сколько отключить все ручные команды.
                                   // При задании любого значения задается +5 минут от текущего.
                                   // Есть кнопки для уменьшения и увеличения по 5 минут
    tm in_TimeInfoManualOff;       // Во сколько отключить все ручные команды. в разделенном формате
#pragma endregion Time

    float rtcTemperature; // Температура из DS3231

    SetupData setupData;

#pragma region LogicTimers

    TimerTON timer_IsFull_Confirm;    // Подтверждение стабильного уровня
    TimerTON timer_Fill_RestartBlock; // Блокировка повторного запуска
    TimerTON timer_Fill_MaxRuntime;   // Ограничение по времени работы

#pragma endregion LogicTimers

    // unsigned long out_FillPumpOn_LastOffTime = 0; // Задержка включения насоса после срабатывания датчика
    bool in_IsFull_Confirmed = false; // Настоящее подтверждённое "полнО"
    // unsigned long in_IsFull_LastStableTime = 0;   // Когда "полнО" стало стабильным
    // unsigned long in_IsFull_LastChangeTime = 0;   // Когда датчик изменил состояние
    // bool in_IsFull_Prev;                          // Датчик наполнения бочки из предыдущего цикла

    // bool out_FillPumpOn_CanStart;

#pragma region Schedule
    ScheduleEntry shedules[MAX_SCHEDULES]; // Расписания
#pragma endregion Schedule

#pragma region IO

    bool in_IsFull;                       // Датчик наполнения бочки
    bool in_IsManualFill;                 // Ручное управление насосом наполнения
    bool in_IsManualIrrigation;           // Ручное управление насосом полива
    bool in_IsManualValveOn[VALVE_COUNT]; // Клапана вручную каждый

    bool out_FillPumpOn;           // Насос для наполнения бочки
    bool out_IrrigationPumpOn;     // Насос для полива
    bool out_ValveOn[VALVE_COUNT]; // Клапана

#pragma endregion IO

#pragma region Methods

    void applySetupDataToTimers()
    {
        timer_Fill_MaxRuntime.PT_presetTime = setupData.fillPumpMaxRuntimeMs;
        timer_Fill_RestartBlock.PT_presetTime = setupData.fillPumpMinOffTime;
        timer_IsFull_Confirm.PT_presetTime = setupData.fullStableTimeMs;
    }

    void allManualsOff()
    {
        in_IsManualFill = false;
        in_IsManualIrrigation = false;
        std::fill(std::begin(in_IsManualValveOn), std::end(in_IsManualValveOn), false);
        setTimeManualOff(0);
    }
    bool anyManuals()
    {
        return in_IsManualFill ||
               in_IsManualIrrigation ||
               std::any_of(
                   std::begin(in_IsManualValveOn),
                   std::end(in_IsManualValveOn),
                   [](bool v)
                   { return v; } // true, если элемент true
               );
    }
    void setTimeManualOff(const time_t time)
    {
        in_TimeManualOff = time;
        localtime_r(&time, &in_TimeInfoManualOff);
    }

    void adjustManualOffTime(int minutes)
    {
        // Если время ещё не установлено — стартуем с текущего времени + 5 минут
        if (in_TimeManualOff == 0)
        {
            setTimeManualOff(now + minutes * 60);
        }
        else
        {
            setTimeManualOff(in_TimeManualOff + minutes * 60);

            // Не позволяем устанавливать время в прошлом
            if (in_TimeManualOff < now)
            {
                setTimeManualOff(now);
            }
        }
    }
    void ensureManualOffTimeoutSet(bool on)
    {
        if (on)
        {
            if (in_TimeManualOff == 0 && now > 0)
            {
                setTimeManualOff(now + 5 * 60);
            }
        }
        else
        {
            if (!anyManuals())
                setTimeManualOff(0);
        }
    }
    void setManualIrrigation(bool on)
    {
        ensureManualOffTimeoutSet(in_IsManualIrrigation = on);
    }

    void setManualFill(bool on)
    {
        ensureManualOffTimeoutSet(in_IsManualFill = on);
    }

    void setManualValve(int index, bool on)
    {
        if (index < 0 || index >= VALVE_COUNT)
            return;
        ensureManualOffTimeoutSet(in_IsManualValveOn[index] = on);
    }

    void allValvesOff() { std::fill(std::begin(out_ValveOn), std::end(out_ValveOn), false); }

    bool anyValveOn()
    {
        return std::any_of(
            std::begin(out_ValveOn),
            std::end(out_ValveOn),
            [](bool v)
            { return v; } // true, если элемент true
        );
    }
#pragma endregion Methods
};

extern StateData CurrentState;
extern StateData WorkState;

#endif
