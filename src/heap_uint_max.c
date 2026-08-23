#include <lgk/heap.h>
#include <lgk/heap_uint_max.h>

static int_fast8_t heap_uint_max_compare(const unsigned *restrict a, const unsigned *restrict b) [[unsequenced]]
{
    return (*a <= *b) - (*a >= *b);
}

HEAP_HELPERS_STATIC(heap_uint_max)

HEAP_INIT(heap_uint_max)
HEAP_PUSH(heap_uint_max)
HEAP_POP(heap_uint_max)
HEAP_PEEK(heap_uint_max)
