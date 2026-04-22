#include "mem.h"

typedef struct {
  const u8* src;
  u32 src_len;
  u32 copy_len;
} mem_copy_test_t;

static void run_mem_copy_test(int* utest_result, mem_copy_test_t* t) {
  u8 dest[512] = {0};
  u8 sentinel = 0xAB;

  for (u32 i = 0; i < sizeof(dest); i++) dest[i] = sentinel;

  sp_mem_copy(t->src, dest, t->copy_len);

  for (u32 i = 0; i < t->copy_len; i++) {
    EXPECT_EQ(dest[i], t->src[i]);
  }

  if (t->copy_len < sizeof(dest)) {
    EXPECT_EQ(dest[t->copy_len], sentinel);
  }
}

typedef struct {
  const c8* label;
  u32 buf_size;
  u32 src_offset;
  u32 dst_offset;
  u32 move_len;
} mem_move_test_t;

static void run_mem_move_test(int* utest_result, mem_move_test_t* t) {
  u8 buf[512];
  u8 expected[512];

  for (u32 i = 0; i < t->buf_size; i++) {
    buf[i] = (u8)(i & 0xFF);
    expected[i] = (u8)(i & 0xFF);
  }

  u8 tmp[512];
  for (u32 i = 0; i < t->move_len; i++) {
    tmp[i] = expected[t->src_offset + i];
  }
  for (u32 i = 0; i < t->move_len; i++) {
    expected[t->dst_offset + i] = tmp[i];
  }

  sp_mem_move(buf + t->src_offset, buf + t->dst_offset, t->move_len);

  for (u32 i = 0; i < t->buf_size; i++) {
    EXPECT_EQ(buf[i], expected[i]);
  }
}

typedef struct {
  const u8* a;
  const u8* b;
  u64 len;
  bool expected;
} mem_is_equal_test_t;

static void run_mem_is_equal_test(int* utest_result, mem_is_equal_test_t* t) {
  bool result = sp_mem_is_equal(t->a, t->b, t->len);
  EXPECT_EQ(result, t->expected);
}

typedef struct {
  u8 fill_pattern[8];
  u64 fill_size;
  u32 buffer_size;
} mem_fill_test_t;

static void run_mem_fill_test(int* utest_result, mem_fill_test_t* t) {
  u8 buffer[512];
  u8 sentinel = 0xCD;
  for (u32 i = 0; i < sizeof(buffer); i++) buffer[i] = sentinel;

  u8 pattern[8];
  for (u32 i = 0; i < t->fill_size; i++) pattern[i] = t->fill_pattern[i];

  sp_mem_fill(buffer, t->buffer_size, pattern, t->fill_size);

  u32 filled = (t->buffer_size / (u32)t->fill_size) * (u32)t->fill_size;
  for (u32 i = 0; i < filled; i++) {
    EXPECT_EQ(buffer[i], t->fill_pattern[i % t->fill_size]);
  }

  for (u32 i = filled; i < t->buffer_size; i++) {
    EXPECT_EQ(buffer[i], sentinel);
  }

  if (t->buffer_size < sizeof(buffer)) {
    EXPECT_EQ(buffer[t->buffer_size], sentinel);
  }
}

typedef struct {
  u8 fill;
  u32 buffer_size;
} mem_fill_u8_test_t;

static void run_mem_fill_u8_test(int* utest_result, mem_fill_u8_test_t* t) {
  u8 buffer[512];
  u8 sentinel = 0xCD;
  for (u32 i = 0; i < sizeof(buffer); i++) buffer[i] = sentinel;

  sp_mem_fill_u8(buffer, t->buffer_size, t->fill);

  for (u32 i = 0; i < t->buffer_size; i++) {
    EXPECT_EQ(buffer[i], t->fill);
  }

  if (t->buffer_size < sizeof(buffer)) {
    EXPECT_EQ(buffer[t->buffer_size], sentinel);
  }
}

typedef struct {
  u32 buffer_size;
} mem_zero_test_t;

