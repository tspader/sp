#include "cli.h"

#define CLI_TEST_MAX_NEEDLES 16

typedef struct {
  const c8* contains [CLI_TEST_MAX_NEEDLES];
  const c8* excludes [CLI_TEST_MAX_NEEDLES];
} cli_usage_expect_t;

typedef struct {
  sp_cli_cmd_t cmd;
  cli_usage_expect_t expect;
} cli_usage_test_t;

CLI_TEST_FIXTURE(cli_usage)

static void run_cli_usage_test(s32* utest_result, sp_mem_t mem, cli_usage_test_t t) {
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &io);
  sp_cli_usage_write(&io.base, &t.cmd);
  sp_str_t usage = sp_io_dyn_mem_writer_as_str(&io);

  sp_carr_for(t.expect.contains, it) {
    if (!t.expect.contains[it]) break;
    bool found = sp_str_contains(usage, sp_str_view(t.expect.contains[it]));
    EXPECT_TRUE(found);
    if (!found) {
      SP_TEST_REPORT("missing {.quote} in:\n{}", sp_fmt_cstr(t.expect.contains[it]), sp_fmt_str(usage));
    }
  }
  sp_carr_for(t.expect.excludes, it) {
    if (!t.expect.excludes[it]) break;
    bool found = sp_str_contains(usage, sp_str_view(t.expect.excludes[it]));
    EXPECT_FALSE(found);
    if (found) {
      SP_TEST_REPORT("unexpected {.quote} in:\n{}", sp_fmt_cstr(t.expect.excludes[it]), sp_fmt_str(usage));
    }
  }
}

UTEST_F(cli_usage, minimal) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .cmd = { .name = "test" },
    .expect = {
      .contains = { "usage", "test" },
      .excludes = { "[options]", "<command>", "options", "arguments", "commands" },
    },
  });
}

UTEST_F(cli_usage, opts_and_args) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .cmd = {
      .name = "install",
      .summary = "Install a package",
      .opts = {
        { .brief = "f", .name = "force", .summary = "Force reinstall" },
        { .name = "version", .kind = SP_CLI_OPT_STRING, .summary = "Version to install", .placeholder = "VERSION" },
      },
      .args = {
        { .name = "package", .summary = "The package to install" },
        { .name = "target", .kind = SP_CLI_ARG_OPTIONAL, .summary = "Install target" },
        { .name = "extra", .kind = SP_CLI_ARG_REST, .summary = "Passed through verbatim" },
      },
    },
    .expect = {
      .contains = {
        "Install a package",
        "usage",
        "install",
        " [options] <package> [target] [extra...]",
        "options",
        "-f, --force",
        "--version VERSION",
        "Force reinstall",
        "Version to install",
        "arguments",
        "<package>",
        "[target]",
        "[extra...]",
        "The package to install",
        "Passed through verbatim",
      },
      .excludes = { "<command>", "commands" },
    },
  });
}

UTEST_F(cli_usage, commands) {
  sp_cli_cmd_t add = { .name = "add", .summary = "Add a package", .handler = cli_handler_ok };
  sp_cli_cmd_t build = { .name = "build", .summary = "Build the project", .handler = cli_handler_ok };

  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .cmd = {
      .name = "pkg",
      .summary = "A package manager",
      .commands = { &add, &build },
    },
    .expect = {
      .contains = {
        "A package manager",
        "pkg",
        " <command>",
        "commands",
        "add",
        "Add a package",
        "build",
        "Build the project",
      },
      .excludes = { "[options]", "arguments" },
    },
  });
}

UTEST_F(cli_usage, parsed_command_usage) {
  sp_cli_cmd_t install = { .name = "install", .handler = cli_handler_ok };
  sp_cli_cmd_t tool = { .name = "tool", .commands = { &install } };
  sp_cli_cmd_t root = { .name = "pkg", .commands = { &tool } };
  const c8* args [] = { "tool", "install" };

  sp_cli_t cli = sp_cli_parse((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = sp_carr_len(args),
  });

  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(ut.mem.arena, &io);
  sp_cli_usage_write(&io.base, cli.cmd);
  sp_str_t usage = sp_io_dyn_mem_writer_as_str(&io);

  EXPECT_EQ(SP_CLI_OK, cli.status);
  EXPECT_TRUE(sp_str_contains(usage, sp_str_lit("install")));
  EXPECT_FALSE(sp_str_contains(usage, sp_str_lit("pkg")));
}
