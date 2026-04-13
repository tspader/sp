/*
  sp_prompt.h (beautiful, interactive prompts for native CLIs)

  ///////////
  // USAGE //
  ///////////
  Define either of the following before you include sp_prompt.h in exactly one C or C++ file:

    #define SP_IMPLEMENTATION
    #define SP_PROMPT_IMPLEMENTATION

  - configure default buffering / buffer size?
  -
*/

#if defined SP_IMPLEMENTATION && !defined(SP_PROMPT_IMPLEMENTATION)
  #define SP_PROMPT_IMPLEMENTATION
#endif

#ifndef SP_PROMPT_H
#define SP_PROMPT_H

#include "sp.h"

#if !defined(SP_FREESTANDING) && !defined(SP_WIN32)
#include <termios.h>
#endif

typedef enum {
  SP_PROMPT_EVENT_NONE,
  SP_PROMPT_EVENT_INIT,
  SP_PROMPT_EVENT_INPUT,
  SP_PROMPT_EVENT_UP,
  SP_PROMPT_EVENT_DOWN,
  SP_PROMPT_EVENT_LEFT,
  SP_PROMPT_EVENT_RIGHT,
  SP_PROMPT_EVENT_ENTER,
  SP_PROMPT_EVENT_TAB,
  SP_PROMPT_EVENT_BACKSPACE,
  SP_PROMPT_EVENT_CTRL_C,
  SP_PROMPT_EVENT_ESCAPE,
} sp_prompt_event_kind_t;

typedef struct {
  sp_prompt_event_kind_t kind;
  union {
    struct {
      u32 codepoint;
    } input;
  };
} sp_prompt_event_t;

typedef enum {
  SP_PROMPT_STATE_ACTIVE,
  SP_PROMPT_STATE_SUBMIT,
  SP_PROMPT_STATE_CANCEL,
  SP_PROMPT_STATE_ERROR,
} sp_prompt_state_t;

typedef enum {
  SP_PROMPT_VALUE_NONE,
  SP_PROMPT_VALUE_STR,
  SP_PROMPT_VALUE_BOOL,
} sp_prompt_value_kind_t;

typedef enum {
  SP_PROMPT_STYLE_NONE,
  SP_PROMPT_STYLE_ANSI,
  SP_PROMPT_STYLE_RGB,
} sp_prompt_style_kind_t;

typedef struct {
  sp_prompt_value_kind_t kind;
  union {
    sp_str_t str;
    bool bool_value;
  } as;
} sp_prompt_value_t;

typedef struct {
  u8 tag;
  union {
    struct {
      u8 r;
      u8 g;
      u8 b;
    } rgb;
    u8 ansi;
  };
} sp_prompt_style_t;

typedef struct {
  u32 codepoint;
  sp_prompt_style_t style;
} sp_prompt_cell_t;

typedef struct {
  s32 cols;
  s32 rows;
  sp_prompt_cell_t* cells;
} sp_prompt_frame_t;

typedef struct {
  void* user_data;
  s32 cols;
  s32 rows;
  s32 cursor_row;
  s32 cursor_col;
  s32 prompt_height;
  sp_prompt_state_t state;
  sp_prompt_value_t value;
  sp_prompt_event_t event;
  sp_prompt_event_t primed_events[8];
  u32 primed_count;
  u32 primed_index;
  u8* write_buffer;
  sp_io_writer_t* writer;
  sp_io_writer_t writer_stdout;
  sp_prompt_cell_t* framebuffer;
  sp_da(sp_prompt_frame_t) frames;
  struct {
    struct { sp_os_file_handle_t in; sp_os_file_handle_t out; } fds;
    sp_termios_t cache;
    bool raw;
  } terminal;
} sp_prompt_ctx_t;

typedef void (*sp_prompt_update_fn)(sp_prompt_ctx_t* ctx);
typedef void (*sp_prompt_render_fn)(sp_prompt_ctx_t* ctx);

typedef struct {
  void* user_data;
  sp_prompt_update_fn update;
  sp_prompt_render_fn render;
} sp_prompt_widget_t;

typedef struct {
  sp_str_t text;
} sp_prompt_intro_t;

typedef struct {
  sp_str_t text;
} sp_prompt_outro_t;

typedef struct {
  sp_str_t message;
  sp_str_t title;
} sp_prompt_note_t;

typedef struct {
  sp_str_t text;
  u32 symbol;
  u8 ansi;
} sp_prompt_message_t;

typedef struct {
  sp_str_t prompt;
  sp_str_t prefill;
  sp_str_t value;
} sp_prompt_text_t;

typedef struct {
  sp_str_t prompt;
  bool value;
} sp_prompt_confirm_t;

typedef struct {
  const c8* label;
  const c8* hint;
  bool selected;
} sp_prompt_select_option_t;

typedef struct {
  u32 cursor;
  u32 visible_offset;
  sp_str_t filter_value;
} sp_prompt_choice_state_t;

typedef struct {
  const c8* prompt;
  sp_prompt_select_option_t* options;
  u32 num_options;
  u32 max_items;
  bool filter;
  sp_prompt_choice_state_t state;
} sp_prompt_select_t;

typedef struct {
  const c8* prompt;
  sp_prompt_select_option_t* options;
  u32 num_options;
  u32 max_items;
  bool filter;
  sp_prompt_choice_state_t state;
} sp_prompt_multiselect_t;

typedef struct {
  sp_str_t prompt;
  sp_str_t prefill;
  sp_str_t value;
  bool mask;
} sp_prompt_password_t;

sp_prompt_ctx_t* sp_prompt_begin();
void             sp_prompt_end(sp_prompt_ctx_t* ctx);
void             sp_prompt_intro(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_outro(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_note(sp_prompt_ctx_t* ctx, const c8* text, const c8* title);
void             sp_prompt_cancel(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_info(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_warn(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_error(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_success(sp_prompt_ctx_t* ctx, const c8* text);
const c8*        sp_prompt_text(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* initial);
bool             sp_prompt_confirm(sp_prompt_ctx_t* ctx, const c8* prompt, bool initial);
void             sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt);
void             sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t prompt);
const c8*        sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);
bool             sp_prompt_submitted(sp_prompt_ctx_t* ctx);
bool             sp_prompt_cancelled(sp_prompt_ctx_t* ctx);
const c8*        sp_prompt_get_str(sp_prompt_ctx_t* ctx);
bool             sp_prompt_get_bool(sp_prompt_ctx_t* ctx);
void             sp_prompt_set_str(sp_prompt_ctx_t* ctx, sp_str_t value);
void             sp_prompt_set_bool(sp_prompt_ctx_t* ctx, bool value);
void             sp_prompt_set_state(sp_prompt_ctx_t* ctx, sp_prompt_state_t state);
void             sp_prompt_line(sp_prompt_ctx_t* ctx, sp_str_t text);
bool             sp_prompt_run(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget);

sp_prompt_ctx_t* sp_prompt_new();
s32  sp_prompt_begin_ex(sp_prompt_ctx_t* ctx);
void sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, s32 cols, s32 rows);
void sp_prompt_prime_events(sp_prompt_ctx_t* ctx, sp_prompt_event_t events[8]);
void sp_prompt_step(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event);
#endif

#if defined(SP_PROMPT_IMPLEMENTATION)
sp_prompt_widget_t sp_prompt_intro_widget(sp_prompt_intro_t* prompt);
sp_prompt_widget_t sp_prompt_outro_widget(sp_prompt_outro_t* prompt);
sp_prompt_widget_t sp_prompt_note_widget(sp_prompt_note_t* prompt);
sp_prompt_widget_t sp_prompt_message_widget(sp_prompt_message_t* prompt);
sp_prompt_widget_t sp_prompt_text_widget(sp_prompt_text_t* prompt);
sp_prompt_widget_t sp_prompt_confirm_widget(sp_prompt_confirm_t* prompt);
sp_prompt_widget_t sp_prompt_select_widget(sp_prompt_select_t* prompt);
sp_prompt_widget_t sp_prompt_multiselect_widget(sp_prompt_multiselect_t* prompt);
sp_prompt_widget_t sp_prompt_password_widget(sp_prompt_password_t* prompt);

static s32 sp_prompt_enable_raw_mode(sp_prompt_ctx_t* ctx) {
#if defined(SP_WIN32)
  HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!GetConsoleMode(hin, &ctx->terminal.cache.input_mode)) return -1;
  if (!GetConsoleMode(hout, &ctx->terminal.cache.output_mode)) return -1;

  SetConsoleOutputCP(CP_UTF8);
  _setmode(0, _O_BINARY);

  DWORD raw_in = ENABLE_VIRTUAL_TERMINAL_INPUT;
  DWORD raw_out = ctx->terminal.cache.output_mode
    | ENABLE_PROCESSED_OUTPUT
    | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hin, raw_in)) return -1;
  if (!SetConsoleMode(hout, raw_out)) return -1;
