#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"


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
      u64* val = sp_rb_it_getp(&it, u64);
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

typedef struct {
  sp_spin_lock_t* lock;
  s32* shared_counter;
  s32 iterations;
  s32 thread_id;
} sp_spin_lock_stress_thread_data_t;

s32 sp_spin_lock_stress_thread(void* userdata) {
  sp_spin_lock_stress_thread_data_t* data = (sp_spin_lock_stress_thread_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_spin_lock(data->lock);
    s32 old_value = *data->shared_counter;
    sp_spin_pause();
    *data->shared_counter = old_value + 1;
    sp_spin_unlock(data->lock);
  }

  return 0;
}

#define SP_SPIN_LOCK_STRESS_THREADS 8
#define SP_SPIN_LOCK_STRESS_ITERATIONS 5000

UTEST(sp_spin_lock, stress_multiple_threads) {
  sp_spin_lock_t lock = 0;
  s32 shared_counter = 0;

  sp_spin_lock_stress_thread_data_t thread_data[SP_SPIN_LOCK_STRESS_THREADS];
  sp_thread_t threads[SP_SPIN_LOCK_STRESS_THREADS];

  for (s32 i = 0; i < SP_SPIN_LOCK_STRESS_THREADS; i++) {
    thread_data[i].lock = &lock;
    thread_data[i].shared_counter = &shared_counter;
    thread_data[i].iterations = SP_SPIN_LOCK_STRESS_ITERATIONS;
    thread_data[i].thread_id = i;

    sp_thread_init(&threads[i], sp_spin_lock_stress_thread, &thread_data[i]);
  }

  for (s32 i = 0; i < SP_SPIN_LOCK_STRESS_THREADS; i++) {
    sp_thread_join(&threads[i]);
  }

  ASSERT_EQ(shared_counter, SP_SPIN_LOCK_STRESS_THREADS * SP_SPIN_LOCK_STRESS_ITERATIONS);
  ASSERT_EQ(lock, 0);
}

typedef struct {
  sp_atomic_s32* counter;
  s32 iterations;
  s32 thread_id;
} sp_atomic_s32_stress_data_t;

s32 sp_atomic_s32_stress_thread(void* userdata) {
  sp_atomic_s32_stress_data_t* data = (sp_atomic_s32_stress_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    s32 op = i % 4;
    switch (op) {
      case 0: sp_atomic_s32_add(data->counter, 1); break;
      case 1: sp_atomic_s32_set(data->counter, i); break;
      case 2: sp_atomic_s32_get(data->counter); break;
      case 3: {
        s32 current = sp_atomic_s32_get(data->counter);
        sp_atomic_s32_cmp_and_swap(data->counter, current, current + 1);
        break;
      }
    }
  }

  return 0;
}

UTEST(sp_atomic_s32, stress_concurrent_operations) {
  sp_atomic_s32 counter = 0;
  const s32 num_threads = 8;
  const s32 iterations = 10000;

  sp_atomic_s32_stress_data_t thread_data[8];
  sp_thread_t threads[8];

  for (s32 i = 0; i < num_threads; i++) {
    thread_data[i].counter = &counter;
    thread_data[i].iterations = iterations;
    thread_data[i].thread_id = i;
    sp_thread_init(&threads[i], sp_atomic_s32_stress_thread, &thread_data[i]);
  }

  for (s32 i = 0; i < num_threads; i++) {
    sp_thread_join(&threads[i]);
  }

  s32 final = sp_atomic_s32_get(&counter);
  ASSERT_TRUE(final >= 0);
}

UTEST_MAIN()
