#ifndef LGK_TNT_H
#define LGK_TNT_H

#include <stdint.h>
#include <lgk_util.h>

#ifdef LGK_TNT_NO_ERRNO
    #define LGK_TNT_STRERROR(x) "failed"
#else
    #include <string.h>
    #include <errno.h>
    #define LGK_TNT_STRERROR(x) strerror(x)
#endif

#ifndef LGK_TNT_LABEL_PREFIX
    #define LGK_TNT_LABEL_PREFIX trap
#endif

#ifndef LGK_TNT_FULL_PATH
    #define LGK_TNT_FILE_NAME_MANGLE(x) lgk_tnt_strip_path(x)
const char *lgk_tnt_strip_path(const char *path);
#else
    #define LGK_TNT_FILE_NAME_MANGLE(x) (x)
#endif

#ifdef LGK_TNT_PRINT_DEFAULT_OVERRIDE
    #define LGK_TNT_PRINT_DEFAULT LGK_TNT_PRINT_DEFAULT_OVERRIDE
#else
    #ifndef LGK_TNT_NO_LEVEL
        #define LGK_TNT_PRINT_DEFAULT lgk_tnt_print_default
    #else
        #define LGK_TNT_PRINT_DEFAULT lgk_tnt_print_default_nolevel
    #endif
#endif

#ifdef LGK_TNT_TRAP_LABEL_PREFIX_OVERRIDE
    #define LGK_TNT_TRAP_LABEL_PREFIX LGK_TNT_TRAP_LABEL_PREFIX_OVERRIDE
#else
    #define LGK_TNT_TRAP_LABEL_PREFIX trap
#endif

enum trace_level : uint_fast8_t
{
    TRACE_LEVEL_CRITICAL=0,
    TRACE_LEVEL_ERROR,
    TRACE_LEVEL_WARNING,
    TRACE_LEVEL_INFO,
    TRACE_LEVEL_DEBUG
};
typedef void lgk_tnt_print_t(const char *file, unsigned line, enum trace_level level, const char *format, ...);

lgk_tnt_print_t lgk_tnt_print_default;
lgk_tnt_print_t lgk_tnt_print_default_nolevel;

#ifdef LGK_TNT_OUTPUT_LEVEL_DYNAMIC
    #define LGK_TNT_OUTPUT_LEVEL trace_output_level
extern volatile unsigned trace_output_level;
#else
    #ifdef LGK_TNT_OUTPUT_LEVEL_STATIC
	#define LGK_TNT_OUTPUT_LEVEL JOIN(TRACE_LEVEL_,LGK_TNT_OUTPUT_LEVEL_STATIC)
    #else
        #define LGK_TNT_OUTPUT_LEVEL TRACE_LEVEL_DEBUG
    #endif
#endif

#define TRACEP(level, print, ...) if(level<=LGK_TNT_OUTPUT_LEVEL) print(LGK_TNT_FILE_NAME_MANGLE(__FILE__), __LINE__, level, __VA_ARGS__)
#define TRACE(level, ...) TRACEP(level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#define TRACEF(level, func, fmt, ...) TRACE(level, #func"(): "fmt, __VA_ARGS__)
#define TRACEFE(level, func) TRACEF(level, func, "%s", LGK_TNT_STRERROR(errno))
#define TRAPLP(cond, label, level, print, ...)\
    if(cond)\
    {\
        TRACEP(level, print, __VA_ARGS__);\
        goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
    }
#define TRAPL(cond, label, level, ...) TRAPLP(cond, label, level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#define TRAPP(cond, label, print, ...) TRAPLP(cond, label, TRACE_LEVEL_CRITICAL, print, __VA_ARGS__)
#define TRAP(cond, label, ...) TRAPLP(cond, label, TRACE_LEVEL_CRITICAL, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#ifndef LGK_TNT_NO_TRACE_SHORTS
    #define CRIT(...) TRACE(TRACE_LEVEL_CRITICAL, __VA_ARGS__)
    #define ERR(...) TRACE(TRACE_LEVEL_ERROR, __VA_ARGS__)
    #define WARN(...) TRACE(TRACE_LEVEL_WARNING, __VA_ARGS__)
    #define INFO(...) TRACE(TRACE_LEVEL_INFO, __VA_ARGS__)
    #define DEBUG(...) TRACE(TRACE_LEVEL_DEBUG, __VA_ARGS__)
    #define CRITF(func, fmt, ...) CRIT(#func"(): "fmt, __VA_ARGS__)
    #define ERRF(func, fmt, ...) ERR(#func"(): "fmt, __VA_ARGS__)
    #define CRITFE(func) CRITF(func, "%s", LGK_TNT_STRERROR(errno))
    #define ERRFE(func) ERRF(func, "%s", LGK_TNT_STRERROR(errno))
    #define DEBUGV(value, fmt) DEBUG(#value"=="fmt, value)
#endif

#define TRAPNULL(ptr) TRAP(!ptr, ptr##_null, "null pointer: "#ptr)
#define TRAPNULL_L(ptr, label) TRAP(!ptr, label##_null, "null pointer: "#ptr)
#define TRAPVEQ(a, b, label) TRAP(a==b, label, #a"=="#b)
#define TRAPVNEQ(a, b, label) TRAP(a!=b, label, #a"!="#b)
#define TRAPF(cond, func, fmt, ...) TRAP(cond, func, #func"(): "fmt, __VA_ARGS__)
#define TRAPFE(cond, func) TRAPF(cond, func, "%s", LGK_TNT_STRERROR(errno))
#define TRAPFS(cond, func, suffix, fmt, ...) TRAP(cond, func##_##suffix, #func"(): "fmt, __VA_ARGS__)
#define TRAPFES(cond, func, suffix) TRAPFS(cond, func, suffix, "%s", LGK_TNT_STRERROR(errno))

#define TRAP_SILENT(cond, label)\
    if(cond)\
    {\
        goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
    }

#endif
