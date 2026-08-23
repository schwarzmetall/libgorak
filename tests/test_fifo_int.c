/*
 * Single-threaded test for fifo_int (no mutex/condvar involved).
 * Run: ctest (from build dir).
 */

#include "test.h"

#include <lgk/fifo_int.h>
#include <lgk/util.h>

static void test_init(void)
{
    static int buffer[4];
    struct fifo_int rb;

    int status = fifo_int_init(&rb, buffer, ASIZE(buffer));
    test_assert(status == 0);
    test_assert(rb.used == 0);
}

static void test_push_pop_and_wraparound(void)
{
    static int buffer[4];
    struct fifo_int rb;
    int item;

    int status = fifo_int_init(&rb, buffer, ASIZE(buffer));
    test_assert(status == 0);

    /* empty: pop must report "empty" (1), not an error */
    status = fifo_int_pop(&rb, &item);
    test_assert(status == 1);

    /* fill via push */
    for (int i = 0; i < (int)ASIZE(buffer); i++) {
        status = fifo_int_push(&rb, &i);
        test_assert(status == 0);
    }
    test_assert(rb.used == ASIZE(buffer));

    /* full: push must report "full" (1) */
    status = fifo_int_push(&rb, &(int){42});
    test_assert(status == 1);

    /* drain, verify FIFO order */
    for (unsigned i = 0; i < ASIZE(buffer); i++) {
        item = -1;
        status = fifo_int_pop(&rb, &item);
        test_assert(status == 0);
        test_assert(item == (int)i);
    }
    test_assert(rb.used == 0);

    /* empty again */
    status = fifo_int_pop(&rb, &item);
    test_assert(status == 1);

    /* exercise wraparound with repeated single-item push/pop */
    for (int i = 0; i < (int)ASIZE(buffer) * 3; i++) {
        status = fifo_int_push(&rb, &i);
        test_assert(status == 0);
        item = -1;
        status = fifo_int_pop(&rb, &item);
        test_assert(status == 0);
        test_assert(item == (int)i);
    }
    test_assert(rb.used == 0);
}

static void test_write_read_partial(void)
{
    static int buffer[4];
    static int src[6] = {10, 11, 12, 13, 14, 15};
    static int dst[6];
    struct fifo_int rb;

    int status = fifo_int_init(&rb, buffer, ASIZE(buffer));
    test_assert(status == 0);

    /* write more than fits: short (partial) transfer */
    int n = fifo_int_write(&rb, src, ASIZE(src));
    test_assert(n == (int)ASIZE(buffer));
    test_assert(rb.used == ASIZE(buffer));

    /* writing to an already-full buffer transfers 0, not an error */
    n = fifo_int_write(&rb, src, 1);
    test_assert(n == 0);

    /* read more than available: short (partial) transfer, drains correctly */
    n = fifo_int_read(&rb, dst, ASIZE(dst));
    test_assert(n == (int)ASIZE(buffer));
    test_assert(rb.used == 0);
    for (int i = 0; i < n; i++)
        test_assert(dst[i] == src[i]);

    /* reading an empty buffer transfers 0, not an error */
    n = fifo_int_read(&rb, dst, ASIZE(dst));
    test_assert(n == 0);
}

static void test_write_read_wraparound(void)
{
    static int buffer[4];
    struct fifo_int rb;
    int item;
    int src[3] = {100, 101, 102};
    int dst[3] = {-1, -1, -1};

    int status = fifo_int_init(&rb, buffer, ASIZE(buffer));
    test_assert(status == 0);

    /* push two items, pop two items, to move i_write/i_read to index 2 */
    test_assert(fifo_int_push(&rb, &(int){1}) == 0);
    test_assert(fifo_int_push(&rb, &(int){2}) == 0);
    test_assert(fifo_int_pop(&rb, &item) == 0 && item == 1);
    test_assert(fifo_int_pop(&rb, &item) == 0 && item == 2);
    test_assert(rb.used == 0);
    test_assert(rb.i_write == 2);
    test_assert(rb.i_read == 2);

    /* bulk write of 3 items straddles the end of the 4-slot buffer */
    int n = fifo_int_write(&rb, src, ASIZE(src));
    test_assert(n == (int)ASIZE(src));
    test_assert(rb.used == ASIZE(src));
    test_assert(rb.i_write == 1);

    /* bulk read straddling the end, verify FIFO order and final indices */
    n = fifo_int_read(&rb, dst, ASIZE(dst));
    test_assert(n == (int)ASIZE(dst));
    test_assert(rb.used == 0);
    test_assert(rb.i_read == 1);
    for (int i = 0; i < n; i++)
        test_assert(dst[i] == src[i]);
}

static void test_init_prefilled(void)
{
    static int buffer[4];
    struct fifo_int rb;

    /* partially prefilled */
    int status = fifo_int_init_prefilled(&rb, buffer, ASIZE(buffer), 2);
    test_assert(status == 0);
    test_assert(rb.used == 2);
    test_assert(rb.i_write == 2);
    test_assert(rb.i_read == 0);

    /* fully prefilled */
    status = fifo_int_init_prefilled(&rb, buffer, ASIZE(buffer), ASIZE(buffer));
    test_assert(status == 0);
    test_assert(rb.used == ASIZE(buffer));
}

int main(void)
{
    test_init();
    test_push_pop_and_wraparound();
    test_write_read_partial();
    test_write_read_wraparound();
    test_init_prefilled();
    return 0;
}
