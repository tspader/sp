#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"

typedef enum {
  OS_LINUX,
  OS_MACOS,
  OS_WINDOWS,
  OS_WASM,
} os_t;

typedef enum {
  ARCH_X86_64,
  ARCH_AARCH64,
  ARCH_WASM32,
} arch_t;

typedef enum {
  LAUNCH_NONE,
  LAUNCH_DIRECT,
  LAUNCH_WASMTIME,
} launch_t;

typedef struct {
  const c8* triple;
  os_t os;
  arch_t arch;
  bool native;
} target_t;

typedef struct {
  u32 passed;
  u32 failed;
} tally_t;

#if defined(SP_WIN32)
  #define HOST_OS OS_WINDOWS
#elif defined(SP_MACOS)
  #define HOST_OS OS_MACOS
#else
  #define HOST_OS OS_LINUX
#endif

#if defined(SP_ARM64)
  #define HOST_ARCH ARCH_AARCH64
#else
  #define HOST_ARCH ARCH_X86_64
#endif

static const target_t targets [] = {
  { .triple = "x86_64-linux-gnu",         .os = OS_LINUX,   .arch = ARCH_X86_64 },
  { .triple = "x86_64-linux-musl",        .os = OS_LINUX,   .arch = ARCH_X86_64 },
  { .triple = "x86_64-linux-none",        .os = OS_LINUX,   .arch = ARCH_X86_64 },
  { .triple = "aarch64-linux-gnu",        .os = OS_LINUX,   .arch = ARCH_AARCH64 },
  { .triple = "aarch64-linux-musl",       .os = OS_LINUX,   .arch = ARCH_AARCH64 },
  { .triple = "aarch64-linux-none",       .os = OS_LINUX,   .arch = ARCH_AARCH64 },
  { .triple = "aarch64-macos-apple",      .os = OS_MACOS,   .arch = ARCH_AARCH64, .native = true },
  { .triple = "x86_64-windows-gnu",       .os = OS_WINDOWS, .arch = ARCH_X86_64 },
  { .triple = "wasm32-wasi-musl",         .os = OS_WASM,    .arch = ARCH_WASM32 },
  { .triple = "wasm32-freestanding-none", .os = OS_WASM,    .arch = ARCH_WASM32 },
};

static const target_t* find_target(sp_str_t triple) {
  sp_carr_for(targets, it) {
    if (sp_str_equal_cstr(triple, targets[it].triple)) {
      return &targets[it];
    }
  }
  return SP_NULLPTR;
}

