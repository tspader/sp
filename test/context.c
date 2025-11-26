#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

//////////////////////////
// BASIC CONTEXT TESTS  //
//////////////////////////

UTEST(sp_context, get_returns_non_null) {
  sp_context_t* ctx = sp_context_get();
  ASSERT_TRUE(ctx != NULL);
}

UTEST(sp_context, allocator_is_valid) {
  sp_context_t* ctx = sp_context_get();
  ASSERT_TRUE(ctx->allocator.on_alloc != NULL);
}

UTEST(sp_context, thread_state_index_starts_at_zero) {
  sp_thread_state_t* state = sp_thread_state_get();
  ASSERT_EQ(state->index, 0);
}

//////////////////////////
// CONTEXT STACK TESTS  //
//////////////////////////

UTEST(sp_context, push_pop_single) {
  sp_thread_state_t* state = sp_thread_state_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();
  sp_context_push(ctx);

  ASSERT_EQ(state->index, initial_index + 1);

  sp_context_pop();
  ASSERT_EQ(state->index, initial_index);
}

UTEST(sp_context, push_pop_multiple) {
  sp_thread_state_t* state = sp_thread_state_get();
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

UTEST(sp_context, push_allocator_changes_allocator) {
  sp_context_t* ctx_before = sp_context_get();
  sp_allocator_t old_allocator = ctx_before->allocator;

  sp_allocator_t new_allocator = sp_mem_libc_allocator_t_init();
  sp_context_push_allocator(new_allocator);

  sp_context_t* ctx_after = sp_context_get();

  // Allocator should be updated
  ASSERT_TRUE(ctx_after->allocator.on_alloc == new_allocator.on_alloc);

  sp_context_pop();

  // After pop, old allocator restored
  sp_context_t* ctx_restored = sp_context_get();
  ASSERT_TRUE(ctx_restored->allocator.on_alloc == old_allocator.on_alloc);
}

UTEST(sp_context, set_modifies_current) {
  sp_context_t ctx = *sp_context_get();
  sp_allocator_t new_allocator = sp_mem_libc_allocator_t_init();
  ctx.allocator = new_allocator;

  sp_context_set(ctx);

  sp_context_t* current = sp_context_get();
  ASSERT_TRUE(current->allocator.on_alloc == new_allocator.on_alloc);
}

//////////////////////////
// SCRATCH ARENA TESTS  //
//////////////////////////

UTEST(sp_context, scratch_initialized_after_sp_init) {
  sp_init(SP_ZERO_STRUCT(sp_config_t));
  sp_context_t* ctx = sp_context_get();
  ASSERT_TRUE(ctx->scratch.buffer != NULL);
  ASSERT_EQ(ctx->scratch.capacity, SP_SCRATCH_SIZE);
}

UTEST(sp_mem_mark, returns_valid_marker) {
  sp_init(SP_ZERO_STRUCT(sp_config_t));
  sp_mem_arena_marker_t marker = sp_mem_mark();
  ASSERT_TRUE(marker.arena != NULL);
}

UTEST(sp_mem_mark, pop_resets_position) {
  sp_init(SP_ZERO_STRUCT(sp_config_t));
  sp_context_t* ctx = sp_context_get();

  sp_mem_arena_marker_t marker = sp_mem_mark();
  u32 bytes_before = ctx->scratch.bytes_used;

  // Allocate some memory from scratch arena
  sp_mem_arena_on_alloc(&ctx->scratch, SP_ALLOCATOR_MODE_ALLOC, 1024, NULL);
  ASSERT_TRUE(ctx->scratch.bytes_used > bytes_before);

  sp_mem_pop(marker);
  ASSERT_EQ(ctx->scratch.bytes_used, bytes_before);
}

//////////////////////////
// MULTITHREADED TESTS  //
//////////////////////////

typedef struct {
  sp_atomic_s32* done_count;
  s32 thread_id;
  bool context_valid;
  bool allocator_works;
  bool independent_context;
} context_thread_data_t;

static sp_context_t* main_thread_context = NULL;

s32 context_thread_func(void* userdata) {
  context_thread_data_t* data = (context_thread_data_t*)userdata;

  // Verify context is properly initialized for this thread
  sp_context_t* ctx = sp_context_get();
  data->context_valid = (ctx != NULL);

  // Verify this is a different context than main thread
  data->independent_context = (ctx != main_thread_context);

  // Verify allocator works
  void* p = sp_alloc(64);
  data->allocator_works = (p != NULL);
  sp_free(p);

  sp_atomic_s32_add(data->done_count, 1);
  return 0;
}

#define NUM_THREADS 8

UTEST(sp_context, multithread_independent_contexts) {
  main_thread_context = sp_context_get();

  sp_atomic_s32 done_count = 0;
  context_thread_data_t thread_data[NUM_THREADS];
  sp_thread_t threads[NUM_THREADS];

  for (s32 i = 0; i < NUM_THREADS; i++) {
    thread_data[i].done_count = &done_count;
    thread_data[i].thread_id = i;
    thread_data[i].context_valid = false;
    thread_data[i].allocator_works = false;
    thread_data[i].independent_context = false;
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
  }
}

typedef struct {
  sp_atomic_s32* done_count;
  s32 thread_id;
  s32 iterations;
  bool all_passed;
} push_pop_thread_data_t;

s32 push_pop_thread_func(void* userdata) {
  push_pop_thread_data_t* data = (push_pop_thread_data_t*)userdata;
  data->all_passed = true;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_thread_state_t* state = sp_thread_state_get();
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

UTEST(sp_context, multithread_push_pop) {
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

UTEST_MAIN()
