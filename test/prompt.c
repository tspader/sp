#include "sp.h"
#define SP_UNIMPLEMENTED() ((void)0)
#define LINENOISE_IMPLEMENTATION
#define SP_PROMPT_IMPLEMENTATION
#include "sp/sp_prompt.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

typedef struct {
  sp_prompt_state_t state;
  const c8* str;
  s32 boolean;
  const c8* lines[32];
  const c8* composited[32];
} sp_prompt_expect_t;

typedef enum {
  SP_PROMPT_TEST_TRISTATE_NONE,
  SP_PROMPT_TEST_TRISTATE_FALSE,
  SP_PROMPT_TEST_TRISTATE_TRUE,
} sp_prompt_test_tristate_t;

typedef struct {
  sp_prompt_widget_t widget;
  sp_prompt_event_t events[8];
  sp_prompt_expect_t expect;
} sp_prompt_case_t;

struct prompt {
  sp_prompt_ctx_t ctx;
  sp_io_writer_t writer;
  sp_app_t* app;
  sp_mem_arena_t* arena;
  sp_mem_t mem;
};

void sp_prompt_step(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event);
void sp_prompt_tick(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget);

UTEST_F_SETUP(prompt) {
  ut.arena = sp_mem_arena_new();
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  ut.ctx = SP_ZERO_STRUCT(sp_prompt_ctx_t);
  ut.app = SP_NULLPTR;
  sp_prompt_ctx_init(&ut.ctx, 80, 20);
  sp_io_writer_from_dyn_mem_a(ut.mem, &ut.writer);
  ut.ctx.writer = &ut.writer;
}

UTEST_F_TEARDOWN(prompt) {
  sp_app_destroy(ut.app);
  sp_io_writer_close(&ut.writer);
  sp_mem_arena_destroy(ut.arena);
}

static u32 count_expected_lines(const c8* lines[32]) {
  u32 count = 0;
  while (count < 32 && lines[count] != SP_NULLPTR) {
    count++;
  }
  return count;
}

static sp_str_t trim_framebuffer_row(sp_prompt_cell_t* row, u32 cols) {
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  sp_for(col, cols) {
    u8 buf[4] = SP_ZERO_INITIALIZE();
    u8 len = sp_utf8_encode(row[col].codepoint, buf);
    sp_io_write_str(&builder, sp_str(buf, len), SP_NULLPTR);
  }

  sp_str_t str = sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
  while (!sp_str_empty(str) && str.data[str.len - 1] == ' ') {
    str.len--;
  }
  return str;
}

static sp_prompt_frame_t sp_prompt_last_frame(sp_prompt_ctx_t* ctx) {
  return ctx->frames[sp_da_size(ctx->frames) - 1];
}

static sp_prompt_cell_t sp_prompt_frame_cell(sp_prompt_frame_t frame, u32 row, u32 col) {
  return frame.cells[row * frame.cols + col];
}

typedef struct {
  u32 row;
  u32 col;
  u32 max_row;
  u32 cells[128][256];
} sp_prompt_vt_t;

static void sp_prompt_vt_clear_eos(sp_prompt_vt_t* vt) {
  sp_for_range(col, vt->col, 256) {
    vt->cells[vt->row][col] = ' ';
  }

  sp_for_range(row, vt->row + 1, 128) {
    sp_for(col, 256) {
      vt->cells[row][col] = ' ';
    }
  }

  s32 last_non_empty = -1;
  sp_for(row, 128) {
    bool row_non_empty = false;
    sp_for(col, 256) {
      if (vt->cells[row][col] != ' ') {
        row_non_empty = true;
        break;
      }
    }
    if (row_non_empty) {
      last_non_empty = (s32)row;
    }
  }

  vt->max_row = last_non_empty < 0 ? 0 : (u32)last_non_empty;
}

static void sp_prompt_vt_put(sp_prompt_vt_t* vt, u32 cp) {
  if (vt->row >= 128 || vt->col >= 256) {
    return;
  }

  vt->cells[vt->row][vt->col] = cp;
  vt->col++;
  if (vt->row > vt->max_row) {
    vt->max_row = vt->row;
  }
}

static void sp_prompt_vt_consume_escape(sp_prompt_vt_t* vt, sp_str_t bytes, u32* i) {
  u32 p = *i;
  if (p + 1 >= bytes.len || bytes.data[p] != 0x1b || bytes.data[p + 1] != '[') {
    return;
  }

  p += 2;
  while (p < bytes.len) {
    c8 ch = bytes.data[p];
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
      if (ch == 'A') {
        if (vt->row > 0) {
          vt->row--;
        }
      } else if (ch == 'J') {
        sp_prompt_vt_clear_eos(vt);
      }
      *i = p;
      return;
    }
    p++;
  }

  *i = bytes.len - 1;
}

static sp_da(sp_str_t) sp_prompt_vt_render_lines(sp_str_t bytes) {
  sp_prompt_vt_t vt = SP_ZERO_INITIALIZE();
  sp_for(row, 128) {
    sp_for(col, 256) {
      vt.cells[row][col] = ' ';
    }
  }

  u32 i = 0;
  while (i < bytes.len) {
    u8 b = (u8)bytes.data[i];

    if (b == 0x1b) {
      sp_prompt_vt_consume_escape(&vt, bytes, &i);
      i++;
      continue;
    }

    if (b == '\r') {
      vt.col = 0;
      i++;
      continue;
    }

    if (b == '\n') {
      if (vt.row + 1 < 128) {
        vt.row++;
      }
      vt.col = 0;
      if (vt.row > vt.max_row) {
        vt.max_row = vt.row;
      }
      i++;
      continue;
    }

    u8 utf8[4] = {0};
    utf8[0] = b;
    u32 count = 1;
    if ((b & 0xE0) == 0xC0) {
      count = 2;
    } else if ((b & 0xF0) == 0xE0) {
      count = 3;
    } else if ((b & 0xF8) == 0xF0) {
      count = 4;
    }

    sp_for_range(j, 1, count) {
      if (i + j < bytes.len) {
        utf8[j] = (u8)bytes.data[i + j];
      }
    }

    sp_prompt_vt_put(&vt, sp_utf8_decode(utf8));
    i += count;
  }

  sp_da(sp_str_t) lines = SP_NULLPTR;
  sp_for_range(row, 0, vt.max_row + 1) {
    sp_io_writer_t builder = sp_zero();
    sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
    sp_for(col, 256) {
      u8 buf[4] = SP_ZERO_INITIALIZE();
      u8 len = sp_utf8_encode(vt.cells[row][col], buf);
      sp_io_write_str(&builder, sp_str(buf, len), SP_NULLPTR);
    }

    sp_str_t line = sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
    while (!sp_str_empty(line) && line.data[line.len - 1] == ' ') {
      line.len--;
    }
    sp_da_push(lines, line);
  }

  return lines;
}

