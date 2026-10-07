/*
 * Single-threaded test for heap_int_min / heap_int_max (no mutex/condvar involved).
 * Run: ctest (from build dir).
 */

#include "test.h"

#include <lgk/heap_int_min.h>
#include <lgk/heap_int_max.h>
#include <lgk/heap.h>
#include <lgk/util.h>

static const int g_shuffled[] = {5, -3, 8, 0, 42, -3, 17, 1, -100, 9};

/* HEAP_PUSHPOP is not part of the heap_int_* library API, so instantiate it locally */
HEAP_STRUCT(test_heap_min, int, unsigned);
HEAP_STRUCT(test_heap_max, int, unsigned);

static int_fast8_t test_heap_min_compare(const int *restrict a, const int *restrict b) [[unsequenced]]
{
    return (*a >= *b) - (*a <= *b);
}

static int_fast8_t test_heap_max_compare(const int *restrict a, const int *restrict b) [[unsequenced]]
{
    return (*a <= *b) - (*a >= *b);
}

HEAP_HELPERS_STATIC(test_heap_min)
static HEAP_INIT(test_heap_min)
static HEAP_PUSH(test_heap_min)
static HEAP_POP(test_heap_min)
static HEAP_PUSHPOP(test_heap_min)

HEAP_HELPERS_STATIC(test_heap_max)
static HEAP_INIT(test_heap_max)
static HEAP_PUSH(test_heap_max)
static HEAP_POP(test_heap_max)
static HEAP_PUSHPOP(test_heap_max)

static void test_init(void)
{
    static int buffer_min[4];
    static int buffer_max[4];
    struct heap_int_min hmin;
    struct heap_int_max hmax;

    int status = heap_int_min_init(&hmin, buffer_min, ASIZE(buffer_min));
    test_assert(status == 0);
    test_assert(hmin.used == 0);
    test_assert(hmin.size == ASIZE(buffer_min));

    status = heap_int_max_init(&hmax, buffer_max, ASIZE(buffer_max));
    test_assert(status == 0);
    test_assert(hmax.used == 0);
    test_assert(hmax.size == ASIZE(buffer_max));
}

static void test_push_pop_ordering_min(void)
{
    static int buffer[ASIZE(g_shuffled)];
    struct heap_int_min h;
    int item;

    int status = heap_int_min_init(&h, buffer, ASIZE(buffer));
    test_assert(status == 0);

    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        status = heap_int_min_push(&h, &g_shuffled[i]);
        test_assert(status == 0);
        test_assert(h.used == i + 1);
    }

    /* drain: the root must always be the smallest remaining item, so the
       popped sequence is non-decreasing */
    int previous = 0;
    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        /* peek returns a pointer into the heap buffer, so the value has to be
           copied out before the pop overwrites that slot */
        const int *root = heap_int_min_peek(&h);
        test_assert(root != NULL);
        const int root_value = *root;

        item = 0;
        status = heap_int_min_pop(&h, &item);
        test_assert(status == 0);
        test_assert(item == root_value);
        test_assert(h.used == ASIZE(g_shuffled) - i - 1);

        if (i)
            test_assert(item >= previous);
        previous = item;
    }
    test_assert(h.used == 0);

    /* smallest and largest of g_shuffled must have come out first and last */
    test_assert(previous == 42);
}

static void test_push_pop_ordering_max(void)
{
    static int buffer[ASIZE(g_shuffled)];
    struct heap_int_max h;
    int item;

    int status = heap_int_max_init(&h, buffer, ASIZE(buffer));
    test_assert(status == 0);

    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        status = heap_int_max_push(&h, &g_shuffled[i]);
        test_assert(status == 0);
        test_assert(h.used == i + 1);
    }

    /* drain: the root must always be the largest remaining item, so the
       popped sequence is non-increasing */
    int previous = 0;
    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        const int *root = heap_int_max_peek(&h);
        test_assert(root != NULL);
        const int root_value = *root;

        item = 0;
        status = heap_int_max_pop(&h, &item);
        test_assert(status == 0);
        test_assert(item == root_value);
        test_assert(h.used == ASIZE(g_shuffled) - i - 1);

        if (i)
            test_assert(item <= previous);
        previous = item;
    }
    test_assert(h.used == 0);

    test_assert(previous == -100);
}

