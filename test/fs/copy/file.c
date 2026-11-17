#include "copy.h"

static const copy_any_mode_t any_mode [] = {
  {
    .name = "basic",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "empty",
    .setup = {
      { .path = "A" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .paths = {
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
      },
    },
  },
  {
    .name = "missing_parent",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "D/B",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "D" },
      },
    },
  },
  {
    .name = "across_dirs",
    .setup = {
      { .path = "A", .content = "A" },
      { "D", FS_SETUP_DIR },
    },
    .src = "A",
    .dst_dir = "D",
    .dst = "B",
    .expect = {
      .paths = {
        { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "dest_parent_is_symlink",
    .setup = {
      { .path = "A", .content = "A" },
      { "D", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "D" },
    },
    .src = "A",
    .dst = "L/B",
    .expect = {
      .paths = {
        { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "dest_parent_is_file",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "P", .content = "P" },
    },
    .src = "A",
    .dst = "P/B",
    .expect = {
      // a file in the parent chain: POSIX reports ENOTDIR, NT reports PATH_NOT_FOUND
#if defined(SP_WIN32)
      .err = SP_ERR_SYS_NOT_FOUND,
#else
      .err = SP_ERR_SYS_NOT_DIR,
#endif
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "source_symlink_followed",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .src = "L",
    .dst = "B",
    .expect = {
      .paths = {
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "source_missing",
    .setup = {
      { .path = "B", .content = "B" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "source_is_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .err = SP_ERR_SYS_IS_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "B" },
      },
    },
  },
};

static const copy_by_mode_t by_mode [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
        },
      },
    },
  },
  {
    .name = "itself",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "A",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
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
      { .path = "A", .content = "A" },
      { .path = "T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .src = "A",
    .dst = "L",
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
      { .path = "A", .content = "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .src = "A",
    .dst = "L",
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
    .name = "symlink_to_itself",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .src = "A",
    .dst = "L",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "L", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "A" },
        },
      },
    },
  },
  {
    .name = "hard_link_to_itself",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "H", .kind = FS_SETUP_HARD_LINK, .target = "A" },
    },
    .src = "A",
    .dst = "H",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "H", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "H", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
          { .path = "H", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
        },
      },
    },
  },
  {
    .name = "dir",
    .setup = {
      { .path = "A", .content = "A" },
      { "B", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .truncate = {
        .err = SP_ERR_SYS_IS_DIR,
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
      .replace = {
        .err = SP_ERR_SYS_IS_DIR,
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
        },
      },
    },
  },
};

sp_test_each(fs, copy_file, copy_any_mode_t, any_mode, .axes = {
  sp_test_axis_named(copy_any_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return copy_run(t, COPY_FILE, it->setup, it->mode, it->src, it->dst_dir, it->dst, &it->expect);
}

sp_test_each(fs, copy_file_onto, copy_by_mode_t, by_mode, .axes = {
  sp_test_axis_named(copy_by_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return copy_run(t, COPY_FILE, it->setup, it->mode, it->src, SP_NULLPTR, it->dst, fs_expect_for(&it->expect, it->mode));
}

typedef struct {
  const c8* name;
  sp_fs_write_mode_t mode;
  u32 size;
} size_test_t;

static const size_test_t sizes [] = {
  { .name = "chunk_minus_one", .size = 4095 },
  { .name = "chunk", .size = 4096 },
  { .name = "chunk_plus_one", .size = 4097 },
  { .name = "large", .size = 1 << 20 },
};

sp_test_each(fs, copy_file_size, size_test_t, sizes, .axes = {
  sp_test_axis_named(size_test_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));

  u8* data = sp_alloc_n(mem, u8, it->size);
  sp_for(i, it->size) {
    data[i] = (u8)(i * 31 + 7);
  }
  sp_must_ok(t, sp_fs_write_slice_at(from, sp_mem_slice(data, it->size), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, sp_fs_copy_file_at(from, to, it->mode));

  sp_str_t copied = sp_zero;
  sp_must_ok(t, sp_fs_read_at(mem, to, &copied));
  sp_must_eq(t, copied.len, it->size);
  sp_expect_mem_eq(t, copied.data, data, it->size);
  return SP_OK;
}

#if defined(SP_POSIX)
static sp_sys_file_perms_t mode(u32 value) {
  return (sp_sys_file_perms_t) { .value = value };
}

// postcondition: dest has source's bytes and its mode less the umask, on a fresh inode; timestamps unspecified
sp_test(fs, copy_file_creates_with_source_mode, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, sp_sys_set_file_perms_s(from.dir, from.sub, mode(0755)));

  sp_sys_file_meta_t from_meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(from.dir, from.sub, &from_meta));

  mode_t mask = umask(027);
  sp_err_t copied_err = sp_fs_copy_file_at(from, to, SP_FS_WRITE_REPLACE);
  umask(mask);
  sp_must_ok(t, copied_err);

  sp_sys_file_meta_t to_meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(to.dir, to.sub, &to_meta));
  sp_must_eq(t, to_meta.perms.value & 07777, 0750);
  sp_must_eq(t, to_meta.size, from_meta.size);
  sp_must(t, to_meta.id != from_meta.id);

  sp_str_t copied = sp_zero;
  sp_fs_read_at(mem, to, &copied);
  sp_expect_str_eq(t, copied, sp_str_lit("A"));
  return SP_OK;
}

sp_test(fs, copy_file_replaces_read_only_dest) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, sp_fs_write_at(to, sp_str_lit("B"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, sp_sys_set_file_perms_s(to.dir, to.sub, mode(0444)));

  sp_must_ok(t, sp_fs_copy_file_at(from, to, SP_FS_WRITE_REPLACE));

  sp_str_t copied = sp_zero;
  sp_fs_read_at(mem, to, &copied);
  sp_expect_str_eq(t, copied, sp_str_lit("A"));
  return SP_OK;
}

sp_test(fs, copy_file_unwritable_parent_leaves_nothing) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t dir = sp_path_join(mem, sandbox, sp_str_lit("D"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("D/B"));
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  sp_must_ok(t, sp_fs_create_dir_at(dir));

  sp_must_ok(t, sp_sys_set_file_perms_s(dir.dir, dir.sub, mode(0555)));
  sp_err_t result = sp_fs_copy_file_at(from, to, SP_FS_WRITE_REPLACE);
  sp_must_ok(t, sp_sys_set_file_perms_s(dir.dir, dir.sub, mode(0755)));

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  sp_sys_file_meta_t meta = sp_zero;
  sp_expect_err_eq(t, sp_fs_get_path_metadata_at(to, &meta), SP_ERR_SYS_NOT_FOUND);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
#endif
