#define SP_UNIMPLEMENTED() ((void)0);
#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_prompt.h"

typedef struct {
  const c8* name;
  const c8* version;
  const c8* url;
  u32 chunks;
} cargo_package_t;

static const cargo_package_t cargo_packages[] = {
  { "kram",   "1.0.2", "https://git.example.com/kram.git",   18 },
  { "spum",   "0.9.1", "https://git.example.com/spum.git",   42 },
  { "peeper", "2.4.0", "https://git.example.com/peeper.git",  9 },
  { "horse",  "0.3.7", "https://git.example.com/horse.git",  12 },
  { "mimi",   "1.8.3", "https://git.example.com/mimi.git",    7 },
  { "mare",   "0.2.0", "https://git.example.com/mare.git",   24 },
};

static sp_err_t cargo_duration(sp_io_writer_t* io, sp_fmt_arg_t* arg) {
  return sp_fmt_write_duration(io, arg->value.u);
}

static void cargo_log(sp_prompt_ctx_t* ctx, const c8* verb, sp_str_t message) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_str_t line = sp_fmt(
    s.mem,
    SP_ANSI_BOLD SP_ANSI_FG_GREEN "{:>12}" SP_ANSI_RESET " " SP_ANSI_DIM "▐" SP_ANSI_RESET " {}",
    sp_fmt_cstr(verb), sp_fmt_str(message)
  ).value;
  sp_prompt_log_str(ctx, line);
  sp_mem_end_scratch(s);
}

static bool cargo_tick(sp_prompt_ctx_t* ctx, u32* done, u32 total, f64 ms) {
  (*done)++;
  sp_prompt_send_progress_f32(ctx, (f32)*done / (f32)total);
  sp_sleep_ms(ms);
  return !sp_prompt_is_aborted(ctx);
}

static s32 cargo_worker(void* userdata) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)userdata;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  u32 total = 0;
  sp_carr_for(cargo_packages, it) {
    total += 2 * cargo_packages[it].chunks;
  }

  sp_tm_timer_t timer = sp_tm_start_timer();
  cargo_log(ctx, "Resolving", sp_str_lit("your_stupid_package"));
  sp_sleep_ms(120);
  cargo_log(ctx, "Resolved", sp_fmt(s.mem, "{} packages in {}",
    sp_fmt_uint(sp_carr_len(cargo_packages)),
    sp_fmt_u64_custom(sp_tm_read_timer(&timer), cargo_duration)
  ).value);

  timer = sp_tm_start_timer();
  u32 done = 0;
  sp_carr_for(cargo_packages, it) {
    const cargo_package_t* package = &cargo_packages[it];
    cargo_log(ctx, "Downloading", sp_fmt(s.mem, "{} {}",
      sp_fmt_cstr(package->name),
      sp_fmt_cstr(package->url)
    ).value);
    sp_prompt_send_status(ctx, package->name);

    sp_for(chunk, package->chunks) {
      if (!cargo_tick(ctx, &done, total, 15)) {
        sp_mem_end_scratch(s);
        return 0;
      }
    }
  }
  cargo_log(ctx, "Downloaded", sp_fmt(s.mem, "{} packages in {}",
    sp_fmt_uint(sp_carr_len(cargo_packages)),
    sp_fmt_u64_custom(sp_tm_read_timer(&timer), cargo_duration)
  ).value);

  sp_carr_for(cargo_packages, it) {
    const cargo_package_t* package = &cargo_packages[it];
    cargo_log(ctx, "Compiling", sp_fmt(s.mem, "{} v{}",
      sp_fmt_cstr(package->name),
      sp_fmt_cstr(package->version)
    ).value);
    sp_prompt_send_status(ctx, package->name);
    sp_for(chunk, package->chunks) {
      if (!cargo_tick(ctx, &done, total, 8)) {
        sp_mem_end_scratch(s);
        return 0;
      }
    }
  }

  sp_prompt_complete(ctx);
  sp_mem_end_scratch(s);
  return 0;
}

s32 cargo_main(s32 argc, const c8** argv) {
  SP_UNUSED(argc);
  SP_UNUSED(argv);

  sp_mem_t mem = sp_mem_os_new();
  sp_prompt_ctx_t* ctx = sp_prompt_begin(mem);
  if (!ctx) {
    return 1;
  }

  sp_thread_t worker = sp_zero;
  sp_thread_init(&worker, cargo_worker, ctx);

  sp_prompt_progress(ctx, (sp_prompt_progress_t) {
    .prompt = "your_stupid_package",
    .width = 32,
    .color = { .ansi = SP_ANSI_FG_GREEN_U8 },
  });

  sp_thread_join(&worker);

  if (sp_prompt_submitted(ctx)) {
    cargo_log(ctx, "Finished", sp_str_lit("release [optimized] target(s)"));
  }
  else {
    sp_prompt_cancel(ctx, "build cancelled");
  }

  sp_prompt_end(ctx);
  return 0;
}
SP_MAIN(cargo_main)
