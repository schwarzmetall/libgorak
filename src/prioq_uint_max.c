#include <lgk/queue.h>
#include <lgk/heap_int_max.h>
#include <lgk/prioq_int_max.h>

QUEUE_INIT(prioq_int_max, heap_int_max)
QUEUE_CLOSE(prioq_int_max, heap_int_max)
QUEUE_PUSH(prioq_int_max, heap_int_max)
QUEUE_POP(prioq_int_max, heap_int_max)
QUEUE_TRYPUSH(prioq_int_max, heap_int_max)
QUEUE_TRYPOP(prioq_int_max, heap_int_max)
