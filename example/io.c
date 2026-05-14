#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  sp_mem_t mem = sp_mem_os_new();

  sp_io_file_reader_t r = sp_zero;

  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(mem, u8, 64),
    .capacity = 64
  };
  sp_str_t exe = sp_fs_get_exe_path(mem);

  // sp_io provides utilities for opening a file from a path
  sp_io_file_reader_from_path(&r, exe);
  sp_io_read(&r.base, buffer.data, buffer.capacity, &buffer.len);
  sp_log("sp_io_file_reader_from_path: {}", sp_fmt_str(sp_mem_buffer_as_str(&buffer)));

  sp_io_file_reader_close(&r);
  sp_mem_zero(buffer.data, buffer.capacity);

  // Or, if you already have a file descriptor, wrap it. sp_io_file_t is just an alias
  // for the platform native file handle integer.
  //
  // sp_io will close file descriptors for you automatically if you pass SP_IO_CLOSE_MODE_AUTO
  sp_sys_fd_t fd = sp_sys_open_s(exe, SP_O_RDONLY | SP_O_BINARY, 0);
  sp_io_file_reader_from_file(&r, (sp_io_file_t)fd, SP_IO_CLOSE_MODE_AUTO);
  sp_io_read(&r.base, buffer.data, buffer.capacity, &buffer.len);
  sp_log("sp_io_file_reader_from_file: {}", sp_fmt_str(sp_mem_buffer_as_str(&buffer)));

  sp_io_file_reader_close(&r);

  // sp_io_writer_t is the basis for the string builder, and therefore format strings. This
  // means that any byte buffer can be trivially formatted to
  c8 str [256] = sp_zero;
  sp_io_mem_writer_t mw = sp_zero;
  sp_io_mem_writer_from_buffer(&mw, str, 256);
  {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch();
    sp_fmt_io(&mw.base, s.mem, "hello, {}", sp_fmt_cstr("world"));
    sp_mem_end_scratch(s);
  }
  sp_log("sp_io_writer_from_mem: {}", sp_fmt_cstr(str));

  sp_mem_zero(str, 256);

  // You can also format directly to stdout
  sp_io_file_writer_t fw = sp_zero;
  sp_io_file_writer_from_fd(&fw, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
  {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch();
    sp_fmt_io(&fw.base, s.mem, "hello, {.cyan}", sp_fmt_cstr("stdout"));
    sp_mem_end_scratch(s);
  }
  sp_io_write(&fw.base, "\n", 1, SP_NULLPTR);
  sp_io_file_writer_close(&fw);

  // When a reader is drained, sp_io_read returns SP_ERR_IO_EOF with
  // bytes_read == 0. Any successful call, even a short one, returns SP_OK;
  // the next call after the stream is exhausted is the one that signals EOF.
  sp_io_file_reader_from_path(&r, exe);
  u64 total = 0;
  while (true) {
    u64 bytes_read = 0;
    sp_err_t err = sp_io_read(&r.base, buffer.data, buffer.capacity, &bytes_read);
    total += bytes_read;
    if (err == SP_ERR_IO_EOF) break;
    if (err != SP_OK) {
      sp_log("sp_io_read failed: {}", sp_fmt_uint((u32)err));
      break;
    }
  }
  sp_log("sp_io EOF: drained {} bytes", sp_fmt_uint(total));
  sp_io_file_reader_close(&r);

  return 0;
}
SP_MAIN(run)
