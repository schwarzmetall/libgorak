#include <threads.h>
#include <lgk/tnt.h>
#include <lgk/threads.h>

int lgk_monitor_init(struct lgk_monitor *m, int_fast8_t timed)
{
    TRAPVNULL(m);
    int status = mtx_init(&m->mutex, timed?mtx_timed:mtx_plain);
    TRAPF(status!=thrd_success, mtx_init, status, "i");
    status = cnd_init(&m->cond);
    TRAPF(status!=thrd_success, cnd_init, status, "i");
    return status;
trap_cnd_init:
    mtx_destroy(&m->mutex);
trap_mtx_init:
    return status;
trap_m_null:
    return thrd_error;
}

int lgk_monitor_destroy(struct lgk_monitor *m)
{
    TRAPVNULL(m);
    mtx_destroy(&m->mutex);
    cnd_destroy(&m->cond);
    return thrd_success;
trap_m_null:
    return thrd_error;
}