static void sp_prompt_run_case(s32* utest_result, struct prompt* fixture, sp_prompt_case_t t) {
  sp_prompt_prime_events(&fixture->ctx, t.events);
  bool submitted = sp_prompt_run(&fixture->ctx, t.widget);

  EXPECT_EQ(submitted, t.expect.state == SP_PROMPT_STATE_SUBMIT);
  EXPECT_EQ(fixture->ctx.state, t.expect.state);
  EXPECT_EQ(sp_prompt_submitted(&fixture->ctx), t.expect.state == SP_PROMPT_STATE_SUBMIT);
  EXPECT_EQ(sp_prompt_cancelled(&fixture->ctx), t.expect.state == SP_PROMPT_STATE_CANCEL);

  if (t.expect.str) {
    EXPECT_STREQ(sp_prompt_get_str(&fixture->ctx), t.expect.str);
  }

  if (t.expect.boolean != SP_PROMPT_TEST_TRISTATE_NONE) {
    bool expected = t.expect.boolean == SP_PROMPT_TEST_TRISTATE_TRUE;
    EXPECT_EQ(sp_prompt_get_bool(&fixture->ctx), expected);
  }

  if (t.expect.lines[0]) {
    ASSERT_TRUE(sp_da_size(fixture->ctx.frames) > 0);
    sp_prompt_frame_t frame = fixture->ctx.frames[sp_da_size(fixture->ctx.frames) - 1];

    u32 num_expected = count_expected_lines(t.expect.lines);
    EXPECT_EQ(frame.rows, num_expected);

    sp_for(row, num_expected) {
      sp_str_t actual = trim_framebuffer_row(frame.cells + row * frame.cols, frame.cols);
      SP_EXPECT_STR_EQ_CSTR(actual, t.expect.lines[row]);
    }
  }

  if (t.expect.composited[0]) {
    sp_str_t bytes = {
      .data = (c8*)fixture->writer.dyn_mem.buffer.data,
      .len = (u32)fixture->writer.dyn_mem.buffer.len,
    };
    sp_da(sp_str_t) composited = sp_prompt_vt_render_lines(bytes);
    u32 num_expected = count_expected_lines(t.expect.composited);
    EXPECT_EQ(sp_da_size(composited), num_expected);
    sp_for(row, num_expected) {
      SP_EXPECT_STR_EQ_CSTR(composited[row], t.expect.composited[row]);
    }
  }
}

UTEST_F(prompt, intro_submits_without_update) {
  sp_prompt_intro_t intro = {
    .text = sp_str_view("hello"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_intro_widget(&ut.ctx, intro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "┌  hello",
      },
    },
  });
}

UTEST_F(prompt, outro_submits_without_update) {
  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_outro_widget(&ut.ctx, outro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "└  done",
      },
    },
  });
}

UTEST_F(prompt, note_renders_single_line_box) {
  sp_prompt_note_t note = {
    .message = sp_str_view("x"),
    .title = sp_str_view("T"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_note_widget(&ut.ctx, note),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ─╮",
        "│     │",
        "│  x  │",
        "│     │",
        "├─────╯",
      },
    },
  });
}

UTEST_F(prompt, note_uses_longest_line_for_width) {
  sp_prompt_note_t note = {
    .message = sp_str_view("a\nbbb"),
    .title = sp_str_view("T"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_note_widget(&ut.ctx, note),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  T ───╮",
        "│       │",
        "│  a    │",
        "│  bbb  │",
        "│       │",
        "├───────╯",
      },
    },
  });
}

UTEST_F(prompt, static_widget_ignores_provided_events) {
  sp_prompt_intro_t intro = {
    .text = sp_str_view("eventless"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_intro_widget(&ut.ctx, intro),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'x' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "┌  eventless",
      },
    },
  });
}

UTEST_F(prompt, message_info_renders_symbol_line) {
  sp_prompt_message_t message = {
    .text = sp_str_view("heads up"),
    .symbol = 0x25cf,
    .ansi = 36,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_message_widget(&ut.ctx, message),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "●  heads up",
      },
    },
  });
}

UTEST_F(prompt, message_error_renders_symbol_line) {
  sp_prompt_message_t message = {
    .text = sp_str_view("boom"),
    .symbol = 0x25a0,
    .ansi = 31,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_message_widget(&ut.ctx, message),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "■  boom",
      },
    },
  });
}

UTEST_F(prompt, text_cancels_without_submit_event) {
  sp_prompt_text_t text = {
    .prompt = sp_str_view("Search:"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_text_widget(&ut.ctx, text),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Search:",
        "│  a",
      },
    },
  });
}

UTEST_F(prompt, text_submits_on_enter_with_prefill_fallback) {
  sp_prompt_text_t text = {
    .prompt = sp_str_view("Search:"),
    .prefill = sp_str_view("/home/spader"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_text_widget(&ut.ctx, text),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "/home/spader",
      .lines = {
        "◇  Search:",
        "│  /home/spader",
      },
    },
  });
}

UTEST_F(prompt, confirm_submits_true_on_enter) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_TRUE,
      .lines = {
        "◇  Proceed?",
        "│  ● Yes / ○ No",
      },
    },
  });
}

UTEST_F(prompt, confirm_flips_with_left_and_submits) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_LEFT },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_FALSE,
      .lines = {
        "◇  Proceed?",
        "│  ○ Yes / ● No",
      },
    },
  });
}

UTEST_F(prompt, confirm_flips_with_vim_key_and_submits) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'l' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_TRUE,
      .lines = {
        "◇  Proceed?",
        "│  ● Yes / ○ No",
      },
    },
  });
}

UTEST_F(prompt, confirm_default_value_highlighted_and_escape_cancels) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Proceed?",
        "│  ○ Yes / ● No",
        "└",
      },
    },
  });
}

UTEST_F(prompt, confirm_toggles_with_arrow_and_vim_keys) {
  sp_prompt_confirm_t confirm_arrow = {
    .prompt = "Proceed?",
    .initial = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm_arrow),
    .events = {
      { .kind = SP_PROMPT_EVENT_RIGHT },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_TRUE,
      .lines = {
        "◇  Proceed?",
        "│  ● Yes / ○ No",
      },
    },
  });

  sp_prompt_confirm_t confirm_vim = {
    .prompt = "Proceed?",
    .initial = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm_vim),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'h' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_FALSE,
      .lines = {
        "◇  Proceed?",
        "│  ○ Yes / ● No",
      },
    },
  });
}

void sp_prompt_tick(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  if (ctx->state == SP_PROMPT_STATE_ACTIVE && ctx->widget.on_update) {
    ctx->widget.on_update(ctx);
    sp_prompt_render_frame(ctx, ctx->widget);
  }
}

void sp_prompt_step(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event) {
  ctx->widget = widget;
  ctx->user_data = widget.user_data;
  sp_prompt_dispatch_event(ctx, widget, event);
  sp_prompt_render_frame(ctx, widget);
}

