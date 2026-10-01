#define SP_PROMPT_IMPLEMENTATION
#include "prompt.h"

#define VT_ROWS 128
#define VT_COLS 256

typedef struct {
  u32 row;
  u32 col;
  u32 max_row;
  u32 cells [VT_ROWS][VT_COLS];
} vt_t;

typedef struct {
  sp_prompt_ctx_t* ctx;
  prompt_act_t act;
} job_t;

static void vt_clear(vt_t* vt, u32 row, u32 col) {
  sp_for_range(it, col, VT_COLS) {
    vt->cells[row][it] = ' ';
  }
  sp_for_range(r, row + 1, VT_ROWS) {
    sp_for(c, VT_COLS) {
      vt->cells[r][c] = ' ';
    }
  }
}

static void vt_clear_eos(vt_t* vt) {
  vt_clear(vt, vt->row, vt->col);

  vt->max_row = 0;
  sp_for(row, VT_ROWS) {
    sp_for(col, VT_COLS) {
      if (vt->cells[row][col] != ' ') {
        vt->max_row = row;
        break;
      }
    }
  }
}

static void vt_put(vt_t* vt, u32 codepoint) {
  if (vt->row >= VT_ROWS || vt->col >= VT_COLS) {
    return;
  }

  vt->cells[vt->row][vt->col] = codepoint;
  vt->col++;
  if (vt->row > vt->max_row) {
    vt->max_row = vt->row;
  }
}

static u32 escape_end(sp_str_t bytes, u32 at) {
  if (at + 1 >= bytes.len || bytes.data[at + 1] != '[') {
    return at + 1;
  }

  sp_for_range(it, at + 2, bytes.len) {
    c8 c = bytes.data[it];
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
      return it + 1;
    }
  }
  return bytes.len;
}

static u32 vt_escape(vt_t* vt, sp_str_t bytes, u32 at) {
  u32 end = escape_end(bytes, at);
  switch (bytes.data[end - 1]) {
    case 'A': {
      if (vt->row > 0) {
        vt->row--;
      }
      break;
    }
    case 'J': {
      vt_clear_eos(vt);
      break;
    }
  }
  return end;
}

static sp_str_t trim_right(sp_str_t line) {
  while (!sp_str_empty(line) && line.data[line.len - 1] == ' ') {
    line.len--;
  }
  return line;
}

static sp_str_t encode_row(sp_mem_t mem, const u32* codepoints, u32 count) {
  sp_io_dyn_mem_writer_t builder = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &builder);
  sp_for(it, count) {
    c8 utf8 [4] = sp_zero;
    u8 len = sp_utf8_encode(codepoints[it], utf8);
    sp_io_write_str(&builder.base, sp_str(utf8, len), SP_NULLPTR);
  }
  return trim_right(sp_io_dyn_mem_writer_as_str(&builder));
}

static sp_da(sp_str_t) composite(sp_mem_t mem, sp_str_t bytes) {
  vt_t* vt = sp_alloc_type(mem, vt_t);
  vt->row = 0;
  vt->col = 0;
  vt->max_row = 0;
  vt_clear(vt, 0, 0);

  u32 at = 0;
  while (at < bytes.len) {
    u8 byte = (u8)bytes.data[at];

    if (byte == 0x1b) {
      at = vt_escape(vt, bytes, at);
      continue;
    }

    if (byte == '\r') {
      vt->col = 0;
      at++;
      continue;
    }

    if (byte == '\n') {
      if (vt->row + 1 < VT_ROWS) {
        vt->row++;
      }
      vt->col = 0;
      if (vt->row > vt->max_row) {
        vt->max_row = vt->row;
      }
      at++;
      continue;
    }

    c8 utf8 [4] = sp_zero;
    u32 count = sp_max(sp_utf8_num_bytes_from_byte(byte), 1);
    sp_for(it, count) {
      if (at + it < bytes.len) {
        utf8[it] = bytes.data[at + it];
      }
    }
    vt_put(vt, sp_utf8_decode(utf8));
    at += count;
  }

  sp_da(sp_str_t) lines = sp_da_new(mem, sp_str_t);
  sp_for(row, vt->max_row + 1) {
    sp_da_push(lines, encode_row(mem, vt->cells[row], VT_COLS));
  }
  return lines;
}

static sp_da(sp_str_t) frame_lines(sp_mem_t mem, sp_prompt_frame_t frame) {
  sp_da(sp_str_t) lines = sp_da_new(mem, sp_str_t);
  sp_for(row, frame.rows) {
    u32* codepoints = sp_alloc_n(mem, u32, frame.cols);
    sp_for(col, frame.cols) {
      codepoints[col] = frame.cells[row * frame.cols + col].codepoint;
    }
    sp_da_push(lines, encode_row(mem, codepoints, frame.cols));
  }
  return lines;
}

