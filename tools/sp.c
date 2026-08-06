#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_glob.h"
#include "sp/sp_prompt.h"

#define str(s) sp_str_lit(s)

typedef enum {
  EXEC_LOCAL,
  EXEC_QEMU,
  EXEC_DOCKER,
  EXEC_SSH,
  EXEC_WASMTIME,
} exec_kind_t;

typedef enum {
  SYNC_NONE,
  SYNC_RSYNC,
  SYNC_SCP,
} sync_kind_t;

typedef struct {
  const c8* triple;
  exec_kind_t exec;
  const c8* hint;
  const c8* ext;
  const c8* host;
  const c8* remote;
  sync_kind_t sync;
  bool powershell;
} target_t;

static const target_t targets [] = {
  { .triple = "x86_64-linux-gnu",    .exec = EXEC_LOCAL,    .hint = "local" },
  { .triple = "x86_64-linux-musl",   .exec = EXEC_LOCAL,    .hint = "local" },
  { .triple = "x86_64-linux-none",   .exec = EXEC_LOCAL,    .hint = "local" },
  { .triple = "aarch64-linux-gnu",   .exec = EXEC_DOCKER,   .hint = "docker" },
  { .triple = "aarch64-linux-musl",  .exec = EXEC_QEMU,     .hint = "qemu" },
  { .triple = "aarch64-linux-none",  .exec = EXEC_QEMU,     .hint = "qemu" },
  { .triple = "aarch64-macos",       .exec = EXEC_SSH,      .hint = "ssh miles", .host = "spader@miles", .remote = "source/sp", .sync = SYNC_RSYNC },
  { .triple = "x86_64-windows-gnu",  .exec = EXEC_SSH,      .hint = "ssh piotr", .ext = ".exe", .host = "spader@piotr", .remote = "C:/Users/spader/source/sp", .sync = SYNC_SCP, .powershell = true },
  { .triple = "wasm32-wasi",         .exec = EXEC_WASMTIME, .hint = "wasmtime", .ext = ".wasm" },
  { .triple = "wasm32-freestanding", .exec = EXEC_WASMTIME, .hint = "wasmtime", .ext = ".wasm" },
};

typedef struct {
  struct {
    const c8* target;
    bool no_build;
    bool cpp;
    bool failed;
    const c8* output;
  } args;
  const c8** globs;
  u32 num_globs;
  sp_mem_t mem;
  sp_mem_t shared;
  const c8** argv;
  s32 argc;
  s32 status;
} tool_t;

static sp_str_t build_root(tool_t* tool) {
  return tool->args.cpp ? str("build/cpp") : str("build");
}

static sp_str_t target_ext(const target_t* target) {
  return target->ext ? sp_cstr_as_str(target->ext) : sp_zero_s(sp_str_t);
}

static sp_str_t tests_dir(sp_mem_t mem, tool_t* tool, const target_t* target) {
  return sp_fmt(mem, "{}/{}/test", sp_fmt_str(build_root(tool)), sp_fmt_cstr(target->triple)).value;
}

static sp_str_t test_rel_path(sp_mem_t mem, tool_t* tool, const target_t* target, sp_str_t name) {
  return sp_fmt(mem, "{}/{}/test/{}{}", sp_fmt_str(build_root(tool)), sp_fmt_cstr(target->triple), sp_fmt_str(name), sp_fmt_str(target_ext(target))).value;
}

static sp_str_t last_path(void) {
  return str("build/.sp-last");
}

static sp_str_t failed_path(sp_mem_t mem, tool_t* tool, const target_t* target) {
  return sp_fmt(mem, "{}/{}/.sp-failed", sp_fmt_str(build_root(tool)), sp_fmt_cstr(target->triple)).value;
}

static sp_str_t state_read(sp_mem_t mem, sp_str_t path) {
  sp_str_t content = sp_zero;
  sp_io_read_file(mem, path, &content);
  return content;
}

static void state_write(sp_str_t path, sp_str_t content) {
  sp_io_file_writer_t writer = sp_zero;
  if (sp_io_file_writer_from_path(&writer, path)) return;
  sp_io_write_str(&writer.base, content, SP_NULLPTR);
  sp_io_file_writer_close(&writer);
}

static sp_da(sp_str_t) split_lines(sp_mem_t mem, sp_str_t text) {
  sp_da(sp_str_t) lines = sp_str_split_c8(mem, text, '\n');
  sp_da_for(lines, it) {
    lines[it] = sp_str_strip_right(lines[it], str("\r"));
  }
  while (sp_da_size(lines) && sp_str_empty(lines[sp_da_size(lines) - 1])) {
    sp_da_pop(lines);
  }
  return lines;
}

static bool names_contain(sp_da(sp_str_t) names, sp_str_t name) {
  sp_da_for(names, it) {
    if (sp_str_equal(names[it], name)) return true;
  }
  return false;
}

static bool target_match(sp_mem_t mem, const target_t* target, sp_str_t query) {
  sp_da(sp_str_t) want = sp_str_split_c8(mem, query, '-');
  sp_da(sp_str_t) have = sp_str_split_c8(mem, sp_cstr_as_str(target->triple), '-');
  u32 at = 0;
  sp_da_for(want, it) {
    if (sp_str_empty(want[it])) return false;
    bool found = false;
    while (at < sp_da_size(have)) {
      if (sp_str_starts_with(have[at], want[it])) {
        found = true;
        at++;
        break;
      }
      at++;
    }
    if (!found) return false;
  }
  return true;
}

