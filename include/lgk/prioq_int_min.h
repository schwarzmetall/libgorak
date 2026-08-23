#ifndef LGK_PRIOQ_INT_MIN_H
#define LGK_PRIOQ_INT_MIN_H

#include <lgk/heap_int_min.h>
#include <lgk/queue.h>

QUEUE_STRUCT(prioq_int_min, heap_int_min);

QUEUE_INIT_HEADER(prioq_int_min, heap_int_min);
QUEUE_CLOSE_HEADER(prioq_int_min, heap_int_min);
QUEUE_PUSH_HEADER(prioq_int_min, heap_int_min);
QUEUE_POP_HEADER(prioq_int_min, heap_int_min);
QUEUE_TRYPUSH_HEADER(prioq_int_min, heap_int_min);
QUEUE_TRYPOP_HEADER(prioq_int_min, heap_int_min);

#endif
