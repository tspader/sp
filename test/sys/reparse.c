#include "sp.h"
#include "sp/sp_test.h"
#include "reparse.h"

#if defined(SP_WIN32)

typedef enum {
  ENTRY_JUNCTION,
  ENTRY_DANGLING_JUNCTION,
  ENTRY_STUB_FILE,
  ENTRY_STUB_DIR,
} entry_t;

typedef struct {
  sp_fs_kind_t kind;
  sp_fs_kind_t followed;
  sp_err_t follow;
  sp_err_t open_no_follow;
} expect_t;

typedef struct {
  const c8* name;
  entry_t entry;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "junction",
    .entry = ENTRY_JUNCTION,
    .expect = {
      .kind = SP_FS_KIND_SYMLINK,
      .followed = SP_FS_KIND_DIR,
      .open_no_follow = SP_ERR_SYS_LOOP,
    },
  },
  {
    .name = "dangling_junction",
    .entry = ENTRY_DANGLING_JUNCTION,
    .expect = {
      .kind = SP_FS_KIND_SYMLINK,
      .follow = SP_ERR_SYS_NOT_FOUND,
      .open_no_follow = SP_ERR_SYS_LOOP,
    },
  },
  {
    .name = "stub_file",
    .entry = ENTRY_STUB_FILE,
    .expect = {
      .kind = SP_FS_KIND_FILE,
      .followed = SP_FS_KIND_FILE,
      .open_no_follow = SP_ERR_SYS_NOT_DIR,
    },
  },
  {
    .name = "stub_dir",
    .entry = ENTRY_STUB_DIR,
    .expect = {
      .kind = SP_FS_KIND_DIR,
      .followed = SP_FS_KIND_DIR,
    },
  },
};

sp_test_each(sys, reparse, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t entry = sp_path_join(mem, sandbox, sp_str_lit("E"));
  sp_path_t target = sp_path_join(mem, sandbox, sp_str_lit("T"));
  sp_path_t child = sp_path_join(mem, sandbox, sp_str_lit("T/C"));

  sp_str_t root = sp_zero;
  sp_must_ok(t, sp_fs_canonicalize_path_at(mem, sandbox, &root));
  switch (it->entry) {
    case ENTRY_JUNCTION: {
      sp_must_ok(t, sp_fs_create_dir_at(target));
      sp_must_ok(t, sp_fs_write_at(child, sp_str_lit("C"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
      sp_must_ok(t, reparse_create_junction(mem, entry, sp_fs_join_path(mem, root, sp_str_lit("T"))));
      break;
    }
    case ENTRY_DANGLING_JUNCTION: {
      sp_must_ok(t, reparse_create_junction(mem, entry, sp_fs_join_path(mem, root, sp_str_lit("T"))));
      break;
    }
    case ENTRY_STUB_FILE: {
      sp_must_ok(t, reparse_create_stub(mem, entry, SP_FS_KIND_FILE));
      break;
    }
    case ENTRY_STUB_DIR: {
      sp_must_ok(t, reparse_create_stub(mem, entry, SP_FS_KIND_DIR));
      break;
    }
  }

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_sys_file_meta_t link = sp_zero;
  sp_expect_ok(t, sp_sys_get_link_metadata_s(dir, sp_str_lit("E"), &link));
  sp_expect_eq(t, link.kind, it->expect.kind);

  sp_sys_file_meta_t followed = sp_zero;
  sp_expect_err_eq(t, sp_sys_get_path_metadata_s(dir, sp_str_lit("E"), &followed), it->expect.follow);
  sp_expect_eq(t, followed.kind, it->expect.followed);

  sp_sys_dir_t opened = sp_zero;
  sp_err_t open = sp_sys_open_dir_s(dir, sp_str_lit("E"), SP_SYS_OPEN_DIR_NO_FOLLOW, &opened);
  sp_expect_err_eq(t, open, it->expect.open_no_follow);
  if (!open) sp_fs_close_dir(opened);

  sp_fs_it_t walk = sp_fs_it(mem, dir);
  sp_fs_it_entry_t walked = sp_zero;
  while (sp_fs_it_next(&walk, &walked)) {
    if (sp_str_equal(walked.path, sp_str_lit("E"))) sp_expect_eq(t, walked.kind, it->expect.kind);
    sp_expect(t, !sp_str_equal(walked.path, sp_str_lit("E/C")));
  }
  sp_expect_ok(t, walk.err);
  sp_fs_it_deinit(&walk);

  sp_err_t unlinked = sp_sys_unlink_s(dir, sp_str_lit("E"));
  switch (it->expect.kind) {
    case SP_FS_KIND_DIR: {
      sp_expect_err_eq(t, unlinked, SP_ERR_SYS_IS_DIR);
      sp_expect_ok(t, sp_sys_rmdir_s(dir, sp_str_lit("E")));
      break;
    }
    case SP_FS_KIND_FILE:
    case SP_FS_KIND_SYMLINK:
    case SP_FS_KIND_UNKNOWN:
    case SP_FS_KIND_NONE: {
      sp_expect_ok(t, unlinked);
      break;
    }
  }
  sp_expect_err_eq(t, sp_sys_get_link_metadata_s(dir, sp_str_lit("E"), &link), SP_ERR_SYS_NOT_FOUND);
  sp_fs_close_dir(dir);

  if (it->entry == ENTRY_JUNCTION) {
    sp_str_t content = sp_zero;
    sp_expect_ok(t, sp_fs_read_at(mem, child, &content));
    sp_expect_str_eq(t, content, sp_str_lit("C"));
  }
  return SP_OK;
}

#endif
