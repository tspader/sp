#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct context {

};

UTEST_F_SETUP(context) {
  pthread_setspecific(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

UTEST_F_TEARDOWN(context) {
  pthread_setspecific(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

UTEST_F(context, get_returns_non_null) {
  sp_context_t *ctx = sp_context_get();
  EXPECT_TRUE(ctx != NULL);
}

UTEST_F(context, allocator_is_valid) {
  sp_context_t *ctx = sp_context_get();
  EXPECT_TRUE(ctx->allocator.on_alloc != NULL);
}

UTEST_F(context, thread_state_index_starts_at_zero) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  EXPECT_EQ(state->index, 0);
}

UTEST_F(context, push_pop_single) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();
  sp_context_push(ctx);

  EXPECT_EQ(state->index, initial_index + 1);

  sp_context_pop();
  EXPECT_EQ(state->index, initial_index);
}

UTEST_F(context, push_pop_multiple) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();

  sp_context_push(ctx);
  sp_context_push(ctx);
  sp_context_push(ctx);

  EXPECT_EQ(state->index, initial_index + 3);

  sp_context_pop();
  sp_context_pop();
  sp_context_pop();

  EXPECT_EQ(state->index, initial_index);
}

UTEST_F(context, push_allocator_changes_allocator) {
  sp_context_t *ctx_before = sp_context_get();
  sp_allocator_t old_allocator = ctx_before->allocator;

  sp_allocator_t new_allocator = sp_mem_libc_new();
  sp_context_push_allocator(new_allocator);

  sp_context_t *ctx_after = sp_context_get();

  // Allocator should be updated
  EXPECT_TRUE(ctx_after->allocator.on_alloc == new_allocator.on_alloc);

  sp_context_pop();

  // After pop, old allocator restored
  sp_context_t *ctx_restored = sp_context_get();
  EXPECT_TRUE(ctx_restored->allocator.on_alloc == old_allocator.on_alloc);
}

UTEST_F(context, set_modifies_current) {
  sp_context_t ctx = *sp_context_get();
  sp_allocator_t new_allocator = sp_mem_libc_new();
  ctx.allocator = new_allocator;

  sp_context_set(ctx);

  sp_context_t *current = sp_context_get();
  EXPECT_TRUE(current->allocator.on_alloc == new_allocator.on_alloc);
}

UTEST_F(context, scratch_initted) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  EXPECT_TRUE(arena->buffer != NULL);
  EXPECT_EQ(arena->capacity, SP_RT_SCRATCH_SIZE);
}

UTEST_F(context, begin_scratch) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_TRUE(scratch.marker.arena != NULL);
}

UTEST_F(context, end_scratch) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_EQ(arena->bytes_used, 0);

  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, 1024, NULL);
  EXPECT_GE(arena->bytes_used, 1024);

  sp_alloc(1024);
  EXPECT_GE(arena->bytes_used, 2048);

  sp_mem_end_scratch(scratch);
  EXPECT_EQ(arena->bytes_used, 0);
}

u8 *use_scratch_arena(u32 fill) {
  const u32 num_bytes = 64;

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  u8 *buffer = sp_alloc(num_bytes);
  sp_mem_fill_u8(buffer, num_bytes, fill);

  sp_context_push_allocator(scratch.old_allocator);
  u8 *result = sp_alloc(num_bytes);
  sp_mem_copy(buffer, result, num_bytes);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

UTEST_F(context, use_scratch_allocator_but_return_from_user_allocator) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  EXPECT_EQ(arena->bytes_used, 0);

  u8 *buffer = use_scratch_arena(69);
  EXPECT_EQ(buffer[0], 69);
  EXPECT_EQ(arena->bytes_used, 0);
}

typedef struct {
  sp_atomic_s32 *done_count;
  s32 thread_id;
  bool context_valid;
  bool allocator_works;
  bool independent_context;
  bool scratch_zeroed;
} context_thread_data_t;

static sp_context_t *main_thread_context = NULL;

s32 context_thread_func(void *userdata) {
  context_thread_data_t *data = (context_thread_data_t *)userdata;

  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_t *ctx = sp_context_get();
  data->context_valid = (ctx != NULL);
  data->independent_context = (ctx != main_thread_context);
  data->scratch_zeroed = (arena->bytes_used == 0);

  // Verify allocator works
  void *p = sp_alloc(64);
  data->allocator_works = (p != NULL);
  sp_free(p);

  sp_atomic_s32_add(data->done_count, 1);
  return 0;
}

#define NUM_THREADS 8