#else
  if (sp_tcgetattr(ctx->terminal.fds.in, &ctx->terminal.cache) == -1) {
    return -1;
  }

  sp_termios_t raw = ctx->terminal.cache;
  raw.c_iflag &= (u32)~(SP_BRKINT | SP_ICRNL | SP_INPCK | SP_ISTRIP | SP_IXON);
  raw.c_oflag &= (u32)~(SP_OPOST);
  raw.c_cflag |= (u32)SP_CS8;
  raw.c_lflag &= (u32)~(SP_ECHO | SP_ICANON | SP_IEXTEN | SP_ISIG);
  raw.c_cc[SP_VMIN] = 1;
  raw.c_cc[SP_VTIME] = 0;

  if (sp_tcsetattr(ctx->terminal.fds.in, SP_TCSAFLUSH, &raw) == -1) {
    return -1;
  }
#endif

  ctx->terminal.raw = true;
  return 0;
}

static void sp_prompt_framebuffer_init(sp_prompt_ctx_t* ctx) {
  u32 cell_count = (u32)(ctx->cols * ctx->rows);
  if (ctx->framebuffer == SP_NULLPTR) {
    ctx->framebuffer = sp_alloc(sizeof(sp_prompt_cell_t) * cell_count);
  }
}

static void sp_prompt_framebuffer_clear(sp_prompt_ctx_t* ctx) {
  u32 cell_count = (u32)(ctx->cols * ctx->rows);
  sp_for(it, cell_count) {
    ctx->framebuffer[it] = (sp_prompt_cell_t) {
      .codepoint = ' ',
      .style = SP_ZERO_STRUCT(sp_prompt_style_t),
    };
  }
  ctx->cursor_row = 0;
  ctx->cursor_col = 0;
}

sp_prompt_ctx_t* sp_prompt_new() {
  sp_prompt_ctx_t* ctx = sp_alloc_type(sp_prompt_ctx_t);
  s32 cols = 0;
  s32 rows = 0;
  if (sp_os_is_tty(sp_sys_stdout)) {
    sp_os_tty_size(sp_sys_stdout, &cols, &rows);
  }
  if (cols <= 0) cols = 80;
  if (rows <= 0) rows = 20;
  sp_log("is_tty: {}, cols: {}, rows: {}", SP_FMT_U8(sp_os_is_tty(sp_sys_stdout)), SP_FMT_U32(cols), SP_FMT_U32(rows));
  sp_prompt_ctx_init(ctx, cols, rows);
  return ctx;
}

sp_prompt_ctx_t* sp_prompt_begin() {
  sp_prompt_ctx_t* ctx = sp_prompt_new();
  sp_try_as_null(sp_prompt_begin_ex(ctx));

  // Technically leaks on linenoise error, but who cares?
  return ctx;
}

s32 sp_prompt_begin_ex(sp_prompt_ctx_t* ctx) {
  ctx->terminal.fds.in = sp_sys_stdin;
  ctx->terminal.fds.out = sp_sys_stdout;
  ctx->terminal.raw = false;

  return sp_prompt_enable_raw_mode(ctx);
}

void sp_prompt_end(sp_prompt_ctx_t* ctx) {
  if (ctx->terminal.raw) {
#if defined(SP_WIN32)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ctx->terminal.cache.input_mode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ctx->terminal.cache.output_mode);
#else
    sp_tcsetattr(ctx->terminal.fds.in, SP_TCSAFLUSH, &ctx->terminal.cache);
#endif
    ctx->terminal.raw = false;
  }

  sp_sys_write(ctx->terminal.fds.out, "\n", 1);
}

void sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, s32 cols, s32 rows) {
  *ctx = SP_ZERO_STRUCT(sp_prompt_ctx_t);
  ctx->cols = cols;
  ctx->rows = rows;
  ctx->state = SP_PROMPT_STATE_ACTIVE;
  sp_io_writer_from_fd(&ctx->writer_stdout, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
  ctx->writer = &ctx->writer_stdout;
  ctx->write_buffer = sp_alloc_n(u8, 64);
  sp_io_writer_set_buffer(ctx->writer, ctx->write_buffer, 64);
  ctx->frames = SP_NULLPTR;
  sp_prompt_framebuffer_init(ctx);
  sp_prompt_framebuffer_clear(ctx);
}

void sp_prompt_set_str(sp_prompt_ctx_t* ctx, sp_str_t value) {
  ctx->value.kind = SP_PROMPT_VALUE_STR;
  ctx->value.as.str = value;
}

void sp_prompt_set_bool(sp_prompt_ctx_t* ctx, bool value) {
  ctx->value.kind = SP_PROMPT_VALUE_BOOL;
  ctx->value.as.bool_value = value;
}

const c8* sp_prompt_get_str(sp_prompt_ctx_t* ctx) {
  if (ctx->value.kind != SP_PROMPT_VALUE_STR) {
    return "";
  }
  return sp_str_to_cstr(ctx->value.as.str);
}

bool sp_prompt_get_bool(sp_prompt_ctx_t* ctx) {
  if (ctx->value.kind != SP_PROMPT_VALUE_BOOL) {
    return false;
  }
  return ctx->value.as.bool_value;
}

void sp_prompt_set_state(sp_prompt_ctx_t* ctx, sp_prompt_state_t state) {
  ctx->state = state;
}

bool sp_prompt_submitted(sp_prompt_ctx_t* ctx) {
  return ctx->state == SP_PROMPT_STATE_SUBMIT;
}

bool sp_prompt_cancelled(sp_prompt_ctx_t* ctx) {
  return ctx->state == SP_PROMPT_STATE_CANCEL;
}

void sp_prompt_prime_events(sp_prompt_ctx_t* ctx, sp_prompt_event_t events[8]) {
  ctx->primed_count = 0;
  ctx->primed_index = 0;

  sp_for(it, 8) {
    if (events[it].kind == SP_PROMPT_EVENT_NONE) {
      break;
    }

    ctx->primed_events[ctx->primed_count] = events[it];
    ctx->primed_count++;
  }
}

static void sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style) {
  if (ctx->cursor_row < 0 || ctx->cursor_row >= ctx->rows) {
    return;
  }

  sp_str_for_utf8(text, it) {
    if (ctx->cursor_col >= ctx->cols) {
      break;
    }

    s32 index = (ctx->cursor_row * ctx->cols) + ctx->cursor_col;
    ctx->framebuffer[index].codepoint = it.codepoint;
    ctx->framebuffer[index].style = style;
    ctx->cursor_col++;
  }
}

