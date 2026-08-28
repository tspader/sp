#include "cli.h"

typedef struct {
  sp_cli_err_kind_t err;
  const c8* err_name;
  const c8* err_value;
  const c8* cmd;
  bool help;
  bool flags [CLI_TEST_MAX_BINDS];
  const c8* strs [CLI_TEST_MAX_BINDS];
  const c8* views [CLI_TEST_MAX_BINDS];
  s64 nums [CLI_TEST_MAX_BINDS];
  sp_cli_choice_t choices [CLI_TEST_MAX_BINDS];
  const c8* rest [CLI_TEST_MAX_ARGS];
} cli_parse_expect_t;

typedef struct {
  const c8* args [CLI_TEST_MAX_ARGS];
  cli_binds_t binds;
  sp_cli_cmd_t cmd;
  cli_parse_expect_t expect;
} cli_parse_test_t;

CLI_TEST_FIXTURE(cli_parse)

static void run_cli_parse_test(s32* utest_result, sp_mem_t mem, cli_parse_test_t t) {
  cli_binds = t.binds;

  if (!t.cmd.commands[0] && !t.cmd.handler) t.cmd.handler = cli_handler_ok;

  const c8* argv [CLI_TEST_MAX_ARGS + 2];
  u32 n = cli_count_args(t.args);
  argv[0] = "test";
  for (u32 it = 0; it < n; it++) argv[it + 1] = t.args[it];
  argv[n + 1] = SP_NULLPTR;

  sp_cli_t cli;
  sp_cli_parse((sp_cli_desc_t) {
    .root = &t.cmd,
    .args = argv,
    .num_args = sp_cast(s32, n + 1),
  }, &cli);

  if (t.expect.err) {
    EXPECT_EQ(SP_CLI_ERR, cli.status);
    EXPECT_EQ(t.expect.err, cli.err.kind);
    if (t.expect.err_name) {
      SP_EXPECT_STR_EQ_CSTR(cli.err.name, t.expect.err_name);
    }
    if (t.expect.err_value) {
      SP_EXPECT_STR_EQ_CSTR(cli.err.value, t.expect.err_value);
    }
  }
  else if (t.expect.help) {
    EXPECT_EQ(SP_CLI_HELP, cli.status);
    EXPECT_EQ(SP_CLI_ERR_NONE, cli.err.kind);
  }
  else {
    EXPECT_EQ(SP_CLI_OK, cli.status);
    EXPECT_EQ(SP_CLI_ERR_NONE, cli.err.kind);
  }

  if (t.expect.cmd) {
    EXPECT_NE(SP_NULLPTR, (void*)cli.cmd);
    if (cli.cmd) {
      SP_EXPECT_STR_EQ_CSTR(sp_str_view(cli.cmd->name), t.expect.cmd);
    }
  }

  sp_carr_for(t.expect.flags, it) {
    EXPECT_EQ(t.expect.flags[it], cli_binds.flags[it]);
  }
  sp_carr_for(t.expect.strs, it) {
    SP_EXPECT_STR_EQ_CSTR(sp_cstr_as_str(cli_binds.strs[it]), t.expect.strs[it] ? t.expect.strs[it] : "");
  }
  sp_carr_for(t.expect.views, it) {
    if (!t.expect.views[it]) continue;
    SP_EXPECT_STR_EQ_CSTR(cli_binds.views[it], t.expect.views[it]);
  }
  sp_carr_for(t.expect.nums, it) {
    EXPECT_EQ(t.expect.nums[it], cli_binds.nums[it]);
  }
  sp_carr_for(t.expect.choices, it) {
    if (!t.expect.choices[it].name) continue;
    SP_EXPECT_STR_EQ_CSTR(sp_cstr_as_str(cli_binds.choices[it].name), t.expect.choices[it].name);
    EXPECT_EQ(t.expect.choices[it].value, cli_binds.choices[it].value);
  }

  EXPECT_EQ(cli_count_args(t.expect.rest), cli_count_args(cli.rest));
  sp_carr_for(t.expect.rest, it) {
    if (!t.expect.rest[it]) break;
    if (!cli.rest[it]) break;
    SP_EXPECT_STR_EQ_CSTR(sp_cstr_as_str(cli.rest[it]), t.expect.rest[it]);
  }
}

UTEST_F(cli_parse, empty_args) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .cmd = { .name = "test" },
    .expect = { .cmd = "test" },
  });
}

UTEST_F(cli_parse, required_arg_present) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "foo" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "foo" },
    },
  });
}

UTEST_F(cli_parse, required_arg_missing) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_MISSING_ARG,
      .err_name = "path",
    },
  });
}

