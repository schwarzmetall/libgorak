/*
 * Tests for lgk_thread_create and lgk_thread_join (thread.c).
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include <assert.h>
#include <stdint.h>
#include <threads.h>

#include <lgk_threads.h>

#define TEST_RES_VALUE 42

static int thread_start_returns_value(void *arg)
{
    (void)arg;
    return TEST_RES_VALUE;
}

static int thread_start_returns_zero(void *arg)
{
    (void)arg;
    return 0;
}

static int thread_start_return_id(void *arg)
{
    return (int)(uintptr_t)arg;
}

static void test_create_null_thread_returns_error(void)
{
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    int status = lgk_thread_create(NULL, thread_start_returns_zero, NULL, &mon);
    assert(status == thrd_error);

    lgk_monitor_destroy(&mon);
}

static void test_join_null_thread_returns_error(void)
{
    int res = -1;
    int status = lgk_thread_join(NULL, &res, 0, 0);
    assert(status == thrd_error);
    assert(res == -1);
}

static void test_create_and_join_returns_result(void)
{
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_value, NULL, &mon);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, 5000, 0);
    assert(status == thrd_success);
    assert(res == TEST_RES_VALUE);

    lgk_monitor_destroy(&mon);
}

static void test_create_timeout_zero_join_instant_or_timedout(void)
{
    /* timed monitor, join with 0 ms: returns thrd_success if thread already done else thrd_timedout. */
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_zero, NULL, &mon);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, 0, 0);
    /* Thread is quick; we may get success or timedout depending on scheduling. Both are valid. */
    assert(status == thrd_success || status == thrd_timedout);
    if (status == thrd_success)
        assert(res == 0);

    lgk_monitor_destroy(&mon);
}

static void test_create_timeout_negative_join_waits_indefinitely(void)
{
    /* untimed monitor: join with timeout_ms < 0 waits indefinitely until thread stops. */
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 0) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_zero, NULL, &mon);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, -1, 0);
    assert(status == thrd_success);
    assert(res == 0);

    lgk_monitor_destroy(&mon);
}

#define N_MANY_THREADS 512

static void test_many_threads_create_and_join(void)
{
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 0) == thrd_success);

    static struct lgk_thread many_threads[N_MANY_THREADS];
    static int many_results[N_MANY_THREADS];

    for (unsigned i = 0; i < N_MANY_THREADS; i++) {
        int status = lgk_thread_create(&many_threads[i], thread_start_return_id, (void *)(uintptr_t)i, &mon);
        assert(status == thrd_success);
    }

    for (unsigned i = 0; i < N_MANY_THREADS; i++) {
        many_results[i] = -1;
        int status = lgk_thread_join(&many_threads[i], &many_results[i], -1, 0);
        assert(status == thrd_success);
        assert(many_results[i] == (int)i);
    }

    lgk_monitor_destroy(&mon);
}

int main(void)
{
    test_create_null_thread_returns_error();
    test_join_null_thread_returns_error();
    test_create_and_join_returns_result();
    test_create_timeout_zero_join_instant_or_timedout();
    test_create_timeout_negative_join_waits_indefinitely();
    test_many_threads_create_and_join();
    return 0;
}
