#include "io.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(io_socket)

static bool io_socket_dial(sp_sys_socket_t socket, u16 port) {
  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  s32 rc = sp_sys_socket_connect(socket, dial);
  if (rc == 0) return true;
  if (rc != 1) return false;
  if (sp_sys_socket_wait(socket, false, 1000) != 0) return false;
  return sp_sys_socket_error(socket) == 0;
}

static bool io_socket_pair(sp_sys_socket_t* client, sp_sys_socket_t* server) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  if (sp_sys_socket_open(&listener) != 0) return false;
  if (sp_sys_socket_bind(listener, addr) != 0 || sp_sys_socket_listen(listener, 1) != 0) {
    sp_sys_socket_close(listener);
    return false;
  }

  u16 port = 0;
  if (sp_sys_socket_local_port(listener, &port) != 0) {
    sp_sys_socket_close(listener);
    return false;
  }

  if (sp_sys_socket_open(client) != 0) {
    sp_sys_socket_close(listener);
    return false;
  }
  if (!io_socket_dial(*client, port)) {
    sp_sys_socket_close(listener);
    sp_sys_socket_close(*client);
    return false;
  }

  while (true) {
    s32 rc = sp_sys_socket_accept(listener, server);
    if (rc == 0) break;
    if (rc != 1 || sp_sys_socket_wait(listener, true, 1000) != 0) {
      sp_sys_socket_close(listener);
      sp_sys_socket_close(*client);
      return false;
    }
  }

  sp_sys_socket_close(listener);
  return true;
}

typedef enum {
  IO_SOCKET_STEP_NONE,
  IO_SOCKET_STEP_WRITE,
  IO_SOCKET_STEP_READ,
  IO_SOCKET_STEP_CLOSE_WRITER,
  IO_SOCKET_STEP_READ_UNTIL,
  IO_SOCKET_STEP_LIMIT_COPY,
} io_socket_step_kind_t;

typedef struct {
  io_socket_step_kind_t kind;
  union {
    struct { const c8* data; } write;
    struct { u64 request; sp_err_t err; const c8* content; } read;
    struct { const c8* delim; u64 max; sp_err_t err; const c8* content; } read_until;
    struct { u64 limit; sp_err_t err; const c8* content; } limit_copy;
  };
} io_socket_step_t;

typedef struct {
  u32 timeout_ms;
  u64 reader_buffer;
  io_socket_step_t steps [8];
} io_socket_test_t;

