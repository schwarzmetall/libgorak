#ifndef HEAP_INT_MIN_H
#define HEAP_INT_MIN_H

#include <lgk/heap.h>

HEAP_STRUCT(heap_int_min, int, unsigned);

HEAP_INIT_HEADER(heap_int_min);
HEAP_PUSH_HEADER(heap_int_min);
HEAP_POP_HEADER(heap_int_min);
HEAP_PEEK_HEADER(heap_int_min);

#endif
