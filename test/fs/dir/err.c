#include "fs.h"

typedef enum {
  FROM_DIR,
  FROM_FILE,
  FROM_INVALID,
} from_t;

typedef struct {
  sp_err_t err;
} expect_t;

typedef struct {
  const c8* name;
  from_t from;
  u32 shrink;
  bool close;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "small_buffer",
    .shrink = 1,
    .expect = { .err = SP_ERR_SYS_INVALID },
  },
  {
    .name = "next_after_close",
    .close = true,
    .expect = { .err = SP_ERR_SYS_BAD_FD },
  },
  {
    .name = "file_handle",
    .from = FROM_FILE,
    .expect = { .err = SP_ERR_SYS_NOT_DIR },
  },
  {
    .name = "invalid_handle",
    .from = FROM_INVALID,
    .expect = { .err = SP_ERR_SYS_BAD_FD },
  },
};

sp_test_each(fs, dir_err, test_t, tests) {
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, sp_fs_create_file_at(sp_path_join(sp_test_arena(t), sandbox, sp_str_lit("A"))));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));
  sp_sys_fd_t file = SP_SYS_INVALID_FD;

  sp_sys_dir_t from = dir;
  switch (it->from) {
    case FROM_DIR: break;
    case FROM_FILE: {
      sp_must_ok(t, sp_sys_open_s(dir, sp_str_lit("A"), SP_SYS_OPEN_MODE_RO, 0, &file));
      from = sp_sys_dir(file);
      break;
    }
    case FROM_INVALID: {
      from = sp_sys_dir(SP_SYS_INVALID_FD);
      break;
    }
  }

  sp_fs_dir_it_t iter = sp_fs_dir_it(from);
  if (it->close) sp_expect_ok(t, sp_fs_dir_it_close(&iter));

  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  sp_expect(t, !sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf) - it->shrink), &entry));
  sp_expect_err_eq(t, iter.err, it->expect.err);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));

  if (file != SP_SYS_INVALID_FD) sp_sys_close(file);
  sp_fs_close_dir(dir);
  return SP_OK;
}
