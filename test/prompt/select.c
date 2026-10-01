#include "prompt.h"

static const prompt_case_t tests [] = {
  {
    .name = "enter_submits_initial_selection",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" }, { .label = "C", .selected = true } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "C",
      .lines = {
        "◇  P",
        "│  C",
      },
      .cells = {
        { .row = 1, .col = 3, .codepoint = 'C', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
  {
    .name = "arrow_and_vim_keys_move_cursor",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" }, { .label = "C" } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'k' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "B",
      .lines = {
        "◇  P",
        "│  B",
      },
    },
  },
  {
    .name = "escape_cancels_and_keeps_options",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B", .selected = true }, { .label = "C" } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ○ A",
        "│  ● B",
        "│  ○ C",
        "└",
      },
    },
  },
  {
    .name = "overflow_below_shows_ellipsis",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
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
        "│  ● C",
        "│  ...",
        "└",
      },
    },
  },
  {
    .name = "overflow_above_shows_ellipsis",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
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
        "│  ● F",
        "└",
      },
    },
  },
  {
    .name = "overflow_both_ways_keeps_height",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = {
          { .label = "A" },
          { .label = "B" },
          { .label = "C" },
          { .label = "D" },
          { .label = "E" },
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
    .name = "inactive_options_are_dimmed",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" } },
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
      .cells = {
        { .row = 1, .col = 3, .codepoint = 0x25cf, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 1, .col = 5, .codepoint = 'A' },
        { .row = 2, .col = 3, .codepoint = 0x25cb, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
        { .row = 2, .col = 5, .codepoint = 'B', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
  {
    .name = "hint_is_dimmed",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A", .hint = "B" }, { .label = "C" } },
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ● A (B)",
        "│  ○ C",
        "└",
      },
      .cells = {
        { .row = 1, .col = 7, .codepoint = '(', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
  {
    .name = "filter_matches_substring",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "AB" }, { .label = "CD" }, { .label = "EF" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'D' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "CD",
      .lines = {
        "◇  P D",
        "│  CD",
      },
    },
  },
  {
    .name = "filter_ignores_case",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "AB" }, { .label = "C" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'b' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "AB",
      .lines = {
        "◇  P ab",
        "│  AB",
      },
    },
  },
  {
    .name = "filter_without_matches_submits_nothing",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'z' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "",
      .lines = {
        "◇  P z",
        "│",
      },
    },
  },
  {
    .name = "filter_takes_vim_keys_as_text",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "JA" }, { .label = "JB" } },
        .filter = true,
      },
    },
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JA",
      .lines = {
        "◇  P j",
        "│  JA",
      },
    },
  },
  {
    .name = "filter_shows_single_hidden_tail_option",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
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
        "│  ● E",
        "│  ○ F",
        "└",
      },
    },
  },
  {
    .name = "filter_placeholder_is_dimmed",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
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
        "│  ● A",
        "└",
      },
      .cells = {
        { .row = 0, .col = 5, .codepoint = 'T', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 90 } },
      },
    },
  },
};

sp_test_each_fn(prompt, select, prompt_case_t, tests, prompt_run_case, .setup = prompt_setup, .teardown = prompt_teardown);
