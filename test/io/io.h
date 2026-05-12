#ifndef IO_TEST_H
#define IO_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"


struct io {
  sp_str_t file_path;
  sp_test_file_manager_t file_manager;
  sp_mem_arena_t* arena;
  sp_mem_t mem;
};

UTEST_F_SETUP(io) {
  SKIP_ON_WASM()
  ut.arena = sp_mem_arena_new(sp_mem_os_new());
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  sp_test_file_manager_init(&ut.file_manager);
  ut.file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io_rw.file"));
}

UTEST_F_TEARDOWN(io) {
  SKIP_ON_WASM()
  sp_test_file_manager_cleanup(&ut.file_manager);
  sp_mem_arena_destroy(ut.arena);
}


//////////////////////
// SHARED MATRIX    //
//////////////////////
#define IO_MAX_STEPS 8
#define IO_MAX_RESPONSES 8

typedef enum {
  IO_STEP_NONE,
  IO_STEP_READ,
  IO_STEP_WRITE,
  IO_STEP_FLUSH,
  IO_STEP_SEEK,
  IO_STEP_SIZE,
  IO_STEP_COPY,
} io_step_kind_t;

typedef struct {
  io_step_kind_t kind;
  union {
    struct { u64 request; sp_err_t err; const c8* content; } read;
    struct { const c8* data; sp_err_t err; u64 bytes; } write;
    struct { sp_err_t err; } flush;
    struct { s64 offset; sp_io_whence_t whence; sp_err_t err; s64 pos; } seek;
    struct { sp_err_t err; u64 size; } size;
    struct { u64 buffer; sp_err_t err; u64 copied; } copy;
  };
} io_step_t;

typedef struct {
  u64 bytes;
  sp_err_t err;
  const c8* data;
} io_result_t;

typedef struct {
  sp_io_reader_t base;
  io_result_t results [IO_MAX_RESPONSES];
  u64 num_results;
  u64 cursor;
} io_mock_reader_t;

typedef struct {
  sp_io_writer_t base;
  io_result_t results [IO_MAX_RESPONSES];
  u64 num_results;
  u64 cursor;
  u8 received [256];
  u64 received_len;
} io_mock_writer_t;

u64      io_get_num_results(const io_result_t* responses, u64 max);
sp_err_t io_mock_reader_read(sp_io_reader_t* r, void* ptr, u64 size, u64* bytes_read);
sp_err_t io_mock_writer_write(sp_io_writer_t* w, const void* ptr, u64 size, u64* bytes_written);
void     io_mock_reader_init(io_mock_reader_t* m, const io_result_t* responses, u64 n);
void     io_mock_writer_init(io_mock_writer_t* m, const io_result_t* responses, u64 n);


#endif // IO_TEST_H
