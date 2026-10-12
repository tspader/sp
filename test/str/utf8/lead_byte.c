#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  u8 byte;
  u8 expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "nul",
    .expect = 1
  },
  {
    .name = "ascii",
    .byte = 0x41,
    .expect = 1
  },
  {
    .name = "ascii_max",
    .byte = 0x7F,
    .expect = 1
  },
  {
    .name = "continuation_min",
    .byte = 0x80
  },
  {
    .name = "continuation_max",
    .byte = 0xBF
  },
  {
    .name = "two_byte_min",
    .byte = 0xC0,
    .expect = 2
  },
  {
    .name = "two_byte",
    .byte = 0xC2,
    .expect = 2
  },
  {
    .name = "two_byte_max",
    .byte = 0xDF,
    .expect = 2
  },
  {
    .name = "three_byte_min",
    .byte = 0xE0,
    .expect = 3
  },
  {
    .name = "three_byte_max",
    .byte = 0xEF,
    .expect = 3
  },
  {
    .name = "four_byte_min",
    .byte = 0xF0,
    .expect = 4
  },
  {
    .name = "four_byte",
    .byte = 0xF4,
    .expect = 4
  },
  {
    .name = "four_byte_max",
    .byte = 0xF7,
    .expect = 4
  },
  {
    .name = "five_byte",
    .byte = 0xF8
  },
  {
    .name = "all_ones",
    .byte = 0xFF
  },
};

sp_test_each(utf8, lead_byte, test_t, tests) {
  sp_expect_eq(t, sp_utf8_num_bytes_from_byte(it->byte), it->expect);
  sp_expect_eq(t, sp_utf8_num_bytes_from_ptr(&it->byte), it->expect);
  return SP_OK;
}
