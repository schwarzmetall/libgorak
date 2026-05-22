#include <threads.h>
#include <lgk/tnt.h>
#include <lgk/timespec.h>
#include <lgk/threads.h>

int mtx_timedlock_ms(mtx_t *mutex, int timeout_ms)
{
    TRAPVNULL(mutex);
    int status = thrd_error;
    if(timeout_ms<0)
    {
        status = mtx_lock(mutex);
        TRAPFT(status!=thrd_success, mtx_lock, status);
    }
    else
    {
        struct timespec ts;
        status = timeout_ms ? timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms) : timespec_get(&ts, TIME_UTC);
        TRAPF(status, timespec_get_offset_ms, status, "i");
        status = mtx_timedlock(mutex, &ts);
        TRAPFT(THRD_FAIL(status), mtx_timedlock, status);
    }
    return status;
trap_mtx_timedlock:
trap_mtx_lock:
trap_timespec_get_offset_ms:
trap_mutex_null:
    return thrd_error;
}
