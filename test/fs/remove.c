#include "fs.h"

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  bool dir;
  const c8* path;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "file_basic",
    .setup = {
      { "A" },
    },
    .path = "A",
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "file_missing",
    .path = "A",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "file_is_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .err = SP_ERR_SYS_IS_DIR,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "dir_recursive",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { "A/C", FS_SETUP_DIR },
      { "A/C/D" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      { .path = "A" },
      { .path = "A/B" },
      { .path = "A/C" },
      { .path = "A/C/D" },
    },
  },
  {
    .name = "dir_missing",
    .dir = true,
    .path = "A",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "dir_is_file",
    .setup = {
      { "A" },
    },
    .dir = true,
    .path = "A",
    .err = SP_ERR_SYS_NOT_DIR,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_does_not_follow_symlink",
    .setup = {
      { "B" },
      { "A", FS_SETUP_DIR },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "B" },
      { "A/C" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      { .path = "A" },
      { .path = "A/L" },
      { .path = "A/C" },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_symlink_root_removes_link_only",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .dir = true,
    .path = "L",
    .expect = {
      { .path = "L" },
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "dir_with_fifo",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/F", .kind = FS_SETUP_FIFO },
      { "A/B" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      { .path = "A" },
      { .path = "A/F" },
      { .path = "A/B" },
    },
  },
#endif
  {
    .name = "unicode_file",
    .setup = {
      { "\xc3\xb6\x70\x65\x6e.txt" },
    },
    .path = "\xc3\xb6\x70\x65\x6e.txt",
    .expect = {
      { .path = "\xc3\xb6\x70\x65\x6e.txt" },
    },
  },
  {
    .name = "unicode_dir",
    .setup = {
      { "\xc3\xa4\x62\x63", FS_SETUP_DIR },
      { "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt" },
    },
    .dir = true,
    .path = "\xc3\xa4\x62\x63",
    .expect = {
      { .path = "\xc3\xa4\x62\x63" },
      { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt" },
    },
  },
};

sp_test_each(fs, remove, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sandbox, sp_str_view(it->path));
  sp_err_t result = it->dir
    ? sp_fs_remove_dir(path)
    : sp_fs_remove_file(path);
  sp_expect_err_eq(t, result, it->err);

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}