UTEST_F(prompt, confirm_active_rail_is_blue) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = false,
  };

  sp_prompt_step(
    &ut.ctx,
    sp_prompt_confirm_widget(&ut.ctx, confirm),
    (sp_prompt_event_t) {
      .kind = SP_PROMPT_EVENT_INIT,
    }
  );

  sp_prompt_frame_t frame = {
    .cols = ut.ctx.cols,
    .rows = ut.ctx.cursor_row,
    .cells = ut.ctx.framebuffer,
  };

  EXPECT_EQ(frame.rows, 3);

  sp_prompt_cell_t symbol = sp_prompt_frame_cell(frame, 0, 0);
  sp_prompt_cell_t rail = sp_prompt_frame_cell(frame, 1, 0);
  sp_prompt_cell_t cap = sp_prompt_frame_cell(frame, 2, 0);

  EXPECT_EQ(symbol.codepoint, 0x25c6);
  EXPECT_EQ(symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(symbol.style.ansi, 34);

  EXPECT_EQ(rail.codepoint, 0x2502);
  EXPECT_EQ(rail.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(rail.style.ansi, 34);

  EXPECT_EQ(cap.codepoint, 0x2514);
  EXPECT_EQ(cap.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(cap.style.ansi, 34);
}

UTEST_F(prompt, confirm_styles_true_active_false_inactive) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_TRUE,
      .lines = {
        "◇  Proceed?",
        "│  ● Yes / ○ No",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);

  sp_prompt_cell_t active_symbol = sp_prompt_frame_cell(frame, 1, 3);
  sp_prompt_cell_t active_text = sp_prompt_frame_cell(frame, 1, 5);
  sp_prompt_cell_t inactive_symbol = sp_prompt_frame_cell(frame, 1, 11);
  sp_prompt_cell_t inactive_text = sp_prompt_frame_cell(frame, 1, 13);

  EXPECT_EQ(active_symbol.codepoint, 0x25cf);
  EXPECT_EQ(active_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(active_symbol.style.ansi, 32);

  EXPECT_EQ(active_text.codepoint, 'Y');
  EXPECT_EQ(active_text.style.tag, SP_PROMPT_STYLE_NONE);

  EXPECT_EQ(inactive_symbol.codepoint, 0x25cb);
  EXPECT_EQ(inactive_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_symbol.style.ansi, 90);

  EXPECT_EQ(inactive_text.codepoint, 'N');
  EXPECT_EQ(inactive_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_text.style.ansi, 90);
}

UTEST_F(prompt, confirm_styles_false_active_true_inactive) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Proceed?",
    .initial = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_FALSE,
      .lines = {
        "◇  Proceed?",
        "│  ○ Yes / ● No",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);

  sp_prompt_cell_t inactive_symbol = sp_prompt_frame_cell(frame, 1, 3);
  sp_prompt_cell_t inactive_text = sp_prompt_frame_cell(frame, 1, 5);
  sp_prompt_cell_t active_symbol = sp_prompt_frame_cell(frame, 1, 11);
  sp_prompt_cell_t active_text = sp_prompt_frame_cell(frame, 1, 13);

  EXPECT_EQ(inactive_symbol.codepoint, 0x25cb);
  EXPECT_EQ(inactive_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_symbol.style.ansi, 90);

  EXPECT_EQ(inactive_text.codepoint, 'Y');
  EXPECT_EQ(inactive_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_text.style.ansi, 90);

  EXPECT_EQ(active_symbol.codepoint, 0x25cf);
  EXPECT_EQ(active_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(active_symbol.style.ansi, 32);

  EXPECT_EQ(active_text.codepoint, 'N');
  EXPECT_EQ(active_text.style.tag, SP_PROMPT_STYLE_NONE);
}

UTEST_F(prompt, select_submits_initial_value_on_enter) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust", .selected = true },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "Rust",
      .lines = {
        "◇  Pick language",
        "│  Rust",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);
  sp_prompt_cell_t selected_text = sp_prompt_frame_cell(frame, 1, 3);

  EXPECT_EQ(selected_text.codepoint, 'R');
  EXPECT_EQ(selected_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(selected_text.style.ansi, 90);
}

UTEST_F(prompt, select_toggles_with_arrow_and_vim_keys) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'k' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick language",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, select_escape_cancels) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript", .selected = true },
    { .label = "Rust" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick language",
        "│  ○ TypeScript",
        "│  ● JavaScript",
        "│  ○ Rust",
        "└",
      },
    },
  });
}

UTEST_F(prompt, select_limits_visible_options) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five" },
    { .label = "Six" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick number",
    .options = options,
    .num_options = 6,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick number",
        "│  ○ One",
        "│  ○ Two",
        "│  ● Three",
        "│  ...",
        "└",
      },
    },
  });
}

UTEST_F(prompt, select_top_overflow_shows_ellipsis) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five" },
    { .label = "Six" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick number",
    .options = options,
    .num_options = 6,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
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
        "■  Pick number",
        "│  ...",
        "│  ○ Four",
        "│  ○ Five",
        "│  ● Six",
        "└",
      },
    },
  });
}

UTEST_F(prompt, select_bidirectional_overflow_keeps_height) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five" },
    { .label = "Six" },
    { .label = "Seven" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick number",
    .options = options,
    .num_options = 7,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
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
        "■  Pick number",
        "│  ...",
        "│  ○ Four",
        "│  ● Five",
        "│  ...",
        "└",
      },
    },
  });
}

UTEST_F(prompt, select_styles_active_and_inactive_match_clack) {
  sp_prompt_select_option_t options[] = {
    { .label = "Alpha" },
    { .label = "Beta" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick value",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick value",
        "│  ● Alpha",
        "│  ○ Beta",
        "└",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);

  sp_prompt_cell_t active_symbol = sp_prompt_frame_cell(frame, 1, 3);
  sp_prompt_cell_t active_text = sp_prompt_frame_cell(frame, 1, 5);
  sp_prompt_cell_t inactive_symbol = sp_prompt_frame_cell(frame, 2, 3);
  sp_prompt_cell_t inactive_text = sp_prompt_frame_cell(frame, 2, 5);

  EXPECT_EQ(active_symbol.codepoint, 0x25cf);
  EXPECT_EQ(active_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(active_symbol.style.ansi, 32);

  EXPECT_EQ(active_text.codepoint, 'A');
  EXPECT_EQ(active_text.style.tag, SP_PROMPT_STYLE_NONE);

  EXPECT_EQ(inactive_symbol.codepoint, 0x25cb);
  EXPECT_EQ(inactive_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_symbol.style.ansi, 90);

  EXPECT_EQ(inactive_text.codepoint, 'B');
  EXPECT_EQ(inactive_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(inactive_text.style.ansi, 90);
}

UTEST_F(prompt, select_active_rail_is_blue) {
  sp_prompt_select_option_t options[] = {
    { .label = "Alpha" },
    { .label = "Beta" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick value",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
  };

  sp_prompt_step(
    &ut.ctx,
    sp_prompt_select_widget(&ut.ctx, select),
    (sp_prompt_event_t) {
      .kind = SP_PROMPT_EVENT_INIT,
    }
  );

  sp_prompt_frame_t frame = {
    .cols = ut.ctx.cols,
    .rows = ut.ctx.cursor_row,
    .cells = ut.ctx.framebuffer,
  };

  EXPECT_EQ(frame.rows, 4);

  sp_prompt_cell_t symbol = sp_prompt_frame_cell(frame, 0, 0);
  sp_prompt_cell_t rail_a = sp_prompt_frame_cell(frame, 1, 0);
  sp_prompt_cell_t rail_b = sp_prompt_frame_cell(frame, 2, 0);
  sp_prompt_cell_t cap = sp_prompt_frame_cell(frame, 3, 0);

  EXPECT_EQ(symbol.codepoint, 0x25c6);
  EXPECT_EQ(symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(symbol.style.ansi, 34);

  EXPECT_EQ(rail_a.codepoint, 0x2502);
  EXPECT_EQ(rail_a.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(rail_a.style.ansi, 34);

  EXPECT_EQ(rail_b.codepoint, 0x2502);
  EXPECT_EQ(rail_b.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(rail_b.style.ansi, 34);

  EXPECT_EQ(cap.codepoint, 0x2514);
  EXPECT_EQ(cap.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(cap.style.ansi, 34);
}

UTEST_F(prompt, select_hints_render_diminished) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript", .hint = "recommended" },
    { .label = "JavaScript" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick language",
        "│  ● TypeScript (recommended)",
        "│  ○ JavaScript",
        "└",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);
  sp_prompt_cell_t hint_open = sp_prompt_frame_cell(frame, 1, 16);

  EXPECT_EQ(hint_open.codepoint, '(');
  EXPECT_EQ(hint_open.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(hint_open.style.ansi, 90);
}

UTEST_F(prompt, select_filter_placeholder_and_contains_matching) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick language a",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, select_filter_shows_single_hidden_tail_item) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
    { .label = "Go" },
    { .label = "Python" },
    { .label = "C" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 6,
    .max_visible = 4,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
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
        "■  Pick language Type to filter...",
        "│  ...",
        "│  ○ Rust",
        "│  ○ Go",
        "│  ● Python",
        "│  ○ C",
        "└",
      },
    },
  });
}

UTEST_F(prompt, select_filter_is_case_insensitive) {
  sp_prompt_select_option_t options[] = {
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 's' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'c' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'r' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'i' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'p' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 't' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick language script",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, select_filter_no_matches) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'z' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "",
      .lines = {
        "◇  Pick language z",
        "│",
      },
    },
  });
}

UTEST_F(prompt, select_filter_disables_vim_navigation_keys) {
  sp_prompt_select_option_t options[] = {
    { .label = "JavaScript" },
    { .label = "Jest" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick package",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick package j",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, select_filter_placeholder_is_diminished) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 1,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick language Type to filter...",
        "│  ● TypeScript",
        "└",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);
  sp_prompt_cell_t placeholder_t = sp_prompt_frame_cell(frame, 0, 17);
  EXPECT_EQ(placeholder_t.codepoint, 'T');
  EXPECT_EQ(placeholder_t.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(placeholder_t.style.ansi, 90);
}

UTEST_F(prompt, select_filter_state_resets_between_runs) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_select_t select = {
    .prompt = "Pick language",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick language a",
        "│  JavaScript",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "TypeScript",
      .lines = {
        "◇  Pick language Type to filter...",
        "│  TypeScript",
      },
    },
  });
}

