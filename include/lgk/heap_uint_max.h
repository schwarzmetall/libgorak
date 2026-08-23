#ifndef HEAP_UINT_MAX_H
#define HEAP_UINT_MAX_H

#include <lgk/heap.h>

HEAP_STRUCT(heap_uint_max, unsigned, unsigned);

HEAP_INIT_HEADER(heap_uint_max);
HEAP_PUSH_HEADER(heap_uint_max);
HEAP_POP_HEADER(heap_uint_max);
HEAP_PEEK_HEADER(heap_uint_max);

#endif
