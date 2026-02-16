#include <stdint.h>
#include <lgk_date.h>

uint_fast8_t leap_year(uint_fast16_t year)
{
    if(year % 4) return 0;
    if(!(year % 400)) return 1;
    if(!(year % 100)) return 0;
    return 1;
}

/* NOTE: zero-based (jan^=0, feb^=1) */
uint_fast8_t days_in_month(uint_fast16_t year, uint_fast8_t month)
{
    constexpr uint_fast8_t days_in_month_nonleap[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(month > 11) return 0;
    return days_in_month_nonleap[month] + ((month==1) ? leap_year(year) : 0);
}

uint_fast16_t days_in_year(uint_fast16_t year)
{
    return 365 + leap_year(year);
}

/* NOTE: zero-based (jan^=0, feb^=1) */
uint_fast16_t days_before_month(uint_fast16_t year, uint_fast8_t month)
{
    constexpr uint_fast16_t days_before_month_nonleap[11] =
    {
        31,
        31+28,
        31+28+31,
        31+28+31+30,
        31+28+31+30+31,
        31+28+31+30+31+30,
        31+28+31+30+31+30+31,
        31+28+31+30+31+30+31+31,
        31+28+31+30+31+30+31+31+30,
        31+28+31+30+31+30+31+31+30+31,
        31+28+31+30+31+30+31+31+30+31+30
    };
    if(!month) return 0;
    if(month > 11) return 0;
    return days_before_month_nonleap[month-1] + leap_year(year);
}

int_fast8_t date_valid(uint_fast16_t year, uint_fast8_t month, uint_fast8_t day)
{
    if(!year) return 0;
    if((!month) || (month>11)) return 0;
    if(!day) return 0;
    return (day < days_in_month(year, month));
}

int_fast8_t date_valid_human(uint_fast16_t year, uint_fast8_t month, uint_fast8_t day)
{
    return date_valid(year, month-1, day-1);
}
