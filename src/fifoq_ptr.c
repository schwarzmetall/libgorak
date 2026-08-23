#include <lgk/queue.h>
#include <lgk/fifo_ptr.h>
#include <lgk/fifoq_ptr.h>

QUEUE_INIT(fifoq_ptr, fifo_ptr)
QUEUE_INIT_PREFILLED(fifoq_ptr, fifo_ptr)
QUEUE_CLOSE(fifoq_ptr, fifo_ptr)
QUEUE_PUSH(fifoq_ptr, fifo_ptr)
QUEUE_POP(fifoq_ptr, fifo_ptr)
QUEUE_TRYPUSH(fifoq_ptr, fifo_ptr)
QUEUE_TRYPOP(fifoq_ptr, fifo_ptr)
