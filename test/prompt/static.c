#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "intro_submits_without_events",
    .widget = {
      .kind = PROMPT_WIDGET_INTRO,
      .intro = { .text = "A" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = { "┌  A" },
    },
  },
  {
    .name = "outro_submits_without_events",
    .widget = {
      .kind = PROMPT_WIDGET_OUTRO,
      .outro = { .text = "A" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = { "└  A" },
    },
  },
  {
    .name = "message_renders_symbol_before_text",
    .widget = {
      .kind = PROMPT_WIDGET_MESSAGE,
      .message = { .text = "A", .symbol = 0x25cf, .ansi = 36 },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = { "●  A" },
    },
  },
  {
    .name = "primed_events_are_ignored",
    .widget = {
      .kind = PROMPT_WIDGET_INTRO,
      .intro = { .text = "A" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'B' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = { "┌  A" },
    },
  },
};

sp_test_each_fn(prompt, static_widget, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
