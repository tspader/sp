#include "mem.h"

UTEST_F(mem, slice_basic) {
  u8 data[] = { 0x01, 0x02, 0x03, 0x04 };
  sp_mem_slice_t slice = sp_mem_slice(data, 4);
  EXPECT_EQ(slice.data, data);
  EXPECT_EQ(slice.len, 4u);
}

UTEST_F(mem, slice_sub) {
  u8 data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
  sp_mem_slice_t slice = sp_mem_slice(data, 5);
  sp_mem_slice_t sub = sp_mem_slice_sub(slice, 1, 3);
  EXPECT_EQ(sub.data, data + 1);
  EXPECT_EQ(sub.len, 3u);
  EXPECT_EQ(sub.data[0], 0x02);
}

UTEST_F(mem, slice_prefix_suffix) {
  u8 data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
  sp_mem_slice_t slice = sp_mem_slice(data, 5);

  sp_mem_slice_t prefix = sp_mem_slice_prefix(slice, 2);
  EXPECT_EQ(prefix.len, 2u);
  EXPECT_EQ(prefix.data[0], 0x01);

  sp_mem_slice_t suffix = sp_mem_slice_suffix(slice, 2);
  EXPECT_EQ(suffix.len, 2u);
  EXPECT_EQ(suffix.data[0], 0x04);
}

UTEST_F(mem, slice_iterator) {
  u8 data[] = { 0xAA, 0xBB, 0xCC };
  sp_mem_slice_t slice = sp_mem_slice(data, 3);

  u64 count = 0;
  u8 expected[] = { 0xAA, 0xBB, 0xCC };
  sp_mem_slice_for_it(slice, it) {
    EXPECT_EQ(it.byte, expected[count]);
    count++;
  }
  EXPECT_EQ(count, 3u);
}
