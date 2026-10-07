#ifndef LGK_UTIL_H
#define LGK_UTIL_H

#include <limits.h>

// XXX REMEMBER: Don't use expressions with side effects as macro parameters unless you really know what you are doing!

#define _JOIN_FINAL(a,b) a##b
#define JOIN(a,b) _JOIN_FINAL(a,b)

#ifndef MIN
    #define MIN(a,b) ((a<b)?(a):(b))
#endif
#ifndef MAX
    #define MAX(a,b) ((a>b)?(a):(b))
#endif

#ifndef ABS
    #define ABS(x) (((x)<0)?(-(x)):(x))
#endif

#define INRANGE(v,min,max) (((v)>=(min))&&((v)<=(max)))
#define CLAMP(v,min,max) (((v)<(min))?(min):((((v)>(max))?(max):(v))))

#define ASIZE(a) (sizeof(a)/sizeof((a)[0]))
#define LASTINDEX(a) (ASIZE(a)-1)

#define INTDIVCEIL(i, d) (((i)+(d)-1)/(d))

// Value width (in bits) of a standard unsigned integer type, as an integer constant expression.
// Any other type is a compile error: the association list itself rejects bool, every signed type,
// unsigned _BitInt(N) and extended integer types. typeof_unqual strips _Atomic/const/volatile, so
// UWIDTH(_Atomic(unsigned)) is UINT_WIDTH.
// This is the value width, NOT sizeof(type)*CHAR_BIT: a padded type has fewer value bits than it
// occupies. Use sizeof for storage strides and UWIDTH for bit counts.
#define UWIDTH(type) _Generic((typeof_unqual(type)){},\
    unsigned char: UCHAR_WIDTH,\
    unsigned short: USHRT_WIDTH,\
    unsigned int: UINT_WIDTH,\
    unsigned long: ULONG_WIDTH,\
    unsigned long long: ULLONG_WIDTH)

// 1 iff type is exactly _Atomic of its own unqualified base, else 0. _Generic strips _Atomic from
// its controlling *value* but not from a pointer's target type, so the comparison is done on
// pointers. Needs no <stdatomic.h>, since _Atomic is a keyword.
// const/volatile-qualified atomics yield 0.
#define IS_ATOMIC_TYPE(type) _Generic((type *)nullptr, _Atomic(typeof_unqual(type)) *: 1, default: 0)

#define ASSERT_SIGNED(type) static_assert(((type)-1)<0)

#define SSIZEOF(structname, member) sizeof((struct structname){}.member)
#define STYPEOF(structname, member) typeof((struct structname){}.member)
#define STYPEOF_DEREF(structname, member) typeof(*(struct structname){}.member)

unsigned digits(unsigned value, unsigned base);

#endif
