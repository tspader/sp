#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  struct {
    sp_io_reader_t r;
    sp_io_writer_t w;
  } io = sp_zero();

  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, 64),
    .capacity = 64
  };
  sp_str_t exe = sp_fs_get_exe_path();

  // sp_io provides utilities for opening a file from a path
  sp_io_reader_from_file(&io.r, exe);
  sp_io_read(&io.r, buffer.data, buffer.capacity, &buffer.len);
  sp_log("sp_io_reader_from_file: {}", sp_fmt_str(sp_mem_buffer_as_str(&buffer)));

  sp_io_reader_close(&io.r);
  sp_mem_zero(buffer.data, buffer.capacity);

  // Or, if you already have a file descriptor, from that. sp_sys_fd_t is just a platform
  // appropriate type for a native file handle from the OS (s32, HANDLE)
  //
  // sp_io will close file descriptors for you automatically if you pass SP_IO_CLOSE_MODE_AUTO
  sp_sys_fd_t fd = sp_sys_open(sp_str_to_cstr(exe), SP_O_RDONLY | SP_O_BINARY, 0);
  sp_io_reader_from_fd(&io.r, fd, SP_IO_CLOSE_MODE_AUTO);
  sp_io_read(&io.r, buffer.data, buffer.capacity, &buffer.len);
  sp_log("sp_io_reader_from_fd: {}", sp_fmt_str(sp_mem_buffer_as_str(&buffer)));

  sp_io_reader_close(&io.r);

  // sp_io_writer_t is the basis for the string builder, and therefore format strings. This
  // means that any byte buffer can be trivially formatted to
  c8 str [256] = sp_zero();
  sp_io_writer_from_mem(&io.w, str, 256);
  sp_fmt_io(&io.w, "hello, {}", sp_fmt_cstr("world"));
  sp_log("sp_io_writer_from_mem: {}", sp_fmt_cstr(str));

  sp_io_writer_close(&io.w);
  sp_mem_zero(str, 256);

  // You can also format directly to stdout
  sp_io_writer_from_fd(&io.w, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
  sp_fmt_io(&io.w, "hello, {.cyan}", sp_fmt_cstr("stdout"));
  sp_io_write(&io.w, "\n", 1, SP_NULLPTR);
  return 0;
}
SP_MAIN(run)
