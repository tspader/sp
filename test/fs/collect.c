#include "fs.h"
#include "utest.h"

typedef enum {
  COLLECT_ENT_DIR,
  COLLECT_ENT_FILE,
  COLLECT_ENT_SYMLINK,
  COLLECT_ENT_MISSING,
} collect_entry_kind_t;

typedef struct {
  const c8* path;
  collect_entry_kind_t kind;
  const c8* target;
} collect_entry_t;

typedef struct {
  const c8* name;
  sp_fs_kind_t attr;
} collect_expect_t;

typedef struct {
  const c8* label;
  collect_entry_kind_t root;
  collect_entry_t files [16];
  collect_expect_t expect [16];
  bool recursive;
} collect_test_t;

struct fs_collect {
  sp_test_file_manager_t tmp;
};

UTEST_F_SETUP(fs_collect) {
  sp_test_file_manager_init(&ut.tmp);
  probe_symlinks(ut.tmp.paths.test);
}

UTEST_F_TEARDOWN(fs_collect) {
  sp_test_file_manager_cleanup(&ut.tmp);
}

static void add_entry(sp_test_file_manager_t* tmp, const c8* label, collect_entry_t entry) {
  sp_str_t root = sp_test_file_path(tmp, sp_str_view(label));

  sp_str_t path = sp_fs_join_path(root, sp_str_view(entry.path));
  switch (entry.kind) {
    case COLLECT_ENT_FILE: {
      sp_str_t parent = sp_fs_parent_path(path);
      if (!sp_str_empty(parent) && !sp_fs_exists(parent)) {
        sp_fs_create_dir(parent);
      }
      sp_test_file_create_ex((sp_test_file_config_t) { .path = path });
      break;
    }
    case COLLECT_ENT_DIR: {
      sp_fs_create_dir(path);
      break;
    }
    case COLLECT_ENT_SYMLINK: {
      sp_str_t target = sp_fs_join_path(root, sp_str_view(entry.target));
      sp_fs_create_sym_link(target, path);
      break;
    }
    case COLLECT_ENT_MISSING: {
      break;
    }
  }
}

static void run_collect_test(s32* utest_result, sp_test_file_manager_t* tmp, collect_test_t* test) {
  add_entry(tmp, "", (collect_entry_t) { .kind = test->root, .path = test->label });

  sp_carr_for(test->files, it) {
    if (!test->files[it].path) break;
    add_entry(tmp, test->label, test->files[it]);
  }

  sp_str_t root = sp_test_file_path(tmp, sp_str_view(test->label));
  sp_da(sp_fs_entry_t) results = test->recursive
    ? sp_fs_collect_recursive(root)
    : sp_fs_collect(root);

  u32 num_expected = 0;
  sp_carr_for(test->expect, it) {
    if (!test->expect[it].name) break;
    num_expected++;
  }

  EXPECT_EQ(sp_da_size(results), num_expected);

  sp_for(i, num_expected) {
    collect_expect_t exp = test->expect[i];
    sp_str_t expected_path = sp_fs_join_path(root, sp_str_view(exp.name));

    bool found = false;
    sp_da_for(results, n) {
      sp_fs_entry_t entry = results[n];
      if (sp_str_equal(entry.path, expected_path)) {
        EXPECT_EQ(entry.kind, exp.attr);
        found = true;
        break;
      }
    }

    EXPECT_TRUE(found);
  }
}

UTEST_F(fs_collect, empty_dir) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "empty_dir",
  });
}

UTEST_F(fs_collect, nonexistent) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "nonexistent",
    .root = COLLECT_ENT_MISSING,
    .expect = {}
  });
}

UTEST_F(fs_collect, file_not_dir) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "file_not_dir",
    .root = COLLECT_ENT_FILE,
    .expect = {}
  });
}

