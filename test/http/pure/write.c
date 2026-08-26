#include "../http.h"

typedef enum {
  WIRE_REQUEST_HEAD,
  WIRE_RESPONSE_HEAD,
  WIRE_RESPONSE,
} wire_kind_t;

typedef struct {
  sp_http_error_t err;
  const c8*       wire;
} expect_t;

typedef struct {
  const c8*   name;
  wire_kind_t kind;
  const c8*   method;
  const c8*   target;
  s32         status;
  header_t    headers [HTTP_TEST_MAX_HEADERS];
  const c8*   body;
  expect_t    expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "request_no_headers",
    .kind = WIRE_REQUEST_HEAD,
    .method = "GET",
    .target = "/x",
    .expect = { .wire = "GET /x HTTP/1.1\r\n\r\n" },
  },
  {
    .name = "request_with_headers",
    .kind = WIRE_REQUEST_HEAD,
    .method = "POST",
    .target = "/shot",
    .headers = { { "Host", "A" }, { "Content-Length", "4" } },
    .expect = { .wire = "POST /shot HTTP/1.1\r\nHost: A\r\nContent-Length: 4\r\n\r\n" },
  },
  {
    .name = "request_bad_method",
    .kind = WIRE_REQUEST_HEAD,
    .method = "GE T",
    .target = "/x",
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "request_bad_target",
    .kind = WIRE_REQUEST_HEAD,
    .method = "GET",
    .target = "/x y",
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "request_header_injection",
    .kind = WIRE_REQUEST_HEAD,
    .method = "GET",
    .target = "/x",
    .headers = { { "X", "a\r\nEvil: 1" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "response_head",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 200,
    .headers = { { "X", "y" } },
    .expect = { .wire = "HTTP/1.1 200 OK\r\nX: y\r\n\r\n" },
  },
  {
    .name = "response_head_no_content",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 204,
    .expect = { .wire = "HTTP/1.1 204 No Content\r\n\r\n" },
  },
  {
    .name = "response_head_not_found",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 404,
    .expect = { .wire = "HTTP/1.1 404 Not Found\r\n\r\n" },
  },
  {
    .name = "response_head_unknown_status",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 599,
    .expect = { .wire = "HTTP/1.1 599 \r\n\r\n" },
  },
  {
    .name = "response_head_bad_status",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 42,
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  // sp_http_response_head_write leaves framing to the caller, so a caller
  // Content-Length passes through
  {
    .name = "response_head_caller_framing",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 200,
    .headers = { { "Content-Length", "5" } },
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\n" },
  },
  {
    .name = "response_header_injection",
    .kind = WIRE_RESPONSE_HEAD,
    .status = 200,
    .headers = { { "X\r\nEvil", "1" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "response_with_body",
    .kind = WIRE_RESPONSE,
    .status = 200,
    .headers = { { "Content-Type", "text/plain" } },
    .body = "hello",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello" },
  },
  {
    .name = "response_with_empty_body",
    .kind = WIRE_RESPONSE,
    .status = 200,
    .body = "",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n" },
  },
  // sp_http_response_write owns framing, so caller framing headers are a
  // config error rather than a duplicate on the wire
  {
    .name = "response_caller_content_length",
    .kind = WIRE_RESPONSE,
    .status = 200,
    .headers = { { "Content-Length", "5" } },
    .body = "hello",
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "response_caller_transfer_encoding",
    .kind = WIRE_RESPONSE,
    .status = 200,
    .headers = { { "Transfer-Encoding", "chunked" } },
    .body = "hello",
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);

  sp_http_header_t headers [HTTP_TEST_MAX_HEADERS];
  u32 count = header_count(c->headers, HTTP_TEST_MAX_HEADERS);
  sp_for(it, count) {
    headers[it] = (sp_http_header_t) {
      .name = sp_cstr_as_str(c->headers[it].name),
      .value = sp_cstr_as_str(c->headers[it].value),
    };
  }

  sp_io_dyn_mem_writer_t out = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &out);

  sp_http_error_t err = SP_HTTP_OK;
  switch (c->kind) {
    case WIRE_REQUEST_HEAD: {
      err = sp_http_request_head_write(&out.base, sp_cstr_as_str(c->method), sp_cstr_as_str(c->target), headers, count);
      break;
    }
    case WIRE_RESPONSE_HEAD: {
      err = sp_http_response_head_write(&out.base, c->status, headers, count);
      break;
    }
    case WIRE_RESPONSE: {
      err = sp_http_response_write(&out.base, c->status, headers, count, sp_cstr_as_str(c->body));
      break;
    }
  }

  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&out), c->expect.wire);
  return SP_OK;
}

sp_test_each_fn(http, write, test_t, tests, run);