static const target_t* target_find_exact(sp_str_t triple) {
  sp_carr_for(targets, it) {
    if (sp_str_equal_cstr(triple, targets[it].triple)) return &targets[it];
  }
  return SP_NULLPTR;
}

static sp_da(const target_t*) target_resolve(sp_mem_t mem, sp_str_t query) {
  sp_da(const target_t*) matches = sp_da_new(mem, const target_t*);
  const target_t* exact = target_find_exact(query);
  if (exact) {
    sp_da_push(matches, exact);
    return matches;
  }
  sp_carr_for(targets, it) {
    if (target_match(mem, &targets[it], query)) {
      sp_da_push(matches, &targets[it]);
    }
  }
  return matches;
}

static sp_da(sp_str_t) tests_discover(sp_mem_t mem, sp_str_t dir, sp_str_t ext) {
  sp_da(sp_str_t) names = sp_da_new(mem, sp_str_t);
  sp_fs_for(mem, dir, it) {
    sp_str_t name = it.entry.name;
    if (!sp_str_empty(ext)) {
      if (!sp_str_ends_with(name, ext)) continue;
      name = sp_str_prefix(name, name.len - ext.len);
    }
    else if (sp_str_find_c8(name, '.') != SP_STR_NO_MATCH) {
      continue;
    }
    if (sp_str_equal(name, str("elf"))) continue;
    if (sp_str_equal(name, str("process"))) continue;
    sp_da_push(names, sp_str_copy(mem, name));
  }
  sp_os_qsort(names, sp_da_size(names), sizeof(sp_str_t), sp_str_sort_kernel_alphabetical);
  return names;
}

static sp_da(sp_str_t) filter_globs(sp_mem_t mem, sp_da(sp_str_t) names, const c8** globs, u32 num_globs) {
  sp_glob_set_t* set = sp_glob_set_new(mem);
  sp_for(it, num_globs) {
    sp_str_t pattern = sp_cstr_as_str(globs[it]);
    if (sp_str_empty(pattern)) continue;
    sp_glob_set_add_str(set, pattern);
    if (sp_str_at(pattern, -1) != '*') {
      sp_glob_set_add_str(set, sp_fmt(mem, "{}*", sp_fmt_str(pattern)).value);
    }
  }
  sp_glob_set_build(set);

  sp_da(sp_str_t) kept = sp_da_new(mem, sp_str_t);
  sp_da_for(names, it) {
    if (sp_glob_set_match(set, names[it])) sp_da_push(kept, names[it]);
  }
  return kept;
}

static sp_da(sp_str_t) filter_failed(sp_mem_t mem, sp_da(sp_str_t) names, sp_str_t content) {
  sp_da(sp_str_t) failed = split_lines(mem, content);
  sp_da(sp_str_t) kept = sp_da_new(mem, sp_str_t);
  sp_da_for(names, it) {
    if (names_contain(failed, names[it])) sp_da_push(kept, names[it]);
  }
  return kept;
}

static sp_ps_io_config_t capture_io(void) {
  return (sp_ps_io_config_t) {
    .in = { .mode = SP_PS_IO_MODE_NULL },
    .out = { .mode = SP_PS_IO_MODE_CREATE },
    .err = { .mode = SP_PS_IO_MODE_CREATE },
  };
}

static sp_ps_config_t make_config(sp_mem_t mem, tool_t* tool, const target_t* target) {
  sp_str_t triple = sp_fmt(mem, "TRIPLE={}", sp_fmt_cstr(target->triple)).value;
  if (tool->args.cpp) {
    return (sp_ps_config_t) {
      .command = str("make"),
      .args = { str("MODE=cpp"), triple, str("tests") },
      .io = capture_io(),
    };
  }
  return (sp_ps_config_t) {
    .command = str("make"),
    .args = { triple, str("tests") },
    .io = capture_io(),
  };
}

static sp_ps_config_t sync_config(sp_mem_t mem, tool_t* tool, const target_t* target, sp_da(sp_str_t) names) {
  sp_ps_config_t config = sp_zero;
  switch (target->sync) {
    case SYNC_NONE: {
      break;
    }
    case SYNC_RSYNC: {
      config = (sp_ps_config_t) {
        .command = str("rsync"),
        .args = { str("-azR") },
        .io = capture_io(),
      };
      sp_da_for(names, it) {
        sp_ps_config_add_arg(mem, &config, test_rel_path(mem, tool, target, names[it]));
      }
      sp_ps_config_add_arg(mem, &config, sp_fmt(mem, "{}:{}/", sp_fmt_cstr(target->host), sp_fmt_cstr(target->remote)).value);
      break;
    }
    case SYNC_SCP: {
      config = (sp_ps_config_t) {
        .command = str("scp"),
        .args = { str("-q") },
        .io = capture_io(),
      };
      sp_da_for(names, it) {
        sp_ps_config_add_arg(mem, &config, test_rel_path(mem, tool, target, names[it]));
      }
      sp_ps_config_add_arg(mem, &config, sp_fmt(mem, "{}:{}/{}/{}/test/",
        sp_fmt_cstr(target->host), sp_fmt_cstr(target->remote),
        sp_fmt_str(build_root(tool)), sp_fmt_cstr(target->triple)).value);
      break;
    }
  }
  return config;
}

