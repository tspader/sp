#include "fs.h"

typedef enum {
  VARIANT_STR,
  VARIANT_SLICE,
  VARIANT_CSTR,
} variant_t;

typedef struct {
  const c8* name;
  sp_fs_write_mode_t mode;
  variant_t variant;
  const c8* path;
  const c8* content;
  fs_expect_t expect;
} any_mode_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_fs_write_mode_t mode;
  const c8* path;
  const c8* content;
  fs_expect_by_mode_t expect;
} by_mode_t;

static const any_mode_t any_mode [] = {
  {
    .name = "free_name",
    .path = "A",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "empty",
    .path = "A",
    .content = "",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
      },
    },
  },
  {
    .name = "slice",
    .variant = VARIANT_SLICE,
    .path = "A",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "cstr",
    .variant = VARIANT_CSTR,
    .path = "A",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "missing_parent",
    .path = "D/A",
    .content = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "D" },
      },
    },
  },
};

static const by_mode_t by_mode [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "AB" },
    },
    .path = "A",
    .content = "C",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "AB" },
        },
      },
    },
  },
  {
    .name = "file_with_empty_content",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .path = "A",
    .content = "",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
    },
  },
  {
    .name = "symlink",
    .setup = {
      { .path = "T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
          { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "T" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
          { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "T" },
        },
      },
    },
  },
  {
    .name = "dangling_symlink",
    .setup = {
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
          { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "T" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
          { .path = "T" },
        },
      },
    },
  },
  {
    .name = "dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .content = "A",
    .expect = {
      .truncate = {
        .err = SP_ERR_SYS_IS_DIR,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
      .replace = {
        .err = SP_ERR_SYS_IS_DIR,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
    },
  },
};

static sp_err_t run(sp_test_t* t, const fs_setup_t setup [FS_MAX_SETUP], sp_fs_write_mode_t mode, variant_t variant, const c8* path, const c8* content, const fs_expect_t* expect) {
  if (fs_setup_needs_symlinks(setup)) sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, setup));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_path_t at = sp_path(dir, sp_cstr_as_str(path));
  sp_err_t err = SP_OK;
  switch (variant) {
    case VARIANT_STR: {
      err = sp_fs_write_at(at, sp_cstr_as_str(content), mode, sp_sys_default_file_perms);
      break;
    }
    case VARIANT_SLICE: {
      err = sp_fs_write_slice_at(at, sp_mem_slice((u8*)content, sp_cstr_len(content)), mode, sp_sys_default_file_perms);
      break;
    }
    case VARIANT_CSTR: {
      err = sp_fs_write_cstr_at(at, content, mode, sp_sys_default_file_perms);
      break;
    }
  }
  sp_fs_close_dir(dir);
  sp_expect_err_eq(t, err, expect->err);

  fs_expect_paths(t, sandbox, expect->paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each(fs, write, any_mode_t, any_mode, .axes = {
  sp_test_axis_named(any_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return run(t, (const fs_setup_t [FS_MAX_SETUP]) sp_zero, it->mode, it->variant, it->path, it->content, &it->expect);
}

sp_test_each(fs, write_onto, by_mode_t, by_mode, .axes = {
  sp_test_axis_named(by_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return run(t, it->setup, it->mode, VARIANT_STR, it->path, it->content, fs_expect_for(&it->expect, it->mode));
}
