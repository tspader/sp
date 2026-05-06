#include "mem.h"

UTEST_F(mem, fixed_basic_alloc) {
  sp_aligned() u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 0u);

  u8* first = sp_mem_allocator_alloc(allocator, 16);
  EXPECT_ALIGNED(first);
  EXPECT_EQ(first, storage);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 16u);
}

UTEST_F(mem, fixed_aligned_base_no_padding) {
  sp_aligned() u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 16);
  EXPECT_EQ(a, storage);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 16u);

  u8* b = sp_mem_allocator_alloc(allocator, 32);
  EXPECT_EQ(b, storage + 16);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 48u);

  u8* c = sp_mem_allocator_alloc(allocator, 64);
  EXPECT_EQ(c, storage + 48);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 112u);
}

UTEST_F(mem, fixed_unaligned_base_pads_to_alignment) {
  sp_aligned() u8 storage [256];
  u64 offsets [] = { 1, 3, 7 };
  u64 alloc_sizes [] = { 8, 16, 32 };

  sp_carr_for(offsets, off_it) {
    sp_carr_for(alloc_sizes, size_it) {
      u64 off = offsets[off_it];
      u64 size = alloc_sizes[size_it];
      u8* base = storage + off;
      u64 capacity = sizeof(storage) - off;

      sp_mem_fixed_t fixed = sp_mem_fixed(base, capacity);
      sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

      u8* a = sp_mem_allocator_alloc(allocator, size);
      EXPECT_ALIGNED(a);

      u64 expected_padding = SP_MEM_ALIGNMENT - off;
      EXPECT_EQ(a, base + expected_padding);
      EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), expected_padding + size);
    }
  }
}

UTEST_F(mem, fixed_allocations_are_zeroed) {
  u8 storage [256];
  sp_mem_fill_u8(storage, sizeof(storage), 0xCC);

  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* buf = sp_mem_allocator_alloc(allocator, 64);
  EXPECT_EQ(buf[0], 0x00);
  EXPECT_EQ(buf[63], 0x00);
}

UTEST_F(mem, fixed_padding_after_byte) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* byte = sp_mem_allocator_alloc(allocator, 1);
  u8* word = sp_mem_allocator_alloc(allocator, 8);

  EXPECT_ALIGNED(byte);
  EXPECT_ALIGNED(word);
  EXPECT_TRUE((uintptr_t)word >= (uintptr_t)byte + 1);
}

UTEST_F(mem, fixed_padding_mixed_sizes) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* p1 = sp_mem_allocator_alloc(allocator, 3);
  u8* p2 = sp_mem_allocator_alloc(allocator, 13);
  u8* p3 = sp_mem_allocator_alloc(allocator, 8);

  EXPECT_ALIGNED(p1);
  EXPECT_ALIGNED(p2);
  EXPECT_ALIGNED(p3);
}

UTEST_F(mem, fixed_clear_resets_bytes_used) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);
  EXPECT_GT(sp_mem_fixed_bytes_used(&fixed), 0u);

  sp_mem_fixed_clear(&fixed);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 0u);

  u8* reused = sp_mem_allocator_alloc(allocator, 16);
  EXPECT_EQ(reused, storage);
}

UTEST_F(mem, fixed_overflow_returns_null) {
  u8 storage [64];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* first = sp_mem_allocator_alloc(allocator, 48);
  EXPECT_NE(first, SP_NULLPTR);

  u8* overflow = sp_mem_allocator_alloc(allocator, 64);
  EXPECT_EQ(overflow, SP_NULLPTR);
}

UTEST_F(mem, fixed_free_is_noop) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* first = sp_mem_allocator_alloc(allocator, 32);
  u64 used_before = sp_mem_fixed_bytes_used(&fixed);

  sp_mem_allocator_free(allocator, first);

  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), used_before);
}

UTEST_F(mem, fixed_unaligned_basic) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed_ex(storage, sizeof(storage), 1);
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 1);
  u8* b = sp_mem_allocator_alloc(allocator, 1);
  EXPECT_EQ(b, a + 1);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 2u);
}

UTEST_F(mem, fixed_unaligned_no_padding) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed_ex(storage, sizeof(storage), 1);
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 3);
  u8* b = sp_mem_allocator_alloc(allocator, 5);
  u8* c = sp_mem_allocator_alloc(allocator, 7);

  EXPECT_EQ(b, a + 3);
  EXPECT_EQ(c, b + 5);
  EXPECT_EQ(sp_mem_fixed_bytes_used(&fixed), 15u);
}

UTEST_F(mem, fixed_custom_alignment_4) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed_ex(storage, sizeof(storage), 4);
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 1);
  u8* b = sp_mem_allocator_alloc(allocator, 1);
  u8* c = sp_mem_allocator_alloc(allocator, 1);

  EXPECT_EQ((uintptr_t)a % 4, 0u);
  EXPECT_EQ((uintptr_t)b % 4, 0u);
  EXPECT_EQ((uintptr_t)c % 4, 0u);
  EXPECT_EQ(b, a + 4);
  EXPECT_EQ(c, b + 4);
}

UTEST_F(mem, fixed_custom_alignment_8) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed_ex(storage, sizeof(storage), 8);
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 3);
  u8* b = sp_mem_allocator_alloc(allocator, 5);

  EXPECT_EQ((uintptr_t)a % 8, 0u);
  EXPECT_EQ((uintptr_t)b % 8, 0u);
  EXPECT_EQ(b, a + 8);
}

UTEST_F(mem, fixed_unaligned_base_returns_aligned_ptrs) {
  u8 storage [256];
  u8* base = (u8*)sp_align_up(storage, SP_MEM_ALIGNMENT) + 1;
  u64 capacity = (u64)((storage + sizeof(storage)) - base);

  sp_mem_fixed_t fixed = sp_mem_fixed(base, capacity);
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* a = sp_mem_allocator_alloc(allocator, 8);
  u8* b = sp_mem_allocator_alloc(allocator, 24);

  EXPECT_ALIGNED(a);
  EXPECT_ALIGNED(b);
  EXPECT_GE((uintptr_t)a, (uintptr_t)base);
  EXPECT_GE((uintptr_t)b, (uintptr_t)(a + 8));
}

UTEST_F(mem, fixed_resize_returns_null) {
  u8 storage [256];
  sp_mem_fixed_t fixed = sp_mem_fixed(storage, sizeof(storage));
  sp_mem_t allocator = sp_mem_fixed_as_allocator(&fixed);

  u8* first = sp_mem_allocator_alloc(allocator, 16);
  void* resized = sp_mem_allocator_realloc(allocator, first, 32);
  EXPECT_EQ(resized, SP_NULLPTR);
}
