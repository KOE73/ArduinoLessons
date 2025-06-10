#ifndef DATA_H
#define DATA_H

#include <Arduino.h>
#include <algorithm>
#include <functional> // std::identity
#include "config.h"

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

struct StateData
{
    ScheduleEntry shedules[MAX_SCHEDULES];

    bool hasTime;                  // Время известно. Т.е. соеденились с NTP
    tm timeInfo;                    // Время
    uint16_t minutesSinceMidnight; // Минут с полуночи

    bool in_IsFull;             // Датчик наполнения бочки
    bool in_IsManualFill;       // Ручное управление насосом наполнения
    bool in_IsManualIrrigation; // Ручное управление насосом полива

    bool out_FillPumpOn;            // Насос для наполнения бочки
    bool out_IrrigationPumpOn;      // Насос для полива
    bool valveStateOn[VALVE_COUNT]; // Клапана

    void allValvesOff() { std::fill(std::begin(valveStateOn), std::end(valveStateOn), false); }

    bool anyValveOn()
    {
        return std::any_of(
            std::begin(valveStateOn),
            std::end(valveStateOn),
            [](bool v)
            { return v; } // true, если элемент true
            // std::identity{}  // C++20, если доступно
        );
    }
};

extern StateData CurrentState;



//void initScheduler() {
//    EEPROM.begin(EEPROM_SIZE);
//    loadSchedules();
//}
//
//void loadSchedules() {
//    EEPROM.get(0, schedules);
//}
//
//void saveSchedules() {
//    EEPROM.put(0, schedules);
//    EEPROM.commit();
//}


#endif
