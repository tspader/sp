#ifndef SP_CLI_H
#define SP_CLI_H

#include "sp.h"

#ifndef SP_CLI_MAX_OPTS
  #define SP_CLI_MAX_OPTS 16
#endif

#ifndef SP_CLI_MAX_ARGS
  #define SP_CLI_MAX_ARGS 8
#endif

#ifndef SP_CLI_MAX_COMMANDS
  #define SP_CLI_MAX_COMMANDS 16
#endif

#ifndef SP_CLI_MAX_ENV
  #define SP_CLI_MAX_ENV 16
#endif

#ifndef SP_CLI_MAX_LABEL
  #define SP_CLI_MAX_LABEL 64
#endif

#ifndef SP_CLI_MAX_DEPTH
  #define SP_CLI_MAX_DEPTH 4
#endif

typedef enum {
  SP_CLI_ARG_REQUIRED,
  SP_CLI_ARG_OPTIONAL,
  SP_CLI_ARG_REST,
} sp_cli_arg_kind_t;

typedef enum {
  SP_CLI_OPT_BOOLEAN,
  SP_CLI_OPT_STRING,
  SP_CLI_OPT_INTEGER,
} sp_cli_value_kind_t;

typedef enum {
  SP_CLI_THEME_MERGE,
  SP_CLI_THEME_REPLACE,
} sp_cli_theme_mode_t;

typedef enum {
  SP_CLI_OK,
  SP_CLI_ERR,
  SP_CLI_HELP,
  SP_CLI_CONTINUE,
} sp_cli_result_t;

typedef enum {
  SP_CLI_ERR_NONE,
  SP_CLI_ERR_CUSTOM,
  SP_CLI_ERR_UNKNOWN_OPT,
  SP_CLI_ERR_UNKNOWN_BRIEF,
  SP_CLI_ERR_INVALID_VALUE,
  SP_CLI_ERR_MISSING_VALUE,
  SP_CLI_ERR_MISSING_ARG,
  SP_CLI_ERR_UNEXPECTED_ARG,
  SP_CLI_ERR_UNKNOWN_COMMAND,
  SP_CLI_ERR_MISSING_ENV,
  SP_CLI_ERR_INVALID_ENV,
} sp_cli_err_kind_t;

typedef struct {
  sp_cli_err_kind_t kind;
  sp_str_t name;
  sp_str_t value;
} sp_cli_err_t;

typedef struct sp_cli sp_cli_t;
typedef struct sp_cli_cmd sp_cli_cmd_t;

SP_TYPEDEF_FN(sp_cli_result_t, sp_cli_handler_t, sp_cli_t*);

typedef struct {
  const c8* name;
  sp_cli_arg_kind_t kind;
  const c8* summary;
  const c8** ptr;
} sp_cli_arg_t;

typedef struct {
  const c8* brief;
  const c8* name;
  sp_cli_value_kind_t kind;
  const c8* summary;
  const c8* placeholder;
  void* ptr;
} sp_cli_opt_t;

typedef struct {
  const c8* name;
  sp_cli_value_kind_t kind;
  const c8* summary;
  void* ptr;
  bool required;
} sp_cli_env_t;

#define SP_CLI_NO_OPTS sp_zero
#define SP_CLI_NO_ARGS sp_zero
#define SP_CLI_NO_CMDS sp_zero
#define SP_CLI_NO_PLACEHOLDER SP_NULLPTR

struct sp_cli_cmd {
  const c8* name;
  const c8* summary;
  sp_cli_opt_t opts [SP_CLI_MAX_OPTS];
  sp_cli_arg_t args [SP_CLI_MAX_ARGS];
  sp_cli_env_t env [SP_CLI_MAX_ENV];
  sp_cli_cmd_t* commands [SP_CLI_MAX_COMMANDS];
  sp_cli_handler_t handler;
};

typedef struct {
  sp_fmt_style_t color;
  sp_fmt_style_t attribute;
} sp_cli_theme_entry_t;

typedef struct {
  sp_cli_theme_entry_t heading;
  sp_cli_theme_entry_t command;
  sp_cli_theme_entry_t label;
  sp_cli_theme_entry_t hint;
  sp_cli_theme_entry_t error;
  sp_cli_theme_mode_t mode;
} sp_cli_theme_t;

typedef struct {
  sp_cli_cmd_t* root;
  const c8** args;
  s32 num_args;
  void* user_data;
  sp_cli_theme_t theme;
} sp_cli_desc_t;

struct sp_cli {
  void* user_data;
  sp_cli_result_t status;
  sp_cli_err_t err;
  sp_cli_cmd_t* cmd;
  sp_cli_cmd_t* path [SP_CLI_MAX_DEPTH];
  u32 depth;
  const c8** rest;
  u32 num_rest;
  struct {
    sp_io_stream_writer_t out;
    sp_io_stream_writer_t err;
  } stdio;
  struct {
    sp_io_writer_t* out;
    sp_io_writer_t* err;
  } io;
  sp_cli_theme_t theme;
};

