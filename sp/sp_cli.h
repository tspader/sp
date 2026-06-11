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

#ifndef SP_CLI_MAX_LABEL
  #define SP_CLI_MAX_LABEL 64
#endif

#define SP_CLI_ARG_KIND(X) \
  X(SP_CLI_ARG_REQUIRED, "required") \
  X(SP_CLI_ARG_OPTIONAL, "optional") \
  X(SP_CLI_ARG_REST, "rest")

typedef enum {
  SP_CLI_ARG_KIND(SP_X_NAMED_ENUM_DEFINE)
} sp_cli_arg_kind_t;

#define SP_CLI_OPT_KIND(X) \
  X(SP_CLI_OPT_BOOLEAN, "boolean") \
  X(SP_CLI_OPT_STRING, "string") \
  X(SP_CLI_OPT_INTEGER, "integer")

typedef enum {
  SP_CLI_OPT_KIND(SP_X_NAMED_ENUM_DEFINE)
} sp_cli_opt_kind_t;

#define SP_CLI_RESULT(X) \
  X(SP_CLI_OK, "ok") \
  X(SP_CLI_ERR, "error") \
  X(SP_CLI_HELP, "help") \
  X(SP_CLI_CONTINUE, "continue")

typedef enum {
  SP_CLI_RESULT(SP_X_NAMED_ENUM_DEFINE)
} sp_cli_result_t;

#define SP_CLI_ERR_KIND(X) \
  X(SP_CLI_ERR_NONE, "none") \
  X(SP_CLI_ERR_UNKNOWN_OPT, "unknown_opt") \
  X(SP_CLI_ERR_UNKNOWN_BRIEF, "unknown_brief") \
  X(SP_CLI_ERR_INVALID_VALUE, "invalid_value") \
  X(SP_CLI_ERR_MISSING_VALUE, "missing_value") \
  X(SP_CLI_ERR_MISSING_ARG, "missing_arg") \
  X(SP_CLI_ERR_UNEXPECTED_ARG, "unexpected_arg") \
  X(SP_CLI_ERR_UNKNOWN_COMMAND, "unknown_command") \
  X(SP_CLI_ERR_NO_HANDLER, "no_handler")

typedef enum {
  SP_CLI_ERR_KIND(SP_X_NAMED_ENUM_DEFINE)
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
  sp_cli_opt_kind_t kind;
  const c8* summary;
  const c8* placeholder;
  void* ptr;
} sp_cli_opt_t;

#define SP_CLI_NO_OPTS sp_zero
#define SP_CLI_NO_ARGS sp_zero
#define SP_CLI_NO_CMDS sp_zero
#define SP_CLI_NO_PLACEHOLDER SP_NULLPTR

struct sp_cli_cmd {
  const c8* name;
  const c8* summary;
  sp_cli_opt_t opts [SP_CLI_MAX_OPTS];
  sp_cli_arg_t args [SP_CLI_MAX_ARGS];
  sp_cli_cmd_t* commands [SP_CLI_MAX_COMMANDS];
  sp_cli_handler_t handler;
};

typedef struct {
  sp_cli_cmd_t* root;
  const c8** args;
  u32 num_args;
  void* user_data;
} sp_cli_desc_t;

struct sp_cli {
  void* user_data;
  sp_cli_result_t status;
  sp_cli_err_t err;
  sp_cli_cmd_t* cmd;
  const c8** rest;
  u32 num_rest;
};

SP_API sp_str_t        sp_cli_arg_kind_to_str(sp_cli_arg_kind_t kind);
SP_API sp_str_t        sp_cli_opt_kind_to_str(sp_cli_opt_kind_t kind);
SP_API sp_str_t        sp_cli_result_to_str(sp_cli_result_t result);
SP_API sp_str_t        sp_cli_err_kind_to_str(sp_cli_err_kind_t kind);
SP_API sp_cli_t        sp_cli_parse(sp_cli_desc_t desc);
SP_API sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli);
SP_API sp_cli_result_t sp_cli_run(sp_cli_cmd_t* root, s32 num_args, const c8** args, void* user_data);
SP_API s32             sp_cli_main(sp_cli_cmd_t* root, s32 num_args, const c8** args, void* user_data);
SP_API void            sp_cli_usage_write(sp_io_writer_t* io, sp_cli_cmd_t* cmd);
SP_API void            sp_cli_err_write(sp_io_writer_t* io, sp_cli_err_t* err);
SP_API void            sp_cli_log_error(const c8* fmt, ...);
SP_API void            sp_cli_log_error_v(sp_str_t fmt, va_list args);

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

