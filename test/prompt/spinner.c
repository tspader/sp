#include "prompt.h"

#define SPINNER_MAX_STEPS 2

typedef struct {
  u32 ticks;
  sp_prompt_event_t event;
} step_t;

typedef struct {
  const c8* name;
  sp_prompt_spinner_t spinner;
  step_t steps [SPINNER_MAX_STEPS];
  prompt_expect_t expect;
} test_t;

static void color_red(sp_prompt_ctx_t* ctx, u32 frame_index, sp_prompt_style_t* style) {
  *style = (sp_prompt_style_t) { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 31 };
}

static const test_t tests [] = {
  {
    .name = "starts_on_first_frame",
    .spinner = {
      .prompt = "P",
      .frames = { 'A', 'B', 'C' },
    },
    .expect = {
      .lines = { "A  P" },
    },
  },
  {
    .name = "tick_advances_one_frame",
    .spinner = {
      .prompt = "P",
      .frames = { 'A', 'B', 'C' },
    },
    .steps = {
      { .ticks = 1 },
    },
    .expect = {
      .lines = { "B  P" },
    },
  },
  {
    .name = "wraps_after_last_frame",
    .spinner = {
      .prompt = "P",
      .frames = { 'A', 'B' },
    },
    .steps = {
      { .ticks = 2 },
    },
    .expect = {
      .lines = { "A  P" },
    },
  },
  {
    .name = "idle_ticks_do_not_grow_arena",
    .spinner = {
      .prompt = "P",
      .frames = { 'A', 'B', 'C' },
    },
    .steps = {
      { .ticks = 1024 },
    },
    .expect = {
      .lines = { "B  P" },
    },
  },
  {
    .name = "color_fn_overrides_fixed_color",
    .spinner = {
      .prompt = "P",
      .frames = { 'A' },
      .color = { .ansi = 36, .fn = color_red },
    },
    .expect = {
      .cells = {
        { .row = 0, .col = 0, .codepoint = 'A', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 31 } },
      },
    },
  },
  {
    .name = "enter_submits",
    .spinner = {
      .prompt = "P",
      .frames = { 'A' },
    },
    .steps = {
      { .event = { .kind = SP_PROMPT_EVENT_ENTER } },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
    },
  },
  {
    .name = "ctrl_c_cancels",
    .spinner = {
      .prompt = "P",
      .frames = { 'A' },
    },
    .steps = {
      { .event = { .kind = SP_PROMPT_EVENT_CTRL_C } },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
    },
  },
  {
    .name = "tick_after_submit_does_not_advance",
    .spinner = {
      .prompt = "P",
      .frames = { 'A', 'B' },
    },
    .steps = {
      { .event = { .kind = SP_PROMPT_EVENT_ENTER } },
      { .ticks = 1 },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = { "A  P" },
    },
  },
};

sp_test_each(prompt, spinner, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;

  prompt_start(t, sp_prompt_spinner_widget(ctx, it->spinner));
  u64 used = sp_mem_arena_bytes_used(ctx->arena);

  sp_carr_for(it->steps, n) {
    step_t step = it->steps[n];
    sp_for(tick, step.ticks) {
      prompt_tick(t);
    }
    if (step.event.kind != SP_PROMPT_EVENT_NONE) {
      prompt_send(t, step.event);
    }
  }

  sp_expect_eq(t, sp_mem_arena_bytes_used(ctx->arena), used);
  return prompt_expect_live(t, &it->expect);
}