UTEST_F(context, multithread_independent_contexts) {
  main_thread_context = sp_context_get();

  sp_atomic_s32 done_count = 0;
  context_thread_data_t thread_data[NUM_THREADS] = SP_ZERO_INITIALIZE();
  sp_thread_t threads[NUM_THREADS] = SP_ZERO_INITIALIZE();

  for (s32 i = 0; i < NUM_THREADS; i++) {
    thread_data[i] =
        (context_thread_data_t){.done_count = &done_count, .thread_id = i};
    sp_thread_init(&threads[i], context_thread_func, &thread_data[i]);
  }

  for (s32 i = 0; i < NUM_THREADS; i++) {
    sp_thread_join(&threads[i]);
  }

  EXPECT_EQ(sp_atomic_s32_get(&done_count), NUM_THREADS);

  for (s32 i = 0; i < NUM_THREADS; i++) {
    EXPECT_TRUE(thread_data[i].context_valid);
    EXPECT_TRUE(thread_data[i].allocator_works);
    EXPECT_TRUE(thread_data[i].independent_context);
    EXPECT_TRUE(thread_data[i].scratch_zeroed);
  }
}

typedef struct {
  sp_atomic_s32 *done_count;
  s32 thread_id;
  s32 iterations;
  bool all_passed;
} push_pop_thread_data_t;

UTEST_F(context, push_does_not_overwrite_scratch) {
  // Use scratch arena
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  // allocate from scratch
  u8 *first = sp_alloc(64);
  sp_mem_fill_u8(first, 64, 0x01);
  EXPECT_EQ(first[0], 0x01);

  // manually push the scratch allocator and allocate
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));

  u8 *second = sp_alloc(64);
  sp_mem_fill_u8(second, 64, 0x02);

  // verify that the first allocation isn't overwritten
  EXPECT_EQ(first[0], 0x01);

  sp_context_pop();
  sp_mem_end_scratch(scratch);
}


UTEST_F(context, nested_begin_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  sp_mem_scratch_t s1 = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);

  sp_mem_scratch_t s2 = sp_mem_begin_scratch();
  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);

  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  EXPECT_NE(a, b);

  sp_mem_end_scratch(s2);
  EXPECT_EQ(a[0], 0xAA);

  sp_mem_end_scratch(s1);
  EXPECT_EQ(rt->scratch->bytes_used, 0);
}

UTEST_F(context, begin_scratch_push_unrelated_allocator_end_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);

  // push an unrelated allocator, verify that scratch is untouched
  sp_context_push_allocator(sp_mem_libc_new());

  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(rt->scratch->bytes_used, 64);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  sp_free(b);

  // pop that allocator, allocate again, verify scratch was used
  sp_context_pop();

  u8* c = sp_alloc(64);
  sp_mem_fill_u8(c, 64, 0xCC);
  EXPECT_GE(rt->scratch->bytes_used, 128);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(c[0], 0xCC);

  // pop the scratch allocator, verify cleanup
  sp_mem_end_scratch(scratch);
  EXPECT_EQ(rt->scratch->bytes_used, 0);
}

UTEST_F(context, nested_pop_from_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  // begin one scratch
  sp_mem_scratch_t s1 = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);
  EXPECT_GE(rt->scratch->bytes_used, 64);
  EXPECT_EQ(a[0], 0xAA);

  // begin a nested scratch
  sp_mem_begin_scratch();
  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(rt->scratch->bytes_used, 128);
  EXPECT_EQ(b[0], 0xBB);

  // verify that you're able to bypass the nested scratch and restore to the original
  sp_mem_end_scratch(s1);

  EXPECT_EQ(rt->scratch->bytes_used, 0);
}


s32 push_pop_thread_func(void *userdata) {
  push_pop_thread_data_t *data = (push_pop_thread_data_t *)userdata;
  data->all_passed = true;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_tls_rt_t *state = sp_tls_rt_get();
    u32 initial_index = state->index;

    sp_context_t ctx = *sp_context_get();
    sp_context_push(ctx);
    sp_context_push(ctx);

    if (state->index != initial_index + 2) {
      data->all_passed = false;
      break;
    }

    sp_context_pop();
    sp_context_pop();

    if (state->index != initial_index) {
      data->all_passed = false;
      break;
    }
  }

  sp_atomic_s32_add(data->done_count, 1);
  return 0;
}

UTEST_F(context, multithread_push_pop) {
  sp_atomic_s32 done_count = 0;
  push_pop_thread_data_t thread_data[NUM_THREADS];
  sp_thread_t threads[NUM_THREADS];

  for (s32 i = 0; i < NUM_THREADS; i++) {
    thread_data[i].done_count = &done_count;
    thread_data[i].thread_id = i;
    thread_data[i].iterations = 100;
    thread_data[i].all_passed = false;
    sp_thread_init(&threads[i], push_pop_thread_func, &thread_data[i]);
  }

  for (s32 i = 0; i < NUM_THREADS; i++) {
    sp_thread_join(&threads[i]);
  }

  EXPECT_EQ(sp_atomic_s32_get(&done_count), NUM_THREADS);

  for (s32 i = 0; i < NUM_THREADS; i++) {
    EXPECT_TRUE(thread_data[i].all_passed);
  }
}


