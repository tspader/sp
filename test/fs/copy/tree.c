#include "copy.h"

static const copy_any_mode_t any_mode [] = {
  {
    .name = "nested",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/B/D", .content = "D" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "E/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "E/B", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "E/B/D", .exists = true, .kind = SP_FS_KIND_FILE, .content = "D" },
      },
    },
  },
  {
    .name = "creates_parents",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
    },
    .src = "A",
    .dst = "D/E",
    .expect = {
      .paths = {
        { .path = "D/E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "across_dirs",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "D", FS_SETUP_DIR },
    },
    .src = "A",
    .dst_dir = "D",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "D/E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "into_open_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "D", FS_SETUP_DIR },
    },
    .src = "A",
    .dst_dir = "D",
    .dst = ".",
    .expect = {
      .paths = {
        { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "merges_into_existing_dir",
    .setup = {
      { .path = "A/B/C", .content = "C" },
      { .path = "E/B/D", .content = "D" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/B/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "E/B/D", .exists = true, .kind = SP_FS_KIND_FILE, .content = "D" },
      },
    },
  },
  {
    .name = "merges_through_symlink_to_dir",
    .setup = {
      { .path = "A/B/C", .content = "C" },
      { "E/X", FS_SETUP_DIR },
      { .path = "E/B", .kind = FS_SETUP_DIR_SYMLINK, .target = "X" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/B", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "X" },
        { .path = "E/X/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
  {
    .name = "symlink_preserved",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
      },
    },
  },
  {
    .name = "symlink_to_dir_preserved",
    .setup = {
      { .path = "A/Z/F", .content = "F" },
      { .path = "A/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "Z" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/Z/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
        { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "Z" },
        { .path = "E/L/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
      },
    },
  },
#if defined(SP_WIN32)
  {
    .name = "junction_preserved",
    .setup = {
      { .path = "T/F", .content = "F" },
      { "A", FS_SETUP_DIR },
      { .path = "A/J", .kind = FS_SETUP_JUNCTION, .target = "T" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/J", .exists = true, .kind = SP_FS_KIND_SYMLINK, .junction = true },
        { .path = "E/J/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
      },
    },
  },
#endif
  {
    .name = "into_dir_behind_symlink_in_source",
    .setup = {
      { .path = "A/C", .content = "C" },
      { "X", FS_SETUP_DIR },
      { .path = "A/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "../X" },
    },
    .src = "A",
    .dst = "A/L/B",
    .expect = {
      .paths = {
        { .path = "X/B/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      },
    },
  },
};

static const copy_any_mode_t no_mode [] = {
  {
    .name = "empty",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "empty_subdir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .paths = {
        { .path = "E/B", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "dir_onto_file",
    .setup = {
      { .path = "A/B/C", .content = "C" },
      { .path = "E/B", .content = "B" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "into_itself",
    .setup = {
      { .path = "A/C", .content = "C" },
    },
    .src = "A",
    .dst = "A/B",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "A/B/C" },
      },
    },
  },
  {
    .name = "into_itself_nested",
    .setup = {
      { .path = "A/C", .content = "C" },
    },
    .src = "A",
    .dst = "A/D/E",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "A/D/E/C" },
      },
    },
  },
  {
    .name = "into_itself_through_symlink",
    .setup = {
      { .path = "A/C", .content = "C" },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .src = "A",
    .dst = "L/B",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "A/B/C" },
      },
    },
  },
#if !defined(SP_WASM)
  {
    .name = "into_itself_through_handle",
    .setup = {
      { .path = "A/C", .content = "C" },
      { "A/X", FS_SETUP_DIR },
    },
    .src = "A",
    .dst_dir = "A/X",
    .dst = "B",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
        { .path = "A/X/B/C" },
      },
    },
  },
#endif
  {
    .name = "dest_parent_is_file",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { .path = "P", .content = "P" },
    },
    .src = "A",
    .dst = "P/E",
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
      },
    },
  },
  {
    .name = "dest_is_file",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { .path = "E", .content = "E" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "E", .exists = true, .kind = SP_FS_KIND_FILE, .content = "E" },
      },
    },
  },
  {
    .name = "source_missing",
    .src = "A",
    .dst = "E",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "E" },
      },
    },
  },
  {
    .name = "source_is_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "E" },
      },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "fifo_inside",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/F", .kind = FS_SETUP_FIFO },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .err = SP_ERR_SYS_UNSUPPORTED,
      .paths = {
        { .path = "E/F" },
      },
    },
  },
