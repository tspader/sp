#ifndef SP_TEST_H
#define SP_TEST_H

#include "sp.h"

#if defined(SP_FREESTANDING)
  #define SKIP_ON_FREESTANDING() UTEST_SKIP("unimplemented on freestanding");
#else
  #define SKIP_ON_FREESTANDING()
#endif

#if defined(SP_WASM)
  #define SKIP_ON_WASM() UTEST_SKIP("unimplemented on wasm");
#else
  #define SKIP_ON_WASM()
#endif

#if defined(SP_WIN32)
  #define SKIP_ON_WIN32() UTEST_SKIP("");
#else
  #define SKIP_ON_WIN32()
#endif

#if defined(SP_MACOS)
  #define SKIP_ON_MACOS() UTEST_SKIP("");
#else
  #define SKIP_ON_MACOS()
#endif

#define ut (*utest_fixture)
#define ur (*utest_result)

#define SP_FAIL() *utest_result = UTEST_TEST_FAILURE

#define SP_TEST_REPORT(fmt, ...) \
  do { \
    sp_str_t formatted = sp_fmt(fmt, ##__VA_ARGS__); \
    UTEST_PRINTF("{}", sp_fmt_str(formatted)); \
  } while (0)

#define SP_TEST_REPORT_STR(str) \
  do { \
    UTEST_PRINTF("{}", sp_fmt_str(str)); \
  } while (0)

#define SP_TEST_STREQ(a, b, is_assert) \
  UTEST_SURPRESS_WARNING_BEGIN do { \
    if (!sp_str_equal((a), (b))) { \
      const c8* __sp_test_file_lval = __FILE__; \
      const u32 __sp_test_line_lval = __LINE__; \
      sp_str_builder_t __sp_test_builder = SP_ZERO_INITIALIZE(); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("{}:{} Failure:"), sp_fmt_cstr(__sp_test_file_lval), sp_fmt_uint(__sp_test_line_lval)); \
      sp_str_builder_new_line(&__sp_test_builder); \
      sp_str_builder_indent(&__sp_test_builder); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("\"{}\" != \"{}\""), sp_fmt_str((a)), sp_fmt_str((b))); \
      SP_TEST_REPORT_STR(sp_str_builder_to_str(&__sp_test_builder)); \
      *utest_result = UTEST_TEST_FAILURE; \
 \
      if (is_assert) { \
        return; \
      } \
    } \
  } while (0) \
  UTEST_SURPRESS_WARNING_END

#define SP_EXPECT_STR_EQ_CSTR(a, b) SP_TEST_STREQ((a), SP_CSTR(b), false)
#define SP_EXPECT_STR_EQ(a, b) SP_TEST_STREQ((a), (b), false)

typedef struct {
  u32 len;
  u8* data;
} sp_byte_buffer_t;

void sp_byte_buffer_zero(sp_byte_buffer_t* buffer);


typedef struct {
  sp_str_t root;
  sp_str_t   build;
  sp_str_t     bin;
  sp_str_t     test;
} sp_test_file_paths_t;

typedef struct {
  sp_test_file_paths_t paths;
  sp_mem_arena_t* arena;
  sp_mem_t allocator;
} sp_test_file_manager_t;

typedef struct {
  sp_str_t path;
  sp_str_t content;
} sp_test_file_config_t;

void sp_test_file_manager_init(sp_test_file_manager_t* manager);
sp_str_t sp_test_file_path(sp_test_file_manager_t* manager, sp_str_t name);
void sp_test_file_create_ex(sp_test_file_config_t config);
sp_str_t sp_test_file_create_empty(sp_test_file_manager_t* manager, sp_str_t path);
void sp_test_file_manager_cleanup(sp_test_file_manager_t* manager);


typedef struct sp_test_memory_tracker {
  sp_mem_arena_t* bump;
  sp_mem_t allocator;
} sp_test_memory_tracker;

void sp_test_use_mem_arena(u32 capacity);
void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity);
void sp_test_memory_tracker_deinit(sp_test_memory_tracker* tracker);
u64 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker);
void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker);

#if defined(SP_TEST_AMALGAMATION)
  #define SP_TEST_MAIN()
#else
  #define SP_TEST_MAIN() UTEST_MAIN()
#endif
#endif



////////////////////
// IMPLEMENTATION //
////////////////////
#if !defined(SP_TEST_C)
#if defined(SP_TEST_IMPLEMENTATION)
#define SP_TEST_C

static sp_str_t sp_test_file_manager_top_level = SP_ZERO_INITIALIZE();

static sp_str_t sp_test_file_manager_get_repo_root(sp_mem_t a) {
  sp_str_t path = sp_fs_get_exe_path_a(a);
  if (sp_fs_exists_a(path) && sp_fs_is_file_a(path)) {
    path = sp_fs_parent_path(path);
  }

  while (!sp_str_empty(path)) {
    if (sp_str_equal(sp_fs_get_name(path), SP_LIT("sp"))) {
      sp_str_t marker = sp_fs_join_path_a(a, path, SP_LIT("sp.h"));
      if (sp_fs_exists_a(marker)) {
        return sp_fs_canonicalize_path_a(a, path);
      }
    }

    if (sp_fs_is_root(path)) {
      break;
    }

    sp_str_t parent = sp_fs_parent_path(path);
    if (sp_str_equal(parent, path)) {
      break;
    }

    path = parent;
  }

  SP_ASSERT(false);
  return sp_fs_canonicalize_path_a(a, sp_fs_get_cwd_a(a));
}

