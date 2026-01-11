#include "sp.h"

#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"

typedef struct {
  f32 h;
  f32 s;
  f32 v;
  bool h_locked;
  bool s_locked;
  bool v_locked;
} palette_params_t;

typedef struct {
  struct termios original_ios;
  s32 original_fcntl_flags;
  bool terminal_modified;
  sp_da(sp_color_t) saved_colors;
  sp_color_t current_color;
  palette_params_t params;
  u64 rand_state;
  bool needs_redraw;
  bool should_quit;
} palette_state_t;

static palette_state_t* g_state = SP_NULLPTR;

void palette_restore_terminal(void) {
  if (g_state && g_state->terminal_modified) {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_state->original_ios);
    fcntl(STDIN_FILENO, F_SETFL, g_state->original_fcntl_flags);
    g_state->terminal_modified = false;
  }
  sp_os_print(sp_str_lit("\033[?25h"));
}

void palette_signal_handler(s32 sig) {
  (void)sig;
  palette_restore_terminal();
  _exit(0);
}

void palette_save_terminal(palette_state_t* state) {
  tcgetattr(STDIN_FILENO, &state->original_ios);
  state->terminal_modified = true;
}

void palette_setup_raw_mode(palette_state_t* state) {
  struct termios ios = state->original_ios;
  ios.c_lflag &= ~(ICANON | ECHO);
  ios.c_cc[VMIN] = 0;
  ios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &ios);
  state->original_fcntl_flags = fcntl(STDIN_FILENO, F_GETFL);
  fcntl(STDIN_FILENO, F_SETFL, state->original_fcntl_flags | O_NONBLOCK);
}

u64 palette_rand_next(palette_state_t* state) {
  state->rand_state ^= state->rand_state << 13;
  state->rand_state ^= state->rand_state >> 7;
  state->rand_state ^= state->rand_state << 17;
  return state->rand_state;
}

f32 palette_rand_f32(palette_state_t* state, f32 min, f32 max) {
  u64 r = palette_rand_next(state);
  f32 normalized = (f32)(r & 0xFFFFFF) / (f32)0xFFFFFF;
  return min + normalized * (max - min);
}

void palette_generate_color(palette_state_t* state) {
  f32 h = state->params.h_locked ? state->params.h : palette_rand_f32(state, 0.0f, 360.0f);
  f32 s = state->params.s_locked ? state->params.s : palette_rand_f32(state, 30.0f, 100.0f);
  f32 v = state->params.v_locked ? state->params.v : palette_rand_f32(state, 40.0f, 100.0f);

  sp_color_t hsv = { .h = h, .s = s, .v = v, .a = 1.0f };
  state->current_color = sp_color_hsv_to_rgb(hsv);
  state->needs_redraw = true;
}

sp_str_t palette_color_to_ansi_bg(sp_color_t c) {
  u8 r = (u8)(c.r * 255.0f);
  u8 g = (u8)(c.g * 255.0f);
  u8 b = (u8)(c.b * 255.0f);
  return sp_format("\033[48;2;{};{};{}m", SP_FMT_U32(r), SP_FMT_U32(g), SP_FMT_U32(b));
}

sp_str_t palette_color_to_hex(sp_color_t c) {
  u8 r = (u8)(c.r * 255.0f);
  u8 g = (u8)(c.g * 255.0f);
  u8 b = (u8)(c.b * 255.0f);
  sp_str_builder_t b_out = SP_ZERO_INITIALIZE();
  sp_str_builder_append_c8(&b_out, '#');
  sp_fmt_format_hex(&b_out, r, 2, SP_NULLPTR);
  sp_fmt_format_hex(&b_out, g, 2, SP_NULLPTR);
  sp_fmt_format_hex(&b_out, b, 2, SP_NULLPTR);
  return sp_str_builder_write(&b_out);
}