UTEST_F(cli_parse, optional_arg_present) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "bar" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .arity = SP_CLI_ARG_OPTIONAL, .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "bar" },
    },
  });
}

UTEST_F(cli_parse, optional_arg_missing) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .arity = SP_CLI_ARG_OPTIONAL, .ptr = &cli_binds.strs[0] },
      },
    },
  });
}

UTEST_F(cli_parse, multiple_positionals) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "first", "second" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "one", .ptr = &cli_binds.strs[0] },
        { .name = "two", .ptr = &cli_binds.strs[1] },
      },
    },
    .expect = {
      .strs = { "first", "second" },
    },
  });
}

UTEST_F(cli_parse, unexpected_positional) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "a", "b" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "one", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_UNEXPECTED_ARG,
      .err_value = "b",
      .strs = { "a" },
    },
  });
}

UTEST_F(cli_parse, dash_is_positional) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "-" },
    },
  });
}

UTEST_F(cli_parse, bool_opt_brief) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-v" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, bool_opt_long) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, bool_opt_cluster) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-vf" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
        { .brief = 'f', .name = "force", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[1] },
      },
    },
    .expect = {
      .flags = { true, true },
    },
  });
}

UTEST_F(cli_parse, cluster_unknown_brief) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-vx" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_UNKNOWN_BRIEF,
      .err_name = "x",
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, string_opt_brief) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-o", "foo" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "foo" },
    },
  });
}

UTEST_F(cli_parse, string_opt_brief_attached) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-ofoo" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "foo" },
    },
  });
}

UTEST_F(cli_parse, string_opt_long_eq) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output=bar" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "bar" },
    },
  });
}

UTEST_F(cli_parse, string_opt_long_space) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output", "baz" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "baz" },
    },
  });
}

UTEST_F(cli_parse, string_opt_missing_value_at_end) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_MISSING_VALUE,
      .err_name = "output",
    },
  });
}

UTEST_F(cli_parse, string_opt_missing_value_before_opt) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output", "--verbose" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_MISSING_VALUE,
      .err_name = "output",
    },
  });
}

UTEST_F(cli_parse, string_opt_missing_value_before_brief) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output", "-v" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[0] },
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_MISSING_VALUE,
      .err_name = "output",
    },
  });
}

UTEST_F(cli_parse, int_opt) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--jobs", "4" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'j', .name = "jobs", .kind = SP_CLI_OPT_S64, .ptr = &cli_binds.nums[0] },
      },
    },
    .expect = {
      .nums = { 4 },
    },
  });
}

UTEST_F(cli_parse, int_opt_negative_eq) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--jobs=-4" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'j', .name = "jobs", .kind = SP_CLI_OPT_S64, .ptr = &cli_binds.nums[0] },
      },
    },
    .expect = {
      .nums = { -4 },
    },
  });
}

UTEST_F(cli_parse, int_opt_invalid) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--jobs", "abc" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'j', .name = "jobs", .kind = SP_CLI_OPT_S64, .ptr = &cli_binds.nums[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_name = "jobs",
      .err_value = "abc",
    },
  });
}

UTEST_F(cli_parse, unknown_long_opt) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--bogus" },
    .cmd = { .name = "test" },
    .expect = {
      .err = SP_CLI_ERR_UNKNOWN_OPT,
      .err_name = "bogus",
    },
  });
}

UTEST_F(cli_parse, brief_after_long_only_opt) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-m", "debug" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'f', .name = "force", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
        { .name = "bin", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[1] },
        { .brief = 'm', .name = "mode", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "debug" },
    },
  });
}

UTEST_F(cli_parse, opt_after_positional) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "input.c", "--verbose" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "file", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .flags = { true },
      .strs = { "input.c" },
    },
  });
}

UTEST_F(cli_parse, mixed_opts_and_args) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose", "input.c", "--output", "out.o" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
        { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[1] },
      },
      .args = {
        { .name = "file", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .flags = { true },
      .strs = { "input.c", "out.o" },
    },
  });
}

UTEST_F(cli_parse, command_resolved) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "build" },
    .cmd = { .name = "root", .commands = { &build } },
    .expect = { .cmd = "build" },
  });
}

UTEST_F(cli_parse, command_unknown) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "bogus" },
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .err = SP_CLI_ERR_UNKNOWN_COMMAND,
      .err_name = "bogus",
    },
  });
}

UTEST_F(cli_parse, command_missing_is_help) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .cmd = "root",
      .help = true,
    },
  });
}