static sp_str_t sp_test_file_manager_get_top_level(sp_mem_t a, sp_str_t repo_root) {
  if (!sp_str_empty(sp_test_file_manager_top_level)) {
    if (!sp_fs_exists_a(sp_test_file_manager_top_level)) {
      sp_fs_create_dir_a(sp_test_file_manager_top_level);
    }
    return sp_test_file_manager_top_level;
  }

  sp_str_t tmp = sp_fs_join_path_a(a, repo_root, SP_LIT(".tmp"));
  if (!sp_fs_exists_a(tmp)) {
    sp_fs_create_dir_a(tmp);
  }

  sp_tm_epoch_t now = sp_tm_now_epoch();
  sp_str_t iso = sp_tm_epoch_to_iso8601_a(a, now);
  sp_str_t sanitized = sp_str_replace_c8(iso, ':', '-');
  sp_str_t root = sp_fs_join_path_a(a, tmp, sanitized);

  if (!sp_fs_exists_a(root)) {
    sp_fs_create_dir_a(root);
  }

  // Cache in a long-lived allocator (os) so the result outlives `a`, which is
  // typically a per-test arena that gets destroyed in cleanup.
  sp_test_file_manager_top_level = sp_fs_canonicalize_path_a(sp_mem_os_new(), root);
  return sp_test_file_manager_top_level;
}

void sp_test_file_manager_init(sp_test_file_manager_t* manager) {
  manager->arena = sp_mem_arena_new();
  manager->allocator = sp_mem_arena_as_allocator(manager->arena);
  manager->paths.bin = sp_fs_get_exe_path_a(manager->allocator);
  manager->paths.root = sp_test_file_manager_get_repo_root(manager->allocator);
  manager->paths.build = manager->paths.root;
  manager->paths.test = sp_test_file_manager_get_top_level(manager->allocator, manager->paths.root);

  if (!sp_fs_exists_a(manager->paths.test)) {
    sp_fs_create_dir_a(manager->paths.test);
  }
}

sp_str_t sp_test_file_path(sp_test_file_manager_t* manager, sp_str_t name) {
  return sp_fs_join_path_a(manager->allocator, manager->paths.test, name);
}

void sp_test_file_create_ex(sp_test_file_config_t config) {
  sp_str_t parent = sp_fs_parent_path(config.path);
  if (!sp_str_empty(parent) && !sp_str_equal(parent, config.path) && !sp_fs_exists_a(parent)) {
    sp_fs_create_dir_a(parent);
  }

  sp_fs_remove_file_a(config.path);

  sp_io_writer_t stream = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file_a(&stream, config.path, SP_IO_WRITE_MODE_OVERWRITE);
  SP_ASSERT(stream.file.fd != 0);

  if (config.content.len > 0) {
    u64 bytes_written = 0;
    sp_io_write(&stream, config.content.data, config.content.len, &bytes_written);
    SP_ASSERT(bytes_written == config.content.len);
  }

  sp_io_writer_close(&stream);
}

sp_str_t sp_test_file_create_empty(sp_test_file_manager_t* manager, sp_str_t relative) {
  sp_str_t path = sp_test_file_path(manager, relative);
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = path,
    .content = SP_LIT(""),
  });

  return path;
}

void sp_test_file_manager_cleanup(sp_test_file_manager_t* manager) {
  if (sp_str_empty(manager->paths.test)) {
    return;
  }

  if (sp_str_equal(manager->paths.test, manager->paths.root)) {
    SP_ASSERT(false);
    return;
  }

  if (sp_fs_exists_a(manager->paths.test)) {
    sp_fs_remove_dir_a(manager->paths.test);
  }

  if (manager->arena) {
    sp_mem_arena_destroy(manager->arena);
    manager->arena = SP_NULLPTR;
  }
}

////////////////////
// MEMORY TRACKER //
////////////////////
void sp_test_use_mem_arena(u32 capacity) {
  static sp_mem_arena_t* mem_arena;

  mem_arena = sp_mem_arena_new_ex(capacity, SP_MEM_ARENA_MODE_DEFAULT, SP_MEM_ALIGNMENT);
  sp_mem_t allocator = sp_mem_arena_as_allocator(mem_arena);
  sp_context_push_allocator(allocator);
}

void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity) {
  sp_test_use_mem_arena(capacity);
  sp_context_t* ctx = sp_context_get();
  tracker->bump = (sp_mem_arena_t*)ctx->allocator.user_data;
  tracker->allocator = ctx->allocator;
}

void sp_test_memory_tracker_deinit(sp_test_memory_tracker* tracker) {
  sp_mem_arena_destroy(tracker->bump);
  sp_context_pop();
}

u64 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker) {
  return sp_mem_arena_bytes_used(tracker->bump);
}

void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker) {
  sp_mem_arena_clear(tracker->bump);
}

/////////////////
// BYTE BUFFER //
/////////////////

void sp_byte_buffer_zero(sp_byte_buffer_t* buffer) {
  sp_mem_zero(buffer->data, buffer->len);
}
#endif
#endif