static void sp_prompt_emit_bytes(sp_prompt_ctx_t* ctx, const void* ptr, u64 size) {
  SP_ASSERT(ctx->writer);
  sp_io_write(ctx->writer, ptr, size, SP_NULLPTR);
}

static void sp_prompt_emit_str(sp_prompt_ctx_t* ctx, sp_str_t str) {
  sp_prompt_emit_bytes(ctx, str.data, str.len);
}

#define sp_prompt_emit(ctx, cstr) sp_prompt_emit_bytes(ctx, cstr, sizeof(cstr) - 1)

static void sp_prompt_ansi_home(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, "\r");
}

static void sp_prompt_ansi_up(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, "\x1b[A");
}

static void sp_prompt_ansi_new_line(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, "\n");
}

static void sp_prompt_ansi_clear(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, "\x1b[J");
}

void sp_prompt_line(sp_prompt_ctx_t* ctx, sp_str_t text) {
  sp_prompt_render_line(ctx, text, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

void sp_prompt_step(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event) {
  ctx->user_data = widget.user_data;
  ctx->event = event;
  if (ctx->state == SP_PROMPT_STATE_ACTIVE) {
    SP_ASSERT(widget.update);
    widget.update(ctx);
  }

  sp_prompt_framebuffer_clear(ctx);
  if (widget.render) {
    widget.render(ctx);
  }
}

static bool sp_prompt_stdin_ready(void) {
#if defined(SP_WIN32)
  return WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), 0) == WAIT_OBJECT_0;
#else
  sp_pollfd_t pfd = { .fd = sp_sys_stdin, .events = SP_POLLIN, .revents = 0 };
  s32 r = sp_poll(&pfd, 1, 0);
  return r > 0 && (pfd.revents & SP_POLLIN);
#endif
}

static bool sp_prompt_read_raw_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t* out) {
  if (ctx->primed_index < ctx->primed_count) {
    *out = ctx->primed_events[ctx->primed_index++];
    return true;
  }

  *out = (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_NONE };

  u8 c = 0;
  s64 nread = sp_sys_read(sp_sys_stdin, &c, 1);
  if (nread <= 0) {
    return false;
  }

  if (c == 3) {
    out->kind = SP_PROMPT_EVENT_CTRL_C;
    return true;
  }
  if (c == 9) {
    out->kind = SP_PROMPT_EVENT_TAB;
    return true;
  }
  if (c == 13 || c == 10) {
    out->kind = SP_PROMPT_EVENT_ENTER;
    return true;
  }
  if (c == 127 || c == 8) {
    out->kind = SP_PROMPT_EVENT_BACKSPACE;
    return true;
  }
  if (c == 27) {
    if (!sp_prompt_stdin_ready()) {
      out->kind = SP_PROMPT_EVENT_ESCAPE;
      return true;
    }

    u8 seq[2] = {0};
    if (sp_sys_read(sp_sys_stdin, &seq[0], 1) <= 0) {
      out->kind = SP_PROMPT_EVENT_ESCAPE;
      return true;
    }

    if (sp_prompt_stdin_ready()) {
      if (sp_sys_read(sp_sys_stdin, &seq[1], 1) <= 0) {
        seq[1] = 0;
      }
    }

    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': {
          out->kind = SP_PROMPT_EVENT_UP;
          return true;
        }
        case 'B': {
          out->kind = SP_PROMPT_EVENT_DOWN;
          return true;
        }
        case 'C': {
          out->kind = SP_PROMPT_EVENT_RIGHT;
          return true;
        }
        case 'D': {
          out->kind = SP_PROMPT_EVENT_LEFT;
          return true;
        }
      }
    }

    out->kind = SP_PROMPT_EVENT_ESCAPE;
    return true;
  }

  if ((c & 0x80) == 0) {
    out->kind = SP_PROMPT_EVENT_INPUT;
    out->input.codepoint = c;
    return true;
  }

  u8 utf8_bytes[4] = {0};
  utf8_bytes[0] = c;

  s32 needed = 1;
  if ((c & 0xE0) == 0xC0) {
    needed = 2;
  } else if ((c & 0xF0) == 0xE0) {
    needed = 3;
  } else if ((c & 0xF8) == 0xF0) {
    needed = 4;
  }

  sp_for_range(i, 1, needed) {
    if (sp_sys_read(sp_sys_stdin, &utf8_bytes[i], 1) <= 0) {
      needed = i;
      break;
    }
  }

  out->kind = SP_PROMPT_EVENT_INPUT;
  out->input.codepoint = sp_utf8_decode(utf8_bytes);
  return true;
}

static u32 sp_prompt_trimmed_cols(sp_prompt_cell_t* cells, u32 cols) {
  u32 trim = cols;
  while (trim > 0 && cells[trim - 1].codepoint == ' ') {
    trim--;
  }
  return trim;
}

static bool sp_prompt_style_equal(sp_prompt_style_t left, sp_prompt_style_t right) {
  if (left.tag != right.tag) {
    return false;
  }

  switch (left.tag) {
    case SP_PROMPT_STYLE_NONE: {
      return true;
    }
    case SP_PROMPT_STYLE_ANSI: {
      return left.ansi == right.ansi;
    }
    case SP_PROMPT_STYLE_RGB: {
      return left.rgb.r == right.rgb.r && left.rgb.g == right.rgb.g && left.rgb.b == right.rgb.b;
    }
  }

  return false;
}

static void sp_prompt_write_style(sp_prompt_ctx_t* ctx, sp_prompt_style_t style) {
  switch (style.tag) {
    case SP_PROMPT_STYLE_NONE: {
      sp_prompt_emit_bytes(ctx, "\x1b[0m", 4);
      break;
    }
    case SP_PROMPT_STYLE_ANSI: {
      sp_str_t esc = sp_format("\x1b[{}m", SP_FMT_U8(style.ansi));
      sp_prompt_emit_str(ctx, esc);
      break;
    }
    case SP_PROMPT_STYLE_RGB: {
      sp_str_t esc = sp_format("\x1b[38;2;{};{};{}m", SP_FMT_U8(style.rgb.r), SP_FMT_U8(style.rgb.g), SP_FMT_U8(style.rgb.b));
      sp_prompt_emit_str(ctx, esc);
      break;
    }
  }
}

static void sp_prompt_write_row_cells(sp_prompt_ctx_t* ctx, sp_prompt_cell_t* cells, u32 cols) {
  u32 trim = sp_prompt_trimmed_cols(cells, cols);
  sp_prompt_style_t current = SP_ZERO_STRUCT(sp_prompt_style_t);

  sp_for(col, trim) {
    if (!sp_prompt_style_equal(current, cells[col].style)) {
      current = cells[col].style;
      sp_prompt_write_style(ctx, current);
    }

    u8 utf8[4] = {0};
    u8 len = sp_utf8_encode(cells[col].codepoint, utf8);
    sp_prompt_emit_bytes(ctx, utf8, len);
  }

  sp_prompt_write_style(ctx, SP_ZERO_STRUCT(sp_prompt_style_t));
}