typedef struct {
  sp_ps_output_t output;
  bool cancelled;
  bool spawned;
} ps_result_t;

static bool prompt_cancelling(sp_prompt_ctx_t* ctx) {
  if (!ctx) return false;
  return sp_atomic_s32_get(&ctx->state) == SP_PROMPT_STATE_CANCEL;
}

static void prompt_finish(sp_prompt_ctx_t* ctx) {
  while (true) {
    s32 state = sp_atomic_s32_get(&ctx->state);
    if (state == SP_PROMPT_STATE_CANCEL) return;
    if (state == SP_PROMPT_STATE_ACTIVE) {
      sp_prompt_complete(ctx);
      return;
    }
    sp_os_sleep_ms(1);
  }
}

static void ps_drain(sp_io_reader_t* reader, sp_io_writer_t* writer) {
  if (!reader) return;
  u8 buffer [4096];
  while (true) {
    u64 read = 0;
    sp_err_t err = sp_io_read(reader, buffer, sizeof(buffer), &read);
    if (read) sp_io_write_str(writer, sp_str(sp_cast(c8*, buffer), read), SP_NULLPTR);
    if (err || !read) return;
  }
}

static ps_result_t ps_run(sp_mem_t mem, sp_ps_config_t config, sp_prompt_ctx_t* ctx) {
  ps_result_t result = sp_zero;
  sp_ps_t ps = sp_ps_create(mem, config);
  result.spawned = ps.os != SP_NULLPTR;

  struct {
    sp_io_dyn_mem_writer_t out;
    sp_io_dyn_mem_writer_t err;
  } sink = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &sink.out);
  sp_io_dyn_mem_writer_init(mem, &sink.err);

  sp_ps_status_t status = sp_zero;
  while (result.spawned) {
    ps_drain(sp_ps_io_out(&ps), &sink.out.base);
    ps_drain(sp_ps_io_err(&ps), &sink.err.base);

    status = sp_ps_poll(&ps, 50);
    if (status.state == SP_PS_STATE_DONE) break;

    if (prompt_cancelling(ctx)) {
      sp_ps_kill(&ps);
      result.cancelled = true;
      break;
    }
  }

  ps_drain(sp_ps_io_out(&ps), &sink.out.base);
  ps_drain(sp_ps_io_err(&ps), &sink.err.base);

  result.output.out = sp_io_dyn_mem_writer_take_str(&sink.out);
  result.output.err = sp_io_dyn_mem_writer_take_str(&sink.err);
  result.output.status = status;
  sp_ps_free(&ps);
  return result;
}

static bool ps_ok(ps_result_t* result) {
  return result->spawned && result->output.status.exit_code == 0;
}

static sp_str_t ps_cause(sp_mem_t mem, ps_result_t* result, const c8* sep) {
  if (!result->spawned) {
    return sp_fmt(mem, "spawn{}failed", sp_fmt_cstr(sep)).value;
  }
  s32 code = result->output.status.exit_code;
  if (code == -255) {
    return sp_fmt(mem, "status{}unknown", sp_fmt_cstr(sep)).value;
  }
  if (code < 0) {
    return sp_fmt(mem, "signal{}{}", sp_fmt_cstr(sep), sp_fmt_int(-code)).value;
  }
  return sp_fmt(mem, "exit{}{}", sp_fmt_cstr(sep), sp_fmt_int(code)).value;
}

typedef struct {
  sp_str_t name;
  sp_str_t display;
  sp_ps_config_t config;
  ps_result_t result;
  bool ran;
} test_t;

