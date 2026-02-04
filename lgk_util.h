#ifndef LGK_UTIL_H
#define LGK_UTIL_H

#define ASIZE(a) (sizeof(a)/sizeof(a[0]))
#define MIN(a,b) ((a<b)?(a):(b))
#define MAX(a,b) ((a>b)?(a):(b))
#define BITEXTRACT(value, basename) (((value) & basename##_MASK) >> basename##_SHIFT)

#endif
