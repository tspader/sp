#ifndef FS_COPY_H
#define FS_COPY_H

#include "fs.h"

typedef enum {
  COPY_FILE,
  COPY_TREE,
  COPY_ANY,
} copy_op_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_fs_write_mode_t mode;
  const c8* src;
  const c8* dst_dir;
  const c8* dst;
  fs_expect_t expect;
} copy_any_mode_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_fs_write_mode_t mode;
  const c8* src;
  const c8* dst;
  fs_expect_by_mode_t expect;
} copy_by_mode_t;

static sp_err_t copy_run(sp_test_t* t, copy_op_t op, const fs_setup_t setup [FS_MAX_SETUP], sp_fs_write_mode_t mode, const c8* src, const c8* dst_dir, const c8* dst, const fs_expect_t* expect) {
  if (fs_setup_needs_symlinks(setup)) sp_test_skip_without_symlinks();
  if (fs_setup_needs_hard_links(setup)) sp_test_skip_without_hard_links();
  if (fs_setup_needs_fifos(setup)) posix_skip_without_fifos();
  if (fs_setup_needs_reparse_points(setup)) reparse_skip_unless_supported();
  if (fs_setup_needs_junctions(setup)) reparse_skip_on_network_path();
  if (mode == SP_FS_WRITE_EXCLUSIVE) fs_skip_without_exclusive();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, setup));

  sp_sys_dir_t from_dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &from_dir));

  sp_sys_dir_t to_dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(dst_dir ? sp_path_join(mem, sandbox, sp_cstr_as_str(dst_dir)) : sandbox, &to_dir));

  sp_path_t from = sp_path(from_dir, sp_cstr_as_str(src));
  sp_path_t to = sp_path(to_dir, sp_cstr_as_str(dst));

  sp_err_t err = SP_OK;
  switch (op) {
    case COPY_FILE: err = sp_fs_copy_file_at(from, to, mode); break;
    case COPY_TREE: err = sp_fs_copy_tree_at(from, to, mode); break;
    case COPY_ANY:  err = sp_fs_copy_at(from, to, mode); break;
  }
  sp_fs_close_dir(to_dir);
  sp_fs_close_dir(from_dir);
  sp_expect_err_eq(t, err, expect->err);

  fs_expect_paths(t, sandbox, expect->paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

#endif