#ifndef SP_MEM_ALIGNMENT
  #define SP_MEM_ALIGNMENT 16
#endif


void* context(void* ptr) {
  return sp_align_up(ptr, SP_MEM_ALIGNMENT);
}

#define EXPECT_ALIGNED(ptr) EXPECT_EQ(context(ptr), ptr)

UTEST_F(context, malloc_wrapper_is_aligned) {
  void *p1 = sp_alloc(1);
  void *p2 = sp_alloc(8);
  void *p3 = sp_alloc(16);

  EXPECT_ALIGNED(p1);
  EXPECT_ALIGNED(p2);
  EXPECT_ALIGNED(p3);

  sp_free(p1);
  sp_free(p2);
  sp_free(p3);
}

UTEST_F(context, arena_padding_after_byte) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  u8* byte = sp_alloc(1);

  u8* word = sp_alloc(8);

  EXPECT_ALIGNED(byte);
  EXPECT_ALIGNED(word);
  EXPECT_TRUE((uintptr_t)word >= (uintptr_t)byte + 1);

  sp_mem_end_scratch(scratch);
}

UTEST_F(context, arena_padding_mixed_sizes) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  u8* p1 = sp_alloc(3);
  u8* p2 = sp_alloc(13);
  u8* p3 = sp_alloc(8);

  EXPECT_ALIGNED(p1);
  EXPECT_ALIGNED(p2);
  EXPECT_ALIGNED(p3);

  sp_mem_end_scratch(scratch);
}

#define ALLOCATED_SIZE(size) ((size) + sizeof(sp_arena_alloc_header_t))
UTEST_F(context, arena_capacity_check_includes_padding) {
  u32 capacity = SP_MEM_ALIGNMENT + 4 + (2 * sizeof(sp_arena_alloc_header_t));
  sp_mem_arena_t *arena = sp_mem_arena_new(capacity);
  u8* original_buffer = arena->buffer;
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);
  EXPECT_EQ(arena->bytes_used, 0);
  EXPECT_EQ(arena->capacity, capacity);

  // allocate a single byte, which forces the arena to use SP_MEM_ALIGNMENT - 1 bytes of padding
  void* pa = sp_mem_allocator_alloc(allocator, 1);
  EXPECT_ALIGNED(pa);
  EXPECT_EQ(arena->bytes_used, ALLOCATED_SIZE(1));
  EXPECT_EQ(arena->capacity, capacity);
  EXPECT_EQ(pa, original_buffer + sizeof(sp_arena_alloc_header_t));

  // previous allocation used (header) + (1 byte allocation) + (15 bytes of padding)
  u32 bytes_remaining = arena->capacity - sizeof(sp_arena_alloc_header_t) - SP_MEM_ALIGNMENT;

  // the next allocation requires a header; we can use whatever's left.
  u32 bytes_available = bytes_remaining - sizeof(sp_arena_alloc_header_t);


  EXPECT_GT(bytes_remaining, 8);
  EXPECT_LT(bytes_available, 8);

  // allocate 8 bytes; the arena should have 4 bytes available after padding, but more than 8 if not padding
  // this forces a realloc due to alignment padding
  void* pb = sp_mem_allocator_alloc(allocator, 8);
  EXPECT_ALIGNED(pb);

  // verify the arena resized
  EXPECT_GT(arena->capacity, capacity);

  sp_mem_arena_destroy(arena);
}

UTEST_F(context, arena_realloc_does_not_read_past_old_size) {
  sp_mem_arena_t* arena = sp_mem_arena_new(256);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  // Allocate 16 bytes, fill with 0xAA
  u8* first = sp_mem_allocator_alloc(allocator, 16);
  sp_mem_fill_u8(first, 16, 0xAA);

  // Allocate 16 bytes right after, fill with 0xBB
  u8* second = sp_mem_allocator_alloc(allocator, 16);
  sp_mem_fill_u8(second, 16, 0xBB);

  // Realloc first to 32 bytes
  u8* resized = sp_mem_allocator_realloc(allocator, first, 32);

  // First 16 bytes should be 0xAA (copied from original)
  EXPECT_EQ(resized[0], 0xAA);
  EXPECT_EQ(resized[15], 0xAA);
  // Bytes 16-31 should be zero-initialized (new memory), NOT 0xBB
  // If the bug exists, these will be 0xBB (garbage from 'second')
  EXPECT_EQ(resized[16], 0x00);
  EXPECT_EQ(resized[31], 0x00);

  sp_mem_arena_destroy(arena);
}
