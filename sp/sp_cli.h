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
} sp_cli_arg_arity_t;

typedef enum {
  SP_CLI_OPT_CSTR,
  SP_CLI_OPT_STR,
  SP_CLI_OPT_BOOLEAN,
  SP_CLI_OPT_S8,
  SP_CLI_OPT_S16,
  SP_CLI_OPT_S32,
  SP_CLI_OPT_S64,
  SP_CLI_OPT_U8,
  SP_CLI_OPT_U16,
  SP_CLI_OPT_U32,
  SP_CLI_OPT_U64,
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
  SP_CLI_ERR_INVALID_ARG,
  SP_CLI_ERR_UNEXPECTED_ARG,
  SP_CLI_ERR_UNKNOWN_COMMAND,
  SP_CLI_ERR_MAX_DEPTH,
  SP_CLI_ERR_MISSING_ENV,
  SP_CLI_ERR_INVALID_ENV,
  SP_CLI_ERR_UNKNOWN_SHELL,
} sp_cli_err_kind_t;

typedef struct {
  sp_cli_err_kind_t kind;
  sp_str_t name;
  sp_str_t value;
} sp_cli_err_t;

typedef struct sp_cli sp_cli_t;
typedef struct sp_cli_cmd sp_cli_cmd_t;

SP_TYPEDEF_FN(sp_cli_result_t, sp_cli_handler_t, sp_cli_t*);

typedef enum {
  SP_CLI_SHELL_BASH,
  SP_CLI_SHELL_ZSH,
  SP_CLI_SHELL_FISH,
  SP_CLI_SHELL_POWERSHELL,
} sp_cli_shell_t;

typedef struct {
  sp_cli_shell_t shell;
  sp_str_t prefix;
  sp_str_t emit_prefix;
  sp_io_writer_t* out;
  void* user_data;
} sp_cli_complete_t;

SP_TYPEDEF_FN(void, sp_cli_completer_t, sp_cli_complete_t*);

typedef struct {
  const c8* name;
  sp_cli_arg_arity_t arity;
  sp_cli_value_kind_t kind;
  const c8* summary;
  void* ptr;
  sp_cli_completer_t complete;
} sp_cli_arg_t;

typedef struct {
  c8 brief;
  const c8* name;
  sp_cli_value_kind_t kind;
  const c8* summary;
  const c8* placeholder;
  void* ptr;
  sp_cli_completer_t complete;
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
  const c8** args; // argv-style: args[num_args] must be null
  s32 num_args;
  void* user_data;
  sp_cli_theme_t theme;
  const c8* completer;
} sp_cli_desc_t;

struct sp_cli {
  void* user_data;
  sp_cli_result_t status;
  sp_cli_err_t err;
  sp_cli_cmd_t* cmd;
  sp_cli_cmd_t* path [SP_CLI_MAX_DEPTH];
  u32 depth;
  const c8** rest; // null-terminated; never itself null
  sp_cli_theme_t theme;

  sp_da(sp_cli_opt_t) opts;
  sp_da(sp_cli_env_t) env;
  sp_da(sp_cli_arg_t) args;
  sp_da(sp_cli_cmd_t*) commands;
  struct {
    sp_arr(sp_cli_opt_t, SP_CLI_MAX_DEPTH * SP_CLI_MAX_OPTS) opts;
    sp_arr(sp_cli_env_t, SP_CLI_MAX_DEPTH * SP_CLI_MAX_ENV) envs;
    sp_arr(sp_cli_arg_t, SP_CLI_MAX_ARGS) args;
    sp_arr(sp_cli_cmd_t*, SP_CLI_MAX_COMMANDS) cmds;
  } buffers;
};

SP_API sp_str_t        sp_cli_arg_arity_to_str(sp_cli_arg_arity_t arity);
SP_API sp_str_t        sp_cli_opt_kind_to_str(sp_cli_value_kind_t kind);
SP_API sp_str_t        sp_cli_result_to_str(sp_cli_result_t result);
SP_API sp_str_t        sp_cli_err_kind_to_str(sp_cli_err_kind_t kind);
SP_API void            sp_cli_parse(sp_cli_desc_t desc, sp_cli_t* cli);
SP_API sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli);
SP_API sp_cli_result_t sp_cli_run(sp_cli_desc_t desc);
SP_API s32             sp_cli_main(sp_cli_desc_t desc);
SP_API void            sp_cli_write_help(sp_io_writer_t* io, sp_cli_t* cli);
SP_API void            sp_cli_err_print(sp_io_writer_t* io, sp_cli_err_t err);
SP_API void            sp_cli_candidate(sp_cli_complete_t* ctx, sp_str_t name, sp_str_t summary);
SP_API void            sp_cli_write_completions(sp_io_writer_t* io, sp_cli_desc_t desc, sp_cli_shell_t shell);
SP_API sp_cli_result_t sp_cli_set_error(sp_cli_t* cli, sp_str_t error);
SP_API sp_cli_result_t sp_cli_set_error_c(sp_cli_t* cli, const c8* error);

#endif // SP_CLI_H

#if defined(SP_IMPLEMENTATION) && !defined(SP_CLI_IMPLEMENTATION)
  #define SP_CLI_IMPLEMENTATION
#endif

#if defined(SP_CLI_IMPLEMENTATION) && !defined(SP_CLI_IMPLEMENTED)
#define SP_CLI_IMPLEMENTED

SP_PRIVATE bool sp_cli_token_is_escape(sp_str_t tok) {
  return sp_str_equal(tok, sp_str_lit("--"));
}

SP_PRIVATE bool sp_cli_token_is_long(sp_str_t tok) {
  return sp_str_starts_with(tok, sp_str_lit("--")) && !sp_cli_token_is_escape(tok);
}

SP_PRIVATE bool sp_cli_token_is_short(sp_str_t tok) {
  return tok.len > 1 && sp_str_at(tok, 0) == '-' && sp_str_at(tok, 1) != '-';
}

SP_PRIVATE bool sp_cli_token_is_flag(sp_str_t tok) {
  return tok.len > 1 && sp_str_at(tok, 0) == '-';
}

SP_PRIVATE sp_str_t sp_cli_token_to_long(sp_str_t tok, sp_str_t* value, bool* has_value) {
  sp_str_t body = sp_str_strip_left(tok, sp_str_lit("--"));
  s32 eq = sp_str_find_c8(body, '=');
  if (eq == SP_STR_NO_MATCH) {
    *value = sp_zero_s(sp_str_t);
    *has_value = false;
    return body;
  }
  *value = sp_str_suffix(body, body.len - (eq + 1));
  *has_value = true;
  return sp_str_prefix(body, eq);
}

