#include "sp.h"
#include "test.h"
#include "utest.h"

typedef struct {
  sp_str_t utf8;
  u32 wtf16_len;
  u16 wtf16[8];
  const c8* label;
} wtf8_case_t;

UTEST(wtf8, to_wtf16_cases) {
  wtf8_case_t cases[] = {
    { SP_LIT(""),          0, {0},                                             "empty" },
    { SP_LIT("a"),         1, {0x0061},                                        "ascii single" },
    { SP_LIT("abc"),       3, {0x0061, 0x0062, 0x0063},                        "ascii run" },
    { SP_LIT("/"),         1, {0x002F},                                        "slash" },
    { SP_LIT("C:\\"),      3, {0x0043, 0x003A, 0x005C},                        "drive prefix" },
    { SP_LIT("\xC3\xA9"),  1, {0x00E9},                                        "2-byte utf8 (e-acute)" },
    { SP_LIT("\xE2\x82\xAC"), 1, {0x20AC},                                     "3-byte utf8 (euro)" },
    { SP_LIT("\xF0\x9F\x98\x80"), 2, {0xD83D, 0xDE00},                         "4-byte utf8 (grinning face) surrogate pair" },
    { SP_LIT("\xED\xA0\x80"),     1, {0xD800},                                 "unpaired high surrogate" },
    { SP_LIT("\xED\xBF\xBF"),     1, {0xDFFF},                                 "unpaired low surrogate" },
    { SP_LIT("\xED\xA0\x80\xED\xB0\x80"), 2, {0xD800, 0xDC00},                 "wtf8 surrogate pair via 2 3-byte seqs" },
    { SP_LIT("a\xC3\xA9\xE2\x82\xACz"),   4, {0x0061, 0x00E9, 0x20AC, 0x007A}, "mixed ascii + 2-byte + 3-byte" },
  };

  SP_CARR_FOR(cases, i) {
    wtf8_case_t c = cases[i];
    sp_wide_str_t str = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), c.utf8);
    EXPECT_EQ(str.len, c.wtf16_len);
    sp_for(j, c.wtf16_len) {
      if (str.data[j] != c.wtf16[j]) {
        SP_TEST_REPORT("{}: [{}] got {} but expected {}",
          sp_fmt_cstr(c.label), sp_fmt_uint(j),
          sp_fmt_uint((u64)str.data[j]), sp_fmt_uint((u64)c.wtf16[j]));
        SP_FAIL();
      }
    }
  }
}

UTEST(wtf8, to_wtf16_alloc_null_terminates) {
  sp_wide_str_t w = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("hi"));
  EXPECT_EQ(w.len, (u32)2);
  EXPECT_EQ(w.data[0], (u16)'h');
  EXPECT_EQ(w.data[1], (u16)'i');
  EXPECT_EQ(w.data[2], (u16)0);
}

UTEST(wtf8, to_wtf16_alloc_empty_returns_null) {
  sp_wide_str_t w = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT(""));
  EXPECT_EQ(w.data, (u16*)SP_NULLPTR);
  EXPECT_EQ(w.len, 0u);
}

UTEST(wtf8, to_wtf16_alloc_rejects_invalid) {
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC3")).data, (u16*)SP_NULLPTR);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC3\x28")).data, (u16*)SP_NULLPTR);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC0\x80")).data, (u16*)SP_NULLPTR);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xE0\x80\x80")).data, (u16*)SP_NULLPTR);
}

UTEST(wtf8, reject_invalid) {
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC0")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC3")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC3\x28")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xE2\x82")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xC0\x80")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xE0\x80\x80")).len, 0u);
  EXPECT_EQ(sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), SP_LIT("\xF0\x80\x80\x80")).len, 0u);
}

UTEST(wtf8, roundtrip_ascii) {
  sp_str_t input = SP_LIT("hello/world.txt");
  sp_str_t back = sp_wtf16_to_wtf8_a(sp_mem_get_scratch(), sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), input));
  SP_EXPECT_STR_EQ(back, input);
}

UTEST(wtf8, roundtrip_non_ascii) {
  sp_str_t input = SP_LIT("caf\xC3\xA9/\xE2\x82\xAC");
  sp_str_t back = sp_wtf16_to_wtf8_a(sp_mem_get_scratch(), sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), input));
  SP_EXPECT_STR_EQ(back, input);
}

UTEST(wtf8, roundtrip_4byte_via_surrogate_pair) {
  sp_str_t input = SP_LIT("\xF0\x9F\x98\x80");
  sp_wide_str_t w = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), input);
  EXPECT_EQ(w.len, (u32)2);
  EXPECT_EQ(w.data[0], (u16)0xD83D);
  EXPECT_EQ(w.data[1], (u16)0xDE00);
  sp_str_t back = sp_wtf16_to_wtf8_a(sp_mem_get_scratch(), w);
  SP_EXPECT_STR_EQ(back, input);
}

UTEST(wtf8, roundtrip_unpaired_high_surrogate) {
  sp_str_t input = SP_LIT("\xED\xA0\x80");
  sp_wide_str_t w = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), input);
  EXPECT_EQ(w.len, (u32)1);
  EXPECT_EQ(w.data[0], (u16)0xD800);
  sp_str_t back = sp_wtf16_to_wtf8_a(sp_mem_get_scratch(), w);
  SP_EXPECT_STR_EQ(back, input);
}

UTEST(wtf8, roundtrip_unpaired_low_surrogate) {
  sp_str_t input = SP_LIT("\xED\xBF\xBF");
  sp_wide_str_t w = sp_wtf8_to_wtf16_a(sp_mem_get_scratch(), input);
  EXPECT_EQ(w.len, (u32)1);
  EXPECT_EQ(w.data[0], (u16)0xDFFF);
  sp_str_t back = sp_wtf16_to_wtf8_a(sp_mem_get_scratch(), w);
  SP_EXPECT_STR_EQ(back, input);
}

UTEST(wtf8, validate) {
  EXPECT_TRUE(sp_wtf8_validate(SP_LIT("")));
  EXPECT_TRUE(sp_wtf8_validate(SP_LIT("abc")));
  EXPECT_TRUE(sp_wtf8_validate(SP_LIT("caf\xC3\xA9")));
  EXPECT_TRUE(sp_wtf8_validate(SP_LIT("\xED\xA0\x80")));
  EXPECT_FALSE(sp_wtf8_validate(SP_LIT("\xC0\x80")));
  EXPECT_FALSE(sp_wtf8_validate(SP_LIT("\xC3")));
  EXPECT_FALSE(sp_wtf8_validate(SP_LIT("\xC3\x28")));
}

SP_TEST_MAIN()
