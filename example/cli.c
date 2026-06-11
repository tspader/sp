/*
  This is the CLI for a small, fake package manager. It's meant to exercise
  most of the library. In particular, it shows how to:
  - Define your CLI declaratively and hook up handlers
  - Add commands (e.g. pkg add)
  - Add options (e.g. --foo or --bar=BAZ) to your commands, and give them briefs (e.g. -f)
  - Add arguments (e.g. pkg add some_dependency)
  - Add nested commands (e.g. pkg tool run sqlite)

  And finally, it'll show you the simple API which automatically prints help
  text and parse errors, and then the advanced API which lets you do
  whatever you want.
*/
#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"

typedef struct {
  bool verbose;
  struct {
    const c8* package;
    const c8* version;
    bool force;
  } add;
  struct {
    const c8* target;
    s64 jobs;
  } build;
  const c8* tool;
} pkg_t;

//////////////
// HANDLERS //
//////////////
sp_cli_result_t pkg_add(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  sp_log("adding {.cyan} {.yellow}", sp_fmt_cstr(pkg->add.package), sp_fmt_cstr(pkg->add.version));
  if (pkg->add.force) {
    sp_log("reinstalling from scratch");
  }
  return SP_CLI_OK;
}

sp_cli_result_t pkg_build(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  if (pkg->build.jobs < 1) {
    sp_cli_log_error("Jobs must be >= 1 (got {.cyan})", sp_fmt_int(pkg->build.jobs));
    return SP_CLI_ERR;
  }

  const c8* target = pkg->build.target ? pkg->build.target : "all";
  sp_log("Building {.cyan} with {.yellow} jobs", sp_fmt_cstr(target), sp_fmt_int(pkg->build.jobs));
  return SP_CLI_OK;
}

sp_cli_result_t pkg_tool_run(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  sp_log("Running tool: {.cyan}", sp_fmt_cstr(pkg->tool));
  sp_for(it, cli->num_rest) {
    sp_log("  args[{}] {.gray}", sp_fmt_uint(it), sp_fmt_cstr(cli->rest[it]));
  }
  return SP_CLI_OK;
}

