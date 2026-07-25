#include "test.h"
#include "utest.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_tty)

UTEST_F(sys_tty, pipe_is_not_a_tty) {
  sp_sys_fd_t read_end = SP_SYS_INVALID_FD;
  sp_sys_fd_t write_end = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_pipe(&read_end, &write_end), SP_OK);

  sp_sys_tty_attr_t attr = sp_zero;
  EXPECT_FALSE(sp_sys_is_tty(read_end));
  EXPECT_EQ(sp_sys_tty_get(read_end, &attr), SP_ERR_SYS_NOT_TTY);
  EXPECT_EQ(sp_sys_tty_set(read_end, &attr), SP_ERR_SYS_NOT_TTY);

  sp_sys_close(read_end);
  sp_sys_close(write_end);
}

UTEST_F(sys_tty, size_of_pipe_fails_and_zeroes) {
  sp_sys_fd_t read_end = SP_SYS_INVALID_FD;
  sp_sys_fd_t write_end = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_pipe(&read_end, &write_end), SP_OK);

  u32 cols = 1234;
  u32 rows = 5678;
  EXPECT_NE(sp_sys_tty_size(read_end, &cols, &rows), SP_OK);
  EXPECT_EQ(cols, (u32)0);
  EXPECT_EQ(rows, (u32)0);

  sp_sys_close(read_end);
  sp_sys_close(write_end);
}

UTEST_F(sys_tty, enter_raw_on_pipe_fails_and_leaves_nothing_dirty) {
  sp_sys_fd_t read_end = SP_SYS_INVALID_FD;
  sp_sys_fd_t write_end = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_pipe(&read_end, &write_end), SP_OK);

  sp_sys_tty_mode_t saved = sp_zero;
  EXPECT_NE(sp_sys_tty_enter_raw(read_end, write_end, &saved), SP_OK);
  EXPECT_FALSE(saved.in_dirty);
  EXPECT_FALSE(saved.out_dirty);
  EXPECT_EQ(sp_sys_tty_restore(read_end, write_end, &saved), SP_OK);

  sp_sys_close(read_end);
  sp_sys_close(write_end);
}

UTEST_F(sys_tty, bad_fd_is_reported) {
  sp_sys_tty_attr_t attr = sp_zero;
  EXPECT_NE(sp_sys_tty_get(SP_SYS_INVALID_FD, &attr), SP_OK);
  EXPECT_FALSE(sp_sys_is_tty(SP_SYS_INVALID_FD));
}

UTEST_F(sys_tty, raw_in_clears_echo_and_canonical_mode) {
  sp_sys_tty_attr_t attr = sp_zero;
#if !defined(SP_WIN32)
  attr.c_lflag = (u32)(SP_ECHO | SP_ICANON | SP_ISIG | SP_IEXTEN);
#endif
  EXPECT_TRUE(sp_sys_tty_raw_in(&attr));
#if !defined(SP_WIN32)
  EXPECT_EQ(attr.c_lflag & (u32)(SP_ECHO | SP_ICANON | SP_ISIG | SP_IEXTEN), (u32)0);
  EXPECT_EQ(attr.c_cc[SP_VMIN], (u8)1);
  EXPECT_EQ(attr.c_cc[SP_VTIME], (u8)0);
#endif
}

UTEST_F(sys_tty, round_trip_preserves_attributes) {
  if (!sp_sys_is_tty(sp_sys_stdin)) return;

  sp_sys_tty_attr_t before = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &before), SP_OK);

  sp_sys_tty_mode_t saved = sp_zero;
  ASSERT_EQ(sp_sys_tty_enter_raw(sp_sys_stdin, sp_sys_stdout, &saved), SP_OK);
  EXPECT_TRUE(saved.in_dirty);

  sp_sys_tty_attr_t raw = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &raw), SP_OK);
  EXPECT_NE(sp_sys_memcmp(&raw, &before, sizeof(before)), 0);

  ASSERT_EQ(sp_sys_tty_restore(sp_sys_stdin, sp_sys_stdout, &saved), SP_OK);

  sp_sys_tty_attr_t after = sp_zero;
  ASSERT_EQ(sp_sys_tty_get(sp_sys_stdin, &after), SP_OK);
  EXPECT_EQ(sp_sys_memcmp(&after, &before, sizeof(before)), 0);
}

#endif
