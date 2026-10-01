#include "prompt.h"

typedef struct {
  const c8* name;
  prompt_widget_desc_t widget;
  prompt_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "confirm_rail_is_blue",
    .widget = {
      .kind = PROMPT_WIDGET_CONFIRM,
      .confirm = { .prompt = "P" },
    },
    .expect = {
      .lines = {
        "◆  P",
        "│  ○ Yes / ● No",
        "└",
      },
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x25c6, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
        { .row = 1, .col = 0, .codepoint = 0x2502, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
        { .row = 2, .col = 0, .codepoint = 0x2514, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
      },
    },
  },
  {
    .name = "select_rail_is_blue",
    .widget = {
      .kind = PROMPT_WIDGET_SELECT,
      .select = {
        .prompt = "P",
        .options = { { .label = "A" }, { .label = "B" } },
      },
    },
    .expect = {
      .lines = {
        "◆  P",
        "│  ● A",
        "│  ○ B",
        "└",
      },
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x25c6, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
        { .row = 1, .col = 0, .codepoint = 0x2502, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
        { .row = 2, .col = 0, .codepoint = 0x2502, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
        { .row = 3, .col = 0, .codepoint = 0x2514, .style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 34 } },
      },
    },
  },
};

sp_test_each(prompt, active, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  prompt_start(t, prompt_widget(t, &it->widget));
  return prompt_expect_live(t, &it->expect);
}
