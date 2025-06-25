#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <Arduino.h>

void initTime(); // Инициализация NTP и настройка часового пояса

uint16_t getMinutesSinceMidnight(tm &timeInfo); // Время в минутах с полуночи

/// @bri Признак наличия времени с NTP
/// @return 
bool sntpReady();

/// @brief  Флаг наличия RTC
/// @return true если инициализация была успешна
bool rtcHasRTC();

/// @brief Флаг сбоя RTC по питанию
/// @return 
bool rtcLostPower();

// Обновление RTC из NTP. Только если всё удачно есть
bool rtcAdjustFromSntp();

// Явное задание RTC
bool rtcAdjust(DateTime &dt);

#endif
