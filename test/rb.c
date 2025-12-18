#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct sp_rb {
};

UTEST_F_SETUP(sp_rb) {
}

UTEST_F_TEARDOWN(sp_rb) {
}

UTEST_F(sp_rb, null_init) {
  sp_rq(int) q = SP_NULLPTR;

  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_EQ(0, sp_rq_capacity(q));
  EXPECT_TRUE(sp_rq_empty(q));
  EXPECT_TRUE(sp_rq_full(q));
}

UTEST_F(sp_rb, clear) {
  sp_rq(int) q = SP_NULLPTR;

  q = sp_rq_grow_impl(SP_NULLPTR, sizeof(int), 8);
  ASSERT_TRUE(q != SP_NULLPTR);
  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_EQ(8, sp_rq_capacity(q));

  sp_rq_head(q)->size = 3;
  sp_rq_head(q)->head = 2;
  EXPECT_EQ(3, sp_rq_size(q));

  sp_rq_clear(q);
  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_EQ(0, sp_rq_head(q)->head);
  EXPECT_EQ(8, sp_rq_capacity(q));
  EXPECT_TRUE(sp_rq_empty(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, free_test) {
  sp_rq(int) q = SP_NULLPTR;

  q = sp_rq_grow_impl(SP_NULLPTR, sizeof(int), 8);
  ASSERT_TRUE(q != SP_NULLPTR);

  sp_rq_free(q);
  EXPECT_TRUE(q == SP_NULLPTR);

  sp_rq_free(q);
  EXPECT_TRUE(q == SP_NULLPTR);
}

UTEST_F(sp_rb, push_pop_fifo) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 10);
  sp_rq_push(q, 20);
  sp_rq_push(q, 30);

  EXPECT_EQ(3, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_capacity(q) >= 3);

  EXPECT_EQ(10, *sp_rq_peek(q));
  EXPECT_EQ(30, *sp_rq_back(q));

  sp_rq_pop(q);
  EXPECT_EQ(2, sp_rq_size(q));
  EXPECT_EQ(20, *sp_rq_peek(q));

  sp_rq_pop(q);
  EXPECT_EQ(1, sp_rq_size(q));
  EXPECT_EQ(30, *sp_rq_peek(q));

  sp_rq_pop(q);
  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_empty(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, full_and_grow) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 0);
  s32 initial_cap = sp_rq_capacity(q);

  for (int i = 1; i < initial_cap; i++) {
    sp_rq_push(q, i * 10);
  }

  EXPECT_EQ(initial_cap, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_full(q));

  sp_rq_push(q, initial_cap * 10);
  EXPECT_EQ(initial_cap + 1, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_capacity(q) > initial_cap);
  EXPECT_FALSE(sp_rq_full(q));

  EXPECT_EQ(0, *sp_rq_peek(q));
  EXPECT_EQ(initial_cap * 10, *sp_rq_back(q));

  for (int i = 0; i <= initial_cap; i++) {
    EXPECT_EQ(i * 10, sp_rq_at(q, i));
  }

  sp_rq_free(q);
}

UTEST_F(sp_rb, wrap_access) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 1);
  sp_rq_push(q, 2);
  sp_rq_push(q, 3);
  sp_rq_push(q, 4);

  sp_rq_pop(q);
  sp_rq_pop(q);

  sp_rq_push(q, 5);
  sp_rq_push(q, 6);
  sp_rq_push(q, 7);
  sp_rq_push(q, 8);
  sp_rq_push(q, 9);
  sp_rq_push(q, 10);

  EXPECT_EQ(8, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_full(q));

  EXPECT_EQ(3, sp_rq_at(q, 0));
  EXPECT_EQ(4, sp_rq_at(q, 1));
  EXPECT_EQ(5, sp_rq_at(q, 2));
  EXPECT_EQ(10, sp_rq_at(q, 7));

  EXPECT_EQ(3, *sp_rq_peek(q));
  EXPECT_EQ(10, *sp_rq_back(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, empty_ops) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_pop(q);
  EXPECT_TRUE(sp_rq_empty(q));
  EXPECT_TRUE(sp_rq_peek(q) == SP_NULLPTR);
  EXPECT_TRUE(sp_rq_back(q) == SP_NULLPTR);

  sp_rq_push(q, 42);
  sp_rq_pop(q);

  EXPECT_TRUE(sp_rq_empty(q));
  EXPECT_TRUE(sp_rq_peek(q) == SP_NULLPTR);
  EXPECT_TRUE(sp_rq_back(q) == SP_NULLPTR);

  sp_rq_pop(q);
  EXPECT_TRUE(sp_rq_empty(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, iterate_wrapped) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 1);
  sp_rq_push(q, 2);
  sp_rq_push(q, 3);
  sp_rq_push(q, 4);

  sp_rq_pop(q);
  sp_rq_pop(q);

  sp_rq_push(q, 5);
  sp_rq_push(q, 6);

  int expected[] = {3, 4, 5, 6};
  int idx = 0;

  sp_rq_for(q, it) {
    EXPECT_EQ(expected[idx], sp_rq_at(q, it));
    idx++;
  }

  EXPECT_EQ(4, idx);

  sp_rq_free(q);
}

UTEST_F(sp_rb, iterate_reverse) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 10);
  sp_rq_push(q, 20);
  sp_rq_push(q, 30);
  sp_rq_push(q, 40);

  int expected[] = {40, 30, 20, 10};
  int idx = 0;

  sp_rq_rfor(q, it) {
    EXPECT_EQ(expected[idx], sp_rq_at(q, it));
    idx++;
  }

  EXPECT_EQ(4, idx);

  sp_rq_free(q);
}

