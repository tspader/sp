#include "fs.h"

UTEST_F(fs, mod_time_nonzero) {
  sp_str_t file = sp_test_file_path(&ut.file_manager, SP_LIT("mod_time.file"));
  sp_str_t dir = sp_test_file_path(&ut.file_manager, SP_LIT("mod_time.dir"));
  sp_fs_create_file(file);
  sp_fs_create_dir(dir);

  EXPECT_TRUE(sp_fs_get_mod_time(file).s > 0);
  EXPECT_TRUE(sp_fs_get_mod_time(dir).s > 0);
}

UTEST_F(fs, mod_time_updates_after_write) {
  sp_str_t file = sp_test_file_path(&ut.file_manager, SP_LIT("mod_time_write.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = file,
    .content = SP_LIT("a"),
  });

  sp_tm_epoch_t before = sp_fs_get_mod_time(file);
  sp_os_sleep_ms(100);

  sp_io_writer_t writer = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&writer, file, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&writer, SP_LIT("b"), SP_NULLPTR);
  sp_io_writer_close(&writer);

  sp_tm_epoch_t after = sp_fs_get_mod_time(file);
  ASSERT_TRUE(after.s > before.s || (after.s == before.s && after.ns > before.ns));
}

SP_TEST_MAIN()
