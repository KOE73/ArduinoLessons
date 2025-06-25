#include "ntp_time.h"
#include <time.h>
#include <esp_sntp.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
bool flagHasRTC;

#define TIME_OFFSET 5

static const char *TAG = "TIMEs";

bool ntpSynced = false;

void time_sync_notification_cb(struct timeval *tv)
{
    ntpSynced = true;
    ESP_LOGI(TAG, "Notification: Time synced with NTP server!");
}

void initTime()
{
    // RTC init
    flagHasRTC = rtc.begin();

    if (!rtcHasRTC)
        ESP_LOGI(TAG, "Couldn't find RTC");

    if (!rtc.lostPower())
    {
        syncSystemTimeFromRTC();
        // Установите время компиляции
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // sntp init
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    // configTime(3600 * TIME_OFFSET, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
}

uint16_t getMinutesSinceMidnight(tm &timeInfo)
{
    return timeInfo.tm_hour * 60 + timeInfo.tm_min;
}

void checkTimeSync()
{
    sntp_sync_status_t syncStatus = sntp_get_sync_status();

    switch (syncStatus)
    {
    case SNTP_SYNC_STATUS_COMPLETED:
        ESP_LOGI(TAG, "Time successfully synced with NTP");
        break;
    case SNTP_SYNC_STATUS_IN_PROGRESS:
        ESP_LOGI(TAG, "NTP sync in progress");
        break;
    case SNTP_SYNC_STATUS_RESET:
        ESP_LOGI(TAG, "NTP sync not started or failed");
        break;
    }
}

bool sntpReady()
{
    return ntpSynced;
    //// For DEBUG
    //// checkTimeSync();
    // sntp_sync_status_t syncStatus = sntp_get_sync_status();
    //
    // switch (syncStatus)
    //{
    // case SNTP_SYNC_STATUS_COMPLETED:
    //    return true;
    //}
    // return false;
}

bool rtcHasRTC() { return flagHasRTC; }
bool rtcLostPower() { return rtc.lostPower(); }
bool rtcHasTime() { return rtcHasRTC() && !rtc.lostPower(); }

bool rtcAdjustFromSntp()
{
    if (!sntpReady() || !rtcHasRTC())
        return false;

    // Обновляем RTC из NTP
    DateTime now(time(nullptr));
    rtc.adjust(now);

    return true;
}

// Функция для получения времени из DS3231 в виде структуры tm
bool getRTCTime(struct tm &timeinfo)
{
    // Если нет RTC все сбрасываем
    if (!rtcHasRTC())
    {
        timeinfo = {0};
        timeinfo.tm_mday = 1; // Корректируем обязательные поля:
        timeinfo.tm_isdst = -1;
        return false;
    }

    DateTime now = rtc.now(); // Читаем текущее время из RTC

    // Заполняем структуру tm
    timeinfo.tm_sec = now.second();
    timeinfo.tm_min = now.minute();
    timeinfo.tm_hour = now.hour();
    timeinfo.tm_mday = now.day();
    timeinfo.tm_mon = now.month() - 1;     // В tm месяцы 0-11 (январь = 0)
    timeinfo.tm_year = now.year() - 1900;  // Год с 1900
    timeinfo.tm_wday = now.dayOfTheWeek(); // 0-6 (воскресенье = 0)
    timeinfo.tm_isdst = -1;                // Летнее время не учитывается (-1 = неизвестно)

    return true;
}

bool rtcAdjust(DateTime &dt)
{
    if (!rtcHasRTC())
        return false;

    rtc.adjust(dt);

    return true;
}

float rtcGetTemperature()
{
    if (!rtcHasRTC())
        return -100;
    return rtc.getTemperature();
}

void syncSystemTimeFromRTC()
{
    if (!rtcHasRTC)
        return;

    if (!rtc.lostPower())
        return;

    DateTime now = rtc.now();
    time_t rtc_time = now.unixtime();
    struct timeval tv = {rtc_time, 0};
    settimeofday(&tv, NULL); // Устанавливаем системное время из RTC

    ESP_LOGI(TAG, "Time set from RTC");
}
