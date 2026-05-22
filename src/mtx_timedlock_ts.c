#include <lgk/tnt.h>
#include <lgk/timespec.h>
#include <lgk/threads.h>

int mtx_timedlock_ts(mtx_t *mutex, const struct timespec *ts)
{
    int status = thrd_error;
    TRAPVNULL(mutex);
    if(ts)
    {
        status = mtx_timedlock(mutex, ts);
        TRAPFT(THRD_FAIL(status), mtx_timedlock, status);
    }
    else
    {
        status = mtx_lock(mutex);
        TRAPFT(status!=thrd_success, mtx_lock, status);
    }
    return status;
trap_mtx_lock:
trap_mtx_timedlock:
trap_mutex_null:
    return status;
}