SP_API sp_str_t        sp_cli_arg_kind_to_str(sp_cli_arg_kind_t kind);
SP_API sp_str_t        sp_cli_opt_kind_to_str(sp_cli_value_kind_t kind);
SP_API sp_str_t        sp_cli_result_to_str(sp_cli_result_t result);
SP_API sp_str_t        sp_cli_err_kind_to_str(sp_cli_err_kind_t kind);
SP_API sp_cli_t        sp_cli_parse(sp_cli_desc_t desc);
SP_API sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli);
SP_API sp_cli_result_t sp_cli_run(sp_cli_desc_t desc);
SP_API s32             sp_cli_main(sp_cli_desc_t desc);
SP_API void            sp_cli_write_help(sp_io_writer_t* io, sp_cli_t* cli);
SP_API sp_cli_result_t sp_cli_set_error(sp_cli_t* cli, sp_str_t error);
SP_API sp_cli_result_t sp_cli_set_error_c(sp_cli_t* cli, const c8* error);

#endif // SP_CLI_H

#if defined(SP_IMPLEMENTATION) && !defined(SP_CLI_IMPLEMENTATION)
  #define SP_CLI_IMPLEMENTATION
#endif

#if defined(SP_CLI_IMPLEMENTATION) && !defined(SP_CLI_IMPLEMENTED)
#define SP_CLI_IMPLEMENTED

typedef struct sp_cli_scope {
  sp_cli_cmd_t* cmd;
  struct sp_cli_scope* parent;
} sp_cli_scope_t;

typedef struct {
  sp_cli_t* cli;
  const c8** args;
  u32 num_args;
  u32 it;
  bool raw;
  bool help;
  const c8* positionals [SP_CLI_MAX_ARGS];
  u32 num_positionals;
} sp_cli_parser_t;

SP_PRIVATE sp_err_t sp_cli_fail(sp_cli_t* cli, sp_cli_err_t err) {
  cli->err = err;
  return SP_ERR;
}

SP_PRIVATE u32 sp_cli_num_fixed_args(sp_cli_cmd_t* cmd) {
  u32 num = 0;
  sp_carr_for(cmd->args, it) {
    if (!cmd->args[it].name) break;
    if (cmd->args[it].kind == SP_CLI_ARG_REST) break;
    num++;
  }
  return num;
}

SP_PRIVATE bool sp_cli_has_commands(sp_cli_cmd_t* cmd) {
  return cmd->commands[0] != SP_NULLPTR;
}

SP_PRIVATE bool sp_cli_has_rest(sp_cli_cmd_t* cmd) {
  sp_carr_for(cmd->args, it) {
    if (!cmd->args[it].name) break;
    if (cmd->args[it].kind == SP_CLI_ARG_REST) return true;
  }
  return false;
}

SP_PRIVATE bool sp_cli_done(sp_cli_parser_t* parser) {
  return parser->it >= parser->num_args;
}

SP_PRIVATE sp_str_t sp_cli_peek(sp_cli_parser_t* parser) {
  if (sp_cli_done(parser)) return sp_zero_s(sp_str_t);
  return sp_cstr_as_str(parser->args[parser->it]);
}

SP_PRIVATE void sp_cli_eat(sp_cli_parser_t* parser) {
  parser->it++;
}

SP_PRIVATE bool sp_cli_at_opt(sp_cli_parser_t* parser) {
  sp_str_t arg = sp_cli_peek(parser);
  return arg.len > 1 && sp_str_at(arg, 0) == '-';
}

SP_PRIVATE sp_cli_opt_t* sp_cli_find_opt(sp_cli_scope_t* scope, sp_str_t name) {
  for (sp_cli_scope_t* it = scope; it; it = it->parent) {
    sp_carr_for(it->cmd->opts, i) {
      sp_cli_opt_t* opt = &it->cmd->opts[i];
      if (!opt->name) break;
      if (sp_str_equal_cstr(name, opt->name)) return opt;
    }
  }
  return SP_NULLPTR;
}

SP_PRIVATE sp_cli_opt_t* sp_cli_find_brief(sp_cli_scope_t* scope, c8 brief) {
  for (sp_cli_scope_t* it = scope; it; it = it->parent) {
    sp_carr_for(it->cmd->opts, i) {
      sp_cli_opt_t* opt = &it->cmd->opts[i];
      if (!opt->name) break;
      if (opt->brief && opt->brief[0] == brief) return opt;
    }
  }
  return SP_NULLPTR;
}

