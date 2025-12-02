#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"
SP_TEST_MAIN()

UTEST(sp_cv, init_destroy) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_cv_destroy(&cv);
}

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  bool signaled;
  sp_atomic_s32 ready;
  sp_atomic_s32 done;
} wait_notify_one_data_t;

s32 wait_notify_one_worker(void* userdata) {
  wait_notify_one_data_t* data = (wait_notify_one_data_t*)userdata;

  sp_mutex_lock(data->mutex);
  sp_atomic_s32_add(&data->ready, 1);
  while (!data->signaled) {
    sp_cv_wait(data->cv, data->mutex);
  }
  sp_mutex_unlock(data->mutex);

  sp_atomic_s32_add(&data->done, 1);
  return 0;
}

UTEST(sp_cv, wait_notify_one) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);

  wait_notify_one_data_t data = {
    .cv = &cv,
    .mutex = &mutex,
    .signaled = false,
    .ready = 0,
    .done = 0
  };

  sp_thread_t worker = SP_ZERO_INITIALIZE();
  sp_thread_init(&worker, wait_notify_one_worker, &data);

  while (sp_atomic_s32_get(&data.ready) < 1) {
    sp_spin_pause();
  }

  sp_mutex_lock(&mutex);
  data.signaled = true;
  sp_mutex_unlock(&mutex);
  sp_cv_notify_one(&cv);

  sp_thread_join(&worker);

  EXPECT_EQ(sp_atomic_s32_get(&data.done), 1);

  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}

#define NOTIFY_ALL_NUM_WORKERS 8

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  bool signaled;
  sp_atomic_s32 ready_count;
  sp_atomic_s32 done_count;
} wait_notify_all_data_t;

s32 wait_notify_all_worker(void* userdata) {
  wait_notify_all_data_t* data = (wait_notify_all_data_t*)userdata;

  sp_mutex_lock(data->mutex);
  sp_atomic_s32_add(&data->ready_count, 1);
  while (!data->signaled) {
    sp_cv_wait(data->cv, data->mutex);
  }
  sp_mutex_unlock(data->mutex);

  sp_atomic_s32_add(&data->done_count, 1);
  return 0;
}

UTEST(sp_cv, wait_notify_all) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);

  wait_notify_all_data_t data = {
    .cv = &cv,
    .mutex = &mutex,
    .signaled = false,
    .ready_count = 0,
    .done_count = 0
  };

  sp_thread_t workers[NOTIFY_ALL_NUM_WORKERS] = SP_ZERO_INITIALIZE();
  for (s32 i = 0; i < NOTIFY_ALL_NUM_WORKERS; i++) {
    sp_thread_init(&workers[i], wait_notify_all_worker, &data);
  }

  while (sp_atomic_s32_get(&data.ready_count) < NOTIFY_ALL_NUM_WORKERS) {
    sp_spin_pause();
  }

  sp_mutex_lock(&mutex);
  data.signaled = true;
  sp_mutex_unlock(&mutex);
  sp_cv_notify_all(&cv);

  for (s32 i = 0; i < NOTIFY_ALL_NUM_WORKERS; i++) {
    sp_thread_join(&workers[i]);
  }

  EXPECT_EQ(sp_atomic_s32_get(&data.done_count), NOTIFY_ALL_NUM_WORKERS);

  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}

#define NOTIFY_ONE_NUM_WORKERS 8

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  bool signaled;
  sp_atomic_s32 waiting_count;
  sp_atomic_s32 woken_count;
} notify_one_wakes_single_data_t;

s32 notify_one_wakes_single_worker(void* userdata) {
  notify_one_wakes_single_data_t* data = (notify_one_wakes_single_data_t*)userdata;

  sp_mutex_lock(data->mutex);
  sp_atomic_s32_add(&data->waiting_count, 1);
  while (!data->signaled) {
    sp_cv_wait(data->cv, data->mutex);
  }
  sp_atomic_s32_add(&data->woken_count, 1);
  sp_mutex_unlock(data->mutex);

  return 0;
}

UTEST(sp_cv, notify_one_wakes_single) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);

  notify_one_wakes_single_data_t data = {
    .cv = &cv,
    .mutex = &mutex,
    .signaled = false,
    .waiting_count = 0,
    .woken_count = 0
  };

  sp_thread_t workers[NOTIFY_ONE_NUM_WORKERS] = SP_ZERO_INITIALIZE();
  for (s32 i = 0; i < NOTIFY_ONE_NUM_WORKERS; i++) {
    sp_thread_init(&workers[i], notify_one_wakes_single_worker, &data);
  }

  while (sp_atomic_s32_get(&data.waiting_count) < NOTIFY_ONE_NUM_WORKERS) {
    sp_spin_pause();
  }

  sp_mutex_lock(&mutex);
  data.signaled = true;
  sp_mutex_unlock(&mutex);
  sp_cv_notify_one(&cv);

  sp_os_sleep_ms(100);

  EXPECT_EQ(sp_atomic_s32_get(&data.woken_count), 1);

  sp_cv_notify_all(&cv);

  for (s32 i = 0; i < NOTIFY_ONE_NUM_WORKERS; i++) {
    sp_thread_join(&workers[i]);
  }

  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}

