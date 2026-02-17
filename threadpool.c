#include <stddef.h>
#include <stdint.h>
#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_threads.h>
#include <lgk_queue_int.h>
#include <lgk_threadpool.h>

static int worker_thread_function(void *data)
{
    struct threadpool *tp = data;
    int status = thrd_success;
    while(status==thrd_success)
    {
        int i_work = -1;
        int status_pop = queue_int_pop(&tp->work_queue, &i_work, tp->queue_timeout_ms);
        if(status_pop == thrd_timedout) continue;
        TRAPF(status_pop!=thrd_success, queue_int_pop, "%i", (status = status_pop));
        if(i_work < 0) break;
        struct threadpool_work *work = tp->work_buffer + i_work;
        work->done_callback(work->data, work->start(work->data));
        status = queue_int_push(&tp->work_pool, i_work, tp->queue_timeout_ms);
        TRAPF(status!=thrd_success, queue_int_push, "%i", status);
    }
    return status;
trap_queue_int_push:
trap_queue_int_pop:
    return status;
}

static int threadpool_signal_and_join_workers(struct threadpool *tp, unsigned n_threads, int join_timeout_ms, int_fast8_t timeout_detach)
{
    int status = thrd_success;
    for(unsigned i = 0; i < n_threads; i++)
    {
        int status_push = queue_int_push(&tp->work_queue, -1, tp->queue_timeout_ms);
        if(status_push != thrd_success)
        {
            if(status == thrd_success) status = status_push;
            FATALF(queue_int_push, "%i", status_push);
        }
    }
    for(unsigned i = 0; i < n_threads; i++)
    {
        int status_work = thrd_error;
        int status_join = lgk_thread_join(&tp->thread_buffer[i], &status_work, join_timeout_ms, timeout_detach);
        if(status_join != thrd_success)
        {
            FATALF(lgk_thread_join, "%i", status_join);
            if(status == thrd_success) status = status_join;
        }
        if(status_work != thrd_success)
        {
            FATAL("worker_thread_function() [%u]: %i", i, status_work);
            if(status == thrd_success) status = status_work;
        }
    }
    return status;
}

int threadpool_init(struct threadpool *tp, const struct threadpool_buffer_info *buffer_info, unsigned n_threads, unsigned queue_size, int_fast8_t timed, int queue_timeout_ms)
{
    TRAPNULL(tp);
    TRAPNULL(buffer_info);
    tp->queue_timeout_ms = queue_timeout_ms;
    int status = lgk_monitor_init(&tp->monitor, timed);
    TRAPF(status!=thrd_success, lgk_monitor_init, "%i", status);
    status = queue_int_init(&tp->work_queue, buffer_info->work_queue_buffer, queue_size, timed);
    TRAPF(status!=thrd_success, queue_int_init, "%i", status);
    for(unsigned i=0; i<queue_size; i++) buffer_info->work_pool_buffer[i] = i;
    status = queue_int_init_prefilled(&tp->work_pool, buffer_info->work_pool_buffer, queue_size, queue_size, timed);
    TRAPF(status!=thrd_success, queue_int_init_prefilled, "%i", status);
    unsigned n_threads_created = 0;
    while((n_threads_created < n_threads) && (status==thrd_success)) status = lgk_thread_create(&buffer_info->thread_buffer[n_threads_created++], worker_thread_function, tp, &tp->monitor);
    TRAPF(status!=thrd_success, lgk_thread_create, "%i", status);
    tp->n_threads = n_threads;
    tp->thread_buffer = buffer_info->thread_buffer;
    tp->work_buffer = buffer_info->work_buffer;
    return thrd_success;
trap_lgk_thread_create:
    int status_cleanup = threadpool_signal_and_join_workers(tp, n_threads_created, tp->queue_timeout_ms, 1);
    if(status_cleanup != thrd_success) FATALF(threadpool_signal_and_join_workers, "%i", status_cleanup);
    status_cleanup = queue_int_close(&tp->work_pool);
    if(status_cleanup != thrd_success) FATALF(queue_int_close, "%i", status_cleanup);
trap_queue_int_init_prefilled:
    status_cleanup = queue_int_close(&tp->work_queue);
    if(status_cleanup != thrd_success) FATALF(queue_int_close, "%i", status_cleanup);
trap_queue_int_init:
    status_cleanup = lgk_monitor_destroy(&tp->monitor);
    if(status_cleanup != thrd_success) FATALF(lgk_monitor_destroy, "%i", status_cleanup);
trap_lgk_monitor_init:
    return status;
trap_buffer_info_null:
trap_tp_null:
    return thrd_error;
}

int threadpool_close(struct threadpool *tp, int join_timeout_ms, int_fast8_t timeout_detach)
{
    TRAPNULL(tp);
    int status = threadpool_signal_and_join_workers(tp, tp->n_threads, join_timeout_ms, timeout_detach);
    int status_cleanup = lgk_monitor_destroy(&tp->monitor);
    if(status_cleanup != thrd_success) FATALF(lgk_monitor_destroy, "%i", status_cleanup);
    status_cleanup = queue_int_close(&tp->work_queue);
    if(status_cleanup != thrd_success)
    {
        FATALF(queue_int_close, "%i", status_cleanup);
        if(status == thrd_success) status = status_cleanup;
    }
    status_cleanup = queue_int_close(&tp->work_pool);
    if(status_cleanup != thrd_success)
    {
        FATALF(queue_int_close, "%i", status_cleanup);
        if(status == thrd_success) status = status_cleanup;
    }
    return status;
trap_tp_null:
    return thrd_error;
}

int threadpool_schedule_work(struct threadpool *tp, thrd_start_t start, threadpool_work_done_callback *work_done_cb, void *work_data)
{
    TRAPNULL(tp);
    int i_work;
    int status = queue_int_pop(&tp->work_pool, &i_work, tp->queue_timeout_ms);
    TRAPF(status!=thrd_success, queue_int_pop, "%i", status);
    struct threadpool_work *work = tp->work_buffer + i_work;
    work->start = start;
    work->done_callback = work_done_cb;
    work->data = work_data;
    status = queue_int_push(&tp->work_queue, i_work, tp->queue_timeout_ms);
    TRAPF(status!=thrd_success, queue_int_push, "%i", status);
    return thrd_success;
trap_queue_int_push:
    {
        int status_tmp = queue_int_push(&tp->work_pool, i_work, tp->queue_timeout_ms);
        if(status_tmp != thrd_success) FATALF(queue_int_push, "%i", status_tmp);
    }
trap_queue_int_pop:
    return status;
trap_tp_null:
    return thrd_error;
}
