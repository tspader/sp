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
  ASSERT_EQ(sp_os_link(source_file, hard_link, SP_OS_LINK_HARD), SP_ERR_OK);

  // Verify hard link exists and has same content
  ASSERT_TRUE(sp_os_does_path_exist(hard_link));
  ASSERT_TRUE(sp_os_is_regular_file(hard_link));

  sp_str_t hard_content = sp_io_read_file(hard_link);
  SP_EXPECT_STR_EQ(hard_content, SP_LIT("Hello, World! This is test content for linking."));

  // Test symbolic link
  sp_str_t sym_link = sp_test_file_path(&ut.file_manager, SP_LIT("sym_link.txt"));
  ASSERT_EQ(sp_os_link(source_file, sym_link, SP_OS_LINK_SYMBOLIC), SP_ERR_OK);

  // Verify symbolic link exists
  ASSERT_TRUE(sp_os_does_path_exist(sym_link));

  sp_str_t sym_content = sp_io_read_file(sym_link);
  SP_EXPECT_STR_EQ(sym_content, SP_LIT("Hello, World! This is test content for linking."));

  // Test copy functionality using sp_os_link with COPY kind
  sp_str_t copy_file = sp_test_file_path(&ut.file_manager, SP_LIT("copy.txt"));
  ASSERT_EQ(sp_os_link(source_file, copy_file, SP_OS_LINK_COPY), SP_ERR_OK);

  // Verify copy exists and has same content
  ASSERT_TRUE(sp_os_does_path_exist(copy_file));
  ASSERT_TRUE(sp_os_is_regular_file(copy_file));

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
  ASSERT_EQ(sp_os_link(source_file, copy_file, SP_OS_LINK_COPY), SP_ERR_OK);

  ASSERT_TRUE(sp_os_does_path_exist(copy_file));
  ASSERT_TRUE(sp_os_is_regular_file(copy_file));

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
  sp_os_create_directory(dir);

  // Create symlinks to both
  sp_str_t file_link = sp_test_file_path(&ut.file_manager, SP_LIT("file.link"));
  ASSERT_EQ(sp_os_create_symbolic_link(file, file_link), SP_ERR_OK);

  sp_str_t dir_link = sp_test_file_path(&ut.file_manager, SP_LIT("dir.link"));
  ASSERT_EQ(sp_os_create_symbolic_link(dir, dir_link), SP_ERR_OK);

  // regular files and directories report as what they are
  ASSERT_TRUE(sp_os_is_regular_file(file));
  ASSERT_FALSE(sp_os_is_directory(file));
  ASSERT_FALSE(sp_os_is_symlink(file));

  ASSERT_FALSE(sp_os_is_regular_file(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));
  ASSERT_FALSE(sp_os_is_symlink(dir));

  ASSERT_EQ(sp_os_get_file_attrs(file),    SP_OS_FILE_ATTR_REGULAR_FILE);
  ASSERT_EQ(sp_os_get_file_attrs(dir), SP_OS_FILE_ATTR_DIRECTORY);

  // symlinks report as symlinks
  ASSERT_FALSE(sp_os_is_regular_file(file_link));
  ASSERT_FALSE(sp_os_is_directory(file_link));
  ASSERT_TRUE(sp_os_is_symlink(file_link));

  ASSERT_FALSE(sp_os_is_regular_file(dir_link));
  ASSERT_FALSE(sp_os_is_directory(dir_link));
  ASSERT_TRUE(sp_os_is_symlink(dir_link));

  ASSERT_EQ(sp_os_get_file_attrs(file_link), SP_OS_FILE_ATTR_SYMLINK);
  ASSERT_EQ(sp_os_get_file_attrs(dir_link),  SP_OS_FILE_ATTR_SYMLINK);

  // if you'd like to follow the symlink, use sp_os_is_target_*() instead of sp_os_is_*()
  ASSERT_TRUE(sp_os_is_target_regular_file(file_link));
  ASSERT_FALSE(sp_os_is_target_directory(file_link));

  ASSERT_TRUE(sp_os_is_target_directory(dir_link));
  ASSERT_FALSE(sp_os_is_directory(file_link));

  // sp_io will follow symlinks, since not doing so seems quite surprising
  sp_str_t content_link = sp_io_read_file(file_link);
  SP_EXPECT_STR_EQ(content_link, content);
}

UTEST_F(sp_test_fs, copy_dir_with_nonalphanumeric) {
  sp_str_t foo_bar_dir = sp_test_file_path(&ut.file_manager, SP_LIT("foo.bar"));
  sp_os_create_directory(foo_bar_dir);
  ASSERT_TRUE(sp_os_does_path_exist(foo_bar_dir));
  ASSERT_TRUE(sp_os_is_directory(foo_bar_dir));

  sp_str_t baz_dir = sp_test_file_path(&ut.file_manager, SP_LIT("baz"));
  sp_os_create_directory(baz_dir);
  ASSERT_TRUE(sp_os_does_path_exist(baz_dir));
  ASSERT_TRUE(sp_os_is_directory(baz_dir));

  sp_os_copy(foo_bar_dir, baz_dir);

  sp_str_t expected_path = sp_test_file_path(&ut.file_manager, SP_LIT("baz/foo.bar"));
  ASSERT_TRUE(sp_os_does_path_exist(expected_path));
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
  ASSERT_EQ(sp_os_copy(source_file, copy_file), SP_ERR_OK);

  // Verify copy exists and has same type
  ASSERT_TRUE(sp_os_does_path_exist(copy_file));
  ASSERT_TRUE(sp_os_is_regular_file(copy_file));
  
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

UTEST_MAIN()
