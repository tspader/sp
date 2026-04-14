#define SP_IMPLEMENTATION
#define SP_MAIN
#include "sp.h"

#define SP_MATH_IMPLEMENTATION
#include "sp/sp_math.h"

#if defined(SP_WIN32) || defined(SP_FREESTANDING)
sp_app_config_t sp_main(s32 num_args, const c8** args) { return sp_zero_struct(sp_app_config_t); }
#else
#include <termios.h>
#include <fcntl.h>
#include <signal.h>


typedef struct {
  f32 h;
  f32 s;
  f32 v;
  bool h_locked;
  bool s_locked;
  bool v_locked;
} params_t;

typedef struct {
  sp_termios_t original_ios;
  s32 original_fcntl_flags;
  bool terminal_modified;
  sp_da(sp_color_t) saved_colors;
  sp_color_t current_color;
  params_t params;
  u64 rand_state;
  bool needs_redraw;
  bool should_quit;
  sp_atomic_s32_t shutdown;
} app_t;

static app_t* app = SP_NULLPTR;

void palette_restore_terminal(void) {
  if (app && app->terminal_modified) {
    sp_tcsetattr(0, TCSANOW, &app->original_ios);
    sp_sys_fcntl(0, F_SETFL, app->original_fcntl_flags);
    app->terminal_modified = false;
  }
  sp_os_print(sp_str_lit("\033[?25h"));
}

void palette_signal_handler(sp_os_signal_t sig, void* userdata) {
  (void)sig;
  (void)userdata;
  palette_restore_terminal();
  sp_atomic_s32_set(&app->shutdown, 1);
}

void palette_save_terminal(app_t* app) {
  sp_tcgetattr(0, &app->original_ios);
  app->terminal_modified = true;
}

void palette_setup_raw_mode(app_t* app) {
  sp_termios_t ios = app->original_ios;
  ios.c_lflag &= ~(ICANON | ECHO);
  ios.c_cc[VMIN] = 0;
  ios.c_cc[VTIME] = 0;
  sp_tcsetattr(0, TCSANOW, &ios);
  app->original_fcntl_flags = sp_sys_fcntl(0, F_GETFL, 0);
  sp_sys_fcntl(0, F_SETFL, app->original_fcntl_flags | O_NONBLOCK);
}

u64 palette_rand_next(app_t* app) {
  app->rand_state ^= app->rand_state << 13;
  app->rand_state ^= app->rand_state >> 7;
  app->rand_state ^= app->rand_state << 17;
  return app->rand_state;
}

f32 palette_rand_f32(app_t* app, f32 min, f32 max) {
  u64 r = palette_rand_next(app);
  f32 normalized = (f32)(r & 0xFFFFFF) / (f32)0xFFFFFF;
  return min + normalized * (max - min);
}

void palette_generate_color(app_t* app) {
  f32 h = app->params.h_locked ? app->params.h : palette_rand_f32(app, 0.0f, 360.0f);
  f32 s = app->params.s_locked ? app->params.s : palette_rand_f32(app, 30.0f, 100.0f);
  f32 v = app->params.v_locked ? app->params.v : palette_rand_f32(app, 40.0f, 100.0f);

  sp_color_t hsv = { .h = h, .s = s, .v = v, .a = 1.0f };
  app->current_color = sp_color_hsv_to_rgb(hsv);
  app->needs_redraw = true;
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
  return sp_str_builder_to_str(&b_out);
}

void palette_render(app_t* app) {
  if (!app->needs_redraw) return;
  app->needs_redraw = false;

  sp_str_builder_t out = SP_ZERO_INITIALIZE();

  sp_str_builder_append_cstr(&out, "\033[H\033[2J");

  u32 num_saved = sp_da_size(app->saved_colors);
  u32 strip_width = 8;
  u32 strip_height = 3;

  if (num_saved > 0) {
    sp_for(row, strip_height) {
      sp_da_for(app->saved_colors, i) {
        sp_str_t bg = palette_color_to_ansi_bg(app->saved_colors[i]);
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

  sp_str_t current_bg = palette_color_to_ansi_bg(app->current_color);
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

  sp_str_t hex = palette_color_to_hex(app->current_color);
  sp_str_builder_append(&out, hex);
  sp_str_builder_append_cstr(&out, "\n\n");

  sp_str_builder_append_cstr(&out, "[space] regenerate  [enter] save  [q/esc] quit\n");

  sp_os_print(sp_str_builder_to_str(&out));
}

void palette_print_results(app_t* app) {
  sp_da_for(app->saved_colors, i) {
    sp_str_t hex = palette_color_to_hex(app->saved_colors[i]);
    sp_log("{}", SP_FMT_STR(hex));
  }
}

s32 palette_read_key(void) {
  c8 c = 0;
  u64 n = sp_sys_read(0, &c, 1);
  if (n <= 0) return -1;
  return c;
}

sp_app_result_t on_init(sp_app_t* app) {
  app_t* state = (app_t*)app->user_data;

  palette_save_terminal(state);
  palette_setup_raw_mode(state);

  sp_os_register_signal_handler(SP_OS_SIGNAL_INTERRUPT, palette_signal_handler, SP_NULLPTR);
  sp_os_register_signal_handler(SP_OS_SIGNAL_TERMINATE, palette_signal_handler, SP_NULLPTR);

  sp_os_print(sp_str_lit("\033[?25l"));

  palette_generate_color(state);

  return SP_APP_CONTINUE;
}

sp_app_result_t on_poll(sp_app_t* app) {
  app_t* state = (app_t*)app->user_data;

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
      sp_atomic_s32_set(&state->shutdown, 1);
      break;
    }
    default: {
      break;
    }
  }

  return SP_APP_CONTINUE;
}

sp_app_result_t on_update(sp_app_t* app) {
  app_t* state = (app_t*)app->user_data;
  if (sp_atomic_s32_get(&state->shutdown)) {
    return SP_APP_QUIT;
  }
  palette_render(state);
  return SP_APP_CONTINUE;
}

void on_deinit(sp_app_t* app) {
  app_t* state = (app_t*)app->user_data;

  palette_restore_terminal();

  sp_os_print(sp_str_lit("\033[H\033[2J"));

  palette_print_results(state);
}

sp_app_config_t sp_main(s32 num_args, const c8** args) {
  s32 hue = -1;
  s32 saturation = -1;
  s32 value = -1;

  /* palette [hue] [saturation] [value] */
  if (num_args > 1) hue        = sp_parse_s64(sp_str_from_cstr(args[1]));
  if (num_args > 2) saturation = sp_parse_s64(sp_str_from_cstr(args[2]));
  if (num_args > 3) value      = sp_parse_s64(sp_str_from_cstr(args[3]));

  app = sp_alloc_type(app_t);

  sp_tm_epoch_t now = sp_tm_now_epoch();
  app->rand_state = now.ns ^ (now.s << 32) ^ (u64)&app;
  if (app->rand_state == 0) app->rand_state = 0xDEADBEEFCAFEBABE;

  if (hue >= 0) {
    app->params.h = (f32)hue;
    app->params.h_locked = true;
  }
  if (saturation >= 0) {
    app->params.s = (f32)saturation;
    app->params.s_locked = true;
  }
  if (value >= 0) {
    app->params.v = (f32)value;
    app->params.v_locked = true;
  }

  app->needs_redraw = true;

  return (sp_app_config_t) {
    .user_data = app,
    .on_init = on_init,
    .on_poll = on_poll,
    .on_update = on_update,
    .on_deinit = on_deinit,
    .fps = 15,
  };
}
#endif