static test_t test_init(sp_mem_t mem, tool_t* tool, const target_t* target, sp_str_t name) {
  sp_str_t rel = test_rel_path(mem, tool, target, name);
  test_t test = { .name = name };
  switch (target->exec) {
    case EXEC_LOCAL: {
      sp_str_t path = sp_fmt(mem, "./{}", sp_fmt_str(rel)).value;
      test.display = path;
      test.config = (sp_ps_config_t) { .command = path, .io = capture_io() };
      break;
    }
    case EXEC_QEMU: {
      sp_str_t path = sp_fmt(mem, "./{}", sp_fmt_str(rel)).value;
      test.display = sp_fmt(mem, "qemu-aarch64 {}", sp_fmt_str(path)).value;
      test.config = (sp_ps_config_t) {
        .command = str("qemu-aarch64"),
        .args = { path },
        .io = capture_io(),
      };
      break;
    }
    case EXEC_DOCKER: {
      sp_str_t mount = sp_fmt(mem, "{}:/sp", sp_fmt_str(sp_fs_get_cwd(mem))).value;
      sp_str_t path = sp_fmt(mem, "/sp/{}", sp_fmt_str(rel)).value;
      test.display = sp_fmt(mem, "docker run --rm --platform linux/arm64 -v {} -w /sp debian:stable-slim {}", sp_fmt_str(mount), sp_fmt_str(path)).value;
      test.config = (sp_ps_config_t) {
        .command = str("docker"),
        .args = { str("run"), str("--rm"), str("--platform"), str("linux/arm64"), str("-v"), mount, str("-w"), str("/sp"), str("debian:stable-slim"), path },
        .io = capture_io(),
      };
      break;
    }
    case EXEC_SSH: {
      sp_str_t host = sp_cstr_as_str(target->host);
      if (target->powershell) {
        sp_str_t inner = sp_fmt(mem, "cd {}; ./{}", sp_fmt_cstr(target->remote), sp_fmt_str(rel)).value;
        sp_str_t command = sp_fmt(mem, "powershell -Command {.quote}", sp_fmt_str(inner)).value;
        test.display = sp_fmt(mem, "ssh -q {} {}", sp_fmt_str(host), sp_fmt_str(command)).value;
        test.config = (sp_ps_config_t) {
          .command = str("ssh"),
          .args = { str("-q"), host, command },
          .io = capture_io(),
        };
      }
      else {
        sp_str_t inner = sp_fmt(mem, "cd {} && ./{}", sp_fmt_cstr(target->remote), sp_fmt_str(rel)).value;
        test.display = sp_fmt(mem, "ssh -q {} {.quote}", sp_fmt_str(host), sp_fmt_str(inner)).value;
        test.config = (sp_ps_config_t) {
          .command = str("ssh"),
          .args = { str("-q"), host, inner },
          .io = capture_io(),
        };
      }
      break;
    }
    case EXEC_WASMTIME: {
      sp_str_t path = sp_fmt(mem, "./{}", sp_fmt_str(rel)).value;
      test.display = sp_fmt(mem, "wasmtime run {}", sp_fmt_str(path)).value;
      test.config = (sp_ps_config_t) {
        .command = str("wasmtime"),
        .args = { str("run"), path },
        .io = capture_io(),
      };
      break;
    }
  }
  return test;
}

static sp_da(test_t) tests_init(sp_mem_t mem, tool_t* tool, const target_t* target, sp_da(sp_str_t) names) {
  sp_da(test_t) tests = sp_da_new(mem, test_t);
  sp_da_for(names, it) {
    test_t test = test_init(mem, tool, target, names[it]);
    sp_da_push(tests, test);
  }
  return tests;
}

typedef struct {
  u32 ok;
  u32 fail;
} tally_t;

static tally_t tally(sp_da(test_t) tests) {
  tally_t t = sp_zero;
  sp_da_for(tests, it) {
    if (!tests[it].ran) continue;
    if (ps_ok(&tests[it].result)) {
      t.ok++;
    }
    else {
      t.fail++;
    }
  }
  return t;
}

static void write_failed_state(sp_mem_t mem, tool_t* tool, const target_t* target, sp_da(test_t) tests) {
  sp_str_t path = failed_path(mem, tool, target);
  sp_da(sp_str_t) previous = split_lines(mem, state_read(mem, path));

  sp_da(sp_str_t) ran = sp_da_new(mem, sp_str_t);
  sp_da(sp_str_t) failed = sp_da_new(mem, sp_str_t);
  sp_da_for(tests, it) {
    if (!tests[it].ran) continue;
    sp_da_push(ran, tests[it].name);
    if (!ps_ok(&tests[it].result)) sp_da_push(failed, tests[it].name);
  }

  sp_io_dyn_mem_writer_t writer = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &writer);
  sp_da_for(previous, it) {
    if (names_contain(ran, previous[it])) continue;
    sp_fmt_io(&writer.base, "{}\n", sp_fmt_str(previous[it]));
  }
  sp_da_for(failed, it) {
    sp_fmt_io(&writer.base, "{}\n", sp_fmt_str(failed[it]));
  }

  state_write(path, sp_io_dyn_mem_writer_as_str(&writer));
}

static void log_indented(sp_mem_t mem, sp_str_t prefix, sp_str_t text) {
  sp_da(sp_str_t) lines = split_lines(mem, text);
  sp_da_for(lines, it) {
    sp_log("{}{}", sp_fmt_str(prefix), sp_fmt_str(lines[it]));
  }
}

typedef struct {
  sp_da(sp_str_t) names;
  bool no_failures;
} selection_t;

static selection_t select_names(sp_mem_t mem, tool_t* tool, const target_t* target) {
  selection_t selection = sp_zero;
  selection.names = tests_discover(mem, tests_dir(mem, tool, target), target_ext(target));
  if (tool->args.failed) {
    selection.names = filter_failed(mem, selection.names, state_read(mem, failed_path(mem, tool, target)));
    selection.no_failures = sp_da_size(selection.names) == 0;
  }
  else if (tool->num_globs) {
    selection.names = filter_globs(mem, selection.names, tool->globs, tool->num_globs);
  }
  return selection;
}

