/*
 * Tests for the <lgk/bitmap.h> macro family: init modes, get, set, the tail invariant and sizing.
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include "test.h"
#include <stdint.h>

#include <stddef.h>
#include <stdlib.h>
#include <lgk/bitmap.h>
#include <lgk/util.h>

/* BITS is deliberately NOT a multiple of either word width, so both instantiations have a partial
 * final word and the tail invariant is actually exercised. */
#define BITS 70

BITMAP_STRUCT(test_bitmap, unsigned);
static BITMAP_INIT(test_bitmap)
static BITMAP_GET(test_bitmap)
static BITMAP_SET(test_bitmap)

/* second, narrower instantiation: the same bit count lands on a different word boundary */
BITMAP_STRUCT(test_bitmap8, uint8_t);
static BITMAP_INIT(test_bitmap8)
static BITMAP_GET(test_bitmap8)
static BITMAP_SET(test_bitmap8)

static_assert(BITMAP_WORD_BITS(test_bitmap8) == 8);
static_assert(BITMAP_NWORDS(test_bitmap8, BITS) == 9);
static_assert(BITMAP_NBYTES(test_bitmap8, BITS) == 9);
static_assert(BITMAP_NWORDS(test_bitmap8, 8) == 1);
static_assert(BITMAP_NWORDS(test_bitmap8, 0) == 0);
/* sizing is driven by the word's value width, not by a hard-coded 8 */
static_assert(BITMAP_NWORDS(test_bitmap, BITS) == INTDIVCEIL(BITS, BITMAP_WORD_BITS(test_bitmap)));

/* a static array sized by the macros is a valid, already-cleared bitmap */
static BITMAP_WORD(test_bitmap8) g_static_store[BITMAP_NWORDS(test_bitmap8, BITS)];

#define NWORDS BITMAP_NWORDS(test_bitmap, BITS)
#define REM (BITS % BITMAP_WORD_BITS(test_bitmap))

static void test_init_clear(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    int_fast8_t s = test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_CLEAR);
    test_assert(s == 0);
    test_assert(bm.map == buf);
    test_assert(bm.size == BITS);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_get(&bm, i) == 0);
}

static void test_init_set(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    int_fast8_t s = test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_SET);
    test_assert(s == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_get(&bm, i) == 1);
    /* interior words are all ones ... */
    for (unsigned i_word = 0; i_word + 1 < NWORDS; i_word++)
        test_assert(buf[i_word] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);
    /* ... and the final word carries only the data bits */
    test_assert(buf[NWORDS - 1] == BITMAP_TAIL_MASK(BITMAP_WORD(test_bitmap), BITS));
}

/* Replaces the old test_init_dont_touch, which used a bit count that was a whole multiple of its
 * word and asserted against a word the bitmap did not even own, so it never exercised a tail. */
static void test_init_adopt(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    for (unsigned i_word = 0; i_word < NWORDS; i_word++)
        buf[i_word] = (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0;

    int_fast8_t s = test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_ADOPT);
    test_assert(s == 0);

    /* data bits survive */
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_get(&bm, i) == 1);
    for (unsigned i_word = 0; i_word + 1 < NWORDS; i_word++)
        test_assert(buf[i_word] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);

    /* ADOPT still enforces the invariant: the unused bits of the last owned word are zero */
    static_assert(REM != 0, "BITS must not be a whole multiple of the word width");
    test_assert(buf[NWORDS - 1] == BITMAP_TAIL_MASK(BITMAP_WORD(test_bitmap), BITS));
    test_assert((buf[NWORDS - 1] >> REM) == 0);
}

/* A size that IS a whole multiple of the word must keep its full final word: the tail fixup has to
 * be skipped, not applied with an all-zero mask. */
static void test_init_adopt_word_multiple(void)
{
    const unsigned bits = 2 * BITMAP_WORD_BITS(test_bitmap);
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[2];
    buf[0] = (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0;
    buf[1] = (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0;

    int_fast8_t s = test_bitmap_init(&bm, buf, bits, LGK_BITMAP_ADOPT);
    test_assert(s == 0);
    test_assert(buf[0] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);
    test_assert(buf[1] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);
    test_assert(test_bitmap_get(&bm, bits - 1) == 1);
}

static void test_init_set_word_multiple(void)
{
    const unsigned bits = 2 * BITMAP_WORD_BITS(test_bitmap);
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[2];
    test_assert(test_bitmap_init(&bm, buf, bits, LGK_BITMAP_SET) == 0);
    test_assert(buf[0] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);
    test_assert(buf[1] == (BITMAP_WORD(test_bitmap))~(BITMAP_WORD(test_bitmap))0);
}

static void test_init_size_zero(void)
{
    struct test_bitmap bm;
    int_fast8_t s = test_bitmap_init(&bm, NULL, 0, LGK_BITMAP_CLEAR);
    test_assert(s == 0);
    test_assert(bm.map == NULL);
    test_assert(bm.size == 0);
}

static void test_init_null_map_nonzero_size_returns_error(void)
{
    struct test_bitmap bm;
    test_assert(test_bitmap_init(&bm, NULL, BITS, LGK_BITMAP_CLEAR) == -1);
}

static void test_init_bad_mode_returns_error(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_assert(test_bitmap_init(&bm, buf, BITS, (enum lgk_bitmap_init)(LGK_BITMAP_ADOPT + 1)) == -1);
    test_assert(test_bitmap_init(&bm, buf, BITS, (enum lgk_bitmap_init)-1) == -1);
}

static void test_set_get(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_CLEAR);

    for (unsigned i = 0; i < BITS; i++) {
        test_assert(test_bitmap_set(&bm, i, 1) == 1);
        test_assert(test_bitmap_get(&bm, i) == 1);
    }
    for (unsigned i = 0; i < BITS; i++) {
        test_assert(test_bitmap_set(&bm, i, 0) == 0);
        test_assert(test_bitmap_get(&bm, i) == 0);
    }
}

/* setting every bit through set() must not leak into the tail either */
static void test_set_preserves_tail(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_CLEAR);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_set(&bm, i, 1) == 1);
    test_assert((buf[NWORDS - 1] >> REM) == 0);
}

