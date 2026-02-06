#ifndef LGK_TNT_H
#define LGK_TNT_H

#include <stdint.h>
#include <lgk_util.h>

#ifndef LGK_TNT_NO_ERRNO
    #include <string.h>
    #include <errno.h>
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
    TRACE_LEVEL_TRAP=0,
    TRACE_LEVEL_ERROR,
    TRACE_LEVEL_WARNING,
    TRACE_LEVEL_INFO,
    TRACE_LEVEL_DEBUG
};
typedef void lgk_tnt_print_t(const char *file, unsigned line, enum trace_level level, const char *format, ...);
typedef void lgk_tnt_print_nolevel_t(const char *file, unsigned line, const char *format, ...);

#ifndef LGK_TNT_NO_LEVEL
lgk_tnt_print_t LGK_TNT_PRINT_DEFAULT;
#else
lgk_tnt_print_nolevel_t LGK_TNT_PRINT_DEFAULT;
#endif

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

#ifndef LGK_TNT_NO_LEVEL
    #define TRACEP(level, print, ...) if(level<=LGK_TNT_OUTPUT_LEVEL) print(LGK_TNT_FILE_NAME_MANGLE(__FILE__), __LINE__, level, __VA_ARGS__)
    #define TRACE(level, ...) TRACEP(level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
    #define TRAPLP(cond, label, level, print, ...)\
        if(cond)\
        {\
            TRACEP(level, print, __VA_ARGS__);\
            goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
        }
    #define TRAPL(cond, label, level, ...) TRAPLP(cond, label, level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
    #define TRAPP(cond, label, print, ...) TRAPLP(cond, label, TRACE_LEVEL_TRAP, print, __VA_ARGS__)
    #define TRAP(cond, label, ...) TRAPLP(cond, label, TRACE_LEVEL_TRAP, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
    #ifndef LGK_TNT_NO_LEVEL_SHORTS
        #define DBG(...) TRACE(TRACE_LEVEL_DEBUG, __VA_ARGS__)
        #define INF(...) TRACE(TRACE_LEVEL_INFO, __VA_ARGS__)
        #define WRN(...) TRACE(TRACE_LEVEL_WARNING, __VA_ARGS__)
        #define ERR(...) TRACE(TRACE_LEVEL_ERROR, __VA_ARGS__)
        #define DBGVAR(var, fmt) DBG(#var"=="fmt, var)
    #endif
#else
    #define TRACEP(print, ...) print(LGK_TNT_FILE_NAME_MANGLE(__FILE__), __LINE__, __VA_ARGS__)
    #define TRACE(...) TRACEP(LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
    #define TRAPP(cond, label, print, ...)\
        if(cond)\
        {\
            TRACEP(print, __VA_ARGS__);\
            goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
        }
    #define TRAP(cond, label, ...) TRAPP(cond, label, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#endif

#define TRAPNULL(ptr) TRAP(!ptr, ptr##_null, "null pointer: "#ptr)
#define TRAPF(cond, func, fmt, ...) TRAP(cond, func, #func"(): "fmt, __VA_ARGS__)
#define TRAPFS(cond, func, suffix, fmt, ...) TRAP(cond, func##_##suffix, #func"(): "fmt, __VA_ARGS__)

#ifndef LGK_TNT_NO_ERRNO
    #define TRAPFE(cond, func) TRAPF(cond, func, "%s", strerror(errno))
    #define TRAPFES(cond, func, suffix) TRAPFS(cond, func, suffix, "%s", strerror(errno))
#endif

#define TRAP_SILENT(cond, label)\
    if(cond)\
    {\
        goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
    }

#endif