UTEST_F(prompt, select_outro_flow_renders_separator) {
  sp_prompt_select_option_t options[] = {
    { .label = "Yes" },
    { .label = "No", .selected = true },
  };

  sp_prompt_select_t select = {
    .prompt = "Install dependencies?",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
  };

  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&ut.ctx, select),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "No",
      .lines = {
        "◇  Install dependencies?",
        "│  No",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_outro_widget(&ut.ctx, outro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "└  done",
      },
      .composited = {
        "◇  Install dependencies?",
        "│  No",
        "│",
        "└  done",
      },
    },
  });
}

UTEST_F(prompt, multiselect_space_toggles_and_enter_submits) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  Pick tools",
        "│  TypeScript, JavaScript",
      },
    },
  });
}

UTEST_F(prompt, multiselect_escape_cancels) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript", .selected = true },
    { .label = "JavaScript" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick tools",
        "│  ● TypeScript",
        "│  ○ JavaScript",
        "└",
      },
    },
  });
}

UTEST_F(prompt, multiselect_limits_visible_options) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five" },
    { .label = "Six" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 6,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick tools",
        "│  ○ One",
        "│  ○ Two",
        "│  ○ Three",
        "│  ...",
        "└",
      },
    },
  });
}

UTEST_F(prompt, multiselect_top_overflow_shows_ellipsis) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five" },
    { .label = "Six" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 6,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
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
        "■  Pick tools",
        "│  ...",
        "│  ○ Four",
        "│  ○ Five",
        "│  ○ Six",
        "└",
      },
    },
  });
}

UTEST_F(prompt, multiselect_bidirectional_overflow_keeps_height) {
  sp_prompt_select_option_t options[] = {
    { .label = "One" },
    { .label = "Two" },
    { .label = "Three" },
    { .label = "Four" },
    { .label = "Five", .selected = true },
    { .label = "Six" },
    { .label = "Seven" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 7,
    .max_visible = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
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
        "■  Pick tools",
        "│  ...",
        "│  ○ Four",
        "│  ● Five",
        "│  ...",
        "└",
      },
    },
  });
}

UTEST_F(prompt, multiselect_styles_selected_hover_and_dim_rules) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript", .selected = true },
    { .label = "JavaScript", .selected = true },
    { .label = "Rust" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick tools",
        "│  ● TypeScript",
        "│  ● JavaScript",
        "│  ○ Rust",
        "└",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);

  sp_prompt_cell_t selected_not_hover_symbol = sp_prompt_frame_cell(frame, 1, 3);
  sp_prompt_cell_t selected_not_hover_text = sp_prompt_frame_cell(frame, 1, 5);
  sp_prompt_cell_t selected_hover_symbol = sp_prompt_frame_cell(frame, 2, 3);
  sp_prompt_cell_t selected_hover_text = sp_prompt_frame_cell(frame, 2, 5);
  sp_prompt_cell_t unselected_not_hover_symbol = sp_prompt_frame_cell(frame, 3, 3);
  sp_prompt_cell_t unselected_not_hover_text = sp_prompt_frame_cell(frame, 3, 5);

  EXPECT_EQ(selected_not_hover_symbol.codepoint, 0x25cf);
  EXPECT_EQ(selected_not_hover_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(selected_not_hover_symbol.style.ansi, 32);
  EXPECT_EQ(selected_not_hover_text.codepoint, 'T');
  EXPECT_EQ(selected_not_hover_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(selected_not_hover_text.style.ansi, 90);

  EXPECT_EQ(selected_hover_symbol.codepoint, 0x25cf);
  EXPECT_EQ(selected_hover_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(selected_hover_symbol.style.ansi, 32);
  EXPECT_EQ(selected_hover_text.codepoint, 'J');
  EXPECT_EQ(selected_hover_text.style.tag, SP_PROMPT_STYLE_NONE);

  EXPECT_EQ(unselected_not_hover_symbol.codepoint, 0x25cb);
  EXPECT_EQ(unselected_not_hover_symbol.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(unselected_not_hover_symbol.style.ansi, 90);
  EXPECT_EQ(unselected_not_hover_text.codepoint, 'R');
  EXPECT_EQ(unselected_not_hover_text.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(unselected_not_hover_text.style.ansi, 90);
}

UTEST_F(prompt, multiselect_filter_contains_and_placeholder) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 3,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  Pick tools a",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, multiselect_filter_shows_single_hidden_tail_item) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
    { .label = "Go" },
    { .label = "Python" },
    { .label = "C" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 6,
    .max_visible = 4,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
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
        "■  Pick tools Type to filter...",
        "│  ...",
        "│  ○ Rust",
        "│  ○ Go",
        "│  ○ Python",
        "│  ○ C",
        "└",
      },
    },
  });
}

UTEST_F(prompt, multiselect_filter_disables_vim_navigation_keys) {
  sp_prompt_select_option_t options[] = {
    { .label = "JavaScript" },
    { .label = "Jest" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  Pick tools j",
        "│  JavaScript",
      },
    },
  });
}

UTEST_F(prompt, multiselect_filter_placeholder_is_diminished) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 1,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_ESCAPE },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Pick tools Type to filter...",
        "│  ○ TypeScript",
        "└",
      },
    },
  });

  ASSERT_TRUE(!sp_da_empty(ut.ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&ut.ctx);
  sp_prompt_cell_t placeholder_t = sp_prompt_frame_cell(frame, 0, 14);
  EXPECT_EQ(placeholder_t.codepoint, 'T');
  EXPECT_EQ(placeholder_t.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(placeholder_t.style.ansi, 90);
}

UTEST_F(prompt, multiselect_filter_state_resets_between_runs) {
  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
  };

  sp_prompt_multiselect_t multiselect = {
    .prompt = "Pick tools",
    .options = options,
    .num_options = 2,
    .max_visible = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  Pick tools a",
        "│  JavaScript",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&ut.ctx, multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "◇  Pick tools Type to filter...",
        "│  TypeScript, JavaScript",
      },
    },
  });
}

