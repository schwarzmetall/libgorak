#include <lgk/queue.h>
#include <lgk/heap_uint_min.h>
#include <lgk/prioq_uint_min.h>

QUEUE_INIT(prioq_uint_min, heap_uint_min)
QUEUE_CLOSE(prioq_uint_min, heap_uint_min)
QUEUE_PUSH(prioq_uint_min, heap_uint_min)
QUEUE_POP(prioq_uint_min, heap_uint_min)
QUEUE_TRYPUSH(prioq_uint_min, heap_uint_min)
QUEUE_TRYPOP(prioq_uint_min, heap_uint_min)
