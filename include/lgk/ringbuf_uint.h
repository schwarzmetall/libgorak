#ifndef LGK_RINGBUF_UINT_H
#define LGK_RINGBUF_UINT_H

#include <lgk/ringbuf.h>

RINGBUF_STRUCT(unsigned, unsigned, ringbuf_uint);

RINGBUF_INIT_HEADER(unsigned, unsigned, ringbuf_uint);
RINGBUF_INIT_PREFILLED_HEADER(unsigned, unsigned, ringbuf_uint);
RINGBUF_PUSH_HEADER(unsigned, unsigned, ringbuf_uint);
RINGBUF_POP_HEADER(unsigned, unsigned, ringbuf_uint);
RINGBUF_WRITE_HEADER(unsigned, unsigned, ringbuf_uint);
RINGBUF_READ_HEADER(unsigned, unsigned, ringbuf_uint);

#endif