UTEST_F(prompt, password_submits_prefill_when_empty) {
  sp_prompt_password_t password = {
    .prompt = sp_str_view("Password"),
    .prefill = sp_str_view("sekret"),
    .mask = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_password_widget(&ut.ctx, password),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "sekret",
      .lines = {
        "◇  Password",
        "│  ******",
      },
    },
  });
}

UTEST_F(prompt, password_tab_reveals_value_before_submit) {
  sp_prompt_password_t password = {
    .prompt = sp_str_view("Password"),
    .mask = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_password_widget(&ut.ctx, password),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'b' } },
      { .kind = SP_PROMPT_EVENT_TAB },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "ab",
      .lines = {
        "◇  Password",
        "│  ab",
      },
    },
  });
}

UTEST_F(prompt, intro_text_outro_flow_render_bug) {
  sp_prompt_intro_t intro = {
    .text = sp_str_view("hello"),
  };
  sp_prompt_text_t text = {
    .prompt = sp_str_view("Search:"),
  };
  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_intro_widget(&ut.ctx, intro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "┌  hello",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_text_widget(&ut.ctx, text),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'x' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "x",
      .lines = {
        "◇  Search:",
        "│  x",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_outro_widget(&ut.ctx, outro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "└  done",
      },
      .composited = {
        "┌  hello",
        "│",
        "◇  Search:",
        "│  x",
        "│",
        "└  done",
      },
    },
  });
}

static void expect_framebuffer_lines(s32* utest_result, struct prompt* fixture, const c8* expected[32]) {
  if (!expected[0]) return;

  sp_prompt_frame_t frame = {
    .cols = fixture->ctx.cols,
    .rows = fixture->ctx.cursor_row,
    .cells = fixture->ctx.framebuffer,
  };

  u32 num_expected = count_expected_lines(expected);
  EXPECT_EQ(frame.rows, num_expected);

  sp_for(row, num_expected) {
    sp_str_t actual = trim_framebuffer_row(frame.cells + row * frame.cols, frame.cols);
    SP_EXPECT_STR_EQ_CSTR(actual, expected[row]);
  }
}

typedef struct {
  sp_prompt_widget_t widget;
  u32 ticks;
  sp_prompt_event_t terminal;
  sp_prompt_expect_t expect;
} sp_prompt_tick_case_t;

static void sp_prompt_run_tick_case(s32* utest_result, struct prompt* fixture, sp_prompt_tick_case_t t) {
  sp_prompt_step(&fixture->ctx, t.widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });

  sp_for(it, t.ticks) {
    sp_prompt_tick(&fixture->ctx, t.widget);
  }

  if (t.terminal.kind != SP_PROMPT_EVENT_NONE) {
    sp_prompt_step(&fixture->ctx, t.widget, t.terminal);
  }

  EXPECT_EQ(fixture->ctx.state, t.expect.state);
  expect_framebuffer_lines(utest_result, fixture, (const c8**)t.expect.lines);
}

static void render_spinner_at_frame(struct prompt* fixture, sp_prompt_spinner_t config, u32 frame) {
  sp_prompt_widget_t widget = sp_prompt_spinner_widget(&fixture->ctx, config);
  sp_prompt_step(&fixture->ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });
  sp_for(it, frame) {
    SP_UNUSED(it);
    sp_prompt_tick(&fixture->ctx, widget);
  }
}

UTEST_F(prompt, spinner_renders_initial_frame) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 0x2807, 0x280B, 0x2819 },
    .color = { .ansi = 36 },
  };

  render_spinner_at_frame(utest_fixture, spinner, 0);

  const c8* expected[32] = { "⠇  loading", SP_NULLPTR };
  expect_framebuffer_lines(utest_result, utest_fixture, expected);
}

UTEST_F(prompt, spinner_advances_frame_per_tick) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 0x2807, 0x280B, 0x2819 },
    .color = { .ansi = 36 },
  };

  render_spinner_at_frame(utest_fixture, spinner, 1);

  const c8* expected[32] = { "⠋  loading", SP_NULLPTR };
  expect_framebuffer_lines(utest_result, utest_fixture, expected);
}

UTEST_F(prompt, spinner_wraps_after_full_cycle) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 0x2807, 0x280B },
    .color = { .ansi = 36 },
  };

  render_spinner_at_frame(utest_fixture, spinner, 2);

  const c8* expected[32] = { "⠇  loading", SP_NULLPTR };
  expect_framebuffer_lines(utest_result, utest_fixture, expected);
}

static void spin_color_red(sp_prompt_ctx_t* ctx, u32 frame_index, sp_prompt_style_t* style) {
  SP_UNUSED(ctx);
  SP_UNUSED(frame_index);
  *style = (sp_prompt_style_t) { .tag = SP_PROMPT_STYLE_ANSI, .ansi = 31 };
}

UTEST_F(prompt, spinner_color_fn_overrides_fixed) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 'A' },
    .color = {
      .ansi = 36,
      .fn = spin_color_red,
    },
  };

  render_spinner_at_frame(utest_fixture, spinner, 0);

  sp_prompt_cell_t cell = ut.ctx.framebuffer[0];
  EXPECT_EQ(cell.codepoint, (u32)'A');
  EXPECT_EQ(cell.style.tag, SP_PROMPT_STYLE_ANSI);
  EXPECT_EQ(cell.style.ansi, 31);
}

UTEST_F(prompt, spinner_submits_on_enter) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 0x2807 },
  };

  sp_prompt_run_tick_case(utest_result, utest_fixture, (sp_prompt_tick_case_t) {
    .widget = sp_prompt_spinner_widget(&ut.ctx, spinner),
    .terminal = { .kind = SP_PROMPT_EVENT_ENTER },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
    },
  });
}

UTEST_F(prompt, spinner_cancels_on_ctrl_c) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 0x2807 },
  };

  sp_prompt_run_tick_case(utest_result, utest_fixture, (sp_prompt_tick_case_t) {
    .widget = sp_prompt_spinner_widget(&ut.ctx, spinner),
    .terminal = { .kind = SP_PROMPT_EVENT_CTRL_C },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
    },
  });
}

UTEST_F(prompt, spinner_does_not_update_after_submit) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 'A', 'B' },
  };

  sp_prompt_widget_t widget = sp_prompt_spinner_widget(&ut.ctx, spinner);
  sp_prompt_spinner_widget_t* w = (sp_prompt_spinner_widget_t*)widget.user_data;
  sp_prompt_step(&ut.ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });
  sp_prompt_step(&ut.ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_ENTER });
  EXPECT_EQ(ut.ctx.state, SP_PROMPT_STATE_SUBMIT);

  u32 frame_at_submit = w->frame_index;
  sp_prompt_tick(&ut.ctx, widget);
  EXPECT_EQ(w->frame_index, frame_at_submit);
}

UTEST_F(prompt, spinner_widget_assigns_default_fps) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
  };
  sp_prompt_widget_t widget = sp_prompt_spinner_widget(&ut.ctx, spinner);

  EXPECT_EQ(widget.fps, 12u);
}

