#ifndef LGK_TNT_H
#define LGK_TNT_H

#include <stddef.h>
#include <stdint.h>
#include <lgk/util.h>

#ifndef LGK_TNT_NO_ERRNO
    #include <string.h>
    #include <errno.h>
#endif

#ifndef LGK_TNT_NO_THREADS
    /* Forward declaration (real declaration in <lgk/threads.h>). Avoids
     * pulling <stdatomic.h>/<threads.h> into every user of tnt.h. */
    const char *lgk_thrdstrerror(int thrd_status);
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

/* Macro suffix convention
 * -----------------------
 *   <PREFIX><position-letters><modifiers><operation>
 *
 * Position letters describe the leading parameters left-to-right:
 *   V  bare identifier of a variable; the macro uses it both as a
 *      trap-label fragment (for goto) and as the printed name, e.g.
 *      "myvar==<value>"
 *   X  arbitrary expression; the macro stringifies it for the message
 *      and the caller must pass an explicit label fragment
 *   F  bare identifier of a function call; the macro uses it both as
 *      a trap-label fragment and renders the message as "func()==<value>"
 *
 * Modifier letters come after the position letters:
 *   E  annotate the printed value with strerror(errno) when errno is
 *      available; same signature as the unmodified F-macro (takes
 *      value, fmt). Under LGK_TNT_NO_ERRNO only the value is printed,
 *      so call sites stay portable.
 *   T  annotate the printed status (int) with lgk_thrdstrerror(status)
 *      when C11 thread support is available; takes a status arg
 *      (always int, no fmt). Under LGK_TNT_NO_THREADS only the int
 *      is printed.
 *   S  append a caller-supplied suffix to the auto-generated label;
 *      use when the same function is trapped more than once in a
 *      single function (each trap site needs a distinct label)
 *
 * Output of an E-annotated macro: "func()==<value>: <strerror(errno)>"
 * Output of a T-annotated macro:  "func()==<status>: <thrdstrerror>"
 * (the ": <str>" tail is dropped under the corresponding NO_ option).
 *
 * When several modifiers combine, list them alphabetically (E, T, S),
 * e.g. TRAPFES, TRAPFTS.
 *
 * Operation suffix (last): NULL, EQ, NEQ, GT, LT, GTE, LTE, OOR, INR.
 * F-traps carry no operation suffix; they trap on an opaque cond.
 *
 * Ordering rules within position letters:
 *   - V precedes X when both are present (no TRAPXV* macros exist)
 *   - For 3-arg range macros: VVX, VXV, VXX, VVV (V-positions first
 *     where natural)
 *
 * Examples:
 *   DEBUGV(count, "u")            -> "count==<n>"
 *   CRITF(open, fd, "i")          -> "open()==<fd>"
 *   CRITFE(read, n, "zi")         -> "read()==<n>: <strerror(errno)>"
 *   TRAPVNULL(p)                  trap if p is NULL, label trap_p_null
 *   TRAPVVEQ(a, b, "i")           trap if a==b, label trap_a_eq_b
 *   TRAPVXNEQ(v, e, lbl, "i")     v auto-labels, e needs lbl
 *   TRAPVVVOOR(x, lo, hi, "i")    range check, all auto-labelled
 *   TRAPF(cond, foo, x, "i")      trap if cond, prints "foo()==<x>"
 *   TRAPFE(fd<0, open, fd, "i")   prints "open()==<fd>: <strerror>"
 *   TRAPFES(c, foo, retry, r, "i")
 *                                 errno + suffix; label trap_foo_retry
 *   TRAPFT(s!=thrd_success, mtx_lock, s)
 *                                 prints "mtx_lock()==<s>: <thrdstrerror>"
 */

#define TRACEP(level, print, ...) ((level<=LGK_TNT_OUTPUT_LEVEL_INTERNAL) ? print(__FILE__, __LINE__, __func__, level, __VA_ARGS__) : (void)0)
#define TRACE(level, ...) TRACEP(level, LGK_TNT_PRINT_DEFAULT, __VA_ARGS__)
#define TRACEV(level, value, fmt) TRACE(level, #value"==%" fmt, (value))
#define TRACEF(level, func, value, fmt) TRACE(level, #func"()==%" fmt, (value))
#ifdef LGK_TNT_NO_ERRNO
    #define TRACEFE(level, func, value, fmt) TRACE(level, #func"()==%" fmt, (value))
#else
    #define TRACEFE(level, func, value, fmt) TRACE(level, #func"()==%" fmt ": %s", (value), strerror(errno))
