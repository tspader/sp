#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "enter_submits_initial_yes",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P", .initial = true },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_TRUE,
      .lines = {
        "◇  P",
        "│  ● Yes / ○ No",
      },
      .cells = {
        { .row = 1, .col = 3, .codepoint = 0x25cf, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 1, .col = 5, .codepoint = 'Y' },
        { .row = 1, .col = 11, .codepoint = 0x25cb, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 1, .col = 13, .codepoint = 'N', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
  {
    .name = "enter_submits_initial_no",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_FALSE,
      .lines = {
        "◇  P",
        "│  ○ Yes / ● No",
      },
      .cells = {
        { .row = 1, .col = 3, .codepoint = 0x25cb, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 1, .col = 5, .codepoint = 'Y', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 1, .col = 11, .codepoint = 0x25cf, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 1, .col = 13, .codepoint = 'N' },
      },
    },
  },
  {
    .name = "left_selects_no",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P", .initial = true },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_LEFT },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_FALSE,
      .lines = {
        "◇  P",
        "│  ○ Yes / ● No",
      },
    },
  },
  {
    .name = "right_selects_yes",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_RIGHT },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_TRUE,
      .lines = {
        "◇  P",
        "│  ● Yes / ○ No",
      },
    },
  },
  {
    .name = "vim_h_selects_no",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P", .initial = true },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'h' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_FALSE,
      .lines = {
        "◇  P",
        "│  ○ Yes / ● No",
      },
    },
  },
  {
    .name = "vim_l_selects_yes",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'l' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = PROMPT_BOOL_TRUE,
      .lines = {
        "◇  P",
        "│  ● Yes / ○ No",
      },
    },
  },
  {
    .name = "escape_cancels_and_keeps_options",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P" },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ○ Yes / ● No",
        "└",
      },
    },
  },
};

sp_test_each_fn(prompt, confirm, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
