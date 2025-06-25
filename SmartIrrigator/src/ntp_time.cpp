#include "ntp_time.h"
#include <time.h>
#include <esp_sntp.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
bool flagHasRTC;

#define TIME_OFFSET 5

static const char *TAG = "TIMEs";

void time_sync_notification_cb(struct timeval *tv)
{
    Serial.println("Notification: Time synced with NTP server!");
}

void initTime()
{
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    // configTime(3600 * TIME_OFFSET, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)

    flagHasRTC = rtc.begin();

    if (!rtcHasRTC)
        ESP_LOGI(TAG, "Couldn't find RTC");

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, let's set the time!");
        // Установите время компиляции
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

uint16_t getMinutesSinceMidnight(tm &timeInfo)
{
    return timeInfo.tm_hour * 60 + timeInfo.tm_min;
}

bool sntpReady()
{
    // For DEBUG
    checkTimeSync();
    sntp_sync_status_t syncStatus = sntp_get_sync_status();

    switch (syncStatus)
    {
    case SNTP_SYNC_STATUS_COMPLETED:
        return true;
    }
    return false;
}

void checkTimeSync()
{
    sntp_sync_status_t syncStatus = sntp_get_sync_status();

    switch (syncStatus)
    {
    case SNTP_SYNC_STATUS_COMPLETED:
        Serial.println("Time successfully synced with NTP");
        break;
    case SNTP_SYNC_STATUS_IN_PROGRESS:
        Serial.println("NTP sync in progress");
        break;
    case SNTP_SYNC_STATUS_RESET:
        Serial.println("NTP sync not started or failed");
        break;
    }
}

bool rtcHasRTC() { return flagHasRTC; }
bool rtcLostPower() { return rtc.lostPower(); }

bool rtcAdjustFromSntp()
{
    if (!sntpReady() || !rtcHasRTC())
        return false;

    // Обновляем RTC из NTP
    DateTime now(time(nullptr));
    rtc.adjust(now);

    return true;
}

bool rtcAdjust(DateTime &dt)
{
    if (!rtcHasRTC())
        return false;

    rtc.adjust(dt);

    return true;
}
