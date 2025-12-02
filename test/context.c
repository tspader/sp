#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

struct sp_test_context {

};

UTEST_F_SETUP(sp_test_context) {
  pthread_setspecific(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

UTEST_F_TEARDOWN(sp_test_context) {
  pthread_setspecific(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

UTEST_F(sp_test_context, get_returns_non_null) {
  sp_context_t *ctx = sp_context_get();
  ASSERT_TRUE(ctx != NULL);
}

UTEST_F(sp_test_context, allocator_is_valid) {
  sp_context_t *ctx = sp_context_get();
  ASSERT_TRUE(ctx->allocator.on_alloc != NULL);
}

UTEST_F(sp_test_context, thread_state_index_starts_at_zero) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  ASSERT_EQ(state->index, 0);
}

UTEST_F(sp_test_context, push_pop_single) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();
  sp_context_push(ctx);

  ASSERT_EQ(state->index, initial_index + 1);

  sp_context_pop();
  ASSERT_EQ(state->index, initial_index);
}

UTEST_F(sp_test_context, push_pop_multiple) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();

  sp_context_push(ctx);
  sp_context_push(ctx);
  sp_context_push(ctx);

  ASSERT_EQ(state->index, initial_index + 3);

  sp_context_pop();
  sp_context_pop();
  sp_context_pop();

  ASSERT_EQ(state->index, initial_index);
}

UTEST_F(sp_test_context, push_allocator_changes_allocator) {
  sp_context_t *ctx_before = sp_context_get();
  sp_allocator_t old_allocator = ctx_before->allocator;

  sp_allocator_t new_allocator = sp_mem_libc_new();
  sp_context_push_allocator(new_allocator);

  sp_context_t *ctx_after = sp_context_get();

  // Allocator should be updated
  ASSERT_TRUE(ctx_after->allocator.on_alloc == new_allocator.on_alloc);

  sp_context_pop();

  // After pop, old allocator restored
  sp_context_t *ctx_restored = sp_context_get();
  ASSERT_TRUE(ctx_restored->allocator.on_alloc == old_allocator.on_alloc);
}

UTEST_F(sp_test_context, set_modifies_current) {
  sp_context_t ctx = *sp_context_get();
  sp_allocator_t new_allocator = sp_mem_libc_new();
  ctx.allocator = new_allocator;

  sp_context_set(ctx);

  sp_context_t *current = sp_context_get();
  ASSERT_TRUE(current->allocator.on_alloc == new_allocator.on_alloc);
}

UTEST_F(sp_test_context, scratch_initted) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  ASSERT_TRUE(arena->buffer != NULL);
  ASSERT_EQ(arena->capacity, SP_RT_SCRATCH_SIZE);
}

UTEST_F(sp_test_context, begin_scratch) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  ASSERT_TRUE(scratch.marker.arena != NULL);
}

UTEST_F(sp_test_context, end_scratch) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_t *ctx = sp_context_get();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_EQ(arena->bytes_used, 0);

  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, 1024, NULL);
  EXPECT_EQ(arena->bytes_used, 1024);

  sp_alloc(1024);
  EXPECT_EQ(arena->bytes_used, 2048);

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

UTEST_F(sp_test_context, use_scratch_allocator_but_return_from_user_allocator) {
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

UTEST_F(sp_test_context, multithread_independent_contexts) {
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

  ASSERT_EQ(sp_atomic_s32_get(&done_count), NUM_THREADS);

  for (s32 i = 0; i < NUM_THREADS; i++) {
    ASSERT_TRUE(thread_data[i].context_valid);
    ASSERT_TRUE(thread_data[i].allocator_works);
    ASSERT_TRUE(thread_data[i].independent_context);
    ASSERT_TRUE(thread_data[i].scratch_zeroed);
  }
}

typedef struct {
  sp_atomic_s32 *done_count;
  s32 thread_id;
  s32 iterations;
  bool all_passed;
} push_pop_thread_data_t;

UTEST_F(sp_test_context, push_does_not_overwrite_scratch) {
  // Use scratch arena
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  // Allocate and fill with 0xAA
  u8 *first = sp_alloc(64);
  sp_mem_fill_u8(first, 64, 0x01);
  ASSERT_EQ(first[0], 0x01);

  // Push context with scratch arena as allocator
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));

  // Allocate again - BUG: this overwrites 'first' because child context
  // has bytes_used=0 but same buffer pointer
  u8 *second = sp_alloc(64);
  sp_mem_fill_u8(second, 64, 0x02);

  // first should still be 0xAA, but due to bug it's now 0xBB
  ASSERT_EQ(first[0], 0x01);

  sp_context_pop();
  sp_mem_end_scratch(scratch);
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

UTEST_F(sp_test_context, multithread_push_pop) {
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

  ASSERT_EQ(sp_atomic_s32_get(&done_count), NUM_THREADS);

  for (s32 i = 0; i < NUM_THREADS; i++) {
    ASSERT_TRUE(thread_data[i].all_passed);
  }
}

SP_TEST_MAIN()
