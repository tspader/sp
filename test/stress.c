#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

UTEST(dynamic_array, stress_test) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 128 * 1024 * 1024);

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    const s32 iterations = 1000000;

    for (s32 i = 0; i < iterations; i++) {
      sp_dynamic_array_push(&arr, &i);
    }

    ASSERT_EQ(arr.size, (u32)iterations);

    // Verify sampling
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 0);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, iterations/2), iterations/2);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, iterations-1), iterations-1);

    // Clear and reuse
    sp_dynamic_array_clear(&arr);
    ASSERT_EQ(arr.size, 0);

    // Push again
    for (s32 i = 0; i < 1000; i++) {
      sp_dynamic_array_push(&arr, &i);
    }
    ASSERT_EQ(arr.size, 1000);
  }

  // Test random operations
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    // Mix of operations
    for (u32 i = 0; i < 10000; i++) {
      u32 op = i % 4;
      switch (op) {
      case 0: { // Push
          s32 val = i;
          sp_dynamic_array_push(&arr, &val);
          break;
      }
      case 1: { // Push_n
          s32 vals[10];
          for (u32 j = 0; j < 10; j++) vals[j] = i + j;
          sp_dynamic_array_push_n(&arr, vals, 10);
          break;
      }
      case 2: { // Reserve
          if (arr.size < 100000) {
          sp_dynamic_array_reserve(&arr, 5);
          }
          break;
      }
      case 3: { // Clear periodically
          if (i % 1000 == 0 && i > 0) {
          sp_dynamic_array_clear(&arr);
          }
          break;
      }
      }
    }

    // Array should still be functional
    s32 final_val = 9999;
    sp_dynamic_array_push(&arr, &final_val);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, arr.size - 1), 9999);
  }

  sp_test_memory_tracker_deinit(&tracker);
}

UTEST(dyn_array, large_stress_test) {
  sp_dyn_array(u64) arr = SP_NULLPTR;

  const s32 count = 100000;

  for (s32 i = 0; i < count; i++) {
      sp_dyn_array_push(arr, (u64)i * 12345);
  }

  ASSERT_EQ(sp_dyn_array_size(arr), count);

  for (s32 i = 0; i < count; i++) {
      ASSERT_EQ(arr[i], (u64)i * 12345);
  }

  sp_dyn_array_clear(arr);
  ASSERT_EQ(sp_dyn_array_size(arr), 0);

  for (s32 i = 0; i < 1000; i++) {
      sp_dyn_array_push(arr, (u64)i);
  }
  ASSERT_EQ(sp_dyn_array_size(arr), 1000);

  sp_dyn_array_free(arr);
}

UTEST(hash_table, stress) {
  sp_ht(u64, u64) ht = SP_NULLPTR;

  const s32 count = 10000;

  for (u64 i = 0; i < count; i++) {
      sp_ht_insert(ht, i, i * i);
  }

  ASSERT_EQ(sp_ht_size(ht), count);

  for (s32 i = 0; i < 100; i++) {
      u64 key = rand() % count;
      ASSERT_TRUE(sp_ht_exists(ht, key));
      ASSERT_EQ(*sp_ht_getp(ht, key), key * key);
  }

  for (u64 i = 0; i < count; i += 2) {
      sp_ht_erase(ht, i);
  }

  ASSERT_EQ(sp_ht_size(ht), count / 2);

  for (u64 i = 1; i < count; i += 2) {
      ASSERT_TRUE(sp_ht_exists(ht, i));
      ASSERT_EQ(*sp_ht_getp(ht, i), i * i);
  }

  sp_ht_free(ht);
}

UTEST(ring_buffer, large_buffer_stress) {
  sp_ring_buffer_t rb;
  sp_ring_buffer_init(&rb, 1000, sizeof(u64));

  for (u64 i = 0; i < 1000; i++) {
      sp_ring_buffer_push(&rb, &i);
  }

  ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

  for (u64 i = 0; i < 500; i++) {
      u64* val = (u64*)sp_ring_buffer_pop(&rb);
      ASSERT_EQ(*val, i);
  }

  for (u64 i = 1000; i < 1500; i++) {
      sp_ring_buffer_push(&rb, &i);
  }

  ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

  u64 expected = 500;
  sp_ring_buffer_for(rb, it) {
      u64* val = sp_rb_it(it, u64);
      ASSERT_EQ(*val, expected);
      expected++;
  }

  sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, continuous_overwrite_stress) {
  sp_ring_buffer_t rb;
  sp_ring_buffer_init(&rb, 100, sizeof(int));

  for (s32 i = 0; i < 10000; i++) {
      sp_ring_buffer_push_overwrite(&rb, &i);
  }

  ASSERT_EQ(rb.size, 100);

  for (s32 i = 0; i < 100; i++) {
      int* val = (int*)sp_ring_buffer_pop(&rb);
      ASSERT_EQ(*val, 9900 + i);
  }

  ASSERT_TRUE(sp_ring_buffer_is_empty(&rb));

  sp_ring_buffer_destroy(&rb);
}

UTEST(sp_dyn_array_push_f, stress_test) {
  typedef struct {
    u32 id;
    c8 data[256];
  } large_struct_t;

  large_struct_t* arr = SP_NULLPTR;

  for (u32 i = 0; i < 1000; i++) {
    large_struct_t item;
    item.id = i;
    for (int j = 0; j < 256; j++) {
      item.data[j] = (c8)((i + j) % 256);
    }
    sp_dyn_array_push_f((void**)&arr, &item, sizeof(large_struct_t));
  }

  ASSERT_EQ(sp_dyn_array_size(arr), 1000);

  for (u32 i = 0; i < 1000; i++) {
    ASSERT_EQ(arr[i].id, i);
    for (int j = 0; j < 256; j++) {
      ASSERT_EQ(arr[i].data[j], (c8)((i + j) % 256));
    }
  }

  sp_dyn_array_free(arr);
}

UTEST_MAIN()
