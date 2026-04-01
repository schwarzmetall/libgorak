/* There is no fallback error reporting path: if fprintf/vfprintf to stderr fails, it is not reported elsewhere. */
#include <stdarg.h>
#include <stdio.h>
#include <lgk/tnt.h>
#include <lgk/util.h>

void lgk_tnt_print_stderr_nosource(const char *file, unsigned line, const char *function, enum trace_level level, const char *format, ...)
{
    (void)file;
    (void)line;
    (void)function;
    static const char *const level_tags[] =
    {
        "EMERGENCY",
        "ALERT    ",
        "CRITICAL ",
        "ERROR    ",
        "WARNING  ",
        "NOTICE   ",
        "INFO     ",
        "DEBUG    "
    };
    uint_fast8_t level_index = (uint_fast8_t)level;
    static const char *const tag_fallback = "INVALID  ";
    const char *const tag = (level_index < ASIZE(level_tags)) ? level_tags[level_index] : tag_fallback;
    va_list ap;
    va_start(ap, format);
    fprintf(stderr,"[%s] ", tag);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}
