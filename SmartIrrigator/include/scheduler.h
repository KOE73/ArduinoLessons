#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

struct ScheduleEntry {
    uint8_t valve;        // Номер клапана: 1–3
    uint16_t startTime;   // Время включения в формате "минут с полуночи" (0–1439)
    uint16_t endTime;     // Время выключения в том же формате
};

void initScheduler();
void loadSchedules();
void saveSchedules();
void checkSchedules(uint16_t nowMinutes); // Проверка расписания в текущий момент

bool isValveScheduled(uint8_t valve);
bool getScheduleState(uint8_t valve);     // true — клапан должен быть открыт

void setManualIrrigation(bool state);
bool isManualIrrigationActive();

#endif
