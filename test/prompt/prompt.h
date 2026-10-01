#ifndef PROMPT_TEST_H
#define PROMPT_TEST_H

#define SP_PRIVATE
#define SP_IMP
#define SP_PROMPT_PRIVATE_HEADER
#include "sp/sp_prompt.h"
#include "sp/sp_test.h"

#define PROMPT_COLS 80
#define PROMPT_ROWS 20
#define PROMPT_MAX_LINES 8
#define PROMPT_MAX_CELLS 8
#define PROMPT_MAX_OPTIONS 8
#define PROMPT_MAX_ACTS 8
#define PROMPT_MAX_TEXT 32
#define PROMPT_MAX_TICKS 64

typedef struct {
  sp_prompt_state_t fire;
  u32 progress;
  u32 status;
  sp_prompt_event_data_t last_progress;
  sp_str_t last_status;
} prompt_probe_t;

typedef struct {
  sp_prompt_ctx_t ctx;
  sp_io_dyn_mem_writer_t writer;
  prompt_probe_t probe;
  sp_app_t* app;
} prompt_fixture_t;

typedef struct {
  u32 row;
  u32 col;
  u32 codepoint;
  sp_prompt_style_t style;
} prompt_cell_t;

typedef enum {
  PROMPT_BOOL_NONE,
  PROMPT_BOOL_FALSE,
  PROMPT_BOOL_TRUE,
} prompt_bool_t;

typedef struct {
  sp_prompt_state_t state;
  const c8* str;
  prompt_bool_t boolean;
  const c8* lines [PROMPT_MAX_LINES];
  const c8* composited [PROMPT_MAX_LINES];
  prompt_cell_t cells [PROMPT_MAX_CELLS];
} prompt_expect_t;

typedef enum {
  PROMPT_WIDGET_NONE,
  PROMPT_WIDGET_INTRO,
  PROMPT_WIDGET_OUTRO,
  PROMPT_WIDGET_MESSAGE,
  PROMPT_WIDGET_TEXT,
  PROMPT_WIDGET_PASSWORD,
  PROMPT_WIDGET_CONFIRM,
  PROMPT_WIDGET_SELECT,
  PROMPT_WIDGET_MULTISELECT,
} prompt_widget_kind_t;

typedef struct {
  const c8* prompt;
  sp_prompt_select_option_t options [PROMPT_MAX_OPTIONS];
  u32 max_visible;
  bool filter;
} prompt_choice_t;

typedef struct {
  prompt_widget_kind_t kind;
  union {
    struct { const c8* text; } intro;
    struct { const c8* text; } outro;
    struct { const c8* text; u32 symbol; u8 ansi; } message;
    struct { const c8* prompt; const c8* prefill; } text;
    struct { const c8* prompt; const c8* prefill; } password;
    sp_prompt_confirm_t confirm;
    prompt_choice_t select;
    prompt_choice_t multiselect;
  };
} prompt_widget_desc_t;

typedef struct {
  const c8* name;
  prompt_widget_desc_t widget;
  sp_prompt_event_t events [SP_PROMPT_PRIMED_EVENT_CAP];
  prompt_expect_t expect;
} prompt_case_t;

typedef enum {
  PROMPT_ACT_NONE,
  PROMPT_ACT_PROGRESS,
  PROMPT_ACT_STATUS,
  PROMPT_ACT_LOG,
  PROMPT_ACT_COMPLETE,
  PROMPT_ACT_ABORT,
  PROMPT_ACT_TICK,
  PROMPT_ACT_RUN,
} prompt_act_kind_t;

typedef struct {
  prompt_act_kind_t kind;
  bool threaded;
  union {
    f32 value;
    const c8* text;
    u32 ticks;
  };
} prompt_act_t;

sp_err_t           prompt_setup(sp_test_t* t);
void               prompt_teardown(sp_test_t* t);
prompt_fixture_t*  prompt_fixture(sp_test_t* t);
sp_str_t           prompt_written(sp_test_t* t);
sp_str_t           prompt_text(sp_test_t* t);

sp_prompt_widget_t prompt_widget(sp_test_t* t, prompt_widget_desc_t* desc);
sp_err_t           prompt_run(sp_test_t* t, sp_prompt_widget_t widget, sp_prompt_event_t* events, const prompt_expect_t* expect);
sp_err_t           prompt_run_case(sp_test_t* t, prompt_case_t* it);

void               prompt_start(sp_test_t* t, sp_prompt_widget_t widget);
void               prompt_send(sp_test_t* t, sp_prompt_event_t event);
void               prompt_tick(sp_test_t* t);

sp_err_t           prompt_act(sp_test_t* t, const prompt_act_t* acts);

sp_err_t           prompt_expect_last(sp_test_t* t, const prompt_expect_t* expect);
sp_err_t           prompt_expect_live(sp_test_t* t, const prompt_expect_t* expect);

#endif
