#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int mtx_timedlock_ts(mtx_t *mutex, const struct timespec *ts)
{
    TRAPNULL(mutex);
    int status = thrd_error;
    if(ts)
    {
        status = mtx_timedlock(mutex, ts);
        TRAPF((status!=thrd_success)&&(status!=thrd_timedout), mtx_timedlock, "%i", status);
    }
    else
    {
        status = mtx_lock(mutex);
        TRAPF(status!=thrd_success, mtx_lock, "%i", status);
    }
    return status;
trap_mtx_lock:
trap_mtx_timedlock:
trap_mutex_null:
    return status;
}
