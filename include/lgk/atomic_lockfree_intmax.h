#ifndef ATOMIC_LOCKFREE_INTMAX_H
#define ATOMIC_LOCKFREE_INTMAX_H

#include <stdatomic.h>
#include <limits.h>

// widest always-lock-free atomic integer type (type will not be defined if target offers none)

#if ATOMIC_LLONG_LOCK_FREE == 2
typedef atomic_llong atomic_lockfree_intmax_t;
typedef atomic_ullong atomic_lockfree_uintmax_t;
    #define ATOMIC_LOCKFREE_INTMAX_WIDTH LLONG_WIDTH
    #define ATOMIC_LOCKFREE_INTMAX_MIN LLONG_MIN 
    #define ATOMIC_LOCKFREE_INTMAX_MAX LLONG_MAX 
    #define ATOMIC_LOCKFREE_UINTMAX_WIDTH ULLONG_WIDTH
    #define ATOMIC_LOCKFREE_UINTMAX_MAX ULLONG_MAX
#elif ATOMIC_LONG_LOCK_FREE == 2
typedef atomic_long atomic_lockfree_intmax_t;
typedef atomic_ulong atomic_lockfree_uintmax_t;
    #define ATOMIC_LOCKFREE_INTMAX_WIDTH LONG_WIDTH
    #define ATOMIC_LOCKFREE_INTMAX_MIN LONG_MIN 
    #define ATOMIC_LOCKFREE_INTMAX_MAX LONG_MAX 
    #define ATOMIC_LOCKFREE_UINTMAX_WIDTH ULONG_WIDTH
    #define ATOMIC_LOCKFREE_UINTMAX_MAX ULONG_MAX
#elif ATOMIC_INT_LOCK_FREE == 2
typedef atomic_int atomic_lockfree_intmax_t;
typedef atomic_uint atomic_lockfree_uintmax_t;
    #define ATOMIC_LOCKFREE_INTMAX_WIDTH INT_WIDTH
    #define ATOMIC_LOCKFREE_INTMAX_MIN INT_MIN 
    #define ATOMIC_LOCKFREE_INTMAX_MAX INT_MAX 
    #define ATOMIC_LOCKFREE_UINTMAX_WIDTH UINT_WIDTH
    #define ATOMIC_LOCKFREE_UINTMAX_MAX UINT_MAX
#elif ATOMIC_SHORT_LOCK_FREE == 2
typedef atomic_short atomic_lockfree_intmax_t;
typedef atomic_ushort atomic_lockfree_uintmax_t;
    #define ATOMIC_LOCKFREE_INTMAX_WIDTH SHRT_WIDTH
    #define ATOMIC_LOCKFREE_INTMAX_MIN SHRT_MIN 
    #define ATOMIC_LOCKFREE_INTMAX_MAX SHRT_MAX 
    #define ATOMIC_LOCKFREE_UINTMAX_WIDTH USHRT_WIDTH
    #define ATOMIC_LOCKFREE_UINTMAX_MAX USHRT_MAX
#elif ATOMIC_CHAR_LOCK_FREE  == 2
typedef atomic_char atomic_lockfree_intmax_t;
typedef atomic_uchar atomic_lockfree_uintmax_t;
    #define ATOMIC_LOCKFREE_INTMAX_WIDTH CHAR_WIDTH
    #define ATOMIC_LOCKFREE_INTMAX_MIN CHAR_MIN 
    #define ATOMIC_LOCKFREE_INTMAX_MAX CHAR_MAX 
    #define ATOMIC_LOCKFREE_UINTMAX_WIDTH UCHAR_WIDTH
    #define ATOMIC_LOCKFREE_UINTMAX_MAX UCHAR_MAX
#endif

#endif