UTEST_F(cli_parse, command_missing_with_root_handler) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .cmd = { .name = "root", .commands = { &build }, .handler = cli_handler_ok },
    .expect = { .cmd = "root" },
  });
}

UTEST_F(cli_parse, opts_at_each_level) {
  sp_cli_cmd_t install = {
    .name = "install",
    .opts = {
      { .brief = 'f', .name = "force", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[1] },
    },
    .handler = cli_handler_ok,
  };
  sp_cli_cmd_t tool = { .name = "tool", .commands = { &install } };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose", "tool", "install", "--force" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .commands = { &tool },
    },
    .expect = {
      .cmd = "install",
      .flags = { true, true },
    },
  });
}

UTEST_F(cli_parse, command_positionals) {
  sp_cli_cmd_t install = {
    .name = "install",
    .args = {
      { .name = "package", .ptr = &cli_binds.strs[0] },
      { .name = "version", .arity = SP_CLI_ARG_OPTIONAL, .ptr = &cli_binds.strs[1] },
    },
    .handler = cli_handler_ok,
  };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "install", "sp", "1.0.0" },
    .cmd = { .name = "root", .commands = { &install } },
    .expect = {
      .cmd = "install",
      .strs = { "sp", "1.0.0" },
    },
  });
}

UTEST_F(cli_parse, error_reports_deepest_command) {
  sp_cli_cmd_t install = {
    .name = "install",
    .args = {
      { .name = "package", .ptr = &cli_binds.strs[0] },
    },
    .handler = cli_handler_ok,
  };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "install" },
    .cmd = { .name = "root", .commands = { &install } },
    .expect = {
      .err = SP_CLI_ERR_MISSING_ARG,
      .err_name = "package",
      .cmd = "install",
    },
  });
}

UTEST_F(cli_parse, parent_opt_after_command) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "build", "--verbose" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .commands = { &build },
    },
    .expect = {
      .cmd = "build",
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, parent_brief_after_command) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "build", "-v" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .commands = { &build },
    },
    .expect = {
      .cmd = "build",
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, child_opt_shadows_parent) {
  sp_cli_cmd_t build = {
    .name = "build",
    .opts = {
      { .brief = 'm', .name = "mode", .ptr = &cli_binds.strs[1] },
    },
    .handler = cli_handler_ok,
  };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "build", "--mode", "debug" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'm', .name = "mode", .ptr = &cli_binds.strs[0] },
      },
      .commands = { &build },
    },
    .expect = {
      .cmd = "build",
      .strs = { "", "debug" },
    },
  });
}

UTEST_F(cli_parse, command_at_max_depth) {
  sp_cli_cmd_t c = { .name = "c", .handler = cli_handler_ok };
  sp_cli_cmd_t b = { .name = "b", .commands = { &c } };
  sp_cli_cmd_t a = { .name = "a", .commands = { &b } };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "a", "b", "c" },
    .cmd = { .name = "root", .commands = { &a } },
    .expect = { .cmd = "c" },
  });
}

UTEST_F(cli_parse, command_beyond_max_depth) {
  sp_cli_cmd_t d = { .name = "d", .handler = cli_handler_ok };
  sp_cli_cmd_t c = { .name = "c", .commands = { &d } };
  sp_cli_cmd_t b = { .name = "b", .commands = { &c } };
  sp_cli_cmd_t a = { .name = "a", .commands = { &b } };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "a", "b", "c", "d" },
    .cmd = { .name = "root", .commands = { &a } },
    .expect = {
      .err = SP_CLI_ERR_MAX_DEPTH,
      .err_name = "d",
      .cmd = "c",
    },
  });
}

UTEST_F(cli_parse, unknown_opt_misses_all_scopes) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "build", "--bogus" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .commands = { &build },
    },
    .expect = {
      .err = SP_CLI_ERR_UNKNOWN_OPT,
      .err_name = "bogus",
    },
  });
}

UTEST_F(cli_parse, bool_opt_explicit_true) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose=true" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, bool_opt_explicit_false) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose=false" },
    .binds = {
      .flags = { true },
    },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
  });
}

UTEST_F(cli_parse, bool_opt_invalid_value) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose=banana" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_name = "verbose",
      .err_value = "banana",
    },
  });
}

UTEST_F(cli_parse, bool_opt_does_not_consume_token) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose", "true" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .flags = { true },
      .strs = { "true" },
    },
  });
}

UTEST_F(cli_parse, double_dash_ends_options) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--", "--verbose" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .strs = { "--verbose" },
    },
  });
}

UTEST_F(cli_parse, double_dash_ends_commands) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--", "build" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
      .commands = { &build },
      .handler = cli_handler_ok,
    },
    .expect = {
      .cmd = "root",
      .rest = { "build" },
    },
  });
}

