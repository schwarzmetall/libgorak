#ifndef HEAP_INT_MAX_H
#define HEAP_INT_MAX_H

#include <lgk/heap.h>

HEAP_STRUCT(heap_int_max, int, unsigned);

HEAP_INIT_HEADER(heap_int_max);
HEAP_PUSH_HEADER(heap_int_max);
HEAP_POP_HEADER(heap_int_max);
HEAP_PEEK_HEADER(heap_int_max);

#endif
