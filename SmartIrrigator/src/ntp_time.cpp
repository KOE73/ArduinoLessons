#include "ntp_time.h"
#include <time.h>

#define TIME_OFFSET 5

void initTime() {
    configTime(3600 * TIME_OFFSET, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
}

bool getHasTime() {
    struct tm timeinfo;
    return getLocalTime(&timeinfo,0);
}

uint16_t getMinutesSinceMidnight() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_hour * 60 + timeinfo.tm_min;
    }
    return 0; // по умолчанию
}
