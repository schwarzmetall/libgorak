#ifndef LGK_TIME_MS_H
#define LGK_TIME_MS_H

#include <stdint.h>
#include <time.h>

int_fast8_t timespec_offset_ms(struct timespec *ts, int offset_ms);
int_fast8_t timespec_get_offset_ms(struct timespec *ts, int base, int offset_ms);

int_fast64_t timespec_to_ms(const struct timespec *ts);
int_fast8_t time_ms(int base, int_fast64_t *out_ms);
int_fast8_t time_offset_ms(int base, int offset_ms, int_fast64_t *out_ms);

#endif
