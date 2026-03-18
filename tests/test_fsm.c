/*
 * Tests for lgk/fsm.h macro-generated FSM.
 * Instantiates an FSM with mixed type kinds:
 *   context  = struct test_context *  (pointer to struct)
 *   state    = int_fast8_t            (simple signed type)
 *   event    = int_fast8_t            (simple signed type)
 *   event_data = struct test_event_data *  (pointer to struct)
 * Run: ctest (from build dir)
 */

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include <lgk/fsm.h>

/* ── types used by the FSM ──────────────────────────────────────────── */

enum tfsm_state_t : int_fast8_t { STATE_IDLE = 0, STATE_RUNNING, STATE_DONE, N_STATES };
enum tfsm_event_t : uint_fast8_t { EVENT_START = 0, EVENT_FINISH };

struct test_context
{
    int enter_count;
    int event_count;
    enum tfsm_state_t last_enter_state;
    enum tfsm_event_t last_event;
};

struct test_event_data
{
    int value;
};

/* ── instantiate the FSM ────────────────────────────────────────────── */

FSM_STATIC(tfsm, struct test_context *, enum tfsm_state_t, enum tfsm_event_t, struct test_event_data *)

/* ── handlers ───────────────────────────────────────────────────────── */

static enum tfsm_state_t enter_idle(struct test_context *ctx, enum tfsm_state_t state)
{
    ctx->enter_count++;
    ctx->last_enter_state = state;
    return 0;
}

static enum tfsm_state_t enter_running(struct test_context *ctx, enum tfsm_state_t state)
{
    ctx->enter_count++;
    ctx->last_enter_state = state;
    return 0;
}

static enum tfsm_state_t enter_done(struct test_context *ctx, enum tfsm_state_t state)
{
    ctx->enter_count++;
    ctx->last_enter_state = state;
    return 0;
}

static enum tfsm_state_t event_idle(struct test_context *ctx, enum tfsm_state_t state, enum tfsm_event_t event, struct test_event_data *data)
{
    (void)state;
    (void)data;
    ctx->event_count++;
    ctx->last_event = event;
    if (event == EVENT_START) return STATE_RUNNING;
    return 0;
}

static enum tfsm_state_t event_running(struct test_context *ctx, enum tfsm_state_t state, enum tfsm_event_t event, struct test_event_data *data)
{
    (void)state;
    (void)data;
    ctx->event_count++;
    ctx->last_event = event;
    if (event == EVENT_FINISH) return STATE_DONE;
    return 0;
}

static enum tfsm_state_t event_done(struct test_context *ctx, enum tfsm_state_t state, enum tfsm_event_t event, struct test_event_data *data)
{
    (void)state;
    (void)event;
    (void)data;
    ctx->event_count++;
    return 0;
}

/* enter handler that triggers a chained transition */
static enum tfsm_state_t enter_auto_chain(struct test_context *ctx, enum tfsm_state_t state)
{
    (void)state;
    ctx->enter_count++;
    return STATE_DONE;
}

/* enter handler that returns an out-of-range state */
static enum tfsm_state_t enter_out_of_range(struct test_context *ctx, enum tfsm_state_t state)
{
    (void)ctx;
    (void)state;
    return N_STATES;
}

/* handler tables */
static tfsm_enter_handler *g_enter[N_STATES] = {
    enter_idle, enter_running, enter_done
};
static tfsm_event_handler *g_event[N_STATES] = {
    event_idle, event_running, event_done
};

/* ── helpers ────────────────────────────────────────────────────────── */

static struct tfsm make_fsm(struct test_context *ctx,
                            tfsm_enter_handler **eh,
                            tfsm_event_handler **evh)
{
    struct tfsm fsm;
    tfsm_init(&fsm, ctx, eh, evh, N_STATES);
    return fsm;
}

/* ── init tests ─────────────────────────────────────────────────────── */

static void test_init(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm;
    int_fast8_t ret = tfsm_init(&fsm, &ctx, g_enter, g_event, N_STATES);
    assert(ret == 0);
    assert(fsm.current == 0);
    assert(fsm.next == 0);
    assert(fsm.context == &ctx);
    assert(fsm.n_states == N_STATES);
    assert(fsm.enter_handlers == g_enter);
    assert(fsm.event_handlers == g_event);
}

static void test_init_null_enter_handlers(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm;
    int_fast8_t ret = tfsm_init(&fsm, &ctx, NULL, g_event, N_STATES);
    assert(ret < 0);
}

static void test_init_null_event_handlers(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm;
    int_fast8_t ret = tfsm_init(&fsm, &ctx, g_enter, NULL, N_STATES);
    assert(ret < 0);
}

static void test_init_null_context(void)
{
    struct tfsm fsm;
    int_fast8_t ret = tfsm_init(&fsm, NULL, g_enter, g_event, N_STATES);
    assert(ret == 0);
    assert(fsm.context == NULL);
}

/* ── reset tests ────────────────────────────────────────────────────── */

static void test_reset(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    fsm.current = STATE_DONE;
    fsm.next = STATE_RUNNING;

    int_fast8_t ret = tfsm_reset(&fsm);
    assert(ret == 0);
    assert(fsm.current == 0);
    assert(fsm.next == 0);
}

static void test_reset_null(void)
{
    int_fast8_t ret = tfsm_reset(NULL);
    assert(ret < 0);
}

/* ── event tests ────────────────────────────────────────────────────── */

