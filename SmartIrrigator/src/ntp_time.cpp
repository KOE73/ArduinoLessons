#include "ntp_time.h"
#include <time.h>

#define TIME_OFFSET 5

void initTime()
{
    // configTime(3600 * TIME_OFFSET, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", "ntp1.stratum2.ru"); // МСК по умолчанию (UTC+3)
}


uint16_t getMinutesSinceMidnight(tm &timeInfo)
{
    return timeInfo.tm_hour * 60 + timeInfo.tm_min;
}
