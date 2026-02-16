#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

static int thread_start_wrapper(void *arg_wrapper)
{
    struct lgk_thread *t = arg_wrapper;
    TRAPNULL(t);
    t->res = t->start(t->arg);
    int status = mtx_timedlock_ms(&t->monitor->mutex, t->timeout_ms);
    TRAPF(status!=thrd_success, mtx_timedlock_ms, "%i", status);
    t->stopped = 1;
    status = cnd_signal(&t->monitor->cond);
    if(status!=thrd_success) FATALF(cnd_signal, "%i", status);
    int status_unlock = mtx_unlock(&t->monitor->mutex);
    return (status == thrd_success) ? status_unlock : status;
trap_mtx_timedlock_ms:
    return status;
trap_t_null:
    return thrd_error;
}

int lgk_thread_create(struct lgk_thread *t, thrd_start_t start, void *arg, struct lgk_monitor *monitor, int timeout_ms)
{
    TRAPNULL(t);
    t->start = start;
    t->arg = arg;
    t->res = 0;
    t->monitor = monitor;
    t->stopped = 0;
    t->timeout_ms = timeout_ms;
    int status = thrd_create(&t->thread, thread_start_wrapper, t);
    if(status!=thrd_success) FATALF(thrd_create, "%i", status);
    return status;
trap_t_null:
    return thrd_error;
}

int lgk_thread_join(struct lgk_thread *t, int *res, int_fast8_t timeout_detach)
{
    TRAPNULL(t);
    int res_wrapper;
    int status = thrd_success;
    int status_unlock = thrd_success;
    if(t->timeout_ms >= 0)
    {
        TRAPNULL_L(t->monitor, t_monitor);
        struct timespec ts;
        status = timespec_get_offset_ms(&ts, TIME_UTC, t->timeout_ms);
        TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);
        status = mtx_timedlock_ts(&t->monitor->mutex, &ts);
        TRAPF(status!=thrd_success, mtx_timedlock_ts, "%i", status);
        while(!t->stopped && (status==thrd_success)) status = cnd_timedwait_ts(&t->monitor->cond, &t->monitor->mutex, &ts);
        TRAPF((status!=thrd_success)&&(status!=thrd_timedout), cnd_timedwait_ts, "%i", status);
        status_unlock = mtx_unlock(&t->monitor->mutex);
        TRAPF(status_unlock!=thrd_success, mtx_unlock, "%i", status_unlock);
        if(status == thrd_timedout)
        {
            if(timeout_detach)
            {
                int status_detach = thrd_detach(t->thread);
                if(status_detach != thrd_success) FATALF(thrd_detach, "%i", status_detach);
            }
            return status;
        }
    }
    status = thrd_join(t->thread, &res_wrapper);
    TRAPF(status!=thrd_success, thrd_join, "%i", status);
    if(res_wrapper != thrd_success) status = thrd_error;
    else *res = t->res;
    if(!t->stopped) FATAL("t->stopped==%i", t->stopped);
    return status;
trap_thrd_join:
    return status;
trap_mtx_unlock:
    return status_unlock;
trap_cnd_timedwait_ts:
    status_unlock = mtx_unlock(&t->monitor->mutex);
    if(status_unlock != thrd_success) FATALF(mutex_unlock, "%i", status_unlock);
trap_mtx_timedlock_ts:
    return status;
trap_timespec_get_offset_ms:
trap_t_monitor_null:
trap_t_null:
    return thrd_error;
}

