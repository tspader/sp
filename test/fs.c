#include "sp.h"
#include "test.h"
#include "utest.h"

#if defined (SP_POSIX)
  #include "sys/stat.h"
#endif

static bool are_symlinks_available = false;

static void probe_symlinks(sp_str_t test_dir) {
  static bool probed = false;
  if (probed) return;
  probed = true;

  sp_str_t target = sp_fs_join_path(test_dir, SP_LIT(".symlink_probe_target"));
  sp_str_t link = sp_fs_join_path(test_dir, SP_LIT(".symlink_probe_link"));
  sp_fs_create_file(target);
  are_symlinks_available = sp_fs_create_sym_link(target, link) == SP_OK;
  if (are_symlinks_available) sp_fs_remove_file(link);
  sp_fs_remove_file(target);
}

#define SKIP_IF_NO_SYMLINKS() \
  if (!are_symlinks_available) { UTEST_SKIP("symlinks not available"); }

struct fs {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(fs) {
  sp_test_file_manager_init(&ut.file_manager);
  probe_symlinks(ut.file_manager.paths.test);
}

UTEST_F_TEARDOWN(fs) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(fs, mod_time) {
  sp_str_t file = sp_test_file_path(&ut.file_manager, sp_str_lit("a.file"));
  sp_str_t dir = sp_test_file_path(&ut.file_manager, sp_str_lit("a.dir"));
  sp_fs_create_dir(dir);
  sp_fs_create_file(file);
  sp_tm_epoch_t mod_time = SP_ZERO_INITIALIZE();
  mod_time = sp_fs_get_mod_time(file);
  EXPECT_TRUE(mod_time.s > 0);
  mod_time = sp_fs_get_mod_time(dir);
  EXPECT_TRUE(mod_time.s > 0);
}

#if defined(SP_POSIX)
UTEST_F(fs, copy_preserves_file_attributes) {
  // Create a test file with specific content
  sp_str_t source_file = sp_test_file_create_empty(&ut.file_manager, SP_LIT("source_attrs.txt"));
  sp_str_t test_content = SP_LIT("Test content for attribute preservation - this should be preserved exactly");
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = source_file,
    .content = test_content,
  });

  // Change file permissions to something specific (0o755 = rwxr-xr-x)
  const c8* source_cstr = sp_str_to_cstr(source_file);
  ASSERT_EQ(chmod(source_cstr, 0755), 0);

  // Get original file stat information
  struct stat original_stat = {0};
  ASSERT_EQ(stat(source_cstr, &original_stat), 0);

  // Copy the file
  sp_str_t copy_file = sp_test_file_path(&ut.file_manager, SP_LIT("copy_attrs.txt"));
  ASSERT_EQ(sp_fs_copy(source_file, copy_file), SP_OK);

  // Verify copy exists and has same type
  ASSERT_TRUE(sp_fs_exists(copy_file));
  ASSERT_TRUE(sp_fs_is_regular_file(copy_file));

  // Get copied file stat information
  const c8* copy_cstr = sp_str_to_cstr(copy_file);
  struct stat copy_stat = {0};
  ASSERT_EQ(stat(copy_cstr, &copy_stat), 0);

  // Verify file permissions (mode) are preserved
  ASSERT_EQ(original_stat.st_mode, copy_stat.st_mode);

  // Verify file size is preserved
  ASSERT_EQ(original_stat.st_size, copy_stat.st_size);

  // Verify content is identical
  sp_str_t copy_content = sp_io_read_file(copy_file);
  SP_EXPECT_STR_EQ(copy_content, test_content);
}
#endif

typedef struct {
  sp_str_t file_path;
  sp_str_t stem;
} sp_test_file_stem_case_t;

UTEST(fs_path, path_stem) {
  sp_test_file_stem_case_t cases [] = {
    {
      .file_path = SP_LIT("foo.bar"),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo."),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo.bar.baz"),
      .stem = SP_LIT("foo.bar")
    },
    {
      .file_path = SP_LIT("foo"),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo.bar."),
      .stem = SP_LIT("foo.bar")
    },
    {
      .file_path = SP_LIT(".foo"),
      .stem = SP_LIT("")
    },
  };

  SP_CARR_FOR(cases, index) {
    sp_str_t stem = sp_fs_get_stem(cases[index].file_path);
    SP_EXPECT_STR_EQ(stem, cases[index].stem);
  }
}

UTEST(fs_path, normalize_path) {
  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\file.txt");
    sp_str_t copy = sp_fs_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t copy = sp_fs_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users\\Test/sub\\file.txt");
    sp_str_t copy = sp_fs_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/sub/file.txt");
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t copy = sp_fs_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "");
  }

  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\");
    sp_str_t copy = sp_fs_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test");
  }
}

UTEST(fs_path, parent_path) {
  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users/Test");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test///");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users");
  }

  {
    sp_str_t path = SP_LIT("C:/");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/");
  }

  {
    sp_str_t path = SP_LIT("Test");
    sp_str_t parent = sp_fs_parent_path(path);
    EXPECT_TRUE(sp_str_empty(parent));
  }

  {
    sp_str_t path = SP_LIT("foo/bar");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "foo");
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t parent = sp_fs_parent_path(path);
    EXPECT_TRUE(sp_str_empty(parent));
  }

  {
    sp_str_t path = SP_LIT("/");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "/");
  }

  {
    sp_str_t path = SP_LIT("/home/user/file");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "/home/user");
  }
}

UTEST(fs_path, canonicalize_path) {
  {
    sp_str_t path = SP_LIT("test/..");
    sp_str_t canonical = sp_fs_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/');
  }

  {
    sp_str_t path = SP_LIT("../../another");
    sp_str_t canonical = sp_fs_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    sp_str_t filename = sp_fs_get_name(canonical);
    SP_EXPECT_STR_EQ_CSTR(filename, "another");
  }

  {
    sp_str_t exe = sp_fs_get_exe_path();
    sp_str_t canonical = sp_fs_canonicalize_path(exe);
    ASSERT_TRUE(sp_str_equal(canonical, exe));
  }

  {
    sp_str_t path = SP_LIT("test/");
    sp_str_t canonical = sp_fs_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/');
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t canonical = sp_fs_canonicalize_path(path);
    ASSERT_EQ(canonical.len, 0);
  }
}

