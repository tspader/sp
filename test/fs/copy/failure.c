#include "fs.h"

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_fs_write_mode_t mode;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "truncate_removes_file_it_created",
    .expect = {
      { .path = "B" },
    },
  },
  {
    .name = "truncate_leaves_existing_file",
    .setup = {
      { .path = "B", .content = "B" },
    },
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "new_leaves_nothing",
    .mode = SP_FS_WRITE_NEW,
    .expect = {
      { .path = "B" },
    },
  },
  {
    .name = "replace_keeps_destination",
    .setup = {
      { .path = "B", .content = "B" },
    },
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "exclusive_leaves_nothing",
    .mode = SP_FS_WRITE_EXCLUSIVE,
    .expect = {
      { .path = "B" },
    },
  },
};

static sp_err_t failing_write(sp_sys_fd_t fd, const void* buf, u64 count, u64* bytes_written) {
  if (bytes_written) *bytes_written = 0;
  return SP_ERR_SYS_IO;
}

static sp_err_t failing_pwrite(sp_sys_fd_t fd, const void* buf, u64 count, u64 offset, u64* bytes_written) {
  if (bytes_written) *bytes_written = 0;
  return SP_ERR_SYS_IO;
}

static sp_err_t failing_transfer(sp_sys_fd_t in, u64* in_pos, sp_sys_fd_t out, u64 count, u64* bytes_moved) {
  if (bytes_moved) *bytes_moved = 0;
  return SP_ERR_SYS_IO;
}

static sp_err_t failing_transfer_positional(sp_sys_fd_t in, u64* in_pos, sp_sys_fd_t out, u64 count, u64 offset, u64* bytes_moved) {
  if (bytes_moved) *bytes_moved = 0;
  return SP_ERR_SYS_IO;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, fs_apply_setup(t, sandbox, c->setup));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.write = failing_write;
  vt.pwrite = failing_pwrite;
  vt.transfer = failing_transfer;
  vt.transfer_positional = failing_transfer_positional;

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_copy_file_at(from, to, c->mode);
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, SP_ERR_SYS_IO);
  fs_expect_paths(t, sandbox, c->expect);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each_fn(fs, copy_failure, test_t, tests, run, .serial = true);
