#include <stdatomic.h>
#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

static int thread_start_wrapper(void *arg_wrapper)
{
    struct lgk_thread *t = arg_wrapper;
    TRAPNULL(t);
    t->res = t->start(t->arg);
    atomic_store_explicit(&t->stopped, 1, memory_order_release);
    int status = cnd_broadcast(&t->monitor->cond);
    if(status != thrd_success) FATALF(cnd_broadcast, "%i", status);
    return status;
trap_t_null:
    return thrd_error;
}

int lgk_thread_create(struct lgk_thread *t, thrd_start_t start, void *arg, struct lgk_monitor *monitor)
{
    TRAPNULL(t);
    t->start = start;
    t->arg = arg;
    t->res = 0;
    t->monitor = monitor;
    atomic_init(&t->stopped, 0);
    int status = thrd_create(&t->thread, thread_start_wrapper, t);
    if(status!=thrd_success) FATALF(thrd_create, "%i", status);
    return status;
trap_t_null:
    return thrd_error;
}

int lgk_thread_join(struct lgk_thread *t, int *res, int timeout_ms, int_fast8_t timeout_detach)
{
    TRAPNULL(t);
    int res_wrapper;
    int status = thrd_success;
    int status_unlock = thrd_success;
    if(timeout_ms >= 0)
    {
        TRAPNULL_L(t->monitor, t_monitor);
        struct timespec ts;
        status = timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms);
        TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);
        status = mtx_timedlock_ts(&t->monitor->mutex, &ts);
        TRAPF(status!=thrd_success, mtx_timedlock_ts, "%i", status);
        while(!atomic_load_explicit(&t->stopped, memory_order_acquire) && (status==thrd_success)) status = cnd_timedwait_ts(&t->monitor->cond, &t->monitor->mutex, &ts);
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
    if(!atomic_load_explicit(&t->stopped, memory_order_acquire)) FATAL("t->stopped==%i", (int)atomic_load(&t->stopped));
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

