#define SP_IMPLEMENTATION
#include "sp.h"

static bool is_hidden(sp_fs_entry_t entry) {
  return sp_str_starts_with(entry.name, sp_str_lit("."));
}

static void print_entry(sp_fs_entry_t entry, u32 depth) {
  sp_print("{:$}", sp_fmt_uint(depth * 2), sp_fmt_cstr(""));
  switch (entry.kind) {
    case SP_FS_KIND_DIR: {
      sp_log("{.blue}/", sp_fmt_str(entry.name));
      break;
    }
    case SP_FS_KIND_FILE: {
      sp_log("{}", sp_fmt_str(entry.name));
      break;
    }
    case SP_FS_KIND_SYMLINK: {
      sp_log("{.cyan}", sp_fmt_str(entry.name));
      break;
    }
    case SP_FS_KIND_NONE: {
      break;
    }
  }
}

s32 run(s32 num_args, const c8** args) {
  sp_str_t dir = num_args > 1 ? sp_cstr_as_str(args[1]) : sp_str_lit(".");

  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_fs_it_t it = sp_fs_it_new_at(s.mem, sp_path_resolve(dir), 0);
  sp_err_t err = it.err;
  u32 dirs = 0;
  while (!err && sp_fs_it_next(&it)) {
    switch (it.yield) {
      case SP_FS_IT_ENTRY: {
        if (is_hidden(it.entry)) break;

        print_entry(it.entry, sp_da_size(it.stack) - 1);
        if (it.entry.kind == SP_FS_KIND_DIR) err = sp_fs_it_enter(&it);
        break;
      }
      case SP_FS_IT_LEAVE: {
        dirs++;
        break;
      }
    }
  }
  if (!err) err = it.err;

  if (err) {
    sp_log("{.red}: {}", sp_fmt_str(dir), sp_fmt_str(sp_err_str(err)));
  }
  else {
    sp_log("{} {.gray}", sp_fmt_uint(dirs), sp_fmt_cstr("directories"));
  }

  sp_fs_it_deinit(&it);
  sp_mem_end_scratch(s);
  return err ? 1 : 0;
}
SP_MAIN(run)
