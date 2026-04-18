/*
  sp_prompt.h
  beautiful, interactive, utf-8 prompts for native CLIs

  ## TL;DR
  If you don't want to read this documentation, grep for the following tags to jump to code:

  types
    @values      the values produced by prompts
    @framebuffer the representation of a rendered output; styling + coloring
    @event       events available to be handled by widgets
    @context     the handle into the library; mostly opaque
    @widgets     userdata types for builtin widgets

  functions
    @lifecycle   opening and closing a context
    @widgets     public API for using builtin widgets
    @values      the values produced by prompts
    @custom      helpful functions for writing your own widgets
    @advanced    what it says on the tin

  widgets
    intro
    outro
    note
    cancel
    info
    warn
    error
    success
    text
    confirm
    select
    multiselect
    password


  ## USAGE
  Define either of the following before you include sp_prompt.h in exactly one C or C++ file:

    #define SP_IMPLEMENTATION
    #define SP_PROMPT_IMPLEMENTATION


  ### INITALIZATION
  To use the library, you need (a) a context, and (b) to set up the terminal for drawing. In the common
  case, you can do both with one call:

    sp_prompt_ctx_t* ctx = sp_prompt_begin();

  This detects the size of the terminal, saves the current terminal state, and enters raw mode. If you
  need more control over the order of these operations, or want a custom output size, do this:

    sp_prompt_ctx_t ctx = sp_zero(); // or sp_prompt_new() to use the sp.h allocator + detect size
    sp_prompt_init(&ctx, 69, 420);
    sp_prompt_begin_ex(&ctx, 69, 420);


  ### RUNNING A WIDGET
  Now, call into widgets. The library ships with quite a few widgets out of the box (plus primitives for
  creating your own). Widgets are synchronous, and have both a result and a state that can be queried:

    // Each widget has its own specific options
    sp_prompt_select_option_t options[] = {
      { .label = "hey",   .hint = "recommended" },
      { .label = "hello", .selected = true },
      { .label = "howdy", .hint = "ropers only" },
      { .label = "hi" },
      { .label = "hullo", .hint = "questionable" },
      { .label = "whatup" },
    };

    // This will block until the widget has resolved itself
    sp_prompt_select(ctx, (sp_prompt_select_t) {
      .prompt = "Pick a greeting",
      .options = options,
      .num_options = sp_carr_len(options),
      .max_items = 4,
    });

    // Check whether the user cancelled
    bool cancelled = sp_prompt_cancelled(ctx);

    // Get the entered value
    const c8* greeting = sp_prompt_get_str(ctx);

  This renders something like this:

    ┌  sp_prompt.h widget: select
    │
    ◆  Pick a greeting
    │  ○ hey (recommended)
    │  ● hello
    │  ○ howdy (ropers only)
    │  ○ hi
    │  ...
    └


  ### RUNNING ANOTHER WIDGET
  You can continue to run widgets in the exact same way. For example, you could print the
  result of the previous widget in a box:

    sp_prompt_note(ctx, sp_prompt_get_str(ctx), "Greeting");

  Which renders the following; note that widget output is not strictly additive. That is, the selection
  widget is smart enough to overwrite the options with a single line for the final selection:

    ┌  sp_prompt.h widget: select
    │
    ◇  Pick a greeting
    │  howdy
    │
    ◇  Greeting ─╮
    │            │
    │  howdy     │
    │            │
    ├────────────╯


  ### CLEANUP
  When you're done, just do this:

    sp_prompt_end(ctx);

  Unlike many libraries where freeing is more or less pointless if you're exiting anyway, you **must** call
  this function before your program exits. Otherwise, the user's terminal will be left in raw mode, and it
  will be unusable.


  ## CUSTOM WIDGETS
  At its core, sp_prompt.h isn't much more than a loop like this:

  fn run(widget):
    while (!widget.done)
      event := poll()
      widget.handle(event)
      widget.render()

  A widget is simple an instance of sp_prompt_widget_t, which has three members:
    - An update function which handles an event
    - A render function which writes lines of text to a buffer
    - A userdata pointer, so your widget can have internal state

  Widgets are run using sp_prompt_run(). All the builtin widgets are simply closures which wrap the arguments
  of an ergonomic API into a struct and call sp_prompt_run()


  ### RENDERING
  sp_prompt.h is not sophisticated. It does not present anything resembling a rich immediate mode UI, with
  buttons and input areas which can be composed. Instead, it provides more or less one API:

    sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style);

  This writes a line of text at the widget's cursor and fills each touched cell with the given style. There
  are two auxiliary functions, too: One to calculate text width, and one to repeat a codepoint. Other than
  that, it's just a little arithmetic and box drawing characters.

*/

