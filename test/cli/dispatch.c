#include "cli.h"

typedef struct {
  sp_cli_result_t result;
  const c8* dispatched;
  sp_cli_err_kind_t err;
} cli_dispatch_expect_t;

typedef struct {
  const c8* args [CLI_TEST_MAX_ARGS];
  sp_cli_cmd_t cmd;
  cli_dispatch_expect_t expect;
} cli_dispatch_test_t;

CLI_TEST_FIXTURE(cli_dispatch)

static void run_cli_dispatch_test(s32* utest_result, sp_mem_t mem, cli_dispatch_test_t t) {
  cli_binds = sp_zero_s(cli_binds_t);
  cli_dispatched = sp_zero_s(sp_str_t);

  const c8* argv [CLI_TEST_MAX_ARGS + 1];
  u32 n = cli_count_args(t.args);
  argv[0] = "test";
  for (u32 it = 0; it < n; it++) argv[it + 1] = t.args[it];

  sp_cli_t cli = sp_cli_parse((sp_cli_desc_t) {
    .root = &t.cmd,
    .args = argv,
    .num_args = sp_cast(s32, n + 1),
  });
  sp_cli_result_t result = sp_cli_dispatch(&cli);

  EXPECT_EQ(t.expect.result, result);
  SP_EXPECT_STR_EQ_CSTR(cli_dispatched, t.expect.dispatched ? t.expect.dispatched : "");
  EXPECT_EQ(t.expect.err, cli.err.kind);
}

UTEST_F(cli_dispatch, root_handler) {
  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .cmd = { .name = "root", .handler = cli_handler_ok },
    .expect = {
      .dispatched = "root",
    },
  });
}

UTEST_F(cli_dispatch, command_handler) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .args = { "build" },
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .dispatched = "build",
    },
  });
}

UTEST_F(cli_dispatch, handler_result_propagates) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_err };

  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .args = { "build" },
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .result = SP_CLI_ERR,
      .dispatched = "build",
    },
  });
}

UTEST_F(cli_dispatch, nested_command_handler) {
  sp_cli_cmd_t install = { .name = "install", .handler = cli_handler_ok };
  sp_cli_cmd_t tool = { .name = "tool", .commands = { &install } };

  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .args = { "tool", "install" },
    .cmd = { .name = "root", .commands = { &tool } },
    .expect = {
      .dispatched = "install",
    },
  });
}

UTEST_F(cli_dispatch, parse_error_skips_dispatch) {
  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .args = { "--bogus" },
    .cmd = { .name = "root", .handler = cli_handler_ok },
    .expect = {
      .result = SP_CLI_ERR,
      .err = SP_CLI_ERR_UNKNOWN_OPT,
    },
  });
}

UTEST_F(cli_dispatch, missing_command_skips_dispatch) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };

  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .result = SP_CLI_HELP,
    },
  });
}

UTEST_F(cli_dispatch, help_skips_dispatch) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_err };

  run_cli_dispatch_test(&ur, ut.mem.arena, (cli_dispatch_test_t) {
    .args = { "build", "--help" },
    .cmd = { .name = "root", .commands = { &build } },
    .expect = {
      .result = SP_CLI_HELP,
    },
  });
}

static sp_cli_result_t cli_handler_user_data(sp_cli_t* cli) {
  *sp_cast(u32*, cli->user_data) = 69;
  return SP_CLI_OK;
}

static sp_cli_result_t cli_handler_continue(sp_cli_t* cli) {
  cli_dispatched = sp_str_view(cli->cmd->name);
  return SP_CLI_CONTINUE;
}

UTEST_F(cli_dispatch, run_user_data) {
  u32 value = 0;
  sp_cli_cmd_t cmd = { .name = "root", .handler = cli_handler_user_data };
  const c8* args [] = { "root" };

  EXPECT_EQ(SP_CLI_OK, sp_cli_run((sp_cli_desc_t) {
    .root = &cmd, .args = args, .num_args = sp_carr_len(args), .user_data = &value,
  }));
  EXPECT_EQ(69u, value);
}

UTEST_F(cli_dispatch, run_skips_program_name) {
  sp_cli_cmd_t build = { .name = "build", .handler = cli_handler_ok };
  sp_cli_cmd_t root = { .name = "root", .commands = { &build } };
  const c8* args [] = { "root", "build" };

  cli_dispatched = sp_zero_s(sp_str_t);
  EXPECT_EQ(SP_CLI_OK, sp_cli_run((sp_cli_desc_t) {
    .root = &root, .args = args, .num_args = sp_carr_len(args),
  }));
  SP_EXPECT_STR_EQ_CSTR(cli_dispatched, "build");
}

UTEST_F(cli_dispatch, run_handler_continue_propagates) {
  sp_cli_cmd_t cmd = { .name = "root", .handler = cli_handler_continue };
  const c8* args [] = { "root" };

  EXPECT_EQ(SP_CLI_CONTINUE, sp_cli_run((sp_cli_desc_t) {
    .root = &cmd, .args = args, .num_args = sp_carr_len(args),
  }));
}

UTEST_F(cli_dispatch, main_exit_codes) {
  sp_cli_cmd_t ok = { .name = "root", .handler = cli_handler_ok };
  sp_cli_cmd_t err = { .name = "root", .handler = cli_handler_err };
  const c8* args [] = { "root" };

  EXPECT_EQ(0, sp_cli_main((sp_cli_desc_t) {
    .root = &ok, .args = args, .num_args = sp_carr_len(args),
  }));
  EXPECT_EQ(1, sp_cli_main((sp_cli_desc_t) {
    .root = &err, .args = args, .num_args = sp_carr_len(args),
  }));
}