static void sp_prompt_present(sp_prompt_ctx_t* ctx) {
  // erase the previous frame; a prompt's height can change across frames, so
  // use last frame's height instead of this frame's
  if (ctx->prompt_height) {
    sp_prompt_ansi_home(ctx);
    sp_for(it, ctx->prompt_height - 1) {
      sp_prompt_ansi_up(ctx);
    }
    sp_prompt_ansi_home(ctx);
  }

  sp_prompt_ansi_clear(ctx);

  // render the styled framebuffer to the terminal
  sp_for(line, ctx->cursor_row) {
    sp_prompt_write_row_cells(
      ctx,
      ctx->framebuffer + line * (u32)ctx->cols,
      (u32)ctx->cols
    );

    if (line + 1 < ctx->cursor_row) {
      sp_prompt_ansi_home(ctx);
      sp_prompt_ansi_new_line(ctx);
    }
  }

  // save this frame's height
  switch (ctx->state) {
    case SP_PROMPT_STATE_ACTIVE: {
      ctx->prompt_height = ctx->cursor_row; break;
    }
    case SP_PROMPT_STATE_CANCEL:
    case SP_PROMPT_STATE_ERROR:
    case SP_PROMPT_STATE_SUBMIT: {
      ctx->prompt_height = 0; break;
    }
  }

  sp_io_flush(ctx->writer);
}

bool sp_prompt_run(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  SP_ASSERT(widget.update);
  ctx->state = SP_PROMPT_STATE_ACTIVE;
  ctx->value = SP_ZERO_STRUCT(sp_prompt_value_t);

  sp_prompt_event_t event = { .kind = SP_PROMPT_EVENT_INIT };

  if (!sp_da_empty(ctx->frames)) {
    sp_prompt_emit_bytes(ctx, "\r\n│\r\n", 7);
  }

  while (true) {
    sp_prompt_step(ctx, widget, event);
    sp_prompt_present(ctx);

    if (ctx->state != SP_PROMPT_STATE_ACTIVE) break;

    sp_prompt_read_raw_event(ctx, &event);
  }

  if (ctx->cursor_row > 0) {
    u32 rows = (u32)ctx->cursor_row;
    u32 cell_count = rows * (u32)ctx->cols;

    sp_prompt_frame_t frame = {
      .cols = ctx->cols,
      .rows = ctx->cursor_row,
      .cells = sp_alloc(sizeof(sp_prompt_cell_t) * cell_count),
    };

    sp_mem_copy(ctx->framebuffer, frame.cells, sizeof(sp_prompt_cell_t) * cell_count);
    sp_da_push(ctx->frames, frame);
  }

  return ctx->state == SP_PROMPT_STATE_SUBMIT;
}

static u32 sp_prompt_text_width(sp_str_t text) {
  u32 width = 0;
  sp_str_for_utf8(text, it) {
    SP_UNUSED(it);
    width++;
  }
  return width;
}

static sp_str_t sp_prompt_repeat(u32 codepoint, u32 count) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_for(it, count) {
    sp_str_builder_append_utf8(&builder, codepoint);
  }
  return sp_str_builder_to_str(&builder);
}

static void sp_prompt_static_update(sp_prompt_ctx_t* ctx) {
  SP_UNUSED(ctx->event);
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
}

static void sp_prompt_intro_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro_t* prompt = (sp_prompt_intro_t*)ctx->user_data;
  sp_prompt_line(ctx, sp_format("┌  {}", SP_FMT_STR(prompt->text)));
}

static void sp_prompt_outro_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_outro_t* prompt = (sp_prompt_outro_t*)ctx->user_data;
  sp_prompt_line(ctx, sp_format("└  {}", SP_FMT_STR(prompt->text)));
}

static void sp_prompt_note_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_note_t* prompt = (sp_prompt_note_t*)ctx->user_data;

  sp_da(sp_str_t) message_lines = sp_str_split_c8(prompt->message, '\n');

  u32 title_width = sp_prompt_text_width(prompt->title);
  u32 max_line_width = 0;
  sp_da_for(message_lines, it) {
    u32 line_width = sp_prompt_text_width(message_lines[it]);
    if (line_width > max_line_width) {
      max_line_width = line_width;
    }
  }

  u32 width = title_width;
  if (max_line_width > width) {
    width = max_line_width;
  }
  width += 2;

  u32 top_tail_width = 1;
  if (width > title_width + 1) {
    top_tail_width = width - title_width - 1;
  }

  sp_str_t top_tail = sp_prompt_repeat(0x2500, top_tail_width);
  sp_str_t spacer = sp_prompt_repeat(' ', width);
  sp_str_t bottom = sp_prompt_repeat(0x2500, width + 2);

  sp_prompt_line(ctx, sp_format("◇  {} {}╮", SP_FMT_STR(prompt->title), SP_FMT_STR(top_tail)));
  sp_prompt_line(ctx, sp_format("│  {}│", SP_FMT_STR(spacer)));

  sp_da_for(message_lines, it) {
    sp_str_t line = message_lines[it];
    u32 line_width = sp_prompt_text_width(line);
    sp_str_t pad = sp_prompt_repeat(' ', width - line_width);
    sp_prompt_line(ctx, sp_format("│  {}{}│", SP_FMT_STR(line), SP_FMT_STR(pad)));
  }

  sp_prompt_line(ctx, sp_format("│  {}│", SP_FMT_STR(spacer)));
  sp_prompt_line(ctx, sp_format("├{}╯", SP_FMT_STR(bottom)));
}

sp_prompt_widget_t sp_prompt_intro_widget(sp_prompt_intro_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_static_update,
    .render = sp_prompt_intro_render,
  };
}

sp_prompt_widget_t sp_prompt_outro_widget(sp_prompt_outro_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_static_update,
    .render = sp_prompt_outro_render,
  };
}

sp_prompt_widget_t sp_prompt_note_widget(sp_prompt_note_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_static_update,
    .render = sp_prompt_note_render,
  };
}

static void sp_prompt_message_update(sp_prompt_ctx_t* ctx) {
  SP_UNUSED(ctx->event);
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
}

static void sp_prompt_message_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_message_t* prompt = (sp_prompt_message_t*)ctx->user_data;
  sp_prompt_style_t style = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = prompt->ansi,
  };
  sp_prompt_render_line(ctx, sp_format("{}  ", SP_FMT_STR(sp_prompt_repeat(prompt->symbol, 1))), style);
  sp_prompt_render_line(ctx, prompt->text, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

sp_prompt_widget_t sp_prompt_message_widget(sp_prompt_message_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_message_update,
    .render = sp_prompt_message_render,
  };
}

void sp_prompt_intro(sp_prompt_ctx_t* ctx, const c8* title) {
  sp_prompt_intro_t prompt = {
    .text = sp_str_view(title),
  };
  sp_prompt_run(ctx, sp_prompt_intro_widget(&prompt));
}

void sp_prompt_outro(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_outro_t prompt = {
    .text = sp_str_view(message),
  };
  sp_prompt_run(ctx, sp_prompt_outro_widget(&prompt));
}

