#define SP_UNIMPLEMENTED() ((void)0);
#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_prompt.h"

typedef struct {
  const c8* label;
  const c8* detail;
} fancy_probe_t;

typedef struct {
  fancy_probe_t* probes;
  u32 num_probes;
  u32 frame;
  u32 done;
} fancy_scan_t;

typedef struct {
  const c8* section;
  const c8* text;
  bool selected;
} fancy_changelog_item_t;

typedef struct {
  fancy_changelog_item_t* items;
  u32 num_items;
  u32 cursor;
} fancy_changelog_t;

typedef enum {
  FANCY_JOB_QUEUED,
  FANCY_JOB_ACTIVE,
  FANCY_JOB_DONE,
  FANCY_JOB_FAILED,
} fancy_job_state_t;

typedef struct {
  const c8* label;
  const c8* status;
  fancy_job_state_t state;
} fancy_target_t;

typedef struct {
  sp_prompt_ctx_t* ctx;
  sp_mutex_t lock;
  fancy_target_t targets[5];
  u32 num_targets;
  u32 frame;
  f32 build_progress;
  f32 upload_progress;
  f32 overall_progress;
  const c8* latest;
  sp_thread_t worker;
  bool worker_started;
} fancy_publish_t;

static sp_prompt_style_t fancy_style_ansi(u8 ansi) {
  return (sp_prompt_style_t) { .tag = SP_PROMPT_STYLE_ANSI, .ansi = ansi };
}

static sp_prompt_style_t fancy_style_rgb(u8 r, u8 g, u8 b) {
  return (sp_prompt_style_t) {
    .tag = SP_PROMPT_STYLE_RGB,
    .rgb = { .r = r, .g = g, .b = b },
  };
}

static void fancy_nl(sp_prompt_ctx_t* ctx) {
  ctx->cursor_col = 0;
  ctx->cursor_row++;
}

static void fancy_render_cstr(sp_prompt_ctx_t* ctx, const c8* text, sp_prompt_style_t style) {
  sp_prompt_render_line(ctx, sp_str_view(text), style);
}

static void fancy_render_str(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style) {
  sp_prompt_render_line(ctx, text, style);
}

static void fancy_render_row(sp_prompt_ctx_t* ctx, const c8* symbol, sp_prompt_style_t symbol_style, sp_str_t text, sp_prompt_style_t text_style) {
  fancy_render_cstr(ctx, symbol, symbol_style);
  fancy_render_cstr(ctx, "  ", SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_render_str(ctx, text, text_style);
  fancy_nl(ctx);
}

static void fancy_render_rail(sp_prompt_ctx_t* ctx, sp_str_t text, sp_prompt_style_t style) {
  fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  fancy_render_str(ctx, text, style);
  fancy_nl(ctx);
}

static const c8* fancy_scan_spinner(u32 frame) {
  static const c8* frames[] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };
  return frames[frame % sp_carr_len(frames)];
}

static void fancy_scan_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  switch (event.kind) {
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
    case SP_PROMPT_EVENT_STATUS:
    case SP_PROMPT_EVENT_ABORT: {
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
  }
}

static void fancy_scan_update(sp_prompt_ctx_t* ctx) {
  fancy_scan_t* scan = (fancy_scan_t*)ctx->user_data;
  scan->frame++;
  if (scan->frame % 9 == 0 && scan->done < scan->num_probes) {
    scan->done++;
  }
  if (scan->done == scan->num_probes && scan->frame % 9 == 3) {
    sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
  }
}

static void fancy_scan_render(sp_prompt_ctx_t* ctx) {
  fancy_scan_t* scan = (fancy_scan_t*)ctx->user_data;
  fancy_render_row(ctx, "◆", fancy_style_rgb(0x55, 0xaa, 0xff), SP_LIT("Inspecting workspace"), SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));

  sp_for(it, scan->num_probes) {
    bool done = it < scan->done;
    bool active = it == scan->done;
    sp_prompt_style_t symbol_style = done ? fancy_style_ansi(SP_ANSI_FG_GREEN_U8) : active ? fancy_style_rgb(0x55, 0xaa, 0xff) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8);
    const c8* symbol = done ? "✓" : active ? fancy_scan_spinner(scan->frame) : "·";
    fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_render_cstr(ctx, symbol, symbol_style);
    fancy_render_cstr(ctx, "  ", SP_ZERO_STRUCT(sp_prompt_style_t));
    fancy_render_cstr(ctx, scan->probes[it].label, SP_ZERO_STRUCT(sp_prompt_style_t));
    u32 width = sp_prompt_text_width(sp_str_view(scan->probes[it].label));
    sp_for_range(pad, width, 20) {
      SP_UNUSED(pad);
      fancy_render_cstr(ctx, " ", SP_ZERO_STRUCT(sp_prompt_style_t));
    }
    fancy_render_cstr(ctx, done ? scan->probes[it].detail : active ? "checking" : "queued", done ? fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8) : active ? fancy_style_ansi(SP_ANSI_FG_CYAN_U8) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_nl(ctx);
  }
}

