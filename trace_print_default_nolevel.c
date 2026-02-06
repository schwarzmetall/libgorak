#include <stdarg.h>
#include <stdio.h>
#include <lgk_tnt.h>
#include <lgk_util.h>

void lgk_trace_print_default_nolevel(const char *file, unsigned line, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr,"%s:%d: ", file, line);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}