static void run_mem_zero_test(int* utest_result, mem_zero_test_t* t) {
  u8 buffer[512];
  u8 sentinel = 0xEF;
  for (u32 i = 0; i < sizeof(buffer); i++) buffer[i] = sentinel;

  sp_mem_zero(buffer, t->buffer_size);

  for (u32 i = 0; i < t->buffer_size; i++) {
    EXPECT_EQ(buffer[i], 0);
  }

  if (t->buffer_size < sizeof(buffer)) {
    EXPECT_EQ(buffer[t->buffer_size], sentinel);
  }
}

UTEST_F(mem, copy_zero_length) {
  run_mem_copy_test(utest_result, &(mem_copy_test_t){
    .src = (const u8*)"hello", .src_len = 5, .copy_len = 0
  });
}

UTEST_F(mem, copy_one_byte) {
  run_mem_copy_test(utest_result, &(mem_copy_test_t){
    .src = (const u8*)"A", .src_len = 1, .copy_len = 1
  });
}

UTEST_F(mem, copy_basic_string) {
  run_mem_copy_test(utest_result, &(mem_copy_test_t){
    .src = (const u8*)"hello world", .src_len = 11, .copy_len = 11
  });
}

UTEST_F(mem, copy_256_bytes) {
  static u8 data[256];
  for (u32 i = 0; i < 256; i++) data[i] = (u8)i;
  run_mem_copy_test(utest_result, &(mem_copy_test_t){
    .src = data, .src_len = 256, .copy_len = 256
  });
}

UTEST_F(mem, copy_partial) {
  run_mem_copy_test(utest_result, &(mem_copy_test_t){
    .src = (const u8*)"hello world", .src_len = 11, .copy_len = 5
  });
}

UTEST_F(mem, move_zero_length) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "zero", .buf_size = 32, .src_offset = 0, .dst_offset = 16, .move_len = 0
  });
}

UTEST_F(mem, move_non_overlapping) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "no overlap", .buf_size = 64, .src_offset = 0, .dst_offset = 32, .move_len = 16
  });
}

UTEST_F(mem, move_self_copy) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "self", .buf_size = 32, .src_offset = 0, .dst_offset = 0, .move_len = 32
  });
}

UTEST_F(mem, move_overlap_forward) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "fwd overlap", .buf_size = 32, .src_offset = 0, .dst_offset = 4, .move_len = 16
  });
}

UTEST_F(mem, move_overlap_backward) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "bwd overlap", .buf_size = 32, .src_offset = 4, .dst_offset = 0, .move_len = 16
  });
}

UTEST_F(mem, move_overlap_one_byte_shift) {
  run_mem_move_test(utest_result, &(mem_move_test_t){
    .label = "1 byte shift", .buf_size = 32, .src_offset = 0, .dst_offset = 1, .move_len = 31
  });
}

UTEST_F(mem, is_equal_zero_length) {
  u8 a[] = {1, 2, 3};
  u8 b[] = {9, 8, 7};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = a, .b = b, .len = 0, .expected = true
  });
}

UTEST_F(mem, is_equal_identical) {
  u8 data[] = {0xDE, 0xAD, 0xBE, 0xEF};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = data, .b = data, .len = 4, .expected = true
  });
}

UTEST_F(mem, is_equal_same_content) {
  u8 a[] = {1, 2, 3, 4, 5};
  u8 b[] = {1, 2, 3, 4, 5};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = a, .b = b, .len = 5, .expected = true
  });
}

UTEST_F(mem, is_equal_first_byte_differs) {
  u8 a[] = {0, 2, 3};
  u8 b[] = {1, 2, 3};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = a, .b = b, .len = 3, .expected = false
  });
}

UTEST_F(mem, is_equal_last_byte_differs) {
  u8 a[] = {1, 2, 3};
  u8 b[] = {1, 2, 4};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = a, .b = b, .len = 3, .expected = false
  });
}

UTEST_F(mem, is_equal_partial_match) {
  u8 a[] = {1, 2, 3, 4};
  u8 b[] = {1, 2, 9, 9};
  run_mem_is_equal_test(utest_result, &(mem_is_equal_test_t){
    .a = a, .b = b, .len = 2, .expected = true
  });
}

