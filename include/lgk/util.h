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

#define BITWIDTH(type) _Generic((typeof_unqual(type)){},\
    char: CHAR_WIDTH,\
    short: SHRT_WIDTH,\
    int: INT_WIDTH,\
    long: LONG_WIDTH,\
    long long: LLONG_WIDTH,\
    unsigned char: UCHAR_WIDTH,\
    unsigned short: USHRT_WIDTH,\
    unsigned int: UINT_WIDTH,\
    unsigned long: ULONG_WIDTH,\
    unsigned long long: ULLONG_WIDTH)

// 1 iff type is exactly _Atomic of its own unqualified base, else 0. _Generic strips _Atomic from
// its controlling *value* but not from a pointer's target type, so the comparison is done on
// pointers.
#define IS_ATOMIC_TYPE(type) _Generic((type *){},\
        _Atomic typeof_unqual(type) *: 1,\
        _Atomic const typeof_unqual(type) *: 1,\
        _Atomic volatile typeof_unqual(type) *: 1,\
        _Atomic const volatile typeof_unqual(type) *: 1,\
        default: 0)

#define IS_SIGNED(type) (((typeof_unqual(type))-1)<0)
#define IS_UNSIGNED(type) (((typeof_unqual(type))-1)>0)

#define SSIZEOF(structname, member) sizeof((struct structname){}.member)
#define STYPEOF(structname, member) typeof((struct structname){}.member)
#define STYPEOF_DEREF(structname, member) typeof(*(struct structname){}.member)

unsigned digits(unsigned value, unsigned base);

#endif
