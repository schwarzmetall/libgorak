/*
 * Tests for the <lgk/atomic_bitmap.h> macro family: the five init modes, get/set, the _explicit
 * order validation and the tail invariant.
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include "test.h"
#include <stdint.h>

#include <stddef.h>
#include <stdlib.h>
#include <lgk/bitmap_atomic.h>
#include <lgk/atomic_lockfree_intmax.h>
#include <lgk/util.h>

/* Not a multiple of either word width, so both instantiations have a partial final word. */
#define BITS 70

/* The word the target is actually good at, where it has one; otherwise the narrowest thing that
 * certainly exists. This is the instantiation pattern the header documents. */
#ifdef LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH
BITMAP_ATOMIC_STRUCT(test_abm, lgk_atomic_widest_lockfree_uint);
#else
BITMAP_ATOMIC_STRUCT(test_abm, _Atomic(unsigned char));
#endif
static BITMAP_ATOMIC_INIT(test_abm)
static BITMAP_ATOMIC_GET_EXPLICIT(test_abm)
static BITMAP_ATOMIC_GET(test_abm)
static BITMAP_ATOMIC_SET_EXPLICIT(test_abm)
static BITMAP_ATOMIC_SET(test_abm)

/* fixed-width instantiation, so the expected words are known exactly regardless of target */
BITMAP_ATOMIC_STRUCT(test_abm16, _Atomic(uint16_t));
static BITMAP_ATOMIC_INIT(test_abm16)
static BITMAP_ATOMIC_FILL(test_abm16)
static BITMAP_ATOMIC_GET(test_abm16)
static BITMAP_ATOMIC_SET(test_abm16)

static_assert(BITMAP_WORD_BITWIDTH(test_abm16) == 16);
static_assert(BITMAP_NWORDS(test_abm16, BITS) == 5);
/* the stride is sizeof the _Atomic element type, which may exceed sizeof the base */
static_assert(BITMAP_NBYTES(test_abm16, BITS) == 5 * sizeof(_Atomic(uint16_t)));
/* the instantiated word really is atomic, and UWIDTH sees through _Atomic to the base */
static_assert(IS_ATOMIC_TYPE(BITMAP_WORD_TYPE(test_abm16)));
static_assert(IS_ATOMIC_TYPE(BITMAP_WORD_TYPE(test_abm)));

/* static storage needs no atomic_init, so it is the legitimate ADOPT / STORE_* case */
static BITMAP_WORD_TYPE(test_abm16) g_static_store[BITMAP_NWORDS(test_abm16, BITS)];

#define NWORDS16 BITMAP_NWORDS(test_abm16, BITS)
#define REM16 (BITS % 16)
#define TAIL16 BITMAP_TAIL_MASK(test_abm16, BITS)

#define NWORDS BITMAP_NWORDS(test_abm, BITS)
#define REM (BITS % BITMAP_WORD_BITWIDTH(test_abm))

static void test_init_clear(void)
{
    struct test_abm bm;
    BITMAP_WORD_TYPE(test_abm) buf[NWORDS];
    int_fast8_t s = test_abm_init(&bm, buf, BITS, 0);
    test_assert(s == 0);
    test_assert(bm.map == buf);
    test_assert(bm.size == BITS);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm_get(&bm, i) == 0);
}

static void test_init_set_masks_tail(void)
{
    struct test_abm16 bm;
    BITMAP_WORD_TYPE(test_abm16) buf[NWORDS16];
    test_assert(test_abm16_init(&bm, buf, BITS, 1) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == 1);
    for (unsigned i_word = 0; i_word + 1 < NWORDS16; i_word++)
        test_assert(atomic_load(&buf[i_word]) == 0xffff);
    test_assert(atomic_load(&buf[NWORDS16 - 1]) == TAIL16);
    test_assert(TAIL16 == 0x3f);   /* 70 = 4*16 + 6 */
}

