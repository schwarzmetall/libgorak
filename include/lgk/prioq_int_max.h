#ifndef LGK_PRIOQ_INT_MAX_H
#define LGK_PRIOQ_INT_MAX_H

#include <lgk/heap_int_max.h>
#include <lgk/queue.h>

QUEUE_STRUCT(prioq_int_max, heap_int_max);

QUEUE_INIT_HEADER(prioq_int_max, heap_int_max);
QUEUE_CLOSE_HEADER(prioq_int_max, heap_int_max);
QUEUE_PUSH_HEADER(prioq_int_max, heap_int_max);
QUEUE_POP_HEADER(prioq_int_max, heap_int_max);
QUEUE_TRYPUSH_HEADER(prioq_int_max, heap_int_max);
QUEUE_TRYPOP_HEADER(prioq_int_max, heap_int_max);

#endif
