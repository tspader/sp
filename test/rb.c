#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct sp_rb {
  sp_mem_arena_t* arena;
  sp_mem_t mem;
};

UTEST_F_SETUP(sp_rb) {
  ut.arena = sp_mem_arena_new(sp_mem_os_new());
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
}

UTEST_F_TEARDOWN(sp_rb) {
  sp_mem_arena_destroy(ut.arena);
}

UTEST_F(sp_rb, null_init) {
  sp_rb(int) q = SP_NULLPTR;

  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_EQ(0, sp_rb_capacity(q));
  EXPECT_TRUE(sp_rb_empty(q));
  EXPECT_TRUE(sp_rb_full(q));
}

UTEST_F(sp_rb, clear) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init_cap(ut.mem, q, 8);

  ASSERT_TRUE(q != SP_NULLPTR);
  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_EQ(8, sp_rb_capacity(q));

  sp_rb_head(q)->size = 3;
  sp_rb_head(q)->head = 2;
  EXPECT_EQ(3, sp_rb_size(q));

  sp_rb_clear(q);
  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_EQ(0, sp_rb_head(q)->head);
  EXPECT_EQ(8, sp_rb_capacity(q));
  EXPECT_TRUE(sp_rb_empty(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, free_test) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init_cap(ut.mem, q, 8);

  ASSERT_TRUE(q != SP_NULLPTR);

  sp_rb_free(ut.mem, q);
  EXPECT_TRUE(q == SP_NULLPTR);

  sp_rb_free(ut.mem, q);
  EXPECT_TRUE(q == SP_NULLPTR);
}

UTEST_F(sp_rb, push_pop_fifo) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 10);
  sp_rb_push(ut.mem, q, 20);
  sp_rb_push(ut.mem, q, 30);

  EXPECT_EQ(3, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_capacity(q) >= 3);

  EXPECT_EQ(10, *sp_rb_peek(q));
  EXPECT_EQ(30, *sp_rb_back(q));

  sp_rb_pop(q);
  EXPECT_EQ(2, sp_rb_size(q));
  EXPECT_EQ(20, *sp_rb_peek(q));

  sp_rb_pop(q);
  EXPECT_EQ(1, sp_rb_size(q));
  EXPECT_EQ(30, *sp_rb_peek(q));

  sp_rb_pop(q);
  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_empty(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, full_and_grow) {
  sp_rb(s32) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 0);
  u32 initial_cap = sp_rb_capacity(q);

  for (u32 i = 1; i < initial_cap; i++) {
    sp_rb_push(ut.mem, q, (s32)(i * 10));
  }

  EXPECT_EQ(initial_cap, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_full(q));

  sp_rb_push(ut.mem, q, (s32)(initial_cap * 10));
  EXPECT_EQ(initial_cap + 1, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_capacity(q) > initial_cap);
  EXPECT_FALSE(sp_rb_full(q));

  EXPECT_EQ(0, *sp_rb_peek(q));
  EXPECT_EQ((s32)(initial_cap * 10), *sp_rb_back(q));

  for (u32 i = 0; i <= initial_cap; i++) {
    EXPECT_EQ((s32)(i * 10), sp_rb_at(q, i));
  }

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, wrap_access) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 1);
  sp_rb_push(ut.mem, q, 2);
  sp_rb_push(ut.mem, q, 3);
  sp_rb_push(ut.mem, q, 4);

  sp_rb_pop(q);
  sp_rb_pop(q);

  sp_rb_push(ut.mem, q, 5);
  sp_rb_push(ut.mem, q, 6);
  sp_rb_push(ut.mem, q, 7);
  sp_rb_push(ut.mem, q, 8);
  sp_rb_push(ut.mem, q, 9);
  sp_rb_push(ut.mem, q, 10);

  EXPECT_EQ(8, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_full(q));

  EXPECT_EQ(3, sp_rb_at(q, 0));
  EXPECT_EQ(4, sp_rb_at(q, 1));
  EXPECT_EQ(5, sp_rb_at(q, 2));
  EXPECT_EQ(10, sp_rb_at(q, 7));

  EXPECT_EQ(3, *sp_rb_peek(q));
  EXPECT_EQ(10, *sp_rb_back(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, empty_ops) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_pop(q);
  EXPECT_TRUE(sp_rb_empty(q));
  EXPECT_TRUE(sp_rb_peek(q) == SP_NULLPTR);
  EXPECT_TRUE(sp_rb_back(q) == SP_NULLPTR);

  sp_rb_push(ut.mem, q, 42);
  sp_rb_pop(q);

  EXPECT_TRUE(sp_rb_empty(q));
  EXPECT_TRUE(sp_rb_peek(q) == SP_NULLPTR);
  EXPECT_TRUE(sp_rb_back(q) == SP_NULLPTR);

  sp_rb_pop(q);
  EXPECT_TRUE(sp_rb_empty(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, iterate_wrapped) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 1);
  sp_rb_push(ut.mem, q, 2);
  sp_rb_push(ut.mem, q, 3);
  sp_rb_push(ut.mem, q, 4);

  sp_rb_pop(q);
  sp_rb_pop(q);

  sp_rb_push(ut.mem, q, 5);
  sp_rb_push(ut.mem, q, 6);

  int expected[] = {3, 4, 5, 6};
  int idx = 0;

  sp_rb_for(q, it) {
    EXPECT_EQ(expected[idx], sp_rb_at(q, it));
    idx++;
  }

  EXPECT_EQ(4, idx);

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, iterate_reverse) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 10);
  sp_rb_push(ut.mem, q, 20);
  sp_rb_push(ut.mem, q, 30);
  sp_rb_push(ut.mem, q, 40);

  int expected[] = {40, 30, 20, 10};
  int idx = 0;

  sp_rb_rfor(q, it) {
    EXPECT_EQ(expected[idx], sp_rb_at(q, it));
    idx++;
  }

  EXPECT_EQ(4, idx);

  sp_rb_free(ut.mem, q);
}