static void test_set_isolates_neighbours(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_SET);
    test_assert(test_bitmap_set(&bm, 33, 0) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_get(&bm, i) == (i == 33 ? 0 : 1));
}

static void test_get_out_of_bounds_returns_error(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_CLEAR);
    test_assert(test_bitmap_get(&bm, BITS) == -1);
    test_assert(test_bitmap_get(&bm, BITS + 1) == -1);
}

static void test_set_out_of_bounds_returns_error(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, BITS, LGK_BITMAP_CLEAR);
    test_assert(test_bitmap_set(&bm, BITS, 1) == -1);
    test_assert(test_bitmap_set(&bm, BITS + 1, 0) == -1);
}

static void test_get_zero_size_returns_error(void)
{
    struct test_bitmap bm;
    BITMAP_WORD(test_bitmap) buf[NWORDS];
    test_bitmap_init(&bm, buf, 0, LGK_BITMAP_CLEAR);
    test_assert(test_bitmap_get(&bm, 0) == -1);
}

static void test_narrow_word(void)
{
    struct test_bitmap8 bm;
    BITMAP_WORD(test_bitmap8) buf[BITMAP_NWORDS(test_bitmap8, BITS)];
    test_assert(test_bitmap8_init(&bm, buf, BITS, LGK_BITMAP_SET) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap8_get(&bm, i) == 1);
    /* 70 bits over 8-bit words: 8 full words plus 6 data bits in the ninth */
    test_assert(buf[8] == 0x3f);
    test_assert(test_bitmap8_get(&bm, BITS) == -1);

    /* the shift in set() is on the instantiated word, so the top bit of a narrow word is reachable
     * without the UB the old bare `1 <<` would have had once the word reached int's width */
    test_assert(test_bitmap8_set(&bm, 7, 0) == 0);
    test_assert(buf[0] == 0x7f);
    test_assert(test_bitmap8_set(&bm, 7, 1) == 1);
    test_assert(buf[0] == 0xff);
    test_assert(test_bitmap8_set(&bm, BITS, 1) == -1);
}

static void test_static_storage_is_valid_cleared_bitmap(void)
{
    struct test_bitmap8 bm;
    test_assert(test_bitmap8_init(&bm, g_static_store, BITS, LGK_BITMAP_ADOPT) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap8_get(&bm, i) == 0);
}

/* Replaces test_alloc_free / test_alloc_zero_size_free: there is no allocating constructor any
 * more, so the caller sizes its own storage with BITMAP_NBYTES. */
static void test_heap_storage(void)
{
    BITMAP_WORD(test_bitmap) *store = malloc(BITMAP_NBYTES(test_bitmap, BITS));
    test_assert(store != NULL);

    struct test_bitmap bm;
    test_assert(test_bitmap_init(&bm, store, BITS, LGK_BITMAP_CLEAR) == 0);
    test_assert(bm.size == BITS);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_bitmap_get(&bm, i) == 0);

    test_assert(test_bitmap_set(&bm, 0, 1) == 1);
    test_assert(test_bitmap_set(&bm, BITS - 1, 1) == 1);
    test_assert(test_bitmap_get(&bm, 0) == 1);
    test_assert(test_bitmap_get(&bm, BITS - 1) == 1);
    test_assert((store[NWORDS - 1] >> REM) == 0);

    free(store);
}

int main(void)
{
    test_init_clear();
    test_init_set();
    test_init_adopt();
    test_init_adopt_word_multiple();
    test_init_set_word_multiple();
    test_init_size_zero();
    test_init_null_map_nonzero_size_returns_error();
    test_init_bad_mode_returns_error();
    test_set_get();
    test_set_preserves_tail();
    test_set_isolates_neighbours();
    test_get_out_of_bounds_returns_error();
    test_set_out_of_bounds_returns_error();
    test_get_zero_size_returns_error();
    test_narrow_word();
    test_static_storage_is_valid_cleared_bitmap();
    test_heap_storage();
    return 0;
}
