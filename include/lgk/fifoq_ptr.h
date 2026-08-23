#ifndef LGK_FIFOQ_PTR_H
#define LGK_FIFOQ_PTR_H

#include <lgk/fifo_ptr.h>
#include <lgk/queue.h>

QUEUE_STRUCT(fifoq_ptr, fifo_ptr);

QUEUE_INIT_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_INIT_PREFILLED_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_CLOSE_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_PUSH_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_POP_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_TRYPUSH_HEADER(fifoq_ptr, fifo_ptr);
QUEUE_TRYPOP_HEADER(fifoq_ptr, fifo_ptr);

#endif
