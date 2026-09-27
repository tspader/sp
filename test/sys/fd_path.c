#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  HANDLE_FILE,
  HANDLE_DIR,
  HANDLE_CWD,
  HANDLE_PIPE,
  HANDLE_GARBAGE,
} handle_t;

typedef enum {
  FATE_KEPT,
  FATE_REMOVED,
} fate_t;

typedef struct {
  const c8* name;
  handle_t handle;
  fate_t fate;
  sp_err_t err;
} test_t;

static const test_t tests [] = {
  { .name = "file_is_named",            .handle = HANDLE_FILE },
  { .name = "dir_is_named",             .handle = HANDLE_DIR },
  { .name = "cwd_is_named",             .handle = HANDLE_CWD },
  { .name = "removed_file_has_no_name", .handle = HANDLE_FILE,    .fate = FATE_REMOVED, .err = SP_ERR_SYS_NOT_FOUND },
  { .name = "removed_dir_has_no_name",  .handle = HANDLE_DIR,     .fate = FATE_REMOVED, .err = SP_ERR_SYS_NOT_FOUND },
  { .name = "pipe_has_no_name",         .handle = HANDLE_PIPE,                          .err = SP_ERR_SYS_NOT_FOUND },
  { .name = "garbage_is_not_a_handle",  .handle = HANDLE_GARBAGE,                       .err = SP_ERR_SYS_BAD_FD },
};

#define GARBAGE_FD ((sp_sys_fd_t)0x55555)

static bool same_file(sp_path_t a, sp_path_t b) {
  sp_sys_file_meta_t ma = sp_zero;
  sp_sys_file_meta_t mb = sp_zero;
  if (sp_sys_get_link_metadata_s(a.dir, a.sub, &ma)) return false;
  if (sp_sys_get_link_metadata_s(b.dir, b.sub, &mb)) return false;
  return ma.id == mb.id && ma.device == mb.device;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_test_skip_on_wasm()

  sp_sys_fd_t sandbox = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_fs_get_cwd(), sp_test_dir(t), 0, &sandbox));

  sp_path_t witness = { .dir = sandbox, .sub = sp_str_lit("A") };
  sp_sys_pipe_t pipe = sp_zero;
  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  switch (c->handle) {
    case HANDLE_FILE: {
      sp_must_ok(t, sp_fs_create_file_at(witness));
      sp_must_ok(t, sp_sys_open_s(witness.dir, witness.sub, SP_SYS_OPEN_MODE_RO, 0, &fd));
      break;
    }
    case HANDLE_DIR: {
      sp_must_ok(t, sp_fs_create_dir_at(witness));
      sp_must_ok(t, sp_sys_open_dir_s(witness.dir, witness.sub, 0, &fd));
      break;
    }
    case HANDLE_CWD: {
      witness = sp_path_at_cwd(sp_str_lit("."));
      fd = witness.dir;
      break;
    }
    case HANDLE_PIPE: {
      sp_must_ok(t, sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)));
      fd = pipe.r;
      break;
    }
    case HANDLE_GARBAGE: {
      fd = GARBAGE_FD;
      break;
    }
  }

  switch (c->fate) {
    case FATE_KEPT: {
      break;
    }
    case FATE_REMOVED: {
      sp_err_t removed = c->handle == HANDLE_DIR ?
        sp_sys_rmdir_s(witness.dir, witness.sub) :
        sp_sys_unlink_s(witness.dir, witness.sub);
      sp_must_ok(t, removed);
      break;
    }
  }

  c8 buf [SP_PATH_MAX];
  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  u64 len = 0;
  sp_err_t err = sp_sys_get_fd_path(fd, buf, sizeof(buf), &len);

  sp_expect_err_eq(t, err, c->err);
  if (!err) {
    sp_expect_eq(t, buf[len], 0);
    sp_expect(t, same_file(witness, sp_path_at_cwd(sp_str(buf, (u32)len))));
  }

  switch (c->handle) {
    case HANDLE_FILE:
    case HANDLE_DIR: {
      sp_sys_close(fd);
      break;
    }
    case HANDLE_PIPE: {
      sp_sys_close(pipe.r);
      sp_sys_close(pipe.w);
      break;
    }
    case HANDLE_CWD:
    case HANDLE_GARBAGE: {
      break;
    }
  }
  sp_sys_close(sandbox);
  return SP_OK;
}

