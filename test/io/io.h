#ifndef IO_TEST_H
#define IO_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"


//////////////////
// io FIXTURE   //
//////////////////
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

typedef enum {
  IO_STEP_NONE,
  IO_STEP_READ,
  IO_STEP_WRITE,
  IO_STEP_FLUSH,
} io_step_kind_t;

typedef struct {
  io_step_kind_t kind;
  union {
    struct { u64 request; sp_err_t err; const c8* content; } read;
    struct { const c8* data; sp_err_t err; u64 bytes; } write;
    struct { sp_err_t err; } flush;
  };
} io_step_t;

#endif // IO_TEST_H
