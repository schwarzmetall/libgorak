#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

int cnd_timedwait_ms(cnd_t *cond, mtx_t *mutex, int timeout_ms)
{
    TRAPNULL(cond);
    TRAPNULL(mutex);
    int status = thrd_error;
    if(timeout_ms<0)
    {
        status = cnd_wait(cond, mutex);
        if(status!=thrd_success) FATALF(cnd_wait, "%i", status);
        return status;
    }
    struct timespec ts;
    status = timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms);
    TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);
    status = cnd_timedwait(cond, mutex, &ts);
    if((status!=thrd_success)&&(status!=thrd_timedout)) FATALF(cnd_timedwait, "%i", status);
    return status;
trap_timespec_get_offset_ms:
trap_mutex_null:
trap_cond_null:
    return thrd_error;
}