static u32 num_options(const sp_prompt_select_option_t* options) {
  u32 count = 0;
  while (count < PROMPT_MAX_OPTIONS && options[count].label) {
    count++;
  }
  return count;
}

static void probe_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  prompt_probe_t* probe = (prompt_probe_t*)sp_prompt_get_user_data(ctx);
  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      if (probe->fire != SP_PROMPT_STATE_ACTIVE) {
        sp_prompt_set_state(ctx, probe->fire);
      }
      break;
    }
    case SP_PROMPT_EVENT_PROGRESS: {
      probe->last_progress = event.progress.data;
      probe->progress++;
      break;
    }
    case SP_PROMPT_EVENT_STATUS: {
      probe->last_status = event.status.value;
      probe->status++;
      break;
    }
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_INPUT:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_ENTER:
    case SP_PROMPT_EVENT_TAB:
    case SP_PROMPT_EVENT_BACKSPACE:
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE:
    case SP_PROMPT_EVENT_ABORT: {
      break;
    }
  }
}

static void probe_update(sp_prompt_ctx_t* ctx) {
}

static void perform(sp_prompt_ctx_t* ctx, prompt_act_t act) {
  c8 text [PROMPT_MAX_TEXT] = sp_zero;

  switch (act.kind) {
    case PROMPT_ACT_PROGRESS: {
      sp_prompt_send_progress_f32(ctx, act.value);
      break;
    }
    case PROMPT_ACT_STATUS: {
      sp_cstr_copy_to(act.text, text, sizeof(text));
      sp_prompt_send_status(ctx, text);
      break;
    }
    case PROMPT_ACT_LOG: {
      sp_cstr_copy_to(act.text, text, sizeof(text));
      sp_prompt_log(ctx, text);
      break;
    }
    case PROMPT_ACT_COMPLETE: {
      sp_prompt_complete(ctx);
      break;
    }
    case PROMPT_ACT_ABORT: {
      sp_prompt_abort(ctx);
      break;
    }
    case PROMPT_ACT_NONE:
    case PROMPT_ACT_TICK:
    case PROMPT_ACT_RUN: {
      break;
    }
  }

  sp_mem_zero(text, sizeof(text));
}

static s32 perform_on_thread(void* userdata) {
  job_t* job = (job_t*)userdata;
  perform(job->ctx, job->act);
  return 0;
}

static sp_err_t perform_threaded(sp_test_t* t, sp_prompt_ctx_t* ctx, prompt_act_t act) {
  sp_test_skip_on_freestanding();
  sp_test_skip_on_wasm();

  job_t job = { .ctx = ctx, .act = act };
  sp_thread_t worker = sp_zero;
  sp_thread_init(&worker, perform_on_thread, &job);
  sp_thread_join(&worker);
  return SP_OK;
}

static sp_app_t* probe_app(sp_test_t* t) {
  prompt_fixture_t* f = prompt_fixture(t);
  if (!f->app) {
    f->app = sp_app_new(sp_test_arena(t), sp_prompt_app(&f->ctx, (sp_prompt_widget_t) {
      .user_data = &f->probe,
      .on_event = probe_event,
      .on_update = probe_update,
      .fps = 60,
    }));
  }
  return f->app;
}

static void expect_frame(sp_test_t* t, sp_prompt_frame_t frame, const prompt_expect_t* expect) {
  if (expect->lines[0]) {
    sp_da(sp_str_t) lines = frame_lines(sp_test_arena(t), frame);
    sp_expect_strs_eq(t, lines, sp_da_size(lines), expect->lines);
  }

  sp_carr_for(expect->cells, it) {
    prompt_cell_t want = expect->cells[it];
    if (!want.codepoint) break;

    sp_test_kv(t, "cell", sp_test_format(t, "{},{}", sp_fmt_uint(want.row), sp_fmt_uint(want.col)));
    if (want.row >= frame.rows || want.col >= frame.cols) {
      sp_test_fail(t, "cell is outside the {}x{} frame", sp_fmt_uint(frame.cols), sp_fmt_uint(frame.rows));
      continue;
    }

    sp_prompt_cell_t cell = frame.cells[want.row * frame.cols + want.col];
    sp_expect_eq(t, cell.codepoint, want.codepoint);
    sp_expect_eq(t, (u32)cell.style.tag, (u32)want.style.tag);
    switch (want.style.tag) {
      case SP_PROMPT_STYLE_NONE: {
        break;
      }
      case SP_PROMPT_STYLE_ANSI: {
        sp_expect_eq(t, cell.style.ansi, want.style.ansi);
        break;
      }
      case SP_PROMPT_STYLE_RGB: {
        sp_expect_eq(t, cell.style.rgb.r, want.style.rgb.r);
        sp_expect_eq(t, cell.style.rgb.g, want.style.rgb.g);
        sp_expect_eq(t, cell.style.rgb.b, want.style.rgb.b);
        break;
      }
    }
  }
  sp_test_kv_clear(t, "cell");
}