#endif
#ifdef LGK_TNT_NO_THREADS
    #define TRACEFT(level, func, status) TRACE(level, #func"()==%i", (status))
#else
    #define TRACEFT(level, func, status) TRACE(level, #func"()==%i: %s", (status), lgk_thrdstrerror(status))
#endif
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
    #define EMERG(...)  TRACE(TRACE_LEVEL_EMERGENCY, __VA_ARGS__)
    #define ALERT(...)  TRACE(TRACE_LEVEL_ALERT,     __VA_ARGS__)
    #define CRIT(...)   TRACE(TRACE_LEVEL_CRITICAL,  __VA_ARGS__)
    #define ERR(...)    TRACE(TRACE_LEVEL_ERROR,     __VA_ARGS__)
    #define WARN(...)   TRACE(TRACE_LEVEL_WARNING,   __VA_ARGS__)
    #define NOTICE(...) TRACE(TRACE_LEVEL_NOTICE,    __VA_ARGS__)
    #define INFO(...)   TRACE(TRACE_LEVEL_INFO,      __VA_ARGS__)
    #define DEBUG(...)  TRACE(TRACE_LEVEL_DEBUG,     __VA_ARGS__)

    #define EMERGV(value, fmt)  EMERG(#value"==%" fmt, (value))
    #define ALERTV(value, fmt)  ALERT(#value"==%" fmt, (value))
    #define CRITV(value, fmt)   CRIT(#value"==%" fmt, (value))
    #define ERRV(value, fmt)    ERR(#value"==%" fmt, (value))
    #define WARNV(value, fmt)   WARN(#value"==%" fmt, (value))
    #define NOTICEV(value, fmt) NOTICE(#value"==%" fmt, (value))
    #define INFOV(value, fmt)   INFO(#value"==%" fmt, (value))
    #define DEBUGV(value, fmt)  DEBUG(#value"==%" fmt, (value))

    #define EMERGF(func, value, fmt)  EMERG(#func"()==%" fmt, (value))
    #define ALERTF(func, value, fmt)  ALERT(#func"()==%" fmt, (value))
    #define CRITF(func, value, fmt)   CRIT(#func"()==%" fmt, (value))
    #define ERRF(func, value, fmt)    ERR(#func"()==%" fmt, (value))
    #define WARNF(func, value, fmt)   WARN(#func"()==%" fmt, (value))
    #define NOTICEF(func, value, fmt) NOTICE(#func"()==%" fmt, (value))
    #define INFOF(func, value, fmt)   INFO(#func"()==%" fmt, (value))
    #define DEBUGF(func, value, fmt)  DEBUG(#func"()==%" fmt, (value))

#ifdef LGK_TNT_NO_ERRNO
    #define EMERGFE(func, value, fmt)  EMERG(#func"()==%" fmt, (value))
    #define ALERTFE(func, value, fmt)  ALERT(#func"()==%" fmt, (value))
    #define CRITFE(func, value, fmt)   CRIT(#func"()==%" fmt, (value))
    #define ERRFE(func, value, fmt)    ERR(#func"()==%" fmt, (value))
    #define WARNFE(func, value, fmt)   WARN(#func"()==%" fmt, (value))
    #define NOTICEFE(func, value, fmt) NOTICE(#func"()==%" fmt, (value))
    #define INFOFE(func, value, fmt)   INFO(#func"()==%" fmt, (value))
    #define DEBUGFE(func, value, fmt)  DEBUG(#func"()==%" fmt, (value))
#else
    #define EMERGFE(func, value, fmt)  EMERG(#func"()==%" fmt ": %s",  (value), strerror(errno))
    #define ALERTFE(func, value, fmt)  ALERT(#func"()==%" fmt ": %s",  (value), strerror(errno))
    #define CRITFE(func, value, fmt)   CRIT(#func"()==%" fmt ": %s",   (value), strerror(errno))
    #define ERRFE(func, value, fmt)    ERR(#func"()==%" fmt ": %s",    (value), strerror(errno))
    #define WARNFE(func, value, fmt)   WARN(#func"()==%" fmt ": %s",   (value), strerror(errno))
    #define NOTICEFE(func, value, fmt) NOTICE(#func"()==%" fmt ": %s", (value), strerror(errno))
    #define INFOFE(func, value, fmt)   INFO(#func"()==%" fmt ": %s",   (value), strerror(errno))
    #define DEBUGFE(func, value, fmt)  DEBUG(#func"()==%" fmt ": %s",  (value), strerror(errno))