typedef struct {
  sp_str_t file_path;
  sp_str_t extension;
} sp_test_file_extension_case_t;

UTEST(fs_path, path_extension) {
  sp_test_file_extension_case_t cases [] = {
    {
      .file_path = SP_LIT("foo.bar"),
      .extension = SP_LIT("bar")
    },
    {
      .file_path = SP_LIT("foo."),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT("foo.bar.baz"),
      .extension = SP_LIT("baz")
    },
    {
      .file_path = SP_LIT("foo"),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT("foo.bar."),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT(".foo"),
      .extension = SP_LIT("foo")
    },
  };

  SP_CARR_FOR(cases, index) {
    sp_str_t extension = sp_fs_get_ext(cases[index].file_path);
    SP_EXPECT_STR_EQ(extension, cases[index].extension);
  }
}


UTEST(fs_path, extract_file_name) {
  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t filename = sp_fs_get_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/");
    sp_str_t filename = sp_fs_get_name(path);
    ASSERT_EQ(filename.len, 0);
  }

  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\file.txt");
    sp_str_t filename = sp_fs_get_name(path);
    SP_EXPECT_STR_EQ(path, filename);
    //SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("file.txt");
    sp_str_t filename = sp_fs_get_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t filename = sp_fs_get_name(path);
    ASSERT_EQ(filename.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/foo/bar/");
    sp_str_t filename = sp_fs_get_name(path);
    EXPECT_TRUE(sp_str_empty(filename));
  }

  {
    sp_str_t path = SP_LIT("/home/user/document.pdf");
    sp_str_t filename = sp_fs_get_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "document.pdf");
  }
}

UTEST(fs_path, get_executable_path) {
  sp_str_t exe_path = sp_fs_get_exe_path();

  ASSERT_GT(exe_path.len, 0);

  bool has_backslash = false;
  for (u32 i = 0; i < exe_path.len; i++) {
    if (exe_path.data[i] == '\\') {
      has_backslash = true;
      break;
    }
  }
  ASSERT_FALSE(has_backslash);

  ASSERT_NE(exe_path.data[exe_path.len - 1], '/');

  sp_str_t filename = sp_fs_get_name(exe_path);
  ASSERT_GT(filename.len, 0);
}

UTEST(fs_path, integration_test) {
  sp_str_t exe = sp_fs_get_exe_path();
  sp_str_t parent1 = sp_fs_parent_path(exe);
  sp_str_t parent2 = sp_fs_parent_path(parent1);
  sp_str_t parent3 = sp_fs_parent_path(parent2);
  sp_str_t install = sp_fs_canonicalize_path(parent3);

  ASSERT_GT(install.len, 0);
  ASSERT_NE(install.data[install.len - 1], '/');

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, install);
  sp_str_builder_append(&builder, SP_LIT("/build/space-dll.bat"));
  sp_str_t dll_path = sp_str_builder_to_str(&builder);

  bool has_double_slash = false;
  for (u32 i = 1; i < dll_path.len; i++) {
    if (dll_path.data[i-1] == '/' && dll_path.data[i] == '/') {
      has_double_slash = true;
      break;
    }
  }
  ASSERT_FALSE(has_double_slash);
}

//////////////////////
// COLLECT FIXTURE  //
//////////////////////
typedef enum {
  COLLECT_ENT_FILE,
  COLLECT_ENT_DIR,
  COLLECT_ENT_SYMLINK,
} collect_ent_kind_t;

typedef enum {
  COLLECT_ROOT_DIR,
  COLLECT_ROOT_FILE,
  COLLECT_ROOT_MISSING,
} collect_root_kind_t;

typedef struct {
  const c8* path;
  collect_ent_kind_t kind;
  const c8* symlink_target;
} collect_setup_ent_t;

typedef struct {
  const c8* name;
  sp_fs_attr_t attr;
} collect_expected_ent_t;

typedef struct {
  const c8* label;
  collect_root_kind_t root_kind;
  bool recursive;
  collect_setup_ent_t setup[16];
  collect_expected_ent_t expected[16];
  bool expect_null;
} collect_test_t;

struct fs_collect {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(fs_collect) {
  sp_test_file_manager_init(&ut.file_manager);
  probe_symlinks(ut.file_manager.paths.test);
}

UTEST_F_TEARDOWN(fs_collect) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

static u32 collect_count_setup(collect_setup_ent_t* setup) {
  u32 n = 0;
  while (n < 16 && setup[n].path) n++;
  return n;
}

static u32 collect_count_expected(collect_expected_ent_t* expected) {
  u32 n = 0;
  while (n < 16 && expected[n].name) n++;
  return n;
}

static void run_collect_test(int* utest_result, sp_test_file_manager_t* fm, collect_test_t* t) {
  sp_str_t collect_path = sp_test_file_path(fm, sp_str_view(t->label));

  switch (t->root_kind) {
    case COLLECT_ROOT_DIR: {
      sp_fs_create_dir(collect_path);
      break;
    }
    case COLLECT_ROOT_FILE: {
      sp_test_file_create_ex((sp_test_file_config_t) { .path = collect_path });
      break;
    }
    case COLLECT_ROOT_MISSING: {
      break;
    }
  }

  u32 setup_count = collect_count_setup(t->setup);
  sp_for(i, setup_count) {
    collect_setup_ent_t* s = &t->setup[i];
    sp_str_t full = sp_fs_join_path(collect_path, sp_str_view(s->path));
    switch (s->kind) {
      case COLLECT_ENT_FILE: {
        sp_str_t parent = sp_fs_parent_path(full);
        if (!sp_str_empty(parent) && !sp_fs_exists(parent)) {
          sp_fs_create_dir(parent);
        }
        sp_test_file_create_ex((sp_test_file_config_t) { .path = full });
        break;
      }
      case COLLECT_ENT_DIR: {
        sp_fs_create_dir(full);
        break;
      }
      case COLLECT_ENT_SYMLINK: {
        sp_str_t target = sp_fs_join_path(collect_path, sp_str_view(s->symlink_target));
        sp_fs_create_sym_link(target, full);
        break;
      }
    }
  }

  sp_da(sp_fs_entry_t) results = t->recursive
    ? sp_fs_collect_recursive(collect_path)
    : sp_fs_collect(collect_path);

  if (t->expect_null) {
    EXPECT_EQ(results, SP_NULLPTR);
    return;
  }

  u32 expected_count = collect_count_expected(t->expected);
  EXPECT_EQ(sp_da_size(results), expected_count);

  sp_for(i, expected_count) {
    collect_expected_ent_t* exp = &t->expected[i];
    sp_str_t expected_path = sp_fs_join_path(collect_path, sp_str_view(exp->name));
    bool found = false;
    sp_da_for(results, j) {
      if (sp_str_equal(results[j].file_path, expected_path) &&
          results[j].attributes == exp->attr) {
        found = true;
        break;
      }
    }
    if (!found) {
      UTEST_PRINTF("  missing: name=\"%s\" attr=%d\n", exp->name, exp->attr);
      EXPECT_TRUE(found);
    }
  }
}

UTEST_F(fs_collect, empty_dir) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "empty_dir",
  });
}

