#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  u32 codepoint;
  u8 len;
  u8 bytes [4];
} test_t;

static const test_t tests [] = {
  {
    .name = "nul",
    .len = 1
  },
  {
    .name = "ascii",
    .codepoint = 0x41,
    .len = 1,
    .bytes = { 0x41 }
  },
  {
    .name = "ascii_max",
    .codepoint = 0x7F,
    .len = 1,
    .bytes = { 0x7F }
  },
  {
    .name = "two_byte_min",
    .codepoint = 0x80,
    .len = 2,
    .bytes = { 0xC2, 0x80 }
  },
  {
    .name = "two_byte",
    .codepoint = 0xA2,
    .len = 2,
    .bytes = { 0xC2, 0xA2 }
  },
  {
    .name = "two_byte_max",
    .codepoint = 0x7FF,
    .len = 2,
    .bytes = { 0xDF, 0xBF }
  },
  {
    .name = "three_byte_min",
    .codepoint = 0x800,
    .len = 3,
    .bytes = { 0xE0, 0xA0, 0x80 }
  },
  {
    .name = "three_byte",
    .codepoint = 0x20AC,
    .len = 3,
    .bytes = { 0xE2, 0x82, 0xAC }
  },
  {
    .name = "three_byte_cjk",
    .codepoint = 0x4E2D,
    .len = 3,
    .bytes = { 0xE4, 0xB8, 0xAD }
  },
  {
    .name = "three_byte_max",
    .codepoint = 0xFFFF,
    .len = 3,
    .bytes = { 0xEF, 0xBF, 0xBF }
  },
  {
    .name = "four_byte_min",
    .codepoint = 0x10000,
    .len = 4,
    .bytes = { 0xF0, 0x90, 0x80, 0x80 }
  },
  {
    .name = "four_byte",
    .codepoint = 0x1F600,
    .len = 4,
    .bytes = { 0xF0, 0x9F, 0x98, 0x80 }
  },
  {
    .name = "four_byte_max",
    .codepoint = 0x10FFFF,
    .len = 4,
    .bytes = { 0xF4, 0x8F, 0xBF, 0xBF }
  },
  {
    .name = "surrogate_min",
    .codepoint = 0xD800,
    .len = 3,
    .bytes = { 0xED, 0xA0, 0x80 }
  },
  {
    .name = "surrogate_max",
    .codepoint = 0xDFFF,
    .len = 3,
    .bytes = { 0xED, 0xBF, 0xBF }
  },
  {
    .name = "above_max",
    .codepoint = 0x110000
  },
  {
    .name = "u32_max",
    .codepoint = 0xFFFFFFFF
  },
};

sp_test_each(utf8, codec, test_t, tests) {
  c8 out [4] = sp_zero;
  sp_must_eq(t, sp_utf8_encode(it->codepoint, out), it->len);
  sp_expect_eq(t, sp_utf8_num_bytes_from_codepoint(it->codepoint), it->len);
  if (!it->len) return SP_OK;

  sp_expect_mem_eq(t, out, it->bytes, it->len);
  sp_expect_eq(t, sp_utf8_decode((const c8*)it->bytes), it->codepoint);
  sp_expect_eq(t, sp_utf8_num_bytes_from_byte(it->bytes[0]), it->len);
  return SP_OK;
}