#endif

#ifdef LGK_TNT_NO_THREADS
    #define EMERGFT(func, status)  EMERG(#func"()==%i",  (status))
    #define ALERTFT(func, status)  ALERT(#func"()==%i",  (status))
    #define CRITFT(func, status)   CRIT(#func"()==%i",   (status))
    #define ERRFT(func, status)    ERR(#func"()==%i",    (status))
    #define WARNFT(func, status)   WARN(#func"()==%i",   (status))
    #define NOTICEFT(func, status) NOTICE(#func"()==%i", (status))
    #define INFOFT(func, status)   INFO(#func"()==%i",   (status))
    #define DEBUGFT(func, status)  DEBUG(#func"()==%i",  (status))
#else
    #define EMERGFT(func, status)  EMERG(#func"()==%i: %s",  (status), lgk_thrdstrerror(status))
    #define ALERTFT(func, status)  ALERT(#func"()==%i: %s",  (status), lgk_thrdstrerror(status))
    #define CRITFT(func, status)   CRIT(#func"()==%i: %s",   (status), lgk_thrdstrerror(status))
    #define ERRFT(func, status)    ERR(#func"()==%i: %s",    (status), lgk_thrdstrerror(status))
    #define WARNFT(func, status)   WARN(#func"()==%i: %s",   (status), lgk_thrdstrerror(status))
    #define NOTICEFT(func, status) NOTICE(#func"()==%i: %s", (status), lgk_thrdstrerror(status))
    #define INFOFT(func, status)   INFO(#func"()==%i: %s",   (status), lgk_thrdstrerror(status))
    #define DEBUGFT(func, status)  DEBUG(#func"()==%i: %s",  (status), lgk_thrdstrerror(status))
#endif
#endif

#define TRAPXNULL(expr, label) TRAP(!(expr), label##_null, #expr" == NULL")
#define TRAPVNULL(var) TRAPXNULL(var, var)

#define TRAPXXEQ(a, b, labela, labelb, fmt) TRAP((a)==(b), labela##_eq_##labelb, "("#a")==("#b")==%"fmt, (a))
#define TRAPXXNEQ(a, b, labela, labelb, fmt) TRAP((a)!=(b), labela##_neq_##labelb, "("#a")==%"fmt" != ("#b")==%"fmt, (a), (b))
#define TRAPXXGT(a, b, labela, labelb, fmt) TRAP((a)>(b), labela##_gt_##labelb, "("#a")==%"fmt" > ("#b")==%"fmt, (a), (b))
#define TRAPXXLT(a, b, labela, labelb, fmt) TRAP((a)<(b), labela##_lt_##labelb, "("#a")==%"fmt" < ("#b")==%"fmt, (a), (b))
#define TRAPXXGTE(a, b, labela, labelb, fmt) TRAP((a)>=(b), labela##_gte_##labelb, "("#a")==%"fmt" >= ("#b")==%"fmt, (a), (b))
#define TRAPXXLTE(a, b, labela, labelb, fmt) TRAP((a)<=(b), labela##_lte_##labelb, "("#a")==%"fmt" <= ("#b")==%"fmt, (a), (b))
#define TRAPVXEQ(var, expr, labelx, fmt) TRAPXXEQ(var, expr, var, labelx, fmt)
#define TRAPVXNEQ(var, expr, labelx, fmt) TRAPXXNEQ(var, expr, var, labelx, fmt)
#define TRAPVXGT(var, expr, labelx, fmt) TRAPXXGT(var, expr, var, labelx, fmt)
#define TRAPVXLT(var, expr, labelx, fmt) TRAPXXLT(var, expr, var, labelx, fmt)
#define TRAPVXGTE(var, expr, labelx, fmt) TRAPXXGTE(var, expr, var, labelx, fmt)
#define TRAPVXLTE(var, expr, labelx, fmt) TRAPXXLTE(var, expr, var, labelx, fmt)
#define TRAPXVEQ(expr, var, labelx, fmt) TRAPXXEQ(expr, var, labelx, var, fmt)
#define TRAPXVNEQ(expr, var, labelx, fmt) TRAPXXNEQ(expr, var, labelx, var, fmt)
#define TRAPXVGT(expr, var, labelx, fmt) TRAPXXGT(expr, var, labelx, var, fmt)
#define TRAPXVLT(expr, var, labelx, fmt) TRAPXXLT(expr, var, labelx, var, fmt)
#define TRAPXVGTE(expr, var, labelx, fmt) TRAPXXGTE(expr, var, labelx, var, fmt)
#define TRAPXVLTE(expr, var, labelx, fmt) TRAPXXLTE(expr, var, labelx, var, fmt)
#define TRAPVVEQ(a, b, fmt) TRAPVXEQ(a, b, b, fmt)
#define TRAPVVNEQ(a, b, fmt) TRAPVXNEQ(a, b, b, fmt)
#define TRAPVVGT(a, b, fmt) TRAPVXGT(a, b, b, fmt)
#define TRAPVVLT(a, b, fmt) TRAPVXLT(a, b, b, fmt)
#define TRAPVVGTE(a, b, fmt) TRAPVXGTE(a, b, b, fmt)
#define TRAPVVLTE(a, b, fmt) TRAPVXLTE(a, b, b, fmt)

