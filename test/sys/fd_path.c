#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  TARGET_FILE,
  TARGET_DIR,
  TARGET_CWD,
  TARGET_PIPE,
  TARGET_GARBAGE,
} target_t;

typedef enum {
  FATE_KEPT,
  FATE_UNLINKED,
  FATE_REMOVED,
  FATE_RENAMED,
  FATE_REPLACED,
} fate_t;

typedef struct {
  sp_err_t err;
  const c8* path;
} expect_t;

typedef struct {
  const c8* name;
  target_t target;
  const c8* path;
  const c8* alias;
  const c8* decoy;
  fate_t fate;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "file",
    .target = TARGET_FILE,
    .path = "A",
    .expect = { .path = "A" },
  },
  {
    .name = "dir",
    .target = TARGET_DIR,
    .path = "A",
    .expect = { .path = "A" },
  },
  {
    .name = "cwd",
    .target = TARGET_CWD,
    .expect = { .path = "." },
  },
  {
    .name = "deleted_suffix_is_a_name",
    .target = TARGET_FILE,
    .path = "A (deleted)",
    .expect = { .path = "A (deleted)" },
  },
  {
    .name = "renamed",
    .target = TARGET_FILE,
    .path = "A",
    .fate = FATE_RENAMED,
    .expect = { .path = "B" },
  },
  {
    .name = "unlinked",
    .target = TARGET_FILE,
    .path = "A",
    .fate = FATE_UNLINKED,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "unlinked_with_a_live_alias",
    .target = TARGET_FILE,
    .path = "A",
    .alias = "B",
    .fate = FATE_UNLINKED,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "unlinked_beside_a_deleted_suffix",
    .target = TARGET_FILE,
    .path = "A",
    .decoy = "A (deleted)",
    .fate = FATE_UNLINKED,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "replaced",
    .target = TARGET_FILE,
    .path = "A",
    .fate = FATE_REPLACED,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "removed_dir",
    .target = TARGET_DIR,
    .path = "A",
    .fate = FATE_REMOVED,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "pipe",
    .target = TARGET_PIPE,
    .expect = { .err = SP_ERR_SYS_NOT_FOUND },
  },
  {
    .name = "garbage",
    .target = TARGET_GARBAGE,
    .expect = { .err = SP_ERR_SYS_BAD_FD },
  },
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

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  sp_path_t path = sp_path_join(mem, sandbox, sp_cstr_as_str(c->path));
  sp_path_t alias = sp_path_join(mem, sandbox, sp_cstr_as_str(c->alias));
  sp_path_t decoy = sp_path_join(mem, sandbox, sp_cstr_as_str(c->decoy));
  sp_path_t other = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_path_t expect = sp_path_join(mem, sandbox, sp_cstr_as_str(c->expect.path));

  sp_sys_pipe_t pipe = sp_zero;
  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  switch (c->target) {
    case TARGET_FILE: {
      sp_must_ok(t, sp_fs_create_file_at(path));
      if (c->alias) sp_must_ok(t, sp_fs_create_hard_link_at(path, alias));
      if (c->decoy) sp_must_ok(t, sp_fs_create_file_at(decoy));
      sp_must_ok(t, sp_sys_open_s(path.dir, path.sub, SP_SYS_OPEN_MODE_RO, 0, &fd));
      break;
    }
    case TARGET_DIR: {
      sp_must_ok(t, sp_fs_create_dir_at(path));
      sp_must_ok(t, sp_sys_open_dir_s(path.dir, path.sub, 0, &fd));
      break;
    }
    case TARGET_CWD: {
      expect = sp_path_at_cwd(sp_cstr_as_str(c->expect.path));
      fd = expect.dir;
      break;
    }
    case TARGET_PIPE: {
      sp_must_ok(t, sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)));
      fd = pipe.r;
      break;
    }
    case TARGET_GARBAGE: {
      fd = GARBAGE_FD;
      break;
    }
  }

  switch (c->fate) {
    case FATE_KEPT: {
      break;
    }
    case FATE_UNLINKED: {
      sp_must_ok(t, sp_sys_unlink_s(path.dir, path.sub));
      break;
    }
    case FATE_REMOVED: {
      sp_must_ok(t, sp_sys_rmdir_s(path.dir, path.sub));
      break;
    }
    case FATE_RENAMED: {
      sp_must_ok(t, sp_sys_rename_s(path.dir, path.sub, other.dir, other.sub));
      break;
    }
    case FATE_REPLACED: {
      sp_must_ok(t, sp_sys_unlink_s(path.dir, path.sub));
      sp_must_ok(t, sp_fs_create_file_at(path));
      break;
    }
  }

  c8 buf [SP_PATH_MAX];
  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  u64 len = 0;
  sp_err_t err = sp_sys_get_fd_path(fd, buf, sizeof(buf), &len);

  sp_expect_err_eq(t, err, c->expect.err);
  if (!err) {
    sp_expect_eq(t, buf[len], 0);
    sp_expect(t, same_file(expect, sp_path_at_cwd(sp_str(buf, (u32)len))));
  }

  switch (c->target) {
    case TARGET_FILE:
    case TARGET_DIR: {
      sp_sys_close(fd);
      break;
    }
    case TARGET_PIPE: {
      sp_sys_close(pipe.r);
      sp_sys_close(pipe.w);
      break;
    }
    case TARGET_CWD:
    case TARGET_GARBAGE: {
      break;
    }
  }
  return SP_OK;
}

sp_test_each_fn(sys, fd_path, test_t, tests, run);

typedef struct {
  const c8* name;
  s32 room;
  sp_err_t err;
} room_t;

static const room_t rooms [] = {
  { .name = "fits_with_nul",    .room = 1 },
  { .name = "no_room_for_nul",  .room = 0,  .err = SP_ERR_SYS_NAME_TOO_LONG },
  { .name = "too_small",        .room = -1, .err = SP_ERR_SYS_NAME_TOO_LONG },
};

sp_test_each(sys, fd_path_room, room_t, rooms) {
  sp_test_skip_on_wasm()

  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sandbox.dir, sandbox.sub, 0, &dir));

  c8 full [SP_PATH_MAX];
  u64 len = 0;
  sp_must_ok(t, sp_sys_get_fd_path(dir, full, sizeof(full), &len));

  c8 buf [SP_PATH_MAX];
  sp_for(at, sizeof(buf)) buf[at] = (c8)0xAB;
  u64 n = 1;
  sp_err_t err = sp_sys_get_fd_path(dir, buf, (u64)((s64)len + it->room), &n);

  sp_expect_err_eq(t, err, it->err);
  if (err) {
    sp_expect_eq(t, n, 0);
  }
  else {
    sp_expect_eq(t, n, len);
    sp_expect_eq(t, buf[len], 0);
    sp_expect_mem_eq(t, buf, full, len);
  }

  sp_sys_close(dir);
  return SP_OK;
}