static void test_full_and_empty(void)
{
    static int buffer[4];
    struct heap_int_min hmin;
    struct heap_int_max hmax;
    int item;

    int status = heap_int_min_init(&hmin, buffer, ASIZE(buffer));
    test_assert(status == 0);

    /* empty: pop must report "empty" (1), not an error; peek yields NULL */
    test_assert(heap_int_min_peek(&hmin) == NULL);
    status = heap_int_min_pop(&hmin, &item);
    test_assert(status == 1);

    /* fill to capacity */
    for (int i = 0; i < (int)ASIZE(buffer); i++) {
        status = heap_int_min_push(&hmin, &i);
        test_assert(status == 0);
    }
    test_assert(hmin.used == ASIZE(buffer));

    /* full: push must report "full" (1) */
    status = heap_int_min_push(&hmin, &(int){42});
    test_assert(status == 1);
    test_assert(hmin.used == ASIZE(buffer));

    /* popping from a full heap must still yield the correct root, and must not
       read past the end of the buffer */
    for (unsigned i = 0; i < ASIZE(buffer); i++) {
        item = -1;
        status = heap_int_min_pop(&hmin, &item);
        test_assert(status == 0);
        test_assert(item == (int)i);
    }
    test_assert(hmin.used == 0);
    status = heap_int_min_pop(&hmin, &item);
    test_assert(status == 1);

    /* same for the max-heap */
    status = heap_int_max_init(&hmax, buffer, ASIZE(buffer));
    test_assert(status == 0);
    test_assert(heap_int_max_peek(&hmax) == NULL);
    test_assert(heap_int_max_pop(&hmax, &item) == 1);

    for (int i = 0; i < (int)ASIZE(buffer); i++)
        test_assert(heap_int_max_push(&hmax, &i) == 0);
    test_assert(heap_int_max_push(&hmax, &(int){42}) == 1);

    for (unsigned i = 0; i < ASIZE(buffer); i++) {
        item = -1;
        status = heap_int_max_pop(&hmax, &item);
        test_assert(status == 0);
        test_assert(item == (int)(ASIZE(buffer) - i - 1));
    }
    test_assert(hmax.used == 0);
}

static void test_duplicates(void)
{
    static int buffer[6];
    struct heap_int_min hmin;
    struct heap_int_max hmax;
    int item;

    /* all-equal items exercise the compare function's "equal" result */
    int status = heap_int_min_init(&hmin, buffer, ASIZE(buffer));
    test_assert(status == 0);
    for (unsigned i = 0; i < ASIZE(buffer); i++)
        test_assert(heap_int_min_push(&hmin, &(int){7}) == 0);
    for (unsigned i = 0; i < ASIZE(buffer); i++) {
        item = -1;
        test_assert(heap_int_min_pop(&hmin, &item) == 0);
        test_assert(item == 7);
    }
    test_assert(hmin.used == 0);

    /* partial duplicates must not disturb the ordering */
    static const int with_dups[6] = {3, 1, 3, 1, 2, 2};
    static const int sorted_asc[6] = {1, 1, 2, 2, 3, 3};

    status = heap_int_min_init(&hmin, buffer, ASIZE(buffer));
    test_assert(status == 0);
    for (unsigned i = 0; i < ASIZE(with_dups); i++)
        test_assert(heap_int_min_push(&hmin, &with_dups[i]) == 0);
    for (unsigned i = 0; i < ASIZE(sorted_asc); i++) {
        item = -1;
        test_assert(heap_int_min_pop(&hmin, &item) == 0);
        test_assert(item == sorted_asc[i]);
    }

    status = heap_int_max_init(&hmax, buffer, ASIZE(buffer));
    test_assert(status == 0);
    for (unsigned i = 0; i < ASIZE(with_dups); i++)
        test_assert(heap_int_max_push(&hmax, &with_dups[i]) == 0);
    for (unsigned i = 0; i < ASIZE(sorted_asc); i++) {
        item = -1;
        test_assert(heap_int_max_pop(&hmax, &item) == 0);
        test_assert(item == sorted_asc[ASIZE(sorted_asc) - i - 1]);
    }
}

static void test_interleaved_push_pop(void)
{
    static int buffer[8];
    struct heap_int_min h;
    int item;

    int status = heap_int_min_init(&h, buffer, ASIZE(buffer));
    test_assert(status == 0);

    /* interleaving keeps the heap partially filled, so upheap and downheap run
       against varying tree depths */
    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        test_assert(heap_int_min_push(&h, &g_shuffled[i]) == 0);
        if (h.used < 3)
            continue;

        const int *root = heap_int_min_peek(&h);
        test_assert(root != NULL);
        const int root_value = *root;
        item = 0;
        test_assert(heap_int_min_pop(&h, &item) == 0);
        test_assert(item == root_value);

        /* the popped item must be <= everything still in the heap */
        for (unsigned j = 0; j < h.used; j++)
            test_assert(item <= h.buffer[j]);
    }

    /* whatever is left must still drain in ascending order */
    int previous = 0;
    for (unsigned popped = 0; h.used; popped++) {
        item = 0;
        test_assert(heap_int_min_pop(&h, &item) == 0);
        if (popped)
            test_assert(item >= previous);
        previous = item;
    }
    test_assert(h.used == 0);
}

