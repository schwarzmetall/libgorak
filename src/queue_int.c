#include <lgk/queue.h>
#include <lgk/queue_int.h>

QUEUE_INIT(int, unsigned, queue_int)
QUEUE_INIT_PREFILLED(int, unsigned, queue_int)
QUEUE_CLOSE(int, unsigned, queue_int)
QUEUE_PUSH(int, unsigned, queue_int)
QUEUE_POP(int, unsigned, queue_int)
QUEUE_TRYPUSH(int, unsigned, queue_int)
QUEUE_TRYPOP(int, unsigned, queue_int)