void sp_prompt_note(sp_prompt_ctx_t* ctx, const c8* message, const c8* title) {
  sp_prompt_note_t prompt = {
    .message = sp_str_view(message),
    .title = sp_str_view(title),
  };
  sp_prompt_run(ctx, sp_prompt_note_widget(&prompt));
}

void sp_prompt_cancel(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_message_t prompt = {
    .text = sp_str_view(message),
    .symbol = 0x25a0,
    .ansi = 31,
  };
  sp_prompt_run(ctx, sp_prompt_message_widget(&prompt));
}

void sp_prompt_info(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_message_t prompt = {
    .text = sp_str_view(message),
    .symbol = 0x25cf,
    .ansi = 36,
  };
  sp_prompt_run(ctx, sp_prompt_message_widget(&prompt));
}

void sp_prompt_warn(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_message_t prompt = {
    .text = sp_str_view(message),
    .symbol = 0x25b2,
    .ansi = 33,
  };
  sp_prompt_run(ctx, sp_prompt_message_widget(&prompt));
}

void sp_prompt_error(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_message_t prompt = {
    .text = sp_str_view(message),
    .symbol = 0x25a0,
    .ansi = 31,
  };
  sp_prompt_run(ctx, sp_prompt_message_widget(&prompt));
}

void sp_prompt_success(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_message_t prompt = {
    .text = sp_str_view(message),
    .symbol = 0x25c6,
    .ansi = 32,
  };
  sp_prompt_run(ctx, sp_prompt_message_widget(&prompt));
}

static void sp_prompt_str_append_codepoint(sp_str_t* value, u32 codepoint) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, *value);
  sp_str_builder_append_utf8(&builder, codepoint);
  *value = sp_str_builder_to_str(&builder);
}

static sp_str_t sp_prompt_str_pop_codepoint(sp_str_t value) {
  if (sp_str_empty(value)) {
    return value;
  }

  sp_utf8_it_t it = sp_utf8_rit(value);
  if (!sp_utf8_it_valid(&it)) {
    return SP_LIT("");
  }

  return sp_str(value.data, (u32)it.index);
}

static void sp_prompt_text_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_text_t* text = (sp_prompt_text_t*)ctx->user_data;

  switch (ctx->event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_str_append_codepoint(&text->value, ctx->event.input.codepoint);
      break;
    }
    case SP_PROMPT_EVENT_BACKSPACE: {
      text->value = sp_prompt_str_pop_codepoint(text->value);
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      sp_prompt_set_str(ctx, sp_str_empty(text->value) ? text->prefill : text->value);
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_TAB:
    case SP_PROMPT_EVENT_ESCAPE: {
      break;
    }
  }
}

static sp_str_t sp_prompt_state_symbol(sp_prompt_state_t state) {
  switch (state) {
    case SP_PROMPT_STATE_ACTIVE: {
      return SP_LIT("◆");
    }
    case SP_PROMPT_STATE_SUBMIT: {
      return SP_LIT("◇");
    }
    case SP_PROMPT_STATE_CANCEL: {
      return SP_LIT("■");
    }
    case SP_PROMPT_STATE_ERROR: {
      return SP_LIT("▲");
    }
  }

  sp_unreachable();
  return sp_str_lit("");
}

static sp_prompt_style_t sp_prompt_rail_style(sp_prompt_ctx_t* ctx) {
  switch (ctx->state) {
    case SP_PROMPT_STATE_ACTIVE: {
      return (sp_prompt_style_t) {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = 34,
      };
    }
    case SP_PROMPT_STATE_SUBMIT:
    case SP_PROMPT_STATE_CANCEL:
    case SP_PROMPT_STATE_ERROR: {
      return SP_ZERO_STRUCT(sp_prompt_style_t);
    }
  }
  sp_unreachable();
  return SP_ZERO_STRUCT(sp_prompt_style_t);
}

static void sp_prompt_write_state_prefix(sp_prompt_ctx_t* ctx) {
  sp_prompt_render_line(ctx, sp_prompt_state_symbol(ctx->state), sp_prompt_rail_style(ctx));
  sp_prompt_render_line(ctx, SP_LIT("  "), SP_ZERO_STRUCT(sp_prompt_style_t));
}

static void sp_prompt_write_rail_prefix(sp_prompt_ctx_t* ctx) {
  sp_prompt_render_line(ctx, SP_LIT("│"), sp_prompt_rail_style(ctx));
  sp_prompt_render_line(ctx, SP_LIT("  "), SP_ZERO_STRUCT(sp_prompt_style_t));
}

static void sp_prompt_line_rail_end(sp_prompt_ctx_t* ctx) {
  sp_prompt_render_line(ctx, SP_LIT("└"), sp_prompt_rail_style(ctx));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

static void sp_prompt_text_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_text_t* text = (sp_prompt_text_t*)ctx->user_data;
  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, text->prompt, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (ctx->cursor_row < 0 || ctx->cursor_row >= ctx->rows) {
    return;
  }

  sp_prompt_write_rail_prefix(ctx);

  if (sp_str_empty(text->value)) {
    if (sp_str_empty(text->prefill)) {
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    } else {
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = 90,
      };
      sp_prompt_render_line(ctx, text->prefill, style);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }
  } else {
    sp_prompt_render_line(ctx, text->value, SP_ZERO_STRUCT(sp_prompt_style_t));
    ctx->cursor_col = 0;
    ctx->cursor_row++;
  }
}

sp_prompt_widget_t sp_prompt_text_widget(sp_prompt_text_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_text_update,
    .render = sp_prompt_text_render,
  };
}

static void sp_prompt_confirm_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_confirm_t* confirm = (sp_prompt_confirm_t*)ctx->user_data;

  switch (ctx->event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      if (ctx->event.input.codepoint == 'y' || ctx->event.input.codepoint == 'Y') {
        confirm->value = true;
      } else if (ctx->event.input.codepoint == 'n' || ctx->event.input.codepoint == 'N') {
        confirm->value = false;
      } else if (
        ctx->event.input.codepoint == 'h' ||
        ctx->event.input.codepoint == 'H' ||
        ctx->event.input.codepoint == 'j' ||
        ctx->event.input.codepoint == 'J' ||
        ctx->event.input.codepoint == 'k' ||
        ctx->event.input.codepoint == 'K' ||
        ctx->event.input.codepoint == 'l' ||
        ctx->event.input.codepoint == 'L'
      ) {
        confirm->value = !confirm->value;
      }
      break;
    }
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_TAB: {
      confirm->value = !confirm->value;
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_BACKSPACE: {
      break;
    }
  }

  if (ctx->state == SP_PROMPT_STATE_SUBMIT) {
    sp_prompt_set_bool(ctx, confirm->value);
  }
}

