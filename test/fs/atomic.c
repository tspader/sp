#include "fs.h"

typedef enum {
  OP_REPLACE,
  OP_EXCLUSIVE,
  OP_ABORT,
} op_t;

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* path;
  const c8* content;
  op_t op;
  expect_t expect;
} test_t;

// Every path is relative to a directory fd opened on the sandbox, never to
// the process root.
static const test_t open_at_tests [] = {
  {
    .name = "nested_parents_created",
    .path = "A/B/C",
    .content = "C",
    .op = OP_REPLACE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
  {
    .name = "replace_existing",
    .setup = {
      { .path = "A", .content = "old" },
    },
    .path = "A",
    .content = "new",
    .op = OP_REPLACE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "new" },
      },
    },
  },
  {
    .name = "exclusive_new",
    .path = "A/B",
    .content = "B",
    .op = OP_EXCLUSIVE,
    .expect = {
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "exclusive_existing",
    .setup = {
      { .path = "A", .content = "old" },
    },
    .path = "A",
    .content = "new",
    .op = OP_EXCLUSIVE,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "old" },
      },
    },
  },
  {
    .name = "parent_is_file",
    .setup = {
      { .path = "P", .content = "P" },
    },
    .path = "P/B",
    .content = "B",
    .op = OP_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "abort_leaves_nothing",
    .path = "A/B",
    .content = "B",
    .op = OP_ABORT,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B" },
      },
    },
  },
};

// The temp lives in a staging directory "S" under the sandbox, so the
// destination's parent is first probed by the commit, not the open.
static const test_t staged_tests [] = {
  {
    .name = "parent_created_at_commit",
    .path = "A/B/C",
    .content = "C",
    .op = OP_REPLACE,
    .expect = {
      .paths = {
        { .path = "S", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
  {
    .name = "exclusive_parent_created_at_commit",
    .path = "A/B",
    .content = "B",
    .op = OP_EXCLUSIVE,
    .expect = {
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "exclusive_existing",
    .setup = {
      { .path = "A", .content = "old" },
    },
    .path = "A",
    .content = "new",
    .op = OP_EXCLUSIVE,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "old" },
      },
    },
  },
  {
    .name = "parent_is_file",
    .setup = {
      { .path = "P", .content = "P" },
    },
    .path = "P/B",
    .content = "B",
    .op = OP_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "exclusive_parent_is_file",
    .setup = {
      { .path = "P", .content = "P" },
    },
    .path = "P/B",
    .content = "B",
    .op = OP_EXCLUSIVE,
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "staging_is_file",
    .setup = {
      { .path = "S", .content = "S" },
    },
    .path = "A",
    .content = "A",
    .op = OP_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "S", .exists = true, .kind = SP_FS_KIND_FILE, .content = "S" },
      },
    },
  },
  {
    .name = "abort_leaves_nothing",
    .path = "A/B",
    .content = "B",
    .op = OP_ABORT,
    .expect = {
      .paths = {
        { .path = "A" },
        { .path = "A/B" },
      },
    },
  },
};

static sp_err_t drive(sp_fs_atomic_t* af, sp_err_t err, const test_t* it) {
  if (!err) err = sp_io_write_str(sp_fs_atomic_writer(af), sp_str_view(it->content), SP_NULLPTR);
  if (!err) {
    switch (it->op) {
      case OP_REPLACE:   err = sp_fs_atomic_commit(af, SP_FS_ATOMIC_REPLACE); break;
      case OP_EXCLUSIVE: err = sp_fs_atomic_commit(af, SP_FS_ATOMIC_EXCLUSIVE); break;
      case OP_ABORT:     err = sp_fs_atomic_abort(af); break;
    }
  }
  return err;
}

sp_test_each(fs, atomic_open_at, test_t, open_at_tests) {
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &dir));

  sp_fs_atomic_t af = sp_zero;
  sp_err_t err = drive(&af, sp_fs_atomic_open_at(&af, dir, sp_str_view(it->path)), it);
  sp_sys_close(dir);

  sp_expect_err_eq(t, err, it->expect.err);
  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each(fs, atomic_staged, test_t, staged_tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t path = sp_fs_join_path(mem, sandbox, sp_str_view(it->path));
  sp_str_t staging = sp_fs_join_path(mem, sandbox, sp_str_lit("S"));

  sp_fs_atomic_t af = sp_zero;
  sp_err_t err = drive(&af, sp_fs_atomic_open_staged(&af, path, staging), it);

  sp_expect_err_eq(t, err, it->expect.err);
  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test(fs, atomic_staged_empty_staging_is_bug) {
  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));

  sp_fs_atomic_t af = sp_zero;
  sp_expect_err_eq(t, sp_fs_atomic_open_staged(&af, path, sp_str_lit("")), SP_ERR_SYS_BUG);
  return SP_OK;
}
