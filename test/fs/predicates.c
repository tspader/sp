#include "fs.h"

typedef struct {
  const c8* path;
  bool exists;
  bool is_regular_file;
  bool is_dir;
  bool is_symlink;
  bool is_target_regular_file;
  bool is_target_dir;
  sp_fs_kind_t attr;
} fs_predicate_expected_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  fs_predicate_expected_t expected[16];
} fs_predicate_test_t;

static u32 fs_count_predicates(fs_predicate_expected_t* expected) {
  u32 n = 0;
  while (n < 16 && expected[n].path) n++;
  return n;
}

static void run_fs_predicate_test(s32* utest_result, sp_test_file_manager_t* fm, fs_predicate_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir_a(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  u32 expected_count = fs_count_predicates(t->expected);
  sp_for(i, expected_count) {
    fs_predicate_expected_t* exp = &t->expected[i];
    sp_str_t path = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(exp->path));

    fs_expect_bool(utest_result, path, "exists", sp_fs_exists_a(path), exp->exists);
    fs_expect_bool(utest_result, path, "is_regular_file", sp_fs_is_file_a(path), exp->is_regular_file);
    fs_expect_bool(utest_result, path, "is_dir", sp_fs_is_dir_a(path), exp->is_dir);
    fs_expect_bool(utest_result, path, "is_symlink", sp_fs_is_symlink_a(path), exp->is_symlink);
    fs_expect_bool(utest_result, path, "is_target_regular_file", sp_fs_is_target_file_a(path), exp->is_target_regular_file);
    fs_expect_bool(utest_result, path, "is_target_dir", sp_fs_is_target_dir_a(path), exp->is_target_dir);
    fs_expect_attr(utest_result, path, sp_fs_get_kind_a(path), exp->attr);
  }
}

UTEST_F(fs, predicate_matrix) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_predicate_test(&ur, &ut.file_manager, &(fs_predicate_test_t) {
    .label = "predicate_matrix",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "dir", .kind = FS_SETUP_DIR },
      { .path = "file.link", .kind = FS_SETUP_SYMLINK, .target = "file.txt" },
      { .path = "dir.link", .kind = FS_SETUP_SYMLINK, .target = "dir" },
    },
    .expected = {
      {
        .path = "file.txt",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = true,
        .is_target_regular_file = true,
        .attr = SP_FS_KIND_FILE,
      },
      {
        .path = "dir",
        .exists = FS_EXPECT_EXIST,
        .is_dir = true,
        .is_target_dir = true,
        .attr = SP_FS_KIND_DIR,
      },
      {
        .path = "file.link",
        .exists = FS_EXPECT_EXIST,
        .is_symlink = true,
        .is_target_regular_file = true,
        .attr = SP_FS_KIND_SYMLINK,
      },
      {
        .path = "dir.link",
        .exists = FS_EXPECT_EXIST,
        .is_symlink = true,
        .is_target_dir = true,
        .attr = SP_FS_KIND_SYMLINK,
      },
      {
        .path = "missing",
        .attr = SP_FS_KIND_NONE,
      },
    },
  });
}

UTEST_F(fs, unicode_predicate_matrix) {
  run_fs_predicate_test(&ur, &ut.file_manager, &(fs_predicate_test_t) {
    .label = "unicode_predicate_matrix",
    .setup = {
      { .path = "\xc3\xa9t\xc3\xa9.txt", .kind = FS_SETUP_FILE, .content = "data" },
      { .path = "\xc3\xb1\x61\x6d\x65", .kind = FS_SETUP_DIR },
    },
    .expected = {
      {
        .path = "\xc3\xa9t\xc3\xa9.txt",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = true,
        .is_target_regular_file = true,
        .attr = SP_FS_KIND_FILE,
      },
      {
        .path = "\xc3\xb1\x61\x6d\x65",
        .exists = FS_EXPECT_EXIST,
        .is_dir = true,
        .is_target_dir = true,
        .attr = SP_FS_KIND_DIR,
      },
      {
        .path = "missing\xc3\xa9",
        .attr = SP_FS_KIND_NONE,
      },
    },
  });
}

SP_TEST_MAIN()
