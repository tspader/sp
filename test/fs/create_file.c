#include "fs.h"
#include "test.h"

typedef enum {
  CREATE_FILE_EMPTY,
  CREATE_FILE_SLICE,
  CREATE_FILE_STR,
  CREATE_FILE_CSTR,
} create_file_variant_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  const c8* path;
  create_file_variant_t variant;
  c8 content [32];
  fs_expected_path_t expect[16];
} create_file_test_t;

static void run_create_file_test(s32* utest_result, sp_test_file_manager_t* fm, create_file_test_t t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_cstr_as_str(t.label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t.setup);

  sp_str_t path = sp_fs_join_path(fm->mem, sandbox, sp_cstr_as_str(t.path));
  sp_err_t result = SP_OK;
  switch (t.variant) {
    case CREATE_FILE_EMPTY: {
      result = sp_fs_create_file(path);
      break;
    }
    case CREATE_FILE_SLICE: {
      sp_mem_slice_t slice = { (u8*)t.content, sp_cstr_len(t.content) };
      result = sp_fs_create_file_slice(path, slice);
      break;
    }
    case CREATE_FILE_STR: {
      result = sp_fs_create_file_str(path, sp_cstr_as_str(t.content));
      break;
    }
    case CREATE_FILE_CSTR: {
      result = sp_fs_create_file_cstr(path, t.content);
      break;
    }
  }
  EXPECT_EQ(result, SP_OK);

  fs_expect_paths(utest_result, fm, sandbox, t.expect);
}

UTEST_F(fs, create_file_basic) {
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
    .label = "create_file_basic",
    .path = "file.txt",
    .expect = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_file_idempotent) {
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
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
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
    .label = "create_file_unicode",
    .path = "\xc3\xb1\x61\x6d\x65.txt",
    .expect = {
      { .path = "\xc3\xb1\x61\x6d\x65.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_file_slice) {
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
    .label = "create_file_slice",
    .path = "slice.file",
    .variant = CREATE_FILE_SLICE,
    .content = "spum",
    .expect = {
      { .path = "slice.file", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE, .content = "spum" },
    },
  });
}

UTEST_F(fs, create_file_str) {
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
    .label = "create_file_str",
    .path = "str.file",
    .variant = CREATE_FILE_STR,
    .content = "spum",
    .expect = {
      { .path = "str.file", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE, .content = "spum" },
    },
  });
}

UTEST_F(fs, create_file_cstr) {
  SKIP_ON_WASM()
  run_create_file_test(&ur, &ut.file_manager, (create_file_test_t){
    .label = "create_file_cstr",
    .path = "cstr.file",
    .variant = CREATE_FILE_CSTR,
    .content = "spum",
    .expect = {
      { .path = "cstr.file", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE, .content = "spum" },
    },
  });
}
