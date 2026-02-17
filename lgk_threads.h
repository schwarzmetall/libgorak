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
    int timeout_ms;
    atomic_int_fast8_t stopped;
};  

int mtx_timedlock_ts(mtx_t *mutex, const struct timespec *timeout_ts);
int mtx_timedlock_ms(mtx_t *mutex, int timeout_ms);
int cnd_timedwait_ts(cnd_t *cond, mtx_t *mutex, const struct timespec *timeout_ts);
int cnd_timedwait_ms(cnd_t *cond, mtx_t *mutex, int timeout_ms);

int lgk_monitor_init(struct lgk_monitor *m, int_fast8_t timed);
int lgk_monitor_destroy(struct lgk_monitor *m);

int lgk_thread_create(struct lgk_thread *t, thrd_start_t start, void *arg, struct lgk_monitor *monitor, int timeout_ms);
int lgk_thread_join(struct lgk_thread *t, int *res, int_fast8_t timeout_detach);

#endif
