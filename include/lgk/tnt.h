#ifndef LGK_TNT_H
#define LGK_TNT_H

#include <stddef.h>
#include <stdint.h>
#include <lgk/util.h>

#ifdef LGK_TNT_NO_ERRNO
    #define LGK_TNT_STRERROR(x) "failed"
#else
    #define LGK_TNT_STRERROR(x) strerror(x)
    #include <string.h>
    #include <errno.h>
#endif

#ifndef LGK_TNT_OUTPUT_LEVEL
    #ifdef NDEBUG
        #define LGK_TNT_OUTPUT_LEVEL TRACE_LEVEL_INFO
    #else
        #define LGK_TNT_OUTPUT_LEVEL TRACE_LEVEL_DEBUG
    #endif
#endif

#ifndef LGK_TNT_TRAP_LABEL_PREFIX
    #define LGK_TNT_TRAP_LABEL_PREFIX trap
#endif

#ifndef LGK_TNT_PRINT_DEFAULT
    #define LGK_TNT_PRINT_DEFAULT lgk_tnt_print_stderr
#endif

#ifndef LGK_TRAP_TRACE_LEVEL_DEFAULT
    #define LGK_TRAP_TRACE_LEVEL_DEFAULT TRACE_LEVEL_CRITICAL
#endif

#ifndef LGK_TNT_PRINT_SYSLOG_FACILITY_DEFAULT
    #define LGK_TNT_PRINT_SYSLOG_FACILITY_DEFAULT LOG_USER
#endif

#ifndef LGK_TNT_PRINT_SYSLOG_STRBUF_SIZE
    #define LGK_TNT_PRINT_SYSLOG_STRBUF_SIZE 1024
#endif

enum trace_level : int_fast8_t
{
    TRACE_LEVEL_EMERGENCY=0,
    TRACE_LEVEL_ALERT,
    TRACE_LEVEL_CRITICAL,
    TRACE_LEVEL_ERROR,
    TRACE_LEVEL_WARNING,
    TRACE_LEVEL_NOTICE,
    TRACE_LEVEL_INFO,
    TRACE_LEVEL_DEBUG
};
typedef void lgk_tnt_print_t(const char *file, unsigned line, const char *function, enum trace_level level, const char *format, ...);

extern lgk_tnt_print_t LGK_TNT_PRINT_DEFAULT;
extern int lgk_tnt_print_syslog_facility;
extern enum trace_level lgk_tnt_output_level_dynamic;

#ifdef LGK_TNT_OUTPUT_LEVEL_DYNAMIC
    #define LGK_TNT_OUTPUT_LEVEL_INTERNAL lgk_tnt_output_level_dynamic
#else
    #define LGK_TNT_OUTPUT_LEVEL_INTERNAL LGK_TNT_OUTPUT_LEVEL
#endif

#define TRACEP(level, print, ...) ((level<=LGK_TNT_OUTPUT_LEVEL_INTERNAL) ? print(__FILE__, __LINE__, __func__, level, __VA_ARGS__) : (void)0)
#define TRACE(level, ...) TRACEP(level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#define TRACEF(level, func, fmt, ...) TRACE(level, #func"(): " fmt, __VA_ARGS__)
#define TRACEFE(level, func) TRACEF(level, func, "%s", LGK_TNT_STRERROR(errno))
#define TRAPLP(cond, label, level, print, ...)\
    {\
        if(cond)\
        {\
            TRACEP(level, print, __VA_ARGS__);\
            goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
        }\
    }
#define TRAPL(cond, label, level, ...) TRAPLP(cond, label, level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#define TRAPP(cond, label, print, ...) TRAPLP(cond, label, LGK_TRAP_TRACE_LEVEL_DEFAULT, print, __VA_ARGS__)
#define TRAP(cond, label, ...) TRAPLP(cond, label, LGK_TRAP_TRACE_LEVEL_DEFAULT, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#ifndef LGK_TNT_NO_TRACE_SHORTS
    #define EMERG(...) TRACE(TRACE_LEVEL_EMERGENCY, __VA_ARGS__)
    #define ALERT(...) TRACE(TRACE_LEVEL_ALERT, __VA_ARGS__)
    #define CRIT(...) TRACE(TRACE_LEVEL_CRITICAL, __VA_ARGS__)
    #define ERR(...) TRACE(TRACE_LEVEL_ERROR, __VA_ARGS__)
    #define WARN(...) TRACE(TRACE_LEVEL_WARNING, __VA_ARGS__)
    #define NOTICE(...) TRACE(TRACE_LEVEL_NOTICE, __VA_ARGS__)
    #define INFO(...) TRACE(TRACE_LEVEL_INFO, __VA_ARGS__)
    #define DEBUG(...) TRACE(TRACE_LEVEL_DEBUG, __VA_ARGS__)
    #define CRITF(func, fmt, ...) CRIT(#func"(): " fmt, __VA_ARGS__)
    #define ERRF(func, fmt, ...) ERR(#func"(): " fmt, __VA_ARGS__)
    #define CRITFE(func) CRITF(func, "%s", LGK_TNT_STRERROR(errno))
    #define ERRFE(func) ERRF(func, "%s", LGK_TNT_STRERROR(errno))
    #define DEBUGV(value, fmt) DEBUG(#value"==" fmt, value)
#endif

#define TRAPNULL(ptr) TRAP(!ptr, ptr##_null, "null pointer: "#ptr)
#define TRAPNULL_L(ptr, label) TRAP(!ptr, label##_null, "null pointer: "#ptr)
#define TRAPVEQ(a, b, label) TRAP(a==b, label, #a"=="#b)
#define TRAPVNEQ(a, b, label) TRAP(a!=b, label, #a"!="#b)
#define TRAPF(cond, func, fmt, ...) TRAP(cond, func, #func"(): " fmt, __VA_ARGS__)
#define TRAPFE(cond, func) TRAPF(cond, func, "%s", LGK_TNT_STRERROR(errno))
#define TRAPFS(cond, func, suffix, fmt, ...) TRAP(cond, func##_##suffix, #func"(): " fmt, __VA_ARGS__)
#define TRAPFES(cond, func, suffix) TRAPFS(cond, func, suffix, "%s", LGK_TNT_STRERROR(errno))

#define TRAP_SILENT(cond, label)\
    {\
        if(cond)\
        {\
            goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
        }\
    }\

#endif
