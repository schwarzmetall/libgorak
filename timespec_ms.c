#include <time.h>
#include <stdint.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>

void timespec_offset_ms(struct timespec *ts, int offset_ms)
{
    int_least32_t nsec = ts->tv_nsec + (offset_ms * 1000 * 1000);
    if((nsec >= 0) && (nsec < 999999999)) return;  // TODO: what is faster on most systems? just divide ad add sec_offset or check first?
    // TODO FIXME negative offsets not handled!!
    ts->tv_sec += nsec / 1000000000;
    ts->tv_nsec = nsec % 1000000000;
}

int timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms)
{
    int status = timespec_get(ts, base);
    TRAP(status!=base, timespec_get, "timespec_get(%i): %i", base, status);
    timespec_offset_ms(ts, offset_ms);
    return status;
trap_timespec_get:
    return status;
}