#endif
};

static const copy_by_mode_t by_mode [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A/B", .content = "B" },
      { .path = "E/B", .content = "E" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
        },
      },
      .replace = {
        .paths = {
          { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "E" },
        },
      },
    },
  },
  {
    .name = "symlink",
    .setup = {
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
      { "E", FS_SETUP_DIR },
      { .path = "E/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "../A" },
    },
    .src = "A",
    .dst = "E",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
        },
      },
      .replace = {
        .paths = {
          { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "../A" },
        },
      },
    },
  },
  {
    .name = "itself",
    .setup = {
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
    },
    .src = "A",
    .dst = "A",
    .expect = {
      .truncate = {
        .paths = {
          { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
          { .path = "A/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
        },
      },
      .replace = {
        .paths = {
          { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
          { .path = "A/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
        },
      },
      .fail = {
        .err = SP_ERR_SYS_EXISTS,
        .paths = {
          { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
          { .path = "A/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
        },
      },
    },
  },
};

sp_test_each(fs, copy_tree, copy_any_mode_t, any_mode, .axes = {
  sp_test_axis_named(copy_any_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return copy_run(t, COPY_TREE, it->setup, it->mode, it->src, it->dst_dir, it->dst, &it->expect);
}

sp_test_each(fs, copy_tree_no_mode, copy_any_mode_t, no_mode) {
  return copy_run(t, COPY_TREE, it->setup, it->mode, it->src, it->dst_dir, it->dst, &it->expect);
}

sp_test_each(fs, copy_tree_onto, copy_by_mode_t, by_mode, .axes = {
  sp_test_axis_named(copy_by_mode_t, mode, fs_write_mode_name, FS_WRITE_MODES)
}) {
  return copy_run(t, COPY_TREE, it->setup, it->mode, it->src, SP_NULLPTR, it->dst, fs_expect_for(&it->expect, it->mode));
}

sp_test(fs, copy_tree_deeper_than_path_max) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t from = sp_path_join(mem, sp_test_dir(t), sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sp_test_dir(t), sp_str_lit("B"));
  sp_must_ok(t, fs_create_deep_tree(from));

  sp_must_ok(t, sp_fs_copy_tree_at(from, to, SP_FS_WRITE_REPLACE));

  sp_sys_dir_t cur = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(to, &cur));
  sp_for(level, FS_DEEP_LEVELS) {
    sp_sys_dir_t next = sp_zero;
    sp_err_t err = sp_fs_open_dir_at(sp_path(cur, sp_str_lit(FS_DEEP_NAME)), &next);
    sp_fs_close_dir(cur);
    sp_must_ok(t, err);
    cur = next;
  }
  sp_sys_file_meta_t meta = sp_zero;
  sp_err_t err = sp_sys_get_link_metadata_s(cur, sp_str_lit("F"), &meta);
  sp_fs_close_dir(cur);
  sp_must_ok(t, err);
  sp_expect_eq(t, meta.kind, SP_FS_KIND_FILE);
  return SP_OK;
}

#if defined(SP_POSIX)
sp_test(fs, copy_tree_unreadable_subdir_fails) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t locked = sp_path_join(mem, sandbox, sp_str_lit("A/B"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("E"));
  sp_must_ok(t, sp_fs_create_dir_at(locked));
  sp_must_ok(t, sp_fs_write_at(sp_path_join(mem, sandbox, sp_str_lit("A/B/C")), sp_str_lit("C"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));

  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, (sp_sys_file_perms_t) { .value = 0 }));
  sp_err_t result = sp_fs_copy_tree_at(from, to, SP_FS_WRITE_REPLACE);
  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, (sp_sys_file_perms_t) { .value = 0755 }));

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  sp_sys_file_meta_t meta = sp_zero;
  sp_expect_err_eq(t, sp_fs_get_path_metadata_at(sp_path_join(mem, sandbox, sp_str_lit("E/B/C")), &meta), SP_ERR_SYS_NOT_FOUND);
  return SP_OK;
}
#endif
