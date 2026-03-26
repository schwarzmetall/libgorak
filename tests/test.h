#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>

#define test_assert(expr) do {                                              \
    if (!(expr)) {                                                          \
        fprintf(stderr, "%s:%d: test assertion failed: %s\n",               \
                __FILE__, __LINE__, #expr);                                 \
        abort();                                                            \
    }                                                                       \
} while (0)

#endif