sp_test_each_fn(sys, fd_path, test_t, tests, run);

sp_test(sys, fd_path_survives_unlink_only_through_a_live_name) {
  sp_test_skip_on_wasm()

  sp_sys_fd_t sandbox = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_fs_get_cwd(), sp_test_dir(t), 0, &sandbox));

  sp_path_t opened = { .dir = sandbox, .sub = sp_str_lit("A") };
  sp_path_t alias = { .dir = sandbox, .sub = sp_str_lit("B") };
  sp_must_ok(t, sp_fs_create_file_at(opened));
  sp_must_ok(t, sp_fs_create_hard_link_at(opened, alias));

  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_s(opened.dir, opened.sub, SP_SYS_OPEN_MODE_RO, 0, &fd));
  sp_must_ok(t, sp_sys_unlink_s(opened.dir, opened.sub));

  c8 buf [SP_PATH_MAX];
  u64 len = 0;
  sp_err_t err = sp_sys_get_fd_path(fd, buf, sizeof(buf), &len);
  if (err) {
    sp_expect_err_eq(t, err, SP_ERR_SYS_NOT_FOUND);
  }
  else {
    sp_expect(t, same_file(alias, sp_path_at_cwd(sp_str(buf, (u32)len))));
  }

  sp_sys_close(fd);
  sp_sys_close(sandbox);
  return SP_OK;
}

sp_test(sys, fd_path_is_unsupported_on_wasm) {
#if !defined(SP_WASM)
  return sp_test_skip(t, "wasm only");
#else
  c8 buf [SP_PATH_MAX];
  u64 len = 1;
  sp_expect_err_eq(t, sp_sys_get_fd_path(sp_sys_get_root(0), buf, sizeof(buf), &len), SP_ERR_SYS_UNSUPPORTED);
  sp_expect_eq(t, len, 0);
  return SP_OK;
#endif
}

sp_test(sys, fd_path_refuses_overflow) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_fs_join_path(mem, sp_test_dir(t), sp_str_lit("file.bin"));
  sp_fs_create_file(path);

  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_s(sp_fs_get_cwd(), path, SP_SYS_OPEN_MODE_PATH, 0, &fd));

  c8 full [SP_PATH_MAX] = sp_zero;
  u64 len = 0;
  if (sp_sys_get_fd_path(fd, full, sizeof(full), &len)) {
    sp_sys_close(fd);
    sp_test_fail(t, "get_fd_path failed with a full-size buffer");
    return SP_OK;
  }

  c8 buf [SP_PATH_MAX];
  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  u64 n = 0;
  sp_err_t err = sp_sys_get_fd_path(fd, buf, len + 1, &n);
  if (err) {
    sp_test_fail(t, "exact fit failed: {}", sp_fmt_str(sp_err_str(err)));
  }
  else if (n != len) {
    sp_test_fail(t, "exact fit returned {} but expected {}", sp_fmt_int((s64)n), sp_fmt_int((s64)len));
  }
  else if (buf[len] != 0) {
    sp_test_fail(t, "exact fit did not NUL-terminate");
  }
  else if (!sp_mem_is_equal(buf, full, len)) {
    sp_test_fail(t, "exact fit returned different path");
  }

  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  err = sp_sys_get_fd_path(fd, buf, len, &n);
  if (err != SP_ERR_SYS_NAME_TOO_LONG) {
    sp_test_fail(t, "no room for NUL: returned {} but expected SP_ERR_SYS_NAME_TOO_LONG", sp_fmt_str(sp_err_str(err)));
  }

  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  err = sp_sys_get_fd_path(fd, buf, len - 1, &n);
  if (err != SP_ERR_SYS_NAME_TOO_LONG) {
    sp_test_fail(t, "undersized buffer: returned {} but expected SP_ERR_SYS_NAME_TOO_LONG", sp_fmt_str(sp_err_str(err)));
  }

  sp_sys_close(fd);
  return SP_OK;
}
