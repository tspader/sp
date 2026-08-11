#include "sp/sp_test.h"
#include "wait.h"

static const test_t tests [] = {
  {
    .name = "signaled_until_cleared",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT } } },
    },
  },
  {
    .name = "double_signal_ok",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
  {
    .name = "double_clear_ok",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_CLEAR },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT } } },
    },
  },
  {
    .name = "thread_signal_wakes_wait",
    .events = 1,
    .steps = {
      { .kind = STEP_SPAWN },
      { .kind = STEP_WAIT },
    },
  },
};

sp_test_each_fn(sys, event, test_t, tests, run);
