#include "prompt.h"

#define FLOW_MAX_STEPS 3

typedef struct {
  bool again;
  const c8* log;
  prompt_widget_desc_t widget;
  sp_prompt_event_t events [SP_PROMPT_PRIMED_EVENT_CAP];
  prompt_expect_t expect;
} step_t;

typedef struct {
  const c8* name;
  step_t steps [FLOW_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "select_then_outro_renders_separator",
    .steps = {
      {
        .widget = {
          .kind = PROMPT_WIDGET_SELECT,
          .select = {
            .prompt = "P",
            .options = { { .label = "A" }, { .label = "B", .selected = true } },
          },
        },
        .events = {
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
        .widget = {
          .kind = PROMPT_WIDGET_OUTRO,
          .outro = { .text = "C" },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .lines = { "└  C" },
          .composited = {
            "◇  P",
            "│  B",
            "│",
            "└  C",
          },
        },
      },
    },
  },
  {
    .name = "confirm_then_outro_renders_separator",
    .steps = {
      {
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
        },
      },
      {
        .widget = {
          .kind = PROMPT_WIDGET_OUTRO,
          .outro = { .text = "A" },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .lines = { "└  A" },
          .composited = {
            "◇  P",
            "│  ○ Yes / ● No",
            "│",
            "└  A",
          },
        },
      },
    },
  },
  {
    .name = "intro_text_outro_renders_separators",
    .steps = {
      {
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
        .widget = {
          .kind = PROMPT_WIDGET_TEXT,
          .text = { .prompt = "P" },
        },
        .events = {
          { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'B' } },
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
        .widget = {
          .kind = PROMPT_WIDGET_OUTRO,
          .outro = { .text = "C" },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .lines = { "└  C" },
          .composited = {
            "┌  A",
            "│",
            "◇  P",
            "│  B",
            "│",
            "└  C",
          },
        },
      },
    },
  },
  {
    .name = "log_renders_above_frame",
    .steps = {
      {
        .log = "A",
        .widget = {
          .kind = PROMPT_WIDGET_INTRO,
          .intro = { .text = "B" },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .lines = { "┌  B" },
          .composited = {
            "A",
            "┌  B",
          },
        },
      },
    },
  },
  {
    .name = "select_filter_resets_between_runs",
    .steps = {
      {
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
        .again = true,
        .events = {
          { .kind = SP_PROMPT_EVENT_ENTER },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .str = "AB",
          .lines = {
            "◇  P Type to filter...",
            "│  AB",
          },
        },
      },
    },
  },
  {
    .name = "multiselect_filter_resets_and_selection_persists_between_runs",
    .steps = {
      {
        .widget = {
          .kind = PROMPT_WIDGET_MULTISELECT,
          .multiselect = {
            .prompt = "P",
            .options = { { .label = "AB" }, { .label = "CD" } },
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
        .again = true,
        .events = {
          { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
          { .kind = SP_PROMPT_EVENT_ENTER },
        },
        .expect = {
          .state = SP_PROMPT_STATE_SUBMIT,
          .lines = {
            "◇  P Type to filter...",
            "│  AB, CD",
          },
        },
      },
    },
  },
};

sp_test_each(prompt, flow, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  prompt_widget_desc_t* widget = SP_NULLPTR;

  sp_carr_for(it->steps, n) {
    step_t* step = &it->steps[n];
    if (!step->again) {
      if (step->widget.kind == PROMPT_WIDGET_NONE) break;
      widget = &step->widget;
    }

    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(n)));
    if (step->log) {
      sp_prompt_log(&prompt_fixture(t)->ctx, step->log);
    }
    sp_try(prompt_run(t, prompt_widget(t, widget), step->events, &step->expect));
  }
  return SP_OK;
}