UTEST_F(fs_collect, nonexistent) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "nonexistent",
    .root_kind = COLLECT_ROOT_MISSING,
    .expect_null = true,
  });
}

UTEST_F(fs_collect, file_not_dir) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "file_not_dir",
    .root_kind = COLLECT_ROOT_FILE,
    .expect_null = true,
  });
}

UTEST_F(fs_collect, single_file) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "single_file",
    .setup = {
      { "hello.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "hello.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, multiple_files) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "multiple_files",
    .setup = {
      { "a.c", COLLECT_ENT_FILE },
      { "b.h", COLLECT_ENT_FILE },
      { "c.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "a.c", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "b.h", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "c.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, subdirectory) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "subdirectory",
    .setup = {
      { "child", COLLECT_ENT_DIR },
    },
    .expected = {
      { "child", SP_OS_FILE_ATTR_DIRECTORY },
    },
  });
}

UTEST_F(fs_collect, mixed_types) {
  SKIP_IF_NO_SYMLINKS();
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "mixed_types",
    .setup = {
      { "file.txt", COLLECT_ENT_FILE },
      { "subdir", COLLECT_ENT_DIR },
      { "link", COLLECT_ENT_SYMLINK, "file.txt" },
    },
    .expected = {
      { "file.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "subdir", SP_OS_FILE_ATTR_DIRECTORY },
      { "link", SP_OS_FILE_ATTR_SYMLINK },
    },
  });
}

UTEST_F(fs_collect, skips_dot_and_dotdot) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "skips_dot_and_dotdot",
    .setup = {
      { "real.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "real.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, hidden_file) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "hidden_file",
    .setup = {
      { ".gitignore", COLLECT_ENT_FILE },
      { "visible.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { ".gitignore", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "visible.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, does_not_recurse) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "does_not_recurse",
    .setup = {
      { "child", COLLECT_ENT_DIR },
      { "child/deep.txt", COLLECT_ENT_FILE },
      { "top.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "child", SP_OS_FILE_ATTR_DIRECTORY },
      { "top.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_empty_dir) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_empty_dir",
    .recursive = true,
  });
}

UTEST_F(fs_collect, recursive_flat_dir) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_flat_dir",
    .recursive = true,
    .setup = {
      { "a.txt", COLLECT_ENT_FILE },
      { "b.txt", COLLECT_ENT_FILE },
      { "c.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "a.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "b.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "c.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_nested_dirs) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_nested_dirs",
    .recursive = true,
    .setup = {
      { "sub", COLLECT_ENT_DIR },
      { "sub/deep", COLLECT_ENT_DIR },
      { "a.txt", COLLECT_ENT_FILE },
      { "sub/b.txt", COLLECT_ENT_FILE },
      { "sub/deep/c.txt", COLLECT_ENT_FILE },
    },
    .expected = {
      { "a.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "sub", SP_OS_FILE_ATTR_DIRECTORY },
      { "sub/b.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "sub/deep", SP_OS_FILE_ATTR_DIRECTORY },
      { "sub/deep/c.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs_collect, recursive_symlink_not_followed) {
  SKIP_IF_NO_SYMLINKS();
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_symlink_not_followed",
    .recursive = true,
    .setup = {
      { "real_dir", COLLECT_ENT_DIR },
      { "real_dir/file.txt", COLLECT_ENT_FILE },
      { "link", COLLECT_ENT_SYMLINK, "real_dir" },
    },
    .expected = {
      { "real_dir", SP_OS_FILE_ATTR_DIRECTORY },
      { "real_dir/file.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "link", SP_OS_FILE_ATTR_SYMLINK },
    },
  });
}

UTEST_F(fs_collect, recursive_nonexistent) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_nonexistent",
    .root_kind = COLLECT_ROOT_MISSING,
    .recursive = true,
    .expect_null = true,
  });
}

UTEST_F(fs_collect, recursive_file_not_dir) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "recursive_file_not_dir",
    .root_kind = COLLECT_ROOT_FILE,
    .recursive = true,
    .expect_null = true,
  });
}

////////////////
// CREATE DIR //
////////////////
typedef enum {
  CREATE_DIR_ENT_FILE,
  CREATE_DIR_ENT_DIR,
  CREATE_DIR_ENT_SYMLINK,
} create_dir_ent_kind_t;

typedef struct {
  const c8* path;
  create_dir_ent_kind_t kind;
  const c8* symlink_target;
} create_dir_setup_ent_t;

#define FS_EXPECT_EXIST true
#define FS_EXPECT_NOT_EXIST false

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_attr_t attr;
} create_dir_expected_ent_t;

typedef struct {
  const c8* label;
  const c8* target;
  create_dir_setup_ent_t setup[16];
  create_dir_expected_ent_t expected[16];
  bool expect_ok;
} create_dir_test_t;

struct fs_create_dir {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(fs_create_dir) {
  sp_test_file_manager_init(&ut.file_manager);
  probe_symlinks(ut.file_manager.paths.test);
}

UTEST_F_TEARDOWN(fs_create_dir) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

static void run_create_dir_test(s32* utest_result, sp_test_file_manager_t* fm, create_dir_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);

  u32 setup_count = 0;
  while (setup_count < 16 && t->setup[setup_count].path) setup_count++;

