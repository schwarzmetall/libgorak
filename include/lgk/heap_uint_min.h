#ifndef HEAP_UINT_MIN_H
#define HEAP_UINT_MIN_H

#include <lgk/heap.h>

HEAP_STRUCT(heap_uint_min, unsigned, unsigned);

HEAP_INIT_HEADER(heap_uint_min);
HEAP_PUSH_HEADER(heap_uint_min);
HEAP_POP_HEADER(heap_uint_min);
HEAP_PEEK_HEADER(heap_uint_min);

#endif
