#define SP_IMPLEMENTATION
#include "sp.h"

static sp_err_t start(sp_mem_t mem, sp_path_t config, sp_path_t state) {
  sp_str_t content = sp_zero;
  sp_try(sp_io_read_file_at(mem, config, &content));
  sp_print("{}", sp_fmt_str(content));

  return sp_fs_write_atomic_at(state, sp_tm_epoch_to_iso8601(mem, sp_tm_now_epoch()));
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_path_t config = sp_path_resolve(sp_str_lit("/config/app.toml"));
  sp_path_t state = sp_path_resolve(sp_str_lit("/data/last_run"));
  sp_err_t err = start(s.mem, config, state);
  if (err) {
    sp_log("{.red}", sp_fmt_str(sp_err_str(err)));
  }

  sp_mem_end_scratch(s);
  return err ? 1 : 0;
}
SP_MAIN(run)
