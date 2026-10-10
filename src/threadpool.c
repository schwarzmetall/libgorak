#include <threads.h>
#include <lgk/tnt.h>
#include <lgk/threads.h>
#include <lgk/threadpool.h>

static FIFO_INIT(fifo_threadpool_work)
static FIFO_PUSH(fifo_threadpool_work)
static FIFO_POP(fifo_threadpool_work)

static QUEUE_INIT(fifoq_threadpool_work, fifo_threadpool_work)
static QUEUE_CLOSE(fifoq_threadpool_work, fifo_threadpool_work)
static QUEUE_PUSH(fifoq_threadpool_work, fifo_threadpool_work)
static QUEUE_POP(fifoq_threadpool_work, fifo_threadpool_work)

static int worker_thread_function(void *data)
{
    struct threadpool *tp = data;
    int status = thrd_success;
    while(status==thrd_success)
    {
        struct threadpool_work work = {};
        int status_pop = fifoq_threadpool_work_pop(&tp->work_queue, &work, tp->queue_timeout_ms);
        if(status_pop == thrd_timedout) continue;
        status = status_pop;
        TRAPFT(status_pop!=thrd_success, fifoq_threadpool_work_pop, status);
        if(work.start == NULL) break;
        work.done_callback(work.data, work.start(work.data));
    }
    return status;
trap_fifoq_threadpool_work_pop:
    return status;
}

static int threadpool_signal_and_join_workers(struct threadpool *tp, unsigned n_threads, int join_timeout_ms, int_fast8_t timeout_detach)
{
    TRAPVNULL(tp);
    int status = thrd_success;
    struct threadpool_work shutdown_work = {};
    for(unsigned i = 0; i < n_threads; i++)
    {
        int status_push = fifoq_threadpool_work_push(&tp->work_queue, &shutdown_work, tp->queue_timeout_ms);
        if(status_push != thrd_success)
        {
            if(status == thrd_success) status = status_push;
            CRITFT(fifoq_threadpool_work_push, status_push);
        }
    }
    for(unsigned i = 0; i < n_threads; i++)
    {
        int status_work = thrd_error;
        int status_join = lgk_thread_join(&tp->thread_buffer[i], &status_work, join_timeout_ms, timeout_detach);
        if(status_join != thrd_success)
        {
            CRITFT(lgk_thread_join, status_join);
            if(status == thrd_success) status = status_join;
        }
        if(status_work != thrd_success)
        {
            CRIT("worker_thread_function() [%u]: %i", i, status_work);
            if(status == thrd_success) status = status_work;
        }
    }
    return status;
trap_tp_null:
    return thrd_error;
}

int threadpool_init(struct threadpool *tp, struct lgk_thread *thread_buffer, struct threadpool_work *queue_buffer, unsigned n_threads, unsigned queue_size, int_fast8_t timed_join, int queue_timeout_ms)
{
    TRAPVNULL(tp);
    TRAPVNULL(thread_buffer);
    TRAPVNULL(queue_buffer);
    tp->queue_timeout_ms = queue_timeout_ms;
    int status = lgk_monitor_init(&tp->monitor, timed_join);
    TRAPFT(status!=thrd_success, lgk_monitor_init, status);
    status = fifoq_threadpool_work_init(&tp->work_queue, queue_buffer, queue_size, (queue_timeout_ms>=0));
    TRAPFT(status!=thrd_success, fifoq_threadpool_work_init, status);
    unsigned n_threads_created = 0;
    while((n_threads_created < n_threads) && (status==thrd_success)) status = lgk_thread_create(&thread_buffer[n_threads_created++], worker_thread_function, tp, &tp->monitor);
    TRAPFT(status!=thrd_success, lgk_thread_create, status);
    tp->n_threads = n_threads;
    tp->queue_size = queue_size;
    tp->thread_buffer = thread_buffer;
    return thrd_success;
trap_lgk_thread_create:
    int status_cleanup = threadpool_signal_and_join_workers(tp, n_threads_created, tp->queue_timeout_ms, 1);
    if(status_cleanup != thrd_success) CRITFT(threadpool_signal_and_join_workers, status_cleanup);
    status_cleanup = fifoq_threadpool_work_close(&tp->work_queue);
    if(status_cleanup != thrd_success) CRITFT(fifoq_threadpool_work_close, status_cleanup);
trap_fifoq_threadpool_work_init:
    status_cleanup = lgk_monitor_destroy(&tp->monitor);
    if(status_cleanup != thrd_success) CRITFT(lgk_monitor_destroy, status_cleanup);
trap_lgk_monitor_init:
    return status;
trap_queue_buffer_null:
trap_thread_buffer_null:
trap_tp_null:
    return thrd_error;
}

int threadpool_close(struct threadpool *tp, int join_timeout_ms, int_fast8_t timeout_detach)
{
    TRAPVNULL(tp);
    int status = threadpool_signal_and_join_workers(tp, tp->n_threads, join_timeout_ms, timeout_detach);
    int status_cleanup = lgk_monitor_destroy(&tp->monitor);
    if(status_cleanup != thrd_success) CRITFT(lgk_monitor_destroy, status_cleanup);
    status_cleanup = fifoq_threadpool_work_close(&tp->work_queue);
    if(status_cleanup != thrd_success)
    {
        CRITFT(fifoq_threadpool_work_close, status_cleanup);
        if(status == thrd_success) status = status_cleanup;
    }
    return status;
trap_tp_null:
    return thrd_error;
}

int threadpool_schedule_work(struct threadpool *tp, thrd_start_t start, threadpool_work_done_callback *work_done_cb, void *work_data)
{
    TRAPVNULL(tp);
    TRAPVNULL(start);
    const struct threadpool_work work =
    {
        start,
        work_done_cb,
        work_data
    };
    int status = fifoq_threadpool_work_push(&tp->work_queue, &work, tp->queue_timeout_ms);
    TRAPFT(status!=thrd_success, fifoq_threadpool_work_push, status);
    return thrd_success;
trap_fifoq_threadpool_work_push:
    return status;
trap_start_null:
trap_tp_null:
    return thrd_error;
}
