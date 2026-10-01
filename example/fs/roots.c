#define SP_IMPLEMENTATION
#include "sp.h"

static void print_root(s32 it) {
  c8 buf [SP_PATH_MAX];
  u64 len = 0;
  sp_err_t err = sp_sys_get_root_label(it, buf, sizeof(buf), &len);
  if (err) {
    sp_log("{:>4} {.red}", sp_fmt_int(sp_sys_get_root(it)), sp_fmt_str(sp_err_str(err)));
  }
  else {
    sp_log("{:>4} {.cyan}", sp_fmt_int(sp_sys_get_root(it)), sp_fmt_str(sp_str(buf, (u32)len)));
  }
}

static void print_path(sp_str_t path) {
  sp_path_t resolved = sp_path_from_str(path);
  sp_log(
    "{:>4} {} {.gray} {}",
    sp_fmt_int(resolved.dir),
    sp_fmt_str(resolved.sub),
    sp_fmt_cstr("<-"),
    sp_fmt_str(path)
  );
}

s32 run(s32 num_args, const c8** args) {
  for (s32 it = 0; sp_sys_get_root(it) != SP_SYS_INVALID_FD; it++) {
    print_root(it);
  }

  sp_for_range(it, 1, (u32)num_args) {
    print_path(sp_cstr_as_str(args[it]));
  }
  return 0;
}
SP_MAIN(run)