#if defined SP_IMPLEMENTATION && !defined(SP_PROMPT_IMPLEMENTATION)
  #define SP_PROMPT_IMPLEMENTATION
#endif

#ifndef SP_PROMPT_H
#define SP_PROMPT_H

#include "sp.h"

#define SP_PROMPT_OK 0
#define SP_PROMPT_ERROR 1

////////////
// VALUES //
////////////
// @values
typedef enum {
  SP_PROMPT_VALUE_NONE,
  SP_PROMPT_VALUE_STR,
  SP_PROMPT_VALUE_BOOL,
} sp_prompt_value_kind_t;

typedef struct {
  sp_prompt_value_kind_t kind;
  union {
    sp_str_t str;
    bool bool_value;
  } as;
} sp_prompt_value_t;


/////////////////
// FRAMEBUFFER //
/////////////////
// @framebuffer
typedef enum {
  SP_PROMPT_STYLE_NONE,
  SP_PROMPT_STYLE_ANSI,
  SP_PROMPT_STYLE_RGB,
} sp_prompt_style_kind_t;

typedef struct {
  sp_prompt_style_kind_t tag;
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


////////////
// EVENTS //
////////////
// @event
typedef enum {
  SP_PROMPT_STATE_ACTIVE,
  SP_PROMPT_STATE_SUBMIT,
  SP_PROMPT_STATE_CANCEL,
  SP_PROMPT_STATE_ERROR,
} sp_prompt_state_t;

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

/////////////
// CONTEXT //
/////////////
// @context
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
  struct {
    sp_prompt_event_t events[8];
    u32 count;
    u32 index;
  } primed;
  sp_io_writer_t* writer;
  sp_prompt_cell_t* framebuffer;
  sp_da(sp_prompt_frame_t) frames;
  struct {
    struct { sp_sys_fd_t in; sp_sys_fd_t out; } fds;
    sp_tty_mode_t cache;
    bool raw;
  } terminal;
  sp_mem_arena_t* arena;
} sp_prompt_ctx_t;


/////////////
// WIDGETS //
/////////////
// @widgets
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

/////////
// API //
/////////
// @lifecycle
sp_prompt_ctx_t* sp_prompt_begin();
void             sp_prompt_end(sp_prompt_ctx_t* ctx);

// @widgets
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
bool             sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt);
void             sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t prompt);
const c8*        sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);
bool             sp_prompt_submitted(sp_prompt_ctx_t* ctx);
bool             sp_prompt_cancelled(sp_prompt_ctx_t* ctx);

// @values
const c8*        sp_prompt_get_str(sp_prompt_ctx_t* ctx);
bool             sp_prompt_get_bool(sp_prompt_ctx_t* ctx);
void             sp_prompt_set_str(sp_prompt_ctx_t* ctx, sp_str_t value);
void             sp_prompt_set_bool(sp_prompt_ctx_t* ctx, bool value);
void             sp_prompt_set_state(sp_prompt_ctx_t* ctx, sp_prompt_state_t state);

// @custom
void             sp_prompt_line(sp_prompt_ctx_t* ctx, sp_str_t text);
void             sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style);
u32              sp_prompt_text_width(sp_str_t text);
sp_str_t         sp_prompt_repeat(u32 codepoint, u32 count);

