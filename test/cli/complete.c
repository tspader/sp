#include "cli.h"

static void cli_complete_fruits(sp_cli_complete_t* ctx) {
  sp_cli_candidate(ctx, sp_str_lit("apple"), sp_zero_s(sp_str_t));
  sp_cli_candidate(ctx, sp_str_lit("banana"), sp_zero_s(sp_str_t));
}

static void cli_complete_veggies(sp_cli_complete_t* ctx) {
  sp_cli_candidate(ctx, sp_str_lit("carrot"), sp_zero_s(sp_str_t));
  sp_cli_candidate(ctx, sp_str_lit("potato"), sp_zero_s(sp_str_t));
}

static void cli_complete_colon(sp_cli_complete_t* ctx) {
  sp_cli_candidate(ctx, sp_str_lit("a:b"), sp_str_lit("desc:tail"));
}

static void cli_complete_specials(sp_cli_complete_t* ctx) {
  sp_cli_candidate(ctx, sp_str_lit("a b$c"), sp_zero_s(sp_str_t));
}

typedef struct {
  sp_cli_shell_t shell;
  const c8* words [CLI_TEST_MAX_ARGS];
  sp_cli_cmd_t cmd;
  const c8* expect [CLI_TEST_MAX_ARGS];
} cli_complete_test_t;

CLI_TEST_FIXTURE(cli_complete)

static void run_cli_complete_test(s32* utest_result, sp_mem_t mem, cli_complete_test_t t) {
  u32 num_words = 0;
  while (num_words < CLI_TEST_MAX_ARGS && t.words[num_words]) num_words++;

  u32 num_expect = 0;
  while (num_expect < CLI_TEST_MAX_ARGS && t.expect[num_expect]) num_expect++;

  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &io);
  sp_cli_complete(&io.base, (sp_cli_desc_t) { .root = &t.cmd }, t.shell, t.words, num_words);
  sp_str_t actual = sp_io_dyn_mem_writer_as_str(&io);

  sp_da(sp_str_t) lines = sp_str_split_c8(mem, actual, '\n');
  u32 num_lines = 0;
  sp_da_for(lines, it) {
    if (!sp_str_empty(lines[it])) num_lines++;
  }

  EXPECT_EQ(num_expect, num_lines);

  u32 li = 0;
  sp_da_for(lines, it) {
    if (sp_str_empty(lines[it])) continue;
    if (li < num_expect) SP_EXPECT_STR_EQ_CSTR(lines[it], t.expect[li]);
    li++;
  }
}

UTEST_F(cli_complete, commands) {
  sp_cli_cmd_t add = { .name = "add", .handler = cli_handler_ok };
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "" },
    .cmd = { .name = "root", .commands = { &add, &build } },
    .expect = { "add", "build" },
  });
}

UTEST_F(cli_complete, options_on_dash) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "-" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN },
        { .brief = 'f', .name = "force", .kind = SP_CLI_OPT_BOOLEAN },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "--verbose", "--force" },
  });
}

UTEST_F(cli_complete, subcommand_scopes_options) {
  sp_cli_cmd_t add = {
    .name = "add",
    .opts = {
      { .brief = 'f', .name = "force", .kind = SP_CLI_OPT_BOOLEAN },
    },
    .handler = cli_handler_ok,
  };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "add", "-" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'v', .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN },
      },
      .commands = { &add },
    },
    .expect = { "--force", "--verbose" },
  });
}

UTEST_F(cli_complete, pending_long_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, pending_short_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "-f", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'f', .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, short_cluster_inline_value_consumes_rest) {
  sp_cli_cmd_t child = { .name = "child", .handler = cli_handler_ok };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "-fv", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'f', .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
        { .brief = 'v', .name = "veggie", .kind = SP_CLI_OPT_STR, .complete = cli_complete_veggies },
      },
      .commands = { &child },
    },
    .expect = { "child" },
  });
}

UTEST_F(cli_complete, choice_values) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--mode", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "mode", .choices = { { "A" }, { "B" } } },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "A", "B" },
  });
}

UTEST_F(cli_complete, completer_wins_over_choices) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--mode", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "mode", .complete = cli_complete_fruits, .choices = { { "A" }, { "B" } } },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, positional_choice_values) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "mode", .choices = { { "A" }, { "B" } } },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "A", "B" },
  });
}

UTEST_F(cli_complete, positional_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "fruit", .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, prefix_filters_candidates) {
  sp_cli_cmd_t add = { .name = "add", .handler = cli_handler_ok };
  sp_cli_cmd_t archive = { .name = "archive", .handler = cli_handler_ok };
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "a" },
    .cmd = { .name = "root", .commands = { &add, &archive, &build } },
    .expect = { "add", "archive" },
  });
}

UTEST_F(cli_complete, inline_long_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit=a" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "--fruit=apple" },
  });
}

UTEST_F(cli_complete, bash_split_long_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit", "=", "a" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple" },
  });
}

UTEST_F(cli_complete, bash_split_trailing_separator) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit", "=" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, bash_split_pair_in_context) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit", "=", "apple", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .args = {
        { .name = "veggie", .complete = cli_complete_veggies },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "carrot", "potato" },
  });
}

