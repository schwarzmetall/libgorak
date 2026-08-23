/*
 * Tests for prioq_int_min / prioq_int_max: batched ordering and timeout checks
 * on a single thread, plus a two-thread producer/consumer run.
 * Run: ctest (from build dir).
 */

#include "test.h"
#include <stdint.h>
#include <threads.h>

#include <lgk/prioq_int_min.h>
#include <lgk/prioq_int_max.h>
#include <lgk/util.h>

#define QUEUE_SIZE       1024
#define N_ITEMS          (QUEUE_SIZE << 6)
#define QUEUE_TIMEOUT_MS 5000
#define SHORT_TIMEOUT_MS 50
#define VALUE_RANGE      4096
#define SEED             12345u

/* the batch tests push g_batch into a queue of exactly ASIZE(g_batch) slots and
   expect it to drain fully sorted */
static const int g_batch[8]       = {5, -3, 8, 0, 42, -3, 17, 1};
static const int g_sorted_asc[8]  = {-3, -3, 0, 1, 5, 8, 17, 42};
static const int g_sorted_desc[8] = {42, 17, 8, 5, 1, 0, -3, -3};

/* deterministic pseudo-random values, so the producer's multiset can be
   recomputed by the checker without sorting anything */
static int next_value(uint32_t *state)
{
    *state = (*state * 1103515245u) + 12345u;
    return (int)((*state >> 8) % VALUE_RANGE);
}

#define DEFINE_PRIOQ_TESTS(name, expected_drain)                               \
                                                                               \
static struct name g_q_##name;                                                 \
static int g_buffer_##name[QUEUE_SIZE];                                        \
                                                                               \