  sp_for(i, setup_count) {
    create_dir_setup_ent_t* s = &t->setup[i];
    sp_str_t full = sp_fs_join_path(sandbox, sp_str_view(s->path));
    sp_str_t parent = sp_fs_parent_path(full);
    if (!sp_str_empty(parent) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    switch (s->kind) {
      case CREATE_DIR_ENT_FILE: {
        sp_test_file_create_ex((sp_test_file_config_t) { .path = full });
        break;
      }
      case CREATE_DIR_ENT_DIR: {
        sp_fs_create_dir(full);
        break;
      }
      case CREATE_DIR_ENT_SYMLINK: {
        sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(s->symlink_target));
        ASSERT_EQ(sp_fs_create_sym_link(target, full), SP_OK);
        break;
      }
    }
  }

  sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(t->target));
  sp_err_t result = sp_fs_create_dir(target);

  if (t->expect_ok && result) {
    SP_TEST_REPORT("{} does not exist with code {}", SP_FMT_STR(target), SP_FMT_S32(result));
    SP_FAIL();
  } else if (!t->expect_ok && !result){
    SP_TEST_REPORT("{} exists with code {}", SP_FMT_STR(target), SP_FMT_S32(result));
    SP_FAIL();
  }

  u32 expected_count = 0;
  while (expected_count < 16 && t->expected[expected_count].path) expected_count++;

  sp_for(i, expected_count) {
    create_dir_expected_ent_t* exp = &t->expected[i];
    sp_str_t expected_path = sp_fs_join_path(sandbox, sp_str_view(exp->path));
    bool exists = sp_fs_exists(expected_path);
    if (exists != exp->exists) {
      if (exp->exists) {
        SP_TEST_REPORT("expected {} to exist", SP_FMT_STR(expected_path));
      } else {
        SP_TEST_REPORT("expected {} not to exist", SP_FMT_STR(expected_path));
      }
      SP_FAIL();
    }

    if (exp->exists) {
      sp_fs_attr_t attr = sp_fs_get_file_attrs(expected_path);
      if (attr != exp->attr) {
        SP_TEST_REPORT(
          "{} had attr {} but expected {}",
          SP_FMT_STR(expected_path),
          SP_FMT_S32(attr),
          SP_FMT_S32(exp->attr)
        );
        SP_FAIL();
      }
    }
  }
}

UTEST_F(fs_create_dir, existing_directory) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "existing_directory",
    .target = "dir1",
    .setup = {
      { .path = "dir1", .kind = CREATE_DIR_ENT_DIR },
    },
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, create_one_level) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "create_one_level",
    .target = "dir1",
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, create_multi_level) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "create_multi_level",
    .target = "dir1/dir2",
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "dir1/dir2", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, destination_is_file) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_file",
    .target = "file",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
    .expect_ok = false,
  });
}

UTEST_F(fs_create_dir, destination_parent_is_file) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_parent_is_file",
    .target = "file/dir1",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "file/dir1", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
    .expect_ok = false,
  });
}

#if defined(SP_POSIX)
UTEST_F(fs_create_dir, destination_is_symlink_to_directory) {
  SKIP_IF_NO_SYMLINKS();
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_symlink_to_directory",
    .target = "sym_name",
    .setup = {
      { .path = "dir", .kind = CREATE_DIR_ENT_DIR },
      { .path = "sym_name", .kind = CREATE_DIR_ENT_SYMLINK, .symlink_target = "dir" },
    },
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "sym_name", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
    .expect_ok = false,
  });
}

UTEST_F(fs_create_dir, destination_is_symlink_to_file) {
  SKIP_IF_NO_SYMLINKS();
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_symlink_to_file",
    .target = "sym_name",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
      { .path = "sym_name", .kind = CREATE_DIR_ENT_SYMLINK, .symlink_target = "file" },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "sym_name", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
    .expect_ok = false,
  });
}
#endif


/////////////
// HARNESS //
/////////////
typedef enum {
  FS_SETUP_FILE,
  FS_SETUP_DIR,
  FS_SETUP_SYMLINK,
  FS_SETUP_HARD_LINK,
} fs_setup_kind_t;

typedef struct {
  const c8* path;
  fs_setup_kind_t kind;
  union {
    const c8* target;
    const c8* content;
  };
} fs_setup_t;

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_attr_t attr;
} fs_expected_path_t;

static u32 fs_count_setup(fs_setup_t* setup) {
  u32 n = 0;
  while (n < 16 && setup[n].path) n++;
  return n;
}

static u32 fs_count_expected_paths(fs_expected_path_t* expected) {
  u32 n = 0;
  while (n < 16 && expected[n].path) n++;
  return n;
}

static void fs_expect_bool(s32* utest_result, sp_str_t path, const c8* label, bool actual, bool expected) {
  if (actual == expected) return;

  SP_TEST_REPORT(
    "{} {} was {} but expected {}",
    SP_FMT_CSTR(label),
    SP_FMT_STR(path),
    SP_FMT_CSTR(actual ? "true" : "false"),
    SP_FMT_CSTR(expected ? "true" : "false")
  );
  SP_FAIL();
}

static void fs_expect_attr(s32* utest_result, sp_str_t path, sp_fs_attr_t actual, sp_fs_attr_t expected) {
  if (actual == expected) return;

  SP_TEST_REPORT(
    "{} had attr {} but expected {}",
    SP_FMT_STR(path),
    SP_FMT_S32(actual),
    SP_FMT_S32(expected)
  );
  SP_FAIL();
}

static void fs_expect_paths(s32* utest_result, sp_str_t sandbox, fs_expected_path_t* expected) {
  u32 expected_count = fs_count_expected_paths(expected);
  sp_for(i, expected_count) {
    fs_expected_path_t* exp = &expected[i];
    sp_str_t path = sp_fs_join_path(sandbox, sp_str_view(exp->path));
    bool exists = sp_fs_exists(path);
    if (exists != exp->exists) {
      if (exp->exists) {
        SP_TEST_REPORT("expected {} to exist", SP_FMT_STR(path));
      } else {
        SP_TEST_REPORT("expected {} not to exist", SP_FMT_STR(path));
      }
      SP_FAIL();
    }

    if (exp->exists) {
      fs_expect_attr(utest_result, path, sp_fs_get_file_attrs(path), exp->attr);
    }
  }
}

