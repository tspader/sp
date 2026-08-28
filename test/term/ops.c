#include "term.h"

#define TERM_OPS_MAX_OPS 8

typedef enum {
  TERM_OP_NONE = 0,
  TERM_OP_STYLE,
  TERM_OP_RGB,
  TERM_OP_RESET,
  TERM_OP_TEXT,
} term_op_kind_t;

typedef struct {
  term_op_kind_t kind;
  sp_fmt_style_t style;
  u8 rgb [3];
  const c8* text;
} term_op_t;

typedef struct {
  const c8* text;
} term_ops_expect_t;

typedef struct {
  sp_term_color_t color;
  term_op_t ops [TERM_OPS_MAX_OPS];
  term_ops_expect_t expect;
} term_ops_test_t;

static void run_term_ops(s32* utest_result, term_ops_test_t t) {
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &io);
  sp_term_t term = { .io = &io.base, .color = t.color };

  sp_carr_for(t.ops, it) {
    term_op_t op = t.ops[it];
    if (op.kind == TERM_OP_NONE) break;
    switch (op.kind) {
      case TERM_OP_NONE:  break;
      case TERM_OP_STYLE: EXPECT_EQ(sp_term_style(&term, op.style), SP_OK); break;
      case TERM_OP_RGB:   EXPECT_EQ(sp_term_rgb(&term, op.rgb[0], op.rgb[1], op.rgb[2]), SP_OK); break;
      case TERM_OP_RESET: EXPECT_EQ(sp_term_reset(&term), SP_OK); break;
      case TERM_OP_TEXT:  EXPECT_EQ(sp_io_write_cstr(term.io, op.text, SP_NULLPTR), SP_OK); break;
    }
  }

  SP_EXPECT_STR_EQ_CSTR(sp_io_dyn_mem_writer_as_str(&io), t.expect.text);
}

UTEST(term_ops, ansi) {
  term_ops_test_t cases[] = {
    {
      .color = SP_TERM_COLOR_ANSI,
      .ops = {
        { .kind = TERM_OP_STYLE, .style = sp_fmt_style_red },
        { .kind = TERM_OP_TEXT, .text = "x" },
        { .kind = TERM_OP_RESET },
      },
      .expect = { "\033[31mx\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .ops = {
        { .kind = TERM_OP_RGB, .rgb = { 250, 100, 25 } },
        { .kind = TERM_OP_TEXT, .text = "x" },
        { .kind = TERM_OP_RESET },
      },
      .expect = { "\033[38;2;250;100;25mx\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .ops = {
        { .kind = TERM_OP_STYLE, .style = sp_fmt_style_quote },
        { .kind = TERM_OP_TEXT, .text = "x" },
      },
      .expect = { "x" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_ops(utest_result, cases[i]);
}

UTEST(term_ops, plain) {
  term_ops_test_t cases[] = {
    {
      .ops = {
        { .kind = TERM_OP_STYLE, .style = sp_fmt_style_red },
        { .kind = TERM_OP_TEXT, .text = "x" },
        { .kind = TERM_OP_RESET },
      },
      .expect = { "x" }
    },
    {
      .ops = {
        { .kind = TERM_OP_RGB, .rgb = { 250, 100, 25 } },
        { .kind = TERM_OP_TEXT, .text = "x" },
        { .kind = TERM_OP_RESET },
      },
      .expect = { "x" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_ops(utest_result, cases[i]);
}
