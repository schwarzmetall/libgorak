#include <lgk/queue.h>
#include <lgk/fifo_int.h>
#include <lgk/fifoq_int.h>

QUEUE_INIT(fifoq_int, fifo_int)
QUEUE_INIT_PREFILLED(fifoq_int, fifo_int)
QUEUE_CLOSE(fifoq_int, fifo_int)
QUEUE_PUSH(fifoq_int, fifo_int)
QUEUE_POP(fifoq_int, fifo_int)
QUEUE_TRYPUSH(fifoq_int, fifo_int)
QUEUE_TRYPOP(fifoq_int, fifo_int)
