#ifndef LGK_THREADPOOL_H
#define LGK_THREADPOOL_H

#include <stdint.h>
#include <threads.h>
#include <lgk_queue_int.h>

#warning "threadpool should not be used. Needs to be fixed first."

#define THREADPOOL_FLAG_UNTIMED 0x01

#define THREADPOOL_STATIC(name, thread_buffer_size, queue_buffer_size)\
    struct threadpool name = { 0 };\
    static thrd_t name##_thread_buffer[thread_buffer_size] = { 0 };\
    static int name##_work_queue_buffer[queue_buffer_size] = { 0 };\
    static int name##_work_pool_buffer[queue_buffer_size] = { 0 };\
    static struct threadpool_work name##_work_buffer[queue_buffer_size] = { 0 };\
    static const struct threadpool_buffer_info name##_buffer_info =\
    {\
        name##_thread_buffer,\
        name##_work_queue_buffer,\
        name##_work_pool_buffer,\
        name##_work_buffer\
    }

typedef void threadpool_work_done_callback(void *work_data, int result);

struct threadpool_buffer_info
{
    thrd_t *thread_buffer;
    int *work_queue_buffer;
    int *work_pool_buffer;
    struct threadpool_work *work_buffer;
};

struct threadpool
{
    unsigned n_threads;
    thrd_t *thread_buffer;
    struct threadpool_work *work_buffer;
    struct queue_int work_queue;
    struct queue_int work_pool;
    uint_fast8_t close;
    unsigned timeout_ms;
};

struct threadpool_work
{
    thrd_start_t start;
    threadpool_work_done_callback *done_callback;
    void *data;
};

int threadpool_init(struct threadpool *tp, const struct threadpool_buffer_info *buffer_info, unsigned n_threads, unsigned queue_size, uint_fast8_t flags, unsigned timeout_ms);
int threadpool_close(struct threadpool *tp);
int threadpool_schedule_work(struct threadpool *tp, thrd_start_t start, threadpool_work_done_callback *work_done_cb, void *work_data);

#endif
