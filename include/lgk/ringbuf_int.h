#ifndef LGK_RINGBUF_INT_H
#define LGK_RINGBUF_INT_H

#include <lgk/ringbuf.h>

RINGBUF_STRUCT(int, unsigned, ringbuf_int);

RINGBUF_INIT_HEADER(int, unsigned, ringbuf_int);
RINGBUF_INIT_PREFILLED_HEADER(int, unsigned, ringbuf_int);
RINGBUF_PUSH_HEADER(int, unsigned, ringbuf_int);
RINGBUF_POP_HEADER(int, unsigned, ringbuf_int);
RINGBUF_WRITE_HEADER(int, unsigned, ringbuf_int);
RINGBUF_READ_HEADER(int, unsigned, ringbuf_int);

#endif