UTEST_F(cli_parse, rest_captures_trailing) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "script", "foo", "--bar" },
    .cmd = {
      .name = "run",
      .args = {
        { .name = "entry", .ptr = &cli_binds.strs[0] },
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
    },
    .expect = {
      .strs = { "script" },
      .rest = { "foo", "--bar" },
    },
  });
}

UTEST_F(cli_parse, rest_empty) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "script" },
    .cmd = {
      .name = "run",
      .args = {
        { .name = "entry", .ptr = &cli_binds.strs[0] },
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
    },
    .expect = {
      .strs = { "script" },
    },
  });
}

UTEST_F(cli_parse, rest_after_double_dash) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "script", "--", "--verbose" },
    .cmd = {
      .name = "run",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "entry", .ptr = &cli_binds.strs[0] },
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
    },
    .expect = {
      .strs = { "script" },
      .rest = { "--verbose" },
    },
  });
}

UTEST_F(cli_parse, rest_stops_option_parsing) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "script", "x", "--verbose" },
    .cmd = {
      .name = "run",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "entry", .ptr = &cli_binds.strs[0] },
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
    },
    .expect = {
      .strs = { "script" },
      .rest = { "x", "--verbose" },
    },
  });
}

UTEST_F(cli_parse, opts_before_rest_parse) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--verbose", "script", "a" },
    .cmd = {
      .name = "run",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
      .args = {
        { .name = "entry", .ptr = &cli_binds.strs[0] },
        { .name = "args", .arity = SP_CLI_ARG_REST },
      },
    },
    .expect = {
      .flags = { true },
      .strs = { "script" },
      .rest = { "a" },
    },
  });
}

UTEST_F(cli_parse, help_long) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--help" },
    .cmd = { .name = "test" },
    .expect = {
      .cmd = "test",
      .help = true,
    },
  });
}

UTEST_F(cli_parse, help_brief) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-h" },
    .cmd = { .name = "test" },
    .expect = {
      .cmd = "test",
      .help = true,
    },
  });
}

UTEST_F(cli_parse, help_skips_validation) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "-h" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "path", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .help = true,
    },
  });
}

UTEST_F(cli_parse, help_reports_deepest_command) {
  sp_cli_cmd_t install = {
    .name = "install",
    .args = {
      { .name = "package", .ptr = &cli_binds.strs[0] },
    },
    .handler = cli_handler_ok,
  };

  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "install", "--help" },
    .cmd = { .name = "root", .commands = { &install } },
    .expect = {
      .cmd = "install",
      .help = true,
    },
  });
}

UTEST_F(cli_parse, binds_views_into_args) {
  const c8* package = "sp";
  const c8* mode = "debug";
  const c8* args [] = { "prog", "--mode", mode, package, "x", SP_NULLPTR };
  sp_cli_cmd_t cmd = {
    .name = "run",
    .opts = {
      { .name = "mode", .ptr = &cli_binds.strs[1] },
    },
    .args = {
      { .name = "entry", .ptr = &cli_binds.strs[0] },
      { .name = "args", .arity = SP_CLI_ARG_REST },
    },
    .handler = cli_handler_ok,
  };

  cli_binds = sp_zero_s(cli_binds_t);
  sp_cli_t cli;
  sp_cli_parse((sp_cli_desc_t) {
    .root = &cmd, .args = args, .num_args = sp_carr_len(args) - 1,
  }, &cli);

  EXPECT_EQ(SP_CLI_OK, cli.status);
  EXPECT_EQ(package, cli_binds.strs[0]);
  EXPECT_EQ(mode, cli_binds.strs[1]);
  EXPECT_EQ(args + 4, cli.rest);
  EXPECT_EQ(SP_NULLPTR, (void*)cli.rest[1]);
}

UTEST_F(cli_parse, attached_values_are_argv_tails) {
  const c8* eq = "--mode=release";
  const c8* cluster = "-odist";
  const c8* args [] = { "prog", eq, cluster, SP_NULLPTR };
  sp_cli_cmd_t cmd = {
    .name = "run",
    .opts = {
      { .name = "mode", .ptr = &cli_binds.strs[0] },
      { .brief = 'o', .name = "output", .ptr = &cli_binds.strs[1] },
    },
    .handler = cli_handler_ok,
  };

  cli_binds = sp_zero_s(cli_binds_t);
  sp_cli_t cli;
  sp_cli_parse((sp_cli_desc_t) {
    .root = &cmd, .args = args, .num_args = sp_carr_len(args) - 1,
  }, &cli);

  EXPECT_EQ(SP_CLI_OK, cli.status);
  EXPECT_EQ(eq + 7, cli_binds.strs[0]);
  EXPECT_EQ(cluster + 2, cli_binds.strs[1]);
}

