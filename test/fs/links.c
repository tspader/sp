#include "fs.h"

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  bool symlink;
  const c8* target;
  const c8* link;
  const c8* rewrite;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "hard_link_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .target = "A",
    .link = "B",
    .rewrite = "B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "hard_link_existing_destination_fails",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .target = "A",
    .link = "B",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "hard_link_directory_fails",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .target = "A",
    .link = "B",
    .expect = {
      // hard-linking a directory: POSIX reports EPERM; NT reports
      // STATUS_FILE_IS_A_DIRECTORY from FileLinkInformation
#if defined(SP_WIN32)
      .err = SP_ERR_SYS_IS_DIR,
#else
      .err = SP_ERR_SYS_ACCESS_DENIED,
#endif
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "B" },
      },
    },
  },
  {
    .name = "symlink_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .symlink = true,
    .target = "A",
    .link = "L",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .content = "A" },
      },
    },
  },
  {
    .name = "symlink_directory",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .symlink = true,
    .target = "A",
    .link = "L",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK },
      },
    },
  },
  {
    .name = "symlink_existing_destination_fails",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .symlink = true,
    .target = "A",
    .link = "B",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
};

sp_test_each(fs, links, test_t, tests) {
  if (it->symlink) fs_skip_if_no_symlinks(t);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t target = sp_fs_join_path(mem, sandbox, sp_str_view(it->target));
  sp_str_t link = sp_fs_join_path(mem, sandbox, sp_str_view(it->link));

  sp_err_t result = it->symlink
    ? sp_fs_create_sym_link(target, link)
    : sp_fs_create_hard_link(target, link);
  sp_expect_err_eq(t, result, it->expect.err);

  // a hard link shares content with its target: rewrite the target through
  // one name, then the expected paths observe the update through the other
  if (it->rewrite) {
    sp_io_file_writer_t writer = sp_zero;
    sp_io_file_writer_from_path(&writer, target);
    sp_io_write_str(&writer.base, sp_str_view(it->rewrite), SP_NULLPTR);
    sp_io_file_writer_close(&writer);
  }

  fs_expect_paths(t, sandbox, it->expect.paths);
  return SP_OK;
}