#define TRAPXXXOOR(x, xmin, xmax, labelx, labelmin, labelmax, fmt) TRAP(((x)<(xmin))||((x)>(xmax)), labelx##_oor_##labelmin##_##labelmax, "("#x")==%"fmt" out of range [%"fmt",%"fmt"]", (x), (xmin), (xmax))
#define TRAPVXXOOR(v, xmin, xmax, labelmin, labelmax, fmt) TRAPXXXOOR(v, xmin, xmax, v, labelmin, labelmax, fmt)
#define TRAPVVXOOR(v, vmin, xmax, labelmax, fmt) TRAPVXXOOR(v, vmin, xmax, vmin, labelmax, fmt)
#define TRAPVXVOOR(v, xmin, vmax, labelmin, fmt) TRAPVXXOOR(v, xmin, vmax, labelmin, vmax, fmt)
#define TRAPVVVOOR(v, vmin, vmax, fmt) TRAPVXXOOR(v, vmin, vmax, vmin, vmax, fmt)\

#define TRAPXXXINR(x, xmin, xmax, labelx, labelmin, labelmax, fmt) TRAP(((x)>=(xmin))&&((x)<=(xmax)), labelx##_inr_##labelmin##_##labelmax, "("#x")==%"fmt" in range [%"fmt",%"fmt"]", (x), (xmin), (xmax))
#define TRAPVXXINR(v, xmin, xmax, labelmin, labelmax, fmt) TRAPXXXINR(v, xmin, xmax, v, labelmin, labelmax, fmt)
#define TRAPVVXINR(v, vmin, xmax, labelmax, fmt) TRAPVXXINR(v, vmin, xmax, vmin, labelmax, fmt)
#define TRAPVXVINR(v, xmin, vmax, labelmin, fmt) TRAPVXXINR(v, xmin, vmax, labelmin, vmax, fmt)
#define TRAPVVVINR(v, vmin, vmax, fmt) TRAPVXXINR(v, vmin, vmax, vmin, vmax, fmt)\

#define TRAPF(cond, func, value, fmt) TRAP(cond, func, #func"()==%" fmt, (value))
#define TRAPFS(cond, func, suffix, value, fmt) TRAP(cond, func##_##suffix, #func"()==%" fmt, (value))
#ifdef LGK_TNT_NO_ERRNO
    #define TRAPFE(cond, func, value, fmt) TRAP(cond, func, #func"()==%" fmt, (value))
    #define TRAPFES(cond, func, suffix, value, fmt) TRAP(cond, func##_##suffix, #func"()==%" fmt, (value))
#else
    #define TRAPFE(cond, func, value, fmt) TRAP(cond, func, #func"()==%" fmt ": %s", (value), strerror(errno))
    #define TRAPFES(cond, func, suffix, value, fmt) TRAP(cond, func##_##suffix, #func"()==%" fmt ": %s", (value), strerror(errno))
#endif
#ifdef LGK_TNT_NO_THREADS
    #define TRAPFT(cond, func, status) TRAP(cond, func, #func"()==%i", (status))
    #define TRAPFTS(cond, func, suffix, status) TRAP(cond, func##_##suffix, #func"()==%i", (status))
#else
    #define TRAPFT(cond, func, status) TRAP(cond, func, #func"()==%i: %s", (status), lgk_thrdstrerror(status))
    #define TRAPFTS(cond, func, suffix, status) TRAP(cond, func##_##suffix, #func"()==%i: %s", (status), lgk_thrdstrerror(status))
#endif

#define TRAP_SILENT(cond, label)\
    {\
        if(cond)\
        {\
            goto JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, label));\
        }\
    }\

#endif
