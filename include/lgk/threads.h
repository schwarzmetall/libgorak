#ifndef LGK_THREADS_H
#define LGK_THREADS_H

#include <stdatomic.h>
#include <stdint.h>
#include <threads.h>

#define THRD_FAIL(x) ((x!=thrd_success) && (x!=thrd_timedout))

struct lgk_monitor
{
    mtx_t mutex;
    cnd_t cond;
};

struct lgk_thread
{
    thrd_t thread;
    thrd_start_t start;
    void *arg;
    int res;
    struct lgk_monitor *monitor;
    atomic_int_fast8_t stopped;
};  

const char *lgk_thrdstrerror(int thrd_status);

#warning ("wall-clock TIME_UTC in timed thread functions (C11-C23) issue explanation here")
/* 
 * Timed mutex/condition waits (mtx_timedlock_ms, cnd_timedwait_ms, *_ts, queue
 * push/pop timeouts, lgk_thread_join): deadlines are built with TIME_UTC and
 * passed to C11 mtx_timedlock/cnd_timedwait, which interpret time_point as
 * calendar/wall-clock time (on glibc: CLOCK_REALTIME).
 *
 * C11–C23 oversight: C23 added TIME_MONOTONIC to timespec_get, but standard
 * mtx_timedlock/cnd_timedwait were never extended to accept monotonic abstime.
 * Duration-style timeouts are therefore vulnerable to NTP steps and manual clock
 * changes (spurious thrd_timedout or waits longer than requested).
 */

[[deprecated("wall-clock TIME_UTC in timed thread functions (C11-C23) issue; see threads.h for details")]] // not actual deprecation - remove when issue is resolved
int mtx_timedlock_ts(mtx_t *mutex, const struct timespec *timeout_ts);
[[deprecated("wall-clock TIME_UTC in timed thread functions (C11-C23) issue; see threads.h for details")]] // not actual deprecation - remove when issue is resolved
int mtx_timedlock_ms(mtx_t *mutex, int timeout_ms);
[[deprecated("wall-clock TIME_UTC in timed thread functions (C11-C23) issue; see threads.h for details")]] // not actual deprecation - remove when issue is resolved
int cnd_timedwait_ts(cnd_t *cond, mtx_t *mutex, const struct timespec *timeout_ts);
[[deprecated("wall-clock TIME_UTC in timed thread functions (C11-C23) issue; see threads.h for details")]] // not actual deprecation - remove when issue is resolved
int cnd_timedwait_ms(cnd_t *cond, mtx_t *mutex, int timeout_ms);

int lgk_monitor_init(struct lgk_monitor *m, int_fast8_t timed);
int lgk_monitor_destroy(struct lgk_monitor *m);

int lgk_thread_create(struct lgk_thread *t, thrd_start_t start, void *arg, struct lgk_monitor *monitor);
int lgk_thread_join(struct lgk_thread *t, int *res, int timeout_ms, int_fast8_t timeout_detach);

#endif
