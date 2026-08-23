#ifndef LGK_PRIOQ_UINT_MIN_H
#define LGK_PRIOQ_UINT_MIN_H

#include <lgk/heap_uint_min.h>
#include <lgk/queue.h>

QUEUE_STRUCT(prioq_uint_min, heap_uint_min);

QUEUE_INIT_HEADER(prioq_uint_min, heap_uint_min);
QUEUE_CLOSE_HEADER(prioq_uint_min, heap_uint_min);
QUEUE_PUSH_HEADER(prioq_uint_min, heap_uint_min);
QUEUE_POP_HEADER(prioq_uint_min, heap_uint_min);
QUEUE_TRYPUSH_HEADER(prioq_uint_min, heap_uint_min);
QUEUE_TRYPOP_HEADER(prioq_uint_min, heap_uint_min);

#endif
