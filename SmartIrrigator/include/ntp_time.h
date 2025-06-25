#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <Arduino.h>
#include <RTClib.h>

/*
Действующие лица
Системное время     - Время операционной системы. При включении откуда то должно загрузиться.
NTP                 - Приходит только при наличии интернета
RTC                 - DS3231 если есть, то есть. Могла сесть батарейка, тогда нет. Или его вообще не припаяли.

Ситуация	                Источник времени                                    Действия
Включили, пока ничего нет   DS3231 → Системное время                            Прочитать и если удалось, установить ситемное время.
Есть WiFi и NTP             NTP → DS3231 → Системное время                      Обновлять RTC из NTP
Нет WiFi, но RTC работает	DS3231 → Системное время                            Использовать RTC, сохранять время в NVS
Нет WiFi и RTC              Системное время (с дрейфом)                         Использовать последнее сохранённое время


📌 Status Register (0x0F) DS3231
Бит	Название	Описание
7	OSF         Oscillator Stop Flag — флаг остановки генератора (1 = часы остановились)
6	EN32kHz     Включение вывода 32kHz (1 = включён)
5	BSY         Флаг занятости (1 = идёт преобразование температуры)
4	A2F         Флаг срабатывания будильника 2 (1 = будильник сработал)
3	A1F         Флаг срабатывания будильника 1 (1 = будильник сработал)
2-0	-           Не используются (обычно 0)
*/

void initTime(); // Инициализация NTP и настройка часового пояса

uint16_t getMinutesSinceMidnight(tm &timeInfo); // Время в минутах с полуночи

/// @bri Признак наличия времени с NTP
/// @return 
bool sntpReady();

/// @brief  Флаг наличия RTC
/// @return true если инициализация была успешна
bool rtcHasRTC();

/// @brief Флаг сбоя RTC по питанию, проще говоря времени нет!
/// @return 
bool rtcLostPower();

/// @brief Признак наличия времени RTC!
/// @return 
bool rtcHasTime();

// Обновление RTC из NTP. Только если всё удачно есть
bool rtcAdjustFromSntp();

bool getRTCTime(struct tm &timeinfo);

float rtcGetTemperature();

// Явное задание RTC
bool rtcAdjust(DateTime &dt);

void  syncTimeFromNTP();

void syncSystemTimeFromRTC();

#endif
