#include "fs.h"

#if defined(SP_POSIX)

typedef struct {
  const c8* name;
  sp_fs_write_mode_t mode;
} test_t;

static const test_t tests [] = {
  { .name = "truncate" },
  { .name = "append", .mode = SP_FS_WRITE_APPEND },
};

static sp_str_t fd_path(sp_mem_t mem, sp_sys_fd_t fd) {
  return sp_fmt(mem, "/dev/fd/{}", sp_fmt_int(fd)).value;
}

sp_test_each(fs, write_pipe, test_t, tests) {
  sp_sys_pipe_t pipe = sp_zero;
  sp_must_ok(t, sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)));

  sp_err_t err = sp_fs_write(fd_path(sp_test_arena(t), pipe.w), sp_str_lit("A"), it->mode);
  sp_sys_close(pipe.w);

  c8 buffer [2] = sp_zero;
  u64 len = 0;
  sp_sys_read(pipe.r, buffer, sizeof(buffer), &len);
  sp_sys_close(pipe.r);

  sp_expect_ok(t, err);
  sp_expect_str_eq(t, sp_str(buffer, (u32)len), sp_str_lit("A"));
  return SP_OK;
}

sp_test(fs, read_pipe) {
  sp_sys_pipe_t pipe = sp_zero;
  sp_must_ok(t, sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)));

  sp_sys_write(pipe.w, "A", 1, SP_NULLPTR);
  sp_sys_close(pipe.w);

  sp_str_t content = sp_zero;
  sp_err_t err = sp_fs_read(sp_test_arena(t), fd_path(sp_test_arena(t), pipe.r), &content);
  sp_sys_close(pipe.r);

  sp_expect_ok(t, err);
  sp_expect_str_eq(t, content, sp_str_lit("A"));
  return SP_OK;
}

#endif
