#define SP_IMPLEMENTATION
#include "sp.h"

static sp_err_t save(sp_mem_t mem, sp_str_t file, sp_str_t* settings) {
  sp_try(sp_fs_create_dir(sp_fs_parent_path(file)));
  sp_try(sp_fs_write_atomic(file, sp_str_lit("theme = \"dark\"\n")));
  return sp_io_read_file(mem, file, settings);
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_str_t file = sp_fs_join_path(s.mem, sp_fs_get_config_path(s.mem), sp_str_lit("sp/settings.toml"));
  sp_str_t settings = sp_zero;
  sp_err_t err = save(s.mem, file, &settings);
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
