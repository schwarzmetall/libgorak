/*
 * Two-thread test for queue_int: one producer, one consumer.
 * Run: ctest (from build dir).
 */

#include "test.h"
#include <stdint.h>
#include <threads.h>

#include <lgk/queue_int.h>

#define QUEUE_SIZE 8192
#define N_ITEMS    (QUEUE_SIZE << 6)
#define QUEUE_TIMEOUT_MS 5000

static struct queue_int g_q;
static int g_buffer[QUEUE_SIZE];

static int producer_thread(void *arg)
{
    (void)arg;
    for (int i = 0; i < N_ITEMS; i++) {
        int status = queue_int_push(&g_q, i, QUEUE_TIMEOUT_MS);
        if (status != thrd_success)
            return -1;
    }
    return 0;
}

static int consumer_thread(void *arg)
{
    int *collected = arg;
    for (int i = 0; i < N_ITEMS; i++) {
        int item;
        int status = queue_int_pop(&g_q, &item, QUEUE_TIMEOUT_MS);
        if (status != thrd_success)
            return -1;
        collected[i] = item;
    }
    return 0;
}

static void test_two_threads_producer_consumer(void)
{
    static int collected[N_ITEMS];
    thrd_t prod, cons;
    int res_prod, res_cons;

    int status = queue_int_init(&g_q, g_buffer, QUEUE_SIZE, 1);
    test_assert(status == thrd_success);

    test_assert(thrd_create(&prod, producer_thread, NULL) == thrd_success);
    test_assert(thrd_create(&cons, consumer_thread, collected) == thrd_success);

    test_assert(thrd_join(prod, &res_prod) == thrd_success);
    test_assert(thrd_join(cons, &res_cons) == thrd_success);

    test_assert(res_prod == 0);
    test_assert(res_cons == 0);

    for (int i = 0; i < N_ITEMS; i++)
        test_assert(collected[i] == i);

    queue_int_close(&g_q);
}

int main(void)
{
    test_two_threads_producer_consumer();
    return 0;
}
