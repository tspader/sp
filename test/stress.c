#include "sp.h"
#include "test.h"

#include "utest.h"


UTEST(stress, dyn_array) {
  sp_dyn_array(u64) arr = SP_NULLPTR;

  const s32 count = 100000;

  for (s32 i = 0; i < count; i++) {
      sp_dyn_array_push(arr, (u64)i * 12345);
  }

  EXPECT_EQ(sp_dyn_array_size(arr), count);

  for (s32 i = 0; i < count; i++) {
      EXPECT_EQ(arr[i], (u64)i * 12345);
  }

  sp_dyn_array_clear(arr);
  EXPECT_EQ(sp_dyn_array_size(arr), 0);

  for (s32 i = 0; i < 1000; i++) {
      sp_dyn_array_push(arr, (u64)i);
  }
  EXPECT_EQ(sp_dyn_array_size(arr), 1000);

  sp_dyn_array_free(arr);
}

UTEST(stress, hash_table) {
  sp_ht(u64, u64) ht = SP_NULLPTR;

  const s32 count = 10000;

  for (u64 i = 0; i < count; i++) {
      sp_ht_insert(ht, i, i * i);
  }

  EXPECT_EQ(sp_ht_size(ht), count);

  for (s32 i = 0; i < 100; i++) {
      u64 key = rand() % count;
      EXPECT_TRUE(sp_ht_exists(ht, key));
      EXPECT_EQ(*sp_ht_getp(ht, key), key * key);
  }

  for (u64 i = 0; i < count; i += 2) {
      sp_ht_erase(ht, i);
  }

  EXPECT_EQ(sp_ht_size(ht), count / 2);

  for (u64 i = 1; i < count; i += 2) {
      EXPECT_TRUE(sp_ht_exists(ht, i));
      EXPECT_EQ(*sp_ht_getp(ht, i), i * i);
  }

  sp_ht_free(ht);
}

UTEST(stress, ring_buffer) {
  sp_rb(u64) rq = SP_NULLPTR;

  for (u64 i = 0; i < 1000; i++) {
    sp_rb_push(rq, i);
  }

  EXPECT_EQ(1000, sp_rb_size(rq));

  for (u64 i = 0; i < 500; i++) {
    u64* val = sp_rb_peek(rq);
    EXPECT_EQ(*val, i);
    sp_rb_pop(rq);
  }

  for (u64 i = 1000; i < 1500; i++) {
    sp_rb_push(rq, i);
  }

  EXPECT_EQ(1000, sp_rb_size(rq));

  u64 expected = 500;
  sp_rb_for(rq, it) {
    EXPECT_EQ(sp_rb_at(rq, it), expected);
    expected++;
  }

  sp_rb_free(rq);
}

UTEST(stress, ring_buffer_continuous_overwrite) {
  sp_rb(s32) rq = SP_NULLPTR;

  sp_rb_set_mode(rq, SP_RQ_MODE_OVERWRITE);

  s32 cap = sp_rb_capacity(rq);

  for (s32 i = 0; i < 10000; i++) {
    sp_rb_push(rq, i);
  }

  EXPECT_EQ(cap, sp_rb_size(rq));

  for (s32 i = 0; i < cap; i++) {
    s32* val = sp_rb_peek(rq);
    EXPECT_EQ(*val, 10000 - cap + i);
    sp_rb_pop(rq);
  }

  EXPECT_TRUE(sp_rb_empty(rq));

  sp_rb_free(rq);
}

