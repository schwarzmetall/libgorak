#ifndef LGK_TIMESPEC_H
#define LGK_TIMESPEC_H

#include <stdint.h>
#include <time.h>

int_fast8_t timespec_offset_ms(struct timespec *ts, int offset_ms);
int_fast8_t timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms);

int64_t timespec_to_ms(const struct timespec *ts);
int_fast8_t timespec_get_ms(int base, int64_t *out_ms);

#endif
