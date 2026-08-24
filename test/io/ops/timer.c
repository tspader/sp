#include "harness.h"

#define TIMER_MAX_STEPS 4

typedef enum {
  STEP_NONE,
  STEP_TIMER,
  STEP_WAIT,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { u32 slot; sp_io_timeout_kind_t timeout; s64 ms; sp_io_clock_t clock; } timer;
    ops_wait_t wait;
  };
} step_t;

typedef struct {
  const c8* name;
  step_t steps [TIMER_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "duration_completes_after_at_least_duration",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DURATION, .ms = 50 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .min_elapsed_ms = 50 } } },
    },
  },
  {
    .name = "deadline_completes_after_deadline",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DEADLINE, .ms = 50 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .after_deadline = true } } },
    },
  },
  {
    .name = "deadline_in_past_completes",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DEADLINE, .ms = -50 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1 } } },
    },
  },
  {
    .name = "boot_clock_duration_completes",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DURATION, .ms = 10, .clock = SP_IO_CLOCK_BOOT } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .min_elapsed_ms = 10 } } },
    },
  },
  {
    .name = "real_clock_deadline_completes",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DEADLINE, .ms = 10, .clock = SP_IO_CLOCK_REAL } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .after_deadline = true } } },
    },
  },
  {
    .name = "resubmitted_op_completes_again",
    .steps = {
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DURATION, .ms = 10 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .min_elapsed_ms = 10 } } },
      { .kind = STEP_TIMER, .timer = { .timeout = SP_IO_TIMEOUT_DURATION, .ms = 10 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .min_elapsed_ms = 10 } } },
    },
  },
  {
    .name = "wait_zero_duration_returns_at_once",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AFTER } },
    },
  },
  {
    .name = "wait_duration_returns_after_at_least_duration",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AFTER, .ms = 50, .expect = { .min_elapsed_ms = 50 } } },
    },
  },
  {
    .name = "wait_deadline_returns_after_deadline",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AT, .ms = 50, .expect = { .after_deadline = true } } },
    },
  },
  {
    .name = "wait_real_clock_deadline_returns_after_deadline",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AT, .ms = 50, .clock = SP_IO_CLOCK_REAL, .expect = { .after_deadline = true } } },
    },
  },
  {
    .name = "wait_past_deadline_returns_at_once",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AT, .ms = -50 } },
    },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, FIXTURE_NONE));

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_TIMER: {
        sp_io_op_t* op = &h.ops[step->timer.slot];
        op->kind = SP_IO_OP_TIMEOUT;
        op->timeout.timeout = harness_make_timeout(&h, step->timer.timeout, step->timer.ms, step->timer.clock);
        harness_arm(&h, op);
        sp_expect_ok(t, sp_io_submit(h.io, op));
        break;
      }
      case STEP_WAIT: {
        harness_wait(t, &h, &step->wait);
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, timer, test_t, tests, run);
