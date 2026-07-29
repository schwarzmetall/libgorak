#include <lgk/ringbuf.h>
#include <lgk/queue_ptr.h>

RINGBUF_INIT(void *, unsigned, queue_ptr_ringbuf)
RINGBUF_INIT_PREFILLED(void *, unsigned, queue_ptr_ringbuf)
RINGBUF_PUSH(void *, unsigned, queue_ptr_ringbuf)
RINGBUF_POP(void *, unsigned, queue_ptr_ringbuf)

QUEUE_INIT(void *, unsigned, queue_ptr)
QUEUE_INIT_PREFILLED(void *, unsigned, queue_ptr)
QUEUE_CLOSE(void *, unsigned, queue_ptr)
QUEUE_PUSH(void *, unsigned, queue_ptr)
QUEUE_POP(void *, unsigned, queue_ptr)
QUEUE_TRYPUSH(void *, unsigned, queue_ptr)
QUEUE_TRYPOP(void *, unsigned, queue_ptr)