static void name##_test_batched_ordering(void)                                 \
{                                                                              \
    static int buffer[ASIZE(g_batch)];                                         \
    struct name q;                                                             \
    int item;                                                                  \
                                                                               \
    int status = name##_init(&q, buffer, ASIZE(buffer), 1);                     \
    test_assert(status == thrd_success);                                        \
                                                                               \
    /* empty queue: trypop must not block, must report thrd_busy */             \
    status = name##_trypop(&q, &item, QUEUE_TIMEOUT_MS);                        \
    test_assert(status == thrd_busy);                                           \
                                                                               \
    for (unsigned i = 0; i < ASIZE(g_batch); i++) {                            \
        status = name##_trypush(&q, &g_batch[i], QUEUE_TIMEOUT_MS);             \
        test_assert(status == thrd_success);                                    \
    }                                                                          \
                                                                               \
    /* full queue: trypush must not block, must report thrd_busy */             \
    status = name##_trypush(&q, &(int){42}, QUEUE_TIMEOUT_MS);                  \
    test_assert(status == thrd_busy);                                           \
                                                                               \
    /* draining a queue that was filled before any pop happened yields the      \
       items in full priority order */                                          \
    for (unsigned i = 0; i < ASIZE(expected_drain); i++) {                      \
        item = 0;                                                               \
        status = name##_trypop(&q, &item, QUEUE_TIMEOUT_MS);                    \
        test_assert(status == thrd_success);                                    \
        test_assert(item == expected_drain[i]);                                 \
    }                                                                          \
                                                                               \
    /* empty again */                                                          \
    status = name##_trypop(&q, &item, QUEUE_TIMEOUT_MS);                        \
    test_assert(status == thrd_busy);                                           \
                                                                               \
    test_assert(name##_close(&q) == thrd_success);                              \
}                                                                              \
                                                                               \
static void name##_test_timeouts(void)                                         \
{                                                                              \
    static int buffer[4];                                                      \
    struct name q;                                                             \
    int item;                                                                  \
                                                                               \
    int status = name##_init(&q, buffer, ASIZE(buffer), 1);                     \
    test_assert(status == thrd_success);                                        \
                                                                               \
    /* nothing to pop and no other thread to push: pop must time out */        \
    status = name##_pop(&q, &item, SHORT_TIMEOUT_MS);                           \
    test_assert(status == thrd_timedout);                                       \
                                                                               \
    for (int i = 0; i < (int)ASIZE(buffer); i++)                               \
        test_assert(name##_push(&q, &i, QUEUE_TIMEOUT_MS) == thrd_success);     \
                                                                               \
    /* no room and no other thread to pop: push must time out */               \
    status = name##_push(&q, &(int){42}, SHORT_TIMEOUT_MS);                     \
    test_assert(status == thrd_timedout);                                       \
                                                                               \
    for (unsigned i = 0; i < ASIZE(buffer); i++)                               \
        test_assert(name##_pop(&q, &item, QUEUE_TIMEOUT_MS) == thrd_success);   \
                                                                               \
    test_assert(name##_close(&q) == thrd_success);                              \
}                                                                              \
                                                                               \
static int name##_producer_thread(void *arg)                                    \
{                                                                              \
    (void)arg;                                                                 \
    uint32_t state = SEED;                                                     \
    for (int i = 0; i < N_ITEMS; i++) {                                        \
        int item = next_value(&state);                                         \
        int status = name##_push(&g_q_##name, &item, QUEUE_TIMEOUT_MS);         \
        if (status != thrd_success) return -1;                                  \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static int name##_consumer_thread(void *arg)                                    \
{                                                                              \
    unsigned *counts = arg;                                                    \
    for (int i = 0; i < N_ITEMS; i++) {                                        \
        int item;                                                              \
        int status = name##_pop(&g_q_##name, &item, QUEUE_TIMEOUT_MS);          \
        if (status != thrd_success) return -1;                                  \
        if ((item < 0) || (item >= VALUE_RANGE)) return -1;                     \
        counts[item]++;                                                        \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static void name##_test_two_threads_producer_consumer(void)                    \
{                                                                              \
    static unsigned counts[VALUE_RANGE];                                       \
    static unsigned expected[VALUE_RANGE];                                     \
    thrd_t prod, cons;                                                         \
    int res_prod, res_cons;                                                    \
                                                                               \
    uint32_t state = SEED;                                                     \
    for (int i = 0; i < N_ITEMS; i++)                                          \
        expected[next_value(&state)]++;                                        \
                                                                               \
    int status = name##_init(&g_q_##name, g_buffer_##name, QUEUE_SIZE, 1);      \
    test_assert(status == thrd_success);                                        \
                                                                               \
    test_assert(thrd_create(&prod, name##_producer_thread, NULL)               \
                == thrd_success);                                              \
    test_assert(thrd_create(&cons, name##_consumer_thread, counts)             \
                == thrd_success);                                              \
                                                                               \
    test_assert(thrd_join(prod, &res_prod) == thrd_success);                    \
    test_assert(thrd_join(cons, &res_cons) == thrd_success);                    \
                                                                               \
    test_assert(res_prod == 0);                                                \
    test_assert(res_cons == 0);                                                \
                                                                               \
    /* a priority queue that is filled and drained concurrently only orders the \
       items that happen to be resident at pop time, so the consumer sequence   \
       is not globally sorted; what must hold is that every pushed item comes    \
       out exactly once */                                                      \
    for (unsigned i = 0; i < VALUE_RANGE; i++)                                 \
        test_assert(counts[i] == expected[i]);                                  \
                                                                               \
    test_assert(name##_close(&g_q_##name) == thrd_success);                     \
}

DEFINE_PRIOQ_TESTS(prioq_int_min, g_sorted_asc)
DEFINE_PRIOQ_TESTS(prioq_int_max, g_sorted_desc)

int main(void)
{
    prioq_int_min_test_batched_ordering();
    prioq_int_min_test_timeouts();
    prioq_int_min_test_two_threads_producer_consumer();

    prioq_int_max_test_batched_ordering();
    prioq_int_max_test_timeouts();
    prioq_int_max_test_two_threads_producer_consumer();

    return 0;
}
