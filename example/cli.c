/*
  This is the CLI for a small, fake package manager. It's meant to exercise
  most of the library. In particular, it shows how to:
  - Define your CLI declaratively and hook up handlers
  - Add commands (e.g. pkg add)
  - Add options (e.g. --foo or --bar=BAZ) to your commands, and give them briefs (e.g. -f)
  - Add options which only accept a fixed set of names, each mapped to a value (e.g. --profile debug)
  - Add arguments (e.g. pkg add some_dependency)
  - Add nested commands (e.g. pkg tool run sqlite)

  And finally, it'll show you the simple API which automatically prints help
  text and parse errors, and then the advanced API which lets you do
  whatever you want.
*/
#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"

typedef enum {
  PKG_PROFILE_DEBUG,
  PKG_PROFILE_RELEASE,
} pkg_profile_t;

typedef struct {
  bool verbose;
  const c8* home;
  bool color;
  struct {
    const c8* package;
    sp_str_t version;
    bool force;
  } add;
  struct {
    const c8* target;
    u32 jobs;
    sp_cli_choice_t profile;
  } build;
  const c8* tool;
} pkg_t;

//////////////
// HANDLERS //
//////////////
sp_cli_result_t pkg_add(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  sp_str_t version = sp_str_empty(pkg->add.version) ? sp_str_lit("latest") : pkg->add.version;
  if (pkg->verbose) sp_log("PKG_HOME={.gray}", sp_fmt_cstr(pkg->home));
  if (pkg->add.force) {
    sp_log("Force reinstalling {.cyan} {.yellow} to project", sp_fmt_cstr(pkg->add.package), sp_fmt_str(version));
  } else {
    sp_log("Adding {.cyan} {.yellow} to project", sp_fmt_cstr(pkg->add.package), sp_fmt_str(version));
  }
  return SP_CLI_OK;
}

sp_cli_result_t pkg_build(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  if (pkg->build.jobs < 1) {
    return sp_cli_set_error_c(cli, "Jobs must be >= 1");
  }

  const c8* target = pkg->build.target ? pkg->build.target : "all";
  if (pkg->verbose) sp_log("PKG_HOME={.gray}", sp_fmt_cstr(pkg->home));
  sp_log("Building {.cyan} ({.yellow}) with {.yellow} jobs", sp_fmt_cstr(target), sp_fmt_cstr(pkg->build.profile.name), sp_fmt_uint(pkg->build.jobs));
  return SP_CLI_OK;
}

sp_cli_result_t pkg_tool_run(sp_cli_t* cli) {
  pkg_t* pkg = sp_cast(pkg_t*, cli->user_data);
  sp_log("Running tool: {.cyan}", sp_fmt_cstr(pkg->tool));
  for (u32 it = 0; cli->rest[it]; it++) {
    sp_log("  args[{}] {.gray}", sp_fmt_uint(it), sp_fmt_cstr(cli->rest[it]));
  }
  return SP_CLI_OK;
}

void complete_packages(sp_cli_complete_t* ctx) {
  static const struct { const c8* name; const c8* summary; } packages [] = {
    { "libpng",  "PNG image codec" },
    { "zlib",    "Compression library" },
    { "sqlite3", "Embedded SQL database" },
    { "openssl", "TLS and crypto" },
    { "curl",    "URL transfer library" },
    { "ncurses", "Terminal UI library" },
  };
  sp_carr_for(packages, it) {
    sp_cli_candidate(ctx, sp_cstr_as_str(packages[it].name), sp_cstr_as_str(packages[it].summary));
  }
}

void complete_targets(sp_cli_complete_t* ctx) {
  static const c8* targets [] = { "all", "core", "cli", "tests", "docs" };
  sp_carr_for(targets, it) {
    sp_cli_candidate(ctx, sp_cstr_as_str(targets[it]), sp_zero_s(sp_str_t));
  }
}

void complete_tools(sp_cli_complete_t* ctx) {
  static const c8* tools [] = { "fmt", "lint", "repl", "bench" };
  sp_carr_for(tools, it) {
    sp_cli_candidate(ctx, sp_cstr_as_str(tools[it]), sp_zero_s(sp_str_t));
  }
}

s32 run(s32 num_args, const c8** args) {
  pkg_t pkg = {
    .build = {
      .jobs = 1,
      .profile = { "debug", PKG_PROFILE_DEBUG },
    },
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
          .complete = complete_tools,
        },
        {
          .name = "args",
          .arity = SP_CLI_ARG_REST,
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
          .brief = 'f',
          .name = "force",
          .kind = SP_CLI_OPT_BOOLEAN,
          .summary = "Force reinstall even if already installed",
          .ptr = &pkg.add.force,
        },
      },
      .args = {
        {
          .name = "package",
          .summary = "The package to add",
          .ptr = &pkg.add.package,
          .complete = complete_packages,
        },
        {
          .name = "version",
          .arity = SP_CLI_ARG_OPTIONAL,
          .kind = SP_CLI_OPT_STR,
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
          .brief = 'j',
          .name = "jobs",
          .kind = SP_CLI_OPT_U32,
          .summary = "Number of parallel jobs",
          .placeholder = "N",
          .ptr = &pkg.build.jobs,
        },
        {
          .name = "target",
          .summary = "Build only the named target",
          .placeholder = "NAME",
          .ptr = &pkg.build.target,
          .complete = complete_targets,
        },
        {
          .brief = 'p',
          .name = "profile",
          .kind = SP_CLI_OPT_CHOICE,
          .summary = "Build profile",
          .ptr = &pkg.build.profile,
          .choices = {
            { "debug",   PKG_PROFILE_DEBUG },
            { "release", PKG_PROFILE_RELEASE },
          },
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
        .brief = 'v',
        .name = "verbose",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "Show verbose output",
        .ptr = &pkg.verbose,
      },
    },
    .env = {
      {
        .name = "PKG_HOME",
        .summary = "Where packages are installed",
        .ptr = &pkg.home,
      .required = true
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
    .completer = "PKG_COMPLETE",
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
    SP_UNREACHABLE_RETURN(1);
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
    // sp_cli_err_print(), or switch on cli.err.kind and do something else
    // entirely. Handler errors never pass through the cli: handlers print
    // their own and return SP_CLI_ERR.
    //
    // run() has no monopoly on output: sp_cli_write_help() renders the same
    // help it prints, so you can send it anywhere or skip it entirely.

    sp_cli_t parsed;
    sp_cli_parse(cli, &parsed);
    // ... inspect parsed.status / parsed.cmd / pkg here ...
    sp_cli_result_t result = sp_cli_dispatch(&parsed);
    return result == SP_CLI_ERR ? 1 : 0;
  */

}
SP_MAIN(run)
