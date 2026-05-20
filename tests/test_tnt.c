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
    TRAPVNULL(p);
    test_assert(0 && "TRAPNULL should jump on NULL");
    TRAP_LABEL(p_null):
    jumped = 1;
    test_assert(jumped == 1);
}

static void test_trapnull_nonnull(void)
{
    int val = 42;
    int *q = &val;
    TRAPVNULL(q);
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

static void test_trap_xnull_compound(void)
{
    int *p = NULL;
    int jumped = 0;
    TRAPXNULL(p, simple);
    test_assert(0 && "TRAPXNULL(NULL) should jump");
    TRAP_LABEL(simple_null):
    jumped = 1;
    test_assert(jumped == 1);

    /* Compound expression: with the parenthesized macro, !(a==b) traps when a != b.
       Without the parens this expands to (!a)==b, which would not trap for a=5,b=2. */
    int a = 5, b = 2;
    jumped = 0;
    TRAPXNULL(a == b, cmp);
    test_assert(0 && "TRAPXNULL(a==b) with a!=b should jump after parens fix");
    TRAP_LABEL(cmp_null):
    jumped = 1;
    test_assert(jumped == 1);

    /* TRAPXNULL on a non-NULL pointer must not jump. */
    int val = 42;
    int *q = &val;
    TRAPXNULL(q, nonnull);
    test_assert(*q == 42);
    return;
    TRAP_LABEL(nonnull_null):
    test_assert(0 && "TRAPXNULL on non-NULL must not jump");
}

static void test_trap_compare_xx(void)
{
    int a = 1;
    int arr[] = {1, 2};
    int val = 5;
    int *pa = &val;
    int jumped;

    /* arithmetic */
    jumped = 0;
    TRAPXXEQ(1 + 1, 2, aa, bb, "%d");
    test_assert(0 && "TRAPXXEQ(1+1,2) should jump");
    TRAP_LABEL(aa_eq_bb):
    jumped = 1;
    test_assert(jumped == 1);

    /* bitwise: (a&1)==1 must be parenthesized */
    jumped = 0;
    TRAPXXEQ(a & 1, 1, mx, my, "%d");
    test_assert(0 && "TRAPXXEQ(a&1,1) with a=1 should jump");
    TRAP_LABEL(mx_eq_my):
    jumped = 1;
    test_assert(jumped == 1);

    /* shift: (a<<1)!=3 -> 2!=3 true */
    jumped = 0;
    TRAPXXNEQ(a << 1, 3, sa, sb, "%d");
    test_assert(0 && "TRAPXXNEQ(a<<1,3) should jump");
    TRAP_LABEL(sa_neq_sb):
    jumped = 1;
    test_assert(jumped == 1);

    /* ternary: (a?5:0)>1 -> 5>1 true */
    jumped = 0;
    TRAPXXGT(a ? 5 : 0, 1, ta, tb, "%d");
    test_assert(0 && "TRAPXXGT(ternary,1) should jump");
    TRAP_LABEL(ta_gt_tb):
    jumped = 1;
    test_assert(jumped == 1);

    /* deref: *pa<10 -> 5<10 true */
    jumped = 0;
    TRAPXXLT(*pa, 10, da, db, "%d");
    test_assert(0 && "TRAPXXLT(*pa,10) should jump");
    TRAP_LABEL(da_lt_db):
    jumped = 1;
    test_assert(jumped == 1);

    /* array+arith: (arr[0]+arr[1])>=3 -> 3>=3 true */
    jumped = 0;
    TRAPXXGTE(arr[0] + arr[1], 3, aa2, bb2, "%d");
    test_assert(0 && "TRAPXXGTE(arr[0]+arr[1],3) should jump");
    TRAP_LABEL(aa2_gte_bb2):
    jumped = 1;
    test_assert(jumped == 1);

    /* unary minus: -a<=0 -> -1<=0 true */
    jumped = 0;
    TRAPXXLTE(-a, 0, na, nb, "%d");
    test_assert(0 && "TRAPXXLTE(-a,0) should jump");
    TRAP_LABEL(na_lte_nb):
    jumped = 1;
    test_assert(jumped == 1);
}

static void test_trap_compare_xx_no_jump(void)
{
    int a = 1;
    /* (a&2)==1 with a=1 -> 0==1 false; without parens 1&(2==1)=1&0=0 also falsy.
       The point is just to confirm the no-jump path works with a compound expr. */
    TRAPXXEQ(a & 2, 1, nj, nj2, "%d");
    /* Confirm we fell through. Also exercise a different operator: */
    TRAPXXGT(a << 1, 100, nj3, nj4, "%d");
    TRAPXXLT(a ? 5 : 0, 1, nj5, nj6, "%d");
    return;
    TRAP_LABEL(nj_eq_nj2):
    test_assert(0 && "TRAPXXEQ no-jump path failed");
    TRAP_LABEL(nj3_gt_nj4):
    test_assert(0 && "TRAPXXGT no-jump path failed");
    TRAP_LABEL(nj5_lt_nj6):
    test_assert(0 && "TRAPXXLT no-jump path failed");
}

static void test_trap_compare_vx_vv(void)
{
    int arr[] = {1, 2};
    int myvar = 3;
    int big = 10, small = 1;
    int eq1 = 5, eq2 = 5;
    int jumped;

    /* TRAPVXEQ: myvar == (arr[0]+arr[1]) -> 3==3 true */
    jumped = 0;
    TRAPVXEQ(myvar, arr[0] + arr[1], lx, "%d");
    test_assert(0 && "TRAPVXEQ should jump");
    TRAP_LABEL(myvar_eq_lx):
    jumped = 1;
    test_assert(jumped == 1);

    /* TRAPVVGT: big > small -> 10>1 true */
    jumped = 0;
    TRAPVVGT(big, small, "%d");
    test_assert(0 && "TRAPVVGT should jump");
    TRAP_LABEL(big_gt_small):
    jumped = 1;
    test_assert(jumped == 1);

    /* TRAPVVLTE: eq1 <= eq2 -> 5<=5 true */
    jumped = 0;
    TRAPVVLTE(eq1, eq2, "%d");
    test_assert(0 && "TRAPVVLTE should jump");
    TRAP_LABEL(eq1_lte_eq2):
    jumped = 1;
    test_assert(jumped == 1);
}

static void test_trap_range_oor(void)
{
    int jumped;

    /* below range: 0 not in [10,20], complex bound expressions */
    jumped = 0;
    TRAPXXXOOR(2 * 0, 5 + 5, 4 * 5, xv1, lmin1, lmax1, "%d");
    test_assert(0 && "below-range OOR should jump");
    TRAP_LABEL(xv1_oor_lmin1_lmax1):
    jumped = 1;
    test_assert(jumped == 1);

    /* above range: 30 not in [10,20] */
    jumped = 0;
    TRAPXXXOOR(30, 10, 20, xv2, lmin2, lmax2, "%d");
    test_assert(0 && "above-range OOR should jump");
    TRAP_LABEL(xv2_oor_lmin2_lmax2):
    jumped = 1;
    test_assert(jumped == 1);

    /* in range: no trap */
    TRAPXXXOOR(15, 10, 20, xv3, lmin3, lmax3, "%d");
    return;
    TRAP_LABEL(xv3_oor_lmin3_lmax3):
    test_assert(0 && "in-range OOR must not jump");
}

static void test_trap_range_oor_variants(void)
{
    {
        int v = 5, xmin = 10, xmax = 20;
        int jumped = 0;
        TRAPVXXOOR(v, xmin, xmax, lm, lx, "%d");
        test_assert(0 && "TRAPVXXOOR should jump");
        TRAP_LABEL(v_oor_lm_lx):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 5, vmin = 10, xmax = 20;
        int jumped = 0;
        TRAPVVXOOR(v, vmin, xmax, lm2, "%d");
        test_assert(0 && "TRAPVVXOOR should jump");
        TRAP_LABEL(v_oor_vmin_lm2):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 25, xmin = 10, vmax = 20;
        int jumped = 0;
        TRAPVXVOOR(v, xmin, vmax, lm3, "%d");
        test_assert(0 && "TRAPVXVOOR should jump");
        TRAP_LABEL(v_oor_lm3_vmax):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 25, vmin = 10, vmax = 20;
        int jumped = 0;
        TRAPVVVOOR(v, vmin, vmax, "%d");
        test_assert(0 && "TRAPVVVOOR should jump");
        TRAP_LABEL(v_oor_vmin_vmax):
        jumped = 1;
        test_assert(jumped == 1);
    }
}

static void test_trap_range_inr(void)
{
    int jumped;

    /* in range: 2 in [1,10] */
    jumped = 0;
    TRAPXXXINR(1 + 1, 1, 10, xv1, lmin1, lmax1, "%d");
    test_assert(0 && "in-range INR should jump");
    TRAP_LABEL(xv1_inr_lmin1_lmax1):
    jumped = 1;
    test_assert(jumped == 1);

    /* below range: 0 not in [10,20] */
    TRAPXXXINR(0, 10, 20, xv2, lmin2, lmax2, "%d");
    /* above range: 30 not in [10,20] */
    TRAPXXXINR(30, 10, 20, xv3, lmin3, lmax3, "%d");
    return;
    TRAP_LABEL(xv2_inr_lmin2_lmax2):
    test_assert(0 && "below-range INR must not jump");
    TRAP_LABEL(xv3_inr_lmin3_lmax3):
    test_assert(0 && "above-range INR must not jump");
}

static void test_trap_range_inr_variants(void)
{
    {
        int v = 15, xmin = 10, xmax = 20;
        int jumped = 0;
        TRAPVXXINR(v, xmin, xmax, lm, lx, "%d");
        test_assert(0 && "TRAPVXXINR should jump");
        TRAP_LABEL(v_inr_lm_lx):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 15, vmin = 10, xmax = 20;
        int jumped = 0;
        TRAPVVXINR(v, vmin, xmax, lm2, "%d");
        test_assert(0 && "TRAPVVXINR should jump");
        TRAP_LABEL(v_inr_vmin_lm2):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 15, xmin = 10, vmax = 20;
        int jumped = 0;
        TRAPVXVINR(v, xmin, vmax, lm3, "%d");
        test_assert(0 && "TRAPVXVINR should jump");
        TRAP_LABEL(v_inr_lm3_vmax):
        jumped = 1;
        test_assert(jumped == 1);
    }
    {
        int v = 15, vmin = 10, vmax = 20;
        int jumped = 0;
        TRAPVVVINR(v, vmin, vmax, "%d");
        test_assert(0 && "TRAPVVVINR should jump");
        TRAP_LABEL(v_inr_vmin_vmax):
        jumped = 1;
        test_assert(jumped == 1);
    }
}

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
    test_trap_xnull_compound();
    test_trap_compare_xx();
    test_trap_compare_xx_no_jump();
    test_trap_compare_vx_vv();
    test_trap_range_oor();
    test_trap_range_oor_variants();
    test_trap_range_inr();
    test_trap_range_inr_variants();
    return 0;
}
