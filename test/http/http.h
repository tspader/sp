#ifndef HTTP_TEST_H
#define HTTP_TEST_H

// The suite links against private sp_http functions across translation units,
// so they must have external linkage; the implementation TU (fetch.c) defines
// SP_PRIVATE empty for the same reason.
#define SP_PRIVATE
#define SP_IMP
#define SP_HTTP_EVERYTHING_PUBLIC
#include "sp/sp_http.h"
#include "sp/sp_test.h"

#define HTTP_TEST_MAX_HEADERS 4

typedef struct {
  const c8* name;
  const c8* value;
} header_t;

SP_INLINE u32 header_count(const header_t* headers, u32 capacity) {
  u32 count = 0;
  sp_for(it, capacity) {
    if (!headers[it].name) break;
    count++;
  }
  return count;
}

SP_INLINE sp_err_t expect_headers(sp_test_t* t, sp_str_t headers, const header_t* expected, u32 capacity) {
  u32 count = header_count(expected, capacity);
  sp_http_headers_it_t it = sp_http_headers_it(headers);
  sp_http_header_t header = sp_zero;
  sp_for(at, count) {
    sp_test_kv_c(t, "header", expected[at].name);
    sp_must_eq(t, sp_http_headers_it_next(&it, &header), true);
    sp_expect_str_eq_c(t, header.name, expected[at].name);
    sp_expect_str_eq_c(t, header.value, expected[at].value);
  }
  sp_test_kv_clear(t, "header");
  sp_must_eq(t, sp_http_headers_it_next(&it, &header), false);
  return SP_OK;
}

// header lines as they sit in a parsed head: CRLF-separated, no terminator
SP_INLINE sp_str_t header_lines(sp_mem_t mem, const header_t* headers, u32 capacity) {
  sp_io_dyn_mem_writer_t w = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &w);
  u32 count = header_count(headers, capacity);
  sp_for(it, count) {
    if (it) sp_io_write_str(&w.base, sp_str_lit("\r\n"), SP_NULLPTR);
    sp_fmt_io(&w.base, "{}: {}", sp_fmt_cstr(headers[it].name), sp_fmt_cstr(headers[it].value));
  }
  return sp_io_dyn_mem_writer_as_str(&w);
}

#endif
