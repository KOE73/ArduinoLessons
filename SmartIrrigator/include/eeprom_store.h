#ifndef EEPROM_STORE_H
#define EEPROM_STORE_H

#include <Arduino.h>

void saveLastFillPumpOffTime(uint32_t timestamp);
uint32_t getLastFillPumpOffTime();

#endif
