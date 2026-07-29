#ifndef LGK_QUEUE_PTR_H
#define LGK_QUEUE_PTR_H

#include <lgk/ringbuf.h>
#include <lgk/queue.h>

RINGBUF_STRUCT(void *, unsigned, queue_ptr_ringbuf);
RINGBUF_INIT_HEADER(void *, unsigned, queue_ptr_ringbuf);
RINGBUF_INIT_PREFILLED_HEADER(void *, unsigned, queue_ptr_ringbuf);
RINGBUF_PUSH_HEADER(void *, unsigned, queue_ptr_ringbuf);
RINGBUF_POP_HEADER(void *, unsigned, queue_ptr_ringbuf);

QUEUE_STRUCT(void *, unsigned, queue_ptr);
QUEUE_INIT_HEADER(void *, unsigned, queue_ptr);
QUEUE_INIT_PREFILLED_HEADER(void *, unsigned, queue_ptr);
QUEUE_CLOSE_HEADER(void *, unsigned, queue_ptr);
QUEUE_PUSH_HEADER(void *, unsigned, queue_ptr);
QUEUE_POP_HEADER(void *, unsigned, queue_ptr);
QUEUE_TRYPUSH_HEADER(void *, unsigned, queue_ptr);
QUEUE_TRYPOP_HEADER(void *, unsigned, queue_ptr);

#endif