typedef struct {
  sp_str_t cluster;
  u32 it;
} sp_cli_shorts_t;

SP_PRIVATE sp_cli_shorts_t sp_cli_token_to_short(sp_str_t tok) {
  return (sp_cli_shorts_t) { .cluster = sp_str_strip_left(tok, sp_str_lit("-")) };
}

SP_PRIVATE bool sp_cli_shorts_done(sp_cli_shorts_t* shorts) {
  return shorts->it >= shorts->cluster.len;
}

SP_PRIVATE c8 sp_cli_shorts_next_flag(sp_cli_shorts_t* shorts) {
  if (sp_cli_shorts_done(shorts)) return 0;
  return sp_str_at(shorts->cluster, shorts->it++);
}

SP_PRIVATE sp_str_t sp_cli_shorts_flag_str(sp_cli_shorts_t* shorts) {
  return sp_str_sub(shorts->cluster, sp_cast(s32, shorts->it) - 1, 1);
}

SP_PRIVATE sp_str_t sp_cli_shorts_next_value(sp_cli_shorts_t* shorts) {
  if (sp_cli_shorts_done(shorts)) return sp_zero_s(sp_str_t);
  sp_str_t value = sp_str_suffix(shorts->cluster, shorts->cluster.len - shorts->it);
  shorts->it = shorts->cluster.len;
  return value;
}

typedef enum {
  SP_CLI_PARSE_STRICT,
  SP_CLI_PARSE_COMPLETE,
} sp_cli_parse_mode_t;

typedef struct {
  sp_cli_t* cli;
  const c8** args;
  u32 num_args;
  u32 it;
  sp_cli_shorts_t shorts;
  sp_cli_parse_mode_t mode;
  bool raw;
  u32 arg;
} sp_cli_parser_t;

SP_PRIVATE sp_cli_result_t sp_cli_fail(sp_cli_t* cli, sp_cli_err_t err) {
  cli->err = err;
  return SP_CLI_ERR;
}

SP_PRIVATE sp_cli_result_t sp_cli_fail_named(sp_cli_t* cli, sp_cli_err_kind_t kind, const c8* name) {
  return sp_cli_fail(cli, (sp_cli_err_t) { .kind = kind, .name = sp_cstr_as_str(name) });
}

SP_PRIVATE sp_cli_result_t sp_cli_fail_valued(sp_cli_t* cli, sp_cli_err_kind_t kind, sp_str_t value) {
  return sp_cli_fail(cli, (sp_cli_err_t) { .kind = kind, .value = value });
}

SP_PRIVATE const c8* sp_cli_no_rest = SP_NULLPTR;

SP_PRIVATE void sp_cli_init(sp_cli_t* cli, sp_cli_desc_t desc) {
  *cli = sp_zero_s(sp_cli_t);
  cli->user_data = desc.user_data;
  cli->rest = &sp_cli_no_rest;
  cli->opts = sp_arr_init(&cli->buffers.opts);
  cli->env = sp_arr_init(&cli->buffers.envs);
  cli->args = sp_arr_init(&cli->buffers.args);
  cli->commands = sp_arr_init(&cli->buffers.cmds);
}

SP_PRIVATE void sp_cli_load_opt(sp_cli_t* cli, sp_cli_opt_t* opt) {
  sp_cli_opt_t copy = *opt;
  sp_da_for(cli->opts, it) {
    if (sp_cstr_equal(copy.name, cli->opts[it].name)) return;
    if (copy.brief && cli->opts[it].brief == copy.brief) copy.brief = 0;
  }
  sp_da_push(cli->opts, copy);
}

SP_PRIVATE void sp_cli_load_env(sp_cli_t* cli, sp_cli_env_t* var) {
  sp_da_for(cli->env, it) {
    if (sp_cstr_equal(var->name, cli->env[it].name)) return;
  }
  sp_da_push(cli->env, *var);
}

SP_PRIVATE void sp_cli_load_cmd(sp_cli_t* cli) {
  sp_da_clear(cli->opts);
  sp_da_clear(cli->env);
  sp_da_clear(cli->args);
  sp_da_clear(cli->commands);

  for (u32 i = cli->depth; i > 0; i--) {
    sp_cli_cmd_t* scope = cli->path[i - 1];
    sp_carr_for_until(scope->opts, it, scope->opts[it].name) {
      sp_cli_load_opt(cli, &scope->opts[it]);
    }
    sp_carr_for_until(scope->env, it, scope->env[it].name) {
      sp_cli_load_env(cli, &scope->env[it]);
    }
  }

  sp_cli_cmd_t* cmd = cli->cmd;
  sp_carr_for_until(cmd->args, it, cmd->args[it].name) {
    if (it) sp_assert(cmd->args[it - 1].arity != SP_CLI_ARG_REST);
    sp_da_push(cli->args, cmd->args[it]);
  }
  sp_carr_for_until(cmd->commands, it, cmd->commands[it]) {
    sp_da_push(cli->commands, cmd->commands[it]);
  }
}

SP_PRIVATE void sp_cli_push_cmd(sp_cli_t* cli, sp_cli_cmd_t* cmd) {
  sp_assert(cli->depth < SP_CLI_MAX_DEPTH);
  cli->cmd = cmd;
  cli->path[cli->depth++] = cmd;
  sp_cli_load_cmd(cli);
}

SP_PRIVATE bool sp_cli_done(sp_cli_parser_t* parser) {
  return parser->it >= parser->num_args;
}

SP_PRIVATE sp_str_t sp_cli_peek(sp_cli_parser_t* parser) {
  if (sp_cli_done(parser)) return sp_zero_s(sp_str_t);
  return sp_cstr_as_str(parser->args[parser->it]);
}

SP_PRIVATE sp_str_t sp_cli_next(sp_cli_parser_t* parser) {
  sp_str_t tok = sp_cli_peek(parser);
  parser->it++;
  return tok;
}

SP_PRIVATE sp_cli_opt_t* sp_cli_find_opt(sp_cli_t* cli, sp_str_t name) {
  sp_da_for(cli->opts, it) {
    if (sp_str_equal_cstr(name, cli->opts[it].name)) return &cli->opts[it];
  }
  return SP_NULLPTR;
}

SP_PRIVATE sp_cli_opt_t* sp_cli_find_brief(sp_cli_t* cli, c8 brief) {
  sp_da_for(cli->opts, it) {
    if (cli->opts[it].brief == brief) return &cli->opts[it];
  }
  return SP_NULLPTR;
}

