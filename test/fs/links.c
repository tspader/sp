#include "fs.h"

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* target;
  const c8* link;
  const c8* rewrite;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} hard_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* target;
  sp_fs_kind_t kind;
  const c8* link;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} sym_t;

static const hard_t hard_links [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .target = "A",
    .link = "B",
    .rewrite = "B",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "existing_destination_fails",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .target = "A",
    .link = "B",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "directory_fails",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .target = "A",
    .link = "B",
    // hard-linking a directory: POSIX reports EPERM; NT reports
    // STATUS_FILE_IS_A_DIRECTORY from FileLinkInformation
#if defined(SP_WIN32)
    .err = SP_ERR_SYS_IS_DIR,
#else
    .err = SP_ERR_SYS_ACCESS_DENIED,
#endif
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "B" },
    },
  },
};

static const sym_t sym_links [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .target = "A",
    .kind = SP_FS_KIND_FILE,
    .link = "L",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .content = "A", .target = "A" },
    },
  },
  {
    .name = "directory",
    .setup = {
      { .path = "A/F", .content = "F" },
    },
    .target = "A",
    .kind = SP_FS_KIND_DIR,
    .link = "L",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "A" },
      { .path = "L/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
    },
  },
  {
    .name = "dangling",
    .target = "A",
    .kind = SP_FS_KIND_FILE,
    .link = "L",
    .expect = {
      { .path = "A" },
      { .path = "L", .target = "A" },
    },
  },
  {
    .name = "existing_destination_fails",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .target = "A",
    .kind = SP_FS_KIND_FILE,
    .link = "B",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
};

sp_test_each(fs, hard_link, hard_t, hard_links) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  fs_apply_setup(t, sandbox, it->setup);

  sp_path_t target = sp_path_join(mem, sandbox, sp_cstr_as_str(it->target));
  sp_path_t link = sp_path_join(mem, sandbox, sp_cstr_as_str(it->link));
  sp_expect_err_eq(t, sp_fs_create_hard_link_at(target, link), it->err);

  // a hard link shares content with its target: rewrite the target through
  // one name, then the expected paths observe the update through the other
  if (it->rewrite) {
    sp_io_file_writer_t writer = sp_zero;
    sp_io_file_writer_from_path_at(&writer, target);
    sp_io_write_str(&writer.base, sp_cstr_as_str(it->rewrite), SP_NULLPTR);
    sp_io_file_writer_close(&writer);
  }

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}

sp_test_each(fs, sym_link, sym_t, sym_links) {
  sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  fs_apply_setup(t, sandbox, it->setup);

  sp_path_t link = sp_path_join(sp_test_arena(t), sandbox, sp_cstr_as_str(it->link));
  sp_expect_err_eq(t, sp_fs_create_sym_link_at(sp_cstr_as_str(it->target), link, it->kind), it->err);

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}
