/*
 * Tests for timespec_offset_ms and timespec_get_offset_ms (timespec_ms.c).
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include <assert.h>
#include <time.h>

#include <lgk_timespec.h>

#define NSEC_PER_SEC (1000*1000*1000)

static void test_timespec_offset_ms_zero(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 0);
    assert(ts.tv_sec == 0 && ts.tv_nsec == 0);
}

static void test_timespec_offset_ms_positive_ms_only(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 500);
    assert(ts.tv_sec == 0 && ts.tv_nsec == 500*1000*1000);
}

static void test_timespec_offset_ms_positive_seconds_only(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 5000);
    assert(ts.tv_sec == 5 && ts.tv_nsec == 0);
}

static void test_timespec_offset_ms_nsec_carry(void)
{
    struct timespec ts = { 0, 500*1000*1000 };
    timespec_offset_ms(&ts, 600);
    assert(ts.tv_sec == 1 && ts.tv_nsec == 100*1000*1000);
}

static void test_timespec_offset_ms_negative(void)
{
    struct timespec ts = { 2, 100*1000*1000 };
    timespec_offset_ms(&ts, -1500);
    assert(ts.tv_sec == 0 && ts.tv_nsec == 600*1000*1000);
}

static void test_timespec_offset_ms_negative_into_negative_nsec_normalized(void)
{
    /* 1 s - 1500 ms = -0.5 s -> tv_sec=-1, tv_nsec=500000000 */
    struct timespec ts = { 1, 0 };
    timespec_offset_ms(&ts, -1500);
    assert(ts.tv_sec == -1 && ts.tv_nsec == 500*1000*1000);
}

static void test_timespec_get_offset_ms_success(void)
{
    struct timespec ts = { 0, 0 };
    int status = timespec_get_offset_ms(&ts, TIME_UTC, 0);
    assert(status == TIME_UTC);
    assert(ts.tv_nsec >= 0 && ts.tv_nsec < NSEC_PER_SEC);
}

static void test_timespec_get_offset_ms_applies_offset(void)
{
    struct timespec ts_before = { 0, 0 };
    timespec_get_offset_ms(&ts_before, TIME_UTC, 0);

    struct timespec ts_after = { 0, 0 };
    int status = timespec_get_offset_ms(&ts_after, TIME_UTC, 2000);
    assert(status == TIME_UTC);
    assert(ts_after.tv_sec >= ts_before.tv_sec);
    assert(ts_after.tv_nsec >= 0 && ts_after.tv_nsec < NSEC_PER_SEC);
    /* ts_after should be roughly ts_before + 2 s (allow 1 s tolerance) */
    assert(ts_after.tv_sec >= ts_before.tv_sec + 1);
}

int main(void)
{
    test_timespec_offset_ms_zero();
    test_timespec_offset_ms_positive_ms_only();
    test_timespec_offset_ms_positive_seconds_only();
    test_timespec_offset_ms_nsec_carry();
    test_timespec_offset_ms_negative();
    test_timespec_offset_ms_negative_into_negative_nsec_normalized();
    test_timespec_get_offset_ms_success();
    test_timespec_get_offset_ms_applies_offset();
    return 0;
}
