/*
 * TNT configuration and functional tests.
 * Compiled under multiple #define configurations to verify build correctness.
 * Run: ctest (from build dir) or cmake --build build && ctest --test-dir build
 */

#include "test.h"
#include <string.h>
#include <lgk/tnt.h>

/* Generates the correct goto label regardless of LGK_TNT_TRAP_LABEL_PREFIX */
#define TRAP_LABEL(name) JOIN(LGK_TNT_TRAP_LABEL_PREFIX, JOIN(_, name))

extern const char *lgk_tnt_strip_path(const char *path);

static int trace_count;

static void counting_print(const char *file, unsigned line, const char *function,
                           enum trace_level level, const char *format, ...)
{
    (void)file; (void)line; (void)function; (void)level; (void)format;
    trace_count++;
}

static void test_trace_calls_print(void)
{
    trace_count = 0;
    TRACEP(TRACE_LEVEL_EMERGENCY, counting_print, "emerg %d", 1);
    test_assert(trace_count == 1);
}

static void test_trace_level_filtering(void)
{
    trace_count = 0;
    TRACEP(TRACE_LEVEL_EMERGENCY, counting_print, "emerg %d", 1);
    test_assert(trace_count == 1);

    trace_count = 0;
    TRACEP(TRACE_LEVEL_DEBUG, counting_print, "debug %d", 1);
    if (TRACE_LEVEL_DEBUG <= LGK_TNT_OUTPUT_LEVEL_INTERNAL)
        test_assert(trace_count == 1);
    else
        test_assert(trace_count == 0);
}

static void test_trap_jumps_on_true(void)
{
    int jumped = 0;
    TRAPLP(1, jump1, TRACE_LEVEL_CRITICAL, counting_print, "trap %d", 1);
    test_assert(0 && "should not reach past TRAP");
    TRAP_LABEL(jump1):
    jumped = 1;
    test_assert(jumped == 1);
}

static void test_trap_nojump_on_false(void)
{
    int jumped = 0;
    TRAPLP(0, jump2, TRACE_LEVEL_CRITICAL, counting_print, "no trap %d", 1);
    test_assert(jumped == 0);
    return;
    TRAP_LABEL(jump2):
    test_assert(0 && "should not jump on false condition");
}

static void test_trap_silent(void)
{
    trace_count = 0;
    int jumped = 0;
    TRAP_SILENT(1, silent1);
    test_assert(0 && "TRAP_SILENT should jump");
    TRAP_LABEL(silent1):
    jumped = 1;
    test_assert(jumped == 1);
    test_assert(trace_count == 0);
}

static void test_trapnull(void)
{
    int *p = NULL;
    int jumped = 0;
    TRAPNULL(p);
    test_assert(0 && "TRAPNULL should jump on NULL");
    TRAP_LABEL(p_null):
    jumped = 1;
    test_assert(jumped == 1);
}

static void test_trapnull_nonnull(void)
{
    int val = 42;
    int *q = &val;
    TRAPNULL(q);
    test_assert(*q == 42);
    return;
    TRAP_LABEL(q_null):
    test_assert(0 && "TRAPNULL should not jump on non-NULL");
}

static void test_strip_path(void)
{
    test_assert(strcmp(lgk_tnt_strip_path("/foo/bar.c"), "bar.c") == 0);
    test_assert(strcmp(lgk_tnt_strip_path("/a/b/c/d.h"), "d.h") == 0);

    const char *nopath = "bar.c";
    test_assert(lgk_tnt_strip_path(nopath) == nopath);

    const char *empty = "";
    test_assert(lgk_tnt_strip_path(empty) == empty);

    test_assert(lgk_tnt_strip_path("/foo/")[0] == '\0');
}

#ifndef LGK_TNT_NO_TRACE_SHORTS
static void test_trace_shorts(void)
{
    EMERG("emergency %d", 1);
    ALERT("alert %d", 1);
    CRIT("critical %d", 1);
    ERR("error %d", 1);
    WARN("warning %d", 1);
    NOTICE("notice %d", 1);
    INFO("info %d", 1);
    DEBUG("debug %d", 1);
    CRITF(testfunc, "crit %d", 1);
    ERRF(testfunc, "err %d", 1);
    CRITFE(testfunc);
    ERRFE(testfunc);
    DEBUGV(42, "%d");
}
#endif

#ifndef LGK_TNT_NO_ERRNO
static void test_errno_macros(void)
{
    errno = EINVAL;
    TRACEFE(TRACE_LEVEL_CRITICAL, testfunc);

    int trapped = 0;
    errno = EINVAL;
    TRAPFE(1, efunc);
    test_assert(0 && "TRAPFE should jump");
    TRAP_LABEL(efunc):
    trapped = 1;
    test_assert(trapped == 1);

    int trapped2 = 0;
    errno = EINVAL;
    TRAPFES(1, sfunc, err);
    test_assert(0 && "TRAPFES should jump");
    TRAP_LABEL(sfunc_err):
    trapped2 = 1;
    test_assert(trapped2 == 1);
}
#endif

#ifdef LGK_TNT_OUTPUT_LEVEL_DYNAMIC
static void test_dynamic_output_level(void)
{
    enum trace_level saved = lgk_tnt_output_level_dynamic;

    lgk_tnt_output_level_dynamic = TRACE_LEVEL_DEBUG;
    trace_count = 0;
    TRACEP(TRACE_LEVEL_DEBUG, counting_print, "debug %d", 1);
    test_assert(trace_count == 1);

    lgk_tnt_output_level_dynamic = TRACE_LEVEL_WARNING;
    trace_count = 0;
    TRACEP(TRACE_LEVEL_DEBUG, counting_print, "debug %d", 1);
    test_assert(trace_count == 0);

    trace_count = 0;
    TRACEP(TRACE_LEVEL_WARNING, counting_print, "warn %d", 1);
    test_assert(trace_count == 1);

    lgk_tnt_output_level_dynamic = saved;
}
#endif

int main(void)
{
    test_trace_calls_print();
    test_trace_level_filtering();
    test_trap_jumps_on_true();
    test_trap_nojump_on_false();
    test_trap_silent();
    test_trapnull();
    test_trapnull_nonnull();
    test_strip_path();
#ifndef LGK_TNT_NO_TRACE_SHORTS
    test_trace_shorts();
#endif
#ifndef LGK_TNT_NO_ERRNO
    test_errno_macros();
#endif
#ifdef LGK_TNT_OUTPUT_LEVEL_DYNAMIC
    test_dynamic_output_level();
#endif
    return 0;
}
