#include "tls.h"

typedef struct {
  bool                ok;
  sp_http_addr_kind_t kind;
  u8                  data [16];
} expect_t;

typedef struct {
  const c8* name;
  const c8* host;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "v4",
    .host = "1.2.3.4",
    .expect = { true, SP_HTTP_ADDR_V4, { 1, 2, 3, 4 } },
  },
  {
    .name = "v4_max",
    .host = "255.255.255.255",
    .expect = { true, SP_HTTP_ADDR_V4, { 255, 255, 255, 255 } },
  },
  {
    .name = "v4_zero",
    .host = "0.0.0.0",
    .expect = { true, SP_HTTP_ADDR_V4 },
  },
  { .name = "v4_leading_zero", .host = "01.2.3.4" },
  { .name = "v4_octet_too_big", .host = "256.1.1.1" },
  { .name = "v4_three_parts", .host = "1.2.3" },
  { .name = "v4_five_parts", .host = "1.2.3.4.5" },
  { .name = "v4_empty_part", .host = "1..2.3" },
  { .name = "v4_junk", .host = "a.b.c.d" },
  { .name = "hostname_not_literal", .host = "example.com" },
  { .name = "hostname_single_label", .host = "localhost" },
  {
    .name = "v6_loopback",
    .host = "::1",
    .expect = { true, SP_HTTP_ADDR_V6, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
  },
  {
    .name = "v6_all_zero",
    .host = "::",
    .expect = { true, SP_HTTP_ADDR_V6 },
  },
  {
    .name = "v6_full",
    .host = "2001:db8:1:2:3:4:5:6",
    .expect = { true, SP_HTTP_ADDR_V6, { 0x20, 0x01, 0x0d, 0xb8, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6 } },
  },
  {
    .name = "v6_run_middle",
    .host = "1::2",
    .expect = { true, SP_HTTP_ADDR_V6, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 } },
  },
  {
    .name = "v6_run_trailing",
    .host = "1:2:3:4:5:6:7::",
    .expect = { true, SP_HTTP_ADDR_V6, { 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 0 } },
  },
  {
    .name = "v6_run_leading",
    .host = "::2:3:4:5:6:7:8",
    .expect = { true, SP_HTTP_ADDR_V6, { 0, 0, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8 } },
  },
  {
    .name = "v6_v4_tail",
    .host = "::ffff:1.2.3.4",
    .expect = { true, SP_HTTP_ADDR_V6, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 1, 2, 3, 4 } },
  },
  {
    .name = "v6_hex_case",
    .host = "ABCD::ef01",
    .expect = { true, SP_HTTP_ADDR_V6, { 0xab, 0xcd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xef, 0x01 } },
  },
  { .name = "v6_two_runs", .host = "1::2::3" },
  { .name = "v6_too_many_groups", .host = "1:2:3:4:5:6:7:8:9" },
  { .name = "v6_run_with_eight_groups", .host = "1::2:3:4:5:6:7:8" },
  { .name = "v6_seven_groups_no_run", .host = "1:2:3:4:5:6:7" },
  { .name = "v6_lone_leading_colon", .host = ":1:2:3:4:5:6:7" },
  { .name = "v6_trailing_colon", .host = "1:2:3:4:5:6:7:8:" },
  { .name = "v6_group_too_long", .host = "12345::" },
  { .name = "v6_bad_char", .host = "1:2:3:g::" },
  { .name = "v6_v4_not_tail", .host = "::1.2.3.4:5" },
  { .name = "v6_v4_tail_no_room", .host = "1:2:3:4:5:6:7:1.2.3.4" },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_addr_t addr = sp_zero;
  bool ok = sp_http_addr_parse(sp_cstr_as_str(c->host), &addr);
  sp_expect_eq(t, ok, c->expect.ok);
  if (!ok || !c->expect.ok) return SP_OK;

  sp_expect_eq(t, (s32)addr.kind, (s32)c->expect.kind);
  sp_expect_mem_eq(t, addr.data, c->expect.data, sizeof(addr.data));
  return SP_OK;
}

sp_test_each_fn(tls, addr, test_t, tests, run);
