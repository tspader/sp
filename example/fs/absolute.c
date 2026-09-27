#define SP_IMPLEMENTATION
#include "sp.h"

static sp_err_t save(sp_mem_t mem, sp_str_t dir, sp_str_t file, sp_str_t* settings) {
  sp_try(sp_fs_create_dir_at(sp_path_resolve(dir)));
  sp_try(sp_fs_write_atomic_at(sp_path_resolve(file), sp_str_lit("theme = \"dark\"\n")));
  return sp_io_read_file_at(mem, sp_path_resolve(file), settings);
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_str_t config = sp_fs_get_config_path(s.mem);
  sp_str_t dir = sp_fs_join_path(s.mem, config, sp_str_lit("sp"));
  sp_str_t file = sp_fs_join_path(s.mem, dir, sp_str_lit("settings.toml"));
  sp_str_t settings = sp_zero;
  sp_err_t err = sp_str_empty(config) ? SP_ERR_SYS_NOT_FOUND : save(s.mem, dir, file, &settings);
  if (err) {
    sp_log("{.red}: {}", sp_fmt_str(file), sp_fmt_str(sp_err_str(err)));
  }
  else {
    sp_log("{.cyan}", sp_fmt_str(file));
    sp_print("{}", sp_fmt_str(settings));
  }

  sp_mem_end_scratch(s);
  return err ? 1 : 0;
}
SP_MAIN(run)
