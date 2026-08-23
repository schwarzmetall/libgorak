#ifndef LGK_FIFO_PTR_H
#define LGK_FIFO_PTR_H

#include <lgk/fifo.h>

FIFO_STRUCT(fifo_ptr, void *, unsigned);

FIFO_INIT_HEADER(fifo_ptr);
FIFO_INIT_PREFILLED_HEADER(fifo_ptr);
FIFO_PUSH_HEADER(fifo_ptr);
FIFO_POP_HEADER(fifo_ptr);
FIFO_WRITE_HEADER(fifo_ptr);
FIFO_READ_HEADER(fifo_ptr);

#endif
