#include <lgk/queue.h>
#include <lgk/queue_ptr.h>

QUEUE_INIT(void *, unsigned, queue_ptr)
QUEUE_INIT_PREFILLED(void *, unsigned, queue_ptr)
QUEUE_CLOSE(void *, unsigned, queue_ptr)
QUEUE_PUSH(void *, unsigned, queue_ptr)
QUEUE_POP(void *, unsigned, queue_ptr)
