#ifndef LGK_FIFOQ_UINT_H
#define LGK_FIFOQ_UINT_H

#include <lgk/fifo_uint.h>
#include <lgk/queue.h>

QUEUE_STRUCT(fifoq_uint, fifo_uint);

QUEUE_INIT_HEADER(fifoq_uint, fifo_uint);
QUEUE_INIT_PREFILLED_HEADER(fifoq_uint, fifo_uint);
QUEUE_CLOSE_HEADER(fifoq_uint, fifo_uint);
QUEUE_PUSH_HEADER(fifoq_uint, fifo_uint);
QUEUE_POP_HEADER(fifoq_uint, fifo_uint);
QUEUE_TRYPUSH_HEADER(fifoq_uint, fifo_uint);
QUEUE_TRYPOP_HEADER(fifoq_uint, fifo_uint);

#endif
