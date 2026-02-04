#ifndef LGK_THREADS_H
#define LGK_THREADS_H

#include <threads.h>

int mtx_timedlock_ms(mtx_t *restrict mutex, unsigned timeout_ms);

#endif
