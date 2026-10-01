#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "enter_submits_masked_prefill_when_empty",
    .widget = {
      .kind = PROMPT_WIDGET_PASSWORD,
      .password = { .prompt = "P", .prefill = "AB" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "AB",
      .lines = {
        "◇  P",
        "│  **",
      },
    },
  },
  {
    .name = "tab_reveals_value",
    .widget = {
      .kind = PROMPT_WIDGET_PASSWORD,
      .password = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'A' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'B' } },
      { .kind = SP_PROMPT_EVENT_TAB },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "AB",
      .lines = {
        "◇  P",
        "│  AB",
      },
    },
  },
};

sp_test_each_fn(prompt, password, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