UTEST_F(cli_parse, string_opt_empty_value_errors) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--output=" },
    .binds = {
      .strs = { "unset" },
    },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "output", .ptr = &cli_binds.strs[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_MISSING_VALUE,
      .err_name = "output",
      .strs = { "unset" },
    },
  });
}

UTEST_F(cli_parse, declared_help_opt_wins) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--help" },
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = 'h', .name = "help", .kind = SP_CLI_OPT_BOOLEAN, .ptr = &cli_binds.flags[0] },
      },
    },
    .expect = {
      .cmd = "test",
      .flags = { true },
    },
  });
}

UTEST_F(cli_parse, str_opt_long_space) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode", "release" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_STR, .ptr = &cli_binds.views[0] },
      },
    },
    .expect = {
      .views = { "release" },
    },
  });
}

UTEST_F(cli_parse, str_opt_long_eq) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode=release" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_STR, .ptr = &cli_binds.views[0] },
      },
    },
    .expect = {
      .views = { "release" },
    },
  });
}

UTEST_F(cli_parse, str_opt_attached_value_is_argv_tail) {
  const c8* eq = "--mode=release";
  const c8* args [] = { "prog", eq, SP_NULLPTR };
  sp_cli_cmd_t cmd = {
    .name = "run",
    .opts = {
      { .name = "mode", .kind = SP_CLI_OPT_STR, .ptr = &cli_binds.views[0] },
    },
    .handler = cli_handler_ok,
  };

  cli_binds = sp_zero_s(cli_binds_t);
  sp_cli_t cli;
  sp_cli_parse((sp_cli_desc_t) {
    .root = &cmd, .args = args, .num_args = sp_carr_len(args) - 1,
  }, &cli);

  EXPECT_EQ(SP_CLI_OK, cli.status);
  EXPECT_EQ(eq + 7, cli_binds.views[0].data);
  SP_EXPECT_STR_EQ_CSTR(cli_binds.views[0], "release");
}

UTEST_F(cli_parse, str_arg) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "release" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "version", .kind = SP_CLI_OPT_STR, .ptr = &cli_binds.views[0] },
      },
    },
    .expect = {
      .views = { "release" },
    },
  });
}

UTEST_F(cli_parse, int_arg) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "4" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "jobs", .kind = SP_CLI_OPT_S64, .ptr = &cli_binds.nums[0] },
      },
    },
    .expect = {
      .nums = { 4 },
    },
  });
}

UTEST_F(cli_parse, int_arg_invalid) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "abc" },
    .binds = {
      .nums = { 999 },
    },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "jobs", .kind = SP_CLI_OPT_S64, .ptr = &cli_binds.nums[0] },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_ARG,
      .err_name = "jobs",
      .err_value = "abc",
      .nums = { 999 },
    },
  });
}

UTEST_F(cli_parse, choice_opt) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode", "B" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .choices = { { "B", 2 } },
    },
  });
}

UTEST_F(cli_parse, choice_opt_invalid) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode", "C" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_name = "mode",
      .err_value = "C",
    },
  });
}

UTEST_F(cli_parse, choice_opt_rejects_number) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode", "2" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_name = "mode",
      .err_value = "2",
    },
  });
}

UTEST_F(cli_parse, choice_opt_missing_keeps_preset) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .binds = {
      .choices = { { "A", 1 } },
    },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .choices = { { "A", 1 } },
    },
  });
}

UTEST_F(cli_parse, choices_validate_other_kinds) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "--mode", "B" },
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "mode", .ptr = &cli_binds.strs[0], .choices = { { "A" }, { "B" } } },
      },
    },
    .expect = {
      .strs = { "B" },
    },
  });
}

UTEST_F(cli_parse, choice_arg) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "A" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .choices = { { "A", 1 } },
    },
  });
}

UTEST_F(cli_parse, choice_arg_invalid) {
  run_cli_parse_test(&ur, ut.mem.arena, (cli_parse_test_t) {
    .args = { "C" },
    .cmd = {
      .name = "test",
      .args = {
        { .name = "mode", .kind = SP_CLI_OPT_CHOICE, .ptr = &cli_binds.choices[0], .choices = { { "A", 1 }, { "B", 2 } } },
      },
    },
    .expect = {
      .err = SP_CLI_ERR_INVALID_ARG,
      .err_name = "mode",
      .err_value = "C",
    },
  });
}
