#include "test.h"
#include "utest.h"

#if !defined(SP_WASM)

typedef enum {
  TTY_FD_PIPE_READ,
  TTY_FD_PIPE_WRITE,
  TTY_FD_INVALID,
} tty_fd_t;

typedef struct {
  sp_err_t get;
} tty_expect_t;

typedef struct {
  tty_fd_t fd;
  tty_expect_t expect;
} tty_test_t;

typedef struct {
  bool changes_attributes;
} tty_mode_expect_t;

typedef struct {
  sp_sys_tty_mode_t mode;
  tty_mode_expect_t expect;
} tty_mode_test_t;

UTEST_EMPTY_FIXTURE(sys_tty)

void run_tty_test(s32* utest_result, tty_test_t t) {
  sp_sys_fd_t read_end = SP_SYS_INVALID_FD;
  sp_sys_fd_t write_end = SP_SYS_INVALID_FD;
  sp_sys_fd_t fd = SP_SYS_INVALID_FD;

  if (t.fd != TTY_FD_INVALID) {
    sp_sys_pipe_t p = sp_zero;
    ASSERT_EQ(sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)), SP_OK);
    read_end = p.r;
    write_end = p.w;
    fd = t.fd == TTY_FD_PIPE_READ ? read_end : write_end;
  }

  EXPECT_FALSE(sp_sys_is_tty(fd));

  sp_sys_tty_attr_t attr = sp_zero;
  EXPECT_EQ(sp_sys_tty_get(fd, &attr), t.expect.get);
  EXPECT_FALSE(attr.present);

  EXPECT_EQ(sp_sys_tty_set(fd, &attr), SP_OK);

  sp_sys_tty_attr_t out = sp_zero;
  EXPECT_EQ(sp_sys_tty_mode_apply(&attr, &out, SP_SYS_TTY_MODE_RAW), SP_ERR_SYS_NOT_TTY);

  u32 cols = 1234;
  u32 rows = 5678;
  EXPECT_NE(sp_sys_tty_size(fd, &cols, &rows), SP_OK);
  EXPECT_EQ(cols, (u32)0);
  EXPECT_EQ(rows, (u32)0);

  sp_sys_tty_state_t saved = sp_zero;
  EXPECT_NE(sp_tty_set_mode(fd, fd, SP_SYS_TTY_MODE_RAW, &saved), SP_OK);
  EXPECT_FALSE(saved.in.present);
  EXPECT_FALSE(saved.out.present);
  EXPECT_EQ(sp_tty_restore(fd, fd, &saved), SP_OK);

  if (t.fd != TTY_FD_INVALID) {
    sp_sys_close(read_end);
    sp_sys_close(write_end);
  }
}

void run_tty_mode_test(s32* utest_result, tty_mode_test_t t) {
  if (!sp_sys_is_tty(sp_sys_stdin)) return;

  sp_sys_tty_attr_t before = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &before), SP_OK);
  ASSERT_TRUE(before.present);

  sp_sys_tty_state_t saved = sp_zero;
  ASSERT_EQ(sp_tty_set_mode(sp_sys_stdin, sp_sys_stdout, t.mode, &saved), SP_OK);
  EXPECT_TRUE(saved.in.present);
  EXPECT_EQ(sp_sys_memcmp(saved.in.opaque, before.opaque, sizeof(before.opaque)), 0);

  sp_sys_tty_attr_t during = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &during), SP_OK);
  if (t.expect.changes_attributes) {
    EXPECT_NE(sp_sys_memcmp(during.opaque, before.opaque, sizeof(before.opaque)), 0);
  }

  ASSERT_EQ(sp_tty_restore(sp_sys_stdin, sp_sys_stdout, &saved), SP_OK);

  sp_sys_tty_attr_t after = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &after), SP_OK);
  EXPECT_EQ(sp_sys_memcmp(after.opaque, before.opaque, sizeof(before.opaque)), 0);
}

UTEST_F(sys_tty, pipe_read_end_is_not_a_tty) {
  run_tty_test(&ur, (tty_test_t) {
    .fd = TTY_FD_PIPE_READ,
    .expect = {
      .get = SP_ERR_SYS_NOT_TTY,
    },
  });
}

UTEST_F(sys_tty, pipe_write_end_is_not_a_tty) {
  run_tty_test(&ur, (tty_test_t) {
    .fd = TTY_FD_PIPE_WRITE,
    .expect = {
      .get = SP_ERR_SYS_NOT_TTY,
    },
  });
}

UTEST_F(sys_tty, invalid_fd_is_not_a_tty) {
  run_tty_test(&ur, (tty_test_t) {
    .fd = TTY_FD_INVALID,
    .expect = {
      .get = SP_ERR_SYS_BAD_FD,
    },
  });
}

UTEST_F(sys_tty, raw_round_trips) {
  run_tty_mode_test(&ur, (tty_mode_test_t) {
    .mode = SP_SYS_TTY_MODE_RAW,
    .expect = {
      .changes_attributes = true,
    },
  });
}

UTEST_F(sys_tty, no_echo_round_trips) {
  run_tty_mode_test(&ur, (tty_mode_test_t) {
    .mode = SP_SYS_TTY_MODE_NO_ECHO,
    .expect = {
      .changes_attributes = true,
    },
  });
}

UTEST_F(sys_tty, cooked_round_trips) {
  run_tty_mode_test(&ur, (tty_mode_test_t) {
    .mode = SP_SYS_TTY_MODE_COOKED,
  });
}

#endif
