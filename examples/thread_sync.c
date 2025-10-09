#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct {
  int id;
} task_t;

typedef struct {
  bool stop;
  sp_mutex_t queue_mutex;
  sp_semaphore_t done_sem;
  task_t queue[4];
  u32 queue_size;
} worker_data_t;

static worker_data_t worker_data;

static void process_task(task_t* task) {
  SP_UNUSED(task);
}

static s32 worker_thread(void* userdata) {
  worker_data_t* data = (worker_data_t*)userdata;

  while (!data->stop) {
    sp_mutex_lock(&data->queue_mutex);
    if (data->queue_size > 0) {
      task_t task = data->queue[--data->queue_size];
      sp_mutex_unlock(&data->queue_mutex);

      process_task(&task);

      sp_semaphore_signal(&data->done_sem);
    } else {
      sp_mutex_unlock(&data->queue_mutex);
      sp_os_sleep_ms(10);
    }
  }
  return 0;
}

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

int main(void) {
  sp_example_init();

  sp_mutex_init(&worker_data.queue_mutex, SP_MUTEX_PLAIN);
  sp_semaphore_init(&worker_data.done_sem);
  worker_data.queue_size = 1;
  worker_data.queue[0] = (task_t){ .id = 1 };
  worker_data.stop = false;

  sp_thread_t thread;
  sp_thread_init(&thread, worker_thread, &worker_data);
  sp_semaphore_wait(&worker_data.done_sem);
  worker_data.stop = true;
  sp_thread_join(&thread);

  sp_example_shutdown();
  return 0;
}
