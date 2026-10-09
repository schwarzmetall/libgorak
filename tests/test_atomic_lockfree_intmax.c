/*
 * Tests for <lgk/atomic_widest_lockfree.h>: the typedefs and the width macro agree, the selected
 * type really is atomic, and it really is lock-free at run time.
 *
 * The header defines nothing when the target has no always-lock-free atomic integer type, so every
 * check is conditional on the width macro. On such a target this test still builds and passes,
 * asserting only that the header kept its promise to define nothing.
 *
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include "test.h"

#include <stdatomic.h>
#include <limits.h>
#include <lgk/atomic_lockfree_intmax.h>
#include <lgk/util.h>

#ifdef ATOMIC_LOCKFREE_INTMAX_WIDTH

/* the width macro cannot drift from the typedef: it is the selected type's own *_WIDTH */
static_assert(BITWIDTH(atomic_lockfree_intmax_t) == ATOMIC_LOCKFREE_INTMAX_WIDTH);
static_assert(BITWIDTH(atomic_lockfree_uintmax_t) == ATOMIC_LOCKFREE_UINTMAX_WIDTH);

static_assert(IS_ATOMIC_TYPE(atomic_lockfree_intmax_t));
static_assert(IS_ATOMIC_TYPE(atomic_lockfree_uintmax_t));

/* "widest": nothing wider than the selection may be always-lock-free */
#if ATOMIC_LOCKFREE_INTMAX_WIDTH < LLONG_WIDTH
static_assert(ATOMIC_LLONG_LOCK_FREE != 2);
#endif

#if ATOMIC_LOCKFREE_INTMAX_WIDTH < LONG_WIDTH
static_assert(ATOMIC_LONG_LOCK_FREE != 2);
#endif

#if ATOMIC_LOCKFREE_INTMAX_WIDTH < INT_WIDTH
static_assert(ATOMIC_INT_LOCK_FREE != 2);
#endif

#if ATOMIC_LOCKFREE_INTMAX_WIDTH < SHRT_WIDTH
static_assert(ATOMIC_SHORT_LOCK_FREE != 2);
#endif

/* usable in arithmetic in #if, not just #ifdef */
#if ATOMIC_LOCKFREE_INTMAX_WIDTH < UCHAR_WIDTH
#  error "selected width is narrower than unsigned char"
#endif

static void test_is_lock_free_at_runtime(void)
{
    /* The compile-time claim is "always lock-free", so this must hold for an actual object. This is
     * the authoritative per-object answer the header points callers at. */
    atomic_lockfree_uintmax_t obj;
    atomic_init(&obj, 0);
    test_assert(atomic_is_lock_free(&obj));
}

static void test_object_behaves(void)
{
    atomic_lockfree_uintmax_t obj;
    atomic_init(&obj, 0);

    const typeof_unqual(atomic_lockfree_uintmax_t) top = (typeof_unqual(atomic_lockfree_uintmax_t))1 << (ATOMIC_LOCKFREE_INTMAX_WIDTH - 1);
    test_assert(atomic_fetch_or(&obj, top) == 0);
    test_assert(atomic_load(&obj) == top);
    test_assert(atomic_fetch_and(&obj, ~top) == top);
    test_assert(atomic_load(&obj) == 0);
}

int main(void)
{
    test_is_lock_free_at_runtime();
    test_object_behaves();
    return 0;
}

#else /* no always-lock-free atomic integer type on this target */

static_assert(ATOMIC_CHAR_LOCK_FREE != 2);
static_assert(ATOMIC_SHORT_LOCK_FREE != 2);
static_assert(ATOMIC_INT_LOCK_FREE != 2);
static_assert(ATOMIC_LONG_LOCK_FREE != 2);
static_assert(ATOMIC_LLONG_LOCK_FREE != 2);

int main(void)
{
    return 0;
}

#endif
