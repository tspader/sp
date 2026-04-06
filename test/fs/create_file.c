#include "fs.h"
#include "test.h"

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  const c8* path;
  fs_expected_path_t expect[16];
} create_file_test_t;

static void run_create_file_test(s32* utest_result, sp_test_file_manager_t* fm, create_file_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t path = sp_fs_join_path(sandbox, sp_str_view(t->path));
  sp_fs_create_file(path);

  fs_expect_paths(utest_result, sandbox, t->expect);
}

UTEST_F(fs, create_file_basic) {
  run_create_file_test(&ur, &ut.file_manager, &(create_file_test_t) {
    .label = "create_file_basic",
    .path = "file.txt",
    .expect = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_file_idempotent) {
  run_create_file_test(&ur, &ut.file_manager, &(create_file_test_t) {
    .label = "create_file_idempotent",
    .setup = {
      { .path = "existing.txt", .kind = FS_SETUP_FILE },
    },
    .path = "existing.txt",
    .expect = {
      { .path = "existing.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_file_unicode) {
  run_create_file_test(&ur, &ut.file_manager, &(create_file_test_t) {
    .label = "create_file_unicode",
    .path = "\xc3\xb1\x61\x6d\x65.txt",
    .expect = {
      { .path = "\xc3\xb1\x61\x6d\x65.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_file_with_content) {
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_view("create_file_with_content"));
  sp_fs_create_dir(sandbox);

  u8 buffer [] = { 's', 'p', 'u', 'm', 0 };
  sp_str_t path = SP_ZERO_INITIALIZE();
  sp_str_t content = SP_ZERO_INITIALIZE();
  sp_err_t result = SP_OK;

  path = sp_fs_join_path(sandbox, sp_str_lit("slice.file"));
  result = sp_fs_create_file_slice(path, (sp_mem_slice_t) { buffer, 4 });
  sp_io_read_file(path, &content);
  EXPECT_EQ(result, SP_OK);
  EXPECT_TRUE(sp_fs_exists(path));
  SP_EXPECT_STR_EQ_CSTR(content, "spum");

  path = sp_fs_join_path(sandbox, sp_str_lit("str.file"));
  result = sp_fs_create_file_str(path, (sp_str_t) { (c8*)buffer, 4 });
  sp_io_read_file(path, &content);
  EXPECT_EQ(result, SP_OK);
  EXPECT_TRUE(sp_fs_exists(path));
  SP_EXPECT_STR_EQ_CSTR(content, "spum");

  path = sp_fs_join_path(sandbox, sp_str_lit("cstr.file"));
  result = sp_fs_create_file_cstr(path, (const c8*)buffer);
  sp_io_read_file(path, &content);
  EXPECT_EQ(result, SP_OK);
  EXPECT_TRUE(sp_fs_exists(path));
  SP_EXPECT_STR_EQ_CSTR(content, "spum");

}

SP_TEST_MAIN()