typedef struct {
  s32 id;
  float x;
  float y;
  u64 flags;
} test_item_t;

UTEST_F(sp_rb, struct_type) {
  sp_rq(test_item_t) q = SP_NULLPTR;

  test_item_t a = {.id = 1, .x = 1.5f, .y = 2.5f, .flags = 0xDEADBEEF};
  test_item_t b = {.id = 2, .x = 3.5f, .y = 4.5f, .flags = 0xCAFEBABE};
  test_item_t c = {.id = 3, .x = 5.5f, .y = 6.5f, .flags = 0x12345678};

  sp_rq_push(q, a);
  sp_rq_push(q, b);
  sp_rq_push(q, c);

  test_item_t* p = sp_rq_peek(q);
  EXPECT_EQ(1, p->id);
  EXPECT_TRUE(p->x == 1.5f);
  EXPECT_TRUE(p->y == 2.5f);
  EXPECT_EQ(0xDEADBEEF, p->flags);

  sp_rq_pop(q);

  p = sp_rq_peek(q);
  EXPECT_EQ(2, p->id);
  EXPECT_TRUE(p->x == 3.5f);
  EXPECT_TRUE(p->y == 4.5f);
  EXPECT_EQ(0xCAFEBABE, p->flags);

  p = sp_rq_back(q);
  EXPECT_EQ(3, p->id);
  EXPECT_TRUE(p->x == 5.5f);
  EXPECT_TRUE(p->y == 6.5f);
  EXPECT_EQ(0x12345678, p->flags);

  sp_rq_free(q);
}

UTEST_F(sp_rb, capacity_one) {
  sp_rq(int) q = sp_rq_grow_impl(SP_NULLPTR, sizeof(int), 1);

  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_EQ(1, sp_rq_capacity(q));
  EXPECT_TRUE(sp_rq_empty(q));
  EXPECT_FALSE(sp_rq_full(q));

  sp_rq_push(q, 42);
  EXPECT_EQ(1, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_full(q));
  EXPECT_EQ(42, *sp_rq_peek(q));
  EXPECT_EQ(42, *sp_rq_back(q));

  sp_rq_pop(q);
  EXPECT_TRUE(sp_rq_empty(q));
  EXPECT_TRUE(sp_rq_peek(q) == SP_NULLPTR);

  sp_rq_push(q, 100);
  EXPECT_EQ(100, *sp_rq_peek(q));

  sp_rq_push(q, 200);
  EXPECT_TRUE(sp_rq_capacity(q) > 1);
  EXPECT_EQ(2, sp_rq_size(q));
  EXPECT_EQ(100, *sp_rq_peek(q));
  EXPECT_EQ(200, *sp_rq_back(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, overwrite_mode) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 1);
  sp_rq_set_mode(q, SP_RQ_MODE_OVERWRITE);
  EXPECT_EQ(SP_RQ_MODE_OVERWRITE, sp_rq_mode(q));

  s32 cap = sp_rq_capacity(q);

  for (int i = 2; i <= cap; i++) {
    sp_rq_push(q, i);
  }

  EXPECT_TRUE(sp_rq_full(q));
  EXPECT_EQ(cap, sp_rq_capacity(q));

  sp_rq_push(q, cap + 1);
  EXPECT_EQ(cap, sp_rq_capacity(q));
  EXPECT_EQ(cap, sp_rq_size(q));

  EXPECT_EQ(2, *sp_rq_peek(q));
  EXPECT_EQ(cap + 1, *sp_rq_back(q));

  sp_rq_push(q, cap + 2);
  sp_rq_push(q, cap + 3);

  EXPECT_EQ(4, *sp_rq_peek(q));
  EXPECT_EQ(cap + 3, *sp_rq_back(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, overwrite_preserves_order) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 0);
  sp_rq_set_mode(q, SP_RQ_MODE_OVERWRITE);
  s32 cap = sp_rq_capacity(q);

  for (int i = 1; i < cap * 2; i++) {
    sp_rq_push(q, i);
  }

  EXPECT_EQ(cap, sp_rq_size(q));

  int expected_start = cap * 2 - cap;
  sp_rq_for(q, it) {
    EXPECT_EQ(expected_start + it, sp_rq_at(q, it));
  }

  sp_rq_free(q);
}

UTEST_F(sp_rb, mode_preserved_on_grow) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_push(q, 1);
  sp_rq_set_mode(q, SP_RQ_MODE_GROW);
  s32 cap = sp_rq_capacity(q);

  for (int i = 2; i <= cap + 1; i++) {
    sp_rq_push(q, i);
  }

  EXPECT_TRUE(sp_rq_capacity(q) > cap);
  EXPECT_EQ(SP_RQ_MODE_GROW, sp_rq_mode(q));

  sp_rq_free(q);
}

UTEST_F(sp_rb, set_mode_on_null) {
  sp_rq(int) q = SP_NULLPTR;

  sp_rq_set_mode(q, SP_RQ_MODE_OVERWRITE);

  EXPECT_TRUE(q != SP_NULLPTR);
  EXPECT_EQ(SP_RQ_MODE_OVERWRITE, sp_rq_mode(q));
  EXPECT_EQ(0, sp_rq_size(q));
  EXPECT_TRUE(sp_rq_capacity(q) > 0);

  sp_rq_push(q, 1);
  EXPECT_EQ(1, sp_rq_size(q));

  sp_rq_free(q);
}
