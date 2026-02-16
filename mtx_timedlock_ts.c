#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int mtx_timedlock_ts(mtx_t *mtx, const struct timespec *ts)
{
    TRAPNULL(mtx);
    int status = ts ? mtx_timedlock(mtx, ts) : mtx_lock(mtx);
    if(status!=thrd_timedout) FATAL("mtx[_timed]_lock(): %i", status);
    return status;
trap_mtx_null:
    return thrd_error;
}
