#include "../http.h"

typedef struct {
  const c8* decoded;
  u32       len;
} expect_t;

typedef struct {
  const c8* name;
  const c8* encoded;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "plain",
    .encoded = "abc",
    .expect = { .decoded = "abc" },
  },
  {
    .name = "space",
    .encoded = "a%20b",
    .expect = { .decoded = "a b" },
  },
  {
    .name = "lowercase_hex",
    .encoded = "%2f",
    .expect = { .decoded = "/" },
  },
  {
    .name = "uppercase_hex",
    .encoded = "%2F",
    .expect = { .decoded = "/" },
  },
  {
    .name = "adjacent_escapes",
    .encoded = "%41%42",
    .expect = { .decoded = "AB" },
  },
  {
    .name = "plus_is_literal",
    .encoded = "a+b",
    .expect = { .decoded = "a+b" },
  },
  {
    .name = "truncated_escape_kept",
    .encoded = "a%2",
    .expect = { .decoded = "a%2" },
  },
  {
    .name = "bad_hex_kept",
    .encoded = "a%zzb",
    .expect = { .decoded = "a%zzb" },
  },
  {
    .name = "nul_byte_decoded",
    .encoded = "a%00b",
    .expect = { .decoded = "a\0b", .len = 3 },
  },
  {
    .name = "empty",
    .encoded = "",
    .expect = { .decoded = "" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_str_t decoded = sp_http_percent_decode(sp_test_arena(t), sp_cstr_as_str(c->encoded));
  sp_str_t expected = c->expect.len ? sp_str(sp_cast(c8*, c->expect.decoded), c->expect.len) : sp_cstr_as_str(c->expect.decoded);
  sp_expect_str_eq(t, decoded, expected);
  return SP_OK;
}

sp_test_each_fn(http, decode, test_t, tests, run);
