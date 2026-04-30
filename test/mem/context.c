#include "mem.h"
#include "sp.h"

UTEST_F(mem, get_returns_non_null) {
  sp_context_t *ctx = sp_context_get();
  EXPECT_TRUE(ctx != SP_NULLPTR);
}

UTEST_F(mem, allocator_is_valid) {
  sp_context_t *ctx = sp_context_get();
  EXPECT_TRUE(ctx->allocator.on_alloc != SP_NULLPTR);
}

UTEST_F(mem, thread_state_index_starts_at_zero) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  EXPECT_EQ(state->index, 0u);
}

UTEST_F(mem, push_pop_single) {
  sp_tls_rt_t *state = sp_tls_rt_get();
  u32 initial_index = state->index;

  sp_context_t ctx = *sp_context_get();
  sp_context_push(ctx);

  EXPECT_EQ(state->index, initial_index + 1);

  sp_context_pop();
  EXPECT_EQ(state->index, initial_index);
}

UTEST_F(mem, push_pop_multiple) {
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

UTEST_F(mem, push_allocator_changes_allocator) {
  sp_context_t *ctx_before = sp_context_get();
  sp_mem_t old_allocator = ctx_before->allocator;

  sp_mem_t new_allocator = sp_mem_os_new();
  sp_context_push_allocator(new_allocator);

  sp_context_t *ctx_after = sp_context_get();
  EXPECT_TRUE(ctx_after->allocator.on_alloc == new_allocator.on_alloc);

  sp_context_pop();

  sp_context_t *ctx_restored = sp_context_get();
  EXPECT_TRUE(ctx_restored->allocator.on_alloc == old_allocator.on_alloc);
}

UTEST_F(mem, set_modifies_current) {
  sp_context_t ctx = *sp_context_get();
  sp_mem_t new_allocator = sp_mem_os_new();
  ctx.allocator = new_allocator;

  sp_context_set(ctx);

  sp_context_t *current = sp_context_get();
  EXPECT_TRUE(current->allocator.on_alloc == new_allocator.on_alloc);
}

UTEST_F(mem, scratch_initted) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();
  EXPECT_TRUE(arena->head != SP_NULLPTR);
  EXPECT_GT(sp_mem_arena_capacity(arena), 0u);
}

UTEST_F(mem, begin_scratch) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_TRUE(scratch.marker.arena != SP_NULLPTR);
}

UTEST_F(mem, end_scratch) {
  sp_mem_arena_t *arena = sp_mem_get_scratch_arena();

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);

  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, 1024, SP_NULLPTR);
  EXPECT_GE(sp_mem_arena_bytes_used(arena), 1024u);

  sp_alloc(1024);
  EXPECT_GE(sp_mem_arena_bytes_used(arena), 2048u);

  sp_mem_end_scratch(scratch);
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);
}

typedef struct {
  sp_atomic_s32_t *done_count;
  s32 thread_id;
  bool context_valid;
  bool allocator_works;
  bool independent_context;
  bool scratch_zeroed;
} context_thread_data_t;

static sp_context_t *main_thread_context = SP_NULLPTR;

static s32 context_thread_func(void *userdata) {
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

UTEST_F(mem, multithread_independent_contexts) {
#if defined(SP_FREESTANDING)
  UTEST_SKIP("threads not available in freestanding");
#endif
  main_thread_context = sp_context_get();

  sp_atomic_s32_t done_count = 0;
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
  sp_atomic_s32_t *done_count;
  s32 thread_id;
  s32 iterations;
  bool all_passed;
} push_pop_thread_data_t;

UTEST_F(mem, push_does_not_overwrite_scratch) {
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


UTEST_F(mem, nested_begin_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  struct {
    sp_mem_scratch_t a;
    sp_mem_scratch_t b;
  } s = sp_zero();

  s.a = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);

  s.b = sp_mem_begin_scratch();
  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);

  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  EXPECT_NE(a, b);

  sp_mem_end_scratch(s.b);
  EXPECT_EQ(a[0], 0xAA);

  sp_mem_end_scratch(s.a);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_carr_for(tls->scratch, it) {
    sp_mem_arena_t* arena = tls->scratch[it];
    EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0u);
  }
}

UTEST_F(mem, begin_scratch_push_unrelated_allocator_end_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  sp_mem_scratch_t s = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);

  sp_context_push_allocator(sp_mem_os_new());

  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(sp_mem_arena_bytes_used(s.marker.arena), 64u);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(b[0], 0xBB);
  sp_free(b);

  sp_context_pop();

  u8* c = sp_alloc(64);
  sp_mem_fill_u8(c, 64, 0xCC);
  EXPECT_GE(sp_mem_arena_bytes_used(s.marker.arena), 128u);
  EXPECT_EQ(a[0], 0xAA);
  EXPECT_EQ(c[0], 0xCC);

  sp_mem_end_scratch(s);
  EXPECT_EQ(sp_mem_arena_bytes_used(s.marker.arena), 0u);
}

UTEST_F(mem, nested_pop_from_scratch) {
  sp_tls_rt_t* rt = sp_tls_rt_get();

  struct {
    sp_mem_scratch_t a;
    sp_mem_scratch_t b;
  } s = sp_zero();

  s.a = sp_mem_begin_scratch();
  u8* a = sp_alloc(64);
  sp_mem_fill_u8(a, 64, 0xAA);
  EXPECT_GE(sp_mem_arena_bytes_used(s.a.marker.arena), 64u);
  EXPECT_EQ(a[0], 0xAA);

  s.b = sp_mem_begin_scratch();
  u8* b = sp_alloc(64);
  sp_mem_fill_u8(b, 64, 0xBB);
  EXPECT_GE(get_total_scratch_bytes_used(), 128u);
  EXPECT_EQ(b[0], 0xBB);

  sp_mem_end_scratch(s.b);
  sp_mem_end_scratch(s.a);

  EXPECT_EQ(get_total_scratch_bytes_used(), 0u);
}

static s32 push_pop_thread_func(void *userdata) {
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

UTEST_F(mem, multithread_push_pop) {
#if defined(SP_FREESTANDING)
  UTEST_SKIP("threads not available in freestanding");
#endif
  sp_atomic_s32_t done_count = 0;
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

UTEST_F(mem, malloc_wrapper_is_aligned) {
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
