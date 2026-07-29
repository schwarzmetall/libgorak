#ifndef LGK_QUEUE_INT_H
#define LGK_QUEUE_INT_H

#include <lgk/ringbuf.h>
#include <lgk/queue.h>

RINGBUF_STRUCT(int, unsigned, queue_int_ringbuf);
RINGBUF_INIT_HEADER(int, unsigned, queue_int_ringbuf);
RINGBUF_INIT_PREFILLED_HEADER(int, unsigned, queue_int_ringbuf);
RINGBUF_PUSH_HEADER(int, unsigned, queue_int_ringbuf);
RINGBUF_POP_HEADER(int, unsigned, queue_int_ringbuf);

QUEUE_STRUCT(int, unsigned, queue_int);
QUEUE_INIT_HEADER(int, unsigned, queue_int);
QUEUE_INIT_PREFILLED_HEADER(int, unsigned, queue_int);
QUEUE_CLOSE_HEADER(int, unsigned, queue_int);
QUEUE_PUSH_HEADER(int, unsigned, queue_int);
QUEUE_POP_HEADER(int, unsigned, queue_int);
QUEUE_TRYPUSH_HEADER(int, unsigned, queue_int);
QUEUE_TRYPOP_HEADER(int, unsigned, queue_int);

#endif
