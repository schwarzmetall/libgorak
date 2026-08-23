#include <lgk/queue.h>
#include <lgk/fifo_uint.h>
#include <lgk/fifoq_uint.h>

QUEUE_INIT(fifoq_uint, fifo_uint)
QUEUE_INIT_PREFILLED(fifoq_uint, fifo_uint)
QUEUE_CLOSE(fifoq_uint, fifo_uint)
QUEUE_PUSH(fifoq_uint, fifo_uint)
QUEUE_POP(fifoq_uint, fifo_uint)
QUEUE_TRYPUSH(fifoq_uint, fifo_uint)
QUEUE_TRYPOP(fifoq_uint, fifo_uint)
