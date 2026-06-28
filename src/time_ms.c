#include <stdint.h>
#include <time.h>
#include <lgk/tnt.h>
#include <lgk/time_ms.h>

int_fast8_t timespec_offset_ms(struct timespec *ts, int offset_ms)
{
    TRAPVNULL(ts);
    int_fast32_t offset_sec = offset_ms / 1000;
    int_fast32_t ms = offset_ms % 1000;
    /* 0..999 ms -> 0..999000000 nsec, fits in 32-bit integer */
    int_fast32_t nsec = ts->tv_nsec + ms * 1'000'000;
    offset_sec += nsec / (1'000'000'000);
    nsec %= (1'000'000'000);
    if(nsec < 0)
    {
        nsec += (1'000'000'000);
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

int_fast64_t timespec_to_ms(const struct timespec *ts)
{
    return ((int_fast64_t)ts->tv_sec * 1000) + (ts->tv_nsec / (1'000'000));
}

int_fast8_t time_ms(int base, int_fast64_t *out_ms)
{
    TRAPVNULL(out_ms);
    struct timespec ts;
    int status = timespec_get(&ts, base);
    TRAPF(status!=base, timespec_get, status, "i");
    *out_ms = timespec_to_ms(&ts);
    return 0;
trap_timespec_get:
trap_out_ms_null:
    return -1;
}

int_fast8_t time_offset_ms(int base, int offset_ms, int_fast64_t *out_ms)
{
    TRAPVNULL(out_ms);
    int_fast64_t now_ms = 0;
    int_fast8_t status = time_ms(base, &now_ms);
    TRAPF(status, time_ms, status, "i");
    *out_ms = now_ms + offset_ms;
    return 0;
trap_time_ms:
trap_out_ms_null:
    return -1;
}
