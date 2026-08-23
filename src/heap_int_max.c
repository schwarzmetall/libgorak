#include <stdint.h>
#include <lgk/heap.h>
#include <lgk/heap_int_max.h>

static int_fast8_t heap_int_max_compare(const int *restrict a, const int *restrict b) [[unsequenced]]
{
    return (*a <= *b) - (*a >= *b);
}

HEAP_HELPERS_STATIC(heap_int_max)

HEAP_INIT(heap_int_max)
HEAP_PUSH(heap_int_max)
HEAP_POP(heap_int_max)
HEAP_PEEK(heap_int_max)
