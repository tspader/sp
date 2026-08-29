#include "term.h"

typedef enum {
  TERM_FD_INVALID,
  TERM_FD_PIPE_READ,
  TERM_FD_PIPE_WRITE,
} term_fd_kind_t;

typedef struct {
  bool supports_ansi;
} term_supports_expect_t;

typedef struct {
  term_fd_kind_t fd;
  term_supports_expect_t expect;
} term_supports_test_t;

static void run_term_supports(s32* utest_result, term_supports_test_t t) {
  sp_sys_pipe_t pipe = sp_zero;
  ASSERT_EQ(sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)), SP_OK);

  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  switch (t.fd) {
    case TERM_FD_INVALID:    fd = SP_SYS_INVALID_FD; break;
    case TERM_FD_PIPE_READ:  fd = pipe.r; break;
    case TERM_FD_PIPE_WRITE: fd = pipe.w; break;
  }

  EXPECT_EQ(sp_tty_supports_ansi(fd), t.expect.supports_ansi);

  sp_sys_close(pipe.r);
  sp_sys_close(pipe.w);
}

UTEST(term_supports, supports_ansi) {
  SKIP_ON_FREESTANDING();
  SKIP_ON_WASM();
  term_supports_test_t cases[] = {
    { .fd = TERM_FD_INVALID },
    { .fd = TERM_FD_PIPE_READ },
    { .fd = TERM_FD_PIPE_WRITE },
  };
  SP_CARR_FOR(cases, i) run_term_supports(utest_result, cases[i]);
}