static void expect_result(sp_test_t* t, const prompt_expect_t* expect) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;

  sp_expect_eq(t, sp_atomic_s32_load(&ctx->state, SP_ATOMIC_SEQ_CST), (s32)expect->state);
  sp_expect_eq(t, sp_prompt_submitted(ctx), expect->state == SP_PROMPT_STATE_SUBMIT);
  sp_expect_eq(t, sp_prompt_cancelled(ctx), expect->state == SP_PROMPT_STATE_CANCEL);

  if (expect->str) {
    sp_expect_str_eq_c(t, sp_cstr_as_str(sp_prompt_get_str(ctx)), expect->str);
  }

  switch (expect->boolean) {
    case PROMPT_BOOL_NONE: {
      break;
    }
    case PROMPT_BOOL_FALSE:
    case PROMPT_BOOL_TRUE: {
      sp_expect_eq(t, sp_prompt_get_bool(ctx), expect->boolean == PROMPT_BOOL_TRUE);
      break;
    }
  }

  if (expect->composited[0]) {
    sp_da(sp_str_t) lines = composite(sp_test_arena(t), prompt_written(t));
    sp_expect_strs_eq(t, lines, sp_da_size(lines), expect->composited);
  }
}

sp_err_t prompt_setup(sp_test_t* t) {
  prompt_fixture_t* f = sp_alloc_type(sp_test_arena(t), prompt_fixture_t);
  sp_prompt_ctx_init(&f->ctx, sp_test_mem(t), PROMPT_COLS, PROMPT_ROWS);
  sp_io_dyn_mem_writer_init(sp_test_arena(t), &f->writer);
  f->ctx.tty = (sp_tty_t) {
    .io = &f->writer.base,
    .color = SP_TTY_COLOR_ANSI,
  };
  f->probe = sp_zero_s(prompt_probe_t);
  f->app = SP_NULLPTR;
  sp_test_set_state(t, f);
  return SP_OK;
}

void prompt_teardown(sp_test_t* t) {
  prompt_fixture_t* f = prompt_fixture(t);
  sp_app_destroy(f->app);
  sp_prompt_end(&f->ctx);
  sp_io_dyn_mem_writer_close(&f->writer);
}

prompt_fixture_t* prompt_fixture(sp_test_t* t) {
  return (prompt_fixture_t*)sp_test_state(t);
}

sp_str_t prompt_written(sp_test_t* t) {
  return sp_io_dyn_mem_writer_as_str(&prompt_fixture(t)->writer);
}

sp_str_t prompt_text(sp_test_t* t) {
  sp_str_t bytes = prompt_written(t);
  sp_io_dyn_mem_writer_t text = sp_zero;
  sp_io_dyn_mem_writer_init(sp_test_arena(t), &text);

  u32 at = 0;
  while (at < bytes.len) {
    if (bytes.data[at] == 0x1b) {
      at = escape_end(bytes, at);
      continue;
    }
    sp_io_write_c8(&text.base, bytes.data[at]);
    at++;
  }
  return sp_io_dyn_mem_writer_as_str(&text);
}

