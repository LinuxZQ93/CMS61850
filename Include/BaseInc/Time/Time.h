#ifndef __FRAMEWORK_INCLUDE_TIME_TIME_H__
#define __FRAMEWORK_INCLUDE_TIME_TIME_H__

#include <stdint.h>

namespace base {
class CTime {
public:
    struct SysTime {
        unsigned int milliSec;
        unsigned int sec;
        unsigned int min;
        unsigned int hour;
        unsigned int day;
        unsigned int mon;
        unsigned int year;
        unsigned int wday;
    };
public:
    static unsigned long long int getCurrentTime();
    static void getSysTime(SysTime &tmInfo);
    static uint64_t sysTimeToUTC(const SysTime &tmInfo);
    static void utcToSysTime(uint64_t timeNow, SysTime &tmInfo);
};
}

#endif /* __FRAMEWORK_INCLUDE_TIME_TIME_H__ */