UTEST_F(mem, fill_single_byte_pattern) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {0xFF}, .fill_size = 1, .buffer_size = 16
  });
}

UTEST_F(mem, fill_two_byte_pattern) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {0xAB, 0xCD}, .fill_size = 2, .buffer_size = 16
  });
}

UTEST_F(mem, fill_four_byte_pattern) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {0xDE, 0xAD, 0xBE, 0xEF}, .fill_size = 4, .buffer_size = 16
  });
}

UTEST_F(mem, fill_pattern_larger_than_buffer) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {0xAA, 0xBB, 0xCC, 0xDD}, .fill_size = 4, .buffer_size = 3
  });
}

UTEST_F(mem, fill_exact_multiple) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {1, 2, 3, 4}, .fill_size = 4, .buffer_size = 8
  });
}

UTEST_F(mem, fill_remainder_truncated) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {1, 2, 3, 4}, .fill_size = 4, .buffer_size = 10
  });
}

UTEST_F(mem, fill_zero_buffer) {
  run_mem_fill_test(utest_result, &(mem_fill_test_t){
    .fill_pattern = {0xFF}, .fill_size = 1, .buffer_size = 0
  });
}

UTEST_F(mem, fill_u8_zero) {
  run_mem_fill_u8_test(utest_result, &(mem_fill_u8_test_t){
    .fill = 0x00, .buffer_size = 32
  });
}

UTEST_F(mem, fill_u8_nonzero) {
  run_mem_fill_u8_test(utest_result, &(mem_fill_u8_test_t){
    .fill = 0xAA, .buffer_size = 32
  });
}

UTEST_F(mem, fill_u8_one_byte) {
  run_mem_fill_u8_test(utest_result, &(mem_fill_u8_test_t){
    .fill = 0xFF, .buffer_size = 1
  });
}

UTEST_F(mem, fill_u8_zero_size) {
  run_mem_fill_u8_test(utest_result, &(mem_fill_u8_test_t){
    .fill = 0xFF, .buffer_size = 0
  });
}

UTEST_F(mem, fill_u8_large) {
  run_mem_fill_u8_test(utest_result, &(mem_fill_u8_test_t){
    .fill = 0x42, .buffer_size = 400
  });
}

UTEST_F(mem, zero_basic) {
  run_mem_zero_test(utest_result, &(mem_zero_test_t){ .buffer_size = 32 });
}

UTEST_F(mem, zero_one_byte) {
  run_mem_zero_test(utest_result, &(mem_zero_test_t){ .buffer_size = 1 });
}

UTEST_F(mem, zero_zero_size) {
  run_mem_zero_test(utest_result, &(mem_zero_test_t){ .buffer_size = 0 });
}

UTEST_F(mem, zero_large) {
  run_mem_zero_test(utest_result, &(mem_zero_test_t){ .buffer_size = 400 });
}

static void* mock_alloc_record_size(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr) {
  (void)mode;
  (void)ptr;
  *(u64*)user_data = size;
  return SP_NULLPTR;
}

UTEST_F(mem, alloc_preserves_u64_size) {
  u64 recorded_size = 0;
  sp_allocator_t mock = {
    .on_alloc = mock_alloc_record_size,
    .user_data = &recorded_size,
  };

  sp_context_push_allocator(mock);
  u64 requested = (u64)5 * 1024 * 1024 * 1024;
  sp_alloc(requested);
  sp_context_pop();

  EXPECT_EQ(recorded_size, requested);
}

UTEST_F(mem, libc_metadata_stores_u64_size) {
  sp_allocator_t libc = sp_mem_os_new();

  u64 size = 64;
  void* ptr = sp_mem_allocator_alloc(libc, size);
  ASSERT_TRUE(ptr);

  sp_mem_os_header_t* meta = sp_mem_os_get_header(ptr);
  EXPECT_EQ(meta->size, size);

  sp_mem_allocator_free(libc, ptr);
}
