#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"
#include <sys/stat.h>

struct sp_test_fs {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(sp_test_fs) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(sp_test_fs) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(sp_test_fs, link) {
  // Create a test file with content
  sp_str_t source_file = sp_test_file_create_empty(&ut.file_manager, SP_LIT("source.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = source_file,
    .content = SP_LIT("Hello, World! This is test content for linking."),
  });

  // Test hard link
  sp_str_t hard_link = sp_test_file_path(&ut.file_manager, SP_LIT("hard_link.txt"));
  ASSERT_EQ(sp_fs_link(source_file, hard_link, SP_FS_LINK_HARD), SP_ERR_OK);

  // Verify hard link exists and has same content
  ASSERT_TRUE(sp_fs_exists(hard_link));
  ASSERT_TRUE(sp_fs_is_regular_file(hard_link));

  sp_str_t hard_content = sp_io_read_file(hard_link);
  SP_EXPECT_STR_EQ(hard_content, SP_LIT("Hello, World! This is test content for linking."));

  // Test symbolic link
  sp_str_t sym_link = sp_test_file_path(&ut.file_manager, SP_LIT("sym_link.txt"));
  ASSERT_EQ(sp_fs_link(source_file, sym_link, SP_FS_LINK_SYMBOLIC), SP_ERR_OK);

  // Verify symbolic link exists
  ASSERT_TRUE(sp_fs_exists(sym_link));

  sp_str_t sym_content = sp_io_read_file(sym_link);
  SP_EXPECT_STR_EQ(sym_content, SP_LIT("Hello, World! This is test content for linking."));

  // Test copy functionality using sp_os_link with COPY kind
  sp_str_t copy_file = sp_test_file_path(&ut.file_manager, SP_LIT("copy.txt"));
  ASSERT_EQ(sp_fs_link(source_file, copy_file, SP_FS_LINK_COPY), SP_ERR_OK);

  // Verify copy exists and has same content
  ASSERT_TRUE(sp_fs_exists(copy_file));
  ASSERT_TRUE(sp_fs_is_regular_file(copy_file));

  sp_str_t copy_content = sp_io_read_file(copy_file);
  SP_EXPECT_STR_EQ(copy_content, SP_LIT("Hello, World! This is test content for linking."));

  // Test that modifying original affects hard link but not copy
  // Modify the original file by writing to it directly
  sp_io_stream_t stream = sp_io_from_file(source_file, SP_IO_MODE_WRITE);
  sp_io_write_str(&stream, SP_LIT("Modified content"));
  sp_io_close(&stream);

  sp_str_t modified_hard_content = sp_io_read_file(hard_link);
  SP_EXPECT_STR_EQ(modified_hard_content, SP_LIT("Modified content"));

