#include "eeprom_store.h"

#include <EEPROM.h>

#include <config.h>

#define FILL_PUMP_TIMESTAMP_ADDR 400  // Начальная позиция хранения времени

void saveLastFillPumpOffTime(uint32_t timestamp) {
    EEPROM.put(FILL_PUMP_TIMESTAMP_ADDR, timestamp);
    EEPROM.commit();
}

uint32_t getLastFillPumpOffTime() {
    uint32_t ts = 0;
    EEPROM.get(FILL_PUMP_TIMESTAMP_ADDR, ts);
    return ts;
}