sp_prompt_widget_t prompt_widget(sp_test_t* t, prompt_widget_desc_t* desc) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;

  switch (desc->kind) {
    case PROMPT_WIDGET_NONE: {
      break;
    }
    case PROMPT_WIDGET_INTRO: {
      return sp_prompt_intro_widget(ctx, (sp_prompt_intro_t) {
        .text = sp_cstr_as_str(desc->intro.text),
      });
    }
    case PROMPT_WIDGET_OUTRO: {
      return sp_prompt_outro_widget(ctx, (sp_prompt_outro_t) {
        .text = sp_cstr_as_str(desc->outro.text),
      });
    }
    case PROMPT_WIDGET_MESSAGE: {
      return sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
        .text = sp_cstr_as_str(desc->message.text),
        .symbol = desc->message.symbol,
        .ansi = desc->message.ansi,
      });
    }
    case PROMPT_WIDGET_TEXT: {
      return sp_prompt_text_widget(ctx, (sp_prompt_text_t) {
        .prompt = sp_cstr_as_str(desc->text.prompt),
        .prefill = sp_cstr_as_str(desc->text.prefill),
      });
    }
    case PROMPT_WIDGET_PASSWORD: {
      return sp_prompt_password_widget(ctx, (sp_prompt_password_t) {
        .prompt = sp_cstr_as_str(desc->password.prompt),
        .prefill = sp_cstr_as_str(desc->password.prefill),
        .mask = true,
      });
    }
    case PROMPT_WIDGET_CONFIRM: {
      return sp_prompt_confirm_widget(ctx, desc->confirm);
    }
    case PROMPT_WIDGET_SELECT: {
      return sp_prompt_select_widget(ctx, (sp_prompt_select_t) {
        .prompt = desc->select.prompt,
        .options = desc->select.options,
        .num_options = num_options(desc->select.options),
        .max_visible = desc->select.max_visible,
        .filter = desc->select.filter,
      });
    }
    case PROMPT_WIDGET_MULTISELECT: {
      return sp_prompt_multiselect_widget(ctx, (sp_prompt_multiselect_t) {
        .prompt = desc->multiselect.prompt,
        .options = desc->multiselect.options,
        .num_options = num_options(desc->multiselect.options),
        .max_visible = desc->multiselect.max_visible,
        .filter = desc->multiselect.filter,
      });
    }
  }

  return sp_zero_s(sp_prompt_widget_t);
}

sp_err_t prompt_run(sp_test_t* t, sp_prompt_widget_t widget, sp_prompt_event_t* events, const prompt_expect_t* expect) {
  prompt_fixture_t* f = prompt_fixture(t);
  sp_prompt_prime_events(&f->ctx, events);
  bool submitted = sp_prompt_run(&f->ctx, widget);
  sp_expect_eq(t, submitted, expect->state == SP_PROMPT_STATE_SUBMIT);
  return prompt_expect_last(t, expect);
}

sp_err_t prompt_run_case(sp_test_t* t, prompt_case_t* it) {
  return prompt_run(t, prompt_widget(t, &it->widget), it->events, &it->expect);
}

void prompt_start(sp_test_t* t, sp_prompt_widget_t widget) {
  prompt_fixture_t* f = prompt_fixture(t);
  f->app = sp_app_new(sp_test_arena(t), sp_prompt_app(&f->ctx, widget));
  f->app->on_init(f->app);
}

void prompt_send(sp_test_t* t, sp_prompt_event_t event) {
  prompt_fixture_t* f = prompt_fixture(t);
  sp_prompt_event_t events [SP_PROMPT_PRIMED_EVENT_CAP] = { event };
  sp_prompt_prime_events(&f->ctx, events);
  f->app->on_poll(f->app);
}

void prompt_tick(sp_test_t* t) {
  prompt_fixture_t* f = prompt_fixture(t);
  f->app->on_update(f->app);
}

sp_err_t prompt_act(sp_test_t* t, const prompt_act_t* acts) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;

  sp_for(it, PROMPT_MAX_ACTS) {
    prompt_act_t act = acts[it];
    switch (act.kind) {
      case PROMPT_ACT_NONE: {
        return SP_OK;
      }
      case PROMPT_ACT_PROGRESS:
      case PROMPT_ACT_STATUS:
      case PROMPT_ACT_LOG:
      case PROMPT_ACT_COMPLETE:
      case PROMPT_ACT_ABORT: {
        if (act.threaded) {
          sp_try(perform_threaded(t, ctx, act));
        } else {
          perform(ctx, act);
        }
        break;
      }
      case PROMPT_ACT_TICK: {
        sp_for(tick, act.ticks) {
          sp_app_tick(probe_app(t));
        }
        break;
      }
      case PROMPT_ACT_RUN: {
        sp_for(tick, PROMPT_MAX_TICKS) {
          if (sp_app_tick(probe_app(t)) != SP_APP_CONTINUE) break;
        }
        break;
      }
    }
  }
  return SP_OK;
}

sp_err_t prompt_expect_last(sp_test_t* t, const prompt_expect_t* expect) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;
  expect_result(t, expect);
  sp_must(t, !sp_da_empty(ctx->frames));
  expect_frame(t, ctx->frames[sp_da_size(ctx->frames) - 1], expect);
  return SP_OK;
}

sp_err_t prompt_expect_live(sp_test_t* t, const prompt_expect_t* expect) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;
  expect_result(t, expect);
  expect_frame(t, (sp_prompt_frame_t) {
    .cols = ctx->cols,
    .rows = ctx->cursor_row,
    .cells = ctx->framebuffer,
  }, expect);
  return SP_OK;
}
