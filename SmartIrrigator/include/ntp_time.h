#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <Arduino.h> 

void initTime();                  // Инициализация NTP и настройка часового пояса
uint16_t getMinutesSinceMidnight();  // Текущее время в минутах с полуночи

#endif
