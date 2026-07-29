#include <lgk/ringbuf_ptr.h>

RINGBUF_INIT(void *, unsigned, ringbuf_ptr)
RINGBUF_INIT_PREFILLED(void *, unsigned, ringbuf_ptr)
RINGBUF_PUSH(void *, unsigned, ringbuf_ptr)
RINGBUF_POP(void *, unsigned, ringbuf_ptr)
RINGBUF_WRITE(void *, unsigned, ringbuf_ptr)
RINGBUF_READ(void *, unsigned, ringbuf_ptr)