typedef struct {
  s32 id;
  float x;
  float y;
  u64 flags;
} test_item_t;

UTEST_F(sp_rb, struct_type) {
  sp_rb(test_item_t) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  test_item_t a = {.id = 1, .x = 1.5f, .y = 2.5f, .flags = 0xDEADBEEF};
  test_item_t b = {.id = 2, .x = 3.5f, .y = 4.5f, .flags = 0xCAFEBABE};
  test_item_t c = {.id = 3, .x = 5.5f, .y = 6.5f, .flags = 0x12345678};

  sp_rb_push(ut.mem, q, a);
  sp_rb_push(ut.mem, q, b);
  sp_rb_push(ut.mem, q, c);

  test_item_t* p = sp_rb_peek(q);
  EXPECT_EQ(1, p->id);
  EXPECT_TRUE(p->x == 1.5f);
  EXPECT_TRUE(p->y == 2.5f);
  EXPECT_EQ(0xDEADBEEF, p->flags);

  sp_rb_pop(q);

  p = sp_rb_peek(q);
  EXPECT_EQ(2, p->id);
  EXPECT_TRUE(p->x == 3.5f);
  EXPECT_TRUE(p->y == 4.5f);
  EXPECT_EQ(0xCAFEBABE, p->flags);

  p = sp_rb_back(q);
  EXPECT_EQ(3, p->id);
  EXPECT_TRUE(p->x == 5.5f);
  EXPECT_TRUE(p->y == 6.5f);
  EXPECT_EQ(0x12345678, p->flags);

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, capacity_one) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init_cap(ut.mem, q, 1);

  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_EQ(1, sp_rb_capacity(q));
  EXPECT_TRUE(sp_rb_empty(q));
  EXPECT_FALSE(sp_rb_full(q));

  sp_rb_push(ut.mem, q, 42);
  EXPECT_EQ(1, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_full(q));
  EXPECT_EQ(42, *sp_rb_peek(q));
  EXPECT_EQ(42, *sp_rb_back(q));

  sp_rb_pop(q);
  EXPECT_TRUE(sp_rb_empty(q));
  EXPECT_TRUE(sp_rb_peek(q) == SP_NULLPTR);

  sp_rb_push(ut.mem, q, 100);
  EXPECT_EQ(100, *sp_rb_peek(q));

  sp_rb_push(ut.mem, q, 200);
  EXPECT_TRUE(sp_rb_capacity(q) > 1);
  EXPECT_EQ(2, sp_rb_size(q));
  EXPECT_EQ(100, *sp_rb_peek(q));
  EXPECT_EQ(200, *sp_rb_back(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, overwrite_mode) {
  sp_rb(s32) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 1);
  sp_rb_set_mode(q, SP_RQ_MODE_OVERWRITE);
  EXPECT_EQ(SP_RQ_MODE_OVERWRITE, sp_rb_mode(q));

  u32 cap = sp_rb_capacity(q);

  for (u32 i = 2; i <= cap; i++) {
    sp_rb_push(ut.mem, q, (s32)i);
  }

  EXPECT_TRUE(sp_rb_full(q));
  EXPECT_EQ(cap, sp_rb_capacity(q));

  sp_rb_push(ut.mem, q, (s32)(cap + 1));
  EXPECT_EQ(cap, sp_rb_capacity(q));
  EXPECT_EQ(cap, sp_rb_size(q));

  EXPECT_EQ(2, *sp_rb_peek(q));
  EXPECT_EQ((s32)(cap + 1), *sp_rb_back(q));

  sp_rb_push(ut.mem, q, (s32)(cap + 2));
  sp_rb_push(ut.mem, q, (s32)(cap + 3));

  EXPECT_EQ(4, *sp_rb_peek(q));
  EXPECT_EQ((s32)(cap + 3), *sp_rb_back(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, overwrite_preserves_order) {
  sp_rb(s32) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 0);
  sp_rb_set_mode(q, SP_RQ_MODE_OVERWRITE);
  u32 cap = sp_rb_capacity(q);

  for (u32 i = 1; i < cap * 2; i++) {
    sp_rb_push(ut.mem, q, (s32)i);
  }

  EXPECT_EQ(cap, sp_rb_size(q));

  u32 expected_start = cap * 2 - cap;
  sp_rb_for(q, it) {
    EXPECT_EQ((s32)(expected_start + it), sp_rb_at(q, it));
  }

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, mode_preserved_on_grow) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_push(ut.mem, q, 1);
  sp_rb_set_mode(q, SP_RQ_MODE_GROW);
  u32 cap = sp_rb_capacity(q);

  for (u32 i = 2; i <= cap + 1; i++) {
    sp_rb_push(ut.mem, q, (int)i);
  }

  EXPECT_TRUE(sp_rb_capacity(q) > cap);
  EXPECT_EQ(SP_RQ_MODE_GROW, sp_rb_mode(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, set_mode_after_init) {
  sp_rb(int) q = SP_NULLPTR;
  sp_rb_init(ut.mem, q);

  sp_rb_set_mode(q, SP_RQ_MODE_OVERWRITE);

  EXPECT_TRUE(q != SP_NULLPTR);
  EXPECT_EQ(SP_RQ_MODE_OVERWRITE, sp_rb_mode(q));
  EXPECT_EQ(0, sp_rb_size(q));
  EXPECT_TRUE(sp_rb_capacity(q) > 0);

  sp_rb_push(ut.mem, q, 1);
  EXPECT_EQ(1, sp_rb_size(q));

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, null_is_empty_and_push_allocates) {
  sp_rb(u64) q = SP_NULLPTR;

  EXPECT_EQ(sp_rb_size(q), 0u);
  EXPECT_TRUE(sp_rb_empty(q));
  sp_rb_pop(q);
  sp_rb_clear(q);
  sp_rb_free(ut.mem, q);

  ASSERT_EQ(sp_rb_push(ut.mem, q, 0x0123456789ABCDEFull), SP_OK);
  ASSERT_TRUE(q != SP_NULLPTR);
  EXPECT_EQ(sp_rb_size(q), 1u);
  EXPECT_EQ(*sp_rb_peek(q), 0x0123456789ABCDEFull);
  EXPECT_EQ(sp_rb_mode(q), SP_RQ_MODE_GROW);

  sp_rb_free(ut.mem, q);
  EXPECT_TRUE(q == SP_NULLPTR);
}

// The pushed expression may read the ring, and the ring keeps its order when a push has to
// grow it while it is wrapped
UTEST_F(sp_rb, push_value_reads_the_ring_across_growth) {
  sp_rb(u32) q = SP_NULLPTR;

  sp_for(it, 6) {
    ASSERT_EQ(sp_rb_push(ut.mem, q, sp_rb_size(q)), SP_OK);
  }
  sp_for(it, 4) {
    sp_rb_pop(q);
  }
  sp_for(it, 200) {
    ASSERT_EQ(sp_rb_push(ut.mem, q, *sp_rb_back(q) + 1), SP_OK);
  }

  EXPECT_EQ(sp_rb_size(q), 202u);
  sp_rb_for(q, it) {
    EXPECT_EQ(sp_rb_at(q, it), it + 4);
  }

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, push_returns_the_allocators_error) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_rb(u32) q = SP_NULLPTR;

  EXPECT_EQ(sp_rb_push(mem, q, 1), SP_ERR_SYS_BUSY);
  EXPECT_TRUE(q == SP_NULLPTR);

  failing.at = failing.count;
  EXPECT_EQ(sp_rb_init(mem, q), SP_ERR_SYS_BUSY);
  EXPECT_TRUE(q == SP_NULLPTR);

  EXPECT_EQ(sp_rb_push(mem, q, 2), SP_OK);
  EXPECT_EQ(*sp_rb_peek(q), 2u);
  sp_rb_free(mem, q);
}

static u32 rb_eval_counting(u32* count) {
  (*count)++;
  return 7;
}

static u32 rb_eval_seeing(void* q, void** seen) {
  *seen = q;
  return 7;
}

UTEST_F(sp_rb, failed_first_push_does_not_evaluate_the_value) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_rb(u32) q = SP_NULLPTR;
  u32 count = 0;

  EXPECT_EQ(sp_rb_push(mem, q, rb_eval_counting(&count)), SP_ERR_SYS_BUSY);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(q == SP_NULLPTR);

  ASSERT_EQ(sp_rb_push(mem, q, rb_eval_counting(&count)), SP_OK);
  EXPECT_EQ(count, 1u);
  EXPECT_EQ(*sp_rb_peek(q), 7u);

  sp_rb_free(mem, q);
}

