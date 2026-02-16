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

static void test_create_null_thread_returns_error(void)
{
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    int status = lgk_thread_create(NULL, thread_start_returns_zero, NULL, &mon, 1000);
    assert(status == thrd_error);

    lgk_monitor_destroy(&mon);
}

static void test_join_null_thread_returns_error(void)
{
    int res = -1;
    int status = lgk_thread_join(NULL, &res, 0);
    assert(status == thrd_error);
    assert(res == -1);
}

static void test_create_and_join_returns_result(void)
{
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_value, NULL, &mon, 5000);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, 0);
    assert(status == thrd_success);
    assert(res == TEST_RES_VALUE);

    lgk_monitor_destroy(&mon);
}

static void test_create_timeout_zero_join_instant_or_timedout(void)
{
    /* timeout_ms 0 at create: join waits 0 ms, returns thrd_success if thread already done else thrd_timedout. */
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_zero, NULL, &mon, 0);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, 0);
    /* Thread is quick; we may get success or timedout depending on scheduling. Both are valid. */
    assert(status == thrd_success || status == thrd_timedout);
    if (status == thrd_success)
        assert(res == 0);

    lgk_monitor_destroy(&mon);
}

static void test_create_timeout_negative_join_waits_indefinitely(void)
{
    /* timeout_ms -1 at create: join waits indefinitely until thread stops. */
    struct lgk_monitor mon;
    assert(lgk_monitor_init(&mon, 1) == thrd_success);

    struct lgk_thread t;
    int status = lgk_thread_create(&t, thread_start_returns_zero, NULL, &mon, -1);
    assert(status == thrd_success);

    int res = -1;
    status = lgk_thread_join(&t, &res, 0);
    assert(status == thrd_success);
    assert(res == 0);

    lgk_monitor_destroy(&mon);
}

int main(void)
{
    test_create_null_thread_returns_error();
    test_join_null_thread_returns_error();
    test_create_and_join_returns_result();
    test_create_timeout_zero_join_instant_or_timedout();
    test_create_timeout_negative_join_waits_indefinitely();
    return 0;
}
