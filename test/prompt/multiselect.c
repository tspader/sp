#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "space_toggles_and_enter_submits",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" }, { .label = "C" } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  P",
        "│  A, B",
      },
    },
  },
  {
    .name = "escape_cancels_and_keeps_options",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = { { .label = "A", .selected = true }, { .label = "B" } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ● A",
        "│  ○ B",
        "└",
      },
    },
  },
  {
    .name = "overflow_below_shows_ellipsis",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = {
          { .label = "A" },
          { .label = "B" },
          { .label = "C" },
          { .label = "D" },
          { .label = "E" },
          { .label = "F" },
        },
        .max_visible = 3,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ○ A",
        "│  ○ B",
        "│  ○ C",
        "│  ...",
        "└",
      },
    },
  },
  {
    .name = "overflow_above_shows_ellipsis",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = {
          { .label = "A" },
          { .label = "B" },
          { .label = "C" },
          { .label = "D" },
          { .label = "E" },
          { .label = "F" },
        },
        .max_visible = 3,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ...",
        "│  ○ D",
        "│  ○ E",
        "│  ○ F",
        "└",
      },
    },
  },
  {
    .name = "overflow_both_ways_keeps_height",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = {
          { .label = "A" },
          { .label = "B" },
          { .label = "C" },
          { .label = "D" },
          { .label = "E", .selected = true },
          { .label = "F" },
          { .label = "G" },
        },
        .max_visible = 3,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ...",
        "│  ○ D",
        "│  ● E",
        "│  ...",
        "└",
      },
    },
  },
  {
    .name = "unhovered_options_are_dimmed",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = {
          { .label = "A", .selected = true },
          { .label = "B", .selected = true },
          { .label = "C" },
        },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ● A",
        "│  ● B",
        "│  ○ C",
        "└",
      },
      .cells = {
        { .row = 1, .col = 3, .codepoint = 0x25cf, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 1, .col = 5, .codepoint = 'A', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 2, .col = 3, .codepoint = 0x25cf, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 2, .col = 5, .codepoint = 'B' },
        { .row = 3, .col = 3, .codepoint = 0x25cb, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 3, .col = 5, .codepoint = 'C', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
  {
    .name = "filter_matches_substring",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = { { .label = "AB" }, { .label = "CD" }, { .label = "EF" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'D' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  P D",
        "│  CD",
      },
    },
  },
  {
    .name = "filter_takes_vim_keys_as_text",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = { { .label = "JA" }, { .label = "JB" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  P j",
        "│  JA",
      },
    },
  },
  {
    .name = "filter_shows_single_hidden_tail_option",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = {
          { .label = "A" },
          { .label = "B" },
          { .label = "C" },
          { .label = "D" },
          { .label = "E" },
          { .label = "F" },
        },
        .max_visible = 4,
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P Type to filter...",
        "│  ...",
        "│  ○ C",
        "│  ○ D",
        "│  ○ E",
        "│  ○ F",
        "└",
      },
    },
  },
  {
    .name = "filter_placeholder_is_dimmed",
    .widget = {
      .kind = PROMPT_WIDGET_MULTISELECT,
      .multiselect = {
        .prompt = "P",
        .options = { { .label = "A" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P Type to filter...",
        "│  ○ A",
        "└",
      },
      .cells = {
        { .row = 0, .col = 5, .codepoint = 'T', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
};

sp_test_each_fn(prompt, multiselect, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