UTEST_F(sp_rb, first_push_evaluates_against_the_real_ring) {
  sp_rb(u32) q = SP_NULLPTR;
  void* seen = SP_NULLPTR;

  ASSERT_EQ(sp_rb_push(ut.mem, q, rb_eval_seeing(q, &seen)), SP_OK);
  EXPECT_TRUE(seen != SP_NULLPTR);
  EXPECT_TRUE(seen == (void*)q);
  EXPECT_EQ(sp_rb_size(q), 1u);
  EXPECT_EQ(*sp_rb_peek(q), 7u);

  sp_rb_free(ut.mem, q);
}

UTEST_F(sp_rb, failed_growth_leaves_the_ring_alone) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 1 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_rb(u32) q = SP_NULLPTR;

  ASSERT_EQ(sp_rb_init_cap(mem, q, 4), SP_OK);
  sp_for(it, 6) {
    ASSERT_EQ(sp_rb_push(mem, q, it), SP_OK);
    if (it < 2) sp_rb_pop(q);
  }
  ASSERT_TRUE(sp_rb_full(q));

  EXPECT_EQ(sp_rb_push(mem, q, 6), SP_ERR_SYS_BUSY);
  EXPECT_EQ(sp_rb_size(q), 4u);
  EXPECT_EQ(sp_rb_capacity(q), 4u);
  sp_rb_for(q, it) {
    EXPECT_EQ(sp_rb_at(q, it), it + 2);
  }

  ASSERT_EQ(sp_rb_push(mem, q, 6), SP_OK);
  EXPECT_EQ(sp_rb_size(q), 5u);
  sp_rb_for(q, it) {
    EXPECT_EQ(sp_rb_at(q, it), it + 2);
  }

  sp_rb_free(mem, q);
}

// A full ring that overwrites has no reason to ask for memory
UTEST_F(sp_rb, overwrite_never_allocates) {
  sp_rb(u32) q = SP_NULLPTR;
  ASSERT_EQ(sp_rb_init_cap(ut.mem, q, 4), SP_OK);
  sp_rb_set_mode(q, SP_RQ_MODE_OVERWRITE);

  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_for(it, 100) {
    ASSERT_EQ(sp_rb_push(mem, q, it), SP_OK);
  }

  EXPECT_EQ(failing.count, 0u);
  EXPECT_EQ(sp_rb_size(q), 4u);
  sp_rb_for(q, it) {
    EXPECT_EQ(sp_rb_at(q, it), it + 96);
  }

  sp_rb_free(ut.mem, q);
}
