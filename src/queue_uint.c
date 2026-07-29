#include <lgk/ringbuf.h>
#include <lgk/queue_uint.h>

RINGBUF_INIT(unsigned, unsigned, queue_uint_ringbuf)
RINGBUF_INIT_PREFILLED(unsigned, unsigned, queue_uint_ringbuf)
RINGBUF_PUSH(unsigned, unsigned, queue_uint_ringbuf)
RINGBUF_POP(unsigned, unsigned, queue_uint_ringbuf)

QUEUE_INIT(unsigned, unsigned, queue_uint)
QUEUE_INIT_PREFILLED(unsigned, unsigned, queue_uint)
QUEUE_CLOSE(unsigned, unsigned, queue_uint)
QUEUE_PUSH(unsigned, unsigned, queue_uint)
QUEUE_POP(unsigned, unsigned, queue_uint)
QUEUE_TRYPUSH(unsigned, unsigned, queue_uint)
QUEUE_TRYPOP(unsigned, unsigned, queue_uint)