static void render_kr_at_frame(struct prompt* fixture, sp_prompt_knight_rider_t config, u32 frame) {
  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&fixture->ctx, config);
  sp_prompt_knight_rider_widget_t* state = (sp_prompt_knight_rider_widget_t*)widget.user_data;
  sp_prompt_step(&fixture->ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });
  state->elapsed_ns = (u64)frame * (u64)state->config.ex.interval * 1000000ULL;
  sp_prompt_step(&fixture->ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_NONE });
}

UTEST_F(prompt, knight_rider_renders_lead_at_origin) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  render_kr_at_frame(utest_fixture, kr, 0);

  sp_prompt_cell_t lead = ut.ctx.framebuffer[0];
  EXPECT_EQ(lead.codepoint, 0x2B25u);
  EXPECT_EQ(lead.style.tag, SP_PROMPT_STYLE_RGB);
  EXPECT_EQ(lead.style.rgb.r, 0xFFu);
  EXPECT_EQ(lead.style.rgb.g, 0x00u);
  EXPECT_EQ(lead.style.rgb.b, 0x00u);
}

UTEST_F(prompt, knight_rider_lead_position_advances_with_elapsed_time) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  render_kr_at_frame(utest_fixture, kr, 3);

  EXPECT_EQ(ut.ctx.framebuffer[3].codepoint, 0x2B25u);
  EXPECT_EQ(ut.ctx.framebuffer[2].codepoint, 0x25C6u);
  EXPECT_EQ(ut.ctx.framebuffer[1].codepoint, 0x2B29u);
}

UTEST_F(prompt, knight_rider_holds_lead_at_right_edge) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  render_kr_at_frame(utest_fixture, kr, 6);

  EXPECT_EQ(ut.ctx.framebuffer[5].codepoint, 0x2B25u);
}

UTEST_F(prompt, knight_rider_moves_backward_after_hold_end) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  render_kr_at_frame(utest_fixture, kr, 17);

  EXPECT_EQ(ut.ctx.framebuffer[2].codepoint, 0x2B25u);
}

UTEST_F(prompt, knight_rider_wraps_after_total_elapsed) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&ut.ctx, kr);
  sp_prompt_knight_rider_t resolved = ((sp_prompt_knight_rider_widget_t*)widget.user_data)->config;
  u32 num_frames = (resolved.width + resolved.ex.hold_end + (resolved.width - 1) + resolved.ex.hold_start);
  render_kr_at_frame(utest_fixture, kr, num_frames);

  EXPECT_EQ(ut.ctx.framebuffer[0].codepoint, 0x2B25u);
}

UTEST_F(prompt, knight_rider_holds_frame_within_an_interval) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
  };

  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&ut.ctx, kr);
  sp_prompt_step(&ut.ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });
  ((sp_prompt_knight_rider_widget_t*)widget.user_data)->elapsed_ns = (u64)SP_PROMPT_KR_INTERVAL_MS * 1000000ULL - 1ULL;
  sp_prompt_step(&ut.ctx, widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_NONE });

  EXPECT_EQ(ut.ctx.framebuffer[0].codepoint, 0x2B25u);
}

UTEST_F(prompt, knight_rider_custom_color_overrides_default_palette) {
  sp_prompt_knight_rider_t kr = {
    .prompt = "scan",
    .color = { .r = 100, .g = 200, .b = 50 },
  };

  render_kr_at_frame(utest_fixture, kr, 0);

  sp_prompt_cell_t lead = ut.ctx.framebuffer[0];
  EXPECT_EQ(lead.style.rgb.r, 100u);
  EXPECT_EQ(lead.style.rgb.g, 200u);
  EXPECT_EQ(lead.style.rgb.b, 50u);
}

UTEST_F(prompt, knight_rider_widget_defaults_fps_to_15) {
  sp_prompt_knight_rider_t kr = SP_ZERO_INITIALIZE();
  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&ut.ctx, kr);
  EXPECT_EQ(widget.fps, 15u);
}

UTEST_F(prompt, confirm_outro_flow_renders_separator) {
  sp_prompt_confirm_t confirm = {
    .prompt = "Install dependencies?",
    .initial = false,
  };
  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&ut.ctx, confirm),
    .events = {
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .boolean = SP_PROMPT_TEST_TRISTATE_FALSE,
      .lines = {
        "◇  Install dependencies?",
        "│  ○ Yes / ● No",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_outro_widget(&ut.ctx, outro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "└  done",
      },
      .composited = {
        "◇  Install dependencies?",
        "│  ○ Yes / ● No",
        "│",
        "└  done",
      },
    },
  });
}

typedef struct {
  sp_prompt_event_kind_t kinds[64];
  u32 num_kinds;
  sp_prompt_event_data_t last_progress_data;
  u32 num_progress;
  sp_str_t last_status_value;
  u32 num_status;
  // Fire a lifecycle transition on EVENT_INIT, simulating a worker that resolves
  // the moment the widget starts. ACTIVE means "do nothing"; anything else fires.
  sp_prompt_state_t fire_on_init;
  // Optional handshake flag set on EVENT_INIT, to release a worker that's waiting until
  // the widget has started running (so complete()/abort() lands after on_init resets state).
  sp_atomic_s32_t* signal_on_init;
} probe_state_t;

static void probe_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  probe_state_t* s = (probe_state_t*)ctx->user_data;
  if (s->num_kinds < sp_carr_len(s->kinds)) {
    s->kinds[s->num_kinds++] = event.kind;
  }
  if (event.kind == SP_PROMPT_EVENT_PROGRESS) {
    s->last_progress_data = event.progress.data;
    s->num_progress++;
  }
  if (event.kind == SP_PROMPT_EVENT_STATUS) {
    s->last_status_value = event.status.value;
    s->num_status++;
  }
  if (event.kind == SP_PROMPT_EVENT_INIT && s->fire_on_init != SP_PROMPT_STATE_ACTIVE) {
    sp_prompt_set_state(ctx, s->fire_on_init);
  }
  if (event.kind == SP_PROMPT_EVENT_INIT && s->signal_on_init) {
    sp_atomic_s32_set(s->signal_on_init, 1);
  }
}

static void probe_update(sp_prompt_ctx_t* ctx) {
  SP_UNUSED(ctx);
}

static void probe_render(sp_prompt_ctx_t* ctx) {
  SP_UNUSED(ctx);
}

static sp_prompt_widget_t probe_widget(probe_state_t* state) {
  return (sp_prompt_widget_t) {
    .user_data = state,
    .on_event = probe_event,
    .on_update = probe_update,
    .render = probe_render,
    .fps = 60,
  };
}

static u32 probe_count_kind(probe_state_t* s, sp_prompt_event_kind_t kind) {
  u32 count = 0;
  sp_for(it, s->num_kinds) {
    if (s->kinds[it] == kind) {
      count++;
    }
  }
  return count;
}

static void drive_until_quit(sp_mem_t mem, sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  sp_app_t* app = sp_app_new(mem, sp_prompt_app(ctx, widget));
  sp_for(it, 64) {
    SP_UNUSED(it);
    if (sp_app_tick(app) != SP_APP_CONTINUE) {
      break;
    }
  }
  sp_app_destroy(app);
}

UTEST_F(prompt, state_initially_active) {
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_ACTIVE);
  EXPECT_FALSE(sp_prompt_is_aborted(&ut.ctx));
}

UTEST_F(prompt, complete_transitions_state_to_submit) {
  sp_prompt_complete(&ut.ctx);
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
  EXPECT_TRUE(sp_prompt_is_aborted(&ut.ctx));
}

UTEST_F(prompt, abort_transitions_state_to_cancel) {
  sp_prompt_abort(&ut.ctx);
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_CANCEL);
  EXPECT_TRUE(sp_prompt_is_aborted(&ut.ctx));
}

