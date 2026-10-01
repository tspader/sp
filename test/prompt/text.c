#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "ctrl_c_cancels_and_keeps_input",
    .widget = {
      .kind = PROMPT_WIDGET_TEXT,
      .text = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'A' } },
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  A",
      },
    },
  },
  {
    .name = "enter_submits_prefill_when_empty",
    .widget = {
      .kind = PROMPT_WIDGET_TEXT,
      .text = { .prompt = "P", .prefill = "A" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "A",
      .lines = {
        "◇  P",
        "│  A",
      },
    },
  },
};

sp_test_each_fn(prompt, text, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
