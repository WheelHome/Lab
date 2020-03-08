#ifndef TIMESTAMP_HPP_
#define TIMESTAMP_HPP_

#include <time.h>

namespace
{
    class Timestamp
    {
        static time_t Today()
        {
            time_t today = time(nullptr);
            return today - (today + 28800) % 86400;
        }

        static time_t Time(int day = 0, int hour = 0, int min = 0, int sec = 0)
        {
            time_t today = Today();
            return today + day * 86400 + hour * 3600 + min * 60 + sec;
        }
    };
}
#endif

