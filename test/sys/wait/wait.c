#include "sp/sp_test.h"
#include "wait.h"

static const test_t tests [] = {
  {
    .name = "timeout_expires_when_never_signaled",
    .events = 1,
    .steps = {
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT, .min_elapsed_ms = 30 } } },
    },
  },
  {
    .name = "reports_lowest_signaled_index",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
  {
    .name = "reports_only_signaled_index",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000, .expect = { .index = 1 } } },
    },
  },
  {
    .name = "clear_moves_wait_to_next_signaled",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000, .expect = { .index = 1 } } },
    },
  },
};

sp_test_each_fn(sys, wait, test_t, tests, run);
