#ifndef LGK_UTIL_H
#define LGK_UTIL_H

// XXX REMEMBER: Don't use expressions with side effects as macro parameters!

#define _JOIN_FINAL(a,b) a##b
#define JOIN(a,b) _JOIN_FINAL(a,b)

#ifndef MIN
    #define MIN(a,b) ((a<b)?(a):(b))
#endif
#ifndef MAX
    #define MAX(a,b) ((a>b)?(a):(b))
#endif

#ifndef ABS
    #define ABS(x) ((x)<0 ? -(x) : (x))
#endif

#define INRANGE(v,min,max) (((v)>=min)&&((v)<=max))

#define ASIZE(a) (sizeof(a)/sizeof(a[0]))
#define LASTINDEX(a) (ASIZE(a)-1)

#define BITEXTRACT(value, basename) (((value) & basename##_MASK) >> basename##_SHIFT)
#define BITSIZE(x) (sizeof(x)<<3)

#define INTDIVCEIL(i, d) ((i+d-1)/d)

#define ASSERT_SIGNED(type) static_assert(((type)-1)<0)

#define SIZEOFSM(type, member) sizeof(((type *)0)->member)

unsigned digits(unsigned value, unsigned base);

#endif
