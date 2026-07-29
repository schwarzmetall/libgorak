#ifndef LGK_RINGBUF_PTR_H
#define LGK_RINGBUF_PTR_H

#include <lgk/ringbuf.h>

RINGBUF_STRUCT(void *, unsigned, ringbuf_ptr);

RINGBUF_INIT_HEADER(void *, unsigned, ringbuf_ptr);
RINGBUF_INIT_PREFILLED_HEADER(void *, unsigned, ringbuf_ptr);
RINGBUF_PUSH_HEADER(void *, unsigned, ringbuf_ptr);
RINGBUF_POP_HEADER(void *, unsigned, ringbuf_ptr);
RINGBUF_WRITE_HEADER(void *, unsigned, ringbuf_ptr);
RINGBUF_READ_HEADER(void *, unsigned, ringbuf_ptr);

#endif