// @advanced
sp_prompt_ctx_t* sp_prompt_new();
s32              sp_prompt_begin_ex(sp_prompt_ctx_t* ctx);
void             sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, s32 cols, s32 rows);
void             sp_prompt_prime_events(sp_prompt_ctx_t* ctx, sp_prompt_event_t events[8]);
bool             sp_prompt_run(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget);
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
  if (sp_os_tty_enter_raw(ctx->terminal.fds.in, &ctx->terminal.cache) == -1) return -1;
  ctx->terminal.raw = true;
  return 0;
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
  sp_prompt_ctx_init(ctx, cols, rows);
  return ctx;
}

sp_prompt_ctx_t* sp_prompt_begin() {
  sp_prompt_ctx_t* ctx = sp_prompt_new();
  sp_try_as_null(sp_prompt_begin_ex(ctx));
  return ctx; // Leaks, but who cares?
}

s32 sp_prompt_begin_ex(sp_prompt_ctx_t* ctx) {
  ctx->terminal.fds.in = sp_sys_stdin;
  ctx->terminal.fds.out = sp_sys_stdout;
  ctx->terminal.raw = false;

  return sp_prompt_enable_raw_mode(ctx);
}

void sp_prompt_end(sp_prompt_ctx_t* ctx) {
  if (ctx->terminal.raw) {
    sp_os_tty_restore(ctx->terminal.fds.in, &ctx->terminal.cache);
    ctx->terminal.raw = false;
  }

  sp_sys_write(ctx->terminal.fds.out, "\n", 1);
  sp_mem_arena_destroy(ctx->arena);
}

void sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, s32 cols, s32 rows) {
  *ctx = sp_zero_struct(sp_prompt_ctx_t);
  ctx->cols = cols;
  ctx->rows = rows;
  ctx->state = SP_PROMPT_STATE_ACTIVE;
  ctx->arena = sp_mem_arena_new();
  sp_context_push_arena(ctx->arena);

  // Write buffering is really important, because our rendering algorithm is extremely
  // naive. It's not much more than this:
  //
  // for row:
  //   for column:
  //     cell := cells[row][column]
  //     emit ANSI style for cell if it changed
  //     emit cell
  //
  // In other words, we call write(), one byte at a time. The simplicity of this approach
  // is excellent, but you don't want the terminal emulator to try to re-render (M x N)
  // times every single frame.
  //
  // Empirically, you get pretty bad tearing on Windows without buffering.
  ctx->writer = sp_alloc_type(sp_io_writer_t);
  sp_io_get_std_out(ctx->writer);

  u8* buffer = sp_alloc_n(u8, 64);
  sp_io_writer_set_buffer(ctx->writer, buffer, 64);

  u32 cell_count = (u32)(ctx->cols * ctx->rows);
  if (ctx->framebuffer == SP_NULLPTR) {
    ctx->framebuffer = sp_alloc(sizeof(sp_prompt_cell_t) * cell_count);
  }
  sp_prompt_framebuffer_clear(ctx);
  sp_context_pop();
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
  ctx->primed.count = 0;
  ctx->primed.index = 0;

  sp_for(it, 8) {
    if (events[it].kind == SP_PROMPT_EVENT_NONE) {
      break;
    }

    ctx->primed.events[ctx->primed.count] = events[it];
    ctx->primed.count++;
  }
}

void sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style) {
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

static bool sp_prompt_poll_stdin() {
  sp_sys_pollfd_t pfd = { .fd = sp_sys_stdin, .events = SP_POLLIN };
  return sp_sys_poll(&pfd, 1, 0) > 0 && (pfd.revents & SP_POLLIN);
}

static bool sp_prompt_read_raw_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t* out) {
  if (ctx->primed.index < ctx->primed.count) {
    *out = ctx->primed.events[ctx->primed.index++];
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
    if (!sp_prompt_poll_stdin()) {
      out->kind = SP_PROMPT_EVENT_ESCAPE;
      return true;
    }

    u8 seq[2] = {0};
    if (sp_sys_read(sp_sys_stdin, &seq[0], 1) <= 0) {
      out->kind = SP_PROMPT_EVENT_ESCAPE;
      return true;
    }

    if (sp_prompt_poll_stdin()) {
      if (sp_sys_read(sp_sys_stdin, &seq[1], 1) <= 0) {
        seq[1] = 0;
      }
    }

    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': out->kind = SP_PROMPT_EVENT_UP; return true;
        case 'B': out->kind = SP_PROMPT_EVENT_DOWN; return true;
        case 'C': out->kind = SP_PROMPT_EVENT_RIGHT; return true;
        case 'D': out->kind = SP_PROMPT_EVENT_LEFT; return true;
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

static u32 sp_prompt_num_trimmed_cols(sp_prompt_cell_t* cells, u32 cols) {
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
    case SP_PROMPT_STYLE_NONE: return true;
    case SP_PROMPT_STYLE_ANSI: return left.ansi == right.ansi;
    case SP_PROMPT_STYLE_RGB: return left.rgb.r == right.rgb.r && left.rgb.g == right.rgb.g && left.rgb.b == right.rgb.b;
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
      sp_str_t esc = sp_fmt("\x1b[{}m", sp_fmt_uint(style.ansi));
      sp_prompt_emit_str(ctx, esc);
      break;
    }
    case SP_PROMPT_STYLE_RGB: {
      sp_str_t esc = sp_fmt("\x1b[38;2;{};{};{}m", sp_fmt_uint(style.rgb.r), sp_fmt_uint(style.rgb.g), sp_fmt_uint(style.rgb.b));
      sp_prompt_emit_str(ctx, esc);
      break;
    }
  }
}

static void sp_prompt_write_row_cells(sp_prompt_ctx_t* ctx, sp_prompt_cell_t* cells, u32 cols) {
  u32 trim = sp_prompt_num_trimmed_cols(cells, cols);
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
  ctx->value = sp_zero_struct(sp_prompt_value_t);
  sp_context_push_arena(ctx->arena);

  sp_prompt_event_t event = { .kind = SP_PROMPT_EVENT_INIT };

  if (!sp_da_empty(ctx->frames)) {
    sp_prompt_emit(ctx, "\r\n│\r\n");
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
      .cells = sp_alloc_n(sp_prompt_cell_t, cell_count),
    };

    sp_mem_copy(ctx->framebuffer, frame.cells, sizeof(sp_prompt_cell_t) * cell_count);
    sp_da_push(ctx->frames, frame);
  }

  sp_context_pop();
  return ctx->state == SP_PROMPT_STATE_SUBMIT;
}

u32 sp_prompt_text_width(sp_str_t text) {
  u32 width = 0;
  sp_str_for_utf8(text, it) {
    SP_UNUSED(it);
    width++;
  }
  return width;
}

sp_str_t sp_prompt_repeat(u32 codepoint, u32 count) {
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
  sp_prompt_line(ctx, sp_fmt("┌  {}", sp_fmt_str(prompt->text)));
}

static void sp_prompt_outro_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_outro_t* prompt = (sp_prompt_outro_t*)ctx->user_data;
  sp_prompt_line(ctx, sp_fmt("└  {}", sp_fmt_str(prompt->text)));
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

  sp_prompt_line(ctx, sp_fmt("◇  {} {}╮", sp_fmt_str(prompt->title), sp_fmt_str(top_tail)));
  sp_prompt_line(ctx, sp_fmt("│  {}│", sp_fmt_str(spacer)));

  sp_da_for(message_lines, it) {
    sp_str_t line = message_lines[it];
    u32 line_width = sp_prompt_text_width(line);
    sp_str_t pad = sp_prompt_repeat(' ', width - line_width);
    sp_prompt_line(ctx, sp_fmt("│  {}{}│", sp_fmt_str(line), sp_fmt_str(pad)));
  }

  sp_prompt_line(ctx, sp_fmt("│  {}│", sp_fmt_str(spacer)));
  sp_prompt_line(ctx, sp_fmt("├{}╯", sp_fmt_str(bottom)));
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
  sp_prompt_render_line(ctx, sp_fmt("{}  ", sp_fmt_str(sp_prompt_repeat(prompt->symbol, 1))), style);
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
    *max_items = 8;
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

bool sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt) {
  return sp_prompt_run(ctx, sp_prompt_select_widget(&prompt));
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