SP_PRIVATE sp_cli_err_t sp_cli_assign(sp_cli_value_kind_t kind, void* ptr, sp_str_t value) {
  switch (kind) {
    case SP_CLI_OPT_BOOLEAN: {
      bool parsed = true;
      if (!sp_str_empty(value) && !sp_parse_bool_ex(value, &parsed)) {
        return (sp_cli_err_t) { .kind = SP_CLI_ERR_INVALID_VALUE, .value = value };
      }
      if (ptr) *sp_cast(bool*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_STRING: {
      // Every value is either a whole element of desc.args or a NUL-terminated
      // tail of one (the text after '=' or after a brief cluster), so string
      // options borrow the args array directly instead of copying. A null value
      // passes through as a null pointer, signalling "not set".
      if (ptr) *sp_cast(const c8**, ptr) = value.data;
      break;
    }
    case SP_CLI_OPT_INTEGER: {
      s64 parsed = 0;
      if (!sp_parse_s64_ex(value, &parsed)) {
        return (sp_cli_err_t) { .kind = SP_CLI_ERR_INVALID_VALUE, .value = value };
      }
      if (ptr) *sp_cast(s64*, ptr) = parsed;
      break;
    }
  }
  return sp_zero_s(sp_cli_err_t);
}

SP_PRIVATE sp_err_t sp_cli_assign_opt(sp_cli_parser_t* parser, sp_cli_opt_t* opt, sp_str_t value) {
  if (opt->kind != SP_CLI_OPT_BOOLEAN && sp_str_empty(value)) {
    return sp_cli_fail(parser->cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_MISSING_VALUE,
      .name = sp_cstr_as_str(opt->name),
    });
  }

  sp_cli_err_t err = sp_cli_assign(opt->kind, opt->ptr, value);
  if (err.kind != SP_CLI_ERR_NONE) {
    err.name = sp_cstr_as_str(opt->name);
    return sp_cli_fail(parser->cli, err);
  }
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_cli_value(sp_cli_parser_t* parser, sp_cli_opt_t* opt, sp_str_t* value) {
  sp_str_t next = sp_cli_peek(parser);
  if (sp_cli_done(parser) || sp_str_starts_with(next, sp_str_lit("--"))) {
    return sp_cli_fail(parser->cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_MISSING_VALUE,
      .name = sp_cstr_as_str(opt->name),
    });
  }
  *value = next;
  sp_cli_eat(parser);
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_cli_parse_long(sp_cli_parser_t* parser, sp_cli_scope_t* scope) {
  sp_str_t body = sp_str_strip_left(sp_cli_peek(parser), sp_str_lit("--"));
  sp_cli_eat(parser);

  sp_str_t name = body;
  sp_str_t value = sp_zero_s(sp_str_t);
  bool has_value = false;
  sp_str_for(body, it) {
    if (sp_str_at(body, it) == '=') {
      name = sp_str_prefix(body, it);
      value = sp_str_suffix(body, body.len - (it + 1));
      has_value = true;
      break;
    }
  }

  sp_cli_opt_t* opt = sp_cli_find_opt(scope, name);
  if (!opt) {
    if (sp_str_equal(name, sp_str_lit("help"))) {
      parser->help = true;
      return SP_OK;
    }
    return sp_cli_fail(parser->cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_UNKNOWN_OPT,
      .name = name,
    });
  }

  if (!has_value && opt->kind != SP_CLI_OPT_BOOLEAN) {
    sp_try(sp_cli_value(parser, opt, &value));
  }

  return sp_cli_assign_opt(parser, opt, value);
}

SP_PRIVATE sp_err_t sp_cli_parse_briefs(sp_cli_parser_t* parser, sp_cli_scope_t* scope) {
  sp_str_t cluster = sp_str_strip_left(sp_cli_peek(parser), sp_str_lit("-"));
  sp_cli_eat(parser);

  sp_str_for(cluster, it) {
    c8 brief = sp_str_at(cluster, it);
    sp_cli_opt_t* opt = sp_cli_find_brief(scope, brief);
    if (!opt) {
      if (brief == 'h') {
        parser->help = true;
        return SP_OK;
      }
      return sp_cli_fail(parser->cli, (sp_cli_err_t) {
        .kind = SP_CLI_ERR_UNKNOWN_BRIEF,
        .name = sp_str_sub(cluster, it, 1),
      });
    }

    sp_str_t value = sp_zero_s(sp_str_t);
    if (opt->kind != SP_CLI_OPT_BOOLEAN) {
      if (it + 1 < cluster.len) {
        value = sp_str_suffix(cluster, cluster.len - (it + 1));
      }
      else {
        sp_try(sp_cli_value(parser, opt, &value));
      }
      return sp_cli_assign_opt(parser, opt, value);
    }

    sp_try(sp_cli_assign_opt(parser, opt, value));
  }

  return SP_OK;
}

SP_PRIVATE sp_err_t sp_cli_parse_cmd(sp_cli_parser_t* parser, sp_cli_scope_t scope) {
  sp_cli_t* cli = parser->cli;
  sp_cli_cmd_t* cmd = scope.cmd;
  cli->cmd = cmd;
  if (cli->depth < SP_CLI_MAX_DEPTH) {
    cli->path[cli->depth++] = cmd;
  }

  u32 num_fixed = sp_cli_num_fixed_args(cmd);

  while (!sp_cli_done(parser)) {
    sp_str_t arg = sp_cli_peek(parser);

    if (!parser->raw && sp_str_equal(arg, sp_str_lit("--"))) {
      sp_cli_eat(parser);
      parser->raw = true;
    }
    else if (!parser->raw && sp_str_starts_with(arg, sp_str_lit("--"))) {
      sp_try(sp_cli_parse_long(parser, &scope));
      if (parser->help) return SP_OK;
    }
    else if (!parser->raw && sp_cli_at_opt(parser)) {
      sp_try(sp_cli_parse_briefs(parser, &scope));
      if (parser->help) return SP_OK;
    }
    else {
      if (sp_cli_has_commands(cmd)) break;
      if (parser->num_positionals >= num_fixed && sp_cli_has_rest(cmd)) {
        cli->num_rest = parser->num_args - parser->it;
        cli->rest = parser->args + parser->it;
        parser->it = parser->num_args;
        break;
      }
      if (parser->num_positionals >= SP_CLI_MAX_ARGS || parser->num_positionals >= num_fixed) {
        return sp_cli_fail(cli, (sp_cli_err_t) {
          .kind = SP_CLI_ERR_UNEXPECTED_ARG,
          .value = arg,
        });
      }
      parser->positionals[parser->num_positionals++] = parser->args[parser->it];
      sp_cli_eat(parser);
    }
  }

  sp_carr_for(cmd->args, it) {
    sp_cli_arg_t* arg = &cmd->args[it];
    if (!arg->name) break;

    if (it < parser->num_positionals) {
      if (arg->ptr) *arg->ptr = parser->positionals[it];
    }
    else if (arg->kind == SP_CLI_ARG_REQUIRED) {
      return sp_cli_fail(cli, (sp_cli_err_t) {
        .kind = SP_CLI_ERR_MISSING_ARG,
        .name = sp_cstr_as_str(arg->name),
      });
    }
  }

  if (sp_cli_has_commands(cmd) && !sp_cli_done(parser)) {
    sp_str_t name = sp_cli_peek(parser);
    sp_carr_for(cmd->commands, it) {
      sp_cli_cmd_t* sub = cmd->commands[it];
      if (!sub) break;
      if (sp_str_equal_cstr(name, sub->name)) {
        sp_cli_eat(parser);
        sp_cli_scope_t child = { .cmd = sub, .parent = &scope };
        return sp_cli_parse_cmd(parser, child);
      }
    }
    return sp_cli_fail(cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_UNKNOWN_COMMAND,
      .name = name,
    });
  }

  if (!cmd->handler) parser->help = true;
  return SP_OK;
}