static sp_str_t exe_ext(os_t os) {
  switch (os) {
    case OS_LINUX:
    case OS_MACOS:   return sp_str_lit("");
    case OS_WINDOWS: return sp_str_lit("exe");
    case OS_WASM:    return sp_str_lit("wasm");
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

static launch_t launch_of(const target_t* target) {
  if (target->os == OS_WASM) {
    return LAUNCH_WASMTIME;
  }
  if (target->os == HOST_OS && target->arch == HOST_ARCH) {
    return LAUNCH_DIRECT;
  }
  return LAUNCH_NONE;
}

static s32 compare_paths(const void* a, const void* b) {
  return sp_str_compare_alphabetical(*(const sp_str_t*)a, *(const sp_str_t*)b);
}

static sp_da(sp_str_t) find_tests(sp_mem_t mem, sp_str_t cwd, const target_t* target) {
  sp_da(sp_str_t) tests = sp_da_new(mem, sp_str_t);
  sp_str_t root = sp_fmt(mem, "{}/build/{}/debug/test", sp_fmt_str(cwd), sp_fmt_cstr(target->triple)).value;

  sp_sys_dir_t dir = sp_zero;
  if (sp_fs_open_dir_at(sp_path_cwd(root), &dir)) {
    return tests;
  }

  sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) {
    if (entry.kind != SP_FS_KIND_FILE) {
      continue;
    }
    if (!sp_str_equal(sp_fs_get_ext(entry.name), exe_ext(target->os))) {
      continue;
    }
    sp_da_push(tests, sp_fs_join_path(mem, root, entry.name));
  }
  sp_fs_dir_it_close(&iter);
  sp_fs_close_dir(dir);

  sp_da_sort(tests, compare_paths);
  return tests;
}

static s32 build(sp_mem_t mem, const target_t* target) {
  sp_ps_t ps = sp_ps_create(mem, (sp_ps_config_t) {
    .command = sp_str_lit("spn"),
    .args = { sp_str_lit("build"), sp_str_lit("--target"), sp_cstr_as_str(target->triple) },
  });
  if (!ps.os) {
    sp_log("{:>12 .bold .red} ▐ could not run spn; is it on your PATH?", sp_fmt_cstr("Failed"));
    return 1;
  }

  sp_ps_status_t status = sp_ps_wait(&ps);
  sp_ps_free(&ps);
  return status.exit_code;
}

static sp_ps_output_t run_test(sp_mem_t mem, launch_t launch, sp_str_t cwd, sp_str_t test) {
  sp_ps_config_t config = {
    .cwd = cwd,
    .io = { .err = { .mode = SP_PS_IO_MODE_REDIRECT } },
  };

  switch (launch) {
    case LAUNCH_DIRECT: {
      config.command = test;
      break;
    }
    case LAUNCH_WASMTIME: {
      config.command = sp_str_lit("wasmtime");
      config.args[0] = sp_str_lit("run");
      config.args[1] = sp_str_lit("--dir");
      config.args[2] = cwd;
      config.args[3] = test;
      break;
    }
    case LAUNCH_NONE: {
      break;
    }
  }

  return sp_ps_run(mem, config);
}

static void check(sp_mem_t mem, sp_str_t cwd, const target_t* target, tally_t* tally) {
  if (target->native && target->os != HOST_OS) {
    sp_log("{:>12 .bold .yellow} ▐ {} only builds on its own OS", sp_fmt_cstr("Skipped"), sp_fmt_cstr(target->triple));
    return;
  }

  if (build(mem, target)) {
    tally->failed++;
    return;
  }

  launch_t launch = launch_of(target);
  if (launch == LAUNCH_NONE) {
    sp_log("{:>12 .bold .yellow} ▐ {} can't run on this machine", sp_fmt_cstr("Built"), sp_fmt_cstr(target->triple));
    return;
  }

  sp_da(sp_str_t) tests = find_tests(mem, cwd, target);
  if (sp_da_empty(tests)) {
    tally->failed++;
    sp_log("{:>12 .bold .red} ▐ {} built no tests", sp_fmt_cstr("Failed"), sp_fmt_cstr(target->triple));
    return;
  }

  sp_da_for(tests, it) {
    sp_str_t name = sp_fs_get_stem(tests[it]);
    sp_tm_point_t start = sp_tm_now_point();
    sp_ps_output_t output = run_test(mem, launch, cwd, tests[it]);
    u64 elapsed = sp_tm_ns_to_ms(sp_tm_point_diff(sp_tm_now_point(), start));

    if (output.status.exit_code) {
      tally->failed++;
      sp_log("{}", sp_fmt_str(sp_str_trim(output.out)));
      sp_log("{:>12 .bold .red} ▐ {} exited with code {}", sp_fmt_cstr("Failed"), sp_fmt_str(name), sp_fmt_int(output.status.exit_code));
    }
    else {
      tally->passed++;
      sp_log("{:>12 .bold .green} ▐ {} in {} ms", sp_fmt_cstr("Passed"), sp_fmt_str(name), sp_fmt_uint(elapsed));
    }
    sp_ps_output_free(mem, &output);
  }
}

static sp_cli_result_t run_check(sp_cli_t* cli) {
  tally_t* tally = sp_cast(tally_t*, cli->user_data);
  sp_mem_t mem = sp_mem_heap_as_allocator(sp_mem_heap_new());

  sp_da(const target_t*) selected = sp_da_new(mem, const target_t*);
  for (u32 it = 0; cli->rest[it]; it++) {
    const target_t* target = find_target(sp_cstr_as_str(cli->rest[it]));
    if (!target) {
      return sp_cli_set_error(cli, sp_fmt(mem, "{} is not a target this tool knows", sp_fmt_cstr(cli->rest[it])).value);
    }
    sp_da_push(selected, target);
  }
  if (sp_da_empty(selected)) {
    sp_carr_for(targets, it) {
      sp_da_push(selected, &targets[it]);
    }
  }

  sp_str_t cwd = sp_zero;
  sp_fs_get_cwd_path(mem, &cwd);

  sp_da_for(selected, it) {
    check(mem, cwd, selected[it], tally);
  }

  sp_log("{:>12 .bold} ▐ {} passed, {} failed", sp_fmt_cstr("Checked"), sp_fmt_uint(tally->passed), sp_fmt_uint(tally->failed));
  return SP_CLI_OK;
}

static s32 run(s32 num_args, const c8** args) {
  tally_t tally = sp_zero;
  sp_cli_cmd_t root = {
    .name = "check",
    .summary = "Build every target with spn, and run the tests of the ones this machine can",
    .args = {
      {
        .name = "target",
        .arity = SP_CLI_ARG_REST,
        .summary = "Triples to check, like x86_64-linux-musl; all of them when omitted",
      },
    },
    .handler = run_check,
  };

  s32 code = sp_cli_main((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &tally,
  });
  return code || tally.failed;
}
SP_MAIN(run)
