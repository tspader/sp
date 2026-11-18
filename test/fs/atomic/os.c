#include "fs.h"

#define A16 "AAAAAAAAAAAAAAAA"
#define A255 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 "AAAAAAAAAAAAAAA"

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
    .name = "missing_parent",
    .path = "A/B",
    .content = "B",
    .op = OP_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "A" },
      },
    },
  },
  {
    .name = "name_at_component_limit",
    .path = A255,
    .content = "A",
    .op = OP_REPLACE,
    .expect = {
      .paths = {
        { .path = A255, .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
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
    .path = "B",
    .content = "B",
    .op = OP_EXCLUSIVE,
    .expect = {
      .paths = {
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
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
      // a file in the parent chain: POSIX reports ENOTDIR, NT reports PATH_NOT_FOUND
#if defined(SP_WIN32)
      .err = SP_ERR_SYS_NOT_FOUND,
#else
      .err = SP_ERR_SYS_NOT_DIR,
#endif
      .paths = {
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "abort_leaves_nothing",
    .path = "A",
    .content = "A",
    .op = OP_ABORT,
    .expect = {
      .paths = {
        { .path = "A" },
      },
    },
  },
};

sp_test_each(fs, atomic_open_at, test_t, open_at_tests) {
  if (it->op == OP_EXCLUSIVE) fs_skip_without_exclusive();
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_fs_atomic_t af = sp_zero;
  sp_err_t err = sp_fs_atomic_open_at(&af, sp_path(dir, sp_cstr_as_str(it->path)), sp_sys_default_file_perms);
  if (!err) err = sp_io_write_str(sp_fs_atomic_writer(&af), sp_cstr_as_str(it->content), SP_NULLPTR);
  if (!err) {
    switch (it->op) {
      case OP_REPLACE:   err = sp_fs_atomic_commit(&af, SP_FS_ATOMIC_REPLACE); break;
      case OP_EXCLUSIVE: err = sp_fs_atomic_commit(&af, SP_FS_ATOMIC_EXCLUSIVE); break;
      case OP_ABORT:     err = sp_fs_atomic_abort(&af); break;
    }
  }
  sp_fs_close_dir(dir);

  sp_expect_err_eq(t, err, it->expect.err);
  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
