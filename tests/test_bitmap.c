/*
 * Tests for lgk_bitmap (bitmap.c): init, get, set, alloc, free.
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include <assert.h>
#include <stdint.h>

#include <stddef.h>
#include <lgk/bitmap.h>

#define BITS 24

static void test_init_all_zero(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4];
    int_fast8_t s = lgk_bitmap_init(&bm, buf, BITS, 0);
    assert(s == 0);
    assert(bm.map == buf);
    assert(bm.size == BITS);
    for (unsigned i = 0; i < BITS; i++) {
        int_fast8_t v = lgk_bitmap_get(&bm, i);
        assert(v == 0);
    }
}

static void test_init_all_one(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4];
    int_fast8_t s = lgk_bitmap_init(&bm, buf, BITS, 1);
    assert(s == 0);
    for (unsigned i = 0; i < BITS; i++) {
        int_fast8_t v = lgk_bitmap_get(&bm, i);
        assert(v == 1);
    }
}

static void test_init_dont_touch(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4] = { 0x55, 0xaa, 0x0f, 0xf0 };
    int_fast8_t s = lgk_bitmap_init(&bm, buf, BITS, -1);
    assert(s == 0);
    assert(bm.map[0] == 0x55);
    assert(bm.map[1] == 0xaa);
    assert(bm.map[2] == 0x0f);
    assert(bm.map[3] == 0xf0);
}

static void test_init_size_zero(void)
{
    struct lgk_bitmap bm;
    int_fast8_t s = lgk_bitmap_init(&bm, NULL, 0, 0);
    assert(s == 0);
    assert(bm.map == NULL);
    assert(bm.size == 0);
}

static void test_set_get(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4];
    lgk_bitmap_init(&bm, buf, BITS, 0);

    for (unsigned i = 0; i < BITS; i++) {
        int_fast8_t set_ret = lgk_bitmap_set(&bm, i, 1);
        assert(set_ret == 1);
        assert(lgk_bitmap_get(&bm, i) == 1);
    }
    for (unsigned i = 0; i < BITS; i++) {
        int_fast8_t set_ret = lgk_bitmap_set(&bm, i, 0);
        assert(set_ret == 0);
        assert(lgk_bitmap_get(&bm, i) == 0);
    }
}

static void test_get_out_of_bounds_returns_error(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4];
    lgk_bitmap_init(&bm, buf, BITS, 0);
    assert(lgk_bitmap_get(&bm, BITS) == -1);
    assert(lgk_bitmap_get(&bm, BITS + 1) == -1);
}

static void test_set_out_of_bounds_returns_error(void)
{
    struct lgk_bitmap bm;
    lgk_bitmap_t buf[4];
    lgk_bitmap_init(&bm, buf, BITS, 0);
    assert(lgk_bitmap_set(&bm, BITS, 1) == -1);
    assert(lgk_bitmap_set(&bm, BITS + 1, 0) == -1);
}

static void test_get_zero_size_returns_error(void)
{
    struct lgk_bitmap bm;
    bm.map = NULL;
    bm.size = 0;
    assert(lgk_bitmap_get(&bm, 0) == -1);
}

static void test_alloc_free(void)
{
    struct lgk_bitmap *bm = lgk_bitmap_alloc(BITS, 0);
    assert(bm != NULL);
    assert(bm->map != NULL);
    assert(bm->size == BITS);
    for (unsigned i = 0; i < BITS; i++)
        assert(lgk_bitmap_get(bm, i) == 0);
    lgk_bitmap_set(bm, 0, 1);
    lgk_bitmap_set(bm, BITS - 1, 1);
    assert(lgk_bitmap_get(bm, 0) == 1);
    assert(lgk_bitmap_get(bm, BITS - 1) == 1);
    lgk_bitmap_free(bm);
}

static void test_alloc_zero_size_free(void)
{
    struct lgk_bitmap *bm = lgk_bitmap_alloc(0, 0);
    assert(bm != NULL);
    assert(bm->size == 0);
    /* map may be NULL when size is 0 */
    lgk_bitmap_free(bm);
}

int main(void)
{
    test_init_all_zero();
    test_init_all_one();
    test_init_dont_touch();
    test_init_size_zero();
    test_set_get();
    test_get_out_of_bounds_returns_error();
    test_set_out_of_bounds_returns_error();
    test_get_zero_size_returns_error();
    test_alloc_free();
    test_alloc_zero_size_free();
    return 0;
}
