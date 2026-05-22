#include <lgk/tnt.h>
#include <lgk/timespec.h>
#include <lgk/threads.h>

int cnd_timedwait_ms(cnd_t *cond, mtx_t *mutex, int timeout_ms)
{
    TRAPVNULL(cond);
    TRAPVNULL(mutex);
    int status = thrd_error;
    if(timeout_ms<0)
    {
        status = cnd_wait(cond, mutex);
        TRAPFT(status!=thrd_success, cnd_wait, status);
    }
    else
    {
        struct timespec ts;
        status = timeout_ms ? timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms) : timespec_get(&ts, TIME_UTC);
        TRAPF(status, timespec_get_offset_ms, status, "i");
        status = cnd_timedwait(cond, mutex, &ts);
        TRAPFT(THRD_FAIL(status), cnd_timedwait, status);
    }
    return status;
trap_cnd_timedwait:
trap_cnd_wait:
trap_timespec_get_offset_ms:
trap_mutex_null:
trap_cond_null:
    return thrd_error;
}
