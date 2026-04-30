/*
                                                                                        █████       █████
                                                                                      ▒▒███       ▒▒███
    █████  ████████            ████████  ████████   ██████  █████████████   ████████  ███████      ▒███████
   ███▒▒  ▒▒███▒▒███          ▒▒███▒▒███▒▒███▒▒███ ███▒▒███▒▒███▒▒███▒▒███ ▒▒███▒▒███▒▒▒███▒       ▒███▒▒███
  ▒▒█████  ▒███ ▒███           ▒███ ▒███ ▒███ ▒▒▒ ▒███ ▒███ ▒███ ▒███ ▒███  ▒███ ▒███  ▒███        ▒███ ▒███
   ▒▒▒▒███ ▒███ ▒███           ▒███ ▒███ ▒███     ▒███ ▒███ ▒███ ▒███ ▒███  ▒███ ▒███  ▒███ ███    ▒███ ▒███
   ██████  ▒███████  █████████ ▒███████  █████    ▒▒██████  █████▒███ █████ ▒███████   ▒▒█████  ██ ████ █████
  ▒▒▒▒▒▒   ▒███▒▒▒  ▒▒▒▒▒▒▒▒▒  ▒███▒▒▒  ▒▒▒▒▒      ▒▒▒▒▒▒  ▒▒▒▒▒ ▒▒▒ ▒▒▒▒▒  ▒███▒▒▒     ▒▒▒▒▒  ▒▒ ▒▒▒▒ ▒▒▒▒▒
           ▒███                ▒███                                         ▒███
           █████               █████                                        █████
          ▒▒▒▒▒               ▒▒▒▒▒                                        ▒▒▒▒▒

  >> sp_prompt.h
  beautiful, interactive, zero-dependency utf-8 prompts for native CLIs

  ## TL;DR
  If you don't want to read this documentation, grep for the following tags to jump to code. If you prefer
  examples, see example/prompt.c in the sp.h repo for sample code of every widget.

  widgets
    output:
      intro
      outro
      note
      cancel
      info
      warn
      error
      success
    input:
      text
      password
    choices:
      confirm
      select
      multiselect
    dynamic:
      spinner
      progress
      knight_rider (it's just a fancy spinner)

  types
    @values      the values produced by prompts
    @style       colors and ANSI codes applied to cells in prompt output
    @event       events available to be handled by widgets
    @context     the handle into the library; mostly opaque
    @widgets     userdata types for builtin widgets

  functions
    @lifecycle   opening and closing a context
    @widgets     public API for using builtin widgets
    @values      the values produced by prompts
    @custom      helpful functions for writing your own widgets
    @advanced    what it says on the tin


  ##########
  ## USAGE #
  ##########
  Define the following before you include sp_prompt.h in exactly one C or C++ file:

    #define SP_IMPLEMENTATION

  sp_prompt.h is an extension to sp.h; make sure that sp.h is also on your include path.


  ### INITALIZATION
  To use the library, you need (a) a context, and (b) to set up the terminal for drawing. In the common
  case, you can do both with one call:

    sp_prompt_ctx_t* ctx = sp_prompt_begin();

  This detects the size of the terminal, saves the current terminal state, and enters raw mode. If you
  need more control over the order of these operations, or want a custom output size, do this:

    sp_prompt_ctx_t ctx = sp_zero(); // Zero initialization is required
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

  sp_prompt uses a memory arena instead of forcing you to manage pointless, small allocations. The above function
  frees every byte allocated. If you need data from the prompts to persist, copy it.


  ####################
  ## BUILTIN WIDGETS #
  ####################
  ### OUTPUT
  The most basic widget outputs some text. These are self explanatory:

    void sp_prompt_intro(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_outro(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_note(sp_prompt_ctx_t* ctx, const c8* text, const c8* title);
    void sp_prompt_cancel(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_info(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_warn(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_error(sp_prompt_ctx_t* ctx, const c8* text);
    void sp_prompt_success(sp_prompt_ctx_t* ctx, const c8* text);


  ### TEXT INPUT
  Also very basic. Blocks waiting for input with poll().

    const c8* sp_prompt_text(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);
    const c8* sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);

  They return the entered text, which is allocated with the context's arena and therefore has the same
  lifetime. If you want a persistent copy, make a copy.


  ### SELECTION
  These widgets let the user filter, toggle, and select choices. There are three widgets:

    bool sp_prompt_confirm(sp_prompt_ctx_t* ctx, const c8* prompt, bool initial);
    bool sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt);
    void sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t prompt);

  The confirm widget isn't very interesting. It's just a yes/no toggle. The other two widgets have quite a
  few options and features, which are configured via the second argument struct.

    #### .options
    .options is the list of options. Each option has the following fields:
      - .label is how the option is displayed
      - .hint is a muted hint next to the label
      - .selected is the default selection state of the option

    #### .num_options
    .num_options is the length of the options array. There are better tricks to solve this problem in C APIs,
    but explicitly specifying a length is the best among them here. It keeps the ABI stable and simple, and
    doesn't expose any footguns. Sorry.

    #### .max_visible
    .max_visible is the number of options visible before we truncate. For example, if you passed 4, then only
    four options will show at any given time. This works with scrolling.

    ◆  Pick a letter
    │  ○  A
    │  ●  B
    │  ○  C
    │  ○  D
    │  ...
    └

    #### .filter
    .filter enables type-to-fuzzy-filter over options

  Finally, there are two flavors: select and multiselect. They're the same, except the former is for picking
  1-of-N and the latter lets you toggle selections and pick M-from-N.


  ### SPINNER
  Spinners are different than the prompts we've seen up to now. Whereas everything else has been
  interactive, waiting for user input before rendering a new frame, a spinner needs to update
  many times per second no matter what.

  Despite the difference, the API for spinners is pretty much identical to that of blocking
  widgets, for both users and authors of widgets. The only difference is in how they terminate;
  a blocking widget waits for some input, like Enter.

  A spinner waits for some Work to finish. It doesn't matter what it is; the spinner just wants
  to know whether it completed. And that's exactly the API we provide. If you're using a spinner,
  then you must have another thread doing the Work. On that thread, do one of:

    sp_prompt_complete(ctx);  // success: spinner submits
    sp_prompt_abort(ctx);     // failure: spinner cancels (e.g. file read errored)

  Both wake the prompt. From the other direction, when the user hits Ctrl-C (or whatever the
  widget treats as cancel), the prompt's lifecycle flips to CANCEL. The worker should poll
  sp_prompt_should_stop(ctx) inside its loop and bail out when it returns true. That covers
  both directions with one primitive: the prompt's state field is the rendezvous point, and
  any non-ACTIVE value means "everyone's done here."

  There are maybe 70 prebaked spinners; search for @spinner in the header and you'll find
  them. They're ripped from here, which has better visualizations:

    https://web.archive.org/web/20250520203924/https://antofthy.gitlab.io/info/ascii/Spinners.txt


  ### PROGRESS
  The progress bar updates dynamically, like the spinner. There is almost no difference between them internally,
  besides what they render. The same lifecycle (sp_prompt_complete/abort/should_stop) applies, plus
  two more APIs for controlling what the widget shows:

    sp_prompt_send_progress_f32(ctx, progress);
    sp_prompt_send_status(ctx, "status");

  The builtin progress bar wants an f32 in [0.0, 1.0] for its progress. If you'd like a different kind of progress,
  you're in luck, because we don't hardcode this. You can write your own progress widget which reads any of the
  following from the progress event:

    typedef union {
      u64   u;
      s64   i;
      f64   f;
      void* ptr;
      bool  b;
    } sp_prompt_event_data_t;


  ###################
  ## CUSTOM WIDGETS #
  ###################
  At its core, sp_prompt.h isn't much more than a loop like this:

  fn run(widget):
    while (!widget.done)
      if widget.on_update:
        // animated: poll non-blocking, tick on_update at fps
        if event := poll(0):
          widget.on_event(event)
        else if frame_elapsed():
          widget.on_update()
      else:
        // input-only: block in poll until input arrives, no fps wakeups
        event := poll(-1)
        widget.on_event(event)
      widget.render()

  A widget is simply an instance of sp_prompt_widget_t. Most widgets only fill in the fields they need:
    - on_event:  handler called when the user produces an event (key press, etc.)
    - on_update: handler called once per frame at the widget's fps; useful for animation
    - render:    writes lines of text to a buffer
    - user_data: pointer to widget state, retrievable from ctx during the handlers above
    - fps:       per-widget tick rate; if zero, the prompt picks a sensible default

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

#define SP_PROMPT_DEFAULT_COLS 80
#define SP_PROMPT_DEFAULT_ROWS 20
#define SP_PROMPT_PRIMED_EVENT_CAP 8
#define SP_PROMPT_CELL_BUFFER_BYTES 32
#define SP_PROMPT_BUFFER_EXTRA_BYTES 1024
#define SP_PROMPT_WAKE_DRAIN_SIZE 64
#define SP_PROMPT_DEFAULT_VISIBLE_OPTIONS 8
#define SP_PROMPT_DEFAULT_FPS 15

#define SP_PROMPT_KEY_CTRL_C 3
#define SP_PROMPT_KEY_BACKSPACE 8
#define SP_PROMPT_KEY_TAB 9
#define SP_PROMPT_KEY_LF 10
#define SP_PROMPT_KEY_CR 13
#define SP_PROMPT_KEY_ESCAPE 27
#define SP_PROMPT_KEY_DELETE 127

#define SP_PROMPT_UTF8_2_BYTE_MASK 0xE0
#define SP_PROMPT_UTF8_2_BYTE_PREFIX 0xC0
#define SP_PROMPT_UTF8_3_BYTE_MASK 0xF0
#define SP_PROMPT_UTF8_3_BYTE_PREFIX 0xE0
#define SP_PROMPT_UTF8_4_BYTE_MASK 0xF8
#define SP_PROMPT_UTF8_4_BYTE_PREFIX 0xF0
#define SP_PROMPT_UTF8_2_BYTE_LEN 2
#define SP_PROMPT_UTF8_3_BYTE_LEN 3
#define SP_PROMPT_UTF8_4_BYTE_LEN 4

//////////
// ANSI //
//////////
// Static ANSI sequences. The _FMT variants are sp_fmt_a(sp_context_get_allocator(), ).value templates ({} is the
// placeholder).
#define SP_ANSI_CURSOR_HOME       "\r"
#define SP_ANSI_CURSOR_UP         "\x1b[A"
#define SP_ANSI_CURSOR_UP_N_FMT   "\x1b[{}A"
#define SP_ANSI_NEWLINE           "\n"
#define SP_ANSI_ERASE_DISPLAY     "\x1b[J"
#define SP_ANSI_ERASE_LINE        "\x1b[2K"
#define SP_ANSI_SGR_RESET         "\x1b[0m"
#define SP_ANSI_SGR_ANSI_FMT      "\x1b[{}m"
#define SP_ANSI_SGR_RGB_FMT       "\x1b[38;2;{};{};{}m"
#define SP_ANSI_HIDE_CURSOR       "\x1b[?25l"
#define SP_ANSI_SHOW_CURSOR       "\x1b[?25h"
// DEC private mode 2026: synchronized output. Terminals that don't support it
// ignore the sequence, so the wrap is a safe no-op fallback.
#define SP_ANSI_BEGIN_SYNC        "\x1b[?2026h"
#define SP_ANSI_END_SYNC          "\x1b[?2026l"

/////////////
// CONTEXT //
/////////////
// @context (jump to the next match for the implementation)
typedef struct sp_prompt_ctx_t sp_prompt_ctx_t;
typedef struct sp_prompt_event_t sp_prompt_event_t;


/////////////
// WIDGETS //
/////////////
// @widgets
typedef void (*sp_prompt_event_fn)(sp_prompt_ctx_t* ctx, sp_prompt_event_t event);
typedef void (*sp_prompt_update_fn)(sp_prompt_ctx_t* ctx);
typedef void (*sp_prompt_render_fn)(sp_prompt_ctx_t* ctx);

typedef struct {
  void* user_data;
  sp_prompt_event_fn on_event;
  sp_prompt_update_fn on_update;
  sp_prompt_render_fn render;
  u32 fps;
} sp_prompt_widget_t;

///////////
// TEXT //
//////////
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
} sp_prompt_text_t;

typedef struct {
  sp_str_t text;
} sp_prompt_intro_t;

typedef struct {
  sp_str_t text;
} sp_prompt_outro_t;

typedef struct {
  sp_str_t prompt;
  sp_str_t prefill;
  bool mask;
} sp_prompt_password_t;

///////////////
// SELECTION //
///////////////
typedef struct {
  const c8* prompt;
  bool initial;
} sp_prompt_confirm_t;

typedef struct {
  const c8* label;
  const c8* hint;
  bool selected;
} sp_prompt_select_option_t;

typedef struct {
  const c8* prompt;
  sp_prompt_select_option_t* options;
  u32 num_options;
  u32 max_visible;
  bool filter;
} sp_prompt_select_t;

typedef struct {
  const c8* prompt;
  sp_prompt_select_option_t* options;
  u32 num_options;
  u32 max_visible;
  bool filter;
} sp_prompt_multiselect_t;


///////////
// STYLE //
///////////
// @style
typedef enum {
  SP_PROMPT_STYLE_NONE,
  SP_PROMPT_STYLE_ANSI,
  SP_PROMPT_STYLE_RGB,
} sp_prompt_style_kind_t;

struct sp_prompt_style_t {
  sp_prompt_style_kind_t tag;
  union {
    struct {
      u8 r;
      u8 g;
      u8 b;
    } rgb;
    u8 ansi;
  };
};

//////////////
// SPINNERS //
//////////////
// @spinner
#define SP_PROMPT_SPINNER_MAX_FRAMES 64
#define SP_PROMPT_SPINNER_DEFAULT_FPS 12

typedef struct sp_prompt_style_t sp_prompt_style_t;
typedef void (*sp_prompt_spinner_symbol_fn) (sp_prompt_ctx_t* ctx, u32 frame_index, u32* codepoint);
typedef void (*sp_prompt_spinner_color_fn)  (sp_prompt_ctx_t* ctx, u32 frame_index, sp_prompt_style_t* style);

typedef struct {
  const c8* prompt;
  u32 fps;
  struct {
    u32 frames [SP_PROMPT_SPINNER_MAX_FRAMES];
    struct {
      u8 ansi;
      struct { u8 r, g, b; } rgb;
      sp_prompt_spinner_color_fn fn;
    } color;
  };
} sp_prompt_spinner_t;

// Prebaked spinner frames; use them like this:
//
//   sp_prompt_spinner(ctx, (sp_prompt_spinner_t) {
//     .symbol.frames = SP_PROMPT_SPINNER_PACMAN_MUNCHER
//   })
//
#define SP_PROMPT_SPINNER_SPINNING_LINE { 0x002D, 0x005C, 0x007C, 0x002F }
#define SP_PROMPT_SPINNER_PULSING_CIRCLE { 0x002E, 0x006F, 0x004F, 0x006F }
#define SP_PROMPT_SPINNER_BALLOON { 0x0020, 0x002E, 0x006F, 0x004F, 0x0040, 0x002A }
#define SP_PROMPT_SPINNER_FLIP_LINE { 0x005F, 0x005F, 0x005C, 0x007C, 0x002F, 0x005F, 0x005F }
#define SP_PROMPT_SPINNER_FLIP_LINE_PATROL { 0x005F, 0x005F, 0x005C, 0x007C, 0x002F, 0x005F, 0x005F, 0x005F, 0x005F, 0x002F, 0x007C, 0x005C, 0x005F, 0x005F }
#define SP_PROMPT_SPINNER_PACMAN_MUNCHER { 0x002D, 0x003E, 0x007C, 0x003E, 0x002D } // Eh? I hardly know her...
#define SP_PROMPT_SPINNER_PACMAN_MUNCHER_RIGHT { 0x002D, 0x227A, 0x2039, 0x27E8, 0x2039, 0x227A, 0x002D }
#define SP_PROMPT_SPINNER_PACMAN_MUNCHER_2 { 0x002D, 0x227B, 0x203A, 0x27E9, 0x203A, 0x227B, 0x002D }
#define SP_PROMPT_SPINNER_FOLDING_OVER { 0x002D, 0x003C, 0x007C, 0x003E, 0x002D }
#define SP_PROMPT_SPINNER_FOLDING_OVER_PATROL { 0x002D, 0x003E, 0x007C, 0x003C, 0x002D, 0x002D, 0x003C, 0x007C, 0x003E, 0x002D }
#define SP_PROMPT_SPINNER_BAT_AND_BALL { 0x0064, 0x0071, 0x0070, 0x0062 }
#define SP_PROMPT_SPINNER_UNICODE_SPINNING_LINE { 0x2500, 0x2572, 0x2502, 0x2571 }
#define SP_PROMPT_SPINNER_FLIP_LINE_2 { 0x005F, 0x002D, 0x0060, 0x0027, 0x00B4, 0x002D, 0x005F }
#define SP_PROMPT_SPINNER_FOLDING_OVER_LEFT_RIGHT { 0x002D, 0x227B, 0x203A, 0x27E9, 0x007C, 0x27E8, 0x2039, 0x227A }
#define SP_PROMPT_SPINNER_FOLDING_OVER_RIGHT_LEFT { 0x002D, 0x227A, 0x2039, 0x27E8, 0x007C, 0x27E9, 0x203A, 0x227B }
#define SP_PROMPT_SPINNER_QUARTER_CIRCLE_SPINNER { 0x25DF, 0x25DC, 0x25DD, 0x25DE }
#define SP_PROMPT_SPINNER_EXTENDED_CIRCLE_SPINNER { 0x25DC, 0x25E0, 0x25DD, 0x25DE, 0x25E1, 0x25DF }
#define SP_PROMPT_SPINNER_CORNER_CIRCLE_SPINNER { 0x25F4, 0x25F7, 0x25F6, 0x25F5 }
#define SP_PROMPT_SPINNER_HALF_CIRCLE_SPINNER { 0x25D0, 0x25D3, 0x25D1, 0x25D2 }
#define SP_PROMPT_SPINNER_HALF_CIRCLE_FLIP { 0x25E1, 0x2299, 0x25E0, 0x2299 }
#define SP_PROMPT_SPINNER_ELLIPSES { 0x0020, 0x2024, 0x2025, 0x2026 }
#define SP_PROMPT_SPINNER_CIRCLE_TAIL_B { 0x0062, 0x14C2, 0x0071, 0x14C4 }
#define SP_PROMPT_SPINNER_CIRCLE_TAIL_D { 0x0064, 0x14C7, 0x0070, 0x14C0 }
#define SP_PROMPT_SPINNER_BOUNCING_BALL { 0x002E, 0x006F, 0x004F, 0x00B0, 0x004F, 0x006F, 0x002E }
#define SP_PROMPT_SPINNER_RAIN_CIRCLE { 0x0020, 0x22C5, 0x2218, 0x25CB, 0xE00F, 0x2A00 }
#define SP_PROMPT_SPINNER_PULSING_CIRCLE_2 { 0x25CC, 0x25CB, 0x2299, 0x25CF, 0x2299, 0x25CB }
#define SP_PROMPT_SPINNER_PULSING_DOT { 0x22C5, 0x2219, 0x25CF, 0x2219 }
#define SP_PROMPT_SPINNER_PULSING_HEART { 0x22C5, 0x2022, 0x2764, 0x2022, 0x22C5, 0x22C5, 0x22C5 }
#define SP_PROMPT_SPINNER_PULSING_LINE { 0x2758, 0x2759, 0x275A, 0x2759 }
#define SP_PROMPT_SPINNER_PULSING_LINE_HEART { 0x2758, 0x2759, 0x275A, 0x2764, 0x275A, 0x2759, 0x2758 }
#define SP_PROMPT_SPINNER_PULSING_SQUARE { 0x25AA, 0x25FC, 0x2588, 0x25FC, 0x25AA }
#define SP_PROMPT_SPINNER_PULSING_SQUARE_2 { 0x25AA, 0x25A0, 0x25A1, 0x25AB }
#define SP_PROMPT_SPINNER_CORNERS { 0x231E, 0x231C, 0x231D, 0x231F }
#define SP_PROMPT_SPINNER_LARGE_CORNERS { 0x23BF, 0x23BE, 0x23CB, 0x23CC }
#define SP_PROMPT_SPINNER_CORNER_BOX { 0x25F0, 0x25F3, 0x25F2, 0x25F1 }
#define SP_PROMPT_SPINNER_TRIANGLE { 0x25E3, 0x25E4, 0x25E5, 0x25E2 }
#define SP_PROMPT_SPINNER_MOVING_HORIZONTAL_BAR { 0x23BD, 0x23BC, 0x2015, 0x23BB, 0x23BA, 0x0020 }
#define SP_PROMPT_SPINNER_MOVING_HORIZONTAL_BAR_PATROL { 0x23BD, 0x23BC, 0x2015, 0x23BB, 0x23BA, 0x23BB, 0x2015, 0x23BC }
#define SP_PROMPT_SPINNER_MOVING_VERTICAL_BAR { 0x258F, 0x23A2, 0x23AA, 0x23A5, 0x2595, 0x0020 }
#define SP_PROMPT_SPINNER_MOVING_VERTICAL_BAR_PATROL { 0x258F, 0x23A2, 0x23AA, 0x23A5, 0x2595, 0x23A5, 0x23AA, 0x23A2 }
#define SP_PROMPT_SPINNER_TONE_BAR { 0x02E9, 0x02E8, 0x02E7, 0x02E6, 0x02E5 }
#define SP_PROMPT_SPINNER_TONE_BAR_PATROL { 0x02E9, 0x02E8, 0x02E7, 0x02E6, 0x02E5, 0x02E6, 0x02E7, 0x02E8 }
#define SP_PROMPT_SPINNER_RISING_VERTICAL_BAR { 0x02CC, 0x2577, 0x2758, 0xFE8D, 0x2575, 0x02C8 }
#define SP_PROMPT_SPINNER_GROWTH_AND_DECAY { 0x2024, 0x007C, 0x2020, 0x00A5, 0x2228, 0x2304 }
#define SP_PROMPT_SPINNER_PULSING_STAR { 0x22C5, 0x02D6, 0x002B, 0x27E1, 0x2727, 0x27E1, 0x002B, 0x02D6 }
#define SP_PROMPT_SPINNER_PULSING_PLUS { 0x22C5, 0x02D6, 0x002B, 0x253C, 0x254B, 0x253C, 0x002B, 0x02D6 }
#define SP_PROMPT_SPINNER_BOUNCING_BUBBLE { 0x0027, 0x00B0, 0x00BA, 0x00A4, 0x00F8, 0x002C, 0x00B8, 0x00B8, 0x002C, 0x00F8, 0x00A4, 0x00BA, 0x00B0, 0x0027 }
#define SP_PROMPT_SPINNER_MATHEMATICAL_EQUALS { 0x2212, 0x003D, 0x2261 }
#define SP_PROMPT_SPINNER_MATHEMATICAL_COMPARES { 0x2251, 0x2252, 0x2251, 0x2253 }
#define SP_PROMPT_SPINNER_I_CHING { 0x2631, 0x2632, 0x2634 }
#define SP_PROMPT_SPINNER_ARROW { 0x2190, 0x2196, 0x2191, 0x2197, 0x2192, 0x2198, 0x2193, 0x2199 }
#define SP_PROMPT_SPINNER_DINGBAT_ARROW { 0x27B5, 0x27B4, 0x27B5, 0x27B6 }
#define SP_PROMPT_SPINNER_DIBBAT_BOLD_ARROWS { 0x27B8, 0x27B7, 0x27B8, 0x27B9 }
#define SP_PROMPT_SPINNER_SQUISH { 0x256B, 0x256A }
#define SP_PROMPT_SPINNER_TOGGLE_LINE { 0x22B6, 0x22B7 }
#define SP_PROMPT_SPINNER_TOGGLE_SM_BOX { 0x25AB, 0x25AA }
#define SP_PROMPT_SPINNER_TOGGLE_LG_BOX { 0x25A1, 0x25A0 }
#define SP_PROMPT_SPINNER_TOGGLE_SHOGI { 0x2616, 0x2617 }
#define SP_PROMPT_SPINNER_BRAILLE_CIRCLING_DOT { 0x2840, 0x2804, 0x2802, 0x2801, 0x2808, 0x2810, 0x2820, 0x2880 }
#define SP_PROMPT_SPINNER_BRAILLE_CIRCLING_DOT_DB { 0x28C0, 0x2844, 0x2806, 0x2803, 0x2809, 0x2818, 0x2830, 0x28A0 }
#define SP_PROMPT_SPINNER_BRAILLE_CIRCLING_HOLE { 0x28BF, 0x28FB, 0x28FD, 0x28FE, 0x28F7, 0x28EF, 0x28DF, 0x287F }
#define SP_PROMPT_SPINNER_BRAILLE_CIRCLING_HOLE_DB { 0x28F6, 0x28E7, 0x28CF, 0x285F, 0x283F, 0x28BB, 0x28F9, 0x28FC }
#define SP_PROMPT_SPINNER_BRAILLE_TWIN_CIRCLING_DOTS { 0x2848, 0x2814, 0x2822, 0x2881 }
#define SP_PROMPT_SPINNER_BRAILLE_LEAPFROG { 0x28C0, 0x2884, 0x2882, 0x2881, 0x2848, 0x2850, 0x2860 }
#define SP_PROMPT_SPINNER_BRAILLE_BOUNCING_DOT { 0x2840, 0x2804, 0x2802, 0x2801, 0x2801, 0x2802, 0x2804 }
#define SP_PROMPT_SPINNER_BRAILLE_BOUNCE_SIDE_SIDE { 0x2840, 0x2804, 0x2802, 0x2801, 0x2808, 0x2810, 0x2820, 0x2880, 0x2820, 0x2810, 0x2808, 0x2801, 0x2802, 0x2804 }
#define SP_PROMPT_SPINNER_BRAILLE_BOUNCING_BALL { 0x28E4, 0x2836, 0x281B, 0x281B, 0x2836 }
#define SP_PROMPT_SPINNER_BRAILLE_CLIMBER { 0x28C0, 0x2860, 0x2824, 0x2822, 0x2812, 0x280A, 0x2809, 0x2811, 0x2812, 0x2814, 0x2824, 0x2884 }
#define SP_PROMPT_SPINNER_BRAILLE_COVEYER_BELT { 0x28B8, 0x28F8, 0x28BC, 0x28BA, 0x28B9, 0x284F, 0x2857, 0x2867, 0x28C7, 0x2847 }
#define SP_PROMPT_SPINNER_BRAILLE_6_CIRCLE_WORM { 0x280B, 0x2819, 0x2839, 0x2838, 0x283C, 0x2834, 0x2826, 0x2827, 0x2807, 0x280F }
#define SP_PROMPT_SPINNER_BRAILLE_6_BOUNCE_WORM { 0x2804, 0x2806, 0x2807, 0x280B, 0x2819, 0x2838, 0x2830, 0x2820, 0x2830, 0x2838, 0x2819, 0x280B, 0x2807, 0x2806 }
#define SP_PROMPT_SPINNER_BRAILLE_ZIGZAG_WORM { 0x280B, 0x2819, 0x281A, 0x281E, 0x2816, 0x2826, 0x2834, 0x2832, 0x2833, 0x2813 }
#define SP_PROMPT_SPINNER_BRAILLE_CIRCLE_WORM { 0x280B, 0x2819, 0x2839, 0x2838, 0x28B0, 0x28F0, 0x28E0, 0x28C4, 0x28C6, 0x2846, 0x2807, 0x280F }
#define SP_PROMPT_SPINNER_FALLING_SAND { 0x2801, 0x2802, 0x2804, 0x2840, 0x2848, 0x2850, 0x2860, 0x28C0, 0x28C1, 0x28C2, 0x28C4, 0x28CC, 0x28D4, 0x28E4, 0x28E5, 0x28E6, 0x28EE, 0x28F6, 0x28F7, 0x28FF, 0x287F, 0x283F, 0x289F, 0x281F, 0x285B, 0x281B, 0x282B, 0x288B, 0x280B, 0x280D, 0x2849, 0x2809, 0x2811, 0x2821, 0x2881, 0x2801, 0x2802, 0x2804, 0x2840, 0x2800 }


//////////////
// PROGRESS //
//////////////
// @progress
typedef struct {
  const c8* prompt;
  u32 width;
  struct {
    u8 ansi;
    struct { u8 r, g, b; } rgb;
  } color;
} sp_prompt_progress_t;


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
  SP_PROMPT_EVENT_PROGRESS,
  SP_PROMPT_EVENT_STATUS,
  SP_PROMPT_EVENT_ABORT,
} sp_prompt_event_kind_t;

typedef union {
  u64   u;
  s64   i;
  f64   f;
  void* ptr;
  bool  b;
} sp_prompt_event_data_t;

struct sp_prompt_event_t {
  sp_prompt_event_kind_t kind;
  union {
    struct {
      u32 codepoint;
    } input;
    struct {
      sp_prompt_event_data_t data;
    } progress;
    struct {
      sp_str_t value;
    } status;
  };
};

#define SP_PROMPT_KR_WIDTH         6
#define SP_PROMPT_KR_HOLD_START    30
#define SP_PROMPT_KR_HOLD_END      9
#define SP_PROMPT_KR_TRAIL_LEN     6
#define SP_PROMPT_KR_INTERVAL_MS   45
#define SP_PROMPT_KR_DEFAULT_FPS   15
#define SP_PROMPT_KR_MIN_SPEED     0.05f
#define SP_PROMPT_KR_LEAD_ALPHA    1.0f
#define SP_PROMPT_KR_LEAD_BRIGHTNESS 1.0f
#define SP_PROMPT_KR_GLOW_ALPHA    0.9f
#define SP_PROMPT_KR_GLOW_BRIGHTNESS 1.15f
#define SP_PROMPT_KR_TRAIL_DECAY   0.65f
#define SP_PROMPT_KR_INACTIVE_ALPHA 0.2f

typedef struct {
  const c8* prompt;
  u32 fps;
  f32 speed;
  struct { u8 r, g, b; } color;
  u8 width;
  struct {
    u8 hold_start;
    u8 hold_end;
    u8 interval;
  } ex;
} sp_prompt_knight_rider_t;

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
const c8*        sp_prompt_text(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);
const c8*        sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill);
bool             sp_prompt_confirm(sp_prompt_ctx_t* ctx, const c8* prompt, bool initial);
bool             sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t prompt);
void             sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t prompt);
void             sp_prompt_spinner(sp_prompt_ctx_t* ctx, sp_prompt_spinner_t config);
void             sp_prompt_progress(sp_prompt_ctx_t* ctx, sp_prompt_progress_t config);
void             sp_prompt_knight_rider(sp_prompt_ctx_t* ctx, sp_prompt_knight_rider_t config);
bool             sp_prompt_submitted(sp_prompt_ctx_t* ctx);
bool             sp_prompt_cancelled(sp_prompt_ctx_t* ctx);

// @signals
void             sp_prompt_complete(sp_prompt_ctx_t* ctx);
void             sp_prompt_abort(sp_prompt_ctx_t* ctx);
bool             sp_prompt_is_aborted(sp_prompt_ctx_t* ctx);
void             sp_prompt_send_progress_f32(sp_prompt_ctx_t* ctx, f32 value);
void             sp_prompt_send_progress_f64(sp_prompt_ctx_t* ctx, f64 value);
void             sp_prompt_send_progress_u32(sp_prompt_ctx_t* ctx, u32 value);
void             sp_prompt_send_progress_u64(sp_prompt_ctx_t* ctx, u64 value);
void             sp_prompt_send_progress_s32(sp_prompt_ctx_t* ctx, s32 value);
void             sp_prompt_send_progress_s64(sp_prompt_ctx_t* ctx, s64 value);
void             sp_prompt_send_progress_ptr(sp_prompt_ctx_t* ctx, void* value);
void             sp_prompt_send_progress_bool(sp_prompt_ctx_t* ctx, bool value);
void             sp_prompt_send_status(sp_prompt_ctx_t* ctx, const c8* text);
void             sp_prompt_send_status_str(sp_prompt_ctx_t* ctx, sp_str_t text);

// @values
const c8*        sp_prompt_get_str(sp_prompt_ctx_t* ctx);
bool             sp_prompt_get_bool(sp_prompt_ctx_t* ctx);
void             sp_prompt_set_str(sp_prompt_ctx_t* ctx, sp_str_t value);
void             sp_prompt_set_bool(sp_prompt_ctx_t* ctx, bool value);
const c8*        sp_prompt_join_selection(sp_prompt_select_option_t* options, u32 num_options);
void             sp_prompt_set_state(sp_prompt_ctx_t* ctx, sp_prompt_state_t state);


// @custom
void             sp_prompt_line(sp_prompt_ctx_t* ctx, sp_str_t text);
void             sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style);
u32              sp_prompt_text_width(sp_str_t text);
sp_str_t         sp_prompt_repeat(u32 codepoint, u32 count);

// @advanced
sp_prompt_ctx_t* sp_prompt_new();
s32              sp_prompt_begin_ex(sp_prompt_ctx_t* ctx);
void             sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, u32 cols, u32 rows);
void             sp_prompt_prime_events(sp_prompt_ctx_t* ctx, sp_prompt_event_t events[SP_PROMPT_PRIMED_EVENT_CAP]);
bool             sp_prompt_run(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget);
sp_app_config_t  sp_prompt_app(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget);
#endif


#if defined(SP_PROMPT_IMPLEMENTATION)

typedef struct {
  u32 cursor;
  u32 visible_offset;
  sp_str_t filter_value;
} sp_prompt_choice_state_t;

typedef struct {
  sp_prompt_password_t config;
  sp_str_t value;
  bool mask;
} sp_prompt_password_widget_t;
sp_prompt_widget_t sp_prompt_password_widget(sp_prompt_ctx_t* ctx, sp_prompt_password_t config);

typedef struct {
  sp_prompt_spinner_t config;
  u32 frame_index;
} sp_prompt_spinner_widget_t;
sp_prompt_widget_t sp_prompt_spinner_widget(sp_prompt_ctx_t* ctx, sp_prompt_spinner_t config);

typedef struct {
  sp_prompt_progress_t config;
  f32 value;
  sp_str_t status;
} sp_prompt_progress_widget_t;
sp_prompt_widget_t sp_prompt_progress_widget(sp_prompt_ctx_t* ctx, sp_prompt_progress_t config);

typedef struct {
  sp_prompt_text_t config;
  sp_str_t value;
} sp_prompt_text_widget_t;
sp_prompt_widget_t sp_prompt_text_widget(sp_prompt_ctx_t* ctx, sp_prompt_text_t config);

typedef struct {
  sp_prompt_confirm_t config;
  bool value;
} sp_prompt_confirm_widget_t;
sp_prompt_widget_t sp_prompt_confirm_widget(sp_prompt_ctx_t* ctx, sp_prompt_confirm_t config);

typedef struct {
  sp_prompt_select_t config;
  sp_prompt_choice_state_t state;
} sp_prompt_select_widget_t;

sp_prompt_widget_t sp_prompt_select_widget(sp_prompt_ctx_t* ctx, sp_prompt_select_t config);

typedef struct {
  sp_prompt_multiselect_t config;
  sp_prompt_choice_state_t state;
} sp_prompt_multiselect_widget_t;
sp_prompt_widget_t sp_prompt_multiselect_widget(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t config);

typedef struct {
  sp_prompt_knight_rider_t config;
  sp_tm_timer_t timer;
  u64 elapsed_ns;
} sp_prompt_knight_rider_widget_t;
sp_prompt_widget_t sp_prompt_knight_rider_widget(sp_prompt_ctx_t* ctx, sp_prompt_knight_rider_t config);

sp_prompt_widget_t sp_prompt_intro_widget(sp_prompt_ctx_t* ctx, sp_prompt_intro_t config);
sp_prompt_widget_t sp_prompt_outro_widget(sp_prompt_ctx_t* ctx, sp_prompt_outro_t config);
sp_prompt_widget_t sp_prompt_note_widget(sp_prompt_ctx_t* ctx, sp_prompt_note_t config);
sp_prompt_widget_t sp_prompt_message_widget(sp_prompt_ctx_t* ctx, sp_prompt_message_t config);

/////////////
// CONTEXT //
/////////////
// @context
typedef struct {
  u32 codepoint;
  sp_prompt_style_t style;
} sp_prompt_cell_t;

typedef struct {
  u32 cols;
  u32 rows;
  sp_prompt_cell_t* cells;
} sp_prompt_frame_t;

struct sp_prompt_ctx_t {
  void* user_data;
  u32 cols;
  u32 rows;
  u32 cursor_row;
  u32 cursor_col;
  u32 prompt_height;
  sp_prompt_widget_t widget;
  sp_atomic_s32_t state;
  sp_prompt_value_t value;
  struct {
    sp_prompt_event_t events[SP_PROMPT_PRIMED_EVENT_CAP];
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
  sp_mem_t allocator;
  struct {
    sp_prompt_event_data_t value;
    bool dirty;
  } progress;
  struct {
    sp_str_t value;
    bool dirty;
  } status;
  struct {
    sp_mutex_t lock;
    sp_mem_arena_t* arena;
  } channel;
  struct {
    sp_sys_fd_t read;
    sp_sys_fd_t write;
    sp_atomic_s32_t pending;
  } wake;
};

#define SP_PROMPT_WAKE_NOT_PENDING 0
#define SP_PROMPT_WAKE_PENDING 1

SP_PRIVATE void sp_prompt_wake(sp_prompt_ctx_t* ctx);
SP_PRIVATE sp_prompt_event_t sp_prompt_drain_stdin(sp_prompt_ctx_t* ctx);
SP_PRIVATE void sp_prompt_dispatch_event(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event);


static s32 sp_prompt_enable_raw_mode(sp_prompt_ctx_t* ctx) {
  if (sp_os_tty_enter_raw(ctx->terminal.fds.in, &ctx->terminal.cache) == -1) return -1;
  ctx->terminal.raw = true;
  return 0;
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
  sp_prompt_emit(ctx, SP_ANSI_CURSOR_HOME);
}

static void sp_prompt_ansi_up(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, SP_ANSI_CURSOR_UP);
}

static void sp_prompt_ansi_new_line(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, SP_ANSI_NEWLINE);
}

static void sp_prompt_ansi_clear(sp_prompt_ctx_t* ctx) {
  sp_prompt_emit(ctx, SP_ANSI_ERASE_DISPLAY);
}

static void sp_prompt_framebuffer_clear(sp_prompt_ctx_t* ctx) {
  u32 cell_count = ctx->cols * ctx->rows;
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
  u32 cols = 0;
  u32 rows = 0;
  if (sp_os_is_tty(sp_sys_stdout)) {
    sp_os_tty_size(sp_sys_stdout, &cols, &rows);
  }
  if (cols == 0) cols = SP_PROMPT_DEFAULT_COLS;
  if (rows == 0) rows = SP_PROMPT_DEFAULT_ROWS;
  sp_prompt_ctx_init(ctx, cols, rows);
  return ctx;
}

sp_prompt_ctx_t* sp_prompt_begin() {
  sp_prompt_ctx_t* ctx = sp_prompt_new();
  if (sp_prompt_begin_ex(ctx)) {
    sp_free(ctx);
    return SP_NULLPTR;
  }
  return ctx;
}

s32 sp_prompt_begin_ex(sp_prompt_ctx_t* ctx) {
  ctx->terminal.fds.in = sp_sys_stdin;
  ctx->terminal.fds.out = sp_sys_stdout;
  ctx->terminal.raw = false;

  if (sp_prompt_enable_raw_mode(ctx) == -1) return -1;
  sp_sys_pipe(&ctx->wake.read, &ctx->wake.write);
  sp_prompt_emit(ctx, SP_ANSI_HIDE_CURSOR);
  sp_io_flush(ctx->writer);
  return 0;
}

void sp_prompt_end(sp_prompt_ctx_t* ctx) {
  if (ctx->terminal.raw) {
    sp_prompt_emit(ctx, SP_ANSI_SHOW_CURSOR);
    sp_io_flush(ctx->writer);
    sp_os_tty_restore(ctx->terminal.fds.in, &ctx->terminal.cache);
    ctx->terminal.raw = false;
  }

  if (ctx->wake.read != SP_SYS_INVALID_FD) {
    sp_sys_close(ctx->wake.read);
    ctx->wake.read = SP_SYS_INVALID_FD;
  }
  if (ctx->wake.write != SP_SYS_INVALID_FD) {
    sp_sys_close(ctx->wake.write);
    ctx->wake.write = SP_SYS_INVALID_FD;
  }

  sp_sys_write(ctx->terminal.fds.out, "\n", 1);
  sp_mutex_destroy(&ctx->channel.lock);
  sp_mem_arena_destroy(ctx->channel.arena);
  sp_mem_arena_destroy(ctx->arena);
}

void sp_prompt_ctx_init(sp_prompt_ctx_t* ctx, u32 cols, u32 rows) {
  *ctx = sp_zero_struct(sp_prompt_ctx_t);
  ctx->cols = cols;
  ctx->rows = rows;
  ctx->state = SP_PROMPT_STATE_ACTIVE;
  ctx->wake.read = SP_SYS_INVALID_FD;
  ctx->wake.write = SP_SYS_INVALID_FD;
  ctx->arena = sp_mem_arena_new();
  ctx->allocator = sp_mem_arena_as_allocator(ctx->arena);
  sp_da_init(ctx->allocator, ctx->frames);

  sp_mutex_init(&ctx->channel.lock, SP_MUTEX_PLAIN);
  ctx->channel.arena = sp_mem_arena_new_with_allocator(sp_mem_os_new(), SP_MEM_ARENA_BLOCK_SIZE, SP_MEM_ARENA_MODE_DEFAULT, SP_MEM_ALIGNMENT);

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
  ctx->writer = sp_mem_arena_alloc_type(ctx->arena, sp_io_writer_t);
  sp_io_get_std_out(ctx->writer);

  u64 buffer_size = ctx->cols * ctx->rows * SP_PROMPT_CELL_BUFFER_BYTES + SP_PROMPT_BUFFER_EXTRA_BYTES;
  u8* buffer = sp_mem_arena_alloc_n(ctx->arena, u8, buffer_size);
  sp_io_writer_set_buffer(ctx->writer, buffer, buffer_size);

  u32 cell_count = ctx->cols * ctx->rows;
  if (ctx->framebuffer == SP_NULLPTR) {
    ctx->framebuffer = sp_mem_arena_alloc_n(ctx->arena, sp_prompt_cell_t, cell_count);
  }
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

const c8* sp_prompt_join_selection(sp_prompt_select_option_t* options, u32 num_options) {
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  bool first = true;
  sp_for(it, num_options) {
    if (!options[it].selected) {
      continue;
    }

    if (!first) {
      sp_io_write_str(&builder, SP_LIT(", "), SP_NULLPTR);
    }
    first = false;
    sp_io_write_str(&builder, sp_str_view(options[it].label), SP_NULLPTR);
  }

  return sp_str_to_cstr(sp_io_writer_dyn_mem_as_str(&builder.dyn_mem));
}

// Instead of just writing to the file descriptor that the main loop waits on for events,
// deduplicate them so we don't fill up the pipe with useless bytes and risk deadlocking
// if for some reason we're emitting progress extremely fast.
void sp_prompt_wake(sp_prompt_ctx_t* ctx) {
  if (sp_atomic_s32_cas(&ctx->wake.pending, SP_PROMPT_WAKE_NOT_PENDING, SP_PROMPT_WAKE_PENDING)) {
    u8 byte = 0;
    sp_sys_write(ctx->wake.write, &byte, 1);
  }
}

void sp_prompt_set_state(sp_prompt_ctx_t* ctx, sp_prompt_state_t state) {
  if (!sp_atomic_s32_cas(&ctx->state, SP_PROMPT_STATE_ACTIVE, (s32)state)) {
    return;
  }
  sp_prompt_wake(ctx);
}

void sp_prompt_complete(sp_prompt_ctx_t* ctx) {
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
}

void sp_prompt_abort(sp_prompt_ctx_t* ctx) {
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
}

bool sp_prompt_is_aborted(sp_prompt_ctx_t* ctx) {
  return sp_atomic_s32_get(&ctx->state) != SP_PROMPT_STATE_ACTIVE;
}

static void sp_prompt_send_progress(sp_prompt_ctx_t* ctx, sp_prompt_event_data_t data) {
  sp_mutex_lock(&ctx->channel.lock);
  ctx->progress.value = data;
  ctx->progress.dirty = true;
  sp_mutex_unlock(&ctx->channel.lock);
  sp_prompt_wake(ctx);
}

void sp_prompt_send_progress_f32(sp_prompt_ctx_t* ctx, f32 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .f = (f64)value });
}

void sp_prompt_send_progress_f64(sp_prompt_ctx_t* ctx, f64 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .f = value });
}

void sp_prompt_send_progress_u32(sp_prompt_ctx_t* ctx, u32 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .u = (u64)value });
}

void sp_prompt_send_progress_u64(sp_prompt_ctx_t* ctx, u64 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .u = value });
}

void sp_prompt_send_progress_s32(sp_prompt_ctx_t* ctx, s32 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .i = (s64)value });
}

void sp_prompt_send_progress_s64(sp_prompt_ctx_t* ctx, s64 value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .i = value });
}

void sp_prompt_send_progress_ptr(sp_prompt_ctx_t* ctx, void* value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .ptr = value });
}

void sp_prompt_send_progress_bool(sp_prompt_ctx_t* ctx, bool value) {
  sp_prompt_send_progress(ctx, (sp_prompt_event_data_t) { .b = value });
}

void sp_prompt_send_status_str(sp_prompt_ctx_t* ctx, sp_str_t text) {
  sp_mutex_lock(&ctx->channel.lock);
  if (sp_str_equal(ctx->status.value, text)) {
    sp_mutex_unlock(&ctx->channel.lock);
    return;
  }

  sp_context_push_arena(ctx->channel.arena);
  ctx->status.value = sp_str_copy(text);
  sp_context_pop();
  ctx->status.dirty = true;

  sp_mutex_unlock(&ctx->channel.lock);
  sp_prompt_wake(ctx);
}

void sp_prompt_send_status(sp_prompt_ctx_t* ctx, const c8* text) {
  sp_prompt_send_status_str(ctx, sp_str_view(text));
}

bool sp_prompt_submitted(sp_prompt_ctx_t* ctx) {
  return ctx->state == SP_PROMPT_STATE_SUBMIT;
}

bool sp_prompt_cancelled(sp_prompt_ctx_t* ctx) {
  return ctx->state == SP_PROMPT_STATE_CANCEL;
}

void sp_prompt_prime_events(sp_prompt_ctx_t* ctx, sp_prompt_event_t events[SP_PROMPT_PRIMED_EVENT_CAP]) {
  ctx->primed.count = 0;
  ctx->primed.index = 0;

  sp_for(it, SP_PROMPT_PRIMED_EVENT_CAP) {
    if (events[it].kind == SP_PROMPT_EVENT_NONE) {
      break;
    }

    ctx->primed.events[ctx->primed.count] = events[it];
    ctx->primed.count++;
  }
}

void sp_prompt_render_line(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style) {
  if (ctx->cursor_row >= ctx->rows) {
    return;
  }

  sp_str_for_utf8(text, it) {
    if (ctx->cursor_col >= ctx->cols) {
      break;
    }

    u32 index = (ctx->cursor_row * ctx->cols) + ctx->cursor_col;
    ctx->framebuffer[index].codepoint = it.codepoint;
    ctx->framebuffer[index].style = style;
    ctx->cursor_col++;
  }
}

void sp_prompt_line(sp_prompt_ctx_t* ctx, sp_str_t text) {
  sp_prompt_render_line(ctx, text, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

void sp_prompt_dispatch_event(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget, sp_prompt_event_t event) {
  widget.on_event(ctx, event);
}

static void sp_prompt_render_frame(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  sp_prompt_framebuffer_clear(ctx);
  if (widget.render) {
    widget.render(ctx);
  }
}

static bool sp_prompt_poll_stdin(sp_prompt_ctx_t* ctx) {
  u8 ready = 0;
  return sp_sys_fd_ready(ctx->terminal.fds.in, &ready) == 0 && ready;
}

sp_prompt_event_t sp_prompt_drain_stdin(sp_prompt_ctx_t* ctx) {
  sp_prompt_event_t event = { .kind = SP_PROMPT_EVENT_NONE };

  if (!sp_prompt_poll_stdin(ctx)) {
    return event;
  }

  u8 c = 0;
  s64 nread = sp_sys_read(sp_sys_stdin, &c, 1);
  if (nread <= 0) {
    return event;
  }

  switch (c) {
    case SP_PROMPT_KEY_CTRL_C: event.kind = SP_PROMPT_EVENT_CTRL_C; return event;
    case SP_PROMPT_KEY_TAB: event.kind = SP_PROMPT_EVENT_TAB; return event;
    case SP_PROMPT_KEY_LF:
    case SP_PROMPT_KEY_CR: event.kind = SP_PROMPT_EVENT_ENTER; return event;
    case SP_PROMPT_KEY_BACKSPACE:
    case SP_PROMPT_KEY_DELETE: event.kind = SP_PROMPT_EVENT_BACKSPACE; return event;
    case SP_PROMPT_KEY_ESCAPE: {
      if (!sp_prompt_poll_stdin(ctx)) {
        event.kind = SP_PROMPT_EVENT_ESCAPE;
        return event;
      }

      u8 seq[2] = {0};
      if (sp_sys_read(sp_sys_stdin, &seq[0], 1) <= 0) {
        event.kind = SP_PROMPT_EVENT_ESCAPE;
        return event;
      }

      if (sp_prompt_poll_stdin(ctx)) {
        if (sp_sys_read(sp_sys_stdin, &seq[1], 1) <= 0) {
          seq[1] = 0;
        }
      }

      if (seq[0] == '[') {
        switch (seq[1]) {
          case 'A': event.kind = SP_PROMPT_EVENT_UP;    return event;
          case 'B': event.kind = SP_PROMPT_EVENT_DOWN;  return event;
          case 'C': event.kind = SP_PROMPT_EVENT_RIGHT; return event;
          case 'D': event.kind = SP_PROMPT_EVENT_LEFT;  return event;
        }
      }

      event.kind = SP_PROMPT_EVENT_ESCAPE;
      return event;
    }
  }

  u8 utf8_bytes[4] = { c };
  u32 needed = 1;
  if      ((c & SP_PROMPT_UTF8_2_BYTE_MASK) == SP_PROMPT_UTF8_2_BYTE_PREFIX) needed = SP_PROMPT_UTF8_2_BYTE_LEN;
  else if ((c & SP_PROMPT_UTF8_3_BYTE_MASK) == SP_PROMPT_UTF8_3_BYTE_PREFIX) needed = SP_PROMPT_UTF8_3_BYTE_LEN;
  else if ((c & SP_PROMPT_UTF8_4_BYTE_MASK) == SP_PROMPT_UTF8_4_BYTE_PREFIX) needed = SP_PROMPT_UTF8_4_BYTE_LEN;

  sp_for_range(i, 1, needed) {
    if (sp_sys_read(sp_sys_stdin, &utf8_bytes[i], 1) <= 0) break;
  }

  event.kind = SP_PROMPT_EVENT_INPUT;
  event.input.codepoint = sp_utf8_decode(utf8_bytes);
  return event;
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
      sp_prompt_emit(ctx, SP_ANSI_SGR_RESET);
      break;
    }
    case SP_PROMPT_STYLE_ANSI: {
      sp_str_t esc = sp_fmt_a(sp_context_get_allocator(), SP_ANSI_SGR_ANSI_FMT, sp_fmt_uint(style.ansi)).value;
      sp_prompt_emit_str(ctx, esc);
      break;
    }
    case SP_PROMPT_STYLE_RGB: {
      sp_str_t esc = sp_fmt_a(sp_context_get_allocator(), SP_ANSI_SGR_RGB_FMT, sp_fmt_uint(style.rgb.r), sp_fmt_uint(style.rgb.g), sp_fmt_uint(style.rgb.b)).value;
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
  sp_prompt_emit(ctx, SP_ANSI_BEGIN_SYNC);
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
      ctx->framebuffer + line * ctx->cols,
      ctx->cols
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

  sp_prompt_emit(ctx, SP_ANSI_END_SYNC);
  sp_io_flush(ctx->writer);
}

sp_app_result_t sp_prompt_app_on_init(sp_app_t* app) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)app->user_data;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  ctx->state = SP_PROMPT_STATE_ACTIVE;
  ctx->value = sp_zero_struct(sp_prompt_value_t);

  if (!sp_da_empty(ctx->frames)) {
    sp_prompt_emit(ctx, "\r\n│\r\n");
  }

  sp_prompt_dispatch_event(ctx, ctx->widget, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_INIT });
  sp_prompt_render_frame(ctx, ctx->widget);
  sp_prompt_present(ctx);

  sp_mem_end_scratch(scratch);
  return ctx->state == SP_PROMPT_STATE_ACTIVE ? SP_APP_CONTINUE : SP_APP_QUIT;
}

void sp_prompt_app_on_deinit(sp_app_t* app) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)app->user_data;

  if (ctx->cursor_row > 0) {
    u32 cell_count = ctx->cursor_row * ctx->cols;

    sp_prompt_frame_t frame = {
      .cols = ctx->cols,
      .rows = ctx->cursor_row,
      .cells = sp_mem_arena_alloc_n(ctx->arena, sp_prompt_cell_t, cell_count),
    };

    sp_mem_copy(ctx->framebuffer, frame.cells, sizeof(sp_prompt_cell_t) * cell_count);
    sp_da_push(ctx->frames, frame);
  }
}

sp_app_result_t sp_prompt_app_on_poll(sp_app_t* app) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)app->user_data;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  sp_da(sp_prompt_event_t) events = sp_zero();
  sp_da_init(scratch.marker.mem, events);

  // Drain everything. Primed events come first by definition, but there's no specific order
  // between progress, status, and inputs.
  while (ctx->primed.count > ctx->primed.index) {
    sp_da_push(events, ctx->primed.events[ctx->primed.index++]);
  }

  sp_mutex_lock(&ctx->channel.lock);
  if (ctx->progress.dirty) {
    sp_prompt_event_t event = {
      .kind = SP_PROMPT_EVENT_PROGRESS,
      .progress.data = ctx->progress.value
    };
    sp_da_push(events, event);
    ctx->progress.dirty = false;
  }
  if (ctx->status.dirty) {
    sp_prompt_event_t event = {
      .kind = SP_PROMPT_EVENT_STATUS,
      .status.value = ctx->status.value
    };
    sp_da_push(events, event);
    ctx->status.dirty = false;
  }
  sp_mutex_unlock(&ctx->channel.lock);

  while (true) {
    sp_prompt_event_t event = sp_prompt_drain_stdin(ctx);
    if (event.kind == SP_PROMPT_EVENT_NONE) break;

    sp_da_push(events, event);
  }

  // Widgets that don't .on_update have nothing to do without an event, so we prefer to
  // yield to the kernel until one shows up. We model this with a pipe; when you want to
  // make sure the prompt wakes up, you write to your end of the pipe.
  //
  // We don't actually read the data, though. Still, there's no need for a concurrency
  // primitive like a condition variable or a semaphore because the only data to
  // synchronize is a single sp_atomic_s32_t, the state. And more, using a file descriptor
  // lets us join "wait for stdin" and "wait for wake signal" in one OS primitive.
  //
  // The only unclear thing is that we still drain the wake fd, so it doesn't fill up, but
  // we discard the data.
  if (sp_da_empty(events)) {
    if (!ctx->widget.on_update) {
      u8 ready [2] = sp_zero();
      sp_sys_fd_t fds [2] = { ctx->terminal.fds.in, ctx->wake.read };
      sp_sys_fds_wait(fds, ready, 2);

      if (ready[1]) {
        u8 drain[SP_PROMPT_WAKE_DRAIN_SIZE];
        while (sp_sys_read(ctx->wake.read, drain, sizeof(drain)) > 0) {}
        sp_atomic_s32_set(&ctx->wake.pending, SP_PROMPT_WAKE_NOT_PENDING);
      }
    }
  }

  if (sp_prompt_is_aborted(ctx)) {
    sp_da_push(events, (sp_prompt_event_t) { .kind = SP_PROMPT_EVENT_ABORT });
  }

  sp_da_for(events, it) {
    sp_prompt_dispatch_event(ctx, ctx->widget, events[it]);
  }

  if (!sp_da_empty(events)) {
    sp_prompt_render_frame(ctx, ctx->widget);
    sp_prompt_present(ctx);
  }

  sp_mem_end_scratch(scratch);
  return ctx->state == SP_PROMPT_STATE_ACTIVE ? SP_APP_CONTINUE : SP_APP_QUIT;
}

sp_app_result_t sp_prompt_app_on_update(sp_app_t* app) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)app->user_data;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  if (ctx->state == SP_PROMPT_STATE_ACTIVE && ctx->widget.on_update) {
    ctx->widget.on_update(ctx);
    sp_prompt_render_frame(ctx, ctx->widget);
    sp_prompt_present(ctx);
  }

  sp_mem_end_scratch(scratch);
  return ctx->state == SP_PROMPT_STATE_ACTIVE ? SP_APP_CONTINUE : SP_APP_QUIT;
}

sp_app_config_t sp_prompt_app(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  sp_assert(widget.on_event);

  ctx->widget = widget;
  ctx->user_data = widget.user_data;
  return (sp_app_config_t) {
    .user_data = ctx,
    .on_init   = sp_prompt_app_on_init,
    .on_poll   = sp_prompt_app_on_poll,
    .on_update = sp_prompt_app_on_update,
    .on_deinit = sp_prompt_app_on_deinit,
    .fps = widget.fps ? widget.fps : SP_PROMPT_DEFAULT_FPS,
  };
}

bool sp_prompt_run(sp_prompt_ctx_t* ctx, sp_prompt_widget_t widget) {
  sp_app_run(sp_prompt_app(ctx, widget));
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
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  u8 buf[4] = SP_ZERO_INITIALIZE();
  u8 len = sp_utf8_encode(codepoint, buf);
  sp_for(it, count) {
    sp_io_write_str(&builder, sp_str(buf, len), SP_NULLPTR);
  }
  return sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
}

static void sp_prompt_static_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  SP_UNUSED(event);
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
}

static void sp_prompt_intro_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro_t* prompt = (sp_prompt_intro_t*)ctx->user_data;
  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "┌  {}", sp_fmt_str(prompt->text)).value);
}

static void sp_prompt_outro_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_outro_t* prompt = (sp_prompt_outro_t*)ctx->user_data;
  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "└  {}", sp_fmt_str(prompt->text)).value);
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

  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "◇  {} {}╮", sp_fmt_str(prompt->title), sp_fmt_str(top_tail)).value);
  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "│  {}│", sp_fmt_str(spacer)).value);

  sp_da_for(message_lines, it) {
    sp_str_t line = message_lines[it];
    u32 line_width = sp_prompt_text_width(line);
    sp_str_t pad = sp_prompt_repeat(' ', width - line_width);
    sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "│  {}{}│", sp_fmt_str(line), sp_fmt_str(pad)).value);
  }

  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "│  {}│", sp_fmt_str(spacer)).value);
  sp_prompt_line(ctx, sp_fmt_a(sp_context_get_allocator(), "├{}╯", sp_fmt_str(bottom)).value);
}

sp_prompt_widget_t sp_prompt_intro_widget(sp_prompt_ctx_t* ctx, sp_prompt_intro_t config) {
  sp_prompt_intro_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_intro_t);
  *user_data = config;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_static_update,
    .render = sp_prompt_intro_render,
  };
}

sp_prompt_widget_t sp_prompt_outro_widget(sp_prompt_ctx_t* ctx, sp_prompt_outro_t config) {
  sp_prompt_outro_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_outro_t);
  *user_data = config;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_static_update,
    .render = sp_prompt_outro_render,
  };
}

sp_prompt_widget_t sp_prompt_note_widget(sp_prompt_ctx_t* ctx, sp_prompt_note_t config) {
  sp_prompt_note_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_note_t);
  *user_data = config;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_static_update,
    .render = sp_prompt_note_render,
  };
}

static void sp_prompt_message_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  SP_UNUSED(event);
  sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
}

static void sp_prompt_message_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_message_t* prompt = (sp_prompt_message_t*)ctx->user_data;
  sp_prompt_style_t style = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = prompt->ansi,
  };
  sp_prompt_render_line(ctx, sp_fmt_a(sp_context_get_allocator(), "{}  ", sp_fmt_str(sp_prompt_repeat(prompt->symbol, 1))).value, style);
  sp_prompt_render_line(ctx, prompt->text, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

sp_prompt_widget_t sp_prompt_message_widget(sp_prompt_ctx_t* ctx, sp_prompt_message_t config) {
  sp_prompt_message_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_message_t);
  *user_data = config;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_message_update,
    .render = sp_prompt_message_render,
  };
}

void sp_prompt_intro(sp_prompt_ctx_t* ctx, const c8* title) {
  sp_prompt_run(ctx, sp_prompt_intro_widget(ctx, (sp_prompt_intro_t) {
    .text = sp_str_view(title),
  }));
}

void sp_prompt_outro(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_outro_widget(ctx, (sp_prompt_outro_t) {
    .text = sp_str_view(message),
  }));
}

void sp_prompt_note(sp_prompt_ctx_t* ctx, const c8* message, const c8* title) {
  sp_prompt_run(ctx, sp_prompt_note_widget(ctx, (sp_prompt_note_t) {
    .message = sp_str_view(message),
    .title = sp_str_view(title),
  }));
}

void sp_prompt_cancel(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
    .text = sp_str_view(message),
    .symbol = 0x25a0,
    .ansi = SP_ANSI_FG_RED_U8,
  }));
}

void sp_prompt_info(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
    .text = sp_str_view(message),
    .symbol = 0x25cf,
    .ansi = SP_ANSI_FG_CYAN_U8,
  }));
}

void sp_prompt_warn(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
    .text = sp_str_view(message),
    .symbol = 0x25b2,
    .ansi = SP_ANSI_FG_YELLOW_U8,
  }));
}

void sp_prompt_error(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
    .text = sp_str_view(message),
    .symbol = 0x25a0,
    .ansi = SP_ANSI_FG_RED_U8,
  }));
}

void sp_prompt_success(sp_prompt_ctx_t* ctx, const c8* message) {
  sp_prompt_run(ctx, sp_prompt_message_widget(ctx, (sp_prompt_message_t) {
    .text = sp_str_view(message),
    .symbol = 0x25c6,
    .ansi = SP_ANSI_FG_GREEN_U8,
  }));
}

static void sp_prompt_str_append_codepoint(sp_prompt_ctx_t* ctx, sp_str_t* value, u32 codepoint) {
  sp_context_push_arena(ctx->arena);
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  sp_io_write_str(&builder, *value, SP_NULLPTR);
  u8 buf[4] = SP_ZERO_INITIALIZE();
  u8 len = sp_utf8_encode(codepoint, buf);
  sp_io_write_str(&builder, sp_str(buf, len), SP_NULLPTR);
  *value = sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
  sp_context_pop();
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

static void sp_prompt_text_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_text_widget_t* text = (sp_prompt_text_widget_t*)ctx->user_data;

  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_str_append_codepoint(ctx, &text->value, event.input.codepoint);
      break;
    }
    case SP_PROMPT_EVENT_BACKSPACE: {
      text->value = sp_prompt_str_pop_codepoint(text->value);
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      sp_prompt_set_str(ctx, sp_str_empty(text->value) ? text->config.prefill : text->value);
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
    case SP_PROMPT_EVENT_ESCAPE:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
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
        .ansi = SP_ANSI_FG_BLUE_U8,
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
  sp_prompt_text_widget_t* text = (sp_prompt_text_widget_t*)ctx->user_data;
  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, text->config.prompt, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (ctx->cursor_row >= ctx->rows) {
    return;
  }

  sp_prompt_write_rail_prefix(ctx);

  if (sp_str_empty(text->value)) {
    if (sp_str_empty(text->config.prefill)) {
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    } else {
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
      };
      sp_prompt_render_line(ctx, text->config.prefill, style);
      ctx->cursor_col = 0;
      ctx->cursor_row++;
    }
  } else {
    sp_prompt_render_line(ctx, text->value, SP_ZERO_STRUCT(sp_prompt_style_t));
    ctx->cursor_col = 0;
    ctx->cursor_row++;
  }
}

sp_prompt_widget_t sp_prompt_text_widget(sp_prompt_ctx_t* ctx, sp_prompt_text_t config) {
  sp_prompt_text_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_text_widget_t);
  user_data->config = config;
  user_data->value = SP_LIT("");
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_text_update,
    .render = sp_prompt_text_render,
  };
}

static void sp_prompt_confirm_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_confirm_widget_t* confirm = (sp_prompt_confirm_widget_t*)ctx->user_data;

  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      if (event.input.codepoint == 'y' || event.input.codepoint == 'Y') {
        confirm->value = true;
      } else if (event.input.codepoint == 'n' || event.input.codepoint == 'N') {
        confirm->value = false;
      } else if (
        event.input.codepoint == 'h' ||
        event.input.codepoint == 'H' ||
        event.input.codepoint == 'j' ||
        event.input.codepoint == 'J' ||
        event.input.codepoint == 'k' ||
        event.input.codepoint == 'K' ||
        event.input.codepoint == 'l' ||
        event.input.codepoint == 'L'
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
    case SP_PROMPT_EVENT_BACKSPACE:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
      break;
    }
  }

  if (ctx->state == SP_PROMPT_STATE_SUBMIT) {
    sp_prompt_set_bool(ctx, confirm->value);
  }
}

static void sp_prompt_confirm_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_confirm_widget_t* confirm = (sp_prompt_confirm_widget_t*)ctx->user_data;

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_GREEN_U8,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, sp_str_view(confirm->config.prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
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

sp_prompt_widget_t sp_prompt_confirm_widget(sp_prompt_ctx_t* ctx, sp_prompt_confirm_t config) {
  sp_prompt_confirm_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_confirm_widget_t);
  user_data->config = config;
  user_data->value = config.initial;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_confirm_update,
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
    *max_items = SP_PROMPT_DEFAULT_VISIBLE_OPTIONS;
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

static u32 sp_prompt_select_initial_cursor(sp_prompt_select_widget_t* select) {
  sp_for(it, select->config.num_options) {
    if (select->config.options[it].selected) {
      return it;
    }
  }

  return 0;
}

static bool sp_prompt_select_matches_filter(sp_prompt_select_widget_t* select, u32 option_index) {
  if (!select->config.filter || sp_str_empty(select->state.filter_value)) {
    return true;
  }

  return sp_prompt_str_contains_case_insensitive(sp_prompt_option_label(&select->config.options[option_index]), select->state.filter_value);
}

static u32 sp_prompt_select_filtered_count(sp_prompt_select_widget_t* select) {
  u32 count = 0;
  sp_for(it, select->config.num_options) {
    if (sp_prompt_select_matches_filter(select, it)) {
      count++;
    }
  }

  return count;
}

static u32 sp_prompt_select_filtered_to_option_index(sp_prompt_select_widget_t* select, u32 filtered_index) {
  u32 count = 0;
  sp_for(it, select->config.num_options) {
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

static void sp_prompt_select_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_select_widget_t* select = (sp_prompt_select_widget_t*)ctx->user_data;

  if (event.kind == SP_PROMPT_EVENT_INIT) {
    sp_prompt_choice_state_reset(&select->state, sp_prompt_select_initial_cursor(select));
  }

  u32 filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);

  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      if (select->config.num_options == 0) {
        break;
      }

      if (select->config.filter) {
          sp_prompt_str_append_codepoint(ctx, &select->state.filter_value, event.input.codepoint);
      } else {
        if (event.input.codepoint == 'j' || event.input.codepoint == 'J') {
          sp_prompt_choice_state_move_cursor(&select->state, filtered_count, 1);
        } else if (event.input.codepoint == 'k' || event.input.codepoint == 'K') {
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
      if (select->config.filter) {
        select->state.filter_value = sp_prompt_str_pop_codepoint(select->state.filter_value);
      }
      break;
    }
    case SP_PROMPT_EVENT_ENTER: {
      if (filtered_count > 0) {
        u32 selected_index = sp_prompt_select_filtered_to_option_index(select, select->state.cursor);
        sp_prompt_set_str(ctx, sp_prompt_option_label(&select->config.options[selected_index]));
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
    case SP_PROMPT_EVENT_TAB:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
      break;
    }
  }

  filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);
}

static void sp_prompt_select_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_select_widget_t* select = (sp_prompt_select_widget_t*)ctx->user_data;
  u32 filtered_count = sp_prompt_select_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_GREEN_U8,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, select->config.prompt == SP_NULLPTR ? SP_LIT("") : sp_str_view(select->config.prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  if (select->config.filter) {
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
      sp_prompt_render_line(ctx, sp_prompt_option_label(&select->config.options[selected_index]), inactive);
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
    if (visible_count > select->config.max_visible) {
      visible_count = select->config.max_visible;
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
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->config.options[index]), SP_ZERO_STRUCT(sp_prompt_style_t));
      } else {
        sp_prompt_render_line(ctx, SP_LIT("○ "), inactive);
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->config.options[index]), inactive);
      }

      sp_str_t hint = sp_prompt_option_hint(&select->config.options[index]);
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

sp_prompt_widget_t sp_prompt_select_widget(sp_prompt_ctx_t* ctx, sp_prompt_select_t config) {
  config.max_visible = config.max_visible ? config.max_visible : config.num_options;
  sp_prompt_select_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_select_widget_t);
  user_data->config = config;
  user_data->state = SP_ZERO_STRUCT(sp_prompt_choice_state_t);
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_select_update,
    .render = sp_prompt_select_render,
  };
}

static bool sp_prompt_multiselect_matches_filter(sp_prompt_multiselect_widget_t* select, u32 option_index) {
  if (!select->config.filter || sp_str_empty(select->state.filter_value)) {
    return true;
  }

  return sp_prompt_str_contains_case_insensitive(sp_prompt_option_label(&select->config.options[option_index]), select->state.filter_value);
}

static u32 sp_prompt_multiselect_filtered_count(sp_prompt_multiselect_widget_t* select) {
  u32 count = 0;
  sp_for(it, select->config.num_options) {
    if (sp_prompt_multiselect_matches_filter(select, it)) {
      count++;
    }
  }

  return count;
}

static u32 sp_prompt_multiselect_filtered_to_option_index(sp_prompt_multiselect_widget_t* select, u32 filtered_index) {
  u32 count = 0;
  sp_for(it, select->config.num_options) {
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

static void sp_prompt_multiselect_toggle_current(sp_prompt_multiselect_widget_t* select, u32 filtered_count) {
  if (filtered_count == 0) {
    return;
  }

  u32 option_index = sp_prompt_multiselect_filtered_to_option_index(select, select->state.cursor);
  select->config.options[option_index].selected = !select->config.options[option_index].selected;
}

static void sp_prompt_multiselect_handle_input(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_widget_t* select, u32 codepoint, u32 filtered_count) {
  if (select->config.num_options == 0) {
    return;
  }

  if (codepoint == ' ') {
    sp_prompt_multiselect_toggle_current(select, filtered_count);
    return;
  }

  if (select->config.filter) {
    sp_prompt_str_append_codepoint(ctx, &select->state.filter_value, codepoint);
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

static void sp_prompt_multiselect_backspace(sp_prompt_multiselect_widget_t* select) {
  if (select->config.filter) {
    select->state.filter_value = sp_prompt_str_pop_codepoint(select->state.filter_value);
  }
}

static void sp_prompt_multiselect_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_multiselect_widget_t* select = (sp_prompt_multiselect_widget_t*)ctx->user_data;

  if (event.kind == SP_PROMPT_EVENT_INIT) {
    sp_prompt_choice_state_reset(&select->state, 0);
  }

  u32 filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);

  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      break;
    }
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_multiselect_handle_input(ctx, select, event.input.codepoint, filtered_count);
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
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
      break;
    }
  }

  filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);
}

static void sp_prompt_multiselect_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_multiselect_widget_t* select = (sp_prompt_multiselect_widget_t*)ctx->user_data;
  u32 filtered_count = sp_prompt_multiselect_filtered_count(select);
  sp_prompt_choice_state_sync_window(&select->state, &select->config.max_visible, filtered_count);

  sp_prompt_style_t active_symbol = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_GREEN_U8,
  };

  sp_prompt_style_t inactive = {
    .tag = SP_PROMPT_STYLE_ANSI,
    .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
  };

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, select->config.prompt == SP_NULLPTR ? SP_LIT("") : sp_str_view(select->config.prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  if (select->config.filter) {
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
    sp_for(it, select->config.num_options) {
      if (!select->config.options[it].selected) {
        continue;
      }

      if (!first) sp_prompt_render_line(ctx, sp_str_lit(", "), inactive);
      first = false;
      sp_prompt_render_line(ctx, sp_prompt_option_label(select->config.options + it), inactive);

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
    if (visible_count > select->config.max_visible) {
      visible_count = select->config.max_visible;
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
      bool selected = select->config.options[index].selected;

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
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->config.options[index]), SP_ZERO_STRUCT(sp_prompt_style_t));
      } else {
        sp_prompt_render_line(ctx, sp_prompt_option_label(&select->config.options[index]), inactive);
      }

      sp_str_t hint = sp_prompt_option_hint(&select->config.options[index]);
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

sp_prompt_widget_t sp_prompt_multiselect_widget(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t config) {
  config.max_visible = config.max_visible ? config.max_visible : config.num_options;
  sp_prompt_multiselect_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_multiselect_widget_t);
  user_data->config = config;
  user_data->state = SP_ZERO_STRUCT(sp_prompt_choice_state_t);
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_multiselect_update,
    .render = sp_prompt_multiselect_render,
  };
}

static void sp_prompt_password_update(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_password_widget_t* password = (sp_prompt_password_widget_t*)ctx->user_data;

  switch (event.kind) {
    case SP_PROMPT_EVENT_INPUT: {
      sp_prompt_str_append_codepoint(ctx, &password->value, event.input.codepoint);
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
      sp_prompt_set_str(ctx, sp_str_empty(password->value) ? password->config.prefill : password->value);
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
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
      break;
    }
  }
}

static void sp_prompt_password_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_password_widget_t* password = (sp_prompt_password_widget_t*)ctx->user_data;
  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, password->config.prompt, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  sp_prompt_write_rail_prefix(ctx);

  if (sp_str_empty(password->value)) {
    if (sp_str_empty(password->config.prefill)) {
      ctx->cursor_col = 0;
      ctx->cursor_row++;
      return;
    }

    if (password->mask) {
      sp_str_t masked = sp_prompt_repeat('*', sp_prompt_text_width(password->config.prefill));
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
      };
      sp_prompt_render_line(ctx, masked, style);
    } else {
      sp_prompt_style_t style = {
        .tag = SP_PROMPT_STYLE_ANSI,
        .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8,
      };
      sp_prompt_render_line(ctx, password->config.prefill, style);
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

sp_prompt_widget_t sp_prompt_password_widget(sp_prompt_ctx_t* ctx, sp_prompt_password_t config) {
  sp_prompt_password_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_password_widget_t);
  user_data->config = config;
  user_data->value = SP_LIT("");
  user_data->mask = config.mask;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_password_update,
    .render = sp_prompt_password_render,
  };
}

const c8* sp_prompt_text(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* initial) {
  sp_prompt_run(ctx, sp_prompt_text_widget(ctx, (sp_prompt_text_t) {
    .prompt = sp_str_view(prompt),
    .prefill = sp_str_view(initial),
  }));
  return sp_prompt_get_str(ctx);
}

bool sp_prompt_confirm(sp_prompt_ctx_t* ctx, const c8* prompt, bool initial) {
  sp_prompt_run(ctx, sp_prompt_confirm_widget(ctx, (sp_prompt_confirm_t) {
    .prompt = prompt,
    .initial = initial,
  }));
  return sp_prompt_get_bool(ctx);
}

bool sp_prompt_select(sp_prompt_ctx_t* ctx, sp_prompt_select_t config) {
  return sp_prompt_run(ctx, sp_prompt_select_widget(ctx, config));
}

void sp_prompt_multiselect(sp_prompt_ctx_t* ctx, sp_prompt_multiselect_t config) {
  sp_prompt_run(ctx, sp_prompt_multiselect_widget(ctx, config));
}

const c8* sp_prompt_password(sp_prompt_ctx_t* ctx, const c8* prompt, const c8* prefill) {
  sp_prompt_run(ctx, sp_prompt_password_widget(ctx, (sp_prompt_password_t) {
    .prompt = sp_str_view(prompt),
    .prefill = sp_str_view(prefill),
    .mask = true,
  }));
  return sp_prompt_get_str(ctx);
}

static void sp_prompt_pulse_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  switch (event.kind) {
    case SP_PROMPT_EVENT_ENTER: {
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
    case SP_PROMPT_EVENT_INPUT:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_TAB:
    case SP_PROMPT_EVENT_BACKSPACE:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_STATUS: {
      break;
    }
  }
}

static u32 sp_prompt_spinner_frame_count(sp_prompt_spinner_t* config) {
  u32 count = 0;
  while (count < SP_PROMPT_SPINNER_MAX_FRAMES && config->frames[count] != 0) count++;
  return count;
}

static void sp_prompt_spinner_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_spinner_widget_t* spinner = (sp_prompt_spinner_widget_t*)ctx->user_data;
  if (event.kind == SP_PROMPT_EVENT_INIT) {
    spinner->frame_index = 0;
  }
  sp_prompt_pulse_event(ctx, event);
}

static void sp_prompt_spinner_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_spinner_widget_t* spinner = (sp_prompt_spinner_widget_t*)ctx->user_data;
  u32 frame_count = sp_prompt_spinner_frame_count(&spinner->config);
  spinner->frame_index = frame_count > 0
    ? (spinner->frame_index + 1) % frame_count
    : spinner->frame_index + 1;
}

static void sp_prompt_spinner_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_spinner_widget_t* spinner = (sp_prompt_spinner_widget_t*)ctx->user_data;

  u32 frame_count = sp_prompt_spinner_frame_count(&spinner->config);
  u32 frame_index = spinner->frame_index;

  u32 codepoint = 0;
  sp_prompt_style_t style = SP_ZERO_STRUCT(sp_prompt_style_t);

  codepoint = (frame_count > 0 ? spinner->config.frames[frame_index] : 0);

  if (spinner->config.color.fn) {
    spinner->config.color.fn(ctx, frame_index, &style);
  } else if (spinner->config.color.rgb.r || spinner->config.color.rgb.g || spinner->config.color.rgb.b) {
    style.tag = SP_PROMPT_STYLE_RGB;
    style.rgb.r = spinner->config.color.rgb.r;
    style.rgb.g = spinner->config.color.rgb.g;
    style.rgb.b = spinner->config.color.rgb.b;
  } else if (spinner->config.color.ansi) {
    style.tag = SP_PROMPT_STYLE_ANSI;
    style.ansi = spinner->config.color.ansi;
  }

  if (codepoint != 0) {
    sp_prompt_render_line(ctx, sp_prompt_repeat(codepoint, 1), style);
    sp_prompt_render_line(ctx, SP_LIT("  "), SP_ZERO_STRUCT(sp_prompt_style_t));
  }
  sp_prompt_render_line(ctx, sp_str_view(spinner->config.prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

sp_prompt_widget_t sp_prompt_spinner_widget(sp_prompt_ctx_t* ctx, sp_prompt_spinner_t config) {
  if (!config.frames[0]) {
    u32 frames [] = SP_PROMPT_SPINNER_FALLING_SAND;
    sp_mem_copy(frames, config.frames, sizeof(frames));
  }
  sp_prompt_spinner_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_spinner_widget_t);
  user_data->config = config;
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_spinner_event,
    .on_update = sp_prompt_spinner_update,
    .render = sp_prompt_spinner_render,
    .fps = config.fps ? config.fps : SP_PROMPT_SPINNER_DEFAULT_FPS,
  };
}

void sp_prompt_spinner(sp_prompt_ctx_t* ctx, sp_prompt_spinner_t config) {
  sp_prompt_run(ctx, sp_prompt_spinner_widget(ctx, config));
}

#define SP_PROMPT_PROGRESS_DEFAULT_WIDTH 28
#define SP_PROMPT_PROGRESS_PARTS_PER_CELL 8
#define SP_PROMPT_PROGRESS_PERCENT_SCALE 100.f

static sp_prompt_style_t sp_prompt_progress_fill_style(sp_prompt_progress_widget_t* p) {
  if (p->config.color.rgb.r || p->config.color.rgb.g || p->config.color.rgb.b) {
    return (sp_prompt_style_t) {
      .tag = SP_PROMPT_STYLE_RGB,
      .rgb = { p->config.color.rgb.r, p->config.color.rgb.g, p->config.color.rgb.b },
    };
  }
  if (p->config.color.ansi) {
    return (sp_prompt_style_t) { .tag = SP_PROMPT_STYLE_ANSI, .ansi = p->config.color.ansi };
  }
  return (sp_prompt_style_t) { .tag = SP_PROMPT_STYLE_ANSI, .ansi = SP_ANSI_FG_BLUE_U8 };
}

static void sp_prompt_progress_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_progress_widget_t* p = (sp_prompt_progress_widget_t*)ctx->user_data;
  switch (event.kind) {
    case SP_PROMPT_EVENT_PROGRESS: {
      f32 r = (f32)event.progress.data.f;
      if (r < 0.f) r = 0.f;
      if (r > 1.f) r = 1.f;
      p->value = r;
      break;
    }
    case SP_PROMPT_EVENT_STATUS: {
      p->status = event.status.value;
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
    case SP_PROMPT_EVENT_INIT:
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_INPUT:
    case SP_PROMPT_EVENT_UP:
    case SP_PROMPT_EVENT_DOWN:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_ENTER:
    case SP_PROMPT_EVENT_TAB:
    case SP_PROMPT_EVENT_ABORT:
    case SP_PROMPT_EVENT_BACKSPACE:
      break;
  }
}

static void sp_prompt_progress_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_progress_widget_t* p = (sp_prompt_progress_widget_t*)ctx->user_data;
  u32 width = p->config.width ? p->config.width : SP_PROMPT_PROGRESS_DEFAULT_WIDTH;
  f32 ratio = p->value;

  sp_prompt_write_state_prefix(ctx);
  sp_prompt_render_line(ctx, sp_str_view(p->config.prompt), sp_zero_struct(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  sp_prompt_write_rail_prefix(ctx);

  sp_prompt_style_t fill = sp_prompt_progress_fill_style(p);
  sp_prompt_style_t track = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8 };

  u32 total_eighths = (u32)(ratio * (f32)(width * SP_PROMPT_PROGRESS_PARTS_PER_CELL) + 0.5f);
  sp_for(i, width) {
    u32 cell = total_eighths > i * SP_PROMPT_PROGRESS_PARTS_PER_CELL ? total_eighths - i * SP_PROMPT_PROGRESS_PARTS_PER_CELL : 0;
    if (cell > SP_PROMPT_PROGRESS_PARTS_PER_CELL) cell = SP_PROMPT_PROGRESS_PARTS_PER_CELL;
    u32 codepoint;
    sp_prompt_style_t style;
    if (cell == 0) {
      codepoint = 0x2500;
      style = track;
    } else if (cell == SP_PROMPT_PROGRESS_PARTS_PER_CELL) {
      codepoint = 0x2588;
      style = fill;
    } else {
      codepoint = 0x2590 - cell;
      style = fill;
    }
    sp_prompt_render_line(ctx, sp_prompt_repeat(codepoint, 1), style);
  }

  u32 percent = (u32)(ratio * SP_PROMPT_PROGRESS_PERCENT_SCALE + 0.5f);
  sp_prompt_render_line(ctx, sp_fmt_a(sp_context_get_allocator(), " {}%", sp_fmt_uint(percent)).value, SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (!sp_str_empty(p->status)) {
    sp_prompt_write_rail_prefix(ctx);
    sp_prompt_style_t dim = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8 };
    sp_prompt_render_line(ctx, p->status, dim);
    ctx->cursor_col = 0;
    ctx->cursor_row++;
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

static void sp_prompt_progress_update(sp_prompt_ctx_t* ctx) {
  sp_unused(ctx);
}

sp_prompt_widget_t sp_prompt_progress_widget(sp_prompt_ctx_t* ctx, sp_prompt_progress_t config) {
  sp_prompt_progress_widget_t* user_data = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_progress_widget_t);
  user_data->config = config;
  user_data->value = 0.f;
  user_data->status = SP_LIT("");
  return (sp_prompt_widget_t) {
    .user_data = user_data,
    .on_event = sp_prompt_progress_event,
    .render = sp_prompt_progress_render,
  };
}

void sp_prompt_progress(sp_prompt_ctx_t* ctx, sp_prompt_progress_t config) {
  sp_prompt_run(ctx, sp_prompt_progress_widget(ctx, config));
}

typedef struct {
  s32 active_position;
  bool is_holding;
  u32 hold_progress;
  u32 hold_total;
  u32 movement_progress;
  u32 movement_total;
  bool moving_forward;
} sp_prompt_kr_state_t;

static sp_prompt_kr_state_t sp_prompt_kr_state(sp_prompt_knight_rider_t kr, u32 frame_index) {
  u32 forward = kr.width;
  u32 backward = kr.width - 1;

  if (frame_index < forward) {
    return (sp_prompt_kr_state_t) {
      .active_position = (s32)frame_index,
      .movement_progress = frame_index,
      .movement_total = forward,
      .moving_forward = true,
    };
  }

  if (frame_index < forward + kr.ex.hold_end) {
    return (sp_prompt_kr_state_t) {
      .active_position = (s32)kr.width - 1,
      .is_holding = true,
      .hold_progress = frame_index - forward,
      .hold_total = kr.ex.hold_end,
      .moving_forward = true,
    };
  }

  if (frame_index < forward + kr.ex.hold_end + backward) {
    u32 backward_index = frame_index - forward - kr.ex.hold_end;
    return (sp_prompt_kr_state_t) {
      .active_position = (s32)kr.width - 2 - (s32)backward_index,
      .movement_progress = backward_index,
      .movement_total = backward,
      .moving_forward = false,
    };
  }

  return (sp_prompt_kr_state_t) {
    .active_position = 0,
    .is_holding = true,
    .hold_progress = frame_index - forward - kr.ex.hold_end - backward,
    .hold_total = kr.ex.hold_start,
    .moving_forward = false,
  };
}

static s32 sp_prompt_kr_color_index(u32 char_index, sp_prompt_kr_state_t state) {
  s32 directional = state.moving_forward
    ? state.active_position - (s32)char_index
    : (s32)char_index - state.active_position;

  if (state.is_holding) {
    return directional + (s32)state.hold_progress;
  }

  if (directional > 0 && directional < (s32)SP_PROMPT_KR_TRAIL_LEN) {
    return directional;
  }

  if (directional == 0) {
    return 0;
  }

  return -1;
}

static const u8 sp_prompt_kr_default_trail[SP_PROMPT_KR_TRAIL_LEN][3] = {
  { 0xFF, 0x00, 0x00 },
  { 0xFF, 0x55, 0x55 },
  { 0xDD, 0x00, 0x00 },
  { 0xAA, 0x00, 0x00 },
  { 0x77, 0x00, 0x00 },
  { 0x44, 0x00, 0x00 },
};

static const u8 sp_prompt_kr_default_inactive[3] = { 0x33, 0x00, 0x00 };

static const u32 sp_prompt_kr_diamond_shapes[4] = { 0x2B25, 0x25C6, 0x2B29, 0x2B2A };

static u8 sp_prompt_kr_clamp_u8(f32 v) {
  if (v < 0.0f) return 0;
  if (v > 255.0f) return 255;
  return (u8)v;
}

static void sp_prompt_kr_derive_palette(u8 r, u8 g, u8 b, u8 trail[SP_PROMPT_KR_TRAIL_LEN][3], u8 inactive[3]) {
  if (r == 0 && g == 0 && b == 0) {
    sp_for(it, SP_PROMPT_KR_TRAIL_LEN) {
      trail[it][0] = sp_prompt_kr_default_trail[it][0];
      trail[it][1] = sp_prompt_kr_default_trail[it][1];
      trail[it][2] = sp_prompt_kr_default_trail[it][2];
    }
    inactive[0] = sp_prompt_kr_default_inactive[0];
    inactive[1] = sp_prompt_kr_default_inactive[1];
    inactive[2] = sp_prompt_kr_default_inactive[2];
    return;
  }

  f32 base_r = r;
  f32 base_g = g;
  f32 base_b = b;

  sp_for(it, SP_PROMPT_KR_TRAIL_LEN) {
    f32 alpha;
    f32 brightness;
    if (it == 0) {
      alpha = SP_PROMPT_KR_LEAD_ALPHA;
      brightness = SP_PROMPT_KR_LEAD_BRIGHTNESS;
    }
    else if (it == 1) {
      alpha = SP_PROMPT_KR_GLOW_ALPHA;
      brightness = SP_PROMPT_KR_GLOW_BRIGHTNESS;
    }
    else {
      f32 a = 1.0f;
      sp_for(ii, it - 1) {
        SP_UNUSED(ii);
        a *= SP_PROMPT_KR_TRAIL_DECAY;
      }
      alpha = a;
      brightness = SP_PROMPT_KR_LEAD_BRIGHTNESS;
    }

    f32 r = base_r * brightness;
    f32 g = base_g * brightness;
    f32 b = base_b * brightness;
    if (r > 255.0f) r = 255.0f;
    if (g > 255.0f) g = 255.0f;
    if (b > 255.0f) b = 255.0f;

    trail[it][0] = sp_prompt_kr_clamp_u8(r * alpha);
    trail[it][1] = sp_prompt_kr_clamp_u8(g * alpha);
    trail[it][2] = sp_prompt_kr_clamp_u8(b * alpha);
  }

  inactive[0] = sp_prompt_kr_clamp_u8(base_r * SP_PROMPT_KR_INACTIVE_ALPHA);
  inactive[1] = sp_prompt_kr_clamp_u8(base_g * SP_PROMPT_KR_INACTIVE_ALPHA);
  inactive[2] = sp_prompt_kr_clamp_u8(base_b * SP_PROMPT_KR_INACTIVE_ALPHA);
}

static void sp_prompt_knight_rider_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  sp_prompt_knight_rider_widget_t* kr = (sp_prompt_knight_rider_widget_t*)ctx->user_data;
  if (event.kind == SP_PROMPT_EVENT_INIT) {
    kr->timer = sp_tm_start_timer();
    kr->elapsed_ns = 0;
  }
  sp_prompt_pulse_event(ctx, event);
}

static void sp_prompt_knight_rider_update(sp_prompt_ctx_t* ctx) {
  sp_prompt_knight_rider_widget_t* kr = (sp_prompt_knight_rider_widget_t*)ctx->user_data;
  kr->elapsed_ns = sp_tm_read_timer(&kr->timer);
}

static void sp_prompt_knight_rider_render(sp_prompt_ctx_t* ctx) {
  sp_prompt_knight_rider_widget_t* widget = (sp_prompt_knight_rider_widget_t*)ctx->user_data;
  sp_prompt_knight_rider_t c = widget->config;

  u32 num_frames = (c.width + c.ex.hold_end + (c.width - 1) + c.ex.hold_start);
  u64 interval_ns = (u64)((c.ex.interval / c.speed) * SP_TM_MS_TO_NS);
  u32 frame = (u32)((widget->elapsed_ns / interval_ns) % num_frames);
  sp_prompt_kr_state_t state = sp_prompt_kr_state(c, frame);

  u8 trail[SP_PROMPT_KR_TRAIL_LEN][3];
  u8 inactive[3];
  sp_prompt_kr_derive_palette(c.color.r, c.color.g, c.color.b, trail, inactive);

  f32 fade = 1.0f;
  if (state.is_holding && state.hold_total > 0) {
    f32 progress = (f32)state.hold_progress / (f32)state.hold_total;
    if (progress > 1.0f) progress = 1.0f;
    fade = 1.0f - progress;
  }
  else if (!state.is_holding && state.movement_total > 1) {
    f32 progress = (f32)state.movement_progress / (f32)(state.movement_total - 1);
    if (progress > 1.0f) progress = 1.0f;
    fade = progress;
  }
  if (fade < 0.0f) fade = 0.0f;

  sp_for(it, c.width) {
    s32 idx = sp_prompt_kr_color_index(it, state);
    sp_prompt_style_t style = { .tag = SP_PROMPT_STYLE_RGB };
    u32 codepoint;

    if (idx >= 0 && idx < (s32)SP_PROMPT_KR_TRAIL_LEN) {
      u32 shape = idx < 3 ? (u32)idx : 3;
      codepoint = sp_prompt_kr_diamond_shapes[shape];
      style.rgb.r = trail[idx][0];
      style.rgb.g = trail[idx][1];
      style.rgb.b = trail[idx][2];
    }
    else {
      codepoint = 0x00B7;
      style.rgb.r = sp_prompt_kr_clamp_u8((f32)inactive[0] * fade);
      style.rgb.g = sp_prompt_kr_clamp_u8((f32)inactive[1] * fade);
      style.rgb.b = sp_prompt_kr_clamp_u8((f32)inactive[2] * fade);
    }

    sp_prompt_render_line(ctx, sp_prompt_repeat(codepoint, 1), style);
  }

  sp_prompt_render_line(ctx, SP_LIT("  "), SP_ZERO_STRUCT(sp_prompt_style_t));
  sp_prompt_render_line(ctx, sp_str_view(c.prompt), SP_ZERO_STRUCT(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

sp_prompt_widget_t sp_prompt_knight_rider_widget(sp_prompt_ctx_t* ctx, sp_prompt_knight_rider_t config) {
  config.speed = config.speed <= SP_PROMPT_KR_MIN_SPEED ? 1.0 : config.speed;
  config.width = config.width ? config.width : SP_PROMPT_KR_WIDTH;
  config.ex.hold_start = config.ex.hold_start ? config.ex.hold_start : SP_PROMPT_KR_HOLD_START;
  config.ex.hold_end = config.ex.hold_end ? config.ex.hold_end : SP_PROMPT_KR_HOLD_END;
  config.ex.interval = config.ex.interval ? config.ex.interval : SP_PROMPT_KR_INTERVAL_MS;
  sp_prompt_knight_rider_widget_t* widget = sp_mem_arena_alloc_type(ctx->arena, sp_prompt_knight_rider_widget_t);
  widget->config = config;
  return (sp_prompt_widget_t) {
    .user_data = widget,
    .on_event = sp_prompt_knight_rider_event,
    .on_update = sp_prompt_knight_rider_update,
    .render = sp_prompt_knight_rider_render,
    .fps = config.fps ? config.fps : SP_PROMPT_KR_DEFAULT_FPS,
  };
}

void sp_prompt_knight_rider(sp_prompt_ctx_t* ctx, sp_prompt_knight_rider_t config) {
  sp_prompt_run(ctx, sp_prompt_knight_rider_widget(ctx, config));
}
#endif
