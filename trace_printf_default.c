#include <stdarg.h>
#include <stdio.h>
#include <lgk_tnt.h>
#include <lgk_util.h>

void lgk_trace_printf_default(const char *file, unsigned line, enum trace_level level, const char *format, ...)
{
    static const char *const level_tags[] = {"TRP", "ERR", "WRN", "INF", "DBG"};
    static const char *const tag_fallback = "INV";
    const char *const tag = (level < ASIZE(level_tags)) ? level_tags[level] : tag_fallback;
    va_list ap;
    va_start(ap, format);
    fprintf(stderr,"[%s] %s:%d: ", tag, file, line);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}