s32 run(s32 num_args, const c8** args) {
  pkg_t pkg = {
    .build = { .jobs = 1 },
  };

  ////////////////////
  // CLI DEFINITION //
  ////////////////////
  //
  // You define your CLI as data; what commands it has, what arguments or
  // named options they take, descriptions and help text. Each command has
  // a handler. This is the basic shape of the data:
  //
  //   sp_cli_cmd_t root = {
  //     "pkg", "description", "summary",
  //     opts[], args[],
  //     commands[] // Same thing, nested
  //   }
  //
  // I prefer to make a typed, nested struct so I can initialize everything
  // in one initializer. This isn't required; if you don't do it this way,
  // you'll have to declare any subcommands separately, since you'll have
  // no way to reference them in the parent's initializer
  //
  // I also prefer to be verbose. I put every field on its own line, and I
  // always specify the key. If this is too noisy, search for "@compact" to
  // see a tighter version
  struct {
    struct {
      sp_cli_cmd_t run;
    } tools;
    sp_cli_cmd_t tool;

    sp_cli_cmd_t add;
    sp_cli_cmd_t build;
  } c = {
    .tools = {
      .run = {
        .name = "run",
        .summary = "Run a binary from a package",
        .args = {
          {
            .name = "name",
            .summary = "The binary to run",
            .ptr = &pkg.tool,
          },
          {
            .name = "args",
            .kind = SP_CLI_ARG_REST,
            .summary = "Arguments passed to the binary",
          },
        },
        .handler = pkg_tool_run,
      },
    },
    .tool = {
      .name = "tool",
      .summary = "Run and manage binaries defined by packages",
      .commands = {
        &c.tools.run
      },
    },
    .add = {
      .name = "add",
      .summary = "Add a package to the project",
      .opts = {
        {
          .brief = "f",
          .name = "force",
          .summary = "Force reinstall even if already installed",
          .ptr = &pkg.add.force,
        },
      },
      .args = {
        {
          .name = "package",
          .summary = "The package to add",
          .ptr = &pkg.add.package,
        },
        {
          .name = "version",
          .kind = SP_CLI_ARG_OPTIONAL,
          .summary = "Version to add",
          .ptr = &pkg.add.version,
        },
      },
      .handler = pkg_add,
    },
    .build = {
      .name = "build",
      .summary = "Build the project from source",
      .opts = {
        {
          .brief = "j",
          .name = "jobs",
          .kind = SP_CLI_OPT_INTEGER,
          .summary = "Number of parallel jobs",
          .placeholder = "N",
          .ptr = &pkg.build.jobs,
        },
        {
          .name = "target",
          .kind = SP_CLI_OPT_STRING,
          .summary = "Build only the named target",
          .placeholder = "NAME",
          .ptr = &pkg.build.target,
        },
      },
      .handler = pkg_build,
    },
  };

  sp_cli_cmd_t root = {
    .name = "pkg",
    .summary = "An example package manager",
    .opts = {
      {
        .brief = "v",
        .name = "verbose",
        .summary = "Show verbose output",
        .ptr = &pkg.verbose,
      },
    },
    .commands = { &c.add, &c.build, &c.tool },
  };

  // Unless you have a reason not to, invoke the CLI like this. The cli owns
  // no memory and needs no cleanup; everything it binds (e.g. pkg.add.package)
  // points into args, so it's valid for the life of the program.
  return sp_cli_main(&root, num_args, args, &pkg);

  /*
    /////////////////////
    // HANDLING ERRORS //
    /////////////////////
    // If you'd like to specifically handle the return code, but
    // would otherwise like the library to print usage and parse
    // errors for you, you can call sp_cli_run() directly.
    //
    // sp_cli_main() is a wrapper over sp_cli_run(), just like this
    // example, which collapses success states.

    switch (sp_cli_run(&root, num_args, args, &pkg)) {
      case SP_CLI_OK: return 0;
      case SP_CLI_HELP: return 0;
      case SP_CLI_ERR: return 1;
      case SP_CLI_CONTINUE: {
        // Draw the rest of the owl
        return 0;
      }
    }
    sp_unreachable_return(1);
  */

  /*
    ////////////////////
    // ADVANCED USAGE //
    ////////////////////
    // If you want full control, it's still pretty simple. All you're doing
    // differently is:
    // - Explicitly passing args[1...]
    // - Invoking the parser on your CLI descriptor
    // - Handling the parse result
    // - Invoking the dispatch function on the parsed result
    //
    // Parse errors are structured data (cli.err); render them with
    // sp_cli_err_write(), or switch on cli.err.kind and do something else
    // entirely. Handler errors never pass through the cli: handlers print
    // their own and return SP_CLI_ERR.

    sp_io_stream_writer_t out = sp_io_get_std_out();
    sp_io_stream_writer_t err = sp_io_get_std_err();

    sp_cli_t cli = sp_cli_parse((sp_cli_desc_t) {
      .root = &root,
      .args = args + 1,
      .num_args = num_args ? sp_cast(u32, num_args - 1) : 0,
      .user_data = &pkg,
    });

    switch (cli.status) {
      case SP_CLI_HELP: {
        sp_cli_usage_write(&out.base, cli.cmd);
        return 0;
      }
      case SP_CLI_ERR: {
        sp_fmt_io(&err.base, "{.red}: ", sp_fmt_cstr("error"));
        sp_cli_err_write(&err.base, &cli.err);
        sp_fmt_io(&err.base, "\n");
        sp_cli_usage_write(&err.base, cli.cmd);
        return 1;
      }
      case SP_CLI_OK: break;
      case SP_CLI_CONTINUE: break;
    }

    switch (sp_cli_dispatch(&cli)) {
      case SP_CLI_OK: break;
      case SP_CLI_HELP: break;
      case SP_CLI_CONTINUE: break;
      case SP_CLI_ERR: {
        if (cli.err.kind != SP_CLI_ERR_NONE) {
          sp_fmt_io(&err.base, "{.red}: ", sp_fmt_cstr("error"));
          sp_cli_err_write(&err.base, &cli.err);
          sp_fmt_io(&err.base, "\n");
        }
        return 1;
      }
    }
    return 0;
  */

  /*
    /////////////
    // COMPACT //
    /////////////
    // @compact
    //
    // This is identical to the other one, just more compact.

    struct {
      struct {
        sp_cli_cmd_t run;
      } tools;
      sp_cli_cmd_t tool;

      sp_cli_cmd_t add;
      sp_cli_cmd_t build;
    } compact = {
      .tools = {
        .run = {
          "run",  "Run a binary from a package",
          .args = {
            { "name", SP_CLI_ARG_REQUIRED, "The binary to run", &pkg.tool },
            { "args", SP_CLI_ARG_REST, "Arguments passed to the binary" }
          },
          .handler = pkg_tool_run,
        },
      },
      .tool = {
        "tool", "Run and manage binaries defined by packages",
        .commands = { &compact.tools.run },
      },
      .add = {
        "add", "Add a package to the project",
        .opts = {
          { "f", "force", SP_CLI_OPT_BOOLEAN, "Force reinstall even if already installed", SP_CLI_NO_PLACEHOLDER, &pkg.add.force },
        },
        .args = {
          { "package", SP_CLI_ARG_REQUIRED, "The package to add", &pkg.add.package },
          { "version", SP_CLI_ARG_OPTIONAL, "Version to add", &pkg.add.version },
        },
        .handler = pkg_add,
      },
      .build = {
        "build", "Build the project from source",
        .opts = {
          { "j", "jobs", SP_CLI_OPT_INTEGER, "Number of parallel jobs", "N", &pkg.build.jobs },
          { SP_NULLPTR, "target", SP_CLI_OPT_STRING, "Build only the named target", "NAME", &pkg.build.target },
        },
        .handler = pkg_build,
      },
    };
  */



}
SP_MAIN(run)
