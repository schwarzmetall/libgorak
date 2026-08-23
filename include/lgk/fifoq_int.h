#ifndef LGK_FIFOQ_INT_H
#define LGK_FIFOQ_INT_H

#include <lgk/fifo_int.h>
#include <lgk/queue.h>

QUEUE_STRUCT(fifoq_int, fifo_int);

QUEUE_INIT_HEADER(fifoq_int, fifo_int);
QUEUE_INIT_PREFILLED_HEADER(fifoq_int, fifo_int);
QUEUE_CLOSE_HEADER(fifoq_int, fifo_int);
QUEUE_PUSH_HEADER(fifoq_int, fifo_int);
QUEUE_POP_HEADER(fifoq_int, fifo_int);
QUEUE_TRYPUSH_HEADER(fifoq_int, fifo_int);
QUEUE_TRYPOP_HEADER(fifoq_int, fifo_int);

#endif
