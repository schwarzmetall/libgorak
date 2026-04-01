/* There is no fallback error reporting path: if fprintf/vfprintf to stderr fails, it is not reported elsewhere. */
#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include <lgk/tnt.h>
#include <lgk/util.h>
#include <tnt_strip_path.h>

void lgk_tnt_print_syslog(const char *file, unsigned line, const char *function, enum trace_level level, const char *format, ...)
{
    static const int level_syslog[] =
    {
        LOG_EMERG,
        LOG_ALERT,
        LOG_CRIT,
        LOG_ERR,
        LOG_WARNING,
        LOG_NOTICE,
        LOG_INFO,
        LOG_DEBUG
    };
    uint_fast8_t level_index = (uint_fast8_t)level;
    static const int level_syslog_fallback = LOG_CRIT;
    const int priority = ((level_index < ASIZE(level_syslog)) ? level_syslog[level_index] : level_syslog_fallback) | lgk_tnt_print_syslog_facility;
    va_list ap;
    va_start(ap, format);
    char strbuf[LGK_TNT_PRINT_SYSLOG_STRBUF_SIZE];
    int n = snprintf(strbuf, sizeof(strbuf), "%s:%d:%s(): ", lgk_tnt_strip_path(file), line, function);
    if((unsigned)n >= sizeof(strbuf)) return;
    vsnprintf(strbuf+n, sizeof(strbuf)-n, format, ap);
    syslog(priority, "%s", strbuf);
}
