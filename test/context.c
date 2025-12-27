#include "sp.h"

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
  EXPECT_TRUE(ctx != SP_NULLPTR);
}

UTEST_F(context, allocator_is_valid) {
  sp_context_t *ctx = sp_context_get();
  EXPECT_TRUE(ctx->allocator.on_alloc != SP_NULLPTR);
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
  EXPECT_TRUE(ctx_after->allocator.on_alloc == new_allocator.on_alloc);

  sp_context_pop();

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
  EXPECT_TRUE(arena->head != SP_NULLPTR);
  EXPECT_GT(sp_mem_arena_capacity(arena), 0);
}

UTEST_F(context, begin_scratch) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_TRUE(scratch.marker.arena != SP_NULLPTR);
}

UTEST_F(context, end_scratch) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, 1024, SP_NULLPTR);
  EXPECT_GE(sp_mem_arena_bytes_used(arena), 1024);

  sp_alloc(1024);
  EXPECT_GE(sp_mem_arena_bytes_used(arena), 2048);

  sp_mem_end_scratch(scratch);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);
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
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);

  u8 *buffer = use_scratch_arena(69);
  EXPECT_EQ(buffer[0], 69);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);
}

typedef struct {
  sp_atomic_s32 *done_count;
  s32 thread_id;
  bool context_valid;
  bool allocator_works;
  bool independent_context;
  bool scratch_zeroed;
} context_thread_data_t;

static sp_context_t *main_thread_context = SP_NULLPTR;

s32 context_thread_func(void *userdata) {
  context_thread_data_t *data = (context_thread_data_t *)userdata;

  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_t *ctx = sp_context_get();
  data->context_valid = (ctx != SP_NULLPTR);
  data->independent_context = (ctx != main_thread_context);
  data->scratch_zeroed = (sp_mem_arena_bytes_used(arena) == 0);

  void *p = sp_alloc(64);
  data->allocator_works = (p != SP_NULLPTR);
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
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  u8 *first = sp_alloc(64);
  sp_mem_fill_u8(first, 64, 0x01);
  EXPECT_EQ(first[0], 0x01);

  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));

  u8 *second = sp_alloc(64);
  sp_mem_fill_u8(second, 64, 0x02);

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
  EXPECT_EQ(sp_mem_arena_bytes_used(rt->scratch), 0);
}

UTEST_F(context, begin_scratch_push_unrelated_allocator_end_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);

  sp_context_push_allocator(sp_mem_libc_new());

  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(sp_mem_arena_bytes_used(rt->scratch), 64);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  sp_free(b);

  sp_context_pop();

  u8* c = sp_alloc(64);
  sp_mem_fill_u8(c, 64, 0xCC);
  EXPECT_GE(sp_mem_arena_bytes_used(rt->scratch), 128);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(c[0], 0xCC);

  sp_mem_end_scratch(scratch);
  EXPECT_EQ(sp_mem_arena_bytes_used(rt->scratch), 0);
}

UTEST_F(context, nested_pop_from_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  sp_mem_scratch_t s1 = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);
  EXPECT_GE(sp_mem_arena_bytes_used(rt->scratch), 64);
  EXPECT_EQ(a[0], 0xAA);

  sp_mem_begin_scratch();
  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(sp_mem_arena_bytes_used(rt->scratch), 128);
  EXPECT_EQ(b[0], 0xBB);

  sp_mem_end_scratch(s1);

  EXPECT_EQ(sp_mem_arena_bytes_used(rt->scratch), 0);
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