static void fs_apply_setup(s32* utest_result, sp_test_file_manager_t* fm, sp_str_t sandbox, fs_setup_t* setup) {
  u32 setup_count = fs_count_setup(setup);
  sp_for(i, setup_count) {
    fs_setup_t* ent = &setup[i];
    sp_str_t path = sp_fs_join_path(sandbox, sp_str_view(ent->path));
    sp_str_t parent = sp_fs_parent_path(path);

    if (!sp_str_empty(parent) && !sp_str_equal(parent, path) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    switch (ent->kind) {
      case FS_SETUP_FILE: {
        sp_test_file_create_ex((sp_test_file_config_t) {
          .path = path,
          .content = ent->content ? sp_str_view(ent->content) : SP_LIT(""),
        });
        break;
      }
      case FS_SETUP_DIR: {
        sp_fs_create_dir(path);
        break;
      }
      case FS_SETUP_SYMLINK: {
        sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(ent->target));
        if (sp_fs_create_sym_link(target, path) != SP_OK) {
          SP_TEST_REPORT("failed to create symlink {} -> {}", SP_FMT_STR(path), SP_FMT_STR(target));
          SP_FAIL();
        }
        break;
      }
      case FS_SETUP_HARD_LINK: {
        sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(ent->target));
        if (sp_fs_create_hard_link(target, path) != SP_OK) {
          SP_TEST_REPORT("failed to create hard link {} -> {}", SP_FMT_STR(path), SP_FMT_STR(target));
          SP_FAIL();
        }
        break;
      }
    }
  }
}

////////////////
// PREDICATES //
////////////////
typedef struct {
  const c8* path;
  bool exists;
  bool is_regular_file;
  bool is_dir;
  bool is_symlink;
  bool is_target_regular_file;
  bool is_target_dir;
  sp_fs_attr_t attr;
} fs_predicate_expected_t;

typedef struct {
  const c8* path;
  const c8* normalized;
  const c8* trimmed;
  const c8* parent;
  const c8* name;
  const c8* stem;
  const c8* ext;
} fs_path_case_t;

typedef struct {
  const c8* a;
  const c8* b;
  const c8* joined;
} fs_join_case_t;

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
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  u32 expected_count = fs_count_predicates(t->expected);
  sp_for(i, expected_count) {
    fs_predicate_expected_t* exp = &t->expected[i];
    sp_str_t path = sp_fs_join_path(sandbox, sp_str_view(exp->path));

    fs_expect_bool(utest_result, path, "exists", sp_fs_exists(path), exp->exists);
    fs_expect_bool(utest_result, path, "is_regular_file", sp_fs_is_regular_file(path), exp->is_regular_file);
    fs_expect_bool(utest_result, path, "is_dir", sp_fs_is_dir(path), exp->is_dir);
    fs_expect_bool(utest_result, path, "is_symlink", sp_fs_is_symlink(path), exp->is_symlink);
    fs_expect_bool(utest_result, path, "is_target_regular_file", sp_fs_is_target_regular_file(path), exp->is_target_regular_file);
    fs_expect_bool(utest_result, path, "is_target_dir", sp_fs_is_target_dir(path), exp->is_target_dir);
    fs_expect_attr(utest_result, path, sp_fs_get_file_attrs(path), exp->attr);
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
        .is_dir = false,
        .is_symlink = false,
        .is_target_regular_file = true,
        .is_target_dir = false,
        .attr = SP_OS_FILE_ATTR_REGULAR_FILE,
      },
      {
        .path = "dir",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = false,
        .is_dir = true,
        .is_symlink = false,
        .is_target_regular_file = false,
        .is_target_dir = true,
        .attr = SP_OS_FILE_ATTR_DIRECTORY,
      },
      {
        .path = "file.link",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = false,
        .is_dir = false,
        .is_symlink = true,
        .is_target_regular_file = true,
        .is_target_dir = false,
        .attr = SP_OS_FILE_ATTR_SYMLINK,
      },
      {
        .path = "dir.link",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = false,
        .is_dir = false,
        .is_symlink = true,
        .is_target_regular_file = false,
        .is_target_dir = true,
        .attr = SP_OS_FILE_ATTR_SYMLINK,
      },
      {
        .path = "missing",
        .exists = FS_EXPECT_NOT_EXIST,
        .is_regular_file = false,
        .is_dir = false,
        .is_symlink = false,
        .is_target_regular_file = false,
        .is_target_dir = false,
        .attr = SP_OS_FILE_ATTR_NONE,
      },
    },
  });
}


///////////
// LINKS //
///////////
typedef enum {
  FS_LINK_ACTION_HARD,
  FS_LINK_ACTION_SYMLINK,
} fs_link_action_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  fs_link_action_t action;
  const c8* target;
  const c8* link_path;
  bool expect_ok;
  fs_expected_path_t expected[16];
} fs_link_test_t;

static void run_fs_link_test(s32* utest_result, sp_test_file_manager_t* fm, fs_link_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(t->target));
  sp_str_t link_path = sp_fs_join_path(sandbox, sp_str_view(t->link_path));
  sp_err_t result = SP_OK;

  switch (t->action) {
    case FS_LINK_ACTION_HARD: {
      result = sp_fs_create_hard_link(target, link_path);
      break;
    }
    case FS_LINK_ACTION_SYMLINK: {
      result = sp_fs_create_sym_link(target, link_path);
      break;
    }
  }

  fs_expect_bool(utest_result, link_path, "link_ok", result == SP_OK, t->expect_ok);
  fs_expect_paths(utest_result, sandbox, t->expected);
}

UTEST_F(fs, create_hard_link_file) {
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_hard_link_file",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .action = FS_LINK_ACTION_HARD,
    .target = "file.txt",
    .link_path = "file.hard",
    .expect_ok = true,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "file.hard", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });

  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("create_hard_link_file"));
  sp_str_t source = sp_fs_join_path(sandbox, SP_LIT("file.txt"));
  sp_str_t link = sp_fs_join_path(sandbox, SP_LIT("file.hard"));

  sp_io_writer_t writer = sp_io_writer_from_file(source, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&writer, SP_LIT("updated"));
  sp_io_writer_close(&writer);

  SP_EXPECT_STR_EQ(sp_io_read_file(link), SP_LIT("updated"));
}

