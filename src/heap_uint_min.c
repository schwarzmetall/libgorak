#include <lgk/heap.h>
#include <lgk/heap_uint_min.h>

static int heap_uint_min_compare(const unsigned *restrict a, const unsigned *restrict b) [[unsequenced]]
{
    return (*a <= *b) - (*a >= *b);
}

HEAP_HELPERS_STATIC(heap_uint_min)

HEAP_INIT(heap_uint_min)
HEAP_PUSH(heap_uint_min)
HEAP_POP(heap_uint_min)
HEAP_PEEK(heap_uint_min)