static s32 run_machine(tool_t* tool, const target_t* target) {
  sp_mem_t mem = tool->mem;

  if (!tool->args.no_build) {
    ps_result_t build = ps_run(mem, make_config(mem, tool, target), SP_NULLPTR);
    if (!ps_ok(&build)) {
      sp_log("build {} failed {}", sp_fmt_cstr(target->triple), sp_fmt_str(ps_cause(mem, &build, "=")));
      log_indented(mem, str("\t"), build.output.out);
      log_indented(mem, str("\t"), build.output.err);
      return 1;
    }
    sp_log("build {}", sp_fmt_cstr(target->triple));
  }

  sp_str_t dir = tests_dir(mem, tool, target);
  if (!sp_fs_exists(dir)) {
    sp_log("warn {} missing; run without --no-build", sp_fmt_str(dir));
    return 1;
  }

  selection_t selection = select_names(mem, tool, target);
  if (selection.no_failures) {
    sp_log("warn no failures recorded for {}", sp_fmt_cstr(target->triple));
    sp_log("done 0 ok 0 fail");
    return 0;
  }
  if (sp_da_size(selection.names) == 0) {
    sp_log("warn no tests match");
    sp_log("done 0 ok 0 fail");
    return 1;
  }

  if (target->sync != SYNC_NONE) {
    ps_result_t sync = ps_run(mem, sync_config(mem, tool, target, selection.names), SP_NULLPTR);
    if (!ps_ok(&sync)) {
      sp_log("sync {} failed {}", sp_fmt_cstr(target->triple), sp_fmt_str(ps_cause(mem, &sync, "=")));
      log_indented(mem, str("\t"), sync.output.out);
      log_indented(mem, str("\t"), sync.output.err);
      return 1;
    }
    sp_log("sync {}", sp_fmt_cstr(target->triple));
  }

  sp_da(test_t) tests = tests_init(mem, tool, target, selection.names);
  sp_da_for(tests, it) {
    test_t* test = &tests[it];
    test->result = ps_run(mem, test->config, SP_NULLPTR);
    test->ran = true;
    if (ps_ok(&test->result)) {
      sp_log("ok {}", sp_fmt_str(test->name));
      continue;
    }
    sp_log("fail {} {}", sp_fmt_str(test->name), sp_fmt_str(ps_cause(mem, &test->result, "=")));
    sp_log("\t{}", sp_fmt_str(test->display));
    log_indented(mem, str("\t"), test->result.output.out);
    log_indented(mem, str("\t"), test->result.output.err);
  }

  write_failed_state(mem, tool, target, tests);
  tally_t t = tally(tests);
  sp_log("done {} ok {} fail", sp_fmt_uint(t.ok), sp_fmt_uint(t.fail));
  return t.fail ? 1 : 0;
}

typedef struct {
  const c8* prompt;
  u32 frame;
  sp_str_t status;
} runner_t;

static const u32 runner_frames [] = SP_PROMPT_SPINNER_BRAILLE_CIRCLING_DOT;

static void runner_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
  runner_t* runner = sp_cast(runner_t*, ctx->user_data);
  switch (event.kind) {
    case SP_PROMPT_EVENT_INIT: {
      runner->frame = 0;
      break;
    }
    case SP_PROMPT_EVENT_STATUS: {
      runner->status = event.status.value;
      break;
    }
    case SP_PROMPT_EVENT_CTRL_C:
    case SP_PROMPT_EVENT_ESCAPE: {
      sp_prompt_set_state(ctx, SP_PROMPT_STATE_CANCEL);
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
    case SP_PROMPT_EVENT_PROGRESS:
    case SP_PROMPT_EVENT_ABORT: {
      break;
    }
  }
}

static void runner_update(sp_prompt_ctx_t* ctx) {
  runner_t* runner = sp_cast(runner_t*, ctx->user_data);
  runner->frame = (runner->frame + 1) % sp_carr_len(runner_frames);
}

static void runner_render(sp_prompt_ctx_t* ctx) {
  runner_t* runner = sp_cast(runner_t*, ctx->user_data);
  if (ctx->state == SP_PROMPT_STATE_ACTIVE) {
    sp_prompt_style_t style = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = SP_ANSI_FG_BLUE_U8 };
    sp_prompt_render_line(ctx, sp_prompt_repeat(ctx, runner_frames[runner->frame], 1), style);
    sp_prompt_render_line(ctx, str("  "), sp_zero_s(sp_prompt_style_t));
  }
  else {
    sp_prompt_write_state_prefix(ctx);
  }
  sp_prompt_render_line(ctx, sp_cstr_as_str(runner->prompt), sp_zero_s(sp_prompt_style_t));
  ctx->cursor_col = 0;
  ctx->cursor_row++;

  if (!sp_str_empty(runner->status)) {
    sp_prompt_write_rail_prefix(ctx);
    sp_prompt_style_t dim = { .tag = SP_PROMPT_STYLE_ANSI, .ansi = SP_ANSI_FG_BRIGHT_BLACK_U8 };
    sp_prompt_render_line(ctx, runner->status, dim);
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

static void runner_run(sp_prompt_ctx_t* ctx, const c8* prompt) {
  runner_t runner = { .prompt = prompt };
  sp_prompt_run(ctx, (sp_prompt_widget_t) {
    .user_data = &runner,
    .on_event = runner_event,
    .on_update = runner_update,
    .render = runner_render,
  });
}

typedef struct {
  sp_prompt_ctx_t* ctx;
  sp_mem_t mem;
  sp_ps_config_t config;
  ps_result_t result;
} job_t;

static s32 job_thread(void* userdata) {
  job_t* job = sp_cast(job_t*, userdata);
  job->result = ps_run(job->mem, job->config, job->ctx);
  if (job->result.cancelled) return 0;
  prompt_finish(job->ctx);
  return 0;
}

typedef struct {
  sp_prompt_ctx_t* ctx;
  sp_mem_t mem;
  sp_da(test_t) tests;
} suite_t;

static s32 suite_thread(void* userdata) {
  suite_t* suite = sp_cast(suite_t*, userdata);
  u32 total = sp_cast(u32, sp_da_size(suite->tests));
  sp_da_for(suite->tests, it) {
    test_t* test = &suite->tests[it];
    sp_prompt_send_status_str(suite->ctx, sp_fmt(suite->mem, "{}/{} {}", sp_fmt_uint(it + 1), sp_fmt_uint(total), sp_fmt_str(test->name)).value);
    test->result = ps_run(suite->mem, test->config, suite->ctx);
    if (test->result.cancelled) return 0;
    test->ran = true;
  }
  sp_prompt_send_status_str(suite->ctx, sp_fmt(suite->mem, "{}/{}", sp_fmt_uint(total), sp_fmt_uint(total)).value);
  prompt_finish(suite->ctx);
  return 0;
}

static bool run_job(sp_prompt_ctx_t* ctx, job_t* job, const c8* prompt) {
  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, job_thread, job);
  runner_run(ctx, prompt);
  sp_thread_join(&thread);
  return !job->result.cancelled && !sp_prompt_cancelled(ctx);
}

