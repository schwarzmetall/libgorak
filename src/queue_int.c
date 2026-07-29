#include <lgk/ringbuf.h>
#include <lgk/queue_int.h>

RINGBUF_INIT(int, unsigned, queue_int_ringbuf)
RINGBUF_INIT_PREFILLED(int, unsigned, queue_int_ringbuf)
RINGBUF_PUSH(int, unsigned, queue_int_ringbuf)
RINGBUF_POP(int, unsigned, queue_int_ringbuf)

QUEUE_INIT(int, unsigned, queue_int)
QUEUE_INIT_PREFILLED(int, unsigned, queue_int)
QUEUE_CLOSE(int, unsigned, queue_int)
QUEUE_PUSH(int, unsigned, queue_int)
QUEUE_POP(int, unsigned, queue_int)
QUEUE_TRYPUSH(int, unsigned, queue_int)
QUEUE_TRYPOP(int, unsigned, queue_int)
