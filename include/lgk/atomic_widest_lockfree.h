#ifndef LGK_ATOMIC_WIDEST_LOCKFREE_H
#define LGK_ATOMIC_WIDEST_LOCKFREE_H

#include <limits.h>
#include <stdatomic.h>

// Widest ALWAYS-lock-free atomic integer type the target offers, if it offers one. General
// purpose: nothing here is bitmap-specific.
//
// Defines, iff some width qualifies:
//  - lgk_widest_lockfree_uint             the plain base type
//  - lgk_atomic_widest_lockfree_uint      _Atomic of it; what storage must be declared as
//  - LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH     its value width, i.e. the standard *_WIDTH macro of the
//                                         selected type, so it cannot drift from the typedef
//
// If no width qualifies, this header defines NOTHING - no typedef, no macro. There is deliberately
// no fallback: on such a target the header cannot know a good word, so the decision belongs to the
// instantiator, who knows the platform.
//
//      #include <lgk/atomic_widest_lockfree.h>
//
//      #ifdef LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH
//      ATOMIC_BITMAP_STRUCT(my_flags, lgk_atomic_widest_lockfree_uint);
//      #else
//      #  error "no always-lock-free atomic integer type on this target"
//      /* or, deliberately: ATOMIC_BITMAP_STRUCT(my_flags, _Atomic(unsigned char)); */
//      #endif
//
// LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH serves both as the presence test (#ifdef) and in arithmetic
// (#if LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH >= 32). In #if an undefined macro evaluates to 0, so a
// width comparison is also false when nothing is available.
//
// "Widest", not "fastest" or "native": the promise is always-lock-free and the widest standard type
// that is, nothing about speed.
//
// Only ATOMIC_*_LOCK_FREE == 2 selects. The macros are tri-valued - 0 never lock-free, 2 always,
// 1 *sometimes* - and 1 is not a weaker yes: it means lock-freedom cannot be promised at
// translation time, making it a per-object property to be checked with atomic_is_lock_free(). A
// "widest sometimes-lock-free" rule would be actively harmful: on -march=armv6-m and
// -march=armv5te every width including char reports 1, so it would select long long - the widest
// possible word on a target where the compiler promises lock-freedom for nothing.
//
// Every width is swept at == 2, narrowing: a narrow always-lock-free word still beats none.
//
// Measured with arm-linux-gnueabi-gcc 14.2.0 and gcc 14 (-m32):
//  -march=armv5te   every width 1                      -> nothing defined
//  -march=armv6     int/long 2, char/short/long long 1  -> 32-bit
//  -march=armv6k    every width 2                       -> 64-bit
//  -march=armv7-a   every width 2                       -> 64-bit
//  -march=armv6-m   every width 1                       -> nothing defined
//  -march=armv7-m   int/long 2, long long 1             -> 32-bit
//  gcc -m32         every width 2                       -> 64-bit
//
// A caller who wants a wider word than this selects anyway - 64 bits on ARMv7-M, say, accepting
// that long long is only sometimes lock-free there - names the type directly and checks at run time
// with atomic_is_lock_free() on its own storage.

#if   ATOMIC_LLONG_LOCK_FREE == 2
typedef unsigned long long lgk_widest_lockfree_uint;
#  define LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH ULLONG_WIDTH
#elif ATOMIC_LONG_LOCK_FREE  == 2
typedef unsigned long      lgk_widest_lockfree_uint;
#  define LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH ULONG_WIDTH
#elif ATOMIC_INT_LOCK_FREE   == 2
typedef unsigned int       lgk_widest_lockfree_uint;
#  define LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH UINT_WIDTH
#elif ATOMIC_SHORT_LOCK_FREE == 2
typedef unsigned short     lgk_widest_lockfree_uint;
#  define LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH USHRT_WIDTH
#elif ATOMIC_CHAR_LOCK_FREE  == 2
typedef unsigned char      lgk_widest_lockfree_uint;
#  define LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH UCHAR_WIDTH
#endif

#ifdef LGK_ATOMIC_WIDEST_LOCKFREE_WIDTH
typedef _Atomic(lgk_widest_lockfree_uint) lgk_atomic_widest_lockfree_uint;
#endif

#endif