SP_PRIVATE sp_str_t sp_cli_opt_label(c8* buf, u32 len, sp_cli_opt_t* opt) {
  sp_io_mem_writer_t label = sp_zero;
  sp_io_mem_writer_from_buffer(&label, buf, len);

  if (opt->brief) {
    sp_fmt_io(&label.base, "-{}, ", sp_fmt_cstr(opt->brief));
  }
  else {
    sp_fmt_io(&label.base, "    ");
  }
  sp_fmt_io(&label.base, "--{}", sp_fmt_cstr(opt->name));
  if (opt->placeholder) {
    sp_fmt_io(&label.base, " {}", sp_fmt_cstr(opt->placeholder));
  }

  return sp_io_mem_writer_as_str(&label);
}

SP_PRIVATE sp_str_t sp_cli_arg_label(c8* buf, u32 len, sp_cli_arg_t* arg) {
  sp_io_mem_writer_t label = sp_zero;
  sp_io_mem_writer_from_buffer(&label, buf, len);

  switch (arg->kind) {
    case SP_CLI_ARG_REQUIRED: {
      sp_fmt_io(&label.base, "{}", sp_fmt_cstr(arg->name));
      break;
    }
    case SP_CLI_ARG_OPTIONAL: {
      sp_fmt_io(&label.base, "[{}]", sp_fmt_cstr(arg->name));
      break;
    }
    case SP_CLI_ARG_REST: {
      sp_fmt_io(&label.base, "[{}...]", sp_fmt_cstr(arg->name));
      break;
    }
  }

  return sp_io_mem_writer_as_str(&label);
}

