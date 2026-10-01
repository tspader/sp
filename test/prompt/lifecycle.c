#include "prompt.h"

typedef struct {
  sp_prompt_state_t state;
} expect_t;

typedef struct {
  const c8* name;
  sp_prompt_state_t fire;
  prompt_act_t acts [PROMPT_MAX_ACTS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "starts_active",
  },
  {
    .name = "complete_submits",
    .acts = {
      { .kind = PROMPT_ACT_COMPLETE },
    },
    .expect = { .state = SP_PROMPT_STATE_SUBMIT },
  },
  {
    .name = "abort_cancels",
    .acts = {
      { .kind = PROMPT_ACT_ABORT },
    },
    .expect = { .state = SP_PROMPT_STATE_CANCEL },
  },
  {
    .name = "first_transition_wins",
    .acts = {
      { .kind = PROMPT_ACT_COMPLETE },
      { .kind = PROMPT_ACT_ABORT },
      { .kind = PROMPT_ACT_COMPLETE },
    },
    .expect = { .state = SP_PROMPT_STATE_SUBMIT },
  },
  {
    .name = "complete_on_init_ends_run_submitted",
    .fire = SP_PROMPT_STATE_SUBMIT,
    .acts = {
      { .kind = PROMPT_ACT_RUN },
    },
    .expect = { .state = SP_PROMPT_STATE_SUBMIT },
  },
  {
    .name = "abort_on_init_ends_run_cancelled",
    .fire = SP_PROMPT_STATE_CANCEL,
    .acts = {
      { .kind = PROMPT_ACT_RUN },
    },
    .expect = { .state = SP_PROMPT_STATE_CANCEL },
  },
  {
    .name = "complete_from_thread_ends_run_submitted",
    .acts = {
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
      { .kind = PROMPT_ACT_COMPLETE, .threaded = true },
      { .kind = PROMPT_ACT_RUN },
    },
    .expect = { .state = SP_PROMPT_STATE_SUBMIT },
  },
};

sp_test_each(prompt, lifecycle, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  prompt_fixture_t* f = prompt_fixture(t);
  f->probe.fire = it->fire;
  sp_try(prompt_act(t, it->acts));

  sp_expect_eq(t, sp_atomic_s32_load(&f->ctx.state, SP_ATOMIC_SEQ_CST), (s32)it->expect.state);
  sp_expect_eq(t, sp_prompt_is_aborted(&f->ctx), it->expect.state != SP_PROMPT_STATE_ACTIVE);
  sp_expect_eq(t, sp_prompt_submitted(&f->ctx), it->expect.state == SP_PROMPT_STATE_SUBMIT);
  sp_expect_eq(t, sp_prompt_cancelled(&f->ctx), it->expect.state == SP_PROMPT_STATE_CANCEL);
  return SP_OK;
}
