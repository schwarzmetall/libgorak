#ifndef LGK_QUEUE_UINT_H
#define LGK_QUEUE_UINT_H

#include <lgk/queue.h>

QUEUE_STRUCT(unsigned, unsigned, queue_uint);

QUEUE_INIT_HEADER(unsigned, unsigned, queue_uint);
QUEUE_INIT_PREFILLED_HEADER(unsigned, unsigned, queue_uint);
QUEUE_CLOSE_HEADER(unsigned, unsigned, queue_uint);
QUEUE_PUSH_HEADER(unsigned, unsigned, queue_uint);
QUEUE_POP_HEADER(unsigned, unsigned, queue_uint);

#endif
