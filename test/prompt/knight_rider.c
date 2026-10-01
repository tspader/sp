#include "prompt.h"

typedef struct {
  const c8* name;
  sp_prompt_knight_rider_t config;
  u32 frames;
  u32 early_ns;
  prompt_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lead_starts_at_origin",
    .config = { .prompt = "P" },
    .expect = {
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
      },
    },
  },
  {
    .name = "lead_advances_and_leaves_trail",
    .config = { .prompt = "P" },
    .frames = 3,
    .expect = {
      .cells = {
        { .row = 0, .col = 3, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
        { .row = 0, .col = 2, .codepoint = 0x25C6, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF, .g = 0x55, .b = 0x55 } } },
        { .row = 0, .col = 1, .codepoint = 0x2B29, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xDD } } },
      },
    },
  },
  {
    .name = "lead_holds_at_right_edge",
    .config = { .prompt = "P" },
    .frames = 6,
    .expect = {
      .cells = {
        { .row = 0, .col = 5, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
      },
    },
  },
  {
    .name = "lead_moves_backward_after_hold",
    .config = { .prompt = "P" },
    .frames = 17,
    .expect = {
      .cells = {
        { .row = 0, .col = 2, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
      },
    },
  },
  {
    .name = "wraps_after_full_cycle",
    .config = { .prompt = "P" },
    .frames = SP_PROMPT_KR_WIDTH + SP_PROMPT_KR_HOLD_END + (SP_PROMPT_KR_WIDTH - 1) + SP_PROMPT_KR_HOLD_START,
    .expect = {
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
      },
    },
  },
  {
    .name = "frame_holds_until_interval_elapses",
    .config = { .prompt = "P" },
    .frames = 1,
    .early_ns = 1,
    .expect = {
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 0xFF } } },
      },
    },
  },
  {
    .name = "custom_color_replaces_default_palette",
    .config = {
      .prompt = "P",
      .color = { .r = 100, .g = 200, .b = 50 },
    },
    .expect = {
      .cells = {
        { .row = 0, .col = 0, .codepoint = 0x2B25, .style = { .tag = SP_PROMPT_STYLE_RGB, .rgb = { .r = 100, .g = 200, .b = 50 } } },
      },
    },
  },
};

sp_test_each(prompt, knight_rider, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&prompt_fixture(t)->ctx, it->config);
  sp_prompt_knight_rider_widget_t* kr = (sp_prompt_knight_rider_widget_t*)widget.user_data;

  prompt_start(t, widget);
  kr->elapsed_ns = (u64)it->frames * kr->config.ex.interval * SP_TM_MS_TO_NS - it->early_ns;
  prompt_send(t, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_TAB });

  return prompt_expect_live(t, &it->expect);
}
