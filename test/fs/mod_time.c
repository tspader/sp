#include "fs.h"

sp_test(fs, mod_time_nonzero) {
  sp_sys_fd_t sandbox = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sp_test_dir(t), 0, &sandbox));

  sp_path_t file = { .dir = sandbox, .sub = sp_str_lit("A") };
  sp_path_t dir = { .dir = sandbox, .sub = sp_str_lit("B") };
  sp_fs_create_file_at(file);
  sp_fs_create_dir_at(dir);

  sp_expect(t, sp_fs_get_mod_time_at(file).s > 0);
  sp_expect(t, sp_fs_get_mod_time_at(dir).s > 0);
  sp_sys_close(sandbox);
  return SP_OK;
}

sp_test(fs, mod_time_updates_after_write) {
  sp_sys_fd_t sandbox = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sp_test_dir(t), 0, &sandbox));

  sp_path_t file = { sandbox, sp_str_lit("A") };
  sp_fs_create_file_str_at(file, sp_str_lit("A"));

  sp_tm_epoch_t before = sp_fs_get_mod_time_at(file);
  sp_os_sleep_ms(100);

  sp_io_file_writer_t writer = sp_zero;
  sp_io_file_writer_from_path_at(&writer, file);
  sp_io_write_str(&writer.base, sp_str_lit("B"), SP_NULLPTR);
  sp_io_file_writer_close(&writer);

  sp_tm_epoch_t after = sp_fs_get_mod_time_at(file);
  sp_sys_close(sandbox);
  sp_must(t, after.s > before.s || (after.s == before.s && after.ns > before.ns));
  return SP_OK;
}