SP_PRIVATE sp_str_t sp_cli_str(const c8* cstr) {
  return cstr ? sp_str_view(cstr) : sp_zero_s(sp_str_t);
}

SP_PRIVATE sp_err_t sp_cli_fail(sp_cli_t* cli, sp_cli_err_t err) {
  cli->err = err;
  return SP_ERR;
}

SP_PRIVATE u32 sp_cli_num_opts(sp_cli_cmd_t* cmd) {
  u32 num = 0;
  sp_carr_for(cmd->opts, it) {
    if (!cmd->opts[it].name) break;
    num++;
  }
  return num;
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
  return sp_str_view(parser->args[parser->it]);
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

SP_PRIVATE sp_err_t sp_cli_assign(sp_cli_parser_t* parser, sp_cli_opt_t* opt, sp_str_t value) {
  switch (opt->kind) {
    case SP_CLI_OPT_BOOLEAN: {
      bool parsed = true;
      if (!sp_str_empty(value) && !sp_parse_bool_ex(value, &parsed)) {
        return sp_cli_fail(parser->cli, (sp_cli_err_t) {
          .kind = SP_CLI_ERR_INVALID_VALUE,
          .name = sp_cli_str(opt->name),
          .value = value,
        });
      }
      if (opt->ptr) *sp_cast(bool*, opt->ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_STRING: {
      // Every value is either a whole element of desc.args or a NUL-terminated
      // tail of one (the text after '=' or after a brief cluster), so string
      // options borrow the args array directly instead of copying.
      if (opt->ptr) *sp_cast(const c8**, opt->ptr) = value.data ? value.data : "";
      break;
    }
    case SP_CLI_OPT_INTEGER: {
      s64 parsed = 0;
      if (!sp_parse_s64_ex(value, &parsed)) {
        return sp_cli_fail(parser->cli, (sp_cli_err_t) {
          .kind = SP_CLI_ERR_INVALID_VALUE,
          .name = sp_cli_str(opt->name),
          .value = value,
        });
      }
      if (opt->ptr) *sp_cast(s64*, opt->ptr) = parsed;
      break;
    }
  }
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_cli_value(sp_cli_parser_t* parser, sp_cli_opt_t* opt, sp_str_t* value) {
  sp_str_t next = sp_cli_peek(parser);
  if (sp_cli_done(parser) || sp_str_starts_with(next, sp_str_lit("--"))) {
    return sp_cli_fail(parser->cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_MISSING_VALUE,
      .name = sp_cli_str(opt->name),
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

  return sp_cli_assign(parser, opt, value);
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
      return sp_cli_assign(parser, opt, value);
    }

    sp_try(sp_cli_assign(parser, opt, value));
  }

  return SP_OK;
}

SP_PRIVATE sp_err_t sp_cli_parse_cmd(sp_cli_parser_t* parser, sp_cli_scope_t scope) {
  sp_cli_t* cli = parser->cli;
  sp_cli_cmd_t* cmd = scope.cmd;
  cli->cmd = cmd;

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
        .name = sp_cli_str(arg->name),
      });
    }
  }

  if (sp_cli_has_commands(cmd)) {
    if (!sp_cli_done(parser)) {
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

    if (!cmd->handler) {
      parser->help = true;
      return SP_OK;
    }
  }

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
      sp_fmt_io(&label.base, "<{}>", sp_fmt_cstr(arg->name));
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
    SP_CLI_ARG_KIND(SP_X_NAMED_ENUM_CASE_TO_STRING)
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_opt_kind_to_str(sp_cli_opt_kind_t kind) {
  switch (kind) {
    SP_CLI_OPT_KIND(SP_X_NAMED_ENUM_CASE_TO_STRING)
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_result_to_str(sp_cli_result_t result) {
  switch (result) {
    SP_CLI_RESULT(SP_X_NAMED_ENUM_CASE_TO_STRING)
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_err_kind_to_str(sp_cli_err_kind_t kind) {
  switch (kind) {
    SP_CLI_ERR_KIND(SP_X_NAMED_ENUM_CASE_TO_STRING)
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_cli_t sp_cli_parse(sp_cli_desc_t desc) {
  sp_cli_t cli = sp_zero_s(sp_cli_t);
  cli.user_data = desc.user_data;

  sp_cli_parser_t parser = sp_zero_s(sp_cli_parser_t);
  parser.cli = &cli;
  parser.args = desc.args;
  parser.num_args = desc.num_args;

  sp_cli_scope_t scope = { .cmd = desc.root, .parent = SP_NULLPTR };
  if (sp_cli_parse_cmd(&parser, scope)) {
    cli.status = SP_CLI_ERR;
  }
  else if (parser.help) {
    cli.status = SP_CLI_HELP;
  }
  else {
    cli.status = SP_CLI_OK;
  }
  return cli;
}

sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli) {
  if (cli->status != SP_CLI_OK) {
    return cli->status;
  }
  if (!cli->cmd || !cli->cmd->handler) {
    cli->err = (sp_cli_err_t) {
      .kind = SP_CLI_ERR_NO_HANDLER,
      .name = cli->cmd ? sp_cli_str(cli->cmd->name) : sp_zero_s(sp_str_t),
    };
    return SP_CLI_ERR;
  }
  return cli->cmd->handler(cli);
}

void sp_cli_log_error(const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_cli_log_error_v(sp_cstr_as_str(fmt), args);
  va_end(args);
}

void sp_cli_log_error_v(sp_str_t fmt, va_list args) {
  sp_io_stream_writer_t io = sp_io_get_std_err();
  sp_fmt_io(&io.base, "{.red}: ", sp_fmt_cstr("error"));
  sp_fmt_io_v(&io.base, fmt, args);
  sp_fmt_io(&io.base, "\n");
}

void sp_cli_err_write(sp_io_writer_t* io, sp_cli_err_t* err) {
  switch (err->kind) {
    case SP_CLI_ERR_NONE: {
      break;
    }
    case SP_CLI_ERR_UNKNOWN_OPT: {
      sp_fmt_io(io, "unknown option: --{}", sp_fmt_str(err->name));
      break;
    }
    case SP_CLI_ERR_UNKNOWN_BRIEF: {
      sp_fmt_io(io, "unknown option: -{}", sp_fmt_str(err->name));
      break;
    }
    case SP_CLI_ERR_INVALID_VALUE: {
      sp_fmt_io(io, "invalid value for option --{}: {.quote}", sp_fmt_str(err->name), sp_fmt_str(err->value));
      break;
    }
    case SP_CLI_ERR_MISSING_VALUE: {
      sp_fmt_io(io, "missing value for option: --{}", sp_fmt_str(err->name));
      break;
    }
    case SP_CLI_ERR_MISSING_ARG: {
      sp_fmt_io(io, "missing required argument: {}", sp_fmt_str(err->name));
      break;
    }
    case SP_CLI_ERR_UNEXPECTED_ARG: {
      sp_fmt_io(io, "unexpected argument: {}", sp_fmt_str(err->value));
      break;
    }
    case SP_CLI_ERR_UNKNOWN_COMMAND: {
      sp_fmt_io(io, "unknown command: {}", sp_fmt_str(err->name));
      break;
    }
    case SP_CLI_ERR_NO_HANDLER: {
      sp_fmt_io(io, "no handler for command: {}", sp_fmt_str(err->name));
      break;
    }
  }
}

SP_PRIVATE void sp_cli_error_print(sp_cli_t* cli) {
  sp_io_stream_writer_t io = sp_io_get_std_err();
  sp_fmt_io(&io.base, "{.red}: ", sp_fmt_cstr("error"));
  sp_cli_err_write(&io.base, &cli->err);
  sp_fmt_io(&io.base, "\n");
}

sp_cli_result_t sp_cli_run(sp_cli_cmd_t* root, s32 num_args, const c8** args, void* user_data) {
  sp_cli_t cli = sp_cli_parse((sp_cli_desc_t) {
    .root = root,
    .args = num_args > 1 ? args + 1 : SP_NULLPTR,
    .num_args = num_args > 1 ? sp_cast(u32, num_args - 1) : 0,
    .user_data = user_data,
  });

  switch (cli.status) {
    case SP_CLI_OK: break;
    case SP_CLI_HELP: {
      sp_io_stream_writer_t out = sp_io_get_std_out();
      sp_cli_usage_write(&out.base, cli.cmd);
      return SP_CLI_HELP;
    }
    case SP_CLI_ERR: {
      sp_cli_error_print(&cli);
      sp_io_stream_writer_t err = sp_io_get_std_err();
      sp_cli_usage_write(&err.base, cli.cmd);
      return SP_CLI_ERR;
    }
    case SP_CLI_CONTINUE: {
      sp_unreachable_case();
    }
  }

  // Handlers print their own errors; the library only prints errors it
  // produced itself (e.g. dispatching a command with no handler).
  sp_cli_result_t result = sp_cli_dispatch(&cli);
  if (result == SP_CLI_ERR && cli.err.kind != SP_CLI_ERR_NONE) {
    sp_cli_error_print(&cli);
  }
  return result;
}

s32 sp_cli_main(sp_cli_cmd_t* root, s32 num_args, const c8** args, void* user_data) {
  switch (sp_cli_run(root, num_args, args, user_data)) {
    case SP_CLI_OK: return 0;
    case SP_CLI_HELP: return 0;
    case SP_CLI_CONTINUE: return 0;
    case SP_CLI_ERR: return 1;
  }
  SP_UNREACHABLE_RETURN(1);
}

void sp_cli_usage_write(sp_io_writer_t* io, sp_cli_cmd_t* cmd) {
  c8 buf [SP_CLI_MAX_LABEL];

  if (cmd->summary) {
    sp_fmt_io(io, "{}\n\n", sp_fmt_cstr(cmd->summary));
  }

  sp_fmt_io(io, "{.green}\n  {.cyan}", sp_fmt_cstr("usage"), sp_fmt_cstr(cmd->name));
  if (sp_cli_num_opts(cmd)) {
    sp_fmt_io(io, " [options]");
  }
  sp_carr_for(cmd->args, it) {
    if (!cmd->args[it].name) break;
    sp_fmt_io(io, " {}", sp_fmt_str(sp_cli_arg_label(buf, SP_CLI_MAX_LABEL, &cmd->args[it])));
  }
  if (sp_cli_has_commands(cmd)) {
    sp_fmt_io(io, " <command>");
  }
  sp_fmt_io(io, "\n");

  if (sp_cli_num_opts(cmd)) {
    u32 width = 0;
    sp_carr_for(cmd->opts, it) {
      if (!cmd->opts[it].name) break;
      width = sp_max(width, sp_cli_opt_label(buf, SP_CLI_MAX_LABEL, &cmd->opts[it]).len);
    }
    sp_fmt_io(io, "\n{.green}\n", sp_fmt_cstr("options"));
    sp_carr_for(cmd->opts, it) {
      if (!cmd->opts[it].name) break;
      sp_str_t label = sp_cli_opt_label(buf, SP_CLI_MAX_LABEL, &cmd->opts[it]);
      sp_fmt_io(io, "  {:<$ .yellow}  {}\n", sp_fmt_uint(width), sp_fmt_str(label), sp_fmt_str(sp_cli_str(cmd->opts[it].summary)));
    }
  }

  if (cmd->args[0].name) {
    u32 width = 0;
    sp_carr_for(cmd->args, it) {
      if (!cmd->args[it].name) break;
      width = sp_max(width, sp_cli_arg_label(buf, SP_CLI_MAX_LABEL, &cmd->args[it]).len);
    }
    sp_fmt_io(io, "\n{.green}\n", sp_fmt_cstr("arguments"));
    sp_carr_for(cmd->args, it) {
      if (!cmd->args[it].name) break;
      sp_str_t label = sp_cli_arg_label(buf, SP_CLI_MAX_LABEL, &cmd->args[it]);
      sp_fmt_io(io, "  {:<$ .yellow}  {}\n", sp_fmt_uint(width), sp_fmt_str(label), sp_fmt_str(sp_cli_str(cmd->args[it].summary)));
    }
  }

  if (sp_cli_has_commands(cmd)) {
    u32 width = 0;
    sp_carr_for(cmd->commands, it) {
      if (!cmd->commands[it]) break;
      width = sp_max(width, sp_cli_str(cmd->commands[it]->name).len);
    }
    sp_fmt_io(io, "\n{.green}\n", sp_fmt_cstr("commands"));
    sp_carr_for(cmd->commands, it) {
      sp_cli_cmd_t* sub = cmd->commands[it];
      if (!sub) break;
      sp_fmt_io(io, "  {:<$ .yellow}  {}\n", sp_fmt_uint(width), sp_fmt_str(sp_cli_str(sub->name)), sp_fmt_str(sp_cli_str(sub->summary)));
    }
  }
}

#endif // SP_CLI_IMPLEMENTATION
