#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

typedef struct {
  sp_atomic_s32_t* accumulator;
  s32 value;
  sp_thread_t thread;
} thread_test_t;

s32 fn(void* user_data) {
  thread_test_t* test = (thread_test_t*)user_data;
  sp_atomic_s32_add(test->accumulator, test->value);
  return 0;
}

#define THREAD_C_NUM_THREADS 4

UTEST(thread, hello) {
  SKIP_ON_FREESTANDING();
  thread_test_t threads [THREAD_C_NUM_THREADS] = sp_zero();

  sp_atomic_s32_t accumulator = sp_zero();
  sp_carr_for(threads, it) {
    threads[it].accumulator = &accumulator;
    threads[it].value = it;
    sp_thread_init(&threads[it].thread, fn, &threads[it]);
  }

  sp_carr_for(threads, it) {
    sp_thread_join(&threads[it].thread);
  }

  EXPECT_EQ(sp_atomic_s32_get(&accumulator), (0 + 1 + 2 + 3));
}

typedef struct {
  bool context_valid;
  bool allocator_valid;
  bool alloc_succeeded;
  sp_str_t allocated_string;
} sp_thread_context_test_data_t;

s32 sp_thread_context_test_fn(void* userdata) {
  sp_thread_context_test_data_t* data = (sp_thread_context_test_data_t*)userdata;

  sp_context_t* ctx = sp_context_get();
  data->context_valid = (ctx != SP_NULLPTR);
  data->allocator_valid = (ctx != SP_NULLPTR && ctx->allocator.on_alloc != SP_NULLPTR);

  sp_str_t test_str = sp_str_from_cstr_a(sp_mem_os_new(), "thread allocation test");
  data->allocated_string = test_str;
  data->alloc_succeeded = test_str.data != SP_NULLPTR && test_str.len > 0;

  return 0;
}

UTEST(threading, context_in_child_thread) {
  SKIP_ON_FREESTANDING();
  sp_thread_context_test_data_t data = SP_ZERO_INITIALIZE();

  sp_thread_t thread;
  sp_thread_init(&thread, sp_thread_context_test_fn, &data);
  sp_thread_join(&thread);

  ASSERT_TRUE(data.context_valid);
  ASSERT_TRUE(data.allocator_valid);
  ASSERT_TRUE(data.alloc_succeeded);
  ASSERT_GT(data.allocated_string.len, 0);
  SP_EXPECT_STR_EQ_CSTR(data.allocated_string, "thread allocation test");
}


UTEST(sp_spin_lock, basic_lock_unlock) {
  sp_spin_lock_t lock = 0;

  sp_spin_lock(&lock);
  ASSERT_EQ(lock, 1);

  sp_spin_unlock(&lock);
  ASSERT_EQ(lock, 0);
}

UTEST(sp_spin_lock, try_lock_success) {
  sp_spin_lock_t lock = 0;

  bool acquired = sp_spin_try_lock(&lock);
  ASSERT_TRUE(acquired);
  ASSERT_EQ(lock, 1);

  sp_spin_unlock(&lock);
  ASSERT_EQ(lock, 0);
}

UTEST(sp_spin_lock, try_lock_fails_when_locked) {
  sp_spin_lock_t lock = 0;

  sp_spin_lock(&lock);
  ASSERT_EQ(lock, 1);

  bool second_acquire = sp_spin_try_lock(&lock);
  ASSERT_FALSE(second_acquire);

  sp_spin_unlock(&lock);
}

UTEST(sp_spin_lock, multiple_lock_unlock_cycles) {
  sp_spin_lock_t lock = 0;

  for (s32 i = 0; i < 1000; i++) {
    sp_spin_lock(&lock);
    ASSERT_EQ(lock, 1);
    sp_spin_unlock(&lock);
    ASSERT_EQ(lock, 0);
  }
}

typedef struct {
  sp_spin_lock_t* lock;
  s32* shared_counter;
  s32 iterations;
  s32 thread_id;
} sp_spin_lock_thread_data_t;

