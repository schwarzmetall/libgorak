#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int mtx_timedlock_ms(mtx_t *restrict mutex, unsigned timeout_ms)
{
    struct timespec ts;
    int err = timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms);
    TRAPF(err!=TIME_UTC, timespec_get_offset_ms, "%i", err);
    err = mtx_timedlock(mutex, &ts);
    TRAPF(err!=thrd_success, mtx_timedlock, "%i", err);
    return thrd_success;
trap_mtx_timedlock:
    return err;
trap_timespec_get_offset_ms:
    return thrd_error;
}
