#include "fs.h"

typedef enum {
  VARIANT_EMPTY,
  VARIANT_SLICE,
  VARIANT_STR,
  VARIANT_CSTR,
} variant_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* path;
  variant_t variant;
  const c8* content;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "basic",
    .path = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "idempotent",
    .setup = {
      { "A" },
    },
    .path = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "unicode",
    .path = "\xc3\xb1\x61\x6d\x65.txt",
    .expect = {
      { .path = "\xc3\xb1\x61\x6d\x65.txt", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "slice",
    .path = "A",
    .variant = VARIANT_SLICE,
    .content = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "str",
    .path = "A",
    .variant = VARIANT_STR,
    .content = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "cstr",
    .path = "A",
    .variant = VARIANT_CSTR,
    .content = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
};

sp_test_each(fs, create_file, test_t, tests) {
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &dir));

  sp_path_t path = { .dir = dir, .sub = sp_str_view(it->path) };
  sp_err_t result = SP_OK;
  switch (it->variant) {
    case VARIANT_EMPTY: {
      result = sp_fs_create_file_at(path);
      break;
    }
    case VARIANT_SLICE: {
      result = sp_fs_create_file_slice_at(path, sp_mem_slice((u8*)it->content, sp_cstr_len(it->content)));
      break;
    }
    case VARIANT_STR: {
      result = sp_fs_create_file_str_at(path, sp_str_view(it->content));
      break;
    }
    case VARIANT_CSTR: {
      result = sp_fs_create_file_cstr_at(path, it->content);
      break;
    }
  }
  sp_sys_close(dir);
  sp_expect_ok(t, result);

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}