UTEST_F(cli_complete, inline_short_value) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "-fa" },
    .cmd = {
      .name = "root",
      .opts = {
        { .brief = 'f', .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "-fapple" },
  });
}

UTEST_F(cli_complete, double_dash_ends_commands) {
  sp_cli_cmd_t add = { .name = "add", .handler = cli_handler_ok };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "x", .arity = SP_CLI_ARG_REST, .complete = cli_complete_fruits },
      },
      .commands = { &add },
    },
    .expect = { "apple", "banana" },
  });
}

UTEST_F(cli_complete, options_deduped_across_scopes) {
  sp_cli_cmd_t sub = {
    .name = "sub",
    .opts = {
      { .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN },
      { .name = "local", .kind = SP_CLI_OPT_BOOLEAN },
    },
    .handler = cli_handler_ok,
  };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "sub", "--" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "verbose", .kind = SP_CLI_OPT_BOOLEAN },
        { .name = "global", .kind = SP_CLI_OPT_BOOLEAN },
      },
      .commands = { &sub },
    },
    .expect = { "--verbose", "--local", "--global" },
  });
}

UTEST_F(cli_complete, pending_value_yields_to_long_option) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "--fruit", "--veggie", "" },
    .cmd = {
      .name = "root",
      .opts = {
        { .name = "fruit", .kind = SP_CLI_OPT_STR, .complete = cli_complete_fruits },
        { .name = "veggie", .kind = SP_CLI_OPT_STR, .complete = cli_complete_veggies },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "carrot", "potato" },
  });
}

UTEST_F(cli_complete, bash_escapes_specials) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .words = { "root", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "x", .complete = cli_complete_specials },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "a\\ b\\$c" },
  });
}

UTEST_F(cli_complete, zsh_escapes_colon) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .shell = SP_CLI_SHELL_ZSH,
    .words = { "root", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "x", .complete = cli_complete_colon },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "a\\:b:desc:tail" },
  });
}

UTEST_F(cli_complete, powershell_tab_separated) {
  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .shell = SP_CLI_SHELL_POWERSHELL,
    .words = { "root", "" },
    .cmd = {
      .name = "root",
      .args = {
        { .name = "x", .complete = cli_complete_colon },
      },
      .handler = cli_handler_ok,
    },
    .expect = { "a:b\tdesc:tail" },
  });
}

typedef struct {
  bool anchored;
} cli_completer_path_expect_t;

typedef struct {
  const c8* arg0;
  cli_completer_path_expect_t expect;
} cli_completer_path_test_t;

static void run_cli_completer_path_test(s32* utest_result, sp_mem_t mem, cli_completer_path_test_t t) {
  sp_cli_cmd_t root = { .name = "A" };
  const c8* args [] = { t.arg0 };
  c8 buffer [SP_PATH_MAX];
  sp_str_t path = sp_cli_completer_path((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = 1,
  }, buffer, sizeof(buffer));

  if (t.expect.anchored) {
    c8 cwd [SP_PATH_MAX];
    s64 cwd_len = sp_sys_get_cwd_path(cwd, sizeof(cwd));
    EXPECT_GT(cwd_len, 0);
    if (cwd_len <= 0) return;
    sp_str_t expected = sp_fmt(mem, "{}/{}", sp_fmt_str(sp_str(cwd, sp_cast(u32, cwd_len))), sp_fmt_cstr(t.arg0)).value;
    SP_EXPECT_STR_EQ(path, expected);
  }
  else {
    SP_EXPECT_STR_EQ_CSTR(path, t.arg0);
  }
}

UTEST_F(cli_complete, completer_path_bare) {
  run_cli_completer_path_test(&ur, ut.mem.arena, (cli_completer_path_test_t) {
    .arg0 = "A",
  });
}

UTEST_F(cli_complete, completer_path_relative) {
  run_cli_completer_path_test(&ur, ut.mem.arena, (cli_completer_path_test_t) {
    .arg0 = "B/A",
    .expect = { .anchored = true },
  });
}

UTEST_F(cli_complete, completer_path_relative_backslash) {
  run_cli_completer_path_test(&ur, ut.mem.arena, (cli_completer_path_test_t) {
    .arg0 = "B\\A",
    .expect = { .anchored = true },
  });
}

UTEST_F(cli_complete, completer_path_absolute) {
  run_cli_completer_path_test(&ur, ut.mem.arena, (cli_completer_path_test_t) {
    .arg0 = "/B/A",
  });
}

UTEST_F(cli_complete, powershell_empty_sentinel) {
  sp_cli_cmd_t add = { .name = "add", .handler = cli_handler_ok };
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_complete_test(&ur, ut.mem.arena, (cli_complete_test_t) {
    .shell = SP_CLI_SHELL_POWERSHELL,
    .words = { "root", SP_CLI_COMPLETE_EMPTY },
    .cmd = { .name = "root", .commands = { &add, &build } },
    .expect = { "add", "build" },
  });
}
