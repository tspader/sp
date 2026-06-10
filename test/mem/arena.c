#include "mem.h"

UTEST_F(mem, arena_padding_after_byte) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();

  u8* byte = sp_void_cast(byte, sp_alloc(scratch.mem, 1));

  u8* word = sp_void_cast(word, sp_alloc(scratch.mem, 8));

  EXPECT_ALIGNED(byte);
  EXPECT_ALIGNED(word);
  EXPECT_TRUE((uintptr_t)word >= (uintptr_t)byte + 1);

  sp_mem_end_scratch(scratch);
}

UTEST_F(mem, arena_padding_mixed_sizes) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();

  u8* p1 = sp_void_cast(p1, sp_alloc(scratch.mem, 3));
  u8* p2 = sp_void_cast(p2, sp_alloc(scratch.mem, 13));
  u8* p3 = sp_void_cast(p3, sp_alloc(scratch.mem, 8));

  EXPECT_ALIGNED(p1);
  EXPECT_ALIGNED(p2);
  EXPECT_ALIGNED(p3);

  sp_mem_end_scratch(scratch);
}

UTEST_F(mem, arena_basic_alloc) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);
  EXPECT_EQ(sp_mem_arena_capacity(arena), 256u - sizeof(sp_mem_arena_block_t));

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 16));
  EXPECT_ALIGNED(first);
  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_allocations_are_zeroed) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 64));
  EXPECT_EQ(first[0], 0x00);
  EXPECT_EQ(first[63], 0x00);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_packed_no_header_overhead) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 16));
  EXPECT_ALIGNED(first);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 16u);

  u8* second = sp_void_cast(second, sp_mem_allocator_alloc(allocator, 32));
  EXPECT_ALIGNED(second);
  EXPECT_EQ(second, first + 16);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 48u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_pop_resets_bytes_used) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 64));
  sp_mem_fill_u8(first, 64, 0x69);
  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_pop(marker);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_block_chaining) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 32));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 32));
  u8* c = sp_void_cast(c, sp_mem_allocator_alloc(allocator, 32));

  EXPECT_ALIGNED(a);
  EXPECT_ALIGNED(b);
  EXPECT_ALIGNED(c);

  sp_mem_fill_u8(a, 32, 0xAA);
  sp_mem_fill_u8(b, 32, 0xBB);
  sp_mem_fill_u8(c, 32, 0xCC);

  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  EXPECT_EQ(c[0], 0xCC);

  EXPECT_GT(sp_mem_arena_capacity(arena), 64u - sizeof(sp_mem_arena_block_t));

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_pop_across_blocks) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 32));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 32));
  u8* c = sp_void_cast(c, sp_mem_allocator_alloc(allocator, 32));

  sp_mem_fill_u8(a, 32, 0xAA);
  sp_mem_fill_u8(b, 32, 0xBB);
  sp_mem_fill_u8(c, 32, 0xCC);

  u64 used_before = sp_mem_arena_bytes_used(arena);
  EXPECT_GT(used_before, 0u);

  sp_mem_arena_pop(marker);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_realloc_extends_top_in_place) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 16));
  sp_mem_fill_u8(first, 16, 0xAA);

  u8* resized = sp_void_cast(resized, sp_mem_allocator_realloc(allocator, first, 16, 32));

  EXPECT_EQ(resized, first);
  EXPECT_EQ(resized[0], 0xAA);
  EXPECT_EQ(resized[15], 0xAA);
  EXPECT_EQ(resized[16], 0x00);
  EXPECT_EQ(resized[31], 0x00);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 32u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_realloc_copies_when_not_top) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 16));
  sp_mem_fill_u8(first, 16, 0xAA);
  sp_mem_allocator_alloc(allocator, 16);

  u8* resized = sp_void_cast(resized, sp_mem_allocator_realloc(allocator, first, 16, 32));

  EXPECT_NE(resized, first);
  EXPECT_EQ(resized[0], 0xAA);
  EXPECT_EQ(resized[15], 0xAA);
  EXPECT_EQ(resized[16], 0x00);
  EXPECT_EQ(resized[31], 0x00);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_realloc_shrink_returns_same_pointer) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 32));
  sp_mem_fill_u8(first, 32, 0xAA);

  u8* resized = sp_void_cast(resized, sp_mem_allocator_realloc(allocator, first, 32, 16));

  EXPECT_EQ(resized, first);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 16u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_free_reclaims_top) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 32));
  u8* second = sp_void_cast(second, sp_mem_allocator_alloc(allocator, 32));
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 64u);

  sp_mem_allocator_free(allocator, first, 32);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 64u);

  sp_mem_allocator_free(allocator, second, 32);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 32u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_clear_resets_all_blocks) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_clear(arena);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_block_reuse_after_pop) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  u64 capacity_after_allocs = sp_mem_arena_capacity(arena);

  sp_mem_arena_pop(marker);

  EXPECT_EQ(sp_mem_arena_capacity(arena), capacity_after_allocs);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  EXPECT_EQ(sp_mem_arena_capacity(arena), capacity_after_allocs);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_block_reuse_after_clear) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_allocator_alloc(allocator, 40);
  sp_mem_allocator_alloc(allocator, 40);

  u64 cap_initial = sp_mem_arena_capacity(arena);

  sp_mem_arena_clear(arena);

  sp_mem_allocator_alloc(allocator, 40);
  sp_mem_allocator_alloc(allocator, 40);

  u64 cap_after = sp_mem_arena_capacity(arena);

  EXPECT_EQ(cap_after, cap_initial);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_block_size_grows_geometrically) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_for(it, 64) {
    sp_mem_allocator_alloc(allocator, 32);
  }

  u32 num_blocks = 0;
  for (sp_mem_arena_block_t* block = arena->head; block; block = block->next) {
    num_blocks++;
  }

  EXPECT_LT(num_blocks, 16u);
  EXPECT_GE(sp_mem_arena_capacity(arena), 64u * 32u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_wrappers) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);
  EXPECT_NE(sp_mem_arena_alloc(arena, 8), SP_NULLPTR);
  void* ptr = sp_mem_arena_alloc(arena, 8);
  EXPECT_NE(sp_mem_arena_realloc(arena, ptr, 8, 72), SP_NULLPTR);
  sp_mem_arena_free(arena, ptr, 8);
  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_huge_alloc_returns_null) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, SP_MEM_ALIGNMENT);

  EXPECT_EQ(sp_mem_arena_alloc(arena, (u64)-8), SP_NULLPTR);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_unaligned_basic) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 1);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  u8* first = sp_void_cast(first, sp_mem_allocator_alloc(allocator, 1));
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 1u);

  u8* second = sp_void_cast(second, sp_mem_allocator_alloc(allocator, 1));
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 2u);
  EXPECT_EQ(second, first + 1);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_unaligned_no_padding) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 1);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 3));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 5));
  u8* c = sp_void_cast(c, sp_mem_allocator_alloc(allocator, 7));

  EXPECT_EQ(b, a + 3);
  EXPECT_EQ(c, b + 5);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 15u);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_unaligned_allocations_zeroed) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 1);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* buf = sp_void_cast(buf, sp_mem_allocator_alloc(allocator, 64));
  EXPECT_EQ(buf[0], 0x00);
  EXPECT_EQ(buf[63], 0x00);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_unaligned_block_chaining) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 64, 1);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 60));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 60));

  sp_mem_fill_u8(a, 60, 0xAA);
  sp_mem_fill_u8(b, 60, 0xBB);

  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);

  EXPECT_GT(sp_mem_arena_capacity(arena), 64u - sizeof(sp_mem_arena_block_t));

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_custom_alignment_4) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 4);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 1));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 1));
  u8* c = sp_void_cast(c, sp_mem_allocator_alloc(allocator, 1));

  EXPECT_EQ((uintptr_t)a % 4, 0u);
  EXPECT_EQ((uintptr_t)b % 4, 0u);
  EXPECT_EQ((uintptr_t)c % 4, 0u);

  EXPECT_EQ(b, a + 4);
  EXPECT_EQ(c, b + 4);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_custom_alignment_8) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 8);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_void_cast(a, sp_mem_allocator_alloc(allocator, 3));
  u8* b = sp_void_cast(b, sp_mem_allocator_alloc(allocator, 5));

  EXPECT_EQ((uintptr_t)a % 8, 0u);
  EXPECT_EQ((uintptr_t)b % 8, 0u);

  EXPECT_EQ(b, a + 8);

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_custom_alignment_64) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 4096, 64);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_for(it, 8) {
    u8* p = sp_void_cast(p, sp_mem_allocator_alloc(allocator, 24));
    EXPECT_EQ((uintptr_t)p % 64, 0u);
  }

  sp_mem_arena_destroy(arena);
}

UTEST_F(mem, arena_custom_alignment_64_across_blocks) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 64);
  sp_mem_t allocator = sp_mem_arena_as_allocator(arena);

  sp_for(it, 8) {
    u8* p = sp_void_cast(p, sp_mem_allocator_alloc(allocator, 100));
    EXPECT_EQ((uintptr_t)p % 64, 0u);
  }

  EXPECT_GT(sp_mem_arena_capacity(arena), 256u);

  sp_mem_arena_destroy(arena);
}