typedef struct {
  const c8* stage;
  ps_result_t result;
} stage_failure_t;

static void report_stage(sp_mem_t mem, stage_failure_t* failure) {
  if (!failure->stage) return;
  sp_log("");
  sp_log("{.red} {}", sp_fmt_cstr(failure->stage), sp_fmt_str(ps_cause(mem, &failure->result, " ")));
  log_indented(mem, str("  "), failure->result.output.out);
  log_indented(mem, str("  "), failure->result.output.err);
}

static void report_failures(sp_mem_t mem, sp_da(test_t) tests) {
  sp_da_for(tests, it) {
    test_t* test = &tests[it];
    if (!test->ran || ps_ok(&test->result)) continue;
    sp_log("");
    sp_log("{.red} {} {.gray}", sp_fmt_cstr("fail"), sp_fmt_str(test->name), sp_fmt_str(ps_cause(mem, &test->result, " ")));
    sp_log("  {.gray}", sp_fmt_str(test->display));
    log_indented(mem, str("  "), test->result.output.out);
    log_indented(mem, str("  "), test->result.output.err);
  }
}

static s32 run_pretty(tool_t* tool, const target_t* target) {
  sp_mem_t mem = tool->mem;
  sp_prompt_ctx_t* ctx = sp_prompt_begin(tool->shared);
  if (!ctx) {
    if (target) return run_machine(tool, target);
    sp_log("error: no tty; pass a target");
    return 1;
  }

  sp_tm_timer_t timer = sp_tm_start_timer();
  sp_da(test_t) tests = sp_zero;
  stage_failure_t failure = sp_zero;
  s32 status = 0;
  bool interactive = target == SP_NULLPTR;

  if (target) {
    sp_prompt_intro(ctx, sp_str_to_cstr(mem, sp_fmt(mem, "sp · {}", sp_fmt_cstr(target->triple)).value));
  }
  else {
    sp_prompt_intro(ctx, "sp");
    sp_str_t last = sp_str_trim(state_read(mem, last_path()));
    sp_prompt_select_option_t options [sp_carr_len(targets)] = sp_zero;
    sp_carr_for(targets, it) {
      options[it] = (sp_prompt_select_option_t) {
        .label = targets[it].triple,
        .hint = targets[it].hint,
        .selected = sp_str_equal_cstr(last, targets[it].triple),
      };
    }
    if (!sp_prompt_select(ctx, (sp_prompt_select_t) {
      .prompt = "target",
      .options = options,
      .num_options = sp_carr_len(targets),
    })) {
      goto cancelled;
    }
    target = target_find_exact(sp_cstr_as_str(sp_prompt_get_str(ctx)));
    sp_assert(target);
    sp_fs_create_dir(str("build"));
    state_write(last_path(), sp_fmt(mem, "{}\n", sp_fmt_cstr(target->triple)).value);
  }

  if (!tool->args.no_build) {
    job_t job = {
      .ctx = ctx,
      .mem = tool->shared,
      .config = make_config(mem, tool, target),
    };
    if (!run_job(ctx, &job, "build")) goto cancelled;
    if (!ps_ok(&job.result)) {
      failure = (stage_failure_t) { .stage = "build failed", .result = job.result };
      sp_prompt_error(ctx, "build failed");
      status = 1;
      goto done;
    }
  }

  sp_str_t dir = tests_dir(mem, tool, target);
  if (!sp_fs_exists(dir)) {
    sp_prompt_error(ctx, sp_str_to_cstr(mem, sp_fmt(mem, "{} missing; run without --no-build", sp_fmt_str(dir)).value));
    status = 1;
    goto done;
  }

  selection_t selection = select_names(mem, tool, target);
  if (selection.no_failures) {
    sp_prompt_outro(ctx, "no failures recorded");
    goto done;
  }

  if (interactive && !tool->args.failed && sp_da_size(selection.names)) {
    sp_da(sp_prompt_select_option_t) options = sp_da_new(mem, sp_prompt_select_option_t);
    sp_da_for(selection.names, it) {
      sp_prompt_select_option_t option = { .label = sp_str_to_cstr(mem, selection.names[it]) };
      sp_da_push(options, option);
    }
    sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
      .prompt = "tests · enter runs all",
      .options = options,
      .num_options = sp_cast(u32, sp_da_size(options)),
      .max_visible = 12,
      .filter = true,
    });
    if (sp_prompt_cancelled(ctx)) goto cancelled;

    sp_da(sp_str_t) picked = sp_da_new(mem, sp_str_t);
    sp_da_for(selection.names, it) {
      if (options[it].selected) sp_da_push(picked, selection.names[it]);
    }
    if (sp_da_size(picked)) selection.names = picked;
  }

  if (sp_da_size(selection.names) == 0) {
    sp_prompt_error(ctx, "no tests match");
    status = 1;
    goto done;
  }

  if (target->sync != SYNC_NONE) {
    job_t job = {
      .ctx = ctx,
      .mem = tool->shared,
      .config = sync_config(mem, tool, target, selection.names),
    };
    if (!run_job(ctx, &job, "sync")) goto cancelled;
    if (!ps_ok(&job.result)) {
      failure = (stage_failure_t) { .stage = "sync failed", .result = job.result };
      sp_prompt_error(ctx, "sync failed");
      status = 1;
      goto done;
    }
  }

  tests = tests_init(mem, tool, target, selection.names);
  {
    suite_t suite = {
      .ctx = ctx,
      .mem = tool->shared,
      .tests = tests,
    };
    sp_thread_t thread = sp_zero;
    sp_thread_init(&thread, suite_thread, &suite);
    runner_run(ctx, "run");
    sp_thread_join(&thread);
  }

  if (sp_prompt_cancelled(ctx)) goto cancelled;

  {
    tally_t t = tally(tests);
    write_failed_state(mem, tool, target, tests);

    u64 ms = sp_tm_read_timer(&timer) / 1000000;
    sp_str_t elapsed = sp_fmt(mem, "{}.{}s", sp_fmt_uint(ms / 1000), sp_fmt_uint((ms % 1000) / 100)).value;
    sp_str_t outro = t.fail
      ? sp_fmt(mem, "{} ok · {} fail · {}", sp_fmt_uint(t.ok), sp_fmt_uint(t.fail), sp_fmt_str(elapsed)).value
      : sp_fmt(mem, "{} ok · {}", sp_fmt_uint(t.ok), sp_fmt_str(elapsed)).value;
    sp_prompt_outro(ctx, sp_str_to_cstr(mem, outro));
    status = t.fail ? 1 : 0;
  }
  goto done;