static void test_store_modes(void)
{
    struct test_abm16 bm;
    /* STORE_* requires already-initialized storage; static storage qualifies without atomic_init */
    test_assert(test_abm16_fill(&bm, g_static_store, BITS, 1) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == 1);
    test_assert(atomic_load(&g_static_store[NWORDS16 - 1]) == TAIL16);

    test_assert(test_abm16_fill(&bm, g_static_store, BITS, 0) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == 0);
    for (unsigned i_word = 0; i_word < NWORDS16; i_word++)
        test_assert(atomic_load(&g_static_store[i_word]) == 0);
}

static void test_adopt_keeps_data_and_clears_tail(void)
{
    struct test_abm16 bm;
    BITMAP_WORD_TYPE(test_abm16) buf[NWORDS16];
    /* bring the storage to life first, then dirty every bit including the tail */
    test_assert(test_abm16_init(&bm, buf, BITS, -1) == 0);
    for (unsigned i_word = 0; i_word < NWORDS16; i_word++)
        atomic_store(&buf[i_word], 0xffff);

    test_assert(test_abm16_init(&bm, buf, BITS, -1) == 0);

    /* data bits survive ... */
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == 1);
    for (unsigned i_word = 0; i_word + 1 < NWORDS16; i_word++)
        test_assert(atomic_load(&buf[i_word]) == 0xffff);
    /* ... and the tail is cleared anyway */
    static_assert(REM16 != 0, "BITS must not be a whole multiple of 16");
    test_assert(atomic_load(&buf[NWORDS16 - 1]) == TAIL16);
    test_assert((atomic_load(&buf[NWORDS16 - 1]) >> REM16) == 0);
}

/* On a word boundary the tail fixup must be skipped entirely, not applied with an all-zero mask. */
static void test_adopt_word_multiple_keeps_full_word(void)
{
    struct test_abm16 bm;
    BITMAP_WORD_TYPE(test_abm16) buf[2];
    test_assert(test_abm16_init(&bm, buf, 32, 1) == 0);
    test_assert(atomic_load(&buf[1]) == 0xffff);

    test_assert(test_abm16_init(&bm, buf, 32, -1) == 0);
    test_assert(atomic_load(&buf[0]) == 0xffff);
    test_assert(atomic_load(&buf[1]) == 0xffff);
    test_assert(test_abm16_get(&bm, 31) == 1);
}

static void test_init_size_zero(void)
{
    struct test_abm bm;
    test_assert(test_abm_init(&bm, NULL, 0, 0) == 0);
    test_assert(bm.map == NULL);
    test_assert(bm.size == 0);
    test_assert(test_abm_get(&bm, 0) == -1);
}

static void test_init_null_map_nonzero_size_returns_error(void)
{
    struct test_abm bm;
    test_assert(test_abm_init(&bm, NULL, BITS, -1) == -1);
}

static void test_set_get(void)
{
    struct test_abm bm;
    BITMAP_WORD_TYPE(test_abm) buf[NWORDS];
    test_abm_init(&bm, buf, BITS, 0);

    for (unsigned i = 0; i < BITS; i++) {
        test_assert(test_abm_set(&bm, i, 1) == 1);
        test_assert(test_abm_get(&bm, i) == 1);
    }
    /* every bit set through set() must still leave the tail alone */
    test_assert((atomic_load(&buf[NWORDS - 1]) >> REM) == 0);

    for (unsigned i = 0; i < BITS; i++) {
        test_assert(test_abm_set(&bm, i, 0) == 0);
        test_assert(test_abm_get(&bm, i) == 0);
    }
}

/* fetch_and must not disturb the other bits of the same word */
static void test_set_isolates_neighbours(void)
{
    struct test_abm16 bm;
    BITMAP_WORD_TYPE(test_abm16) buf[NWORDS16];
    test_abm16_init(&bm, buf, BITS, 1);
    test_assert(test_abm16_set(&bm, 17, 0) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == (i == 17 ? 0 : 1));
    test_assert(atomic_load(&buf[1]) == (uint16_t)~(uint16_t)(1u << 1));
}