UTEST(sp_cv, wait_for_timeout) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);

  sp_mutex_lock(&mutex);
  sp_tm_timer_t timer = sp_tm_start_timer();
  bool result = sp_cv_wait_for(&cv, &mutex, 100);
  u64 elapsed = sp_tm_read_timer(&timer);
  sp_mutex_unlock(&mutex);

  EXPECT_FALSE(result);
  EXPECT_GE(elapsed, 100000000ULL);

  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  bool ready;
  sp_atomic_s32 waiter_ready;
} wait_for_signaled_data_t;

s32 wait_for_signaled_signaler(void* userdata) {
  wait_for_signaled_data_t* data = (wait_for_signaled_data_t*)userdata;

  while (sp_atomic_s32_get(&data->waiter_ready) < 1) {
    sp_spin_pause();
  }

  sp_mutex_lock(data->mutex);
  data->ready = true;
  sp_mutex_unlock(data->mutex);
  sp_cv_notify_one(data->cv);

  return 0;
}

UTEST(sp_cv, wait_for_signaled) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);

  wait_for_signaled_data_t data = {
    .cv = &cv,
    .mutex = &mutex,
    .ready = false,
    .waiter_ready = 0
  };

  sp_thread_t signaler = SP_ZERO_INITIALIZE();
  sp_thread_init(&signaler, wait_for_signaled_signaler, &data);

  sp_mutex_lock(&mutex);
  sp_atomic_s32_add(&data.waiter_ready, 1);
  bool result = sp_cv_wait_for(&cv, &mutex, 1000);
  sp_mutex_unlock(&mutex);

  sp_thread_join(&signaler);

  EXPECT_TRUE(result);
  EXPECT_TRUE(data.ready);

  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}

#define PC_NUM_PRODUCERS 4
#define PC_NUM_CONSUMERS 4
#define PC_ITEMS_PER_PRODUCER 100
#define PC_TOTAL_ITEMS (PC_NUM_PRODUCERS * PC_ITEMS_PER_PRODUCER)

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  sp_ring_buffer_t* buffer;
  bool done;
  sp_atomic_s32 produced_count;
  sp_atomic_s32 consumed_count;
  s32 items_to_produce;
} producer_consumer_data_t;

s32 producer_fn(void* userdata) {
  producer_consumer_data_t* data = (producer_consumer_data_t*)userdata;

  for (s32 i = 0; i < data->items_to_produce; i++) {
    sp_mutex_lock(data->mutex);
    sp_ring_buffer_push(data->buffer, &i);
    sp_atomic_s32_add(&data->produced_count, 1);
    sp_mutex_unlock(data->mutex);
    sp_cv_notify_one(data->cv);
  }

  return 0;
}

s32 consumer_fn(void* userdata) {
  producer_consumer_data_t* data = (producer_consumer_data_t*)userdata;

  while (true) {
    sp_mutex_lock(data->mutex);
    while (sp_ring_buffer_is_empty(data->buffer) && !data->done) {
      sp_cv_wait(data->cv, data->mutex);
    }
    if (sp_ring_buffer_is_empty(data->buffer) && data->done) {
      sp_mutex_unlock(data->mutex);
      break;
    }
    sp_ring_buffer_pop(data->buffer);
    sp_atomic_s32_add(&data->consumed_count, 1);
    sp_mutex_unlock(data->mutex);
  }

  return 0;
}

UTEST(sp_cv, multithread_producer_consumer) {
  sp_cv_t cv = SP_ZERO_INITIALIZE();
  sp_mutex_t mutex = SP_ZERO_INITIALIZE();
  sp_ring_buffer_t buffer = SP_ZERO_INITIALIZE();

  sp_cv_init(&cv);
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);
  sp_ring_buffer_init(&buffer, PC_TOTAL_ITEMS + 1, sizeof(s32));

  producer_consumer_data_t data = {
    .cv = &cv,
    .mutex = &mutex,
    .buffer = &buffer,
    .done = false,
    .produced_count = 0,
    .consumed_count = 0,
    .items_to_produce = PC_ITEMS_PER_PRODUCER
  };

  sp_thread_t producers[PC_NUM_PRODUCERS] = SP_ZERO_INITIALIZE();
  sp_thread_t consumers[PC_NUM_CONSUMERS] = SP_ZERO_INITIALIZE();

  sp_for(it, PC_NUM_CONSUMERS) {
    sp_thread_init(&consumers[it], consumer_fn, &data);
  }

  sp_for(it, PC_NUM_PRODUCERS) {
    sp_thread_init(&producers[it], producer_fn, &data);
  }

  sp_for(it, PC_NUM_PRODUCERS) {
    sp_thread_join(&producers[it]);
  }

  sp_mutex_lock(&mutex);
  data.done = true;
  sp_mutex_unlock(&mutex);
  sp_cv_notify_all(&cv);

  sp_for(it, PC_NUM_CONSUMERS) {
    sp_thread_join(&consumers[it]);
  }

  EXPECT_EQ(sp_atomic_s32_get(&data.produced_count), PC_TOTAL_ITEMS);
  EXPECT_EQ(sp_atomic_s32_get(&data.consumed_count), PC_TOTAL_ITEMS);

  sp_ring_buffer_destroy(&buffer);
  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
}
