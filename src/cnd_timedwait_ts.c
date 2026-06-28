#include <lgk/tnt.h>
#include <lgk/time_ms.h>
#include <lgk/threads.h>

int cnd_timedwait_ts(cnd_t *cond, mtx_t *mutex, const struct timespec *ts)
{
    TRAPVNULL(cond);
    TRAPVNULL(mutex);
    int status = thrd_error;
    if(ts)
    {
        status = cnd_timedwait(cond, mutex, ts);
        TRAPFT(THRD_FAIL(status), cnd_timedwait, status);
    }
    else
    {
        status = cnd_wait(cond, mutex);
        TRAPFT(status!=thrd_success, cnd_wait, status);
    }
    return status;
trap_cnd_wait:
trap_cnd_timedwait:
trap_mutex_null:
trap_cond_null:
    return thrd_error;
}
