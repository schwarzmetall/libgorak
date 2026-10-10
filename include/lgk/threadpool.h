#ifndef LGK_THREADPOOL_H
#define LGK_THREADPOOL_H

#include <stdint.h>
#include <threads.h>
#include <lgk/threads.h>
#include <lgk/fifo.h>
#include <lgk/queue.h>

#define THREADPOOL_STATIC(name, n_threads, queue_size)\
    struct threadpool name = {};\
    static struct lgk_thread name##_thread_buffer[n_threads] = {};\
    static struct threadpool_work name##_work_queue_buffer[queue_size] = {}

typedef void threadpool_work_done_callback(void *work_data, int result);

struct threadpool_work
{
    thrd_start_t start;
    threadpool_work_done_callback *done_callback;
    void *data;
};

FIFO_STRUCT(fifo_threadpool_work, struct threadpool_work, unsigned);
QUEUE_STRUCT(fifoq_threadpool_work, fifo_threadpool_work);

struct threadpool
{
    unsigned n_threads;
    unsigned queue_size;
    struct lgk_monitor monitor;
    struct lgk_thread *thread_buffer;
    struct fifoq_threadpool_work work_queue;
    int queue_timeout_ms;
};

int threadpool_init(struct threadpool *tp, struct lgk_thread *thread_buffer, struct threadpool_work *queue_buffer, unsigned n_threads, unsigned queue_size, int_fast8_t timed_join, int queue_timeout_ms);
int threadpool_close(struct threadpool *tp, int join_timeout_ms, int_fast8_t timeout_detach);
int threadpool_schedule_work(struct threadpool *tp, thrd_start_t start, threadpool_work_done_callback *work_done_cb, void *work_data);

#endif
