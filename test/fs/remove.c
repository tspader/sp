#include "fs.h"

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  bool remove_dir;
  const c8* remove_path;
  fs_expected_path_t expected[16];
} remove_test_t;

static void run_remove_test(s32* utest_result, sp_test_file_manager_t* fm, remove_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir_a(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t path = sp_fs_join_path_a(fm->allocator, sandbox, sp_str_view(t->remove_path));
  if (t->remove_dir) {
    sp_fs_remove_dir_a(path);
  } else {
    sp_fs_remove_file_a(path);
  }

  fs_expect_paths(utest_result, fm, sandbox, t->expected);
}

UTEST_F(fs, remove_file_basic) {
  run_remove_test(&ur, &ut.file_manager, &(remove_test_t) {
    .label = "remove_file_basic",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .remove_path = "file.txt",
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
    },
  });
}

UTEST_F(fs, remove_dir_recursive) {
  run_remove_test(&ur, &ut.file_manager, &(remove_test_t) {
    .label = "remove_dir_recursive",
    .setup = {
      { .path = "tree", .kind = FS_SETUP_DIR },
      { .path = "tree/file1.txt", .kind = FS_SETUP_FILE, .content = "a" },
      { .path = "tree/sub", .kind = FS_SETUP_DIR },
      { .path = "tree/sub/file2.txt", .kind = FS_SETUP_FILE, .content = "b" },
    },
    .remove_dir = true,
    .remove_path = "tree",
    .expected = {
      { .path = "tree", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "tree/file1.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "tree/sub", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "tree/sub/file2.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
    },
  });
}

UTEST_F(fs, remove_dir_does_not_follow_symlink) {
  SKIP_IF_NO_SYMLINKS();
  run_remove_test(&ur, &ut.file_manager, &(remove_test_t) {
    .label = "remove_dir_does_not_follow_symlink",
    .setup = {
      { .path = "outside.txt", .kind = FS_SETUP_FILE, .content = "outside" },
      { .path = "tree", .kind = FS_SETUP_DIR },
      { .path = "tree/link", .kind = FS_SETUP_SYMLINK, .target = "outside.txt" },
      { .path = "tree/file.txt", .kind = FS_SETUP_FILE, .content = "inside" },
    },
    .remove_dir = true,
    .remove_path = "tree",
    .expected = {
      { .path = "tree", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "tree/link", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "tree/file.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "outside.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, unicode_remove_file) {
  run_remove_test(&ur, &ut.file_manager, &(remove_test_t) {
    .label = "unicode_remove_file",
    .setup = {
      { .path = "\xc3\xb6\x70\x65\x6e.txt", .kind = FS_SETUP_FILE },
    },
    .remove_path = "\xc3\xb6\x70\x65\x6e.txt",
    .expected = {
      { .path = "\xc3\xb6\x70\x65\x6e.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
    },
  });
}

UTEST_F(fs, unicode_remove_dir) {
  run_remove_test(&ur, &ut.file_manager, &(remove_test_t) {
    .label = "unicode_remove_dir",
    .setup = {
      { .path = "\xc3\xa4\x62\x63", .kind = FS_SETUP_DIR },
      { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt", .kind = FS_SETUP_FILE, .content = "data" },
    },
    .remove_dir = true,
    .remove_path = "\xc3\xa4\x62\x63",
    .expected = {
      { .path = "\xc3\xa4\x62\x63", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
    },
  });
}

SP_TEST_MAIN()