SP_PRIVATE bool sp_cli_assign(sp_cli_value_kind_t kind, void* ptr, sp_str_t value) {
  switch (kind) {
    case SP_CLI_OPT_CSTR: {
      // Every value is either a whole element of desc.args or a NUL-terminated
      // tail of one (the text after '=' or after a brief cluster), so cstr
      // bindings borrow the args array directly instead of copying. A null value
      // passes through as a null pointer, signalling "not set".
      if (ptr) *sp_cast(const c8**, ptr) = value.data;
      break;
    }
    case SP_CLI_OPT_STR: {
      if (ptr) *sp_cast(sp_str_t*, ptr) = value;
      break;
    }
    case SP_CLI_OPT_BOOLEAN: {
      bool parsed = true;
      if (!sp_str_empty(value) && !sp_parse_bool_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(bool*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_S8: {
      s8 parsed = 0;
      if (!sp_parse_s8_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(s8*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_S16: {
      s16 parsed = 0;
      if (!sp_parse_s16_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(s16*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_S32: {
      s32 parsed = 0;
      if (!sp_parse_s32_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(s32*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_S64: {
      s64 parsed = 0;
      if (!sp_parse_s64_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(s64*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_U8: {
      u8 parsed = 0;
      if (!sp_parse_u8_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(u8*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_U16: {
      u16 parsed = 0;
      if (!sp_parse_u16_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(u16*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_U32: {
      u32 parsed = 0;
      if (!sp_parse_u32_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(u32*, ptr) = parsed;
      break;
    }
    case SP_CLI_OPT_U64: {
      u64 parsed = 0;
      if (!sp_parse_u64_ex(value, &parsed)) {
        return false;
      }
      if (ptr) *sp_cast(u64*, ptr) = parsed;
      break;
    }
  }
  return true;
}

SP_PRIVATE sp_cli_result_t sp_cli_assign_opt(sp_cli_parser_t* parser, sp_cli_opt_t* opt, sp_str_t value) {
  if (opt->kind != SP_CLI_OPT_BOOLEAN && sp_str_empty(value)) {
    return sp_cli_fail_named(parser->cli, SP_CLI_ERR_MISSING_VALUE, opt->name);
  }

  if (!sp_cli_assign(opt->kind, opt->ptr, value)) {
    return sp_cli_fail(parser->cli, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_INVALID_VALUE,
      .name = sp_cstr_as_str(opt->name),
      .value = value,
    });
  }
  return SP_CLI_OK;
}

SP_PRIVATE bool sp_cli_take_value(sp_cli_parser_t* parser, sp_str_t* value) {
  if (sp_cli_done(parser)) return false;
  if (sp_cli_token_is_flag(sp_cli_peek(parser))) return false;
  *value = sp_cli_next(parser);
  return true;
}

typedef enum {
  SP_CLI_STEP_ESCAPE,
  SP_CLI_STEP_HELP,
  SP_CLI_STEP_OPT,
  SP_CLI_STEP_OPT_PENDING,
  SP_CLI_STEP_COMMAND,
  SP_CLI_STEP_ARG,
  SP_CLI_STEP_ERR,
} sp_cli_step_kind_t;

typedef struct {
  sp_cli_step_kind_t kind;
  sp_cli_opt_t* opt;
  sp_str_t value;
  sp_cli_cmd_t* cmd;
  sp_cli_err_t err;
} sp_cli_step_t;

SP_PRIVATE sp_cli_step_t sp_cli_read_long(sp_cli_parser_t* parser) {
  sp_str_t value = sp_zero_s(sp_str_t);
  bool has_value = false;
  sp_str_t name = sp_cli_token_to_long(sp_cli_next(parser), &value, &has_value);

  sp_cli_opt_t* opt = sp_cli_find_opt(parser->cli, name);
  if (!opt) {
    if (sp_str_equal(name, sp_str_lit("help"))) {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_HELP };
    }
    return (sp_cli_step_t) {
      .kind = SP_CLI_STEP_ERR,
      .err = { .kind = SP_CLI_ERR_UNKNOWN_OPT, .name = name },
    };
  }

  // Completion words arrive pre-split by the shell: bash breaks --name=value
  // into three words at the '=' (it's in COMP_WORDBREAKS), so in complete mode
  // a bare '=' after a long option is the separator, not a value
  if (parser->mode == SP_CLI_PARSE_COMPLETE && !has_value && sp_str_equal_cstr(sp_cli_peek(parser), "=")) {
    sp_cli_next(parser);
    if (sp_cli_done(parser)) {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT_PENDING, .opt = opt };
    }
    return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT, .opt = opt, .value = sp_cli_next(parser) };
  }

  if (!has_value && opt->kind != SP_CLI_OPT_BOOLEAN) {
    if (!sp_cli_take_value(parser, &value)) {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT_PENDING, .opt = opt };
    }
  }
  return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT, .opt = opt, .value = value };
}

SP_PRIVATE sp_cli_step_t sp_cli_read_brief(sp_cli_parser_t* parser) {
  c8 brief = sp_cli_shorts_next_flag(&parser->shorts);

  sp_cli_opt_t* opt = sp_cli_find_brief(parser->cli, brief);
  if (!opt) {
    if (brief == 'h') {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_HELP };
    }
    return (sp_cli_step_t) {
      .kind = SP_CLI_STEP_ERR,
      .err = { .kind = SP_CLI_ERR_UNKNOWN_BRIEF, .name = sp_cli_shorts_flag_str(&parser->shorts) },
    };
  }

  if (opt->kind == SP_CLI_OPT_BOOLEAN) {
    return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT, .opt = opt };
  }

  sp_str_t value = sp_cli_shorts_next_value(&parser->shorts);
  if (sp_str_empty(value)) {
    if (!sp_cli_take_value(parser, &value)) {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT_PENDING, .opt = opt };
    }
  }
  return (sp_cli_step_t) { .kind = SP_CLI_STEP_OPT, .opt = opt, .value = value };
}

SP_PRIVATE sp_cli_step_t sp_cli_read_command(sp_cli_parser_t* parser) {
  sp_str_t tok = sp_cli_next(parser);

  sp_da_for(parser->cli->commands, it) {
    sp_cli_cmd_t* sub = parser->cli->commands[it];
    if (sp_str_equal_cstr(tok, sub->name)) {
      return (sp_cli_step_t) { .kind = SP_CLI_STEP_COMMAND, .cmd = sub };
    }
  }
  return (sp_cli_step_t) {
    .kind = SP_CLI_STEP_ERR,
    .err = { .kind = SP_CLI_ERR_UNKNOWN_COMMAND, .name = tok },
  };
}

SP_PRIVATE sp_cli_step_t sp_cli_read_arg(sp_cli_parser_t* parser) {
  return (sp_cli_step_t) { .kind = SP_CLI_STEP_ARG, .value = sp_cli_next(parser) };
}

SP_PRIVATE sp_cli_step_t sp_cli_read_step(sp_cli_parser_t* parser) {
  if (!sp_cli_shorts_done(&parser->shorts)) return sp_cli_read_brief(parser);
  if (parser->raw) return sp_cli_read_arg(parser);

  sp_str_t tok = sp_cli_peek(parser);
  if (sp_cli_token_is_escape(tok)) {
    sp_cli_next(parser);
    return (sp_cli_step_t) { .kind = SP_CLI_STEP_ESCAPE };
  }
  if (sp_cli_token_is_long(tok)) return sp_cli_read_long(parser);
  if (sp_cli_token_is_short(tok)) {
    parser->shorts = sp_cli_token_to_short(sp_cli_next(parser));
    return sp_cli_read_brief(parser);
  }
  if (sp_da_size(parser->cli->commands)) return sp_cli_read_command(parser);
  return sp_cli_read_arg(parser);
}

SP_PRIVATE sp_cli_result_t sp_cli_parse_strict(sp_cli_parser_t* parser) {
  sp_cli_t* cli = parser->cli;

  while (!sp_cli_done(parser) || !sp_cli_shorts_done(&parser->shorts)) {
    sp_cli_step_t step = sp_cli_read_step(parser);

    switch (step.kind) {
      case SP_CLI_STEP_ESCAPE: {
        parser->raw = true;
        break;
      }
      case SP_CLI_STEP_HELP: {
        return SP_CLI_HELP;
      }
      case SP_CLI_STEP_OPT: {
        if (sp_cli_assign_opt(parser, step.opt, step.value)) return SP_CLI_ERR;
        break;
      }
      case SP_CLI_STEP_OPT_PENDING: {
        return sp_cli_fail_named(cli, SP_CLI_ERR_MISSING_VALUE, step.opt->name);
      }
      case SP_CLI_STEP_COMMAND: {
        if (cli->depth == SP_CLI_MAX_DEPTH) {
          return sp_cli_fail_named(cli, SP_CLI_ERR_MAX_DEPTH, step.cmd->name);
        }
        sp_cli_push_cmd(cli, step.cmd);
        parser->arg = 0;
        break;
      }
      case SP_CLI_STEP_ARG: {
        if (parser->arg == sp_da_size(cli->args)) {
          return sp_cli_fail_valued(cli, SP_CLI_ERR_UNEXPECTED_ARG, step.value);
        }
        sp_cli_arg_t* arg = &cli->args[parser->arg];
        if (arg->arity == SP_CLI_ARG_REST) {
          cli->rest = parser->args + parser->it - 1;
          parser->it = parser->num_args;
          break;
        }
        if (!sp_cli_assign(arg->kind, arg->ptr, step.value)) {
          return sp_cli_fail(cli, (sp_cli_err_t) {
            .kind = SP_CLI_ERR_INVALID_ARG,
            .name = sp_cstr_as_str(arg->name),
            .value = step.value,
          });
        }
        parser->arg++;
        break;
      }
      case SP_CLI_STEP_ERR: {
        return sp_cli_fail(cli, step.err);
      }
    }
  }
  return SP_CLI_OK;
}

SP_PRIVATE sp_cli_opt_t* sp_cli_parse_complete(sp_cli_parser_t* parser) {
  parser->mode = SP_CLI_PARSE_COMPLETE;

  sp_cli_t* cli = parser->cli;
  sp_cli_opt_t* pending = SP_NULLPTR;

  while (!sp_cli_done(parser) || !sp_cli_shorts_done(&parser->shorts)) {
    sp_cli_step_t step = sp_cli_read_step(parser);
    pending = step.kind == SP_CLI_STEP_OPT_PENDING ? step.opt : SP_NULLPTR;

    switch (step.kind) {
      case SP_CLI_STEP_ESCAPE: {
        parser->raw = true;
        break;
      }
      case SP_CLI_STEP_COMMAND: {
        if (cli->depth < SP_CLI_MAX_DEPTH) {
          sp_cli_push_cmd(cli, step.cmd);
          parser->arg = 0;
        }
        break;
      }
      case SP_CLI_STEP_ARG: {
        if (parser->arg == sp_da_size(cli->args)) break;
        if (cli->args[parser->arg].arity == SP_CLI_ARG_REST) {
          parser->it = parser->num_args;
          break;
        }
        parser->arg++;
        break;
      }
      default: {
        break;
      }
    }
  }
  return pending;
}

SP_PRIVATE sp_cli_result_t sp_cli_check_args(sp_cli_parser_t* parser) {
  sp_cli_t* cli = parser->cli;
  for (u64 it = parser->arg; it < sp_da_size(cli->args); it++) {
    if (cli->args[it].arity == SP_CLI_ARG_REQUIRED) {
      return sp_cli_fail_named(cli, SP_CLI_ERR_MISSING_ARG, cli->args[it].name);
    }
  }
  return SP_CLI_OK;
}

SP_PRIVATE sp_str_t sp_cli_opt_label(c8* buf, u32 len, sp_cli_opt_t* opt) {
  sp_io_mem_writer_t label = sp_zero;
  sp_io_mem_writer_from_buffer(&label, buf, len);

  if (opt->brief) {
    sp_fmt_io(&label.base, "-{}, ", sp_fmt_char(opt->brief));
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
  switch (arg->arity) {
    case SP_CLI_ARG_REQUIRED: { return sp_fmt_buf(buf, len, "{}", sp_fmt_cstr(arg->name)).value; }
    case SP_CLI_ARG_OPTIONAL: { return sp_fmt_buf(buf, len, "[{}]", sp_fmt_cstr(arg->name)).value; }
    case SP_CLI_ARG_REST:     { return sp_fmt_buf(buf, len, "[{}...]", sp_fmt_cstr(arg->name)).value; }
  }
  SP_UNREACHABLE_RETURN(sp_zero_s(sp_str_t));
}

sp_str_t sp_cli_arg_arity_to_str(sp_cli_arg_arity_t arity) {
  switch (arity) {
    case SP_CLI_ARG_REQUIRED: { return sp_str_lit("required"); }
    case SP_CLI_ARG_OPTIONAL: { return sp_str_lit("optional"); }
    case SP_CLI_ARG_REST:     { return sp_str_lit("rest"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_cli_opt_kind_to_str(sp_cli_value_kind_t kind) {
  switch (kind) {
    case SP_CLI_OPT_CSTR:    { return sp_str_lit("cstr"); }
    case SP_CLI_OPT_STR:     { return sp_str_lit("str"); }
    case SP_CLI_OPT_BOOLEAN: { return sp_str_lit("boolean"); }
    case SP_CLI_OPT_S8:      { return sp_str_lit("s8"); }
    case SP_CLI_OPT_S16:     { return sp_str_lit("s16"); }
    case SP_CLI_OPT_S32:     { return sp_str_lit("s32"); }
    case SP_CLI_OPT_S64:     { return sp_str_lit("s64"); }
    case SP_CLI_OPT_U8:      { return sp_str_lit("u8"); }
    case SP_CLI_OPT_U16:     { return sp_str_lit("u16"); }
    case SP_CLI_OPT_U32:     { return sp_str_lit("u32"); }
    case SP_CLI_OPT_U64:     { return sp_str_lit("u64"); }
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
    case SP_CLI_ERR_INVALID_ARG:     { return sp_str_lit("invalid_arg"); }
    case SP_CLI_ERR_UNEXPECTED_ARG:  { return sp_str_lit("unexpected_arg"); }
    case SP_CLI_ERR_UNKNOWN_COMMAND: { return sp_str_lit("unknown_command"); }
    case SP_CLI_ERR_MAX_DEPTH:       { return sp_str_lit("max_depth"); }
    case SP_CLI_ERR_MISSING_ENV:     { return sp_str_lit("missing_env"); }
    case SP_CLI_ERR_INVALID_ENV:     { return sp_str_lit("invalid_env"); }
    case SP_CLI_ERR_UNKNOWN_SHELL:   { return sp_str_lit("unknown_shell"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

SP_PRIVATE sp_cli_theme_t sp_cli_theme_resolve(sp_cli_theme_t theme);

SP_PRIVATE sp_cli_result_t sp_cli_resolve_env(sp_cli_t* cli) {
  sp_da_for(cli->env, it) {
    sp_cli_env_t* var = &cli->env[it];

    sp_str_t value = sp_os_env_get(sp_cstr_as_str(var->name));
    if (sp_str_empty(value)) {
      if (var->required) {
        return sp_cli_fail_named(cli, SP_CLI_ERR_MISSING_ENV, var->name);
      }
      continue;
    }

    if (!sp_cli_assign(var->kind, var->ptr, value)) {
      return sp_cli_fail(cli, (sp_cli_err_t) {
        .kind = SP_CLI_ERR_INVALID_ENV,
        .name = sp_cstr_as_str(var->name),
        .value = value,
      });
    }
  }
  return SP_CLI_OK;
}

void sp_cli_parse(sp_cli_desc_t desc, sp_cli_t* cli) {
  sp_cli_init(cli, desc);
  cli->theme = sp_cli_theme_resolve(desc.theme);

  sp_cli_parser_t parser = sp_zero_s(sp_cli_parser_t);
  parser.cli = cli;
  parser.args = desc.num_args > 1 ? desc.args + 1 : SP_NULLPTR;
  parser.num_args = desc.num_args > 1 ? sp_cast(u32, desc.num_args - 1) : 0;
  sp_cli_push_cmd(cli, desc.root);

  cli->status = sp_cli_parse_strict(&parser);
  if (cli->status != SP_CLI_OK) return;

  cli->status = sp_cli_check_args(&parser);
  if (cli->status != SP_CLI_OK) return;

  if (!cli->cmd->handler) {
    cli->status = SP_CLI_HELP;
    return;
  }

  // Only resolve the environment once everything else parsed; otherwise,
  // the error produced by a missing required env var swallows genuine
  // requests for help
  cli->status = sp_cli_resolve_env(cli);
}

sp_cli_result_t sp_cli_dispatch(sp_cli_t* cli) {
  if (cli->status) return cli->status;
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
    case SP_CLI_ERR_INVALID_ARG: {
      sp_fmt_io(io, "invalid value for argument {}: {.quote}", sp_fmt_str(err.name), sp_fmt_str(err.value));
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
    case SP_CLI_ERR_MAX_DEPTH: {
      sp_fmt_io(io, "command {} exceeds SP_CLI_MAX_DEPTH ({})", sp_fmt_str(err.name), sp_fmt_uint(SP_CLI_MAX_DEPTH));
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
    case SP_CLI_ERR_UNKNOWN_SHELL: {
      sp_fmt_io(io, "unknown completion shell: {.quote}", sp_fmt_str(err.name));
      break;
    }
  }
}

SP_PRIVATE void sp_cli_write_error(sp_io_writer_t* io, sp_cli_err_t err, sp_cli_theme_t theme) {
  sp_fmt_io(io, "{.$ .$}: ", SP_CLI_THEME_ARGS(theme.error), sp_fmt_cstr("error"));
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

SP_PRIVATE void sp_cli_write_label_hint(sp_io_writer_t* io, sp_cli_theme_entry_t label_entry, sp_cli_theme_entry_t hint_entry, sp_str_t label, sp_str_t summary, u32 width, bool required) {
  sp_cli_theme_entry_t hint_style = required ? sp_zero_s(sp_cli_theme_entry_t) : hint_entry;
  const c8* hint = required ? "required" : "optional";
  sp_fmt_io(io, "  {:<$ .$ .$} {.$ .$} {}",
    sp_fmt_uint(width), SP_CLI_THEME_ARGS(label_entry), sp_fmt_str(label),
    SP_CLI_THEME_ARGS(hint_style), sp_fmt_cstr(hint),
    sp_fmt_str(summary));
  sp_fmt_io(io, "\n");
}

SP_PRIVATE void sp_cli_write_synopsis(sp_io_writer_t* io, sp_cli_t* cli) {
  sp_cli_theme_t theme = cli->theme;

  sp_cli_write_heading(io, theme.heading, "usage");

  sp_fmt_io(io, "  ");
  sp_for(it, cli->depth) {
    if (it) sp_fmt_io(io, " ");
    sp_fmt_io(io, "{.$ .$}", SP_CLI_THEME_ARGS(theme.command), sp_fmt_cstr(cli->path[it]->name));
  }

  if (sp_da_size(cli->opts)) sp_fmt_io(io, " [OPTIONS]");

  sp_da_for(cli->args, it) {
    c8 buffer [SP_CLI_MAX_LABEL];
    sp_str_t label = sp_cli_arg_label(buffer, SP_CLI_MAX_LABEL, &cli->args[it]);
    sp_fmt_io(io, " {}", sp_fmt_str(label));
  }

  if (sp_da_size(cli->commands)) sp_fmt_io(io, " <COMMAND>");
  sp_fmt_io(io, "\n");
}

void sp_cli_write_help(sp_io_writer_t* io, sp_cli_t* cli) {
  sp_cli_theme_t theme = cli->theme;

  if (cli->cmd->summary) {
    sp_fmt_io(io, "{}\n", sp_fmt_cstr(cli->cmd->summary));
  }

  sp_cli_write_synopsis(io, cli);

  if (sp_da_size(cli->commands)) {
    u32 width = 0;
    sp_da_for(cli->commands, it) {
      width = sp_max(width, sp_cstr_as_str(cli->commands[it]->name).len);
    }
    sp_cli_write_heading(io, theme.heading, "commands");
    sp_da_for(cli->commands, it) {
      sp_cli_cmd_t* sub = cli->commands[it];
      sp_cli_write_label(io, theme.label, sp_cstr_as_str(sub->name), sp_cstr_as_str(sub->summary), width);
    }
  }

  if (sp_da_size(cli->opts)) {
    c8 buffers [SP_CLI_MAX_DEPTH * SP_CLI_MAX_OPTS][SP_CLI_MAX_LABEL];
    sp_str_t labels [SP_CLI_MAX_DEPTH * SP_CLI_MAX_OPTS];
    u32 width = 0;
    sp_da_for(cli->opts, it) {
      labels[it] = sp_cli_opt_label(buffers[it], SP_CLI_MAX_LABEL, &cli->opts[it]);
      width = sp_max(width, labels[it].len);
    }
    sp_cli_write_heading(io, theme.heading, "options");
    sp_da_for(cli->opts, it) {
      sp_cli_write_label(io, theme.label, labels[it], sp_cstr_as_str(cli->opts[it].summary), width);
    }
  }

  if (sp_da_size(cli->args)) {
    c8 buffers [SP_CLI_MAX_ARGS][SP_CLI_MAX_LABEL];
    sp_str_t labels [SP_CLI_MAX_ARGS];
    u32 width = 0;
    sp_da_for(cli->args, it) {
      labels[it] = sp_cli_arg_label(buffers[it], SP_CLI_MAX_LABEL, &cli->args[it]);
      width = sp_max(width, labels[it].len);
    }
    sp_cli_write_heading(io, theme.heading, "arguments");
    sp_da_for(cli->args, it) {
      sp_cli_arg_t* arg = &cli->args[it];
      sp_cli_write_label_hint(io, theme.label, theme.hint, labels[it], sp_cstr_as_str(arg->summary), width, arg->arity == SP_CLI_ARG_REQUIRED);
    }
  }

  if (sp_da_size(cli->env)) {
    u32 width = 0;
    sp_da_for(cli->env, it) {
      width = sp_max(width, sp_cstr_as_str(cli->env[it].name).len);
    }
    sp_cli_write_heading(io, theme.heading, "environment");
    sp_da_for(cli->env, it) {
      sp_cli_env_t* var = &cli->env[it];
      sp_cli_write_label_hint(io, theme.label, theme.hint, sp_cstr_as_str(var->name), sp_cstr_as_str(var->summary), width, var->required);
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

SP_PRIVATE void sp_cli_write_zsh_escaped(sp_io_writer_t* io, sp_str_t str, bool escape_colon) {
  sp_str_for(str, it) {
    c8 c = sp_str_at(str, it);
    if (c == '\\' || (escape_colon && c == ':')) sp_io_write_c8(io, '\\');
    sp_io_write_c8(io, c);
  }
}

SP_PRIVATE bool sp_cli_bash_is_safe(c8 c) {
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  if (c >= '0' && c <= '9') return true;
  return sp_str_find_c8(sp_str_lit("-_+=/.:,@%^"), c) != SP_STR_NO_MATCH;
}

SP_PRIVATE void sp_cli_write_bash_escaped(sp_io_writer_t* io, sp_str_t str) {
  sp_str_for(str, it) {
    c8 c = sp_str_at(str, it);
    if (!sp_cli_bash_is_safe(c)) sp_io_write_c8(io, '\\');
    sp_io_write_c8(io, c);
  }
}

void sp_cli_candidate(sp_cli_complete_t* ctx, sp_str_t name, sp_str_t summary) {
  if (!sp_str_starts_with(name, ctx->prefix)) return;

  switch (ctx->shell) {
    case SP_CLI_SHELL_BASH: {
      sp_cli_write_bash_escaped(ctx->out, ctx->emit_prefix);
      sp_cli_write_bash_escaped(ctx->out, name);
      sp_io_write_c8(ctx->out, '\n');
      break;
    }
    case SP_CLI_SHELL_ZSH: {
      sp_cli_write_zsh_escaped(ctx->out, ctx->emit_prefix, true);
      sp_cli_write_zsh_escaped(ctx->out, name, true);
      if (!sp_str_empty(summary)) {
        sp_io_write_c8(ctx->out, ':');
        sp_cli_write_zsh_escaped(ctx->out, summary, false);
      }
      sp_io_write_c8(ctx->out, '\n');
      break;
    }
    case SP_CLI_SHELL_FISH:
    case SP_CLI_SHELL_POWERSHELL: {
      sp_fmt_io(ctx->out, "{}{}", sp_fmt_str(ctx->emit_prefix), sp_fmt_str(name));
      if (!sp_str_empty(summary)) sp_fmt_io(ctx->out, "\t{}", sp_fmt_str(summary));
      sp_io_write_c8(ctx->out, '\n');
      break;
    }
  }
}

#ifndef SP_CLI_COMPLETE_EMPTY
  #define SP_CLI_COMPLETE_EMPTY "__sp_complete_empty__"
#endif

SP_PRIVATE void sp_cli_complete_arg(sp_cli_complete_t* ctx, sp_cli_parser_t* parser) {
  sp_cli_t* cli = parser->cli;
  if (parser->arg == sp_da_size(cli->args)) return;
  sp_cli_arg_t* arg = &cli->args[parser->arg];
  if (arg->complete) arg->complete(ctx);
}

SP_PRIVATE void sp_cli_complete_value(sp_cli_complete_t* ctx, sp_cli_opt_t* opt, sp_str_t cursor, sp_str_t value) {
  if (!opt->complete) return;
  ctx->emit_prefix = sp_str_prefix(cursor, sp_cast(s32, cursor.len - value.len));
  ctx->prefix = value;
  opt->complete(ctx);
}

SP_PRIVATE sp_cli_opt_t* sp_cli_find_cluster_opt(sp_cli_t* cli, sp_str_t cursor, sp_str_t* value) {
  sp_cli_shorts_t shorts = sp_cli_token_to_short(cursor);
  c8 brief;
  while ((brief = sp_cli_shorts_next_flag(&shorts))) {
    sp_cli_opt_t* opt = sp_cli_find_brief(cli, brief);
    if (!opt) return SP_NULLPTR;
    if (opt->kind == SP_CLI_OPT_BOOLEAN) continue;
    *value = sp_cli_shorts_next_value(&shorts);
    return opt;
  }
  return SP_NULLPTR;
}

SP_PRIVATE void sp_cli_complete(sp_io_writer_t* out, sp_cli_desc_t desc, sp_cli_shell_t shell, const c8** words, u32 num_words) {
  sp_str_t prefix = sp_cstr_as_str(words[num_words - 1]);

  if (shell == SP_CLI_SHELL_POWERSHELL && sp_str_equal_cstr(prefix, SP_CLI_COMPLETE_EMPTY)) {
    prefix = sp_zero_s(sp_str_t);
  }

  sp_cli_t cli;
  sp_cli_init(&cli, desc);

  sp_cli_parser_t parser = sp_zero_s(sp_cli_parser_t);
  parser.cli = &cli;
  parser.args = num_words > 1 ? words + 1 : SP_NULLPTR;
  parser.num_args = num_words > 1 ? num_words - 2 : 0;
  sp_cli_push_cmd(&cli, desc.root);

  sp_cli_opt_t* pending = sp_cli_parse_complete(&parser);

  // When the cursor sits right after --name=, bash reports the '=' itself as
  // the cursor word; the readline word being completed is empty
  if (shell == SP_CLI_SHELL_BASH && pending && sp_str_equal_cstr(prefix, "=")) {
    prefix = sp_zero_s(sp_str_t);
  }

  sp_cli_complete_t ctx = {
    .shell = shell,
    .prefix = prefix,
    .out = out,
    .user_data = desc.user_data,
  };

  if (pending) {
    sp_cli_complete_value(&ctx, pending, prefix, prefix);
    return;
  }

  if (parser.raw) {
    sp_cli_complete_arg(&ctx, &parser);
    return;
  }

  if (sp_cli_token_is_long(prefix)) {
    sp_str_t value = sp_zero_s(sp_str_t);
    bool has_value = false;
    sp_str_t name = sp_cli_token_to_long(prefix, &value, &has_value);
    if (has_value) {
      sp_cli_opt_t* opt = sp_cli_find_opt(&cli, name);
      if (opt) sp_cli_complete_value(&ctx, opt, prefix, value);
      return;
    }
  }

  if (sp_cli_token_is_short(prefix)) {
    sp_str_t value = sp_zero_s(sp_str_t);
    sp_cli_opt_t* opt = sp_cli_find_cluster_opt(&cli, prefix, &value);
    if (opt) {
      sp_cli_complete_value(&ctx, opt, prefix, value);
      return;
    }
  }

  if (sp_str_starts_with(prefix, sp_str_lit("-"))) {
    sp_da_for(cli.opts, it) {
      sp_cli_opt_t* opt = &cli.opts[it];
      c8 buffer [SP_CLI_MAX_LABEL];
      sp_str_t label = sp_fmt_buf(buffer, SP_CLI_MAX_LABEL, "--{}", sp_fmt_cstr(opt->name)).value;
      sp_cli_candidate(&ctx, label, sp_cstr_as_str(opt->summary));
    }
    return;
  }

  if (sp_da_size(cli.commands)) {
    sp_da_for(cli.commands, it) {
      sp_cli_cmd_t* sub = cli.commands[it];
      sp_cli_candidate(&ctx, sp_cstr_as_str(sub->name), sp_cstr_as_str(sub->summary));
    }
    return;
  }

  sp_cli_complete_arg(&ctx, &parser);
}

SP_PRIVATE sp_str_t sp_cli_completer(sp_cli_desc_t desc) {
  return sp_cstr_as_str(desc.completer ? desc.completer : "COMPLETE");
}

SP_PRIVATE bool sp_cli_path_has_sep(sp_str_t path) {
  sp_str_for(path, it) {
    if (sp_fs_is_sep(sp_str_at(path, it))) return true;
  }
  return false;
}

SP_PRIVATE sp_str_t sp_cli_completer_path(sp_cli_desc_t desc, c8* buffer, u32 len) {
  sp_str_t arg0 = desc.num_args > 0 ? sp_cstr_as_str(desc.args[0]) : sp_cstr_as_str(desc.root->name);
  if (!sp_cli_path_has_sep(arg0)) return arg0;
  if (sp_fs_is_absolute(arg0)) return arg0;

  c8 cwd [SP_PATH_MAX];
  s64 cwd_len = sp_sys_get_cwd_path(cwd, sizeof(cwd));
  if (cwd_len <= 0) return arg0;
  if (sp_cast(u32, cwd_len) + 1 + arg0.len > len) return arg0;

  sp_io_mem_writer_t path = sp_zero;
  sp_io_mem_writer_from_buffer(&path, buffer, len);
  sp_fmt_io(&path.base, "{}/{}", sp_fmt_str(sp_str(cwd, sp_cast(u32, cwd_len))), sp_fmt_str(arg0));
  return sp_io_mem_writer_as_str(&path);
}

void sp_cli_write_completions(sp_io_writer_t* io, sp_cli_desc_t desc, sp_cli_shell_t shell) {
  c8 buffer [SP_PATH_MAX];
  sp_str_t bin = sp_cstr_as_str(desc.root->name);
  sp_str_t var = sp_cli_completer(desc);
  sp_str_t completer = sp_cli_completer_path(desc, buffer, sizeof(buffer));

  switch (shell) {
    case SP_CLI_SHELL_BASH: {
      sp_fmt_io(io,
        "_{}_complete() {{\n"
        "  local IFS=$'\\n'\n"
        "  COMPREPLY=($({}=bash \"{}\" -- \"${{COMP_WORDS[@]:0:COMP_CWORD+1}}\"))\n"
        "  [ $? -ne 0 ] && unset COMPREPLY\n"
        "}}\n"
        "complete -o default -F _{}_complete {}\n",
        sp_fmt_str(bin), sp_fmt_str(var), sp_fmt_str(completer),
        sp_fmt_str(bin), sp_fmt_str(bin));
      break;
    }
    case SP_CLI_SHELL_ZSH: {
      sp_fmt_io(io,
        "#compdef {}\n"
        "_{}() {{\n"
        "  local -a lines\n"
        "  lines=(${{(f)\"$({}=zsh \"{}\" -- \"${{words[@]:0:$CURRENT}}\")\"}})\n"
        "  _describe '{}' lines\n"
        "}}\n"
        "compdef _{} {}\n",
        sp_fmt_str(bin), sp_fmt_str(bin), sp_fmt_str(var), sp_fmt_str(completer),
        sp_fmt_str(bin), sp_fmt_str(bin), sp_fmt_str(bin));
      break;
    }
    case SP_CLI_SHELL_FISH: {
      sp_fmt_io(io,
        "function __{}_complete\n"
        "  set -l cur (commandline -ct)\n"
        "  {}=fish \"{}\" -- (commandline -opc) \"$cur\"\n"
        "end\n"
        "complete -c {} -f -a '(__{}_complete)'\n",
        sp_fmt_str(bin), sp_fmt_str(var), sp_fmt_str(completer),
        sp_fmt_str(bin), sp_fmt_str(bin));
      break;
    }
    case SP_CLI_SHELL_POWERSHELL: {
      sp_fmt_io(io,
        "Register-ArgumentCompleter -Native -CommandName {} -ScriptBlock {{\n"
        "    param($wordToComplete, $commandAst, $cursorPosition)\n"
        "    $prev = $env:{}\n"
        "    $env:{} = 'powershell'\n"
        "    $spElements = @($commandAst.CommandElements |"
        " Where-Object {{ $_.Extent.EndOffset -le $cursorPosition }})\n"
        "    $spWords = @($spElements | ForEach-Object {{ $_.Extent.Text }})\n"
        "    $spEnd = 0\n"
        "    if ($spElements.Count -gt 0) {{ $spEnd = $spElements[-1].Extent.EndOffset }}\n"
        "    if ($cursorPosition -gt $spEnd) {{ $spWords += '{}' }}\n"
        "    $spResults = & \"{}\" -- @spWords\n"
        "    if ($null -eq $prev) {{ Remove-Item Env:\\{} }} else {{ $env:{} = $prev }}\n"
        "    $spResults | ForEach-Object {{\n"
        "        $spParts = $_.Split(\"`t\")\n"
        "        $spValue = $spParts[0]\n"
        "        if ($spParts.Length -ge 2) {{ $spHelp = $spParts[1] }}"
        " else {{ $spHelp = $spParts[0] }}\n"
        "        $spInsert = $spValue\n"
        "        if ($spValue -match '\\s|[\"'']')"
        " {{ $spInsert = \"'\" + ($spValue -replace \"'\", \"''\") + \"'\" }}\n"
        "        [System.Management.Automation.CompletionResult]::new($spInsert, $spValue,"
        " 'ParameterValue', $spHelp)\n"
        "    }}\n"
        "}}\n",
        sp_fmt_str(bin), sp_fmt_str(var), sp_fmt_str(var), sp_fmt_cstr(SP_CLI_COMPLETE_EMPTY),
        sp_fmt_str(completer), sp_fmt_str(var), sp_fmt_str(var));
      break;
    }
  }
}

SP_PRIVATE bool sp_cli_shell_from_str(sp_str_t name, sp_cli_shell_t* shell) {
  if (sp_str_equal_cstr(name, "bash")) { *shell = SP_CLI_SHELL_BASH; return true; }
  if (sp_str_equal_cstr(name, "zsh"))  { *shell = SP_CLI_SHELL_ZSH;  return true; }
  if (sp_str_equal_cstr(name, "fish")) { *shell = SP_CLI_SHELL_FISH; return true; }
  if (sp_str_equal_cstr(name, "powershell")) { *shell = SP_CLI_SHELL_POWERSHELL; return true; }
  if (sp_str_equal_cstr(name, "pwsh"))       { *shell = SP_CLI_SHELL_POWERSHELL; return true; }
  return false;
}

SP_PRIVATE sp_cli_result_t sp_cli_complete_request(sp_io_writer_t* out, sp_io_writer_t* err, sp_cli_desc_t desc, sp_str_t request) {
  sp_cli_shell_t shell;
  if (!sp_cli_shell_from_str(sp_fs_get_stem(request), &shell)) {
    sp_cli_write_error(err, (sp_cli_err_t) {
      .kind = SP_CLI_ERR_UNKNOWN_SHELL,
      .name = request,
    }, sp_cli_theme_resolve(desc.theme));
    return SP_CLI_ERR;
  }

  u32 words = sp_cast(u32, desc.num_args);
  sp_for(it, desc.num_args) {
    if (sp_cstr_equal(desc.args[it], "--")) {
      words = sp_cast(u32, it) + 1;
      break;
    }
  }

  if (words >= sp_cast(u32, desc.num_args)) {
    sp_cli_write_completions(out, desc, shell);
  }
  else {
    sp_cli_complete(out, desc, shell, desc.args + words, sp_cast(u32, desc.num_args) - words);
  }
  return SP_CLI_OK;
}

sp_cli_result_t sp_cli_run(sp_cli_desc_t desc) {
  sp_io_writer_t* out = sp_io_get_std_out();
  sp_io_writer_t* err = sp_io_get_std_err();

  sp_str_t request = sp_os_env_get(sp_cli_completer(desc));
  if (!sp_str_empty(request) && !sp_str_equal_cstr(request, "0")) {
    return sp_cli_complete_request(out, err, desc, request);
  }

  sp_cli_t cli;
  sp_cli_parse(desc, &cli);
  if (!cli.status) {
    cli.status = sp_cli_dispatch(&cli);
  }

  switch (cli.status) {
    case SP_CLI_OK:
    case SP_CLI_CONTINUE: {
      break;
    }
    case SP_CLI_HELP: {
      sp_cli_write_help(out, &cli);
      break;
    }
    case SP_CLI_ERR: {
      sp_cli_write_error(err, cli.err, cli.theme);
      sp_cli_write_synopsis(err, &cli);
      sp_fmt_io(err, "\n");
      sp_fmt_io(err, "Use {.$ .$} for full usage", SP_CLI_THEME_ARGS(cli.theme.label), sp_fmt_cstr("--help"));
      sp_fmt_io(err, "\n");
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
