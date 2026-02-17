/*
 * Tests for threadpool_init, threadpool_close, threadpool_schedule_work (threadpool.c).
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include <assert.h>
#include <stdint.h>
#include <threads.h>

#include <lgk_threadpool.h>

#define N_THREADS  256u
#define QUEUE_SIZE 8192u
#define QUEUE_TIMEOUT_MS 5000

THREADPOOL_STATIC(tp, N_THREADS, QUEUE_SIZE);

static int work_start_return_value(void *arg)
{
    return (int)(uintptr_t)arg;
}

/* For tests that need to pass both result slot and return value. */
struct work_with_slot {
    int *result_slot;
    int return_value;
};

static int work_start_with_slot(void *arg)
{
    struct work_with_slot *w = (struct work_with_slot *)arg;
    return w->return_value;
}

static void work_done_callback(void *work_data, int result)
{
    int *slot = (int *)work_data;
    *slot = result;
}

static void work_done_callback_with_slot(void *work_data, int result)
{
    struct work_with_slot *w = (struct work_with_slot *)work_data;
    *w->result_slot = result;
}

/* For multiple jobs: work_data is (void*)(uintptr_t)index; start returns index; done stores in static array. */
static int multi_results[QUEUE_SIZE*N_THREADS]; /* large enough for N_JOBS */

static int work_start_return_index(void *arg)
{
    return (int)(uintptr_t)arg;
}

static void work_done_store_by_index(void *work_data, int result)
{
    unsigned idx = (unsigned)(uintptr_t)work_data;
    if (idx < sizeof multi_results / sizeof multi_results[0])
        multi_results[idx] = result;
}

static void test_threadpool_init_close_empty(void)
{
    int status = threadpool_init(&tp, &tp_buffer_info, N_THREADS, QUEUE_SIZE, 1, QUEUE_TIMEOUT_MS);
    assert(status == thrd_success);

    status = threadpool_close(&tp, QUEUE_TIMEOUT_MS, 1);
    assert(status == thrd_success);
}

static void test_threadpool_schedule_single(void)
{
    int status = threadpool_init(&tp, &tp_buffer_info, N_THREADS, QUEUE_SIZE, 1, QUEUE_TIMEOUT_MS);
    assert(status == thrd_success);

    int result_slot = -1;
    struct work_with_slot w = { &result_slot, 42 };
    status = threadpool_schedule_work(&tp, work_start_with_slot, work_done_callback_with_slot, &w);
    assert(status == thrd_success);

    status = threadpool_close(&tp, QUEUE_TIMEOUT_MS, 1);
    assert(status == thrd_success);

    assert(result_slot == 42);
}

static void test_threadpool_schedule_multiple(void)
{
    int status = threadpool_init(&tp, &tp_buffer_info, N_THREADS, QUEUE_SIZE, 1, QUEUE_TIMEOUT_MS);
    assert(status == thrd_success);

    enum { N_JOBS = 16 };
    for (int i = 0; i < N_JOBS; i++)
        multi_results[i] = -1;

    for (int i = 0; i < N_JOBS; i++) {
        status = threadpool_schedule_work(&tp, work_start_return_index, work_done_store_by_index, (void *)(uintptr_t)i);
        assert(status == thrd_success);
    }

    status = threadpool_close(&tp, QUEUE_TIMEOUT_MS, 1);
    assert(status == thrd_success);

    for (int i = 0; i < N_JOBS; i++)
        assert(multi_results[i] == i);
}

static void test_threadpool_null_init_returns_error(void)
{
    struct threadpool_buffer_info info = {
        tp_thread_buffer,
        tp_work_queue_buffer,
        tp_work_pool_buffer,
        tp_work_buffer
    };
    int status = threadpool_init(NULL, &info, N_THREADS, QUEUE_SIZE, 1, QUEUE_TIMEOUT_MS);
    assert(status == thrd_error);
}

static void test_threadpool_null_close_returns_error(void)
{
    int status = threadpool_close(NULL, QUEUE_TIMEOUT_MS, 1);
    assert(status == thrd_error);
}

static void test_threadpool_null_schedule_returns_error(void)
{
    int status = threadpool_schedule_work(NULL, work_start_return_value, work_done_callback, NULL);
    assert(status == thrd_error);
}

int main(void)
{
    test_threadpool_null_init_returns_error();
    test_threadpool_null_close_returns_error();
    test_threadpool_null_schedule_returns_error();
    test_threadpool_init_close_empty();
    test_threadpool_schedule_single();
    test_threadpool_schedule_multiple();
    return 0;
}

