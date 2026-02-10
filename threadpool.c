#include <stddef.h>
#include <stdint.h>
#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_queue_int.h>
#include <lgk_threadpool.h>

#define THREADPOOL_TIMEOUT_DEFAULT_MS (10*1000)

static int worker_thread_function(void *data)
{
    struct threadpool *tp = data;
    int status = thrd_error;
    while(!tp->close)
    {
        int i_work;
        status = queue_int_pop(&tp->work_queue, &i_work, tp->timeout_ms);
        if(status == thrd_timedout) continue;
        TRAPF(status!=thrd_success, queue_int_pop, "%i", status);
        if(i_work < 0) break;
        struct threadpool_work *work = tp->work_buffer + i_work;
        work->done_callback(work->data, work->start(work->data));
        status = queue_int_push(&tp->work_pool, i_work, tp->timeout_ms);
        TRAPF(status!=thrd_success, queue_int_push, "%i", status);
    }
    return thrd_success;
trap_queue_int_push:
trap_queue_int_pop:
    return thrd_error;
}

int threadpool_init(struct threadpool *tp, const struct threadpool_buffer_info *buffer_info, unsigned n_threads, unsigned queue_size, uint_fast8_t flags, unsigned timeout_ms)
{
    tp->timeout_ms = (timeout_ms) ? timeout_ms : THREADPOOL_TIMEOUT_DEFAULT_MS;
    uint_fast8_t flags_queue = (flags & THREADPOOL_FLAG_UNTIMED) ? QUEUE_FLAG_UNTIMED : 0;
    int status = queue_int_init(&tp->work_queue, buffer_info->work_queue_buffer, queue_size, flags_queue);
    TRAPF(status!=thrd_success, queue_int_init, "%i", status);
    for(unsigned i=0; i<queue_size; i++) buffer_info->work_pool_buffer[i] = i;
    status = queue_int_init_prefilled(&tp->work_pool, buffer_info->work_pool_buffer, queue_size, queue_size, flags_queue);
    TRAPF(status!=thrd_success, queue_int_init_prefilled, "%i", status);
    unsigned i;
    for(i=0; i<n_threads; i++)
    {
        status = thrd_create(buffer_info->thread_buffer + i, worker_thread_function, tp);
        TRAPF(status!=thrd_success, thrd_create, "%i", status);
    }
    tp->n_threads = n_threads;
    tp->thread_buffer = buffer_info->thread_buffer;
    tp->work_buffer = buffer_info->work_buffer;
    tp->close = 0;
    return thrd_success;
trap_thrd_create:
    tp->close = 1;
    for(unsigned i_exit=0; i_exit<i; i_exit++)
    {
        int status_push = queue_int_push(&tp->work_queue, -1, tp->timeout_ms);
        if(status_push != thrd_success) ERRF(queue_int_push, "%i", status_push);
    }
    for(unsigned i_exit=0; i_exit<i; i_exit++)
    {
        /* TODO: might block indefinitely */
        int status_join = thrd_join(tp->thread_buffer[i], NULL);
        if(status_join != thrd_success) ERRF(thrd_join, "%i", status_join);
    }
    queue_int_close(&tp->work_pool);
trap_queue_int_init_prefilled:
    queue_int_close(&tp->work_queue);
trap_queue_int_init:
    return status;
}

int threadpool_close(struct threadpool *tp)
{
    int status = thrd_success;
    tp->close = 1;
    for(unsigned i=0; i<tp->n_threads; i++)
    {
        int status_push = queue_int_push(&tp->work_queue, -1, tp->timeout_ms);
        if(status_push != thrd_success)
        {
            if(status == thrd_success) status = status_push;
            ERRF(queue_int_push, "%i", status_push);
        }
    }
    for(unsigned i=0; i<tp->n_threads; i++)
    {
        int status_work;
        /* TODO: might block indefinitely */
        int status_join = thrd_join(tp->thread_buffer[i], &status_work);
        if(status_join != thrd_success)
        {
            if(status == thrd_success) status = status_join;
            ERRF(thrd_join, "%i", status_join);
        }
        if(status_work != thrd_success)
        {
            if(status == thrd_success) status = status_work;
	    ERR("joined worker thread %u: %i", i, status_work);
        }
    }
    return status;
}

int threadpool_schedule_work(struct threadpool *tp, thrd_start_t start, threadpool_work_done_callback *work_done_cb, void *work_data)
{
    int i_work;
    int status = queue_int_pop(&tp->work_pool, &i_work, tp->timeout_ms);
    TRAPF(status!=thrd_success, queue_int_pop, "%i", status);
    struct threadpool_work *work = tp->work_buffer + i_work;
    work->start = start;
    work->done_callback = work_done_cb;
    work->data = work_data;
    status = queue_int_push(&tp->work_queue, i_work, tp->timeout_ms);
    TRAPF(status!=thrd_success, queue_int_push, "%i", status);
    return thrd_success;
trap_queue_int_push:
    {
        int status_putback = queue_int_push(&tp->work_pool, i_work, tp->timeout_ms);
        if(status_putback != thrd_success) ERRF(queue_int_push, "%i", status_putback);
    }
trap_queue_int_pop:
    return status;
}
