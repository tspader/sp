#include "fs.h"

sp_test(fs, mod_time_nonzero) {
  sp_str_t file = fs_path(t, sp_str_lit("A"));
  sp_str_t dir = fs_path(t, sp_str_lit("B"));
  sp_fs_create_file(file);
  sp_fs_create_dir(dir);

  sp_expect(t, sp_fs_get_mod_time(file).s > 0);
  sp_expect(t, sp_fs_get_mod_time(dir).s > 0);
  return SP_OK;
}

sp_test(fs, mod_time_updates_after_write) {
  sp_str_t file = fs_path(t, sp_str_lit("A"));
  sp_fs_create_file_str(file, sp_str_lit("A"));

  sp_tm_epoch_t before = sp_fs_get_mod_time(file);
  sp_os_sleep_ms(100);

  sp_io_file_writer_t writer = sp_zero;
  sp_io_file_writer_from_path(&writer, file);
  sp_io_write_str(&writer.base, sp_str_lit("B"), SP_NULLPTR);
  sp_io_file_writer_close(&writer);

  sp_tm_epoch_t after = sp_fs_get_mod_time(file);
  sp_must(t, after.s > before.s || (after.s == before.s && after.ns > before.ns));
  return SP_OK;
}