static void test_pushpop_empty(void)
{
    static int buffer[4];
    struct test_heap_min h;
    int item = -1;

    test_assert(test_heap_min_init(&h, buffer, ASIZE(buffer)) == 0);

    /* on an empty heap the pushed item comes straight back out */
    test_assert(test_heap_min_pushpop(&h, &(int){5}, &item) == 0);
    test_assert(item == 5);
    test_assert(h.used == 0);
}

static void test_pushpop_replace_root(void)
{
    static int buffer[4];
    static const int initial[] = {10, 20, 30, 40};
    static const int expected[] = {20, 25, 30, 40};
    struct test_heap_min h;
    int item;

    test_assert(test_heap_min_init(&h, buffer, ASIZE(buffer)) == 0);
    for (unsigned i = 0; i < ASIZE(initial); i++)
        test_assert(test_heap_min_push(&h, &initial[i]) == 0);
    test_assert(h.used == h.size);

    /* smaller than the root: returned directly, heap untouched (works on a full heap) */
    item = -1;
    test_assert(test_heap_min_pushpop(&h, &(int){5}, &item) == 0);
    test_assert(item == 5);
    test_assert(h.used == ASIZE(initial));
    test_assert(h.buffer[0] == 10);

    /* equal to the root: returned directly as well */
    item = -1;
    test_assert(test_heap_min_pushpop(&h, &(int){10}, &item) == 0);
    test_assert(item == 10);
    test_assert(h.used == ASIZE(initial));
    test_assert(h.buffer[0] == 10);

    /* larger than the root: the root comes out, the item takes its place and sinks */
    item = -1;
    test_assert(test_heap_min_pushpop(&h, &(int){25}, &item) == 0);
    test_assert(item == 10);
    test_assert(h.used == ASIZE(initial));

    for (unsigned i = 0; i < ASIZE(expected); i++) {
        item = -1;
        test_assert(test_heap_min_pop(&h, &item) == 0);
        test_assert(item == expected[i]);
    }
    test_assert(h.used == 0);
}

static void test_pushpop_top_k(void)
{
    static int buffer_min[3];
    static int buffer_max[3];
    static const int largest_asc[] = {9, 17, 42};
    static const int smallest_desc[] = {-3, -3, -100};
    struct test_heap_min hmin;
    struct test_heap_max hmax;
    int item;

    /* a full min-heap fed through pushpop retains the k largest items seen,
       and every item pushed out is <= everything retained */
    test_assert(test_heap_min_init(&hmin, buffer_min, ASIZE(buffer_min)) == 0);
    test_assert(test_heap_max_init(&hmax, buffer_max, ASIZE(buffer_max)) == 0);
    for (unsigned i = 0; i < ASIZE(g_shuffled); i++) {
        if (hmin.used < hmin.size) {
            test_assert(test_heap_min_push(&hmin, &g_shuffled[i]) == 0);
            test_assert(test_heap_max_push(&hmax, &g_shuffled[i]) == 0);
            continue;
        }

        item = 0;
        test_assert(test_heap_min_pushpop(&hmin, &g_shuffled[i], &item) == 0);
        test_assert(hmin.used == hmin.size);
        for (unsigned i_heap = 0; i_heap < hmin.used; i_heap++)
            test_assert(item <= hmin.buffer[i_heap]);

        item = 0;
        test_assert(test_heap_max_pushpop(&hmax, &g_shuffled[i], &item) == 0);
        test_assert(hmax.used == hmax.size);
        for (unsigned i_heap = 0; i_heap < hmax.used; i_heap++)
            test_assert(item >= hmax.buffer[i_heap]);
    }

    for (unsigned i = 0; i < ASIZE(largest_asc); i++) {
        item = 0;
        test_assert(test_heap_min_pop(&hmin, &item) == 0);
        test_assert(item == largest_asc[i]);
    }
    for (unsigned i = 0; i < ASIZE(smallest_desc); i++) {
        item = 0;
        test_assert(test_heap_max_pop(&hmax, &item) == 0);
        test_assert(item == smallest_desc[i]);
    }
}

int main(void)
{
    test_init();
    test_push_pop_ordering_min();
    test_push_pop_ordering_max();
    test_full_and_empty();
    test_duplicates();
    test_interleaved_push_pop();
    test_pushpop_empty();
    test_pushpop_replace_root();
    test_pushpop_top_k();
    return 0;
}
