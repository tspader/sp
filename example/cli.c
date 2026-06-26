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
  const c8* home;
  bool color;
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
  if (pkg->verbose) sp_log("installing into {.gray}", sp_fmt_cstr(pkg->home));
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
  if (pkg->verbose) sp_log("using install root {.gray}", sp_fmt_cstr(pkg->home));
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
  // always specify the key.
  struct {
    sp_cli_cmd_t run;
    sp_cli_cmd_t tool;

    sp_cli_cmd_t add;
    sp_cli_cmd_t build;
  } c = {
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
    .tool = {
      .name = "tool",
      .summary = "Run and manage binaries defined by packages",
      .commands = {
        &c.run
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
    .env = {
      {
        .name = "PKG_HOME",
        .kind = SP_CLI_OPT_STRING,
        .summary = "Where packages are installed",
        .required = true,
        .ptr = &pkg.home,
      },
      {
        .name = "PKG_COLOR",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "Enable colored output",
        .ptr = &pkg.color,
      },
    },
    .commands = { &c.add, &c.build, &c.tool },
  };

  // Every entry point takes the same descriptor: the raw argv/argc exactly as
  // main() received them (the program name is stripped for you), your root
  // command, your user data, and an optional theme.
  sp_cli_desc_t cli = {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &pkg,
  };

  // Unless you have a reason not to, invoke the CLI like this. main() parses,
  // prints help and errors for you, dispatches the handler, and collapses the
  // result into a process exit code. The cli owns no memory and needs no
  // cleanup; everything it binds (e.g. pkg.add.package) points into args, so
  // it's valid for the life of the program.
  return sp_cli_main(cli);

  /*
    /////////////////////
    // HANDLING ERRORS //
    /////////////////////
    // If you want to act on the outcome yourself but still let the library
    // print usage and parse errors, call sp_cli_run(). It does everything
    // main() does but hands you the result instead of an exit code.
    // main() is just sp_cli_run() with the result mapped to 0/1.

    switch (sp_cli_run(cli)) {
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
    // If you want full control, parse and dispatch yourself. sp_cli_parse()
    // does no IO: it just fills in the result (cli.status, cli.cmd, cli.err,
    // and your bound fields). sp_cli_dispatch() runs the handler only when
    // parsing succeeded, so you can inspect or mutate anything in between.
    //
    // Parse errors are structured data (cli.err); render them with
    // sp_cli_err_write(), or switch on cli.err.kind and do something else
    // entirely. Handler errors never pass through the cli: handlers print
    // their own and return SP_CLI_ERR.
    //
    // sp_cli_report() is the print policy run() uses (help to stdout, errors
    // to stderr); call it to get the same output for free, or skip it and
    // render by hand.

    sp_cli_t parsed = sp_cli_parse(cli);
    // ... inspect parsed.status / parsed.cmd / pkg here ...
    sp_cli_result_t result = sp_cli_dispatch(&parsed);
    sp_cli_report(&parsed);
    return result == SP_CLI_ERR ? 1 : 0;
  */

}
SP_MAIN(run)
