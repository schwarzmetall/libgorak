#ifndef LGK_QUEUE_INT_H
#define LGK_QUEUE_INT_H

#include <lgk/queue.h>

QUEUE_STRUCT(int, unsigned, queue_int);

QUEUE_INIT_HEADER(int, unsigned, queue_int);
QUEUE_INIT_PREFILLED_HEADER(int, unsigned, queue_int);
QUEUE_CLOSE_HEADER(int, unsigned, queue_int);
QUEUE_PUSH_HEADER(int, unsigned, queue_int);
QUEUE_POP_HEADER(int, unsigned, queue_int);

#endif
