#ifndef DATE_TIME_H
#define DATE_TIME_H
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
namespace cffex
{
namespace strategy
{
class time
{
public:
    /** seconds: seconds since 00:00:00
 ** return writed bytes which is always 8 bytes
*/
    static int to_string(int seconds, char out[16])
    {
        static const int MAX_TIME_VALUE = 24 * 3600;
        if (seconds < 0 || seconds > MAX_TIME_VALUE)
        {
            strcpy(out, "00:00:00");
            return 8;
        }
        snprintf(out, 15, "%02d:%02d:%02d", seconds / 3600, (seconds % 3600) / 60, seconds % 3600 % 60);
        return 8;
    }

    /** time: 10:10:10 */
    static int to_seconds(const char *time)
    {
        int hour, min, sec;
        if (3 != sscanf(time, "%d:%d:%d", &hour, &min, &sec))
        {
            return 0;
        }
        return hour * 3600 + min * 60 + sec;
    }

    /** timestamp: UTC timestamp */
    static int get_day_seconds(time_t timestamp)
    {
        struct tm t;
        localtime_r(&timestamp, &t);
        return t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
    }

    /** return seconds since 00:00:00 */
    static int get_current_day_seconds()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return get_day_seconds(tv.tv_sec);
    }

    static int64_t get_current_milli_seconds()
    {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return (int64_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
    }
};

}
}
#endif