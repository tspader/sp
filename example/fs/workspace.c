#define SP_IMPLEMENTATION
#include "sp.h"

static sp_err_t list(sp_sys_fd_t dir) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_fs_it_t it = sp_fs_it_new_at(s.mem, sp_path(dir, sp_str_lit(".")), 0);
  while (sp_fs_it_walk(&it)) {
    sp_log("{}", sp_fmt_str(it.entry.rel));
  }

  sp_err_t err = it.err;
  sp_fs_it_deinit(&it);
  sp_mem_end_scratch(s);
  return err;
}

static sp_err_t build(sp_sys_fd_t dir) {
  sp_try(sp_fs_create_dir_at(sp_path(dir, sp_str_lit("src/nested"))));
  sp_try(sp_fs_create_file_cstr_at(sp_path(dir, sp_str_lit("src/A")), "A"));
  sp_try(sp_fs_create_file_cstr_at(sp_path(dir, sp_str_lit("src/nested/B")), "B"));
  sp_try(sp_fs_copy_at(sp_path(dir, sp_str_lit("src")), sp_path(dir, sp_str_lit("dst")), SP_FS_ATOMIC_REPLACE));
  sp_try(sp_fs_remove_dir_at(sp_path(dir, sp_str_lit("src"))));
  return list(dir);
}

static sp_err_t work(sp_path_t path) {
  sp_try(sp_fs_create_dir_at(path));

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_err_t err = sp_fs_open_dir_at(path, &dir);
  if (!err) {
    err = build(dir);
    sp_sys_close(dir);
  }

  sp_err_t removed = sp_fs_remove_dir_at(path);
  return err ? err : removed;
}

s32 run(s32 num_args, const c8** args) {
  sp_str_t name = num_args > 1 ? sp_cstr_as_str(args[1]) : sp_str_lit("workspace");

  sp_err_t err = work(sp_path_from_str(name));
  if (err) {
    sp_log("{.red}: {}", sp_fmt_str(name), sp_fmt_str(sp_err_str(err)));
  }
  return err ? 1 : 0;
}
SP_MAIN(run)
