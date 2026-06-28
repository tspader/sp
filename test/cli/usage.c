#include "cli.h"

typedef struct {
  const c8* name;
  sp_cli_cmd_t* path [SP_CLI_MAX_DEPTH];
  sp_cli_cmd_t cmd;
} cli_usage_test_t;

CLI_TEST_FIXTURE(cli_usage)

static void cli_print_usage_file(sp_str_t path, sp_str_t content) {
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, path);
  sp_io_write_str(&w.base, content, SP_NULLPTR);
  sp_io_file_writer_close(&w);
}

static void run_cli_usage_test(s32* utest_result, sp_mem_t mem, cli_usage_test_t t) {
  SKIP_ON_WASM();

  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &io);
  sp_cli_t cli = { .cmd = &t.cmd, .theme = { .mode = SP_CLI_THEME_REPLACE } };
  sp_carr_for(t.path, it) {
    if (!t.path[it]) break;
    cli.path[cli.depth++] = t.path[it];
  }
  cli.path[cli.depth++] = &t.cmd;

  sp_cli_write_help(&io.base, &cli);
  sp_str_t actual = sp_io_dyn_mem_writer_as_str(&io);

  sp_str_t dir = sp_fmt(mem, "{}/golden", sp_fmt_cstr(SP_CLI_TEST_DIR)).value;
  sp_str_t path = sp_fmt(mem, "{}/{}.txt", sp_fmt_str(dir), sp_fmt_cstr(t.name)).value;

  if (!sp_str_empty(sp_os_env_get(sp_str_lit("SP_CLI_TEST_UPDATE")))) {
    sp_fs_create_dir(dir);
    cli_print_usage_file(path, actual);
    SP_TEST_REPORT("updated {}\n", sp_fmt_str(path));
    return;
  }

  sp_str_t expected = sp_zero;
  if (sp_io_read_file(mem, path, &expected) != SP_OK) {
    SP_TEST_REPORT("missing golden {}; rerun with SP_CLI_TEST_UPDATE=1\n", sp_fmt_str(path));
    SP_FAIL();
    return;
  }

  if (!sp_str_equal(actual, expected)) {
    sp_str_t actual_path = sp_fmt(mem, "{}.actual", sp_fmt_str(path)).value;
    cli_print_usage_file(actual_path, actual);
    SP_TEST_REPORT("{} does not match; wrote {}\n", sp_fmt_str(path), sp_fmt_str(actual_path));
    SP_FAIL();
  }
}

// A bare command: nothing but a name. Only the usage heading and the command
// name should render; no other sections appear.
UTEST_F(cli_usage, minimal) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "minimal",
    .cmd = { .name = "test" },
  });
}

// The summary line renders above the usage heading when present.
UTEST_F(cli_usage, summary) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "summary",
    .cmd = { .name = "test", .summary = "A one line description" },
  });
}

// A single option: triggers the [options] token in the usage line, the options
// heading, and the "-b, --name" label form (brief present).
UTEST_F(cli_usage, option) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "option",
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = "f", .name = "force", .summary = "Force the operation" },
      },
    },
  });
}

// An option without a brief: the label pads with four spaces where "-x, " would
// otherwise sit.
UTEST_F(cli_usage, option_no_brief) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "option_no_brief",
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "verbose", .summary = "Verbose output" },
      },
    },
  });
}

// A placeholder appends to the option label after the name.
UTEST_F(cli_usage, option_placeholder) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "option_placeholder",
    .cmd = {
      .name = "test",
      .opts = {
        { .name = "output", .summary = "Where to write", .placeholder = "PATH" },
      },
    },
  });
}

// Mixed-width option labels: summaries align to the widest label in the column.
UTEST_F(cli_usage, option_alignment) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "option_alignment",
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = "f", .name = "force", .summary = "Short label" },
        { .name = "dry-run", .summary = "No brief, wider" },
        { .brief = "o", .name = "output", .summary = "Brief and placeholder", .placeholder = "PATH" },
      },
    },
  });
}

// A required argument: appears bare in the usage line and carries the "required"
// hint in the arguments section.
UTEST_F(cli_usage, argument) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "argument",
    .cmd = {
      .name = "test",
      .args = {
        { .name = "file", .summary = "The file to read" },
      },
    },
  });
}

// An optional argument: bracketed label and the "optional" hint.
UTEST_F(cli_usage, argument_optional) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "argument_optional",
    .cmd = {
      .name = "test",
      .args = {
        { .name = "file", .kind = SP_CLI_ARG_OPTIONAL, .summary = "The file to read" },
      },
    },
  });
}

// A rest argument: ellipsis label, also rendered as optional.
UTEST_F(cli_usage, argument_rest) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "argument_rest",
    .cmd = {
      .name = "test",
      .args = {
        { .name = "files", .kind = SP_CLI_ARG_REST, .summary = "Files to read" },
      },
    },
  });
}

// Subcommands: the <command> token in the usage line and the commands section.
UTEST_F(cli_usage, command) {
  static sp_cli_cmd_t add = { .name = "add", .summary = "Add a package", .handler = cli_handler_ok };
  static sp_cli_cmd_t remove = { .name = "remove", .summary = "Remove a package", .handler = cli_handler_ok };

  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "command",
    .cmd = {
      .name = "test",
      .commands = { &add, &remove },
    },
  });
}

// A required environment variable: environment section with the "required" hint.
UTEST_F(cli_usage, environment) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "environment",
    .cmd = {
      .name = "test",
      .env = {
        { .name = "API_TOKEN", .summary = "Auth token", .required = true },
      },
    },
  });
}

// An optional environment variable carries the "optional" hint.
UTEST_F(cli_usage, environment_optional) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "environment_optional",
    .cmd = {
      .name = "test",
      .env = {
        { .name = "LOG_LEVEL", .summary = "Verbosity" },
      },
    },
  });
}

// An entry with no summary still renders its label and hint columns.
UTEST_F(cli_usage, no_summary) {
  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "no_summary",
    .cmd = {
      .name = "test",
      .opts = {
        { .brief = "f", .name = "force" },
      },
    },
  });
}

// A nested subcommand renders its full ancestor path in the usage line.
UTEST_F(cli_usage, command_path) {
  static sp_cli_cmd_t uv = { .name = "uv" };
  static sp_cli_cmd_t tool = { .name = "tool" };

  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "command_path",
    .path = { &uv, &tool },
    .cmd = {
      .name = "install",
      .summary = "Install a tool",
      .args = {
        { .name = "package", .summary = "The package to install" },
      },
    },
  });
}

// Every section at once: exercises section ordering and the full usage line.
UTEST_F(cli_usage, everything) {
  static sp_cli_cmd_t add = { .name = "add", .summary = "Add a package", .handler = cli_handler_ok };

  run_cli_usage_test(&ur, ut.mem.arena, (cli_usage_test_t) {
    .name = "everything",
    .cmd = {
      .name = "pkg",
      .summary = "A package manager",
      .opts = {
        { .brief = "v", .name = "verbose", .summary = "Verbose output" },
        { .name = "config", .summary = "Config file", .placeholder = "PATH" },
      },
      .args = {
        { .name = "target", .summary = "The target" },
        { .name = "extra", .kind = SP_CLI_ARG_REST, .summary = "Passed through" },
      },
      .env = {
        { .name = "PKG_HOME", .summary = "Install root", .required = true },
      },
      .commands = { &add },
    },
  });
}
