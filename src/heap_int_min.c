#include <lgk/heap.h>
#include <lgk/heap_int_min.h>

static int heap_int_min_compare(const int *restrict a, const int *restrict b) [[unsequenced]]
{
    return (*a >= *b) - (*a <= *b);
}

HEAP_HELPERS_STATIC(heap_int_min)

HEAP_INIT(heap_int_min)
HEAP_PUSH(heap_int_min)
HEAP_POP(heap_int_min)
HEAP_PEEK(heap_int_min)
