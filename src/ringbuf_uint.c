#include <lgk/ringbuf_uint.h>

RINGBUF_INIT(unsigned, unsigned, ringbuf_uint)
RINGBUF_INIT_PREFILLED(unsigned, unsigned, ringbuf_uint)
RINGBUF_PUSH(unsigned, unsigned, ringbuf_uint)
RINGBUF_POP(unsigned, unsigned, ringbuf_uint)
RINGBUF_WRITE(unsigned, unsigned, ringbuf_uint)
RINGBUF_READ(unsigned, unsigned, ringbuf_uint)