void run_io_socket_test(int* utest_result, io_socket_test_t t) {
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t server = SP_SYS_INVALID_SOCKET;
  ASSERT_TRUE(io_socket_pair(&client, &server));

  sp_io_socket_writer_t writer = sp_zero;
  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_writer_init(&writer, client, t.timeout_ms ? t.timeout_ms : 1000);
  sp_io_socket_reader_init(&reader, server, t.timeout_ms ? t.timeout_ms : 1000);

  u8 reader_buf[64] = sp_zero;
  if (t.reader_buffer) {
    sp_io_reader_set_buffer(&reader.base, reader_buf, t.reader_buffer);
  }

  sp_carr_for(t.steps, it) {
    const io_socket_step_t* step = &t.steps[it];
    if (step->kind == IO_SOCKET_STEP_NONE) break;

    switch (step->kind) {
      case IO_SOCKET_STEP_NONE: break;

      case IO_SOCKET_STEP_WRITE: {
        u64 len = sp_cstr_len(step->write.data);
        EXPECT_EQ(sp_io_write_all(&writer.base, step->write.data, len, SP_NULLPTR), SP_OK);
        break;
      }

      case IO_SOCKET_STEP_READ: {
        u8 dest[64] = sp_zero;
        u64 bytes = 0;
        EXPECT_EQ(sp_io_read(&reader.base, dest, step->read.request, &bytes), step->read.err);
        u64 expect_bytes = sp_cstr_len(step->read.content);
        EXPECT_EQ(bytes, expect_bytes);
        sp_for(jt, expect_bytes) EXPECT_EQ((c8)dest[jt], step->read.content[jt]);
        break;
      }

      case IO_SOCKET_STEP_CLOSE_WRITER: {
        sp_sys_socket_close(client);
        client = SP_SYS_INVALID_SOCKET;
        break;
      }

      case IO_SOCKET_STEP_READ_UNTIL: {
        u8 head_buf[64] = sp_zero;
        sp_io_mem_writer_t head = sp_zero;
        sp_io_mem_writer_from_buffer(&head, head_buf, sizeof(head_buf));
        u64 bytes = 0;
        sp_err_t err = sp_io_read_until(&reader.base, sp_cstr_as_str(step->read_until.delim), &head.base, step->read_until.max, &bytes);
        EXPECT_EQ(err, step->read_until.err);
        u64 expect_bytes = sp_cstr_len(step->read_until.content);
        EXPECT_EQ(bytes, expect_bytes);
        EXPECT_TRUE(sp_str_equal(sp_str((c8*)head_buf, bytes), sp_cstr_as_str(step->read_until.content)));
        break;
      }

      case IO_SOCKET_STEP_LIMIT_COPY: {
        sp_io_limit_reader_t limit = sp_zero;
        sp_io_limit_reader_init(&limit, &reader.base, step->limit_copy.limit);
        u8 body_buf[64] = sp_zero;
        sp_io_mem_writer_t body = sp_zero;
        sp_io_mem_writer_from_buffer(&body, body_buf, sizeof(body_buf));
        u64 bytes = 0;
        EXPECT_EQ(sp_io_copy(&body.base, &limit.base, &bytes), step->limit_copy.err);
        u64 expect_bytes = sp_cstr_len(step->limit_copy.content);
        EXPECT_EQ(bytes, expect_bytes);
        EXPECT_TRUE(sp_str_equal(sp_str((c8*)body_buf, bytes), sp_cstr_as_str(step->limit_copy.content)));
        break;
      }
    }
  }

  if (client != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(client);
  sp_sys_socket_close(server);
}

UTEST_F(io_socket, roundtrip) {
  run_io_socket_test(utest_result, (io_socket_test_t){
    .steps = {
      { .kind = IO_SOCKET_STEP_WRITE, .write = { "hello world" } },
      { .kind = IO_SOCKET_STEP_READ, .read = { 32, SP_OK, "hello world" } },
    },
  });
}

UTEST_F(io_socket, eof_on_peer_close) {
  run_io_socket_test(utest_result, (io_socket_test_t){
    .steps = {
      { .kind = IO_SOCKET_STEP_WRITE, .write = { "x" } },
      { .kind = IO_SOCKET_STEP_CLOSE_WRITER },
      { .kind = IO_SOCKET_STEP_READ, .read = { 8, SP_OK, "x" } },
      { .kind = IO_SOCKET_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_socket, read_timeout) {
  run_io_socket_test(utest_result, (io_socket_test_t){
    .timeout_ms = 50,
    .steps = {
      { .kind = IO_SOCKET_STEP_READ, .read = { 8, SP_ERR_IO_TIMEOUT } },
    },
  });
}

// The HTTP shape: scan a buffered socket reader up to the header terminator,
// then drain a fixed-length body through a limit reader. Bytes past the
// terminator must survive in the reader's buffer.
UTEST_F(io_socket, read_until_then_limit) {
  run_io_socket_test(utest_result, (io_socket_test_t){
    .reader_buffer = 64,
    .steps = {
      { .kind = IO_SOCKET_STEP_WRITE, .write = { "HTTP/1.1 200 OK\r\n\r\nhello" } },
      { .kind = IO_SOCKET_STEP_READ_UNTIL, .read_until = { "\r\n\r\n", 64, SP_OK, "HTTP/1.1 200 OK\r\n\r\n" } },
      { .kind = IO_SOCKET_STEP_LIMIT_COPY, .limit_copy = { 5, SP_OK, "hello" } },
    },
  });
}

#endif
