#include "../http.h"

typedef struct {
  sp_http_error_t check;
  header_t        gets [HTTP_TEST_MAX_HEADERS]; // name looked up via find, value expected; missing names expect ""
} expect_t;

typedef struct {
  const c8* name;
  header_t  headers [HTTP_TEST_MAX_HEADERS];
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "find_exact",
    .headers = { { "X-A", "1" } },
    .expect = { .gets = { { "X-A", "1" } } },
  },
  {
    .name = "find_case_insensitive",
    .headers = { { "Content-Type", "t" } },
    .expect = { .gets = { { "content-type", "t" }, { "CONTENT-TYPE", "t" } } },
  },
  {
    .name = "find_missing",
    .headers = { { "X", "1" } },
    .expect = { .gets = { { "Y", "" } } },
  },
  {
    .name = "find_first_of_duplicates",
    .headers = { { "X", "1" }, { "X", "2" } },
    .expect = { .gets = { { "X", "1" } } },
  },
  {
    .name = "check_ok",
    .headers = { { "X-A", "abc" }, { "X-B", "a\tb" } },
  },
  {
    .name = "check_crlf_value",
    .headers = { { "X", "a\r\nEvil: 1" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "check_lone_lf_value",
    .headers = { { "X", "a\nb" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "check_name_with_space",
    .headers = { { "X Y", "1" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "check_name_with_colon",
    .headers = { { "X:", "1" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "check_name_with_paren",
    .headers = { { "X(Y)", "1" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "check_empty_name",
    .headers = { { "", "1" } },
    .expect = { .check = SP_HTTP_ERR_BAD_CONFIG },
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

  sp_expect_eq(t, (s32)sp_http_headers_check(headers, count), (s32)c->expect.check);
  if (c->expect.check != SP_HTTP_OK) return SP_OK;

  sp_str_t lines = header_lines(mem, c->headers, HTTP_TEST_MAX_HEADERS);
  sp_for(it, HTTP_TEST_MAX_HEADERS) {
    if (!c->expect.gets[it].name) break;
    sp_test_kv_c(t, "find", c->expect.gets[it].name);
    sp_str_t name = sp_cstr_as_str(c->expect.gets[it].name);
    sp_expect_str_eq_c(t, sp_http_headers_find(lines, name), c->expect.gets[it].value);
    // every present header in the gets tables has a non-empty value, so an
    // empty expectation means the name is absent
    sp_expect_eq(t, sp_http_headers_has(lines, name), c->expect.gets[it].value[0] != '\0');
  }
  sp_test_kv_clear(t, "find");
  return SP_OK;
}

sp_test_each_fn(http, headers, test_t, tests, run);
