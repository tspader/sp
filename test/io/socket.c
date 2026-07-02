#include "io.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(io_socket)

static bool io_socket_pair(sp_sys_fd_t out[2]) {
#if defined(SP_WIN32)
  static bool wsa_init = false;
  if (!wsa_init) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    wsa_init = true;
  }

  SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener == INVALID_SOCKET) return false;

  struct sockaddr_in addr = sp_zero;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;
  int addr_len = sizeof(addr);
  if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) != 0 ||
      getsockname(listener, (struct sockaddr*)&addr, &addr_len) != 0 ||
      listen(listener, 1) != 0) {
    closesocket(listener);
    return false;
  }

  SOCKET a = socket(AF_INET, SOCK_STREAM, 0);
  if (a == INVALID_SOCKET || connect(a, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    if (a != INVALID_SOCKET) closesocket(a);
    closesocket(listener);
    return false;
  }
  SOCKET b = accept(listener, SP_NULLPTR, SP_NULLPTR);
  closesocket(listener);
  if (b == INVALID_SOCKET) {
    closesocket(a);
    return false;
  }
  out[0] = (sp_sys_fd_t)a;
  out[1] = (sp_sys_fd_t)b;
  return true;
#elif defined(SP_LINUX)
  s32 fds[2] = sp_zero;
  if (sp_syscall(SP_SYSCALL_NUM_SOCKETPAIR, 1 /*AF_UNIX*/, 1 /*SOCK_STREAM*/, 0, fds, 0, 0) != 0) return false;
  out[0] = fds[0];
  out[1] = fds[1];
  return true;
#elif defined(SP_MACOS)
  int fds[2] = sp_zero;
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) return false;
  out[0] = fds[0];
  out[1] = fds[1];
  return true;
#else
  (void)out;
  return false;
#endif
}

static void io_socket_close(sp_sys_fd_t socket) {
#if defined(SP_WIN32)
  closesocket((SOCKET)socket);
#else
  sp_sys_close(socket);
#endif
}

UTEST_F(io_socket, roundtrip) {
  sp_sys_fd_t pair[2];
  ASSERT_TRUE(io_socket_pair(pair));

  sp_io_socket_writer_t writer = sp_zero;
  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_writer_init(&writer, pair[0], 1000);
  sp_io_socket_reader_init(&reader, pair[1], 1000);

  EXPECT_EQ(sp_io_write_all(&writer.base, "hello world", 11, SP_NULLPTR), SP_OK);

  u8 dest[32] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&reader.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)11);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)dest, bytes), sp_str_lit("hello world")));

  io_socket_close(pair[0]);
  io_socket_close(pair[1]);
}

UTEST_F(io_socket, eof_on_peer_close) {
  sp_sys_fd_t pair[2];
  ASSERT_TRUE(io_socket_pair(pair));

  sp_io_socket_writer_t writer = sp_zero;
  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_writer_init(&writer, pair[0], 1000);
  sp_io_socket_reader_init(&reader, pair[1], 1000);

  EXPECT_EQ(sp_io_write_all(&writer.base, "x", 1, SP_NULLPTR), SP_OK);
  io_socket_close(pair[0]);

  u8 dest[8] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&reader.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)1);
  EXPECT_EQ(sp_io_read(&reader.base, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);

  io_socket_close(pair[1]);
}

UTEST_F(io_socket, read_timeout) {
  sp_sys_fd_t pair[2];
  ASSERT_TRUE(io_socket_pair(pair));

  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_reader_init(&reader, pair[1], 50);

  u8 dest[8] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&reader.base, dest, sizeof(dest), &bytes), SP_ERR_IO_TIMEOUT);
  EXPECT_EQ(bytes, (u64)0);

  io_socket_close(pair[0]);
  io_socket_close(pair[1]);
}

// The HTTP shape: scan a buffered socket reader up to the header terminator,
// then drain a fixed-length body through a limit reader. Bytes past the
// terminator must survive in the reader's buffer.
UTEST_F(io_socket, read_until_then_limit) {
  sp_sys_fd_t pair[2];
  ASSERT_TRUE(io_socket_pair(pair));

  sp_io_socket_writer_t writer = sp_zero;
  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_writer_init(&writer, pair[0], 1000);
  sp_io_socket_reader_init(&reader, pair[1], 1000);

  const c8* message = "HTTP/1.1 200 OK\r\n\r\nhello";
  EXPECT_EQ(sp_io_write_all(&writer.base, message, sp_cstr_len(message), SP_NULLPTR), SP_OK);

  u8 reader_buf[64] = sp_zero;
  sp_io_reader_set_buffer(&reader.base, reader_buf, sizeof(reader_buf));

  u8 head_buf[64] = sp_zero;
  sp_io_mem_writer_t head = sp_zero;
  sp_io_mem_writer_from_buffer(&head, head_buf, sizeof(head_buf));

  u64 head_bytes = 0;
  EXPECT_EQ(sp_io_read_until(&reader.base, sp_str_lit("\r\n\r\n"), &head.base, sizeof(head_buf), &head_bytes), SP_OK);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)head_buf, head_bytes), sp_str_lit("HTTP/1.1 200 OK\r\n\r\n")));

  sp_io_limit_reader_t body = sp_zero;
  sp_io_limit_reader_init(&body, &reader.base, 5);

  u8 body_buf[16] = sp_zero;
  sp_io_mem_writer_t body_out = sp_zero;
  sp_io_mem_writer_from_buffer(&body_out, body_buf, sizeof(body_buf));

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&body_out.base, &body.base, &copied), SP_OK);
  EXPECT_EQ(copied, (u64)5);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)body_buf, 5), sp_str_lit("hello")));

  io_socket_close(pair[0]);
  io_socket_close(pair[1]);
}

#endif