s32 sp_spin_lock_increment_thread(void* userdata) {
  sp_spin_lock_thread_data_t* data = (sp_spin_lock_thread_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_spin_lock(data->lock);
    (*data->shared_counter)++;
    sp_spin_unlock(data->lock);
  }

  return 0;
}

UTEST(sp_spin_lock, mutual_exclusion_two_threads) {
  SKIP_ON_FREESTANDING();
  sp_spin_lock_t lock = 0;
  s32 shared_counter = 0;
  const s32 iterations_per_thread = 10000;

  sp_spin_lock_thread_data_t data1 = SP_ZERO_INITIALIZE();
  data1.lock = &lock;
  data1.shared_counter = &shared_counter;
  data1.iterations = iterations_per_thread;
  data1.thread_id = 1;

  sp_spin_lock_thread_data_t data2 = SP_ZERO_INITIALIZE();
  data2.lock = &lock;
  data2.shared_counter = &shared_counter;
  data2.iterations = iterations_per_thread;
  data2.thread_id = 2;

  sp_thread_t thread1, thread2;
  sp_thread_init(&thread1, sp_spin_lock_increment_thread, &data1);
  sp_thread_init(&thread2, sp_spin_lock_increment_thread, &data2);

  sp_thread_join(&thread1);
  sp_thread_join(&thread2);

  ASSERT_EQ(shared_counter, iterations_per_thread * 2);
  ASSERT_EQ(lock, 0);
}


UTEST(sp_atomic_s32, basic_operations) {
  sp_atomic_s32_t value = 0;

  s32 old = sp_atomic_s32_set(&value, 42);
  ASSERT_EQ(old, 0);
  ASSERT_EQ(sp_atomic_s32_get(&value), 42);

  old = sp_atomic_s32_add(&value, 10);
  ASSERT_EQ(old, 42);
  ASSERT_EQ(sp_atomic_s32_get(&value), 52);

  old = sp_atomic_s32_add(&value, -2);
  ASSERT_EQ(old, 52);
  ASSERT_EQ(sp_atomic_s32_get(&value), 50);
}

UTEST(sp_atomic_s32, cmp_and_swap_success) {
  sp_atomic_s32_t value = 100;

  bool result = sp_atomic_s32_cas(&value, 100, 200);
  ASSERT_TRUE(result);
  ASSERT_EQ(sp_atomic_s32_get(&value), 200);
}

UTEST(sp_atomic_s32, cmp_and_swap_fails) {
  sp_atomic_s32_t value = 100;

  bool result = sp_atomic_s32_cas(&value, 50, 200);
  ASSERT_FALSE(result);
  ASSERT_EQ(sp_atomic_s32_get(&value), 100);
}

UTEST(sp_atomic_s32, add_returns_old_value) {
  sp_atomic_s32_t value = 0;

  for (s32 i = 0; i < 100; i++) {
    s32 old = sp_atomic_s32_add(&value, 1);
    ASSERT_EQ(old, i);
  }

  ASSERT_EQ(sp_atomic_s32_get(&value), 100);
}

typedef struct {
  sp_atomic_s32_t* counter;
  s32 iterations;
} sp_atomic_s32_thread_data_t;

s32 sp_atomic_s32_add_thread(void* userdata) {
  sp_atomic_s32_thread_data_t* data = (sp_atomic_s32_thread_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_atomic_s32_add(data->counter, 1);
  }

  return 0;
}

UTEST(sp_atomic_s32, concurrent_adds) {
  SKIP_ON_FREESTANDING();
  sp_atomic_s32_t counter = 0;
  const s32 iterations = 5000;

  sp_atomic_s32_thread_data_t data1 = {.counter = &counter, .iterations = iterations};
  sp_atomic_s32_thread_data_t data2 = {.counter = &counter, .iterations = iterations};

  sp_thread_t thread1, thread2;
  sp_thread_init(&thread1, sp_atomic_s32_add_thread, &data1);
  sp_thread_init(&thread2, sp_atomic_s32_add_thread, &data2);

  sp_thread_join(&thread1);
  sp_thread_join(&thread2);

  ASSERT_EQ(sp_atomic_s32_get(&counter), iterations * 2);
}