UTEST_F(prompt, lifecycle_is_one_shot_first_writer_wins) {
  // CAS-from-ACTIVE: once the lifecycle has left ACTIVE, no later transition can override it.
  sp_prompt_complete(&ut.ctx);
  sp_prompt_abort(&ut.ctx);
  sp_prompt_complete(&ut.ctx);
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
}

UTEST_F(prompt, complete_drives_widget_to_submit) {
  probe_state_t state = { .fire_on_init = SP_PROMPT_STATE_SUBMIT };
  drive_until_quit(ut.mem, &ut.ctx, probe_widget(&state));
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
  EXPECT_TRUE(sp_prompt_submitted(&ut.ctx));
}

UTEST_F(prompt, abort_drives_widget_to_cancel) {
  probe_state_t state = { .fire_on_init = SP_PROMPT_STATE_CANCEL };
  drive_until_quit(ut.mem, &ut.ctx, probe_widget(&state));
  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_CANCEL);
  EXPECT_TRUE(sp_prompt_cancelled(&ut.ctx));
}

UTEST_F(prompt, prompt_end_frees_channel) {
  sp_prompt_ctx_t* ctx = sp_alloc_type(sp_prompt_ctx_t);
  sp_prompt_ctx_init(ctx, 80, 20);
  sp_prompt_send_progress_f32(ctx, 0.5f);
  sp_prompt_send_status(ctx, "halfway");
  sp_prompt_complete(ctx);
  sp_prompt_end(ctx);
}

typedef struct {
  sp_prompt_ctx_t* ctx;
  sp_atomic_s32_t ready;
} thread_signal_data_t;

static s32 thread_signal_fn(void* userdata) {
  thread_signal_data_t* d = (thread_signal_data_t*)userdata;
  while (sp_atomic_s32_get(&d->ready) == 0) {
    sp_spin_pause();
  }
  sp_prompt_complete(d->ctx);
  return 0;
}

UTEST_F(prompt, complete_from_worker_thread_eventually_drives_submit) {
  SKIP_ON_FREESTANDING();
  SKIP_ON_WASM();
  // Worker waits for the widget to fire EVENT_INIT, then calls complete() to drive
  // the lifecycle to SUBMIT. The wake pipe unblocks the on_poll loop.
  thread_signal_data_t d = { .ctx = &ut.ctx };
  probe_state_t state = { .signal_on_init = &d.ready };

  sp_thread_t worker = SP_ZERO_INITIALIZE();
  sp_thread_init(&worker, thread_signal_fn, &d);

  drive_until_quit(ut.mem, &ut.ctx, probe_widget(&state));
  sp_thread_join(&worker);

  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
  EXPECT_TRUE(sp_prompt_submitted(&ut.ctx));
}

UTEST_F(prompt, progress_initially_clean) {
  EXPECT_FALSE(ut.ctx.progress.dirty);
}

UTEST_F(prompt, send_progress_f32_marks_dirty_and_stores_value) {
  sp_prompt_send_progress_f32(&ut.ctx, 0.5f);
  EXPECT_TRUE(ut.ctx.progress.dirty);
  EXPECT_EQ(ut.ctx.progress.value.f, 0.5);
}

UTEST_F(prompt, send_progress_latest_wins) {
  sp_for(it, 100) {
    sp_prompt_send_progress_f32(&ut.ctx, (f32)it / 100.f);
  }
  EXPECT_TRUE(ut.ctx.progress.dirty);
  EXPECT_EQ(ut.ctx.progress.value.f, (f64)((f32)99 / 100.f));
}

UTEST_F(prompt, send_progress_typed_helpers_round_trip) {
  sp_prompt_send_progress_f64(&ut.ctx, 3.14159);
  EXPECT_EQ(ut.ctx.progress.value.f, 3.14159);

  sp_prompt_send_progress_u32(&ut.ctx, 0xDEADBEEF);
  EXPECT_EQ(ut.ctx.progress.value.u, 0xDEADBEEFu);

  sp_prompt_send_progress_u64(&ut.ctx, 0x1122334455667788ull);
  EXPECT_EQ(ut.ctx.progress.value.u, 0x1122334455667788ull);

  sp_prompt_send_progress_s32(&ut.ctx, -7);
  EXPECT_EQ(ut.ctx.progress.value.i, -7);

  sp_prompt_send_progress_s64(&ut.ctx, -1234567890123ll);
  EXPECT_EQ(ut.ctx.progress.value.i, -1234567890123ll);

  s32 sentinel = 0;
  sp_prompt_send_progress_ptr(&ut.ctx, &sentinel);
  EXPECT_EQ(ut.ctx.progress.value.ptr, (void*)&sentinel);

  sp_prompt_send_progress_bool(&ut.ctx, true);
  EXPECT_TRUE(ut.ctx.progress.value.b);
}

UTEST_F(prompt, progress_event_delivered_with_data) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_progress_f32(&ut.ctx, 0.42f);

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_progress, 1u);
  EXPECT_EQ(state.last_progress_data.f, (f64)0.42f);
}

UTEST_F(prompt, progress_drains_clear_dirty_flag) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_progress_f32(&ut.ctx, 0.25f);

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_progress, 1u);
  EXPECT_FALSE(ut.ctx.progress.dirty);
}

UTEST_F(prompt, progress_coalesces_between_drains) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_for(it, 50) {
    sp_prompt_send_progress_f32(&ut.ctx, (f32)it / 50.f);
  }

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_progress, 1u);
  EXPECT_EQ(state.last_progress_data.f, (f64)((f32)49 / 50.f));
}

UTEST_F(prompt, progress_redelivered_after_new_send) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_progress_f32(&ut.ctx, 0.1f);

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);
  EXPECT_EQ(state.num_progress, 1u);

  sp_prompt_send_progress_f32(&ut.ctx, 0.9f);
  sp_app_tick(app);

  EXPECT_EQ(state.num_progress, 2u);
  EXPECT_EQ(state.last_progress_data.f, (f64)0.9f);
}

UTEST_F(prompt, progress_after_complete_is_dropped) {
  // Fire complete on EVENT_INIT; any send_progress queued before the widget started runs is
  // delivered on the same poll tick, but lifecycle transitions wake the loop and exit before
  // any subsequent progress events are dispatched.
  probe_state_t state = { .fire_on_init = SP_PROMPT_STATE_SUBMIT };
  sp_prompt_send_progress_f32(&ut.ctx, 0.5f);

  drive_until_quit(ut.mem, &ut.ctx, probe_widget(&state));

  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
}

UTEST_F(prompt, progress_widget_renders_active_two_rows_plus_rail) {
  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_progress_widget(&ut.ctx, (sp_prompt_progress_t) {
      .prompt = "Downloading",
      .width = 8,
    }),
    .events = {
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Downloading",
        "│  ──────── 0%",
        "└",
      },
    },
  });
}

UTEST_F(prompt, progress_widget_quarter_full_uses_partial_blocks) {
  sp_prompt_progress_widget_t* p = sp_alloc_type(sp_prompt_progress_widget_t);
  *p = SP_ZERO_STRUCT(sp_prompt_progress_widget_t);
  p->config.prompt = "Loading";
  p->config.width = 8;
  p->value = 0.25f;

  sp_prompt_widget_t widget = {
    .user_data = p,
    .on_event = sp_prompt_progress_event,
    .render = sp_prompt_progress_render,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = widget,
    .events = {
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Loading",
        "│  ██────── 25%",
        "└",
      },
    },
  });
}

