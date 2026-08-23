#ifndef FIFO_INT_H
#define FIFO_INT_H

#include <lgk/fifo.h>

FIFO_STRUCT(fifo_int, int, unsigned);

FIFO_INIT_HEADER(fifo_int);
FIFO_INIT_PREFILLED_HEADER(fifo_int);
FIFO_PUSH_HEADER(fifo_int);
FIFO_POP_HEADER(fifo_int);
FIFO_READ_HEADER(fifo_int);
FIFO_WRITE_HEADER(fifo_int);

#endif
