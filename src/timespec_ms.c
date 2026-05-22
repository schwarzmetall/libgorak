#include <stdint.h>
#include <time.h>
#include <lgk/tnt.h>
#include <lgk/timespec.h>

int_fast8_t timespec_offset_ms(struct timespec *ts, int offset_ms)
{
    TRAPVNULL(ts);
    int_least32_t offset_sec = offset_ms / 1000;
    int_least32_t ms = offset_ms % 1000;
    /* 0..999 ms -> 0..999000000 nsec, fits in 32-bit integer */
    int_least32_t nsec = ts->tv_nsec + ms * 1000*1000;
    offset_sec += nsec / (1000*1000*1000);
    nsec %= (1000*1000*1000);
    if(nsec < 0)
    {
        nsec += (1000*1000*1000);
        offset_sec--;
    }
    ts->tv_sec += offset_sec;
    ts->tv_nsec = nsec;
    return 0;
trap_ts_null:
    return -1;
}

int_fast8_t timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms)
{
    TRAPVNULL(ts);
    int status = timespec_get(ts, base);
    TRAPF(status!=base, timespec_get, status, "i");
    status = timespec_offset_ms(ts, offset_ms);
    TRAPF(status, timespec_offset_ms, status, "i");
    return 0;
trap_timespec_offset_ms:
trap_timespec_get:
trap_ts_null:
    return -1;
}