UTEST_F(fs, create_hard_link_existing_destination_fails) {
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_hard_link_existing_destination_fails",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "dest.txt", .kind = FS_SETUP_FILE, .content = "bye" },
    },
    .action = FS_LINK_ACTION_HARD,
    .target = "file.txt",
    .link_path = "dest.txt",
    .expect_ok = false,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "dest.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST_F(fs, create_hard_link_directory_fails) {
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_hard_link_directory_fails",
    .setup = {
      { .path = "dir", .kind = FS_SETUP_DIR },
    },
    .action = FS_LINK_ACTION_HARD,
    .target = "dir",
    .link_path = "dir.hard",
    .expect_ok = false,
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "dir.hard", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
  });
}

UTEST_F(fs, create_symlink_file) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_symlink_file",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .action = FS_LINK_ACTION_SYMLINK,
    .target = "file.txt",
    .link_path = "file.link",
    .expect_ok = true,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "file.link", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
  });

  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("create_symlink_file"));
  sp_str_t link = sp_fs_join_path(sandbox, SP_LIT("file.link"));
  SP_EXPECT_STR_EQ(sp_io_read_file(link), SP_LIT("hello"));
}

UTEST_F(fs, create_symlink_directory) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_symlink_directory",
    .setup = {
      { .path = "dir", .kind = FS_SETUP_DIR },
    },
    .action = FS_LINK_ACTION_SYMLINK,
    .target = "dir",
    .link_path = "dir.link",
    .expect_ok = true,
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "dir.link", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
  });
}

UTEST_F(fs, create_symlink_existing_destination_fails) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_link_test(&ur, &ut.file_manager, &(fs_link_test_t) {
    .label = "create_symlink_existing_destination_fails",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "dest.txt", .kind = FS_SETUP_FILE, .content = "bye" },
    },
    .action = FS_LINK_ACTION_SYMLINK,
    .target = "file.txt",
    .link_path = "dest.txt",
    .expect_ok = false,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "dest.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}


////////////
// COPY    //
////////////
typedef enum {
  FS_COPY_ACTION_FILE,
  FS_COPY_ACTION_DIR,
  FS_COPY_ACTION_LINK_COPY,
} fs_copy_action_t;

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_attr_t attr;
  const c8* content;
} fs_copy_expected_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  fs_copy_action_t action;
  const c8* src;
  const c8* dst;
  fs_copy_expected_t expected[16];
} fs_copy_test_t;

static u32 fs_count_copy_expected(fs_copy_expected_t* expected) {
  u32 n = 0;
  while (n < 16 && expected[n].path) n++;
  return n;
}

static void run_fs_copy_test(s32* utest_result, sp_test_file_manager_t* fm, fs_copy_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t src = sp_fs_join_path(sandbox, sp_str_view(t->src));
  sp_str_t dst = sp_fs_join_path(sandbox, sp_str_view(t->dst));

  switch (t->action) {
    case FS_COPY_ACTION_FILE: {
      sp_fs_copy_file(src, dst);
      break;
    }
    case FS_COPY_ACTION_DIR: {
      sp_fs_copy(src, dst);
      break;
    }
    case FS_COPY_ACTION_LINK_COPY: {
      sp_fs_link(src, dst, SP_FS_LINK_COPY);
      break;
    }
  }

  u32 expected_count = fs_count_copy_expected(t->expected);
  sp_for(i, expected_count) {
    fs_copy_expected_t* exp = &t->expected[i];
    sp_str_t path = sp_fs_join_path(sandbox, sp_str_view(exp->path));
    bool exists = sp_fs_exists(path);
    fs_expect_bool(utest_result, path, "exists", exists, exp->exists);

    if (exp->exists) {
      fs_expect_attr(utest_result, path, sp_fs_get_file_attrs(path), exp->attr);
      if (exp->content) {
        sp_str_t actual = sp_io_read_file(path);
        SP_TEST_STREQ(actual, sp_str_view(exp->content), false);
      }
    }
  }
}

UTEST_F(fs, copy_file_via_link) {
  run_fs_copy_test(&ur, &ut.file_manager, &(fs_copy_test_t) {
    .label = "copy_file_via_link",
    .setup = {
      { .path = "source.txt", .kind = FS_SETUP_FILE, .content = "test content" },
    },
    .action = FS_COPY_ACTION_LINK_COPY,
    .src = "source.txt",
    .dst = "copy.txt",
    .expected = {
      { .path = "source.txt", .exists = true, .attr = SP_OS_FILE_ATTR_REGULAR_FILE, .content = "test content" },
      { .path = "copy.txt", .exists = true, .attr = SP_OS_FILE_ATTR_REGULAR_FILE, .content = "test content" },
    },
  });
}