UTEST(stress, sp_dyn_array_push_f) {
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

  EXPECT_EQ(sp_dyn_array_size(arr), 1000);

  for (u32 i = 0; i < 1000; i++) {
    EXPECT_EQ(arr[i].id, i);
    for (int j = 0; j < 256; j++) {
      EXPECT_EQ(arr[i].data[j], (c8)((i + j) % 256));
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

UTEST(stress, sp_spin_lock) {
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

  EXPECT_EQ(shared_counter, SP_SPIN_LOCK_STRESS_THREADS * SP_SPIN_LOCK_STRESS_ITERATIONS);
  EXPECT_EQ(lock, 0);
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

UTEST(stress, sp_atomic_s32) {
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
  EXPECT_TRUE(final >= 0);
}

UTEST(stress, sp_context) {
  srand(69);

  sp_tls_rt_t* rt = sp_tls_rt_get();
  u32 initial_index = rt->index;
  sp_mem_arena_t* arena = sp_mem_get_scratch_arena();

  const s32 iterations = 1000;
  u64 sum = 0;

  for (s32 iter = 0; iter < iterations; iter++) {
    // 1. Push context with libc allocator
    sp_context_push_allocator(sp_mem_libc_new());

    // 2. Libc allocations - track with dynamic array (uses libc allocator)
    s32 libc_count = 512 + (rand() % 513);  // 512-1024
    sp_da(u32*) libc_allocs = SP_NULLPTR;

    for (s32 i = 0; i < libc_count; i++) {
      s32 size = 4 + (rand() % 509);  // 4-512 u32s
      u32* buf = sp_alloc(size * sizeof(u32));
      for (s32 j = 0; j < size; j++) {
        buf[j] = (u32)(iter + i + j);
        sum += buf[j];
      }
      sp_dyn_array_push(libc_allocs, buf);
    }

    // 3. Begin scratch
    sp_mem_scratch_t scratch = sp_mem_begin_scratch();

    // 4. Scratch allocations - use fixed-size array since we know max count
    // Limit: 1MB scratch arena, so max ~256 allocs of ~256 u32s = 256KB
    s32 scratch_count = 64 + (rand() % 193);  // 64-256
    u32* scratch_ptrs[256];
    s32 scratch_sizes[256];

    for (s32 i = 0; i < scratch_count; i++) {
      s32 size = 4 + (rand() % 253);  // 4-256 u32s (16-1024 bytes)
      u32* buf = sp_alloc(size * sizeof(u32));
      for (s32 j = 0; j < size; j++) {
        buf[j] = (u32)(iter * 1000 + i + j);
        sum += buf[j];
      }
      scratch_ptrs[i] = buf;
      scratch_sizes[i] = size;
    }

    // 5. Access all scratch buffers (for ASAN)
    for (s32 i = 0; i < scratch_count; i++) {
      u32* buf = scratch_ptrs[i];
      s32 size = scratch_sizes[i];
      for (s32 j = 0; j < size; j++) {
        sum += buf[j];
      }
    }

    // 6. End scratch
    sp_mem_end_scratch(scratch);

    // 7. Free libc allocations
    sp_dyn_array_for(libc_allocs, i) {
      sp_free(libc_allocs[i]);
    }
    sp_dyn_array_free(libc_allocs);

    // 8. Pop context
    sp_context_pop();
  }

  // Verify scratch arena is clean
  EXPECT_EQ(sp_mem_arena_bytes_used(arena), 0);
  EXPECT_EQ(rt->index, initial_index);
  SP_UNUSED(sum);
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
#define FMON_STRESS_TOTAL_FILES 10000
#define FMON_STRESS_BATCH_SIZE 1000
#define FMON_STRESS_DIR_DEPTH 3
#define FMON_STRESS_DIRS_PER_LEVEL 5

typedef struct {
  sp_atomic_s32 add_count;
  sp_atomic_s32 mod_count;
  sp_atomic_s32 rem_count;
} fmon_stress_counters_t;

void fmon_stress_callback(sp_fmon_t* monitor, sp_fmon_event_t* event, void* userdata) {
  (void)monitor;
  fmon_stress_counters_t* counters = (fmon_stress_counters_t*)userdata;
  if (event->events & SP_FILE_CHANGE_EVENT_ADDED) {
    sp_atomic_s32_add(&counters->add_count, 1);
  }
  if (event->events & SP_FILE_CHANGE_EVENT_MODIFIED) {
    sp_atomic_s32_add(&counters->mod_count, 1);
  }
  if (event->events & SP_FILE_CHANGE_EVENT_REMOVED) {
    sp_atomic_s32_add(&counters->rem_count, 1);
  }
}

void fmon_stress_poll(sp_fmon_t* monitor) {
  for (s32 i = 0; i < 5; i++) {
    sp_fmon_process_changes(monitor);
    sp_os_sleep_ms(50);
  }
}

void fmon_stress_create_dir_tree(sp_str_t base, s32 depth, sp_da(sp_str_t)* dirs) {
  sp_da_push(*dirs, sp_str_copy(base));
  if (depth <= 0) return;

  for (s32 i = 0; i < FMON_STRESS_DIRS_PER_LEVEL; i++) {
    sp_str_t name = sp_format_str(SP_LIT("d{}"), SP_FMT_S32(i));
    sp_str_t child = sp_fs_join_path(base, name);
    sp_fs_create_dir(child);
    fmon_stress_create_dir_tree(child, depth - 1, dirs);
  }
}

UTEST(stress, fmon) {
  sp_test_file_manager_t file_manager;
  sp_test_file_manager_init(&file_manager);

  fmon_stress_counters_t counters = {0};
  sp_fmon_t* monitor = SP_ALLOC(sp_fmon_t);
  sp_fmon_init(monitor, fmon_stress_callback,
               SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_REMOVED,
               &counters);

  // Create root watched directory
  sp_str_t root = sp_test_file_path(&file_manager, sp_str_lit("fmon_stress"));
  sp_fs_create_dir(root);
  sp_fmon_add_dir(monitor, root);

  // Create directory tree
  sp_da(sp_str_t) dirs = SP_NULLPTR;
  fmon_stress_create_dir_tree(root, FMON_STRESS_DIR_DEPTH, &dirs);

  // Poll to clear initial dir creation events
  fmon_stress_poll(monitor);
  sp_atomic_s32_set(&counters.add_count, 0);
  sp_atomic_s32_set(&counters.mod_count, 0);
  sp_atomic_s32_set(&counters.rem_count, 0);

  u32 num_dirs = sp_da_size(dirs);
  UTEST_PRINTF("Created %u directories\n", num_dirs);

  // Phase 1: Create files in batches
  sp_da(sp_str_t) files = SP_NULLPTR;
  u32 files_created = 0;
  u32 dir_idx = 0;

  while (files_created < FMON_STRESS_TOTAL_FILES) {
    u32 batch_end = files_created + FMON_STRESS_BATCH_SIZE;
    if (batch_end > FMON_STRESS_TOTAL_FILES) batch_end = FMON_STRESS_TOTAL_FILES;

    while (files_created < batch_end) {
      sp_str_t dir = dirs[dir_idx % num_dirs];
      sp_str_t name = sp_format_str(SP_LIT("f{}.txt"), SP_FMT_U32(files_created));
      sp_str_t path = sp_fs_join_path(dir, name);

      sp_io_stream_t s = sp_io_from_file(path, SP_IO_MODE_WRITE);
      sp_io_write_str(&s, sp_str_lit("initial"));
      sp_io_close(&s);

      sp_da_push(files, path);
      files_created++;
      dir_idx++;
    }

    // Poll between batches
    fmon_stress_poll(monitor);
  }

  s32 add_after_create = sp_atomic_s32_get(&counters.add_count);
  UTEST_PRINTF("Phase 1 (create): %d add events for %u files\n", add_after_create, files_created);

  // Phase 2: Modify files in batches
  u32 files_modified = 0;
  while (files_modified < FMON_STRESS_TOTAL_FILES) {
    u32 batch_end = files_modified + FMON_STRESS_BATCH_SIZE;
    if (batch_end > FMON_STRESS_TOTAL_FILES) batch_end = FMON_STRESS_TOTAL_FILES;

    while (files_modified < batch_end) {
      sp_str_t path = files[files_modified];
      sp_io_stream_t s = sp_io_from_file(path, SP_IO_MODE_WRITE);
      sp_io_write_str(&s, sp_str_lit("modified"));
      sp_io_close(&s);
      files_modified++;
    }

    fmon_stress_poll(monitor);
  }

  s32 mod_after_modify = sp_atomic_s32_get(&counters.mod_count);
  UTEST_PRINTF("Phase 2 (modify): %d mod events for %u files\n", mod_after_modify, files_modified);

  // Phase 3: Delete files in batches
  u32 files_deleted = 0;
  while (files_deleted < FMON_STRESS_TOTAL_FILES) {
    u32 batch_end = files_deleted + FMON_STRESS_BATCH_SIZE;
    if (batch_end > FMON_STRESS_TOTAL_FILES) batch_end = FMON_STRESS_TOTAL_FILES;

    while (files_deleted < batch_end) {
      sp_str_t path = files[files_deleted];
      sp_fs_remove_file(path);
      files_deleted++;
    }

    fmon_stress_poll(monitor);
  }

  s32 rem_after_delete = sp_atomic_s32_get(&counters.rem_count);
  UTEST_PRINTF("Phase 3 (delete): %d rem events for %u files\n", rem_after_delete, files_deleted);

  // Verify we got a reasonable number of events (coalescing is expected)
  s32 total_events = add_after_create + mod_after_modify + rem_after_delete;
  UTEST_PRINTF("Total events: %d (expected ~%u operations)\n", total_events, FMON_STRESS_TOTAL_FILES * 3);

  // Should get at least 10% of operations as events (very conservative due to coalescing)
  EXPECT_TRUE(total_events > FMON_STRESS_TOTAL_FILES / 10);

  // Cleanup
  sp_fmon_deinit(monitor);
  sp_free(monitor);
  sp_da_free(files);
  sp_da_free(dirs);
  sp_test_file_manager_cleanup(&file_manager);
}
#endif

SP_TEST_MAIN()