static sp_prompt_widget_t fancy_scan_widget(fancy_scan_t* scan) {
  return (sp_prompt_widget_t) {
    .user_data = scan,
    .on_event = fancy_scan_event,
    .on_update = fancy_scan_update,
    .render = fancy_scan_render,
    .fps = 30,
  };
}

static void fancy_changelog_move(fancy_changelog_t* log, s32 delta) {
  if (delta < 0 && log->cursor > 0) {
    log->cursor--;
  }
  else if (delta > 0 && log->cursor + 1 < log->num_items) {
    log->cursor++;
  }
}

static void fancy_changelog_next_section(fancy_changelog_t* log) {
  const c8* section = log->items[log->cursor].section;
  sp_for_range(it, log->cursor + 1, log->num_items) {
    if (!sp_cstr_equal(section, log->items[it].section)) {
      log->cursor = it;
      return;
    }
  }
  log->cursor = 0;
}

static void fancy_changelog_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  fancy_changelog_t* log = (fancy_changelog_t*)ctx->user_data;
  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT:
    case SP_PROMPT_EVENT_NONE:
    case SP_PROMPT_EVENT_LEFT:
    case SP_PROMPT_EVENT_RIGHT:
    case SP_PROMPT_EVENT_BACKSPACE:
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_STATUS:
    case SP_PROMPT_EVENT_ABORT: {
      break;
    }
    case SP_PROMPT_EVENT_UP: {
      fancy_changelog_move(log, -1);
      break;
    }
    case SP_PROMPT_EVENT_DOWN: {
      fancy_changelog_move(log, 1);
      break;
    }
    case SP_PROMPT_EVENT_TAB: {
      fancy_changelog_next_section(log);
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
    case SP_PROMPT_EVENT_INPUT: {
      switch (event.input.codepoint) {
        case 'j': {
          fancy_changelog_move(log, 1);
          break;
        }
        case 'k': {
          fancy_changelog_move(log, -1);
          break;
        }
        case 'd': {
          log->items[log->cursor].selected = false;
          break;
        }
        case 'r': {
          log->items[log->cursor].selected = true;
          break;
        }
        case ' ': {
          log->items[log->cursor].selected = !log->items[log->cursor].selected;
          break;
        }
        case 'a': {
          sp_prompt_set_state(ctx, SP_PROMPT_STATE_SUBMIT);
          break;
        }
      }
      break;
    }
  }
}