static void test_event_triggers_transition(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    struct test_event_data data = { .value = 42 };
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret == 1);
    assert(fsm.next == STATE_RUNNING);
    assert(ctx.event_count == 1);
    assert(ctx.last_event == EVENT_START);
}

static void test_event_no_transition(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_FINISH, &data);
    assert(ret == 0);
    assert(fsm.next == 0);
}

static void test_event_with_null_data(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, NULL);
    assert(ret == 1);
    assert(fsm.next == STATE_RUNNING);
}

static void test_event_null_fsm(void)
{
    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(NULL, EVENT_START, &data);
    assert(ret < 0);
}

static void test_event_current_out_of_range(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    fsm.current = N_STATES;

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret < 0);
}

static void test_event_current_negative(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    fsm.current = -1;

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret < 0);
}

static void test_event_during_pending_transition(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    fsm.next = STATE_RUNNING;

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret < 0);
}

static void test_event_null_handler_entry(void)
{
    struct test_context ctx = {0};
    tfsm_event_handler *evh[N_STATES] = { NULL, event_running, event_done };
    struct tfsm fsm = make_fsm(&ctx, g_enter, evh);

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret < 0);
}

static void test_event_null_handlers_table(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    fsm.event_handlers = NULL;

    struct test_event_data data = {0};
    int_fast8_t ret = tfsm_event(&fsm, EVENT_START, &data);
    assert(ret < 0);
}

/* ── step tests ─────────────────────────────────────────────────────── */

static void test_step(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    struct test_event_data data = {0};
    tfsm_event(&fsm, EVENT_START, &data);
    assert(fsm.next == STATE_RUNNING);

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret == 0);
    assert(fsm.current == STATE_RUNNING);
    assert(fsm.next == 0);
    assert(ctx.enter_count == 1);
    assert(ctx.last_enter_state == STATE_RUNNING);
}

static void test_step_null_fsm(void)
{
    int_fast8_t ret = tfsm_step(NULL);
    assert(ret < 0);
}

static void test_step_no_pending_transition(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret < 0);
}

static void test_step_null_enter_handlers_table(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);

    struct test_event_data data = {0};
    tfsm_event(&fsm, EVENT_START, &data);
    fsm.enter_handlers = NULL;

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret == 0);
    assert(fsm.current == STATE_RUNNING);
    assert(fsm.next == 0);
}

static void test_step_null_enter_handler_entry(void)
{
    struct test_context ctx = {0};
    tfsm_enter_handler *eh[N_STATES] = { NULL, NULL, NULL };
    struct tfsm fsm = make_fsm(&ctx, eh, g_event);

    struct test_event_data data = {0};
    tfsm_event(&fsm, EVENT_START, &data);

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret == 0);
    assert(fsm.current == STATE_RUNNING);
    assert(ctx.enter_count == 0);
}

static void test_step_chained_transition(void)
{
    struct test_context ctx = {0};
    tfsm_enter_handler *eh[N_STATES] = { NULL, enter_auto_chain, enter_done };
    struct tfsm fsm = make_fsm(&ctx, eh, g_event);

    struct test_event_data data = {0};
    tfsm_event(&fsm, EVENT_START, &data);

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret == STATE_DONE);
    assert(fsm.current == STATE_RUNNING);
    assert(fsm.next == STATE_DONE);

    ret = tfsm_step(&fsm);
    assert(ret == 0);
    assert(fsm.current == STATE_DONE);
    assert(fsm.next == 0);
    assert(ctx.enter_count == 2);
}

static void test_step_enter_handler_out_of_range(void)
{
    struct test_context ctx = {0};
    tfsm_enter_handler *eh[N_STATES] = { NULL, enter_out_of_range, NULL };
    struct tfsm fsm = make_fsm(&ctx, eh, g_event);

    struct test_event_data data = {0};
    tfsm_event(&fsm, EVENT_START, &data);

    int_fast8_t ret = tfsm_step(&fsm);
    assert(ret < 0);
}

/* ── full cycle ─────────────────────────────────────────────────────── */

static void test_full_cycle(void)
{
    struct test_context ctx = {0};
    struct tfsm fsm = make_fsm(&ctx, g_enter, g_event);
    struct test_event_data data = {0};

    assert(fsm.current == STATE_IDLE);

    assert(tfsm_event(&fsm, EVENT_START, &data) == 1);
    assert(tfsm_step(&fsm) == 0);
    assert(fsm.current == STATE_RUNNING);

    assert(tfsm_event(&fsm, EVENT_FINISH, &data) == STATE_DONE);
    assert(tfsm_step(&fsm) == 0);
    assert(fsm.current == STATE_DONE);

    assert(ctx.enter_count == 2);
    assert(ctx.event_count == 2);
}

/* ── main ───────────────────────────────────────────────────────────── */

int main(void)
{
    test_init();
    test_init_null_enter_handlers();
    test_init_null_event_handlers();
    test_init_null_context();

    test_reset();
    test_reset_null();

    test_event_triggers_transition();
    test_event_no_transition();
    test_event_with_null_data();
    test_event_null_fsm();
    test_event_current_out_of_range();
    test_event_current_negative();
    test_event_during_pending_transition();
    test_event_null_handler_entry();
    test_event_null_handlers_table();

    test_step();
    test_step_null_fsm();
    test_step_no_pending_transition();
    test_step_null_enter_handlers_table();
    test_step_null_enter_handler_entry();
    test_step_chained_transition();
    test_step_enter_handler_out_of_range();

    test_full_cycle();

    return 0;
}
