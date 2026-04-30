#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_MATH_IMPLEMENTATION
#include "sp/sp_math.h"

typedef struct {
  f32 h;
  f32 s;
  f32 v;
  bool h_locked;
  bool s_locked;
  bool v_locked;
} params_t;

typedef struct {
  sp_tty_mode_t saved_mode;
  bool terminal_modified;
  sp_da(sp_color_t) saved_colors;
  sp_color_t current_color;
  params_t params;
  u64 rand_state;
  bool needs_redraw;
  bool should_quit;
  sp_atomic_s32_t shutdown;
} app_t;

void palette_restore_terminal(app_t* app) {
  if (app && app->terminal_modified) {
    sp_os_tty_restore(sp_sys_stdin, &app->saved_mode);
    app->terminal_modified = false;
  }
  sp_os_print(sp_str_lit("\033[?25h"));
}

void palette_signal_handler(sp_os_signal_t sig, void* userdata) {
  (void)sig;
  app_t* app = (app_t*)userdata;
  palette_restore_terminal(app);
  sp_atomic_s32_set(&app->shutdown, 1);
}

void palette_enter_raw_mode(app_t* app) {
  if (sp_os_tty_enter_raw(sp_sys_stdin, &app->saved_mode) == 0) {
    app->terminal_modified = true;
  }
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
  return sp_fmt_a(sp_context_get_allocator(), "\033[48;2;{};{};{}m", sp_fmt_uint(r), sp_fmt_uint(g), sp_fmt_uint(b)).value;
}

sp_str_t palette_color_to_hex(sp_color_t c) {
  u8 r = (u8)(c.r * 255.0f);
  u8 g = (u8)(c.g * 255.0f);
  u8 b = (u8)(c.b * 255.0f);
  sp_io_writer_t b_out = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &b_out);
  sp_io_write_c8(&b_out, '#');
  for (s32 i = 0; i < 3; i++) {
    u8 v = (i == 0) ? r : (i == 1) ? g : b;
    u8 hi = (v >> 4) & 0xf, lo = v & 0xf;
    sp_io_write_c8(&b_out, (c8)(hi < 10 ? '0' + hi : 'a' + hi - 10));
    sp_io_write_c8(&b_out, (c8)(lo < 10 ? '0' + lo : 'a' + lo - 10));
  }
  return sp_io_writer_dyn_mem_as_str(&b_out.dyn_mem);
}

void palette_render(app_t* app) {
  if (!app->needs_redraw) return;
  app->needs_redraw = false;

  sp_io_writer_t out = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &out);

  sp_io_write_cstr(&out, "\033[H\033[2J", SP_NULLPTR);

  u32 num_saved = sp_da_size(app->saved_colors);
  u32 strip_width = 8;
  u32 strip_height = 3;

  if (num_saved > 0) {
    sp_for(row, strip_height) {
      sp_da_for(app->saved_colors, i) {
        sp_str_t bg = palette_color_to_ansi_bg(app->saved_colors[i]);
        sp_io_write_str(&out, bg, SP_NULLPTR);
        sp_for(col, strip_width) {
          (void)col;
          sp_io_write_cstr(&out, " ", SP_NULLPTR);
        }
        sp_io_write_cstr(&out, "\033[0m ", SP_NULLPTR);
      }
      sp_io_write_cstr(&out, "\r\n", SP_NULLPTR);
    }
    sp_io_write_cstr(&out, "\r\n", SP_NULLPTR);
  }

  sp_str_t current_bg = palette_color_to_ansi_bg(app->current_color);
  sp_io_write_cstr(&out, "Current:\r\n", SP_NULLPTR);
  sp_for(row, strip_height) {
    (void)row;
    sp_io_write_str(&out, current_bg, SP_NULLPTR);
    sp_for(col, strip_width) {
      (void)col;
      sp_io_write_cstr(&out, " ", SP_NULLPTR);
    }
    sp_io_write_cstr(&out, "\033[0m\r\n", SP_NULLPTR);
  }

  sp_str_t hex = palette_color_to_hex(app->current_color);
  sp_io_write_str(&out, hex, SP_NULLPTR);
  sp_io_write_cstr(&out, "\r\n\r\n", SP_NULLPTR);

  sp_io_write_cstr(&out, "[space] regenerate  [enter] save  [q/esc] quit\r\n", SP_NULLPTR);

  sp_os_print(sp_io_writer_dyn_mem_as_str(&out.dyn_mem));
}

void palette_print_results(app_t* app) {
  sp_da_for(app->saved_colors, i) {
    sp_str_t hex = palette_color_to_hex(app->saved_colors[i]);
    sp_log_a("{}", sp_fmt_str(hex));
  }
}

s32 palette_read_key(void) {
  u8 ready = 0;
  if (sp_sys_fd_ready(sp_sys_stdin, &ready) != 0 || !ready) return -1;
  c8 c = 0;
  return sp_sys_read(sp_sys_stdin, &c, 1) == 1 ? c : -1;
}

sp_app_result_t on_init(sp_app_t* sp) {
  app_t* app = (app_t*)sp->user_data;

  palette_enter_raw_mode(app);

  sp_os_register_signal_handler(SP_OS_SIGNAL_INTERRUPT, palette_signal_handler, app);
  sp_os_register_signal_handler(SP_OS_SIGNAL_TERMINATE, palette_signal_handler, app);

  sp_os_print(sp_str_lit("\033[?25l"));

  palette_generate_color(app);

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

void on_deinit(sp_app_t* sp) {
  app_t* app = (app_t*)sp->user_data;

  palette_restore_terminal(app);

  sp_os_print(sp_str_lit("\033[H\033[2J"));

  palette_print_results(app);
}

sp_app_config_t app_main(s32 num_args, const c8** args) {
  s32 hue = -1;
  s32 saturation = -1;
  s32 value = -1;

  /* palette [hue] [saturation] [value] */
  if (num_args > 1) hue        = sp_parse_s32(sp_str_from_cstr(args[1]));
  if (num_args > 2) saturation = sp_parse_s32(sp_str_from_cstr(args[2]));
  if (num_args > 3) value      = sp_parse_s32(sp_str_from_cstr(args[3]));

  app_t* app = sp_alloc_type(app_t);

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
SP_APP_MAIN(app_main)
