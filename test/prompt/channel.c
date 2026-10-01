#include "prompt.h"

typedef struct {
  bool dirty;
  f32 value;
  u32 count;
  f32 delivered;
} progress_t;

typedef struct {
  bool dirty;
  const c8* value;
  u32 count;
  const c8* delivered;
} status_t;

typedef struct {
  sp_prompt_state_t state;
  progress_t progress;
  status_t status;
} expect_t;

typedef struct {
  const c8* name;
  sp_prompt_state_t fire;
  prompt_act_t acts [PROMPT_MAX_ACTS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "starts_clean",
  },
  {
    .name = "send_progress_marks_dirty",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.5f },
    },
    .expect = {
      .progress = { .dirty = true, .value = 0.5f },
    },
  },
  {
    .name = "latest_progress_wins",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.25f },
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.75f },
    },
    .expect = {
      .progress = { .dirty = true, .value = 0.75f },
    },
  },
  {
    .name = "tick_delivers_progress_once",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.5f },
      { .kind = PROMPT_ACT_TICK, .ticks = 3 },
    },
    .expect = {
      .progress = { .value = 0.5f, .count = 1, .delivered = 0.5f },
    },
  },
  {
    .name = "progress_coalesces_between_ticks",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.25f },
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.75f },
      { .kind = PROMPT_ACT_TICK, .ticks = 2 },
    },
    .expect = {
      .progress = { .value = 0.75f, .count = 1, .delivered = 0.75f },
    },
  },
  {
    .name = "new_progress_is_delivered_again",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.25f },
      { .kind = PROMPT_ACT_TICK, .ticks = 2 },
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.75f },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .progress = { .value = 0.75f, .count = 2, .delivered = 0.75f },
    },
  },
  {
    .name = "pending_progress_does_not_block_submit",
    .fire = SP_PROMPT_STATE_SUBMIT,
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.5f },
      { .kind = PROMPT_ACT_RUN },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .progress = { .dirty = true, .value = 0.5f },
    },
  },
  {
    .name = "send_status_marks_dirty_and_copies",
    .acts = {
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
    },
    .expect = {
      .status = { .dirty = true, .value = "A" },
    },
  },
  {
    .name = "latest_status_wins",
    .acts = {
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
      { .kind = PROMPT_ACT_STATUS, .text = "B" },
    },
    .expect = {
      .status = { .dirty = true, .value = "B" },
    },
  },
  {
    .name = "tick_delivers_status_once",
    .acts = {
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 3 },
    },
    .expect = {
      .status = { .value = "A", .count = 1, .delivered = "A" },
    },
  },
  {
    .name = "progress_and_status_arrive_in_one_tick",
    .acts = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.5f },
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .progress = { .value = 0.5f, .count = 1, .delivered = 0.5f },
      .status = { .value = "A", .count = 1, .delivered = "A" },
    },
  },
  {
    .name = "pending_status_does_not_block_submit",
    .fire = SP_PROMPT_STATE_SUBMIT,
    .acts = {
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
      { .kind = PROMPT_ACT_RUN },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .status = { .dirty = true, .value = "A" },
    },
  },
  {
    .name = "status_from_thread_is_delivered",
    .acts = {
      { .kind = PROMPT_ACT_STATUS, .threaded = true, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 2 },
    },
    .expect = {
      .status = { .value = "A", .count = 1, .delivered = "A" },
    },
  },
};

sp_test_each(prompt, channel, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  prompt_fixture_t* f = prompt_fixture(t);
  f->probe.fire = it->fire;
  sp_try(prompt_act(t, it->acts));

  sp_expect_eq(t, sp_atomic_s32_load(&f->ctx.state, SP_ATOMIC_SEQ_CST), (s32)it->expect.state);

  sp_expect_eq(t, f->ctx.progress.dirty, it->expect.progress.dirty);
  sp_expect_eq(t, f->ctx.progress.value.f, (f64)it->expect.progress.value);
  sp_expect_eq(t, f->probe.progress, it->expect.progress.count);
  sp_expect_eq(t, f->probe.last_progress.f, (f64)it->expect.progress.delivered);

  sp_expect_eq(t, f->ctx.status.dirty, it->expect.status.dirty);
  sp_expect_str_eq_c(t, f->ctx.status.value, it->expect.status.value);
  sp_expect_eq(t, f->probe.status, it->expect.status.count);
  sp_expect_str_eq_c(t, f->probe.last_status, it->expect.status.delivered);
  return SP_OK;
}
