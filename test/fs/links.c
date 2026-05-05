#include "fs.h"

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  bool symlink;
  const c8* target;
  const c8* link_path;
  bool expect_ok;
  fs_expected_path_t expected[16];
} link_test_t;

static void run_link_test(s32* utest_result, sp_test_file_manager_t* fm, link_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir_a(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t target = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(t->target));
  sp_str_t link_path = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(t->link_path));

  sp_err_t result = t->symlink
    ? sp_fs_create_sym_link_a(target, link_path)
    : sp_fs_create_hard_link_a(target, link_path);

  fs_expect_bool(utest_result, link_path, "link_ok", result == SP_OK, t->expect_ok);
  fs_expect_paths(utest_result, fm, sandbox, t->expected);
}

UTEST_F(fs, create_hard_link_file) {
  sp_mem_t a = ut.file_manager.mem;
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_hard_link_file",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .target = "file.txt",
    .link_path = "file.hard",
    .expect_ok = true,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
      { .path = "file.hard", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });

  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("create_hard_link_file"));
  sp_str_t source = sp_fs_join_path_a(a, sandbox, SP_LIT("file.txt"));
  sp_str_t link = sp_fs_join_path_a(a, sandbox, SP_LIT("file.hard"));

  sp_io_writer_t writer = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file_a(&writer, source, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&writer, SP_LIT("updated"), SP_NULLPTR);
  sp_io_writer_close(&writer);
  sp_str_t link_content = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(a, link, &link_content);
  SP_EXPECT_STR_EQ(link_content, SP_LIT("updated"));
}

UTEST_F(fs, create_hard_link_existing_destination_fails) {
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_hard_link_existing_destination_fails",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "dest.txt", .kind = FS_SETUP_FILE, .content = "bye" },
    },
    .target = "file.txt",
    .link_path = "dest.txt",
    .expect_ok = false,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
      { .path = "dest.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, create_hard_link_directory_fails) {
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_hard_link_directory_fails",
    .setup = {
      { .path = "dir", .kind = FS_SETUP_DIR },
    },
    .target = "dir",
    .link_path = "dir.hard",
    .expect_ok = false,
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_DIR },
      { .path = "dir.hard", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
    },
  });
}

UTEST_F(fs, create_symlink_file) {
  SKIP_IF_NO_SYMLINKS();
  sp_mem_t a = ut.file_manager.mem;
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_symlink_file",
    .symlink = true,
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .target = "file.txt",
    .link_path = "file.link",
    .expect_ok = true,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
      { .path = "file.link", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_SYMLINK },
    },
  });

  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("create_symlink_file"));
  sp_str_t link = sp_fs_join_path_a(a, sandbox, SP_LIT("file.link"));
  sp_str_t symlink_content = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(a, link, &symlink_content);
  SP_EXPECT_STR_EQ(symlink_content, SP_LIT("hello"));
}

UTEST_F(fs, create_symlink_directory) {
  SKIP_IF_NO_SYMLINKS();
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_symlink_directory",
    .symlink = true,
    .setup = {
      { .path = "dir", .kind = FS_SETUP_DIR },
    },
    .target = "dir",
    .link_path = "dir.link",
    .expect_ok = true,
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_DIR },
      { .path = "dir.link", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_SYMLINK },
    },
  });
}

UTEST_F(fs, create_symlink_existing_destination_fails) {
  SKIP_IF_NO_SYMLINKS();
  run_link_test(&ur, &ut.file_manager, &(link_test_t) {
    .label = "create_symlink_existing_destination_fails",
    .symlink = true,
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "dest.txt", .kind = FS_SETUP_FILE, .content = "bye" },
    },
    .target = "file.txt",
    .link_path = "dest.txt",
    .expect_ok = false,
    .expected = {
      { .path = "file.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
      { .path = "dest.txt", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

// canonicalize through a symlink should resolve to the real target
UTEST_F(fs, canonicalize_through_symlink) {
  SKIP_IF_NO_SYMLINKS();
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("canon_through_symlink"));
  sp_fs_create_dir_a(sandbox);

  sp_str_t real = sp_fs_join_path_a(a, sandbox, SP_LIT("real.txt"));
  sp_str_t link = sp_fs_join_path_a(a, sandbox, SP_LIT("link.txt"));

  sp_test_file_create_ex((sp_test_file_config_t) { .path = real, .content = SP_LIT("data") });
  ASSERT_EQ(sp_fs_create_sym_link_a(real, link), SP_OK);

  sp_str_t canon_link = sp_fs_canonicalize_path_a(a, link);
  sp_str_t canon_real = sp_fs_canonicalize_path_a(a, real);
  SP_EXPECT_STR_EQ(canon_link, canon_real);
}

SP_TEST_MAIN()
