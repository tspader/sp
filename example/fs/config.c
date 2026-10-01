#define SP_IMPLEMENTATION
#include "sp.h"

static sp_err_t load(sp_mem_t mem, sp_str_t path, sp_str_t* config) {
  if (!sp_fs_exists(path)) {
    sp_try(sp_fs_create_file_str(path, sp_str_lit("name = \"sp\"\n")));
  }
  return sp_io_read_file(mem, path, config);
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_str_t path = sp_str_lit("app.toml");
  sp_str_t config = sp_zero;
  sp_err_t err = load(s.mem, path, &config);
  if (err) {
    sp_log("{.red}: {}", sp_fmt_str(path), sp_fmt_str(sp_err_str(err)));
  }
  else {
    sp_print("{}", sp_fmt_str(config));
  }

  sp_mem_end_scratch(s);
  return err ? 1 : 0;
}
SP_MAIN(run)
