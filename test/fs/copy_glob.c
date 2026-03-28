#include "fs.h"

typedef struct {
  const c8* label;
  fs_setup_t src[16];
  const c8* glob;
  fs_expected_path_t expect[16];
} copy_glob_test_t;

static void run_copy_glob_test(s32* utest_result, sp_test_file_manager_t* fm, copy_glob_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_str_t src = sp_fs_join_path(sandbox, SP_LIT("src"));
  sp_str_t dst = sp_fs_join_path(sandbox, SP_LIT("dst"));
  sp_fs_create_dir(src);
  sp_fs_create_dir(dst);

  fs_apply_setup(utest_result, fm, src, t->src);
  sp_fs_copy_glob(src, sp_str_view(t->glob), dst);
  fs_expect_paths(utest_result, dst, t->expect);
}

UTEST_F(fs, copy_glob_star) {
  run_copy_glob_test(&ur, &ut.file_manager, &(copy_glob_test_t) {
    .label = "copy_glob_star",
    .src = {
      { .path = "a.txt", .kind = FS_SETUP_FILE },
      { .path = "b.txt", .kind = FS_SETUP_FILE },
    },
    .glob = "*",
    .expect = {
      { .path = "a.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "b.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs, copy_glob_exact_name) {
  run_copy_glob_test(&ur, &ut.file_manager, &(copy_glob_test_t) {
    .label = "copy_glob_exact_name",
    .src = {
      { .path = "a.txt", .kind = FS_SETUP_FILE },
      { .path = "b.txt", .kind = FS_SETUP_FILE },
    },
    .glob = "a.txt",
    .expect = {
      { .path = "a.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "b.txt", .exists = FS_EXPECT_NOT_EXIST },
    },
  });
}

UTEST_F(fs, copy_glob_no_match) {
  run_copy_glob_test(&ur, &ut.file_manager, &(copy_glob_test_t) {
    .label = "copy_glob_no_match",
    .src = {
      { .path = "a.txt", .kind = FS_SETUP_FILE },
      { .path = "b.txt", .kind = FS_SETUP_FILE },
    },
    .glob = "nope.txt",
    .expect = {
      { .path = "a.txt", .exists = FS_EXPECT_NOT_EXIST },
      { .path = "b.txt", .exists = FS_EXPECT_NOT_EXIST },
    },
  });
}

UTEST_F(fs, copy_glob_empty_src) {
  run_copy_glob_test(&ur, &ut.file_manager, &(copy_glob_test_t) {
    .label = "copy_glob_empty_src",
    .glob = "*",
  });
}

SP_TEST_MAIN()
