#include <lgk/ringbuf_int.h>

RINGBUF_INIT(int, unsigned, ringbuf_int)
RINGBUF_INIT_PREFILLED(int, unsigned, ringbuf_int)
RINGBUF_PUSH(int, unsigned, ringbuf_int)
RINGBUF_POP(int, unsigned, ringbuf_int)
RINGBUF_WRITE(int, unsigned, ringbuf_int)
RINGBUF_READ(int, unsigned, ringbuf_int)
