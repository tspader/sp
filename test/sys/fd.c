#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  OP_READ,
  OP_WRITE,
  OP_OPEN_DIR,
} op_t;

typedef struct {
  const c8* name;
  op_t op;
} test_t;

static const test_t tests [] = {
  { .name = "read_refuses_garbage_fd", .op = OP_READ },
  { .name = "write_refuses_garbage_fd", .op = OP_WRITE },
  { .name = "open_dir_refuses_garbage_root", .op = OP_OPEN_DIR },
};

// Invalid by construction, not closed-then-reused: past any fd rlimit on
// POSIX, not a multiple of 4 on NT. No recycling race.
#define GARBAGE_FD ((sp_sys_fd_t)0x55555)

static sp_err_t run(sp_test_t* t, test_t* c) {
  c8 buf [4] = sp_zero;
  u64 n = 0;
  sp_err_t err = SP_OK;

  switch (c->op) {
    case OP_READ:  err = sp_sys_read(GARBAGE_FD, buf, sizeof(buf), &n); break;
    case OP_WRITE: err = sp_sys_write(GARBAGE_FD, "A", 1, &n); break;
    case OP_OPEN_DIR: {
      sp_sys_fd_t out = SP_SYS_INVALID_FD;
      err = sp_sys_open_dir_s(GARBAGE_FD, sp_str_lit("A"), &out);
      if (!err) sp_sys_close(out);
      break;
    }
  }

  sp_expect_err_eq(t, err, SP_ERR_SYS_BAD_FD);
  return SP_OK;
}

sp_test_each_fn(sys, fd, test_t, tests, run);
