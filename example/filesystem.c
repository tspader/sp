#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"

typedef struct {
  sp_str_t dir;
  bool walk;
} args_t;

typedef struct {
  args_t args;
  sp_mem_heap_t* heap;
  sp_mem_t mem;
} ctx_t;

void print_entry(sp_fs_entry_t entry, u32 depth) {
  sp_print("{:$}", sp_fmt_uint(depth * 2), sp_fmt_cstr(""));
  sp_print("{.gray}", sp_fmt_cstr("│ "));
  switch (entry.kind) {
    case SP_FS_KIND_DIR: sp_log("{.blue .bold}", sp_fmt_str(entry.name)); break;
    case SP_FS_KIND_FILE: sp_log("{}", sp_fmt_str(entry.name)); break;
    case SP_FS_KIND_SYMLINK: sp_log("{.cyan} -> {.gray}", sp_fmt_str(entry.name), sp_fmt_str(entry.path)); break;
    case SP_FS_KIND_NONE: break;
  }

}

void iterate(sp_path_t path) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  print_entry((sp_fs_entry_t) {
    .kind = SP_FS_KIND_DIR,
    .name = sp_fs_get_stem(path.sub)
  }, 0);

  sp_fs_it_t it = sp_fs_it_new_at(s.mem, path, 0);
  while (sp_fs_it_next(&it)) {
    if (it.yield == SP_FS_IT_LEAVE) {
      continue;
    }
    print_entry(it.entry, sp_da_size(it.stack));

    // switch (it.entry.kind) {
    //   case SP_FS_KIND_DIR: sp_fs_it_enter(&it);
    //   case SP_FS_KIND_FILE:
    //   case SP_FS_KIND_SYMLINK:
    //   case SP_FS_KIND_NONE: break;
    // }
  }
  sp_mem_end_scratch(s);
}

void walk(sp_path_t path) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_fs_it_t it = sp_fs_it_new_at(s.mem, path, 0);
  while (sp_fs_it_walk(&it)) {
    print_entry(it.entry, sp_da_size(it.stack) - 1);
  }
  sp_mem_end_scratch(s);
}

sp_cli_result_t command(sp_cli_t* cli) {
  ctx_t* ctx = sp_ptr_cast(ctx_t*, cli->user_data);
  args_t args = ctx->args;

  sp_path_t path = {
    .dir = sp_sys_get_root(0),
    .sub = sp_str_empty(args.dir) ?
      sp_str_lit(".") :
      args.dir
  };
  sp_log("{}", sp_fmt_int(path.dir));

  if (args.walk) {
    walk(path);
  } else {
    iterate(path);
  }
  return SP_CLI_OK;
}

s32 run(s32 num_args, const c8** args) {
  ctx_t ctx = sp_zero;
  ctx.heap = sp_mem_heap_new();
  ctx.mem = sp_mem_heap_as_allocator(ctx.heap);

  sp_cli_cmd_t root = {
    .name = "filesystem",
    .summary = "",
    .opts = {
      {
        .brief = 'w',
        .name = "walk",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "Walk (as opposed to flat traversal)",
        .ptr = &ctx.args.walk
      },
    },
    .args = {
      {
        .name = "dir",
        .summary = "The directory",
        .arity = SP_CLI_ARG_OPTIONAL,
        .kind = SP_CLI_OPT_STR,
        .ptr = &ctx.args.dir
      }
    },
    .handler = command,
  };

  return sp_cli_main((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &ctx
  });
}
SP_MAIN(run)
