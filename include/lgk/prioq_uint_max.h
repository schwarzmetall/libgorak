#ifndef LGK_PRIOQ_UINT_MAX_H
#define LGK_PRIOQ_UINT_MAX_H

#include <lgk/heap_uint_max.h>
#include <lgk/queue.h>

QUEUE_STRUCT(prioq_uint_max, heap_uint_max);

QUEUE_INIT_HEADER(prioq_uint_max, heap_uint_max);
QUEUE_CLOSE_HEADER(prioq_uint_max, heap_uint_max);
QUEUE_PUSH_HEADER(prioq_uint_max, heap_uint_max);
QUEUE_POP_HEADER(prioq_uint_max, heap_uint_max);
QUEUE_TRYPUSH_HEADER(prioq_uint_max, heap_uint_max);
QUEUE_TRYPOP_HEADER(prioq_uint_max, heap_uint_max);

#endif
