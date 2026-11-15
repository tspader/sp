#include "fs.h"

#define A16 "AAAAAAAAAAAAAAAA"
#define A255 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 "AAAAAAAAAAAAAAA"

typedef enum {
  VARIANT_STR,
  VARIANT_SLICE,
  VARIANT_CSTR,
} variant_t;

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* path;
  const c8* content;
  sp_fs_write_mode_t mode;
  variant_t variant;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "truncate_new",
    .path = "A",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "truncate_existing",
    .setup = {
      { .path = "A", .content = "AB" },
    },
    .path = "A",
    .content = "C",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
  {
    .name = "truncate_empty",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .path = "A",
    .content = "",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
      },
    },
  },
  {
    .name = "truncate_missing_parent",
    .path = "D/A",
    .content = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "D" },
      },
    },
  },
  {
    .name = "truncate_through_symlink",
    .setup = {
      { .path = "T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
        { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "truncate_slice",
    .path = "A",
    .content = "A",
    .variant = VARIANT_SLICE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "truncate_cstr",
    .path = "A",
    .content = "A",
    .variant = VARIANT_CSTR,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "append_new",
    .path = "A",
    .content = "A",
    .mode = SP_FS_WRITE_APPEND,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "append_existing",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .path = "A",
    .content = "B",
    .mode = SP_FS_WRITE_APPEND,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "AB" },
      },
    },
  },
  {
    .name = "replace_new",
    .path = "A",
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "replace_existing",
    .setup = {
      { .path = "A", .content = "AB" },
    },
    .path = "A",
    .content = "C",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
  {
    .name = "replace_empty",
    .path = "A",
    .content = "",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
      },
    },
  },
  {
    .name = "replace_missing_parent",
    .path = "D/A",
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "D" },
      },
    },
  },
  {
    .name = "replace_name_at_component_limit",
    .path = A255,
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .paths = {
        { .path = A255, .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "replace_through_symlink",
    .setup = {
      { .path = "D/T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "D/T" },
    },
    .path = "L",
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .paths = {
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "D/T" },
        { .path = "D/T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "replace_through_dangling_symlink",
    .setup = {
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
        { .path = "T" },
      },
    },
  },
  {
    .name = "replace_onto_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .content = "A",
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .err = SP_ERR_SYS_IS_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "exclusive_new",
    .path = "A",
    .content = "A",
    .mode = SP_FS_WRITE_EXCLUSIVE,
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "exclusive_existing",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .path = "A",
    .content = "B",
    .mode = SP_FS_WRITE_EXCLUSIVE,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "exclusive_existing_symlink",
    .setup = {
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .mode = SP_FS_WRITE_EXCLUSIVE,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
        { .path = "T" },
      },
    },
  },
};

sp_test_each(fs, write, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_path_t path = sp_path_join(sp_test_arena(t), sandbox, sp_cstr_as_str(it->path));
  sp_err_t err = SP_OK;
  switch (it->variant) {
    case VARIANT_STR: {
      err = sp_fs_write_at(path, sp_cstr_as_str(it->content), it->mode);
      break;
    }
    case VARIANT_SLICE: {
      err = sp_fs_write_slice_at(path, sp_mem_slice((u8*)it->content, sp_cstr_len(it->content)), it->mode);
      break;
    }
    case VARIANT_CSTR: {
      err = sp_fs_write_cstr_at(path, it->content, it->mode);
      break;
    }
  }
  sp_expect_err_eq(t, err, it->expect.err);

  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