UTEST_F(context, arena_basic_alloc) {
  sp_mem_arena_t* arena = sp_mem_arena_new(256);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);
  EXPECT_EQ(sp_mem_arena_capacity(arena), 256);

  u8* first = sp_mem_allocator_alloc(allocator, 16);
  EXPECT_ALIGNED(first);
  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_allocations_are_zeroed) {
  sp_mem_arena_t* arena = sp_mem_arena_new(256);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_mem_allocator_alloc(allocator, 64);
  EXPECT_EQ(first[0], 0x00);
  EXPECT_EQ(first[63], 0x00);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_pop_resets_bytes_used) {
  sp_mem_arena_t* arena = sp_mem_arena_new(256);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  u8* first = sp_mem_allocator_alloc(allocator, 64);
  sp_mem_fill_u8(first, 64, 0x69);
  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_pop(marker);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_block_chaining) {
  sp_mem_arena_t* arena = sp_mem_arena_new(64);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  u8* a = sp_mem_allocator_alloc(allocator, 32);
  u8* b = sp_mem_allocator_alloc(allocator, 32);
  u8* c = sp_mem_allocator_alloc(allocator, 32);

  EXPECT_ALIGNED(a);
  EXPECT_ALIGNED(b);
  EXPECT_ALIGNED(c);

  sp_mem_fill_u8(a, 32, 0xAA);
  sp_mem_fill_u8(b, 32, 0xBB);
  sp_mem_fill_u8(c, 32, 0xCC);

  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  EXPECT_EQ(c[0], 0xCC);

  EXPECT_GT(sp_mem_arena_capacity(arena), 64);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_pop_across_blocks) {
  sp_mem_arena_t* arena = sp_mem_arena_new(64);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  u8* a = sp_mem_allocator_alloc(allocator, 32);
  u8* b = sp_mem_allocator_alloc(allocator, 32);
  u8* c = sp_mem_allocator_alloc(allocator, 32);

  sp_mem_fill_u8(a, 32, 0xAA);
  sp_mem_fill_u8(b, 32, 0xBB);
  sp_mem_fill_u8(c, 32, 0xCC);

  u32 used_before = sp_mem_arena_bytes_used(arena);
  EXPECT_GT(used_before, 0);

  sp_mem_arena_pop(marker);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_realloc_copies_data) {
  sp_mem_arena_t* arena = sp_mem_arena_new(256);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  u8* first = sp_mem_allocator_alloc(allocator, 16);
  sp_mem_fill_u8(first, 16, 0xAA);

  u8* resized = sp_mem_allocator_realloc(allocator, first, 32);

  EXPECT_EQ(resized[0], 0xAA);
  EXPECT_EQ(resized[15], 0xAA);
  EXPECT_EQ(resized[16], 0x00);
  EXPECT_EQ(resized[31], 0x00);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_clear_resets_all_blocks) {
  sp_mem_arena_t* arena = sp_mem_arena_new(64);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  EXPECT_GT(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_clear(arena);

  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_block_reuse_after_pop) {
  sp_mem_arena_t* arena = sp_mem_arena_new(64);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  sp_mem_arena_marker_t marker = sp_mem_arena_mark(arena);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  u32 capacity_after_allocs = sp_mem_arena_capacity(arena);

  sp_mem_arena_pop(marker);

  EXPECT_EQ(sp_mem_arena_capacity(arena), capacity_after_allocs);

  sp_mem_allocator_alloc(allocator, 32);
  sp_mem_allocator_alloc(allocator, 32);

  EXPECT_EQ(sp_mem_arena_capacity(arena), capacity_after_allocs);

  sp_mem_arena_free(arena);
}

UTEST_F(context, arena_reuse_logic_check) {
  // 1. Setup: Default block size 64
  sp_mem_arena_t* arena = sp_mem_arena_new(64);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);

  // 2. Fill Block A
  // Header (16) + Alloc (40) = 56 bytes used. Remaining 8.
  sp_mem_allocator_alloc(allocator, 40);

  // 3. Force creation of Block B
  // Won't fit in A. Creates B. Used: 56.
  sp_mem_allocator_alloc(allocator, 40);

  // Snapshot capacity. Should be 128 (64 + 64).
  u32 cap_initial = sp_mem_arena_capacity(arena);
  EXPECT_EQ(cap_initial, 128);

  // 4. Reset arena (pointers go back to Block A)
  sp_mem_arena_clear(arena);

  // 5. Fill Block A again
  sp_mem_allocator_alloc(allocator, 40);

  // 6. Trigger Reuse of Block B
  // If the bug exists (stale bytes_used respected), Block B looks full (56 used).
  // The allocator would incorrectly skip Block B and allocate Block C.
  sp_mem_allocator_alloc(allocator, 40);

  u32 cap_after = sp_mem_arena_capacity(arena);

  // FAILURE CONDITION: If cap_after > 128, the arena leaked a block instead of reusing.
  EXPECT_EQ(cap_after, 128);

  sp_mem_arena_free(arena);
}
