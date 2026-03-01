#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int cnd_timedwait_ts(cnd_t *cond, mtx_t *mutex, const struct timespec *ts)
{
    TRAPNULL(cond);
    TRAPNULL(mutex);
    int status = thrd_error;
    if(ts)
    {
        status = cnd_timedwait(cond, mutex, ts);
        TRAPF((status!=thrd_success)&&(status!=thrd_timedout), cnd_timedwait, "%i", status);
    }
    else
    {
        status = cnd_wait(cond, mutex);
        TRAPF(status!=thrd_success, cnd_wait, "%i", status);
    }
    return status;
trap_cnd_wait:
trap_cnd_timedwait:
trap_mutex_null:
trap_cond_null:
    return thrd_error;
}