cancelled:
  sp_prompt_cancel(ctx, "cancelled");
  status = 130;

done:
  sp_prompt_end(ctx);
  report_stage(mem, &failure);
  report_failures(mem, tests);
  return status;
}

static void complete_target(sp_cli_complete_t* ctx) {
  sp_carr_for(targets, it) {
    sp_cli_candidate(ctx, sp_cstr_as_str(targets[it].triple), sp_cstr_as_str(targets[it].hint));
  }
}

static sp_str_t complete_typed_target(tool_t* tool) {
  u32 start = 0;
  sp_for(it, sp_cast(u32, tool->argc)) {
    if (sp_cstr_equal(tool->argv[it], "--")) {
      start = it + 2;
      break;
    }
  }
  if (!start || start >= sp_cast(u32, tool->argc)) return sp_zero_s(sp_str_t);

  bool skip = false;
  sp_for_range(it, start, sp_cast(u32, tool->argc) - 1) {
    sp_str_t word = sp_cstr_as_str(tool->argv[it]);
    if (skip) {
      skip = false;
      continue;
    }
    if (sp_str_equal(word, str("--output"))) {
      skip = true;
      continue;
    }
    if (sp_str_starts_with(word, str("-"))) continue;
    if (sp_str_equal(word, str("run"))) continue;
    return word;
  }
  return sp_zero_s(sp_str_t);
}

static void complete_globs(sp_cli_complete_t* ctx) {
  tool_t* tool = sp_cast(tool_t*, ctx->user_data);
  sp_str_t typed = complete_typed_target(tool);
  if (sp_str_empty(typed)) return;

  sp_da(const target_t*) matches = target_resolve(tool->mem, typed);
  if (sp_da_size(matches) != 1) return;

  const target_t* target = matches[0];
  sp_da(sp_str_t) names = tests_discover(tool->mem, tests_dir(tool->mem, tool, target), target_ext(target));
  sp_da_for(names, it) {
    sp_cli_candidate(ctx, names[it], sp_zero_s(sp_str_t));
  }
}

