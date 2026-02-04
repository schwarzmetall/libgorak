#ifndef LGK_TIMESPEC_H
#define LGK_TIMESPEC_H

#include <time.h>

void timespec_offset_ms(struct timespec *ts, int offset_ms);
int timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms);

#endif