static void test_explicit_orders(void)
{
    struct test_abm bm;
    BITMAP_WORD_TYPE(test_abm) buf[NWORDS];
    test_abm_init(&bm, buf, BITS, 0);

    test_assert(test_abm_set_explicit(&bm, 3, 1, memory_order_relaxed) == 1);
    test_assert(test_abm_get_explicit(&bm, 3, memory_order_relaxed) == 1);
    test_assert(test_abm_get_explicit(&bm, 3, memory_order_acquire) == 1);
    test_assert(test_abm_get_explicit(&bm, 3, memory_order_consume) == 1);
    test_assert(test_abm_get_explicit(&bm, 3, memory_order_seq_cst) == 1);

    /* every order is valid for an RMW */
    test_assert(test_abm_set_explicit(&bm, 4, 1, memory_order_release) == 1);
    test_assert(test_abm_set_explicit(&bm, 5, 1, memory_order_acq_rel) == 1);
    test_assert(test_abm_get(&bm, 4) == 1);
    test_assert(test_abm_get(&bm, 5) == 1);
}

/* atomic_load_explicit with release or acq_rel is UB, so get_explicit rejects both */
static void test_get_explicit_rejects_invalid_orders(void)
{
    struct test_abm bm;
    BITMAP_WORD_TYPE(test_abm) buf[NWORDS];
    test_abm_init(&bm, buf, BITS, 1);
    test_assert(test_abm_get_explicit(&bm, 0, memory_order_release) == -1);
    test_assert(test_abm_get_explicit(&bm, 0, memory_order_acq_rel) == -1);
    /* and the valid orders still work on the same object */
    test_assert(test_abm_get_explicit(&bm, 0, memory_order_acquire) == 1);
}

static void test_out_of_bounds_returns_error(void)
{
    struct test_abm bm;
    BITMAP_WORD_TYPE(test_abm) buf[NWORDS];
    test_abm_init(&bm, buf, BITS, 0);
    test_assert(test_abm_get(&bm, BITS) == -1);
    test_assert(test_abm_get(&bm, BITS + 1) == -1);
    test_assert(test_abm_set(&bm, BITS, 1) == -1);
    test_assert(test_abm_set(&bm, BITS + 1, 0) == -1);
    test_assert(test_abm_get_explicit(&bm, BITS, memory_order_acquire) == -1);
    test_assert(test_abm_set_explicit(&bm, BITS, 1, memory_order_relaxed) == -1);
}

/* malloc'ed storage is allocated duration, so it needs an INIT_* mode */
static void test_heap_storage_needs_init_mode(void)
{
    BITMAP_WORD_TYPE(test_abm16) *store = malloc(BITMAP_NBYTES(test_abm16, BITS));
    test_assert(store != NULL);

    struct test_abm16 bm;
    test_assert(test_abm16_init(&bm, store, BITS, 0) == 0);
    for (unsigned i = 0; i < BITS; i++)
        test_assert(test_abm16_get(&bm, i) == 0);
    test_assert(test_abm16_set(&bm, BITS - 1, 1) == 1);
    test_assert(test_abm16_get(&bm, BITS - 1) == 1);
    test_assert(atomic_load(&store[NWORDS16 - 1]) == (uint16_t)(1u << (REM16 - 1)));

    free(store);
}

int main(void)
{
    test_init_clear();
    test_init_set_masks_tail();
    test_store_modes();
    test_adopt_keeps_data_and_clears_tail();
    test_adopt_word_multiple_keeps_full_word();
    test_init_size_zero();
    test_init_null_map_nonzero_size_returns_error();
    test_set_get();
    test_set_isolates_neighbours();
    test_explicit_orders();
    test_get_explicit_rejects_invalid_orders();
    test_out_of_bounds_returns_error();
    test_heap_storage_needs_init_mode();
    return 0;
}
