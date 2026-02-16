#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int mtx_timedlock_ms(mtx_t *mutex, int timeout_ms)
{
    TRAPNULL(mutex);
    int status = thrd_error;
    if(timeout_ms<0)
    {
        status = mtx_lock(mutex);
        TRAPF((status!=thrd_success), mtx_lock, "%i", status);
    }
    else
    {
        struct timespec ts;
        status = timeout_ms ? timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms) : timespec_get(&ts, TIME_UTC);
        TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);
        status = mtx_timedlock(mutex, &ts);
        TRAPF((status!=thrd_success)&&(status!=thrd_timedout), mtx_timedlock, "%i", status);
    }
    return status;
trap_mtx_timedlock:
trap_mtx_lock:
trap_timespec_get_offset_ms:
trap_mutex_null:
    return thrd_error;
}
