#include "sp.h"

#include "test.h"

#include "utest.h"

#if defined(SP_FREESTANDING) || defined(SP_WASM) || defined(SP_WIN32)
struct gdb {};
UTEST(gdb, unsupported) {
  UTEST_SKIP("unimplemented");
}
#else

#define GDB_TEST_MAX_NEEDLES 6

static const c8* GDB_SCRIPTS[] = {
  "sp_str_t.py",
  "sp_tm_epoch_t.py",
  "sp_da.py",
  "sp_ht.py",
  "sp_om.py",
  "sp_mem_arena.py",
  "sp_ps.py",
};

typedef struct {
  const c8* present [GDB_TEST_MAX_NEEDLES];
  const c8* absent [GDB_TEST_MAX_NEEDLES];
} gdb_expect_t;

typedef struct {
  const c8* mark;
  const c8* command;
  gdb_expect_t expect;
} gdb_test_t;

typedef struct gdb {
  sp_mem_arena_t* arena;
  sp_mem_t mem;
  sp_str_t fixture;
} sp_gdb;

static sp_str_t gdb_fixture_path(sp_mem_t mem) {
  sp_str_t exe = sp_fs_parent_path(sp_fs_get_exe_path(mem));
  sp_str_t fixture = sp_fs_join_path(mem, exe, sp_str_lit("gdb_fixture"));
  return sp_fs_replace_ext(mem, fixture, sp_os_get_executable_ext());
}

static bool gdb_available(void) {
  static s32 cached = -1;
  if (cached < 0) {
    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
    sp_ps_output_t probe = sp_ps_run_c(scratch.mem, (sp_ps_config_cstr_t) {
      .command = "gdb",
      .args = { "-nx", "-batch", "-ex", "python 1" },
      .io = { .err = { .mode = SP_PS_IO_MODE_NULL } },
    });
    cached = (probe.status.exit_code == 0) && sp_fs_exists(gdb_fixture_path(scratch.mem));
    sp_mem_end_scratch(scratch);
  }
  return cached != 0;
}

static void gdb_add_ex(sp_mem_t mem, sp_ps_config_t* config, const c8* command) {
  sp_ps_config_add_arg(mem, config, sp_str_lit("-ex"));
  sp_ps_config_add_arg(mem, config, sp_str_view(command));
}

static sp_str_t gdb_run(sp_gdb* fix, const c8* mark, const c8* command) {
  sp_ps_config_t config = sp_zero;
  config.command = sp_str_lit("gdb");
  config.io.err.mode = SP_PS_IO_MODE_NULL;

  sp_ps_config_add_arg(fix->mem, &config, sp_str_lit("-q"));
  sp_ps_config_add_arg(fix->mem, &config, sp_str_lit("-nx"));
  sp_ps_config_add_arg(fix->mem, &config, sp_str_lit("-batch"));
  gdb_add_ex(fix->mem, &config, "set debuginfod enabled off");
  gdb_add_ex(fix->mem, &config, "set pagination off");

  sp_carr_for(GDB_SCRIPTS, it) {
    gdb_add_ex(fix->mem, &config, sp_fmt_mem_cstr(fix->mem, "source {}/{}", sp_fmt_cstr(SP_GDB_TOOLS_DIR), sp_fmt_cstr(GDB_SCRIPTS[it])));
  }

  gdb_add_ex(fix->mem, &config, sp_fmt_mem_cstr(fix->mem, "break {}", sp_fmt_cstr(mark)));
  gdb_add_ex(fix->mem, &config, "run");
  gdb_add_ex(fix->mem, &config, command);
  gdb_add_ex(fix->mem, &config, "kill");
  sp_ps_config_add_arg(fix->mem, &config, fix->fixture);

  return sp_ps_run(fix->mem, config).out;
}

void run_gdb_test(s32* utest_result, sp_gdb* fix, gdb_test_t t) {
  sp_str_t out = gdb_run(fix, t.mark, t.command);

  sp_carr_for(t.expect.present, it) {
    if (!t.expect.present[it]) {
      break;
    }
    sp_str_t needle = sp_str_view(t.expect.present[it]);
    if (!sp_str_contains(out, needle)) {
      SP_TEST_REPORT("[{}] expected {.fg brightred} present in:\n{}\n", sp_fmt_cstr(t.mark), sp_fmt_str(needle), sp_fmt_str(out));
      SP_FAIL();
    }
  }

  sp_carr_for(t.expect.absent, it) {
    if (!t.expect.absent[it]) {
      break;
    }
    sp_str_t needle = sp_str_view(t.expect.absent[it]);
    if (sp_str_contains(out, needle)) {
      SP_TEST_REPORT("[{}] expected {.fg brightred} absent in:\n{}\n", sp_fmt_cstr(t.mark), sp_fmt_str(needle), sp_fmt_str(out));
      SP_FAIL();
    }
  }
}

UTEST_F_SETUP(gdb) {
  if (!gdb_available()) {
    UTEST_SKIP("gdb unavailable");
  }

  ut.arena = sp_mem_arena_new(sp_mem_os_new());
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  ut.fixture = gdb_fixture_path(ut.mem);
}

