#include <lgk/queue.h>
#include <lgk/heap_int_min.h>
#include <lgk/prioq_int_min.h>

QUEUE_INIT(prioq_int_min, heap_int_min)
QUEUE_CLOSE(prioq_int_min, heap_int_min)
QUEUE_PUSH(prioq_int_min, heap_int_min)
QUEUE_POP(prioq_int_min, heap_int_min)
QUEUE_TRYPUSH(prioq_int_min, heap_int_min)
QUEUE_TRYPOP(prioq_int_min, heap_int_min)
