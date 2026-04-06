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
};

UTEST_F_SETUP(prompt) {
  utest_fixture->ctx = SP_ZERO_STRUCT(sp_prompt_ctx_t);
  sp_prompt_ctx_init(&utest_fixture->ctx, 80, 20);
  sp_io_writer_from_dyn_mem(&utest_fixture->writer);
  utest_fixture->ctx.writer = &utest_fixture->writer;
}

UTEST_F_TEARDOWN(prompt) {
  sp_io_writer_close(&utest_fixture->writer);
}

static u32 count_expected_lines(const c8* lines[32]) {
  u32 count = 0;
  while (count < 32 && lines[count] != SP_NULLPTR) {
    count++;
  }
  return count;
}

static sp_str_t trim_framebuffer_row(sp_prompt_cell_t* row, u32 cols) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_for(col, cols) {
    sp_str_builder_append_utf8(&builder, row[col].codepoint);
  }

  sp_str_t str = sp_str_builder_to_str(&builder);
  while (!sp_str_empty(str) && str.data[str.len - 1] == ' ') {
    str.len--;
  }
  return str;
}

static sp_prompt_frame_t sp_prompt_last_frame(sp_prompt_ctx_t* ctx) {
  return ctx->frames[sp_da_size(ctx->frames) - 1];
}

static sp_prompt_cell_t sp_prompt_frame_cell(sp_prompt_frame_t frame, u32 row, u32 col) {
  return frame.cells[row * (u32)frame.cols + col];
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
    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_for(col, 256) {
      sp_str_builder_append_utf8(&builder, vt.cells[row][col]);
    }

    sp_str_t line = sp_str_builder_to_str(&builder);
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
      sp_str_t actual = trim_framebuffer_row(frame.cells + row * (u32)frame.cols, (u32)frame.cols);
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
    .widget = sp_prompt_intro_widget(&intro),
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
    .widget = sp_prompt_outro_widget(&outro),
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
    .widget = sp_prompt_note_widget(&note),
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
    .widget = sp_prompt_note_widget(&note),
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
    .widget = sp_prompt_intro_widget(&intro),
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
    .widget = sp_prompt_message_widget(&message),
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
    .widget = sp_prompt_message_widget(&message),
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
    .widget = sp_prompt_text_widget(&text),
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
    .widget = sp_prompt_text_widget(&text),
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
    .prompt = sp_str_view("Proceed?"),
    .value = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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
    .prompt = sp_str_view("Proceed?"),
    .value = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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
    .prompt = sp_str_view("Proceed?"),
    .value = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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
    .prompt = sp_str_view("Proceed?"),
    .value = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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
    .prompt = sp_str_view("Proceed?"),
    .value = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm_arrow),
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
    .prompt = sp_str_view("Proceed?"),
    .value = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm_vim),
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

UTEST_F(prompt, confirm_active_rail_is_blue) {
  sp_prompt_confirm_t confirm = {
    .prompt = sp_str_view("Proceed?"),
    .value = false,
  };

  sp_prompt_step(
    &utest_fixture->ctx,
    sp_prompt_confirm_widget(&confirm),
    (sp_prompt_event_t) {
      .kind = SP_PROMPT_EVENT_INIT,
    }
  );

  sp_prompt_frame_t frame = {
    .cols = utest_fixture->ctx.cols,
    .rows = utest_fixture->ctx.cursor_row,
    .cells = utest_fixture->ctx.framebuffer,
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
    .prompt = sp_str_view("Proceed?"),
    .value = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);

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
    .prompt = sp_str_view("Proceed?"),
    .value = false,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);

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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);

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
    .max_items = 5,
  };

  sp_prompt_step(
    &utest_fixture->ctx,
    sp_prompt_select_widget(&select),
    (sp_prompt_event_t) {
      .kind = SP_PROMPT_EVENT_INIT,
    }
  );

  sp_prompt_frame_t frame = {
    .cols = utest_fixture->ctx.cols,
    .rows = utest_fixture->ctx.cursor_row,
    .cells = utest_fixture->ctx.framebuffer,
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 4,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .widget = sp_prompt_select_widget(&select),
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
    .max_items = 5,
  };

  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_select_widget(&select),
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
    .widget = sp_prompt_outro_widget(&outro),
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_DOWN },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "TypeScript, JavaScript",
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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
    .max_items = 3,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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
    .max_items = 5,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);

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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
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
    .max_items = 4,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'j' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
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

  ASSERT_TRUE(!sp_da_empty(utest_fixture->ctx.frames));
  sp_prompt_frame_t frame = sp_prompt_last_frame(&utest_fixture->ctx);
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
    .max_items = 5,
    .filter = true,
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "JavaScript",
      .lines = {
        "◇  Pick tools a",
        "│  JavaScript",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_multiselect_widget(&multiselect),
    .events = {
      { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = ' ' } },
      { .kind = SP_PROMPT_EVENT_ENTER },
    },
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .str = "TypeScript, JavaScript",
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
    .widget = sp_prompt_password_widget(&password),
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
    .widget = sp_prompt_password_widget(&password),
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
    .widget = sp_prompt_intro_widget(&intro),
    .expect = {
      .state = SP_PROMPT_STATE_SUBMIT,
      .lines = {
        "┌  hello",
      },
    },
  });

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_text_widget(&text),
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
    .widget = sp_prompt_outro_widget(&outro),
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

UTEST_F(prompt, confirm_outro_flow_renders_separator) {
  sp_prompt_confirm_t confirm = {
    .prompt = sp_str_view("Install dependencies?"),
    .value = false,
  };
  sp_prompt_outro_t outro = {
    .text = sp_str_view("done"),
  };

  sp_prompt_run_case(utest_result, utest_fixture, (sp_prompt_case_t) {
    .widget = sp_prompt_confirm_widget(&confirm),
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
    .widget = sp_prompt_outro_widget(&outro),
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
