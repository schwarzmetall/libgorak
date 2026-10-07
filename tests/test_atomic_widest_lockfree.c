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
#include <lgk/atomic_widest_lockfree.h>
#include <lgk/util.h>

#ifdef LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH

/* the width macro cannot drift from the typedef: it is the selected type's own *_WIDTH */
static_assert(UWIDTH(lgk_widest_lockfree_uint) == LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH);

/* the atomic typedef is _Atomic of exactly the base typedef */
static_assert(IS_ATOMIC_TYPE(lgk_atomic_widest_lockfree_uint));
static_assert(!IS_ATOMIC_TYPE(lgk_widest_lockfree_uint));
static_assert(UWIDTH(lgk_atomic_widest_lockfree_uint) == LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH);

/* "widest": nothing wider than the selection may be always-lock-free */
#if LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH < ULLONG_WIDTH
static_assert(ATOMIC_LLONG_LOCK_FREE != 2);
#endif

/* usable in arithmetic in #if, not just #ifdef */
#if LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH < UCHAR_WIDTH
#  error "selected width is narrower than unsigned char"
#endif

static void test_is_lock_free_at_runtime(void)
{
    /* The compile-time claim is "always lock-free", so this must hold for an actual object. This is
     * the authoritative per-object answer the header points callers at. */
    lgk_atomic_widest_lockfree_uint obj;
    atomic_init(&obj, 0);
    test_assert(atomic_is_lock_free(&obj));
}

static void test_object_behaves(void)
{
    lgk_atomic_widest_lockfree_uint obj;
    atomic_init(&obj, 0);

    const lgk_widest_lockfree_uint top = (lgk_widest_lockfree_uint)1 << (LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH - 1);
    test_assert(atomic_fetch_or(&obj, top) == 0);
    test_assert(atomic_load(&obj) == top);
    test_assert(atomic_fetch_and(&obj, (lgk_widest_lockfree_uint)~top) == top);
    test_assert(atomic_load(&obj) == 0);
}

int main(void)
{
    test_is_lock_free_at_runtime();
    test_object_behaves();
    return 0;
}

#else /* no always-lock-free atomic integer type on this target */

/* The header promises to define NOTHING in this case - no typedef, no macro - so that the
 * instantiator is forced to make the decision explicitly. An undefined macro is 0 in #if, so a
 * width comparison is false rather than an error. */
#if LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH >= 8
#  error "LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH must not be usable when no width qualifies"
#endif

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