UTEST_F(prompt, progress_widget_clamps_out_of_range_inputs) {
  probe_state_t observer = SP_ZERO_INITIALIZE();
  SP_UNUSED(observer);

  sp_prompt_progress_widget_t* p = sp_alloc_type(sp_prompt_progress_widget_t);
  *p = SP_ZERO_STRUCT(sp_prompt_progress_widget_t);
  p->config.prompt = "clamp";
  p->config.width = 4;

  sp_prompt_event_t e_neg = { .kind = SP_PROMPT_EVENT_PROGRESS, .progress = { .data = { .f = -0.5 } } };
  ut.ctx.user_data = p;
  sp_prompt_progress_event(&ut.ctx, e_neg);
  EXPECT_EQ(p->value, 0.f);

  sp_prompt_event_t e_big = { .kind = SP_PROMPT_EVENT_PROGRESS, .progress = { .data = { .f = 5.0 } } };
  sp_prompt_progress_event(&ut.ctx, e_big);
  EXPECT_EQ(p->value, 1.f);
}

UTEST_F(prompt, status_initially_clean) {
  EXPECT_FALSE(ut.ctx.status.dirty);
}

UTEST_F(prompt, send_status_marks_dirty_and_copies_into_arena) {
  c8 buf[] = "transient";
  sp_prompt_send_status_str(&ut.ctx, sp_str(buf, 9));
  // mutate the source after send to prove the channel owns its copy
  buf[0] = 'X';
  EXPECT_TRUE(ut.ctx.status.dirty);
  ASSERT_EQ(ut.ctx.status.value.len, 9u);
  EXPECT_EQ(ut.ctx.status.value.data[0], 't');
}

UTEST_F(prompt, send_status_cstr_round_trip) {
  sp_prompt_send_status(&ut.ctx, "hello world");
  EXPECT_TRUE(ut.ctx.status.dirty);
  ASSERT_EQ(ut.ctx.status.value.len, 11u);
  SP_EXPECT_STR_EQ_CSTR(ut.ctx.status.value, "hello world");
}

UTEST_F(prompt, send_status_latest_wins) {
  sp_prompt_send_status(&ut.ctx, "first");
  sp_prompt_send_status(&ut.ctx, "second");
  sp_prompt_send_status(&ut.ctx, "third");
  EXPECT_TRUE(ut.ctx.status.dirty);
  SP_EXPECT_STR_EQ_CSTR(ut.ctx.status.value, "third");
}

UTEST_F(prompt, status_event_delivered_with_value) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_status(&ut.ctx, "loading");

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_status, 1u);
  SP_EXPECT_STR_EQ_CSTR(state.last_status_value, "loading");
}

UTEST_F(prompt, status_drains_clear_dirty_flag) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_status(&ut.ctx, "x");

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_status, 1u);
  EXPECT_FALSE(ut.ctx.status.dirty);
}

UTEST_F(prompt, progress_and_status_both_delivered_in_same_tick) {
  probe_state_t state = SP_ZERO_INITIALIZE();
  sp_prompt_send_progress_f32(&ut.ctx, 0.4f);
  sp_prompt_send_status(&ut.ctx, "step 4 of 10");

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_progress, 1u);
  EXPECT_EQ(state.num_status, 1u);
  EXPECT_EQ(state.last_progress_data.f, (f64)0.4f);
  SP_EXPECT_STR_EQ_CSTR(state.last_status_value, "step 4 of 10");
}

UTEST_F(prompt, status_after_complete_does_not_block_submit) {
  probe_state_t state = { .fire_on_init = SP_PROMPT_STATE_SUBMIT };
  sp_prompt_send_status(&ut.ctx, "almost there");

  drive_until_quit(ut.mem, &ut.ctx, probe_widget(&state));

  EXPECT_EQ(sp_atomic_s32_get(&ut.ctx.state), SP_PROMPT_STATE_SUBMIT);
}

UTEST_F(prompt, progress_widget_renders_status_below_bar) {
  sp_prompt_progress_widget_t* p = sp_alloc_type(sp_prompt_progress_widget_t);
  *p = SP_ZERO_STRUCT(sp_prompt_progress_widget_t);
  p->config.prompt = "Installing";
  p->config.width = 8;
  p->value = 0.5f;
  p->status = sp_str_view("Linking");

  sp_prompt_widget_t widget = {
    .user_data = p,
    .on_event = sp_prompt_progress_event,
    .render = sp_prompt_progress_render,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = widget,
    .events = {
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Installing",
        "│  ████──── 50%",
        "│  Linking",
        "└",
      },
    },
  });
}

UTEST_F(prompt, progress_widget_omits_status_when_empty) {
  sp_prompt_progress_widget_t* p = sp_alloc_type(sp_prompt_progress_widget_t);
  *p = SP_ZERO_STRUCT(sp_prompt_progress_widget_t);
  p->config.prompt = "Loading";
  p->config.width = 4;
  p->value = 0.5f;

  sp_prompt_widget_t widget = {
    .user_data = p,
    .on_event = sp_prompt_progress_event,
    .render = sp_prompt_progress_render,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = widget,
    .events = {
      { .kind = SP_PROMPT_EVENT_CTRL_C },
    },
    .expect = {
      .state = SP_PROMPT_STATE_CANCEL,
      .lines = {
        "■  Loading",
        "│  ██── 50%",
        "└",
      },
    },
  });
}

typedef struct {
  sp_prompt_ctx_t* ctx;
  sp_atomic_s32_t ready;
} status_thread_data_t;

static s32 status_thread_fn(void* userdata) {
  status_thread_data_t* d = (status_thread_data_t*)userdata;
  while (sp_atomic_s32_get(&d->ready) == 0) {
    sp_spin_pause();
  }
  sp_prompt_send_status(d->ctx, "from worker");
  return 0;
}

UTEST_F(prompt, send_status_from_thread_eventually_delivered) {
  SKIP_ON_FREESTANDING();
  SKIP_ON_WASM();
  probe_state_t state = SP_ZERO_INITIALIZE();
  status_thread_data_t d = { .ctx = &ut.ctx };

  sp_thread_t worker = SP_ZERO_INITIALIZE();
  sp_thread_init(&worker, status_thread_fn, &d);

  sp_atomic_s32_set(&d.ready, 1);
  sp_thread_join(&worker);

  sp_app_t* app = ut.app = sp_app_new(ut.mem, sp_prompt_app(&ut.ctx, probe_widget(&state)));
  sp_app_tick(app);
  sp_app_tick(app);

  EXPECT_EQ(state.num_status, 1u);
  SP_EXPECT_STR_EQ_CSTR(state.last_status_value, "from worker");
}

UTEST_F(prompt, idle_updates_do_not_grow_persistent_arena) {
  sp_prompt_spinner_t spinner = {
    .prompt = "loading",
    .frames = { 'A', 'B', 'C' },
  };

  sp_prompt_widget_t widget = sp_prompt_spinner_widget(&ut.ctx, spinner);
  ut.ctx.widget = widget;
  ut.ctx.user_data = widget.user_data;

  sp_app_t app = SP_ZERO_INITIALIZE();
  app.user_data = &ut.ctx;

  sp_prompt_app_on_init(&app);
  u64 baseline = sp_mem_arena_bytes_used(ut.ctx.arena);

  sp_for(it, 1024) {
    SP_UNUSED(it);
    sp_prompt_app_on_update(&app);
  }

  EXPECT_EQ(sp_mem_arena_bytes_used(ut.ctx.arena), baseline);
}
