#ifndef LGK_QUEUE_PTR_H
#define LGK_QUEUE_PTR_H

#include <lgk/queue.h>

QUEUE_STRUCT(void *, unsigned, queue_ptr);

QUEUE_INIT_HEADER(void *, unsigned, queue_ptr);
QUEUE_INIT_PREFILLED_HEADER(void *, unsigned, queue_ptr);
QUEUE_CLOSE_HEADER(void *, unsigned, queue_ptr);
QUEUE_PUSH_HEADER(void *, unsigned, queue_ptr);
QUEUE_POP_HEADER(void *, unsigned, queue_ptr);

#endif