  sp_str_t unchanged_copy_content = sp_io_read_file(copy_file);
  SP_EXPECT_STR_EQ(unchanged_copy_content, SP_LIT("Hello, World! This is test content for linking."));
}

UTEST_F(sp_test_fs, link_error_cases) {
  // Test that copy works correctly for files
  sp_str_t source_file = sp_test_file_create_empty(&ut.file_manager, SP_LIT("source2.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = source_file,
    .content = SP_LIT("Another test file"),
  });

  sp_str_t copy_file = sp_test_file_path(&ut.file_manager, SP_LIT("copy2.txt"));
  ASSERT_EQ(sp_fs_link(source_file, copy_file, SP_FS_LINK_COPY), SP_ERR_OK);

  ASSERT_TRUE(sp_fs_exists(copy_file));
  ASSERT_TRUE(sp_fs_is_regular_file(copy_file));

  sp_str_t copy_content = sp_io_read_file(copy_file);
  SP_EXPECT_STR_EQ(copy_content, SP_LIT("Another test file"));
}

UTEST_F(sp_test_fs, symlink_semantics) {
  // Create a regular file and a directory
  sp_str_t file = sp_test_file_create_empty(&ut.file_manager, SP_LIT("file.real"));
  sp_str_t content = sp_str_lit("im a damn file");
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = file,
    .content = content,
  });

  sp_str_t dir = sp_test_file_path(&ut.file_manager, SP_LIT("dir.real"));
  sp_fs_create_dir(dir);

  // Create symlinks to both
  sp_str_t file_link = sp_test_file_path(&ut.file_manager, SP_LIT("file.link"));
  ASSERT_EQ(sp_fs_create_sym_link(file, file_link), SP_ERR_OK);

  sp_str_t dir_link = sp_test_file_path(&ut.file_manager, SP_LIT("dir.link"));
  ASSERT_EQ(sp_fs_create_sym_link(dir, dir_link), SP_ERR_OK);

  // regular files and directories report as what they are
  ASSERT_TRUE(sp_fs_is_regular_file(file));
  ASSERT_FALSE(sp_fs_is_dir(file));
  ASSERT_FALSE(sp_fs_is_symlink(file));

  ASSERT_FALSE(sp_fs_is_regular_file(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));
  ASSERT_FALSE(sp_fs_is_symlink(dir));

  ASSERT_EQ(sp_fs_get_file_attrs(file),    SP_OS_FILE_ATTR_REGULAR_FILE);
  ASSERT_EQ(sp_fs_get_file_attrs(dir), SP_OS_FILE_ATTR_DIRECTORY);

  // symlinks report as symlinks
  ASSERT_FALSE(sp_fs_is_regular_file(file_link));
  ASSERT_FALSE(sp_fs_is_dir(file_link));
  ASSERT_TRUE(sp_fs_is_symlink(file_link));

  ASSERT_FALSE(sp_fs_is_regular_file(dir_link));
  ASSERT_FALSE(sp_fs_is_dir(dir_link));
  ASSERT_TRUE(sp_fs_is_symlink(dir_link));

  ASSERT_EQ(sp_fs_get_file_attrs(file_link), SP_OS_FILE_ATTR_SYMLINK);
  ASSERT_EQ(sp_fs_get_file_attrs(dir_link),  SP_OS_FILE_ATTR_SYMLINK);

  // if you'd like to follow the symlink, use sp_os_is_target_*() instead of sp_os_is_*()
  ASSERT_TRUE(sp_fs_is_target_regular_file(file_link));
  ASSERT_FALSE(sp_fs_is_target_dir(file_link));

  ASSERT_TRUE(sp_fs_is_target_dir(dir_link));
  ASSERT_FALSE(sp_fs_is_dir(file_link));

  // sp_io will follow symlinks, since not doing so seems quite surprising
  sp_str_t content_link = sp_io_read_file(file_link);
  SP_EXPECT_STR_EQ(content_link, content);
}

UTEST_F(sp_test_fs, copy_dir_with_nonalphanumeric) {
  sp_str_t foo_bar_dir = sp_test_file_path(&ut.file_manager, SP_LIT("foo.bar"));
  sp_fs_create_dir(foo_bar_dir);
  ASSERT_TRUE(sp_fs_exists(foo_bar_dir));
  ASSERT_TRUE(sp_fs_is_dir(foo_bar_dir));

  sp_str_t baz_dir = sp_test_file_path(&ut.file_manager, SP_LIT("baz"));
  sp_fs_create_dir(baz_dir);
  ASSERT_TRUE(sp_fs_exists(baz_dir));
  ASSERT_TRUE(sp_fs_is_dir(baz_dir));

  sp_fs_copy(foo_bar_dir, baz_dir);

  sp_str_t expected_path = sp_test_file_path(&ut.file_manager, SP_LIT("baz/foo.bar"));
  ASSERT_TRUE(sp_fs_exists(expected_path));
}

UTEST_F(sp_test_fs, copy_preserves_file_attributes) {
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
  ASSERT_EQ(sp_fs_copy(source_file, copy_file), SP_ERR_OK);

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

typedef struct {
  sp_str_t file_path;
  sp_str_t stem;
} sp_test_file_stem_case_t;

UTEST(path_functions, path_stem) {
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

UTEST(path_functions, normalize_path) {
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

UTEST(path_functions, parent_path) {
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
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("Test");
    sp_str_t parent = sp_fs_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t parent = sp_fs_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/");
    sp_str_t parent = sp_fs_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/home/user/file");
    sp_str_t parent = sp_fs_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "/home/user");
  }
}

UTEST(path_functions, canonicalize_path) {
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

UTEST(path_functions, path_extension) {
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


UTEST(path_functions, extract_file_name) {
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
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
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
    sp_str_t path = SP_LIT("/home/user/document.pdf");
    sp_str_t filename = sp_fs_get_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "document.pdf");
  }
}

UTEST(path_functions, get_executable_path) {
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

UTEST(path_functions, integration_test) {
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
  sp_str_t dll_path = sp_str_builder_write(&builder);

  bool has_double_slash = false;
  for (u32 i = 1; i < dll_path.len; i++) {
    if (dll_path.data[i-1] == '/' && dll_path.data[i] == '/') {
      has_double_slash = true;
      break;
    }
  }
  ASSERT_FALSE(has_double_slash);
}


UTEST_MAIN()