UTEST_F(fs, copy_dir_with_nonalphanumeric) {
  run_fs_copy_test(&ur, &ut.file_manager, &(fs_copy_test_t) {
    .label = "copy_dir_with_nonalphanumeric",
    .setup = {
      { .path = "foo.bar", .kind = FS_SETUP_DIR },
      { .path = "baz", .kind = FS_SETUP_DIR },
    },
    .action = FS_COPY_ACTION_DIR,
    .src = "foo.bar",
    .dst = "baz",
    .expected = {
      { .path = "baz/foo.bar", .exists = true, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
  });
}

UTEST_F(fs, unicode_copy_file) {
  run_fs_copy_test(&ur, &ut.file_manager, &(fs_copy_test_t) {
    .label = "unicode_copy_file",
    .setup = {
      { .path = "\xc3\xb6riginal.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .action = FS_COPY_ACTION_FILE,
    .src = "\xc3\xb6riginal.txt",
    .dst = "\xc3\xbc\x63opy.txt",
    .expected = {
      { .path = "\xc3\xb6riginal.txt", .exists = true, .attr = SP_OS_FILE_ATTR_REGULAR_FILE, .content = "hello" },
      { .path = "\xc3\xbc\x63opy.txt", .exists = true, .attr = SP_OS_FILE_ATTR_REGULAR_FILE, .content = "hello" },
    },
  });
}

////////////////
// REMOVE DIR //
////////////////
typedef enum {
  FS_REMOVE_ACTION_FILE,
  FS_REMOVE_ACTION_DIR,
} fs_remove_action_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  fs_remove_action_t action;
  const c8* remove_path;
  fs_expected_path_t expected[16];
} fs_remove_test_t;

static void run_fs_remove_test(s32* utest_result, sp_test_file_manager_t* fm, fs_remove_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t remove_path = sp_fs_join_path(sandbox, sp_str_view(t->remove_path));
  switch (t->action) {
    case FS_REMOVE_ACTION_FILE: {
      sp_fs_remove_file(remove_path);
      break;
    }
    case FS_REMOVE_ACTION_DIR: {
      sp_fs_remove_dir(remove_path);
      break;
    }
  }

  fs_expect_paths(utest_result, sandbox, t->expected);
}

UTEST_F(fs, remove_file_basic) {
  run_fs_remove_test(&ur, &ut.file_manager, &(fs_remove_test_t) {
    .label = "remove_file_basic",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .action = FS_REMOVE_ACTION_FILE,
    .remove_path = "file.txt",
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
  });
}

UTEST_F(fs, remove_dir_recursive) {
  run_fs_remove_test(&ur, &ut.file_manager, &(fs_remove_test_t) {
    .label = "remove_dir_recursive",
    .setup = {
      { .path = "tree", .kind = FS_SETUP_DIR },
      { .path = "tree/file1.txt", .kind = FS_SETUP_FILE, .content = "a" },
      { .path = "tree/sub", .kind = FS_SETUP_DIR },
      { .path = "tree/sub/file2.txt", .kind = FS_SETUP_FILE, .content = "b" },
    },
    .action = FS_REMOVE_ACTION_DIR,
    .remove_path = "tree",
    .expected = {
      { .path = "tree", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "tree/file1.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "tree/sub", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "tree/sub/file2.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
  });
}

UTEST_F(fs, remove_dir_does_not_follow_symlink) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_remove_test(&ur, &ut.file_manager, &(fs_remove_test_t) {
    .label = "remove_dir_does_not_follow_symlink",
    .setup = {
      { .path = "outside.txt", .kind = FS_SETUP_FILE, .content = "outside" },
      { .path = "tree", .kind = FS_SETUP_DIR },
      { .path = "tree/link", .kind = FS_SETUP_SYMLINK, .target = "outside.txt" },
      { .path = "tree/file.txt", .kind = FS_SETUP_FILE, .content = "inside" },
    },
    .action = FS_REMOVE_ACTION_DIR,
    .remove_path = "tree",
    .expected = {
      { .path = "tree", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "tree/link", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "tree/file.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "outside.txt", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
  });
}

UTEST(fs_path, decomposition_reference) {
  fs_path_case_t cases[] = {
    {
      .path = "",
      .normalized = "",
      .trimmed = "",
      .parent = "",
      .name = "",
      .stem = "",
      .ext = "",
    },
    {
      .path = "foo",
      .normalized = "foo",
      .trimmed = "foo",
      .parent = "",
      .name = "foo",
      .stem = "foo",
      .ext = "",
    },
    {
      .path = "foo/",
      .normalized = "foo",
      .trimmed = "foo",
      .parent = "",
      .name = "",
      .stem = "foo/",
      .ext = "",
    },
    {
      .path = "foo/bar",
      .normalized = "foo/bar",
      .trimmed = "foo/bar",
      .parent = "foo",
      .name = "bar",
      .stem = "bar",
      .ext = "",
    },
    {
      .path = "foo/bar.txt",
      .normalized = "foo/bar.txt",
      .trimmed = "foo/bar.txt",
      .parent = "foo",
      .name = "bar.txt",
      .stem = "bar",
      .ext = "txt",
    },
    {
      .path = "foo..txt",
      .normalized = "foo..txt",
      .trimmed = "foo..txt",
      .parent = "",
      .name = "foo..txt",
      .stem = "foo.",
      .ext = "txt",
    },
    {
      .path = ".profile",
      .normalized = ".profile",
      .trimmed = ".profile",
      .parent = "",
      .name = ".profile",
      .stem = "",
      .ext = "profile",
    },
    {
      .path = ".profile.txt",
      .normalized = ".profile.txt",
      .trimmed = ".profile.txt",
      .parent = "",
      .name = ".profile.txt",
      .stem = ".profile",
      .ext = "txt",
    },
    {
      .path = "C:/foo/bar.txt",
      .normalized = "C:/foo/bar.txt",
      .trimmed = "C:/foo/bar.txt",
      .parent = "C:/foo",
      .name = "bar.txt",
      .stem = "bar",
      .ext = "txt",
    },
    {
      .path = "C:\\foo\\bar.txt",
      .normalized = "C:/foo/bar.txt",
      .trimmed = "C:\\foo\\bar.txt",
      .parent = "",
      .name = "C:\\foo\\bar.txt",
      .stem = "C:\\foo\\bar",
      .ext = "txt",
    },
  };

  SP_CARR_FOR(cases, i) {
    SP_EXPECT_STR_EQ_CSTR(sp_fs_normalize_path(sp_str_view(cases[i].path)), cases[i].normalized);
    SP_EXPECT_STR_EQ_CSTR(sp_fs_trim_path(sp_str_view(cases[i].path)),      cases[i].trimmed);
    SP_EXPECT_STR_EQ_CSTR(sp_fs_parent_path(sp_str_view(cases[i].path)),    cases[i].parent);
    SP_EXPECT_STR_EQ_CSTR(sp_fs_get_name(sp_str_view(cases[i].path)),       cases[i].name);
    SP_EXPECT_STR_EQ_CSTR(sp_fs_get_stem(sp_str_view(cases[i].path)),       cases[i].stem);
    SP_EXPECT_STR_EQ_CSTR(sp_fs_get_ext(sp_str_view(cases[i].path)),        cases[i].ext);
  }
}

UTEST(fs_path, join_reference) {
  fs_join_case_t cases[] = {
    {
      .a = "foo",
      .b = "bar",
      .joined = "foo/bar",
    },
    {
      .a = "foo/",
      .b = "bar",
      .joined = "foo/bar",
    },
    {
      .a = "foo",
      .b = "bar/baz",
      .joined = "foo/bar/baz",
    },
    {
      .a = "",
      .b = "bar",
      .joined = "bar",
    },
  };

  SP_CARR_FOR(cases, i) {
    SP_EXPECT_STR_EQ(
      sp_fs_join_path(sp_str_view(cases[i].a), sp_str_view(cases[i].b)),
      sp_str_view(cases[i].joined)
    );
  }
}

UTEST_F(fs, current_path_reference) {
  sp_str_t old_cwd = sp_fs_get_cwd();
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("current_path_reference"));
  sp_fs_create_dir(sandbox);

  ASSERT_EQ(sp_chdir(sp_str_to_cstr(sandbox)), 0);

  sp_str_t cwd = sp_fs_get_cwd();
  sp_str_t canonical_dot = sp_fs_canonicalize_path(SP_LIT("."));
  SP_EXPECT_STR_EQ(cwd, canonical_dot);
  ASSERT_TRUE(sp_fs_is_dir(cwd));

  ASSERT_EQ(sp_chdir(sp_str_to_cstr(old_cwd)), 0);
}

UTEST_F(fs, canonicalize_reference) {
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("canonicalize_reference"));
  sp_str_t dir = sp_fs_join_path(sandbox, SP_LIT("dir"));
  sp_str_t child = sp_fs_join_path(dir, SP_LIT("child"));
  sp_fs_create_dir(child);

  sp_str_t path = sp_fs_join_path(sandbox, SP_LIT("dir/./child/.."));
  sp_str_t canonical = sp_fs_canonicalize_path(path);
  sp_str_t expected = sp_fs_canonicalize_path(dir);

  SP_EXPECT_STR_EQ(canonical, expected);
}

UTEST_F(fs, mod_time_updates_after_write) {
  sp_str_t file = sp_test_file_path(&ut.file_manager, SP_LIT("mod_time_updates_after_write.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = file,
    .content = SP_LIT("a"),
  });

  sp_tm_epoch_t before = sp_fs_get_mod_time(file);
  sp_os_sleep_ms(100);

  sp_io_writer_t writer = sp_io_writer_from_file(file, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&writer, SP_LIT("b"));
  sp_io_writer_close(&writer);

  sp_tm_epoch_t after = sp_fs_get_mod_time(file);
  ASSERT_TRUE(after.s > before.s || (after.s == before.s && after.ns >= before.ns));
  ASSERT_TRUE(after.s != before.s || after.ns != before.ns);
}

///////////////
// UNICODE   //
///////////////

UTEST_F(fs, unicode_create_dir_and_file) {
  run_fs_predicate_test(&ur, &ut.file_manager, &(fs_predicate_test_t) {
    .label = "unicode_create_dir_and_file",
    .setup = {
      { .path = "\xc3\xb1\x61\x6d\x65", .kind = FS_SETUP_DIR },
      { .path = "\xc3\xbc\x6e\x69\x63\x6f\x64\x65.txt", .kind = FS_SETUP_FILE },
    },
    .expected = {
      {
        .path = "\xc3\xb1\x61\x6d\x65",
        .exists = true,
        .is_dir = true,
        .is_target_dir = true,
        .attr = SP_OS_FILE_ATTR_DIRECTORY,
      },
      {
        .path = "\xc3\xbc\x6e\x69\x63\x6f\x64\x65.txt",
        .exists = true,
        .is_regular_file = true,
        .is_target_regular_file = true,
        .attr = SP_OS_FILE_ATTR_REGULAR_FILE,
      },
    },
  });
}

UTEST_F(fs, unicode_remove_file) {
  run_fs_remove_test(&ur, &ut.file_manager, &(fs_remove_test_t) {
    .label = "unicode_remove_file",
    .setup = {
      { .path = "\xc3\xb6\x70\x65\x6e.txt", .kind = FS_SETUP_FILE },
    },
    .action = FS_REMOVE_ACTION_FILE,
    .remove_path = "\xc3\xb6\x70\x65\x6e.txt",
    .expected = {
      { .path = "\xc3\xb6\x70\x65\x6e.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
  });
}

UTEST_F(fs, unicode_remove_dir) {
  run_fs_remove_test(&ur, &ut.file_manager, &(fs_remove_test_t) {
    .label = "unicode_remove_dir",
    .setup = {
      { .path = "\xc3\xa4\x62\x63", .kind = FS_SETUP_DIR },
      { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt", .kind = FS_SETUP_FILE, .content = "data" },
    },
    .action = FS_REMOVE_ACTION_DIR,
    .remove_path = "\xc3\xa4\x62\x63",
    .expected = {
      { .path = "\xc3\xa4\x62\x63", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
      { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
  });
}

UTEST_F(fs_collect, unicode_entries) {
  run_collect_test(&ur, &ut.file_manager, &(collect_test_t) {
    .label = "unicode_entries",
    .setup = {
      { "\xc3\xb1\x61\x6d\x65.txt", COLLECT_ENT_FILE },
      { "\xc3\xbc\x6e\x69", COLLECT_ENT_DIR },
    },
    .expected = {
      { "\xc3\xb1\x61\x6d\x65.txt", SP_OS_FILE_ATTR_REGULAR_FILE },
      { "\xc3\xbc\x6e\x69", SP_OS_FILE_ATTR_DIRECTORY },
    },
  });
}

UTEST_F(fs, unicode_canonicalize) {
  sp_str_t root = sp_test_file_path(&ut.file_manager, SP_LIT("unicode_canonicalize"));
  sp_str_t dir = sp_fs_join_path(root, SP_LIT("\xc3\xa9t\xc3\xa9"));
  sp_fs_create_dir(dir);

  sp_str_t canonical = sp_fs_canonicalize_path(dir);
  ASSERT_GT(canonical.len, 0);
  ASSERT_TRUE(sp_fs_exists(canonical));
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
        .is_dir = false,
        .is_symlink = false,
        .is_target_regular_file = true,
        .is_target_dir = false,
        .attr = SP_OS_FILE_ATTR_REGULAR_FILE,
      },
      {
        .path = "\xc3\xb1\x61\x6d\x65",
        .exists = FS_EXPECT_EXIST,
        .is_regular_file = false,
        .is_dir = true,
        .is_symlink = false,
        .is_target_regular_file = false,
        .is_target_dir = true,
        .attr = SP_OS_FILE_ATTR_DIRECTORY,
      },
      {
        .path = "missing\xc3\xa9",
        .exists = FS_EXPECT_NOT_EXIST,
        .is_regular_file = false,
        .is_dir = false,
        .is_symlink = false,
        .is_target_regular_file = false,
        .is_target_dir = false,
        .attr = SP_OS_FILE_ATTR_NONE,
      },
    },
  });
}

SP_TEST_MAIN()
