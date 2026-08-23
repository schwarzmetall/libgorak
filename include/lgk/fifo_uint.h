#ifndef LGK_FIFO_UINT_H
#define LGK_FIFO_UINT_H

#include <lgk/fifo.h>

FIFO_STRUCT(fifo_uint, unsigned, unsigned);

FIFO_INIT_HEADER(fifo_uint);
FIFO_INIT_PREFILLED_HEADER(fifo_uint);
FIFO_PUSH_HEADER(fifo_uint);
FIFO_POP_HEADER(fifo_uint);
FIFO_WRITE_HEADER(fifo_uint);
FIFO_READ_HEADER(fifo_uint);

#endif