UTEST_F_TEARDOWN(gdb) {
  (void)utest_result;
  sp_mem_arena_destroy(ut.arena);
}

UTEST_F(gdb, da_null) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_da_null",
    .command = "da subject",
    .expect = {
      .present = { "(null)" },
      .absent = { "size =", "[0]" },
    },
  });
}

UTEST_F(gdb, da_empty) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_da_empty",
    .command = "da subject",
    .expect = {
      .present = { "size = 0", "(empty)" },
      .absent = { "[0]" },
    },
  });
}

UTEST_F(gdb, da_ints) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_da_ints",
    .command = "da subject",
    .expect = {
      .present = { "size = 5, capacity = 16", "[0] = 0", "[4] = 40" },
    },
  });
}

UTEST_F(gdb, da_cstrs) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_da_cstrs",
    .command = "da subject",
    .expect = {
      .present = { "size = 2", "[0] = \"hello\"", "[1] = \"world\"" },
    },
  });
}

UTEST_F(gdb, da_usage) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "main",
    .command = "da",
    .expect = {
      .present = { "Usage: da <variable>" },
    },
  });
}

UTEST_F(gdb, da_bad_symbol) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "main",
    .command = "da no_such_symbol",
    .expect = {
      .present = { "Error:" },
      .absent = { "size =" },
    },
  });
}

UTEST_F(gdb, ht_null) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_null",
    .command = "print subject",
    .expect = {
      .present = { "(null) sp_ht" },
      .absent = { "= {" },
    },
  });
}

UTEST_F(gdb, ht_empty) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_empty",
    .command = "print subject",
    .expect = {
      .present = { "sp_ht of length 0" },
      .absent = { "= {" },
    },
  });
}

UTEST_F(gdb, ht_ints) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_ints",
    .command = "print subject",
    .expect = {
      .present = { "sp_ht of length 3", "[7] = 8", "[69] = 420", "[1] = 2" },
    },
  });
}

UTEST_F(gdb, ht_cstr_vals) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_cstrval",
    .command = "print subject",
    .expect = {
      .present = { "sp_ht of length 2", "[1] =", "\"one\"", "\"two\"" },
    },
  });
}

UTEST_F(gdb, ht_strkey) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_strkey",
    .command = "print subject",
    .expect = {
      .present = { "[\"hello\"] = 42", "[\"world\"] = 99" },
    },
  });
}

UTEST_F(gdb, ht_tombstone) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_tombstone",
    .command = "print subject",
    .expect = {
      .present = { "sp_ht of length 2", "[1] = 10", "[3] = 30" },
      .absent = { "= 20", "[2] =" },
    },
  });
}

UTEST_F(gdb, ht_struct) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ht_struct",
    .command = "print subject",
    .expect = {
      .present = { "sp_ht of length 1", "[{x = 1, y = 2}] = {x = 3, y = 4}" },
    },
  });
}

UTEST_F(gdb, om_null) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_om_null",
    .command = "print subject",
    .expect = {
      .present = { "(null) sp_om" },
      .absent = { "= {" },
    },
  });
}

UTEST_F(gdb, om_strkey) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_om_strkey",
    .command = "print subject",
    .expect = {
      .present = { "sp_om of length 3", "[\"alpha\"] = 1", "[\"beta\"] = 2", "[\"gamma\"] = 3" },
    },
  });
}

UTEST_F(gdb, om_structval) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_om_structval",
    .command = "print subject",
    .expect = {
      .present = { "sp_om of length 2", "[\"origin\"] = {x = 10, y = 20}", "[\"corner\"] = {x = 30, y = 40}" },
    },
  });
}

UTEST_F(gdb, str_value) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_str",
    .command = "print subject",
    .expect = {
      .present = { "\"hello world\"" },
    },
  });
}

UTEST_F(gdb, tm_epoch) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_tm",
    .command = "print subject",
    .expect = {
      .present = { "2023-11-14T22:13:20.123456789Z" },
    },
  });
}

UTEST_F(gdb, arena_printer) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_arena",
    .command = "print subject",
    .expect = {
      .present = { "sp_mem_arena_t {", "2 blocks" },
    },
  });
}

UTEST_F(gdb, arena_command) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_arena",
    .command = "arena subject",
    .expect = {
      .present = { "across 2 blocks", "[0]", "[1]" },
    },
  });
}

UTEST_F(gdb, ps_config) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_ps_config",
    .command = "print subject",
    .expect = {
      .present = { "command = \"ls\"", "cwd = \"/home\"", "SP_PS_IO_MODE_NULL" },
    },
  });
}

UTEST_F(gdb, env_var) {
  run_gdb_test(&ur, &ut, (gdb_test_t) {
    .mark = "brk_env_var",
    .command = "print subject",
    .expect = {
      .present = { "key = \"PATH\"", "value = \"/usr/bin\"" },
    },
  });
}

#endif

SP_TEST_MAIN()