static void sp_prompt_confirm_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_confirm_t* confirm = (sp_prompt_confirm_t*)ctx->user_data;

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 32,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 90,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, confirm->prompt, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  sp_prompt_write_rail_prefix(ctx);

  if (confirm->value) {
    sp_prompt_render_line(ctx, SP_LIT("●"), active_symbol);
    sp_prompt_render_line(ctx, SP_LIT(" Yes"), SP_ZERO_STRUCT(sp_prompt_style_t));
    sp_prompt_render_line(ctx, SP_LIT(" / "), SP_ZERO_STRUCT(sp_prompt_style_t));
    sp_prompt_render_line(ctx, SP_LIT("○ No"), inactive);
  } else {
    sp_prompt_render_line(ctx, SP_LIT("○ Yes"), inactive);
    sp_prompt_render_line(ctx, SP_LIT(" / "), SP_ZERO_STRUCT(sp_prompt_style_t));
    sp_prompt_render_line(ctx, SP_LIT("●"), active_symbol);
    sp_prompt_render_line(ctx, SP_LIT(" No"), SP_ZERO_STRUCT(sp_prompt_style_t));
  }

  ctx->cursor_col = 0;
  ctx->cursor_row++;

  switch (ctx->state) {
    case SP_PROMPT_STATE_ACTIVE:
    case SP_PROMPT_STATE_CANCEL:
    case SP_PROMPT_STATE_ERROR: {
      sp_prompt_line_rail_end(ctx);
      break;
    }
    case SP_PROMPT_STATE_SUBMIT: {
      break;
    }
  }
}

sp_prompt_widget_t sp_prompt_confirm_widget(sp_prompt_confirm_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_confirm_update,
    .render = sp_prompt_confirm_render,
  };
}

static void sp_prompt_choice_state_reset(sp_prompt_choice_state_t* state, u32 cursor) {
  state->cursor = cursor;
  state->visible_offset = 0;
  state->filter_value = SP_LIT("");
}

static void sp_prompt_choice_state_sync_window(sp_prompt_choice_state_t* state, u32* max_items, u32 filtered_count) {
  if (*max_items == 0) {
    *max_items = 1;
  }

  if (filtered_count == 0) {
    state->cursor = 0;
    state->visible_offset = 0;
    return;
  }

  if (state->cursor >= filtered_count) {
    state->cursor = filtered_count - 1;
  }

  if (state->visible_offset > state->cursor) {
    state->visible_offset = state->cursor;
  }

  if (state->cursor >= state->visible_offset + *max_items) {
    state->visible_offset = state->cursor + 1 - *max_items;
  }

  u32 max_offset = 0;
  if (filtered_count > *max_items) {
    max_offset = filtered_count - *max_items;
  }

  if (state->visible_offset > max_offset) {
    state->visible_offset = max_offset;
  }
}

static void sp_prompt_choice_state_move_cursor(sp_prompt_choice_state_t* state, u32 filtered_count, s32 delta) {
  if (filtered_count == 0) {
    return;
  }

  if (delta > 0) {
    if (state->cursor + 1 < filtered_count) {
      state->cursor++;
    } else {
      state->cursor = 0;
    }
  } else if (delta < 0) {
    if (state->cursor > 0) {
      state->cursor--;
    } else {
      state->cursor = filtered_count - 1;
    }
  }
}

static sp_str_t sp_prompt_option_label(sp_prompt_select_option_t* option) {
  if (option->label == SP_NULLPTR) {
    return SP_LIT("");
  }

  return sp_str_view(option->label);
}

static sp_str_t sp_prompt_option_hint(sp_prompt_select_option_t* option) {
  if (option->hint == SP_NULLPTR) {
    return SP_LIT("");
  }

  return sp_str_view(option->hint);
}

static bool sp_prompt_str_contains_case_insensitive(sp_str_t str, sp_str_t needle) {
  if (str.len < needle.len) {
    return false;
  }

  for (u32 i = 0; i <= str.len - needle.len; i++) {
    bool equal = true;
    sp_for(j, needle.len) {
      if (sp_c8_to_lower(str.data[i + j]) != sp_c8_to_lower(needle.data[j])) {
        equal = false;
        break;
      }
    }

    if (equal) {
      return true;
    }
  }

  return false;
}

static u32 sp_prompt_select_initial_cursor(sp_prompt_select_t* select) {
  sp_for(it, select->num_options) {
    if (select->options[it].selected) {
      return it;
    }
  }

  return 0;
}

static bool sp_prompt_select_matches_filter(sp_prompt_select_t* select, u32 option_index) {
  if (!select->filter || sp_str_empty(select->state.filter_value)) {
    return true;
  }

  return sp_prompt_str_contains_case_insensitive(sp_prompt_option_label(&select->options[option_index]), select->state.filter_value);
}

static u32 sp_prompt_select_filtered_count(sp_prompt_select_t* select) {
  u32 count = 0;
  sp_for(it, select->num_options) {
    if (sp_prompt_select_matches_filter(select, it)) {
      count++;
    }
  }

  return count;
}

static u32 sp_prompt_select_filtered_to_option_index(sp_prompt_select_t* select, u32 filtered_index) {
  u32 count = 0;
  sp_for(it, select->num_options) {
    if (!sp_prompt_select_matches_filter(select, it)) {
      continue;
    }

    if (count == filtered_index) {
      return it;
    }

    count++;
  }

  return 0;
}

static void sp_prompt_select_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_select_t* select = (sp_prompt_select_t*)ctx->user_data;

  if (ctx->event.kind == SP_PROMPT_EVENT_INIT) {
    sp_prompt_choice_state_reset(&select->state, sp_prompt_select_initial_cursor(select));
  }

  u32 filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);

  switch (ctx->event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      if (select->num_options == 0) {
        break;
      }

      if (select->filter) {
        sp_prompt_str_append_codepoint(&select->state.filter_value, ctx->event.input.codepoint);
      } else {
        if (ctx->event.input.codepoint == 'j' || ctx->event.input.codepoint == 'J') {
          sp_prompt_choice_state_move_cursor(&select->state, filtered_count, 1);
        } else if (ctx->event.input.codepoint == 'k' || ctx->event.input.codepoint == 'K') {
          sp_prompt_choice_state_move_cursor(&select->state, filtered_count, -1);
        }
      }
      break;
    }
    case SP_PROMPT_EVENT_UP: {
      sp_prompt_choice_state_move_cursor(&select->state, filtered_count, -1);
      break;
    }
    case SP_PROMPT_EVENT_DOWN: {
      sp_prompt_choice_state_move_cursor(&select->state, filtered_count, 1);
      break;
    }
    case SP_PROMPT_EVENT_BACKSPACE: {
      if (select->filter) {
        select->state.filter_value = sp_prompt_str_pop_codepoint(select->state.filter_value);
      }
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      if (filtered_count > 0) {
        u32 selected_index = sp_prompt_select_filtered_to_option_index(select, select->state.cursor);
        sp_prompt_set_str(ctx, sp_prompt_option_label(&select->options[selected_index]));
      } else {
        sp_prompt_set_str(ctx, SP_LIT(""));
      }
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_TAB: {
      break;
    }
  }

  filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);
}

