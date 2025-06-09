#include "ntp_time.h"
#include <time.h>

void initTime() {
    configTime(3600 * 3, 0, "pool.ntp.org"); // МСК по умолчанию (UTC+3)
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        delay(100);
    }
}

uint16_t getMinutesSinceMidnight() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return timeinfo.tm_hour * 60 + timeinfo.tm_min;
    }
    return 0; // по умолчанию
}
