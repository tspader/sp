#include "sp.h"
#include "sp/sp_test.h"
#include "sp/sp_io.h"

#define TASK_QUEUE_MAX_WORKERS  8
#define TASK_QUEUE_MAX_CAPACITY 16

typedef struct {
  const c8* name;
  u32 capacity;
  u32 producers;
  u32 foreign;
  u32 consumers;
  u32 items;
} queue_test_t;

static const queue_test_t queue_tests [] = {
  { .name = "single_producer_single_consumer", .capacity = 8, .producers = 1, .consumers = 1, .items = 64 },
  { .name = "many_producers_many_consumers", .capacity = 8, .producers = 4, .consumers = 4, .items = 32 },
  { .name = "foreign_thread_producers", .capacity = 4, .producers = 1, .foreign = 2, .consumers = 2, .items = 32 },
  { .name = "capacity_one_backpressure", .capacity = 1, .producers = 2, .consumers = 2, .items = 16 },
};

typedef struct {
  sp_task_queue_t queue;
  u32 items;
  sp_atomic_u32_t sum;
  sp_atomic_u32_t count;
  u32 storage [TASK_QUEUE_MAX_CAPACITY];
} queue_state_t;

static void queue_producer(void* context) {
  queue_state_t* s = sp_cast(queue_state_t*, context);
  sp_for(it, s->items) {
    u32 value = (u32)it + 1;
    sp_assert(sp_task_queue_put(&s->queue, &value) == SP_OK);
  }
}

static s32 queue_producer_thread(void* context) {
  queue_producer(context);
  return 0;
}

static void queue_consumer(void* context) {
  queue_state_t* s = sp_cast(queue_state_t*, context);
  while (true) {
    u32 value = 0;
    if (sp_task_queue_get(&s->queue, &value)) break;
    sp_atomic_u32_add(&s->sum, value, SP_ATOMIC_RELAXED);
    sp_atomic_u32_add(&s->count, 1, SP_ATOMIC_RELAXED);
  }
}

static sp_err_t run_queue_test(sp_test_t* t, queue_test_t* c) {
  sp_test_skip_on_freestanding();

  sp_io_blocking_t io_mem;
  sp_io_t io = sp_io_blocking_init(&io_mem);
  sp_task_threaded_t* sched_mem = sp_alloc_type(sp_test_arena(t), sp_task_threaded_t);
  sp_task_sched_t sched = sp_task_threaded_init(sched_mem, io);

  queue_state_t s = sp_zero;
  s.items = c->items;
  s.queue = sp_task_queue_init(sp_mem_slice((u8*)s.storage, c->capacity * sizeof(u32)), sizeof(u32));

  sp_task_group_t producers = sp_task_group(sched);
  sp_for(it, c->producers) {
    sp_task_group_spawn(&producers, queue_producer, &s);
  }
  sp_for(it, c->consumers) {
    sp_task_spawn(sched, queue_consumer, &s);
  }

  sp_thread_t threads [TASK_QUEUE_MAX_WORKERS] = sp_zero;
  sp_for(it, c->foreign) {
    sp_thread_init(&threads[it], queue_producer_thread, &s);
  }

  sp_task_group_await(&producers);
  sp_for(it, c->foreign) {
    sp_thread_join(&threads[it]);
  }
  sp_task_queue_close(&s.queue);
  sp_task_run(sched);

  u32 senders = c->producers + c->foreign;
  sp_expect_eq(t, sp_atomic_u32_load(&s.count, SP_ATOMIC_ACQUIRE), senders * c->items);
  sp_expect_eq(t, sp_atomic_u32_load(&s.sum, SP_ATOMIC_ACQUIRE), senders * (c->items * (c->items + 1) / 2));
  return SP_OK;
}

sp_test_each_fn(task, queue, queue_test_t, queue_tests, run_queue_test);
