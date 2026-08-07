#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define MAX_THREADS 8

typedef struct {
  const c8* name;
  u32 threads;
  u32 iterations;
} test_t;

static const test_t tests [] = {
  { .name = "single_thread", .threads = 1, .iterations = 1000 },
  { .name = "two_threads", .threads = 2, .iterations = 10000 },
  { .name = "many_threads", .threads = 8, .iterations = 5000 },
};

typedef struct {
  sp_mutex_t* mutex;
  u64* counter;
  u32 iterations;
} worker_t;

static s32 worker(void* userdata) {
  worker_t* data = (worker_t*)userdata;
  sp_for(i, data->iterations) {
    sp_mutex_lock(data->mutex);
    *data->counter += 1;
    sp_mutex_unlock(data->mutex);
  }
  return 0;
}

sp_test_each(sync, mutex, test_t, tests) {
#if defined(SP_FREESTANDING)
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  sp_mutex_t mutex;
  sp_mutex_init(&mutex);
  u64 counter = 0;

  worker_t data = {
    .mutex = &mutex,
    .counter = &counter,
    .iterations = it->iterations,
  };
  sp_thread_t threads [MAX_THREADS];
  sp_for(n, it->threads) {
    sp_thread_init(&threads[n], worker, &data);
  }
  sp_for(n, it->threads) {
    sp_thread_join(&threads[n]);
  }

  sp_expect_eq(t, counter, (u64)it->threads * it->iterations);
  sp_mutex_destroy(&mutex);
  return SP_OK;
#endif
}

#endif