static sp_cli_result_t cmd_run(sp_cli_t* cli) {
  tool_t* tool = sp_cast(tool_t*, cli->user_data);
  sp_mem_t mem = tool->mem;

  bool machine = false;
  if (tool->args.output) {
    sp_str_t output = sp_cstr_as_str(tool->args.output);
    if (sp_str_equal(output, str("machine"))) {
      machine = true;
    }
    else if (!sp_str_equal(output, str("pretty"))) {
      return sp_cli_set_error_c(cli, "--output must be pretty or machine");
    }
  }
  else {
    machine = !sp_sys_is_tty(sp_sys_stdout);
  }

  tool->globs = cli->rest;
  while (tool->globs[tool->num_globs]) tool->num_globs++;
  sp_for(it, tool->num_globs) {
    sp_str_t glob = sp_cstr_as_str(tool->globs[it]);
    if (sp_str_starts_with(glob, str("-"))) {
      return sp_cli_set_error(cli, sp_fmt(mem, "options must come before globs: {}", sp_fmt_str(glob)).value);
    }
  }

  const target_t* target = SP_NULLPTR;
  if (tool->args.target) {
    sp_str_t query = sp_cstr_as_str(tool->args.target);
    if (sp_str_equal(query, str("-"))) {
      sp_str_t last = sp_str_trim(state_read(mem, last_path()));
      if (sp_str_empty(last)) {
        return sp_cli_set_error_c(cli, "no last target recorded");
      }
      target = target_find_exact(last);
      if (!target) {
        return sp_cli_set_error(cli, sp_fmt(mem, "last target {.quote} is unknown", sp_fmt_str(last)).value);
      }
    }
    else {
      sp_da(const target_t*) matches = target_resolve(mem, query);
      if (sp_da_size(matches) == 0) {
        return sp_cli_set_error(cli, sp_fmt(mem, "no target matches {.quote}", sp_fmt_str(query)).value);
      }
      if (sp_da_size(matches) > 1) {
        sp_io_dyn_mem_writer_t writer = sp_zero;
        sp_io_dyn_mem_writer_init(mem, &writer);
        sp_fmt_io(&writer.base, "{.quote} is ambiguous:", sp_fmt_str(query));
        sp_da_for(matches, it) {
          sp_fmt_io(&writer.base, " {}", sp_fmt_cstr(matches[it]->triple));
        }
        return sp_cli_set_error(cli, sp_io_dyn_mem_writer_as_str(&writer));
      }
      target = matches[0];
    }
  }
  else if (machine) {
    return sp_cli_set_error_c(cli, "a target is required with machine output");
  }

  if (target) {
    sp_fs_create_dir(str("build"));
    state_write(last_path(), sp_fmt(mem, "{}\n", sp_fmt_cstr(target->triple)).value);
  }

  tool->status = machine ? run_machine(tool, target) : run_pretty(tool, target);
  return SP_CLI_OK;
}

static sp_cli_result_t cmd_status(sp_cli_t* cli) {
  sp_unused(cli);
  u32 width = 0;
  sp_carr_for(targets, it) {
    width = sp_max(width, sp_cstr_as_str(targets[it].triple).len);
  }
  sp_carr_for(targets, it) {
    sp_log("{:<$ .cyan} {.gray}", sp_fmt_uint(width), sp_fmt_cstr(targets[it].triple), sp_fmt_cstr(targets[it].hint));
  }
  return SP_CLI_OK;
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  tool_t tool = {
    .mem = sp_mem_heap_as_allocator(heap),
    .shared = sp_mem_os_new(),
    .argv = args,
    .argc = num_args,
  };

  sp_cli_cmd_t run_cmd = {
    .name = "run",
    .summary = "build and run tests on a target; bare run is interactive",
    .opts = {
      {
        .name = "no-build",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "run existing binaries, skip make",
        .ptr = &tool.args.no_build,
      },
      {
        .name = "cpp",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "use the C++-compiled binaries (build/cpp)",
        .ptr = &tool.args.cpp,
      },
      {
        .name = "failed",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "rerun failures from this target's last run",
        .ptr = &tool.args.failed,
      },
      {
        .name = "output",
        .kind = SP_CLI_OPT_CSTR,
        .placeholder = "MODE",
        .summary = "pretty|machine; default: pretty on a tty",
        .ptr = &tool.args.output,
      },
    },
    .args = {
      {
        .name = "target",
        .arity = SP_CLI_ARG_OPTIONAL,
        .summary = "triple or token prefix (x-musl, windows, wasi); - = last used",
        .ptr = &tool.args.target,
        .complete = complete_target,
      },
      {
        .name = "globs",
        .arity = SP_CLI_ARG_REST,
        .summary = "test names, trailing * implied; default all",
        .complete = complete_globs,
      },
    },
    .handler = cmd_run,
  };

  sp_cli_cmd_t status_cmd = {
    .name = "status",
    .summary = "list targets and how they run",
    .handler = cmd_status,
  };

  sp_cli_cmd_t root = {
    .name = "sp",
    .summary = "build and run sp tests on any target",
    .commands = { &run_cmd, &status_cmd },
  };

  sp_cli_desc_t desc = {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &tool,
    .completer = "SP_COMPLETE",
  };

  switch (sp_cli_run(desc)) {
    case SP_CLI_OK: return tool.status;
    case SP_CLI_CONTINUE: return 0;
    case SP_CLI_HELP: return 0;
    case SP_CLI_ERR: return 1;
  }
  SP_UNREACHABLE_RETURN(1);
}
SP_MAIN(run)
