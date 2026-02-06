#ifndef LGK_TNT_H
#define LGK_TNT_H

#include <stdint.h>

#ifndef LGK_TRAP_NO_ERRNO
    #include <string.h>
    #include <errno.h>
#endif

#define JOIN(x,y) JOIN_AGAIN(x,y)
#define JOIN_AGAIN(x,y) x##y

#ifndef LGK_TRAP_LABEL_PREFIX
    #define LGK_TRAP_LABEL_PREFIX trap
#endif

#ifndef LGK_TRACE_FULL_PATH
    #define TRACE_FILE_NAME_MANGLE(x) trace_strip_path(x)
const char *trace_strip_path(const char *path);
#else
    #define TRACE_FILE_NAME_MANGLE(x) (x)
#endif


#ifndef LGK_TRACE_PRINTF_DEFAULT
    #define LGK_TRACE_PRINTF_DEFAULT lgk_trace_printf_default
#endif
#ifndef LGK_TRACE_PRINTF_DEFAULT_LEVEL
    #define LGK_TRACE_PRINTF_DEFAULT_LEVEL lgk_trace_printf_default_level
#endif

enum trace_level : uint_fast8_t
{
    TRACE_LEVEL_TRAP=0,
    TRACE_LEVEL_ERROR,
    TRACE_LEVEL_WARNING,
    TRACE_LEVEL_INFO,
    TRACE_LEVEL_DEBUG
};
typedef void trace_printf_nolevel_t(const char *file, unsigned line, const char *format, ...);
typedef void trace_printf_t(const char *file, unsigned line, enum trace_level level, const char *format, ...);

trace_printf_nolevel_t LGK_TRACE_PRINTF_NOLEVEL_DEFAULT;
trace_printf_t LGK_TRACE_PRINTF_DEFAULT;

#ifdef TRACE_OUTPUT_LEVEL_DYNAMIC
    #define TRACE_OUTPUT_LEVEL trace_output_level
extern volatile unsigned trace_output_level;
#else
    #ifndef TRACE_OUTPUT_LEVEL
        #define TRACE_OUTPUT_LEVEL TRACE_LEVEL_DEBUG
    #endif
#endif

#ifndef TRACE_NO_LEVEL_SUPPORT
    #define TRACEP(level, printf, ...) if(level<=TRACE_OUTPUT_LEVEL) printf(TRACE_FILE_NAME_MANGLE(__FILE__), __LINE__, level, __VA_ARGS__)
    #define TRACE(level, ...) TRACEP(level, LGK_TRACE_PRINTF_DEFAULT, __VA_ARGS__)
    #define TRAPLP(cond, label, level, printf, ...)\
        if(cond)\
        {\
            TRACEP(level, printf, __VA_ARGS__);\
            goto JOIN(LGK_TRAP_LABEL_PREFIX, _##label);\
        }
    #define TRAPL(cond, label, level, ...) TRAPLP(cond, label, level, LGK_TRACE_PRINTF_DEFAULT, __VA_ARGS__)
    #define TRAPP(cond, label, printf, ...) TRAPLP(cond, label, TRACE_LEVEL_TRAP, printf, __VA_ARGS__)
    #define TRAP(cond, label, ...) TRAPLP(cond, label, TRACE_LEVEL_TRAP, LGK_TRACE_PRINTF_DEFAULT, __VA_ARGS__)
    #ifndef TRACE_NO_LEVEL_SHORTS
        #define DBG(...) TRACE(TRACE_LEVEL_DEBUG, __VA_ARGS__)
        #define INF(...) TRACE(TRACE_LEVEL_INFO, __VA_ARGS__)
        #define WRN(...) TRACE(TRACE_LEVEL_WARNING, __VA_ARGS__)
        #define ERR(...) TRACE(TRACE_LEVEL_ERROR, __VA_ARGS__)
        #define DBGVAR(var, fmt) DBG(#var"=="fmt, var)
    #endif
#else
    #define TRACEP(printf, ...) printf(TRACE_FILE_NAME_MANGLE(__FILE__), __LINE__, __VA_ARGS__)
    #define TRACE(...) TRACEP(LGK_TRACE_PRINTF_NOLEVEL_DEFAULT, __VA_ARGS__)
    #define TRAPP(cond, label, printf, ...)\
        if(cond)\
        {\
            TRACEP(printf, __VA_ARGS__);\
            goto TRAP_LABEL_PREFIX##_##label;\
        }
    #define TRAP(cond, label, ...) TRAPP(cond, label, LGK_TRACE_PRINTF_NOLEVEL_DEFAULT, __VA_ARGS__)
#endif

#define TRAPNULL(ptr) TRAP(!ptr, ptr##_null, "null pointer: "#ptr)
#define TRAPF(cond, func, fmt, ...) TRAP(cond, func, #func"(): "fmt, __VA_ARGS__)
#define TRAPFS(cond, func, suffix, fmt, ...) TRAP(cond, func##_##suffix, #func"(): "fmt, __VA_ARGS__)

#ifndef LGK_TRAP_NO_ERRNO
    #define TRAPFE(cond, func) TRAPF(cond, func, "%s", strerror(errno))
    #define TRAPFES(cond, func, suffix) TRAPFS(cond, func, suffix, "%s", strerror(errno))
#endif

#define TRAP_SILENT(cond, label)\
    if(cond)\
    {\
        goto TRAP_LABEL_PREFIX##_##label;\
    }

#endif