static void fancy_changelog_render(sp_prompt_ctx_t* ctx) {
  fancy_changelog_t* log = (fancy_changelog_t*)ctx->user_data;
  fancy_render_row(ctx, "◆", fancy_style_rgb(0x55, 0xaa, 0xff), SP_LIT("Review changelog"), SP_ZERO_STRUCT(sp_prompt_style_t));
  //fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));

  const c8* section = "";
  sp_for(it, log->num_items) {
    fancy_changelog_item_t* item = &log->items[it];
    if (!sp_cstr_equal(section, item->section)) {
      section = item->section;
      fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));
      fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
      fancy_render_cstr(ctx, "[", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
      fancy_render_cstr(ctx, section, fancy_style_rgb(0x9b, 0xdb, 0x8d));
      fancy_render_cstr(ctx, "]", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
      fancy_nl(ctx);
    }

    bool active = it == log->cursor;
    fancy_render_cstr(ctx, active ? "◆  " : "│  ", active ? fancy_style_rgb(0x55, 0xaa, 0xff) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_render_cstr(ctx, item->selected ? "● " : "○ ", item->selected ? fancy_style_ansi(SP_ANSI_FG_GREEN_U8) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_render_cstr(ctx, item->text, active ? SP_ZERO_STRUCT(sp_prompt_style_t) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_nl(ctx);
  }

  fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_render_rail(ctx, SP_LIT("j/k move   space toggle   d drop   r restore   tab section   a accept"), fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
}

static sp_prompt_widget_t fancy_changelog_widget(fancy_changelog_t* log) {
  return (sp_prompt_widget_t) {
    .user_data = log,
    .on_event = fancy_changelog_event,
    .render = fancy_changelog_render,
  };
}

static const c8* fancy_job_symbol(fancy_job_state_t state, u32 frame) {
  switch (state) {
    case FANCY_JOB_QUEUED: return "·";
    case FANCY_JOB_ACTIVE: return fancy_scan_spinner(frame);
    case FANCY_JOB_DONE: return "✓";
    case FANCY_JOB_FAILED: return "×";
  }
  return "·";
}

static sp_prompt_style_t fancy_job_style(fancy_job_state_t state) {
  switch (state) {
    case FANCY_JOB_QUEUED: return fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8);
    case FANCY_JOB_ACTIVE: return fancy_style_rgb(0x55, 0xaa, 0xff);
    case FANCY_JOB_DONE: return fancy_style_ansi(SP_ANSI_FG_GREEN_U8);
    case FANCY_JOB_FAILED: return fancy_style_ansi(SP_ANSI_FG_RED_U8);
  }
  return fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8);
}

static void fancy_render_bar(sp_prompt_ctx_t* ctx, f32 progress, u32 width, sp_prompt_style_t fill) {
  u32 cells = (u32)(progress * (f32)width);
  if (cells > width) {
    cells = width;
  }
  sp_for(it, width) {
    fancy_render_cstr(ctx, it < cells ? "█" : "░", it < cells ? fill : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  }
}

static s32 fancy_publish_worker(void* userdata);

static void fancy_publish_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  fancy_publish_t* publish = (fancy_publish_t*)ctx->user_data;
  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      if (!publish->worker_started) {
        publish->worker_started = true;
        sp_thread_init(&publish->worker, fancy_publish_worker, publish);
      }
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
    case SP_PROMPT_EVENT_STATUS:
    case SP_PROMPT_EVENT_ABORT: {
      break;
    }
    case SP_PROMPT_EVENT_PROGRESS: {
      sp_mutex_lock(&publish->lock);
      publish->overall_progress = (f32)event.progress.data.f;
      sp_mutex_unlock(&publish->lock);
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
      break;
    }
  }
}

static void fancy_publish_update(sp_prompt_ctx_t* ctx) {
  fancy_publish_t* publish = (fancy_publish_t*)ctx->user_data;
  sp_mutex_lock(&publish->lock);
  publish->frame++;
  sp_mutex_unlock(&publish->lock);
}

static void fancy_publish_render(sp_prompt_ctx_t* ctx) {
  fancy_publish_t* publish = (fancy_publish_t*)ctx->user_data;
  sp_mutex_lock(&publish->lock);
  fancy_publish_t copy = *publish;
  sp_mutex_unlock(&publish->lock);

  fancy_render_row(ctx, "◆", fancy_style_rgb(0x55, 0xaa, 0xff), SP_LIT("Publishing v0.13.3"), SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));

  fancy_render_rail(ctx, SP_LIT("Build matrix"), fancy_style_ansi(SP_ANSI_FG_BRIGHT_WHITE_U8));
  fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  fancy_render_bar(ctx, copy.build_progress, 28, fancy_style_rgb(0x55, 0xaa, 0xff));
  fancy_render_str(ctx, sp_fmt_a(sp_context_get_allocator(), "  {}%", sp_fmt_uint((u32)(copy.build_progress * 100.0f))).value, SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_nl(ctx);
  fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));

  sp_for(it, copy.num_targets) {
    fancy_target_t* target = &copy.targets[it];
    fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_render_cstr(ctx, fancy_job_symbol(target->state, copy.frame), fancy_job_style(target->state));
    fancy_render_cstr(ctx, "  ", SP_ZERO_STRUCT(sp_prompt_style_t));
    fancy_render_cstr(ctx, target->label, SP_ZERO_STRUCT(sp_prompt_style_t));
    u32 width = sp_prompt_text_width(sp_str_view(target->label));
    sp_for_range(pad, width, 24) {
      SP_UNUSED(pad);
      fancy_render_cstr(ctx, " ", SP_ZERO_STRUCT(sp_prompt_style_t));
    }
    fancy_render_cstr(ctx, target->status, target->state == FANCY_JOB_ACTIVE ? fancy_style_ansi(SP_ANSI_FG_CYAN_U8) : fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
    fancy_nl(ctx);
  }

  fancy_render_rail(ctx, SP_LIT(""), SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_render_rail(ctx, SP_LIT("Uploads"), fancy_style_ansi(SP_ANSI_FG_BRIGHT_WHITE_U8));
  fancy_render_cstr(ctx, "│  ", fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  fancy_render_bar(ctx, copy.upload_progress, 28, fancy_style_rgb(0x9b, 0xdb, 0x8d));
  fancy_render_str(ctx, sp_fmt_a(sp_context_get_allocator(), "  {}%", sp_fmt_uint((u32)(copy.upload_progress * 100.0f))).value, SP_ZERO_STRUCT(sp_prompt_style_t));
  fancy_nl(ctx);
  fancy_render_str(ctx, sp_fmt_a(sp_context_get_allocator(), "│  latest: {}", sp_fmt_cstr(copy.latest)).value, fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  fancy_nl(ctx);
  fancy_render_str(ctx, sp_fmt_a(sp_context_get_allocator(), "│  total: {}%", sp_fmt_uint((u32)(copy.overall_progress * 100.0f))).value, fancy_style_ansi(SP_ANSI_FG_BRIGHT_BLACK_U8));
  fancy_nl(ctx);
}

static sp_prompt_widget_t fancy_publish_widget(fancy_publish_t* publish) {
  return (sp_prompt_widget_t) {
    .user_data = publish,
    .on_event = fancy_publish_event,
    .on_update = fancy_publish_update,
    .render = fancy_publish_render,
    .fps = 24,
  };
}

static void fancy_publish_set_target(fancy_publish_t* publish, u32 index, fancy_job_state_t state, const c8* status) {
  sp_mutex_lock(&publish->lock);
  publish->targets[index].state = state;
  publish->targets[index].status = status;
  sp_mutex_unlock(&publish->lock);
}

static s32 fancy_publish_worker(void* userdata) {
  fancy_publish_t* publish = (fancy_publish_t*)userdata;
  const c8* upload_paths[] = {
    "build/x86_64-linux-gnu/sp",
    "build/x86_64-linux-musl/sp",
    "build/x86_64-linux-none/sp",
    "build/aarch64-macos/sp",
    "build/x86_64-windows-gnu/sp.exe",
  };

  sp_for(target, publish->num_targets) {
    fancy_publish_set_target(publish, target, FANCY_JOB_ACTIVE, "compiling");
    sp_for(step, 16) {
      if (sp_prompt_is_aborted(publish->ctx)) {
        return 0;
      }

      f32 local = (f32)(step + 1) / 16.0f;
      f32 build = ((f32)target + local) / (f32)publish->num_targets;
      sp_mutex_lock(&publish->lock);
      publish->build_progress = build;
      publish->overall_progress = build * 0.72f;
      sp_mutex_unlock(&publish->lock);
      sp_prompt_send_progress_f32(publish->ctx, build * 0.72f);
      sp_sleep_ms(18);
    }
    fancy_publish_set_target(publish, target, FANCY_JOB_DONE, "ok");
  }

  sp_for(step, 36) {
    if (sp_prompt_is_aborted(publish->ctx)) {
      return 0;
    }

    f32 upload = (f32)(step + 1) / 36.0f;
    sp_mutex_lock(&publish->lock);
    publish->upload_progress = upload;
    publish->overall_progress = 0.72f + upload * 0.28f;
    publish->latest = upload_paths[step % sp_carr_len(upload_paths)];
    sp_mutex_unlock(&publish->lock);
    sp_prompt_send_progress_f32(publish->ctx, 0.72f + upload * 0.28f);
    sp_sleep_ms(22);
  }

  sp_prompt_complete(publish->ctx);
  return 0;
}

static void fancy_prime_enter(sp_prompt_ctx_t* ctx) {
  sp_prompt_event_t events[SP_PROMPT_PRIMED_EVENT_CAP] = {
    { .kind = SP_PROMPT_EVENT_ENTER },
  };
  sp_prompt_prime_events(ctx, events);
}

static void fancy_prime_changelog(sp_prompt_ctx_t* ctx) {
  sp_prompt_event_t events[SP_PROMPT_PRIMED_EVENT_CAP] = {
    { .kind = SP_PROMPT_EVENT_DOWN },
    { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'd' } },
    { .kind = SP_PROMPT_EVENT_TAB },
    { .kind = SP_PROMPT_EVENT_INPUT, .input = { .codepoint = 'a' } },
  };
  sp_prompt_prime_events(ctx, events);
}

static bool fancy_has_arg(s32 argc, const c8** argv, const c8* arg) {
  sp_for_range(it, 1, (u32)argc) {
    if (sp_cstr_equal(argv[it], arg)) {
      return true;
    }
  }
  return false;
}

static sp_str_t fancy_selected_changelog(fancy_changelog_item_t* items, u32 num_items) {
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  const c8* section = "";
  u64 written = 0;
  sp_for(it, num_items) {
    if (!items[it].selected) {
      continue;
    }

    if (!sp_cstr_equal(section, items[it].section)) {
      section = items[it].section;
      sp_io_writer_size(&builder, &written);
      if (written) {
        sp_io_write_c8(&builder, '\n');
      }
      sp_fmt_io(&builder, "[{}]", sp_fmt_cstr(section));
      sp_io_write_c8(&builder, '\n');
    }

    sp_fmt_io(&builder, "- {}", sp_fmt_cstr(items[it].text));
    sp_io_write_c8(&builder, '\n');
  }
  return sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
}

static sp_str_t fancy_plan_note(const c8* kind, const c8* name, const c8* sections) {
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);
  sp_fmt_io(&builder, "version      0.13.3 ({})", sp_fmt_cstr(kind));
  sp_io_write_c8(&builder, '\n');
  sp_fmt_io(&builder, "name         {}", sp_fmt_cstr(name));
  sp_io_write_c8(&builder, '\n');
  sp_io_write_cstr(&builder, "tag          v0.13.3", SP_NULLPTR);
  sp_io_write_c8(&builder, '\n');
  sp_io_write_cstr(&builder, "artifacts    linux, musl, freestanding, macos, windows", SP_NULLPTR);
  sp_io_write_c8(&builder, '\n');
  sp_fmt_io(&builder, "changelog    {}", sp_fmt_cstr(sections));
  return sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
}

s32 fancy_main(s32 argc, const c8** argv) {
  bool scripted = fancy_has_arg(argc, argv, "--scripted");
  sp_prompt_ctx_t* ctx = sp_prompt_begin();
  if (ctx == SP_NULLPTR) {
    sp_log_a("failed to initialize prompt");
    return 1;
  }

  sp_prompt_intro(ctx, "Release conductor");

  fancy_probe_t probes[] = {
    { .label = "git status", .detail = "clean" },
    { .label = "tags", .detail = "v0.13.2 found" },
    { .label = "changelog", .detail = "18 entries" },
    { .label = "targets", .detail = "5 artifacts" },
    { .label = "credentials", .detail = "token available" },
  };
  fancy_scan_t scan = {
    .probes = probes,
    .num_probes = sp_carr_len(probes),
  };

  sp_prompt_run(ctx, fancy_scan_widget(&scan));
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "preflight cancelled");
    sp_prompt_end(ctx);
    return 1;
  }

  sp_prompt_select_option_t release_options[] = {
    { .label = "patch", .hint = "0.13.2 -> 0.13.3", .selected = true },
    { .label = "minor", .hint = "0.13.2 -> 0.14.0" },
    { .label = "major", .hint = "0.13.2 -> 1.0.0" },
    { .label = "nightly", .hint = "timestamped dry run" },
  };

  if (scripted) {
    fancy_prime_enter(ctx);
  }
  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "What are we releasing?",
    .options = release_options,
    .num_options = sp_carr_len(release_options),
    .max_visible = 4,
    .filter = true,
  });
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "release selection cancelled");
    sp_prompt_end(ctx);
    return 1;
  }
  const c8* release_kind = sp_prompt_get_str(ctx);

  if (scripted) {
    fancy_prime_enter(ctx);
  }
  const c8* release_name = sp_prompt_text(ctx, "Release name", "stainless fox");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "release name cancelled");
    sp_prompt_end(ctx);
    return 1;
  }

  sp_prompt_select_option_t sections[] = {
    { .label = "Added", .hint = "6 entries", .selected = true },
    { .label = "Fixed", .hint = "9 entries", .selected = true },
    { .label = "Changed", .hint = "2 entries" },
    { .label = "Removed", .hint = "1 entry" },
  };

  if (scripted) {
    fancy_prime_enter(ctx);
  }
  sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
    .prompt = "Pick changelog sections",
    .options = sections,
    .num_options = sp_carr_len(sections),
    .max_visible = 4,
    .filter = true,
  });
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "changelog section selection cancelled");
    sp_prompt_end(ctx);
    return 1;
  }

  fancy_changelog_item_t changelog_items[] = {
    { .section = "Added", .text = "sp_prompt: add RGB release dashboard widget", .selected = true },
    { .section = "Added", .text = "sp_prompt: support searchable release-type selection", .selected = true },
    { .section = "Added", .text = "tools: collect build artifacts into release staging", .selected = true },
    { .section = "Fixed", .text = "sp_fs: normalize artifact paths before upload", .selected = true },
    { .section = "Fixed", .text = "sp_thread: propagate cancellation during long workers", .selected = true },
    { .section = "Changed", .text = "examples: make prompt demos useful as smoke tests", .selected = false },
    { .section = "Removed", .text = "release: drop the stale manual checksum step", .selected = false },
  };
  fancy_changelog_t changelog = {
    .items = changelog_items,
    .num_items = sp_carr_len(changelog_items),
  };

  if (scripted) {
    fancy_prime_changelog(ctx);
  }
  sp_prompt_run(ctx, fancy_changelog_widget(&changelog));
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "changelog review cancelled");
    sp_prompt_end(ctx);
    return 1;
  }

  if (scripted) {
    fancy_prime_enter(ctx);
  }
  sp_prompt_password(ctx, "GitHub token", "ghp_demo_token");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "credential entry cancelled");
    sp_prompt_end(ctx);
    return 1;
  }

  const c8* selected_sections = sp_prompt_join_selection(sections, sp_carr_len(sections));
  sp_str_t plan = fancy_plan_note(release_kind, release_name, selected_sections);
  sp_prompt_note(ctx, sp_str_to_cstr_a(sp_context_get_allocator(), plan), "Plan");

  if (scripted) {
    fancy_prime_enter(ctx);
  }
  bool confirmed = sp_prompt_confirm(ctx, "Publish this release?", true);
  if (sp_prompt_cancelled(ctx) || !confirmed) {
    sp_prompt_cancel(ctx, "release left unpublished");
    sp_prompt_end(ctx);
    return 1;
  }

  fancy_publish_t publish = SP_ZERO_INITIALIZE();
  publish.ctx = ctx;
  publish.num_targets = 5;
  publish.latest = "waiting for first artifact";
  publish.targets[0] = (fancy_target_t) { .label = "x86_64-linux-gnu", .status = "queued", .state = FANCY_JOB_QUEUED };
  publish.targets[1] = (fancy_target_t) { .label = "x86_64-linux-musl", .status = "queued", .state = FANCY_JOB_QUEUED };
  publish.targets[2] = (fancy_target_t) { .label = "x86_64-linux-none", .status = "queued", .state = FANCY_JOB_QUEUED };
  publish.targets[3] = (fancy_target_t) { .label = "aarch64-macos", .status = "queued", .state = FANCY_JOB_QUEUED };
  publish.targets[4] = (fancy_target_t) { .label = "x86_64-windows-gnu", .status = "queued", .state = FANCY_JOB_QUEUED };
  sp_mutex_init(&publish.lock, SP_MUTEX_PLAIN);

  sp_prompt_run(ctx, fancy_publish_widget(&publish));
  if (publish.worker_started) {
    sp_thread_join(&publish.worker);
  }
  sp_mutex_destroy(&publish.lock);

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "stopped during publish; no release was created");
    sp_prompt_end(ctx);
    return 1;
  }

  sp_str_t curated = fancy_selected_changelog(changelog_items, sp_carr_len(changelog_items));
  sp_prompt_note(ctx, sp_str_to_cstr_a(sp_context_get_allocator(), curated), "Published v0.13.3");
  sp_prompt_success(ctx, "release published");
  sp_prompt_outro(ctx, "done");
  sp_prompt_end(ctx);
  return 0;
}

SP_MAIN(fancy_main)
