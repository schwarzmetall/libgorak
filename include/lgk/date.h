#ifndef LGK_DATEUTIL_H
#define LGK_DATEUTIL_H

#include <stdint.h>

uint_fast8_t leap_year(uint_fast16_t year);
uint_fast8_t days_in_month(uint_fast16_t year, uint_fast8_t month);
uint_fast16_t days_in_year(uint_fast16_t year);
uint_fast16_t days_before_month(uint_fast16_t year, uint_fast8_t month);
int_fast8_t date_valid(uint_fast16_t year, uint_fast8_t month, uint_fast8_t day);

#endif
