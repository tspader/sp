#include "prompt.h"

typedef struct {
  const c8* name;
  u32 width;
  prompt_act_t sends [PROMPT_MAX_ACTS];
  prompt_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "bar_starts_empty",
    .width = 8,
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ──────── 0%",
        "└",
      },
    },
  },
  {
    .name = "quarter_fills_two_of_eight_cells",
    .width = 8,
    .sends = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.25f },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ██────── 25%",
        "└",
      },
    },
  },
  {
    .name = "below_zero_clamps_to_empty",
    .width = 4,
    .sends = {
      { .kind = PROMPT_ACT_PROGRESS, .value = -0.5f },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ──── 0%",
        "└",
      },
    },
  },
  {
    .name = "above_one_clamps_to_full",
    .width = 4,
    .sends = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 5.f },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ████ 100%",
        "└",
      },
    },
  },
  {
    .name = "status_renders_below_bar",
    .width = 8,
    .sends = {
      { .kind = PROMPT_ACT_PROGRESS, .value = 0.5f },
      { .kind = PROMPT_ACT_STATUS, .text = "A" },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  P",
        "│  ████──── 50%",
        "│  A",
        "└",
      },
    },
  },
};

sp_test_each(prompt, progress, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_widget_t widget = sp_prompt_progress_widget(&prompt_fixture(t)->ctx, (sp_prompt_progress_t) {
    .prompt = "P",
    .width = it->width,
  });
  sp_prompt_event_t events [SP_PROMPT_PRIMED_EVENT_CAP] = {
    { .kind = SP_PROMPT_EVENT_CTRL_C },
  };

  sp_try(prompt_act(t, it->sends));
  return prompt_run(t, widget, events, &it->expect);
}