void palette_render(palette_state_t* state) {
  if (!state->needs_redraw) return;
  state->needs_redraw = false;

  sp_str_builder_t out = SP_ZERO_INITIALIZE();

  sp_str_builder_append_cstr(&out, "\033[H\033[2J");

  u32 num_saved = sp_da_size(state->saved_colors);
  u32 strip_width = 8;
  u32 strip_height = 3;

  if (num_saved > 0) {
    sp_for(row, strip_height) {
      sp_da_for(state->saved_colors, i) {
        sp_str_t bg = palette_color_to_ansi_bg(state->saved_colors[i]);
        sp_str_builder_append(&out, bg);
        sp_for(col, strip_width) {
          (void)col;
          sp_str_builder_append_cstr(&out, " ");
        }
        sp_str_builder_append_cstr(&out, "\033[0m ");
      }
      sp_str_builder_append_cstr(&out, "\n");
    }
    sp_str_builder_append_cstr(&out, "\n");
  }

  sp_str_t current_bg = palette_color_to_ansi_bg(state->current_color);
  sp_str_builder_append_cstr(&out, "Current:\n");
  sp_for(row, strip_height) {
    (void)row;
    sp_str_builder_append(&out, current_bg);
    sp_for(col, strip_width) {
      (void)col;
      sp_str_builder_append_cstr(&out, " ");
    }
    sp_str_builder_append_cstr(&out, "\033[0m\n");
  }

  sp_str_t hex = palette_color_to_hex(state->current_color);
  sp_str_builder_append(&out, hex);
  sp_str_builder_append_cstr(&out, "\n\n");

  sp_str_builder_append_cstr(&out, "[space] regenerate  [enter] save  [q/esc] quit\n");

  sp_os_print(sp_str_builder_to_str(&out));
}

void palette_print_results(palette_state_t* state) {
  sp_da_for(state->saved_colors, i) {
    sp_str_t hex = palette_color_to_hex(state->saved_colors[i]);
    SP_LOG("{}", SP_FMT_STR(hex));
  }
}

s32 palette_read_key(void) {
  c8 c = 0;
  ssize_t n = read(STDIN_FILENO, &c, 1);
  if (n <= 0) return -1;
  return c;
}

sp_app_result_t palette_on_init(sp_app_t* app) {
  palette_state_t* state = (palette_state_t*)app->user_data;

  palette_save_terminal(state);
  palette_setup_raw_mode(state);

  signal(SIGINT, palette_signal_handler);
  signal(SIGTERM, palette_signal_handler);

  sp_os_print(sp_str_lit("\033[?25l"));

  palette_generate_color(state);

  return SP_APP_CONTINUE;
}

sp_app_result_t palette_on_poll(sp_app_t* app) {
  palette_state_t* state = (palette_state_t*)app->user_data;

  s32 key = palette_read_key();

  switch (key) {
    case -1: {
      break;
    }
    case ' ': {
      palette_generate_color(state);
      break;
    }
    case '\n':
    case '\r': {
      sp_da_push(state->saved_colors, state->current_color);
      palette_generate_color(state);
      break;
    }
    case 27:
    case 'q':
    case 'Q': {
      state->should_quit = true;
      break;
    }
    default: {
      break;
    }
  }

  return SP_APP_CONTINUE;
}

sp_app_result_t palette_on_update(sp_app_t* app) {
  palette_state_t* state = (palette_state_t*)app->user_data;
  if (state->should_quit) {
    return SP_APP_QUIT;
  }
  palette_render(state);
  return SP_APP_CONTINUE;
}

sp_app_result_t palette_on_deinit(sp_app_t* app) {
  palette_state_t* state = (palette_state_t*)app->user_data;

  palette_restore_terminal();

  sp_os_print(sp_str_lit("\033[H\033[2J"));

  palette_print_results(state);

  return SP_APP_CONTINUE;
}

static const c8* const usage[] = {
  "palette [options]",
  SP_NULLPTR,
};

sp_app_config_t sp_main(s32 num_args, const c8** args) {
  s32 hue = -1;
  s32 saturation = -1;
  s32 value = -1;

  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_INTEGER('H', "hue", &hue, "Lock hue (0-360)"),
    OPT_INTEGER('S', "saturation", &saturation, "Lock saturation (0-100)"),
    OPT_INTEGER('V', "value", &value, "Lock value/brightness (0-100)"),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usage, 0);
  argparse_describe(&argparse, "\nPalette generator - generate and save random colors", "");
  argparse_parse(&argparse, num_args, args);

  static palette_state_t state = SP_ZERO_INITIALIZE();
  g_state = &state;

  sp_tm_epoch_t now = sp_tm_now_epoch();
  state.rand_state = now.ns ^ (now.s << 32) ^ (u64)&state;
  if (state.rand_state == 0) state.rand_state = 0xDEADBEEFCAFEBABE;

  if (hue >= 0) {
    state.params.h = (f32)hue;
    state.params.h_locked = true;
  }
  if (saturation >= 0) {
    state.params.s = (f32)saturation;
    state.params.s_locked = true;
  }
  if (value >= 0) {
    state.params.v = (f32)value;
    state.params.v_locked = true;
  }

  state.needs_redraw = true;

  return (sp_app_config_t) {
    .user_data = &state,
    .on_init = palette_on_init,
    .on_poll = palette_on_poll,
    .on_update = palette_on_update,
    .on_deinit = palette_on_deinit,
    .fps = 15,
  };
}
