#include "sp.h"
#include "sp/sp_test.h"

#define MAX_THREADS 8
#define PUBLISH_SPIN_LIMIT 1000000000ull

typedef enum {
  OP_LOAD,
  OP_STORE,
  OP_EXCHANGE,
  OP_ADD,
  OP_CAS,
  OP_FENCE,
} op_kind_t;

typedef struct {
  u64 result;
  u64 value;
  bool ok;
} expect_t;

typedef struct {
  const c8* name;
  op_kind_t op;
  u64 initial;
  u64 operand;
  u64 desired;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "load_returns_value",
    .op = OP_LOAD,
    .initial = 0x12345678,
    .expect = {
      .result = 0x12345678,
      .value = 0x12345678,
    },
  },
  {
    .name = "store_sets_value",
    .op = OP_STORE,
    .operand = 7,
    .expect = {
      .value = 7,
    },
  },
  {
    .name = "exchange_returns_old_and_sets_new",
    .op = OP_EXCHANGE,
    .initial = 3,
    .operand = 9,
    .expect = {
      .result = 3,
      .value = 9,
    },
  },
  {
    .name = "add_returns_old_and_adds",
    .op = OP_ADD,
    .initial = 40,
    .operand = 2,
    .expect = {
      .result = 40,
      .value = 42,
    },
  },
  {
    .name = "add_carries_past_32_bits",
    .op = OP_ADD,
    .initial = 0xFFFFFFFF,
    .operand = 1,
    .expect = {
      .result = 0xFFFFFFFF,
      .value = 0x100000000,
    },
  },
  {
    .name = "cas_matching_swaps",
    .op = OP_CAS,
    .initial = 5,
    .operand = 5,
    .desired = 6,
    .expect = {
      .ok = true,
      .value = 6,
    },
  },
  {
    .name = "cas_mismatched_fails",
    .op = OP_CAS,
    .initial = 5,
    .operand = 4,
    .desired = 6,
    .expect = {
      .value = 5,
    },
  },
  {
    .name = "fence_accepts_every_order",
    .op = OP_FENCE,
  },
};


static c8 atomic_ptr_slots [16];

static void get_orders(op_kind_t op, const sp_atomic_order_t** orders, u32* len) {
  static const sp_atomic_order_t load [] = {
    SP_ATOMIC_RELAXED, SP_ATOMIC_ACQUIRE, SP_ATOMIC_SEQ_CST
  };
  static const sp_atomic_order_t store [] = {
    SP_ATOMIC_RELAXED, SP_ATOMIC_RELEASE, SP_ATOMIC_SEQ_CST
  };
  static const sp_atomic_order_t rmw [] = {
    SP_ATOMIC_RELAXED, SP_ATOMIC_ACQUIRE, SP_ATOMIC_RELEASE, SP_ATOMIC_ACQ_REL, SP_ATOMIC_SEQ_CST
  };
  switch (op) {
    case OP_LOAD: {
      *orders = load;
      *len = sp_carr_len(load);
      break;
    }
    case OP_STORE: {
      *orders = store;
      *len = sp_carr_len(store);
      break;
    }
    case OP_EXCHANGE:
    case OP_ADD:
    case OP_CAS:
    case OP_FENCE: {
      *orders = rmw;
      *len = sp_carr_len(rmw);
      break;
    }
  }
}

static c8* get_slot(u64 value) {
  return &atomic_ptr_slots[value % sp_carr_len(atomic_ptr_slots)];
}

static bool has_ptr_lane(op_kind_t op) {
  switch (op) {
    case OP_LOAD:
    case OP_STORE:
    case OP_EXCHANGE: {
      return true;
    }
    case OP_ADD:
    case OP_CAS:
    case OP_FENCE: {
      return false;
    }
  }
  return false;
}

sp_test_each(atomic, ops, test_t, tests) {
  const sp_atomic_order_t* orders = SP_NULLPTR;
  u32 num_orders = 0;
  get_orders(it->op, &orders, &num_orders);

  sp_for(o, num_orders) {
    sp_atomic_order_t order = orders[o];

    if (it->op == OP_FENCE) {
      sp_atomic_fence(order);
      continue;
    }

    struct {
      sp_atomic_u32_t a;
      u32 result;
      bool ok;
      struct {
        u32 result;
        u32 value;
      } expected;
      u32 operand;
      u32 desired;
      u32 value;
    } u = {
      .a = (u32)it->initial,
      .expected = { .result = (u32)it->expect.result, .value = (u32)it->expect.value, },
      .operand = (u32)it->operand,
      .desired = (u32)it->desired,
    };

    switch (it->op) {
      case OP_LOAD: {
        u.result = sp_atomic_u32_load(&u.a, order);
        sp_expect_eq(t, u.result, u.expected.result);
        break;
      }
      case OP_STORE: {
        sp_atomic_u32_store(&u.a, u.operand, order);
        break;
      }
      case OP_EXCHANGE: {
        u.result = sp_atomic_u32_exchange(&u.a, u.operand, order);
        sp_expect_eq(t, u.result, u.expected.result);
        break;
      }
      case OP_ADD: {
        u.result = sp_atomic_u32_add(&u.a, u.operand, order);
        sp_expect_eq(t, u.result, u.expected.result);
        break;
      }
      case OP_CAS: {
        u.ok = sp_atomic_u32_cas(&u.a, u.operand, u.desired, order);
        sp_expect_eq(t, u.ok, it->expect.ok);
        break;
      }
      case OP_FENCE: {
        break;
      }
    }
    sp_expect_eq(t, u.a, u.expected.value);

    sp_atomic_u64_t a = it->initial;
    u64 result = 0;
    bool ok = false;
    switch (it->op) {
      case OP_LOAD: result = sp_atomic_u64_load(&a, order); break;
      case OP_STORE: sp_atomic_u64_store(&a, it->operand, order); break;
      case OP_EXCHANGE: result = sp_atomic_u64_exchange(&a, it->operand, order); break;
      case OP_ADD: result = sp_atomic_u64_add(&a, it->operand, order); break;
      case OP_CAS: ok = sp_atomic_u64_cas(&a, it->operand, it->desired, order); break;
      case OP_FENCE: break;
    }
    sp_expect_eq(t, a, it->expect.value);
    if (it->op == OP_CAS) {
      sp_expect_eq(t, ok, it->expect.ok);
    }
    else if (it->op != OP_STORE) {
      sp_expect_eq(t, result, it->expect.result);
    }

    if (has_ptr_lane(it->op)) {
      sp_atomic_ptr_t value = get_slot(it->initial);
      void* result = SP_NULLPTR;
      switch (it->op) {
        case OP_LOAD: {
          result = sp_atomic_ptr_load(&value, order);
          sp_expect(t, result == get_slot(it->expect.result));
          break;
        }
        case OP_STORE: {
          sp_atomic_ptr_store(&value, get_slot(it->operand), order);
          break;
        }
        case OP_EXCHANGE: {
          result = sp_atomic_ptr_exchange(&value, get_slot(it->operand), order);
          sp_expect(t, result == get_slot(it->expect.result));
          break;
        }
        case OP_ADD:
        case OP_CAS:
        case OP_FENCE: {
          sp_expect(t, result == get_slot(it->expect.result));
          break;
        }
      }
      sp_expect(t, value == get_slot(it->expect.value));
    }
  }
  return SP_OK;
}
