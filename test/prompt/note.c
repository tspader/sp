#include "prompt.h"

#define NOTE_MAX_LINES 3
#define NOTE_MAX_ARGS 2

typedef struct {
  const c8* text;
  const c8* fmt;
  const c8* args [NOTE_MAX_ARGS];
} line_t;

typedef struct {
  const c8* name;
  const c8* message;
  line_t lines [NOTE_MAX_LINES];
  prompt_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "single_line_box",
    .lines = {
      { .text = "A" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│  A  │",
        "│     │",
        "├─────╯",
      },
    },
  },
  {
    .name = "longest_line_sets_width",
    .lines = {
      { .text = "A" },
      { .text = "BBB" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ───╮",
        "│       │",
        "│  A    │",
        "│  BBB  │",
        "│       │",
        "├───────╯",
      },
    },
  },
  {
    .name = "blank_line_renders_empty_row",
    .lines = {
      { .text = "A" },
      { .text = "" },
      { .text = "B" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│  A  │",
        "│     │",
        "│  B  │",
        "│     │",
        "├─────╯",
      },
    },
  },
  {
    .name = "no_lines_renders_empty_box",
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│     │",
        "├─────╯",
      },
    },
  },
  {
    .name = "fmt_line_styles_only_the_styled_span",
    .lines = {
      { .fmt = "{.green} {}", .args = { "A", "B" } },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ───╮",
        "│       │",
        "│  A B  │",
        "│       │",
        "├───────╯",
      },
      .cells = {
        { .row = 2, .col = 3, .codepoint = 'A', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 32 } },
        { .row = 2, .col = 4, .codepoint = ' ' },
        { .row = 2, .col = 5, .codepoint = 'B' },
      },
    },
  },
  {
    .name = "fmt_line_keeps_color_and_drops_other_styles",
    .lines = {
      { .fmt = "{.cyan .bold}", .args = { "A" } },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│  A  │",
        "│     │",
        "├─────╯",
      },
      .cells = {
        { .row = 2, .col = 3, .codepoint = 'A', .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 36 } },
      },
    },
  },
  {
    .name = "message_splits_on_newlines",
    .message = "A\nBBB",
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ───╮",
        "│       │",
        "│  A    │",
        "│  BBB  │",
        "│       │",
        "├───────╯",
      },
    },
  },
  {
    .name = "empty_message_renders_empty_box",
    .message = "",
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│     │",
        "├─────╯",
      },
    },
  },
};

sp_test_each(prompt, note, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;

  if (it->message) {
    sp_prompt_note(ctx, it->message, "T");
    return prompt_expect_last(t, &it->expect);
  }

  sp_prompt_note_t note = sp_prompt_note_new(sp_test_arena(t), "T");
  sp_carr_for(it->lines, n) {
    line_t line = it->lines[n];
    if (line.fmt) {
      sp_prompt_note_line_fmt(&note, line.fmt, sp_fmt_cstr(line.args[0]), sp_fmt_cstr(line.args[1]));
    } else if (line.text) {
      sp_prompt_note_line(&note, line.text);
    } else {
      break;
    }
  }
  sp_prompt_note_ex(ctx, note);
  return prompt_expect_last(t, &it->expect);
}
