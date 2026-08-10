#include "sys.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_transfer)

typedef enum {
  SYS_TRANSFER_END_FILE,
  SYS_TRANSFER_END_PIPE,
} sys_transfer_end_t;

typedef struct {
  sp_err_t err;
  u64 moved;
  u64 in_pos;
  const c8* content;
} sys_transfer_expect_t;

typedef struct {
  const c8* label;
  sys_transfer_end_t src;
  sys_transfer_end_t dst;
  const c8* data;
  bool positional;
  bool track_in;
  sys_transfer_expect_t expect;
} sys_transfer_test_t;

static void run_sys_transfer_test(s32* utest_result, sys_transfer_test_t t) {
#if !defined(SP_LINUX)
  t.expect = (sys_transfer_expect_t) { .err = SP_ERR_SYS_UNSUPPORTED };
#endif

  sp_sys_fd_t in = SP_SYS_INVALID_FD;
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  sp_sys_fd_t src_write = SP_SYS_INVALID_FD;
  sp_sys_fd_t dst_read = SP_SYS_INVALID_FD;
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  u64 len = sp_cstr_len(t.data);

  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t sandbox = sp_test_file_create_dir(&fm, t.label);
  if (sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd)) {
    SP_TEST_REPORT("failed to open sandbox {}", sp_fmt_str(sandbox));
    SP_FAIL();
    goto done;
  }

  switch (t.src) {
    case SYS_TRANSFER_END_FILE: {
      sp_test_file_create_ex((sp_test_file_config_t) {
        .path = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("src.bin")),
        .content = sp_cstr_as_str(t.data),
      });
      if (sp_sys_open_s(sandbox_fd, sp_str_lit("src.bin"), SP_SYS_OPEN_MODE_RO, 0, &in) != SP_OK) {
        SP_TEST_REPORT("failed to open src.bin");
        SP_FAIL();
        goto done;
      }
      break;
    }
    case SYS_TRANSFER_END_PIPE: {
      sp_sys_pipe_t p = sp_zero;
      if (sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)) != SP_OK) {
        SP_TEST_REPORT("failed to create source pipe");
        SP_FAIL();
        goto done;
      }
      in = p.r;
      src_write = p.w;
      u64 written = 0;
      if (sp_sys_write(src_write, t.data, len, &written) != SP_OK || written != len) {
        SP_TEST_REPORT("failed to fill source pipe");
        SP_FAIL();
        goto done;
      }
      break;
    }
  }

  switch (t.dst) {
    case SYS_TRANSFER_END_FILE: {
      if (sp_sys_open_s(sandbox_fd, sp_str_lit("dst.bin"), SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_CREATE, &out) != SP_OK) {
        SP_TEST_REPORT("failed to open dst.bin");
        SP_FAIL();
        goto done;
      }
      break;
    }
    case SYS_TRANSFER_END_PIPE: {
      sp_sys_pipe_t p = sp_zero;
      if (sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)) != SP_OK) {
        SP_TEST_REPORT("failed to create destination pipe");
        SP_FAIL();
        goto done;
      }
      dst_read = p.r;
      out = p.w;
      break;
    }
  }

  {
    u64 in_pos = 0;
    u64 moved = 0;
    sp_err_t err;
    if (t.positional) {
      err = sp_sys_transfer_positional(in, t.track_in ? &in_pos : SP_NULLPTR, out, len, 0, &moved);
    }
    else {
      err = sp_sys_transfer(in, t.track_in ? &in_pos : SP_NULLPTR, out, len, &moved);
    }
    sys_expect_err(utest_result, "transfer", err, t.expect.err);
    EXPECT_EQ(moved, t.expect.moved);
    if (t.track_in) EXPECT_EQ(in_pos, t.expect.in_pos);
  }

  if (t.expect.content) {
    switch (t.dst) {
      case SYS_TRANSFER_END_FILE: {
        sp_sys_close(out);
        out = SP_SYS_INVALID_FD;
        sp_str_t actual = sp_zero;
        sp_io_read_file(fm.mem, sp_fs_join_path(fm.mem, sandbox, sp_str_lit("dst.bin")), &actual);
        sys_expect_bytes(utest_result, "transfer", actual.data, (s64)actual.len, t.expect.content);
        break;
      }
      case SYS_TRANSFER_END_PIPE: {
        c8 buf [SYS_TEST_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_sys_read(dst_read, buf, sizeof(buf), &n);
        sys_expect_bytes(utest_result, "transfer", buf, (s64)n, t.expect.content);
        break;
      }
    }
  }

done:
  if (in != SP_SYS_INVALID_FD) sp_sys_close(in);
  if (out != SP_SYS_INVALID_FD) sp_sys_close(out);
  if (src_write != SP_SYS_INVALID_FD) sp_sys_close(src_write);
  if (dst_read != SP_SYS_INVALID_FD) sp_sys_close(dst_read);
  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_transfer, copies_file_to_file) {
  run_sys_transfer_test(utest_result, (sys_transfer_test_t) {
    .label = "sys_transfer_copies_file_to_file",
    .data = "0123456789ABCDEF",
    .positional = true,
    .track_in = true,
    .expect = {
      .moved = 16,
      .in_pos = 16,
      .content = "0123456789ABCDEF",
    },
  });
}