static void sp_prompt_select_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_select_t* select = (sp_prompt_select_t*)ctx->user_data;
  u32 filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 32,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 90,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, select->prompt == SP_NULLPTR ? SP_LIT("") : sp_str_view(select->prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  if (select->filter) {
    sp_prompt_render_line(ctx, SP_LIT(" "), SP_ZERO_STRUCT(sp_prompt_style_t));
    if (sp_str_empty(select->state.filter_value)) {
      sp_prompt_render_line(ctx, SP_LIT("Type to filter..."), inactive);
    } else {
      sp_prompt_render_line(ctx, select->state.filter_value, SP_ZERO_STRUCT(sp_prompt_style_t));
    }
  }
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (ctx->state == SP_PROMPT_STATE_SUBMIT) {
    sp_prompt_write_rail_prefix(ctx);
    if (filtered_count > 0) {
      u32 selected_index = sp_prompt_select_filtered_to_option_index(select, select->state.cursor);
      sp_prompt_render_line(ctx, sp_prompt_option_label(&select->options[selected_index]), inactive);
    }
    ctx->cursor_col = 0;
    ctx->cursor_row++;
  } else if (filtered_count == 0) {
    sp_prompt_write_rail_prefix(ctx);
    sp_prompt_render_line(ctx, SP_LIT("(no options)"), inactive);
    ctx->cursor_col = 0;
    ctx->cursor_row++;
  } else {
    u32 visible_count = filtered_count - select->state.visible_offset;
    if (visible_count > select->max_items) {
      visible_count = select->max_items;
    }

    bool has_top_overflow = select->state.visible_offset > 0;
    bool has_bottom_overflow = select->state.visible_offset + visible_count < filtered_count;
    u32 render_offset = select->state.visible_offset;

    if (has_top_overflow && has_bottom_overflow && visible_count > 0) {
      u32 hidden_top_count = render_offset;
      u32 hidden_bottom_count = filtered_count - (render_offset + visible_count);

      if (hidden_bottom_count == 1 && select->state.cursor > render_offset) {
        render_offset++;
        has_bottom_overflow = false;
      } else if (hidden_top_count == 1 && select->state.cursor + 1 < render_offset + visible_count) {
        render_offset--;
        has_top_overflow = false;
      } else {
        visible_count--;
        if (select->state.cursor > render_offset) {
          render_offset++;
        }
      }
    }

    if (has_top_overflow) {
      sp_prompt_write_rail_prefix(ctx);
      sp_prompt_render_line(ctx, SP_LIT("..."), inactive);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }

    sp_for(it, visible_count) {
      u32 filtered_index = render_offset + it;
      u32 index = sp_prompt_select_filtered_to_option_index(select, filtered_index);
      sp_prompt_write_rail_prefix(ctx);

      if (filtered_index == select->state.cursor) {
        sp_prompt_render_line(ctx, SP_LIT("●"), active_symbol);
        sp_prompt_render_line(ctx, SP_LIT(" "), SP_ZERO_STRUCT(sp_prompt_style_t));
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->options[index]), SP_ZERO_STRUCT(sp_prompt_style_t));
      } else {
        sp_prompt_render_line(ctx, SP_LIT("○ "), inactive);
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->options[index]), inactive);
      }

      sp_str_t hint = sp_prompt_option_hint(&select->options[index]);
      if (!sp_str_empty(hint)) {
        sp_prompt_render_line(ctx, SP_LIT(" ("), inactive);
        sp_prompt_render_line(ctx, hint, inactive);
        sp_prompt_render_line(ctx, SP_LIT(")"), inactive);
      }

      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }

    if (has_bottom_overflow) {
      sp_prompt_write_rail_prefix(ctx);
      sp_prompt_render_line(ctx, SP_LIT("..."), inactive);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }
  }

  switch (ctx->state) {
    case SP_PROMPT_STATE_ACTIVE:
    case SP_PROMPT_STATE_CANCEL:
    case SP_PROMPT_STATE_ERROR: {
      sp_prompt_line_rail_end(ctx);
      break;
    }
    case SP_PROMPT_STATE_SUBMIT: {
      break;
    }
  }
}

sp_prompt_widget_t sp_prompt_select_widget(sp_prompt_select_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_select_update,
    .render = sp_prompt_select_render,
  };
}

static bool sp_prompt_multiselect_matches_filter(sp_prompt_multiselect_t* select, u32 option_index) {
  if (!select->filter || sp_str_empty(select->state.filter_value)) {
    return true;
  }

  return sp_prompt_str_contains_case_insensitive(sp_prompt_option_label(&select->options[option_index]), select->state.filter_value);
}

static u32 sp_prompt_multiselect_filtered_count(sp_prompt_multiselect_t* select) {
  u32 count = 0;
  sp_for(it, select->num_options) {
    if (sp_prompt_multiselect_matches_filter(select, it)) {
      count++;
    }
  }

  return count;
}

static u32 sp_prompt_multiselect_filtered_to_option_index(sp_prompt_multiselect_t* select, u32 filtered_index) {
  u32 count = 0;
  sp_for(it, select->num_options) {
    if (!sp_prompt_multiselect_matches_filter(select, it)) {
      continue;
    }

    if (count == filtered_index) {
      return it;
    }

    count++;
  }

  return 0;
}

static void sp_prompt_multiselect_toggle_current(sp_prompt_multiselect_t* select, u32 filtered_count) {
  if (filtered_count == 0) {
    return;
  }

  u32 option_index = sp_prompt_multiselect_filtered_to_option_index(select, select->state.cursor);
  select->options[option_index].selected = !select->options[option_index].selected;
}

static void sp_prompt_multiselect_handle_input(sp_prompt_multiselect_t* select, u32 codepoint, u32 filtered_count) {
  if (select->num_options == 0) {
    return;
  }

  if (codepoint == ' ') {
    sp_prompt_multiselect_toggle_current(select, filtered_count);
    return;
  }

  if (select->filter) {
    sp_prompt_str_append_codepoint(&select->state.filter_value, codepoint);
    return;
  }

  if (codepoint == 'j' || codepoint == 'J') {
    sp_prompt_choice_state_move_cursor(&select->state, filtered_count, 1);
    return;
  }

  if (codepoint == 'k' || codepoint == 'K') {
    sp_prompt_choice_state_move_cursor(&select->state, filtered_count, -1);
  }
}

static void sp_prompt_multiselect_backspace(sp_prompt_multiselect_t* select) {
  if (select->filter) {
    select->state.filter_value = sp_prompt_str_pop_codepoint(select->state.filter_value);
  }
}

static void sp_prompt_multiselect_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_multiselect_t* select = (sp_prompt_multiselect_t*)ctx->user_data;

  if (ctx->event.kind == SP_PROMPT_EVENT_INIT) {
    sp_prompt_choice_state_reset(&select->state, 0);
  }

  u32 filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);

  switch (ctx->event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_multiselect_handle_input(select, ctx->event.input.codepoint, filtered_count);
      break;
    }
    case SP_PROMPT_EVENT_UP: {
      sp_prompt_choice_state_move_cursor(&select->state, filtered_count, -1);
      break;
    }
    case SP_PROMPT_EVENT_DOWN: {
      sp_prompt_choice_state_move_cursor(&select->state, filtered_count, 1);
      break;
    }
    case SP_PROMPT_EVENT_BACKSPACE: {
      sp_prompt_multiselect_backspace(select);
      break;
    }
    case SP_PROMPT_EVENT_TAB: {
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      //sp_prompt_set_str(ctx, sp_prompt_multiselect_join_labels(select));
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT: {
      break;
    }
  }

  filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);
}

