/*
 * Tests for time_ms.c.
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include "test.h"
#include <time.h>

#include <lgk/time_ms.h>

#define NSEC_PER_SEC (1000*1000*1000)

static void test_timespec_offset_ms_zero(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 0);
    test_assert(ts.tv_sec == 0 && ts.tv_nsec == 0);
}

static void test_timespec_offset_ms_positive_ms_only(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 500);
    test_assert(ts.tv_sec == 0 && ts.tv_nsec == 500*1000*1000);
}

static void test_timespec_offset_ms_positive_seconds_only(void)
{
    struct timespec ts = { 0, 0 };
    timespec_offset_ms(&ts, 5000);
    test_assert(ts.tv_sec == 5 && ts.tv_nsec == 0);
}

static void test_timespec_offset_ms_nsec_carry(void)
{
    struct timespec ts = { 0, 500*1000*1000 };
    timespec_offset_ms(&ts, 600);
    test_assert(ts.tv_sec == 1 && ts.tv_nsec == 100*1000*1000);
}

static void test_timespec_offset_ms_negative(void)
{
    struct timespec ts = { 2, 100*1000*1000 };
    timespec_offset_ms(&ts, -1500);
    test_assert(ts.tv_sec == 0 && ts.tv_nsec == 600*1000*1000);
}

static void test_timespec_offset_ms_negative_into_negative_nsec_normalized(void)
{
    /* 1 s - 1500 ms = -0.5 s -> tv_sec=-1, tv_nsec=500000000 */
    struct timespec ts = { 1, 0 };
    timespec_offset_ms(&ts, -1500);
    test_assert(ts.tv_sec == -1 && ts.tv_nsec == 500*1000*1000);
}

static void test_timespec_get_offset_ms_success(void)
{
    struct timespec ts = { 0, 0 };
    int status = timespec_get_offset_ms(&ts, TIME_UTC, 0);
    test_assert(!status);
    test_assert(ts.tv_nsec >= 0 && ts.tv_nsec < NSEC_PER_SEC);
}

static void test_timespec_get_offset_ms_applies_offset(void)
{
    struct timespec ts_before = { 0, 0 };
    timespec_get_offset_ms(&ts_before, TIME_UTC, 0);

    struct timespec ts_after = { 0, 0 };
    int status = timespec_get_offset_ms(&ts_after, TIME_UTC, 2000);
    test_assert(!status);
    test_assert(ts_after.tv_sec >= ts_before.tv_sec);
    test_assert(ts_after.tv_nsec >= 0 && ts_after.tv_nsec < NSEC_PER_SEC);
    /* ts_after should be roughly ts_before + 2 s (allow 1 s tolerance) */
    test_assert(ts_after.tv_sec >= ts_before.tv_sec + 1);
}

static void test_timespec_to_ms_basic(void)
{
    struct timespec ts = { 2, 500*1000*1000 };
    test_assert(timespec_to_ms(&ts) == 2500);
}

static void test_timespec_to_ms_sub_ms_truncates(void)
{
    struct timespec ts = { 0, 999*1000 };
    test_assert(timespec_to_ms(&ts) == 0);
}

static void test_time_ms_success(void)
{
    int_fast64_t ms_before = 0;
    int status = time_ms(TIME_UTC, &ms_before);
    test_assert(!status);
    test_assert(ms_before > 0);
}

static void test_time_offset_ms_zero_offset(void)
{
    int_fast64_t out = 0;
    int status = time_offset_ms(TIME_UTC, 0, &out);
    test_assert(!status);
    test_assert(out > 0);
}

static void test_time_offset_ms_positive_offset(void)
{
    int_fast64_t base = 0;
    int_fast64_t plus5 = 0;
    time_offset_ms(TIME_UTC, 0, &base);
    time_offset_ms(TIME_UTC, 5000, &plus5);
    test_assert(plus5 - base >= 5000);
    test_assert(plus5 - base < 5050);
}

static void test_time_offset_ms_negative_offset(void)
{
    int_fast64_t base = 0;
    int_fast64_t minus2 = 0;
    time_offset_ms(TIME_UTC, 0, &base);
    time_offset_ms(TIME_UTC, -2000, &minus2);
    test_assert(base - minus2 >= 2000);
    test_assert(base - minus2 < 2050);
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
    test_timespec_to_ms_basic();
    test_timespec_to_ms_sub_ms_truncates();
    test_time_ms_success();
    test_time_offset_ms_zero_offset();
    test_time_offset_ms_positive_offset();
    test_time_offset_ms_negative_offset();
    return 0;
}