sp_str_t sp_cli_arg_kind_to_str(sp_cli_arg_kind_t kind) {
  switch (kind) {
    case SP_CLI_ARG_REQUIRED: { return sp_str_lit("required"); }
    case SP_CLI_ARG_OPTIONAL: { return sp_str_lit("optional"); }
    case SP_CLI_ARG_REST:     { return sp_str_lit("rest"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_opt_kind_to_str(sp_cli_value_kind_t kind) {
  switch (kind) {
    case SP_CLI_OPT_BOOLEAN: { return sp_str_lit("boolean"); }
    case SP_CLI_OPT_STRING:  { return sp_str_lit("string"); }
    case SP_CLI_OPT_INTEGER: { return sp_str_lit("integer"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_result_to_str(sp_cli_result_t result) {
  switch (result) {
    case SP_CLI_OK:       { return sp_str_lit("ok"); }
    case SP_CLI_ERR:      { return sp_str_lit("error"); }
    case SP_CLI_HELP:     { return sp_str_lit("help"); }
    case SP_CLI_CONTINUE: { return sp_str_lit("continue"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_err_kind_to_str(sp_cli_err_kind_t kind) {
  switch (kind) {
    case SP_CLI_ERR_NONE:            { return sp_str_lit("none"); }
    case SP_CLI_ERR_CUSTOM:          { return sp_str_lit("custom"); }
    case SP_CLI_ERR_UNKNOWN_OPT:     { return sp_str_lit("unknown_opt"); }
    case SP_CLI_ERR_UNKNOWN_BRIEF:   { return sp_str_lit("unknown_brief"); }
    case SP_CLI_ERR_INVALID_VALUE:   { return sp_str_lit("invalid_value"); }
    case SP_CLI_ERR_MISSING_VALUE:   { return sp_str_lit("missing_value"); }
    case SP_CLI_ERR_MISSING_ARG:     { return sp_str_lit("missing_arg"); }
    case SP_CLI_ERR_UNEXPECTED_ARG:  { return sp_str_lit("unexpected_arg"); }
    case SP_CLI_ERR_UNKNOWN_COMMAND: { return sp_str_lit("unknown_command"); }
    case SP_CLI_ERR_MISSING_ENV:     { return sp_str_lit("missing_env"); }
    case SP_CLI_ERR_INVALID_ENV:     { return sp_str_lit("invalid_env"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

SP_PRIVATE sp_cli_theme_t sp_cli_theme_resolve(sp_cli_theme_t theme);

SP_PRIVATE sp_err_t sp_cli_resolve_env(sp_cli_t* cli) {
  sp_for(i, cli->depth) {
    sp_cli_cmd_t* cmd = cli->path[i];
    sp_carr_for(cmd->env, it) {
      sp_cli_env_t* var = &cmd->env[it];
      if (!var->name) break;

      sp_str_t value = sp_os_env_get(sp_cstr_as_str(var->name));
      if (sp_str_empty(value)) {
        if (var->required) {
          return sp_cli_fail(cli, (sp_cli_err_t) {
            .kind = SP_CLI_ERR_MISSING_ENV,
            .name = sp_cstr_as_str(var->name),
          });
        }
        continue;
      }

      sp_cli_err_t err = sp_cli_assign(var->kind, var->ptr, value);
      if (err.kind != SP_CLI_ERR_NONE) {
        err.kind = SP_CLI_ERR_INVALID_ENV;
        err.name = sp_cstr_as_str(var->name);
        return sp_cli_fail(cli, err);
      }
    }
  }
  return SP_OK;
}

sp_cli_t sp_cli_parse(sp_cli_desc_t desc) {
  sp_cli_t cli = sp_zero_s(sp_cli_t);
  cli.user_data = desc.user_data;
  cli.theme = sp_cli_theme_resolve(desc.theme);

  sp_cli_parser_t parser = sp_zero_s(sp_cli_parser_t);
  parser.cli = &cli;
  parser.args = desc.num_args > 1 ? desc.args + 1 : SP_NULLPTR;
  parser.num_args = desc.num_args > 1 ? sp_cast(u32, desc.num_args - 1) : 0;

  sp_cli_scope_t scope = { .cmd = desc.root, .parent = SP_NULLPTR };
  sp_err_t parsed = sp_cli_parse_cmd(&parser, scope);

  if (parsed) {
    cli.status = SP_CLI_ERR;
  }
  else if (parser.help) {
    cli.status = SP_CLI_HELP;
  }
  else if (sp_cli_resolve_env(&cli)) {
    // Only resolve the environment if everything else parsed; otherwise,
    // the error produced by a missing required env var swallows genuine
    // requests for help
    cli.status = SP_CLI_ERR;
  }
  else {
    cli.status = SP_CLI_OK;
  }

  return cli;
}

sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli) {
  if (cli->status) return cli->status;
  if (!cli->cmd->handler) return cli->status;
  return cli->cmd->handler(cli);
}

#define SP_CLI_THEME_ARGS(entry) \
  sp_fmt_style((entry).color), \
  sp_fmt_style((entry).attribute)

SP_PRIVATE sp_fmt_style_t sp_cli_style_resolve(sp_fmt_style_t style, sp_fmt_style_t base) {
  if (style == sp_fmt_style_unset) return sp_fmt_style_none;
  return style ? style : base;
}

SP_PRIVATE sp_cli_theme_entry_t sp_cli_entry_resolve(sp_cli_theme_entry_t entry, sp_cli_theme_entry_t base) {
  return (sp_cli_theme_entry_t) {
    .color     = sp_cli_style_resolve(entry.color,     base.color),
    .attribute = sp_cli_style_resolve(entry.attribute, base.attribute),
  };
}

SP_PRIVATE sp_cli_theme_t sp_cli_theme_resolve(sp_cli_theme_t theme) {
  if (theme.mode == SP_CLI_THEME_REPLACE) {
    return theme;
  }

  sp_cli_theme_t base = {
    .heading = { .color = sp_fmt_style_green, .attribute = sp_fmt_style_bold },
    .command = { .color = sp_fmt_style_cyan, .attribute = sp_fmt_style_bold },
    .label = { .color = sp_fmt_style_cyan, .attribute = sp_fmt_style_bold },
    .hint = { .color = sp_fmt_style_gray },
    .error = { .color = sp_fmt_style_red },
  };

  return (sp_cli_theme_t) {
    .heading = sp_cli_entry_resolve(theme.heading, base.heading),
    .command = sp_cli_entry_resolve(theme.command, base.command),
    .label = sp_cli_entry_resolve(theme.label, base.label),
    .hint = sp_cli_entry_resolve(theme.hint, base.hint),
    .error = sp_cli_entry_resolve(theme.error, base.error),
  };
}

void sp_cli_err_print(sp_io_writer_t* io, sp_cli_err_t err) {
  switch (err.kind) {
    case SP_CLI_ERR_NONE: {
      break;
    }
    case SP_CLI_ERR_CUSTOM: {
      sp_fmt_io(io, "{}", sp_fmt_str(err.value));
      break;
    }
    case SP_CLI_ERR_UNKNOWN_OPT: {
      sp_fmt_io(io, "unknown option: --{}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_UNKNOWN_BRIEF: {
      sp_fmt_io(io, "unknown option: -{}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_INVALID_VALUE: {
      sp_fmt_io(io, "invalid value for option --{}: {.quote}", sp_fmt_str(err.name), sp_fmt_str(err.value));
      break;
    }
    case SP_CLI_ERR_MISSING_VALUE: {
      sp_fmt_io(io, "missing value for option: --{}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_MISSING_ARG: {
      sp_fmt_io(io, "missing required argument: {}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_UNEXPECTED_ARG: {
      sp_fmt_io(io, "unexpected argument: {}", sp_fmt_str(err.value));
      break;
    }
    case SP_CLI_ERR_UNKNOWN_COMMAND: {
      sp_fmt_io(io, "unknown command: {}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_MISSING_ENV: {
      sp_fmt_io(io, "missing required environment variable: {}", sp_fmt_str(err.name));
      break;
    }
    case SP_CLI_ERR_INVALID_ENV: {
      sp_fmt_io(io, "invalid value for environment variable {}: {.quote}", sp_fmt_str(err.name), sp_fmt_str(err.value));
      break;
    }
  }
}

SP_PRIVATE void sp_cli_write_diagnostic(sp_io_writer_t* io, sp_cli_err_t err, const c8* label, sp_cli_theme_entry_t theme) {
  sp_fmt_io(io, "{.$ .$}: ", SP_CLI_THEME_ARGS(theme), sp_fmt_cstr(label));
  sp_cli_err_print(io, err);
  sp_fmt_io(io, "\n");
}

SP_PRIVATE void sp_cli_write_heading(sp_io_writer_t* io, sp_cli_theme_entry_t entry, const c8* name) {
  sp_fmt_io(io, "\n");
  sp_fmt_io(io, "{.$ .$}", SP_CLI_THEME_ARGS(entry), sp_fmt_cstr(name));
  sp_fmt_io(io, "\n");
}

SP_PRIVATE void sp_cli_write_label(sp_io_writer_t* io, sp_cli_theme_entry_t entry, sp_str_t label, sp_str_t summary, u32 width) {
  sp_fmt_io(io, "  {:<$ .$ .$} {}",
    sp_fmt_uint(width), SP_CLI_THEME_ARGS(entry), sp_fmt_str(label),
    sp_fmt_str(summary));
  sp_fmt_io(io, "\n");
}

SP_PRIVATE void sp_cli_write_label_opt(sp_io_writer_t* io, sp_cli_theme_entry_t label_entry, sp_cli_theme_entry_t hint_entry, sp_str_t label, sp_str_t summary, u32 width, bool required) {
  sp_cli_theme_entry_t hint_style = required ? sp_zero_s(sp_cli_theme_entry_t) : hint_entry;
  const c8* hint = required ? "required" : "optional";
  sp_fmt_io(io, "  {:<$ .$ .$} {.$ .$} {}",
    sp_fmt_uint(width), SP_CLI_THEME_ARGS(label_entry), sp_fmt_str(label),
    SP_CLI_THEME_ARGS(hint_style), sp_fmt_cstr(hint),
    sp_fmt_str(summary));
  sp_fmt_io(io, "\n");
}

typedef struct {
  sp_cli_opt_t* opts [SP_CLI_MAX_OPTS * SP_CLI_MAX_DEPTH];
  sp_cli_env_t* env  [SP_CLI_MAX_ENV * SP_CLI_MAX_DEPTH];
  sp_cli_arg_t* args [SP_CLI_MAX_ARGS];
  sp_cli_cmd_t* commands [SP_CLI_MAX_COMMANDS];
  u32 num_opts;
  u32 num_env;
  u32 num_args;
  u32 num_commands;
} sp_cli_view_t;

SP_PRIVATE void sp_cli_view_put_opt(sp_cli_view_t* view, sp_cli_opt_t* opt) {
  sp_for(it, view->num_opts) {
    if (sp_str_equal_cstr(sp_cstr_as_str(opt->name), view->opts[it]->name)) {
      view->opts[it] = opt;
      return;
    }
  }
  view->opts[view->num_opts++] = opt;
}

SP_PRIVATE void sp_cli_view_put_env(sp_cli_view_t* view, sp_cli_env_t* var) {
  sp_for(it, view->num_env) {
    if (sp_str_equal_cstr(sp_cstr_as_str(var->name), view->env[it]->name)) {
      view->env[it] = var;
      return;
    }
  }
  view->env[view->num_env++] = var;
}

SP_PRIVATE sp_cli_view_t sp_cli_view(sp_cli_t* cli) {
  sp_cli_view_t view = sp_zero;

  sp_for(i, cli->depth) {
    sp_cli_cmd_t* scope = cli->path[i];
    sp_carr_for_until(scope->opts, it, scope->opts[it].name) {
      sp_cli_view_put_opt(&view, &scope->opts[it]);
    }
    sp_carr_for_until(scope->env, it, scope->env[it].name) {
      sp_cli_view_put_env(&view, &scope->env[it]);
    }
  }

  sp_carr_for_until(cli->cmd->args, it, cli->cmd->args[it].name) {
    view.args[view.num_args++] = &cli->cmd->args[it];
  }
  sp_carr_for_until(cli->cmd->commands, it, cli->cmd->commands[it]) {
    view.commands[view.num_commands++] = cli->cmd->commands[it];
  }

  return view;
}

SP_PRIVATE void sp_cli_write_synopsis(sp_io_writer_t* io, sp_cli_t* cli, sp_cli_view_t* view) {
  sp_cli_theme_t theme = cli->theme;

  sp_cli_write_heading(io, theme.heading, "usage");

  sp_fmt_io(io, "  ");
  sp_for(it, cli->depth) {
    if (it) sp_fmt_io(io, " ");
    sp_fmt_io(io, "{.$ .$}", SP_CLI_THEME_ARGS(theme.command), sp_fmt_cstr(cli->path[it]->name));
  }

  if (view->num_opts) sp_fmt_io(io, " [OPTIONS]");

  sp_for(it, view->num_args) {
    c8 buffer [SP_CLI_MAX_LABEL];
    sp_str_t label = sp_cli_arg_label(buffer, SP_CLI_MAX_LABEL, view->args[it]);
    sp_fmt_io(io, " {}", sp_fmt_str(label));
  }

  if (view->num_commands) sp_fmt_io(io, " <COMMAND>");
  sp_fmt_io(io, "\n");
}

void sp_cli_write_help(sp_io_writer_t* io, sp_cli_t* cli) {
  sp_cli_theme_t theme = cli->theme;
  sp_cli_view_t view = sp_cli_view(cli);

  if (cli->cmd->summary) {
    sp_io_write_cstr(io, cli->cmd->summary, SP_NULLPTR);
    sp_fmt_io(io, "\n");
  }

  sp_cli_write_synopsis(io, cli, &view);

  if (view.num_commands) {
    u32 width = 0;
    sp_for(it, view.num_commands) {
      width = sp_max(width, sp_cstr_as_str(view.commands[it]->name).len);
    }
    sp_cli_write_heading(io, theme.heading, "commands");
    sp_for(it, view.num_commands) {
      sp_cli_cmd_t* sub = view.commands[it];
      sp_cli_write_label(io, theme.label, sp_cstr_as_str(sub->name), sp_cstr_as_str(sub->summary ? sub->summary : ""), width);
    }
  }

  if (view.num_opts) {
    u32 width = 0;
    sp_for(it, view.num_opts) {
      c8 buffer [SP_CLI_MAX_LABEL];
      width = sp_max(width, sp_cli_opt_label(buffer, SP_CLI_MAX_LABEL, view.opts[it]).len);
    }
    sp_cli_write_heading(io, theme.heading, "options");
    sp_for(it, view.num_opts) {
      c8 buffer [SP_CLI_MAX_LABEL];
      sp_cli_opt_t* opt = view.opts[it];
      sp_str_t label = sp_cli_opt_label(buffer, SP_CLI_MAX_LABEL, opt);
      sp_cli_write_label(io, theme.label, label, sp_cstr_as_str(opt->summary ? opt->summary : ""), width);
    }
  }

  if (view.num_args) {
    u32 width = 0;
    sp_for(it, view.num_args) {
      c8 buffer [SP_CLI_MAX_LABEL];
      width = sp_max(width, sp_cli_arg_label(buffer, SP_CLI_MAX_LABEL, view.args[it]).len);
    }
    sp_cli_write_heading(io, theme.heading, "arguments");
    sp_for(it, view.num_args) {
      c8 buffer [SP_CLI_MAX_LABEL];
      sp_cli_arg_t* arg = view.args[it];
      sp_str_t label = sp_cli_arg_label(buffer, SP_CLI_MAX_LABEL, arg);
      sp_cli_write_label_opt(io, theme.label, theme.hint, label, sp_cstr_as_str(arg->summary ? arg->summary : ""), width, arg->kind == SP_CLI_ARG_REQUIRED);
    }
  }

  if (view.num_env) {
    u32 width = 0;
    sp_for(it, view.num_env) {
      width = sp_max(width, sp_cstr_as_str(view.env[it]->name).len);
    }
    sp_cli_write_heading(io, theme.heading, "environment");
    sp_for(it, view.num_env) {
      sp_cli_env_t* var = view.env[it];
      sp_cli_write_label_opt(io, theme.label, theme.hint, sp_cstr_as_str(var->name), sp_cstr_as_str(var->summary ? var->summary : ""), width, var->required);
    }
  }
}

sp_cli_result_t sp_cli_set_error(sp_cli_t* cli, sp_str_t error) {
  cli->err.kind = SP_CLI_ERR_CUSTOM;
  cli->err.value = error;
  return SP_CLI_ERR;
}

sp_cli_result_t sp_cli_set_error_c(sp_cli_t* cli, const c8* error) {
  return sp_cli_set_error(cli, sp_cstr_as_str(error));
}

sp_cli_result_t sp_cli_run(sp_cli_desc_t desc) {
  struct { sp_io_stream_writer_t out; sp_io_stream_writer_t err; } ios = {
    sp_io_get_std_out(), sp_io_get_std_err()
  };
  struct { sp_io_writer_t* out; sp_io_writer_t* err; } io = {
    &ios.out.base, &ios.err.base
  };

  sp_cli_t cli = sp_cli_parse(desc);
  if (!cli.status) {
    cli.status = sp_cli_dispatch(&cli);
  }

  switch (cli.status) {
    case SP_CLI_OK:
    case SP_CLI_CONTINUE: {
      break;
    }
    case SP_CLI_HELP: {
      sp_cli_write_help(io.out, &cli);
      break;
    }
    case SP_CLI_ERR: {
      sp_cli_view_t view = sp_cli_view(&cli);
      sp_cli_write_diagnostic(io.err, cli.err, "error", cli.theme.error);
      sp_cli_write_synopsis(io.err, &cli, &view);
      sp_fmt_io(io.err, "\n");
      sp_fmt_io(io.err, "Use {.cyan} for full usage", sp_fmt_cstr("--help"));
      sp_fmt_io(io.err, "\n");
      break;
    }
  }

  return cli.status;
}

s32 sp_cli_main(sp_cli_desc_t desc) {
  switch (sp_cli_run(desc)) {
    case SP_CLI_OK: return 0;
    case SP_CLI_HELP: return 0;
    case SP_CLI_CONTINUE: return 0;
    case SP_CLI_ERR: return 1;
  }
  SP_UNREACHABLE_RETURN(1);
}

#endif // SP_CLI_IMPLEMENTATION