UTEST_F(fs_collect, single_file) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "single_file",
    .files = {
      { "hello.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { "hello.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, multiple_files) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "multiple_files",
    .files = {
      { "a.c", COLLECT_ENT_FILE },
      { "b.h", COLLECT_ENT_FILE },
      { "c.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { "a.c", SP_FS_KIND_FILE },
      { "b.h", SP_FS_KIND_FILE },
      { "c.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, subdirectory) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "subdirectory",
    .files = {
      { "child", COLLECT_ENT_DIR },
    },
    .expect = {
      { "child", SP_FS_KIND_DIR },
    },
  });
}

UTEST_F(fs_collect, mixed_types) {
  SKIP_IF_NO_SYMLINKS();
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "mixed_types",
    .files = {
      { "file.txt", COLLECT_ENT_FILE },
      { "subdir", COLLECT_ENT_DIR },
      { "link", COLLECT_ENT_SYMLINK, "file.txt" },
    },
    .expect = {
      { "file.txt", SP_FS_KIND_FILE },
      { "subdir", SP_FS_KIND_DIR },
      { "link", SP_FS_KIND_SYMLINK },
    },
  });
}

UTEST_F(fs_collect, hidden_file) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "hidden_file",
    .files = {
      { ".gitignore", COLLECT_ENT_FILE },
      { "visible.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { ".gitignore", SP_FS_KIND_FILE },
      { "visible.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, does_not_recurse) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "does_not_recurse",
    .files = {
      { "child", COLLECT_ENT_DIR },
      { "child/deep.txt", COLLECT_ENT_FILE },
      { "top.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { "child", SP_FS_KIND_DIR },
      { "top.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_empty_dir) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_empty_dir",
    .recursive = true,
  });
}

UTEST_F(fs_collect, recursive_flat_dir) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_flat_dir",
    .recursive = true,
    .files = {
      { "a.txt", COLLECT_ENT_FILE },
      { "b.txt", COLLECT_ENT_FILE },
      { "c.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { "a.txt", SP_FS_KIND_FILE },
      { "b.txt", SP_FS_KIND_FILE },
      { "c.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_nested_dirs) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_nested_dirs",
    .recursive = true,
    .files = {
      { "sub", COLLECT_ENT_DIR },
      { "sub/deep", COLLECT_ENT_DIR },
      { "a.txt", COLLECT_ENT_FILE },
      { "sub/b.txt", COLLECT_ENT_FILE },
      { "sub/deep/c.txt", COLLECT_ENT_FILE },
    },
    .expect = {
      { "a.txt", SP_FS_KIND_FILE },
      { "sub", SP_FS_KIND_DIR },
      { "sub/b.txt", SP_FS_KIND_FILE },
      { "sub/deep", SP_FS_KIND_DIR },
      { "sub/deep/c.txt", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_symlink_not_followed) {
  SKIP_IF_NO_SYMLINKS();
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_symlink_not_followed",
    .recursive = true,
    .files = {
      { "real_dir", COLLECT_ENT_DIR },
      { "real_dir/file.txt", COLLECT_ENT_FILE },
      { "link", COLLECT_ENT_SYMLINK, "real_dir" },
    },
    .expect = {
      { "real_dir", SP_FS_KIND_DIR },
      { "real_dir/file.txt", SP_FS_KIND_FILE },
      { "link", SP_FS_KIND_SYMLINK },
    },
  });
}

UTEST_F(fs_collect, recursive_nonexistent) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_nonexistent",
    .root = COLLECT_ENT_MISSING,
    .recursive = true,
    .expect = {}
  });
}

UTEST_F(fs_collect, recursive_file_not_dir) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "recursive_file_not_dir",
    .root = COLLECT_ENT_FILE,
    .recursive = true,
    .expect = {}
  });
}

UTEST_F(fs_collect, unicode_entries) {
  run_collect_test(&ur, &ut.tmp, &(collect_test_t) {
    .label = "unicode_entries",
    .files = {
      { "\xc3\xb1\x61\x6d\x65.txt", COLLECT_ENT_FILE },
      { "\xc3\xbc\x6e\x69", COLLECT_ENT_DIR },
    },
    .expect = {
      { "\xc3\xb1\x61\x6d\x65.txt", SP_FS_KIND_FILE },
      { "\xc3\xbc\x6e\x69", SP_FS_KIND_DIR },
    },
  });
}

SP_TEST_MAIN()