static void sp_prompt_multiselect_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_multiselect_t* select = (sp_prompt_multiselect_t*)ctx->user_data;
  u32 filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->max_items, filtered_count);

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 32,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = 90,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, select->prompt == SP_NULLPTR ? SP_LIT("") : sp_str_view(select->prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  if (select->filter) {
    sp_prompt_render_line(ctx, SP_LIT(" "), SP_ZERO_STRUCT(sp_prompt_style_t));
    if (sp_str_empty(select->state.filter_value)) {
      sp_prompt_render_line(ctx, SP_LIT("Type to filter..."), inactive);
    } else {
      sp_prompt_render_line(ctx, select->state.filter_value, SP_ZERO_STRUCT(sp_prompt_style_t));
    }
  }
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (ctx->state == SP_PROMPT_STATE_SUBMIT) {
    sp_prompt_write_rail_prefix(ctx);

    bool first = true;
    sp_for(it, select->num_options) {
      if (!select->options[it].selected) {
        continue;
      }

      if (!first) sp_prompt_render_line(ctx, sp_str_lit(", "), inactive);
      first = false;
      sp_prompt_render_line(ctx, sp_prompt_option_label(select->options + it), inactive);

    }

    ctx->cursor_col = 0;
    ctx->cursor_row++;
  } else if (filtered_count == 0) {
    sp_prompt_write_rail_prefix(ctx);
    sp_prompt_render_line(ctx, SP_LIT("(no options)"), inactive);
    ctx->cursor_col = 0;
    ctx->cursor_row++;
  } else {
    u32 visible_count = filtered_count - select->state.visible_offset;
    if (visible_count > select->max_items) {
      visible_count = select->max_items;
    }

    bool has_top_overflow = select->state.visible_offset > 0;
    bool has_bottom_overflow = select->state.visible_offset + visible_count < filtered_count;
    u32 render_offset = select->state.visible_offset;

    if (has_top_overflow && has_bottom_overflow && visible_count > 0) {
      u32 hidden_top_count = render_offset;
      u32 hidden_bottom_count = filtered_count - (render_offset + visible_count);

      if (hidden_bottom_count == 1 && select->state.cursor > render_offset) {
        render_offset++;
        has_bottom_overflow = false;
      } else if (hidden_top_count == 1 && select->state.cursor + 1 < render_offset + visible_count) {
        render_offset--;
        has_top_overflow = false;
      } else {
        visible_count--;
        if (select->state.cursor > render_offset) {
          render_offset++;
        }
      }
    }

    if (has_top_overflow) {
      sp_prompt_write_rail_prefix(ctx);
      sp_prompt_render_line(ctx, SP_LIT("..."), inactive);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }

    sp_for(it, visible_count) {
      u32 filtered_index = render_offset + it;
      u32 index = sp_prompt_multiselect_filtered_to_option_index(select, filtered_index);
      bool hovered = filtered_index == select->state.cursor;
      bool selected = select->options[index].selected;

      sp_prompt_write_rail_prefix(ctx);

      if (selected) {
        sp_prompt_render_line(ctx, SP_LIT("●"), active_symbol);
      } else if (hovered) {
        sp_prompt_render_line(ctx, SP_LIT("○"), SP_ZERO_STRUCT(sp_prompt_style_t));
      } else {
        sp_prompt_render_line(ctx, SP_LIT("○"), inactive);
      }

      sp_prompt_render_line(ctx, SP_LIT(" "), SP_ZERO_STRUCT(sp_prompt_style_t));

      if (hovered) {
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->options[index]), SP_ZERO_STRUCT(sp_prompt_style_t));
      } else {
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->options[index]), inactive);
      }

      sp_str_t hint = sp_prompt_option_hint(&select->options[index]);
      if (!sp_str_empty(hint)) {
        sp_prompt_render_line(ctx, SP_LIT(" ("), inactive);
        sp_prompt_render_line(ctx, hint, inactive);
        sp_prompt_render_line(ctx, SP_LIT(")"), inactive);
      }

      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }

    if (has_bottom_overflow) {
      sp_prompt_write_rail_prefix(ctx);
      sp_prompt_render_line(ctx, SP_LIT("..."), inactive);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }
  }

  switch (ctx->state) {
    case SP_PROMPT_STATE_ACTIVE:
    case SP_PROMPT_STATE_CANCEL:
    case SP_PROMPT_STATE_ERROR: {
      sp_prompt_line_rail_end(ctx);
      break;
    }
    case SP_PROMPT_STATE_SUBMIT: {
      break;
    }
  }
}

sp_prompt_widget_t sp_prompt_multiselect_widget(sp_prompt_multiselect_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_multiselect_update,
    .render = sp_prompt_multiselect_render,
  };
}

static void sp_prompt_password_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_password_t* password = (sp_prompt_password_t*)ctx->user_data;

  switch (ctx->event.kind) {
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_str_append_codepoint(&password->value, ctx->event.input.codepoint);
      break;
    }
    case SP_PROMPT_EVENT_BACKSPACE: {
      password->value = sp_prompt_str_pop_codepoint(password->value);
      break;
    }
    case SP_PROMPT_EVENT_TAB: {
      password->mask = !password->mask;
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      sp_prompt_set_str(ctx, sp_str_empty(password->value) ? password->prefill : password->value);
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_INIT:
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT: {
      break;
    }
  }
}

static void sp_prompt_password_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_password_t* password = (sp_prompt_password_t*)ctx->user_data;
  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, password->prompt, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  sp_prompt_write_rail_prefix(ctx);

  if (sp_str_empty(password->value)) {
    if (sp_str_empty(password->prefill)) {
      ctx->cursor_col = 0;
      ctx->cursor_row++;
      return;
    }

    if (password->mask) {
      sp_str_t masked = sp_prompt_repeat('*', sp_prompt_text_width(password->prefill));
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = 90,
      };
      sp_prompt_render_line(ctx, masked, style);
    } else {
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = 90,
      };
      sp_prompt_render_line(ctx, password->prefill, style);
    }

    ctx->cursor_col = 0;
    ctx->cursor_row++;
    return;
  }

  if (password->mask) {
    sp_str_t masked = sp_prompt_repeat('*', sp_prompt_text_width(password->value));
    sp_prompt_render_line(ctx, masked, SP_ZERO_STRUCT(sp_prompt_style_t));
  } else {
    sp_prompt_render_line(ctx, password->value, SP_ZERO_STRUCT(sp_prompt_style_t));
  }

  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

sp_prompt_widget_t sp_prompt_password_widget(sp_prompt_password_t* prompt) {
  return (sp_prompt_widget_t) {
    .user_data = prompt,
    .update = sp_prompt_password_update,
    .render = sp_prompt_password_render,
  };
}

const c8* sp_prompt_text(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* initial) {
  sp_prompt_text_t text = {
    .prompt = sp_str_view(prompt),
    .prefill = sp_str_view(initial),
    .value = SP_LIT(""),
  };

  sp_prompt_run(ctx, sp_prompt_text_widget(&text));
  return sp_prompt_get_str(ctx);
}

bool sp_prompt_confirm(sp_prompt_ctx_t* ctx, const c8* prompt, bool initial) {
  sp_prompt_confirm_t confirm = {
    .prompt = sp_str_view(prompt),
    .value = initial,
  };

  sp_prompt_run(ctx, sp_prompt_confirm_widget(&confirm));
  return sp_prompt_get_bool(ctx);
}

void sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt) {
  sp_prompt_run(ctx, sp_prompt_select_widget(&prompt));
}

void sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t prompt) {
  sp_prompt_run(ctx, sp_prompt_multiselect_widget(&prompt));
}

const c8* sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill) {
  sp_prompt_password_t password = {
    .prompt = sp_str_view(prompt),
    .prefill = sp_str_view(prefill),
    .value = SP_LIT(""),
    .mask = true,
  };

  sp_prompt_run(ctx, sp_prompt_password_widget(&password));
  return sp_prompt_get_str(ctx);
}
#endif