UTEST_F(sys_transfer, streams_file_to_pipe) {
  run_sys_transfer_test(utest_result, (sys_transfer_test_t) {
    .label = "sys_transfer_streams_file_to_pipe",
    .dst = SYS_TRANSFER_END_PIPE,
    .data = "0123456789ABCDEF",
    .track_in = true,
    .expect = {
      .moved = 16,
      .in_pos = 16,
      .content = "0123456789ABCDEF",
    },
  });
}

UTEST_F(sys_transfer, positional_pipe_source_unsupported) {
  run_sys_transfer_test(utest_result, (sys_transfer_test_t) {
    .label = "sys_transfer_positional_pipe_source_unsupported",
    .src = SYS_TRANSFER_END_PIPE,
    .data = "xx",
    .positional = true,
    .expect = {
      .err = SP_ERR_SYS_UNSUPPORTED,
    },
  });
}

UTEST_F(sys_transfer, streaming_pipe_source_unsupported) {
  run_sys_transfer_test(utest_result, (sys_transfer_test_t) {
    .label = "sys_transfer_streaming_pipe_source_unsupported",
    .src = SYS_TRANSFER_END_PIPE,
    .dst = SYS_TRANSFER_END_PIPE,
    .data = "xx",
    .expect = {
      .err = SP_ERR_SYS_UNSUPPORTED,
    },
  });
}

#if defined(SP_LINUX)
UTEST_F(sys_transfer, reports_zero_moved_at_eof) {
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t sandbox = sp_test_file_create_dir(&fm, "sys_transfer_reports_zero_moved_at_eof");
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd), SP_OK);

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("src.bin")),
    .content = sp_str_lit("0123"),
  });

  sp_sys_fd_t in = SP_SYS_INVALID_FD;
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_open_s(sandbox_fd, sp_str_lit("src.bin"), SP_SYS_OPEN_MODE_RO, 0, &in), SP_OK);
  ASSERT_EQ(sp_sys_open_s(sandbox_fd, sp_str_lit("dst.bin"), SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_CREATE, &out), SP_OK);

  u64 in_pos = 0;
  u64 out_pos = 0;
  u64 moved = 0;
  EXPECT_EQ(sp_sys_transfer_positional(in, &in_pos, out, 64, out_pos, &moved), SP_OK);
  EXPECT_EQ(moved, (u64)4);
  out_pos += moved;

  moved = 99;
  EXPECT_EQ(sp_sys_transfer_positional(in, &in_pos, out, 64, out_pos, &moved), SP_OK);
  EXPECT_EQ(moved, (u64)0);

  sp_sys_close(in);
  sp_sys_close(out);
  sp_sys_close(sandbox_fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_transfer, transfer_positional_writes_at_offset) {
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t sandbox = sp_test_file_create_dir(&fm, "sys_transfer_transfer_positional_writes_at_offset");
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd), SP_OK);

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("src.bin")),
    .content = sp_str_lit("AB"),
  });
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("dst.bin")),
    .content = sp_str_lit("...."),
  });

  sp_sys_fd_t in = SP_SYS_INVALID_FD;
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_open_s(sandbox_fd, sp_str_lit("src.bin"), SP_SYS_OPEN_MODE_RO, 0, &in), SP_OK);
  ASSERT_EQ(sp_sys_open_s(sandbox_fd, sp_str_lit("dst.bin"), SP_SYS_OPEN_MODE_WO, 0, &out), SP_OK);

  u64 in_pos = 0;
  u64 moved = 0;
  EXPECT_EQ(sp_sys_transfer_positional(in, &in_pos, out, 64, 2, &moved), SP_OK);
  EXPECT_EQ(moved, (u64)2);
  EXPECT_EQ(in_pos, (u64)2);

  sp_sys_close(out);
  out = SP_SYS_INVALID_FD;
  sp_str_t actual = sp_zero;
  sp_io_read_file(fm.mem, sp_fs_join_path(fm.mem, sandbox, sp_str_lit("dst.bin")), &actual);
  sys_expect_bytes(utest_result, "transfer_positional", actual.data, (s64)actual.len, "..AB");

  sp_sys_close(in);
  sp_sys_close(sandbox_fd);
  sp_test_file_manager_cleanup(&fm);
}
#endif

#endif
