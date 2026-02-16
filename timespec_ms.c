#include <time.h>
#include <stdint.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>

void timespec_offset_ms(struct timespec *ts, int offset_ms)
{
// TODO refactor to only use 32-bit arithmetics
    int_least64_t nsec = ts->tv_nsec + (offset_ms*1000);
    int_least32_t offset_sec = nsec / (1000*1000*1000);
    nsec %= 1000*1000*1000;
    if(nsec < 0)
    {
        nsec += (1000*1000*1000);
        offset_sec--;
    }
    ts->tv_sec += offset_sec;
    ts->tv_nsec = nsec;
}

int timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms)
{
    int status = timespec_get(ts, base);
    TRAPF(status!=base, timespec_get, "%i", base, status);
    timespec_offset_ms(ts, offset_ms);
    return status;
trap_timespec_get:
    return status;
}
