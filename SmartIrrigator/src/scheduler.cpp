#include "scheduler.h"
#include "config.h"
#include <EEPROM.h>

static ScheduleEntry schedules[MAX_SCHEDULES];
static bool valveStates[3] = {false, false, false};
static bool manualIrrigation = false;

void initScheduler() {
    EEPROM.begin(EEPROM_SIZE);
    loadSchedules();
}

void loadSchedules() {
    EEPROM.get(0, schedules);
}

void saveSchedules() {
    EEPROM.put(0, schedules);
    EEPROM.commit();
}

bool isTimeInRange(uint16_t now, uint16_t start, uint16_t end) {
    if (start <= end) {
        return now >= start && now < end;
    } else {
        return now >= start || now < end; // через полночь
    }
}

void checkSchedules(uint16_t nowMinutes) {
    for (int i = 0; i < 3; i++) {
        valveStates[i] = false;
    }

    for (int i = 0; i < MAX_SCHEDULES; i++) {
        if (schedules[i].valve < 1 || schedules[i].valve > 3) continue;

        if (isTimeInRange(nowMinutes, schedules[i].startTime, schedules[i].endTime)) {
            valveStates[schedules[i].valve - 1] = true;
        }
    }
}

bool isValveScheduled(uint8_t valve) {
    if (valve < 1 || valve > 3) return false;
    return valveStates[valve - 1];
}

bool getScheduleState(uint8_t valve) {
    if (valve < 1 || valve > 3) return false;
    return valveStates[valve - 1];
}

void setManualIrrigation(bool state) {
    manualIrrigation = state;
}

bool isManualIrrigationActive() {
    return manualIrrigation;
}
