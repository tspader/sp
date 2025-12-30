#ifndef SP_TEST_H
#define SP_TEST_H

#include "sp.h"

#define ut (*utest_fixture)
#define ur (*utest_result)

#define SP_TEST_REPORT(fmt, ...) \
  do { \
    sp_str_t formatted = sp_format_str(fmt, ##__VA_ARGS__); \
    UTEST_PRINTF("%s\n", sp_str_to_cstr(formatted)); \
  } while (0)

#define SP_TEST_STREQ(a, b, is_assert) \
  UTEST_SURPRESS_WARNING_BEGIN do { \
    if (!sp_str_equal((a), (b))) { \
      const c8* __sp_test_file_lval = __FILE__; \
      const u32 __sp_test_line_lval = __LINE__; \
      sp_str_builder_t __sp_test_builder = SP_ZERO_INITIALIZE(); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("{}:{} Failure:"), SP_FMT_CSTR(__sp_test_file_lval), SP_FMT_U32(__sp_test_line_lval)); \
      sp_str_builder_new_line(&__sp_test_builder); \
      sp_str_builder_indent(&__sp_test_builder); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("{} != {}"), SP_FMT_QUOTED_STR((a)), SP_FMT_QUOTED_STR((b))); \
      SP_TEST_REPORT(sp_str_builder_write(&__sp_test_builder)); \
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
#define SP_EXPECT_ERR(err) EXPECT_EQ(sp_err_get(), err)


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


typedef struct {
  sp_str_t key;
  sp_str_t before;
  sp_str_t after;
} sp_test_env_var_t;

typedef struct {
  sp_dyn_array(sp_test_env_var_t) vars;
} sp_test_env_manager_t;

void sp_test_env_manager_init(sp_test_env_manager_t* manager);
void sp_test_env_manager_set(sp_test_env_manager_t* manager, sp_str_t key, sp_str_t value);
void sp_test_env_manager_unset(sp_test_env_manager_t* manager, sp_str_t key);
void sp_test_env_manager_cleanup(sp_test_env_manager_t* manager);


typedef struct sp_test_memory_tracker {
  sp_mem_arena_t* bump;
  sp_allocator_t allocator;
} sp_test_memory_tracker;

void sp_test_use_mem_arena(u32 capacity);
void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity);
void sp_test_memory_tracker_deinit(sp_test_memory_tracker* tracker);
u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker);
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
#define SP_TEST_C

#if defined(SP_TEST_IMPLEMENTATION)
void sp_test_file_manager_init(sp_test_file_manager_t* manager) {
  manager->paths.bin = sp_fs_get_exe_path();
  manager->paths.build = sp_fs_parent_path(manager->paths.bin);
  manager->paths.root = sp_fs_parent_path(manager->paths.build);
  manager->paths.test = sp_fs_join_path(manager->paths.build, sp_str_lit("test"));

  //sp_fs_remove_dir(manager->paths.test);
  sp_fs_create_dir(manager->paths.test);
}

sp_str_t sp_test_file_path(sp_test_file_manager_t* manager, sp_str_t name) {
  return sp_fs_join_path(manager->paths.test, name);
}

void sp_test_file_create_ex(sp_test_file_config_t config) {
  sp_fs_remove_file(config.path);

  sp_io_t stream = sp_io_from_file(config.path, SP_IO_MODE_WRITE);
  SP_ASSERT(stream.file.fd != 0);

  if (config.content.len > 0) {
    u64 bytes_written = sp_io_write(&stream, config.content.data, config.content.len);
    SP_ASSERT(bytes_written == config.content.len);
  }

  sp_io_close(&stream);
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
  sp_fs_remove_dir(manager->paths.test);
}

////////////////////
// MEMORY TRACKER //
////////////////////
void sp_test_use_mem_arena(u32 capacity) {
  static sp_mem_arena_t* mem_arena;

  mem_arena = sp_mem_arena_new(capacity);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(mem_arena);
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

u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker) {
  return sp_mem_arena_bytes_used(tracker->bump);
}

void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker) {
  sp_mem_arena_clear(tracker->bump);
}

/////////////////
// ENV MANAGER //
/////////////////

void sp_test_env_manager_init(sp_test_env_manager_t* manager) {
  manager->vars = SP_NULLPTR;
}

void sp_test_env_manager_set(sp_test_env_manager_t* manager, sp_str_t key, sp_str_t value) {
  sp_str_t before = sp_os_get_env_var(key);
  sp_test_env_var_t var = {
    .key = key,
    .before = before,
    .after = value,
  };
  sp_dyn_array_push(manager->vars, var);
  sp_os_export_env_var(key, value, SP_ENV_EXPORT_OVERWRITE_DUPES);
}

void sp_test_env_manager_unset(sp_test_env_manager_t* manager, sp_str_t key) {
  sp_str_t before = sp_os_get_env_var(key);
  sp_test_env_var_t var = {
    .key = key,
    .before = before,
    .after = SP_LIT(""),
  };
  sp_dyn_array_push(manager->vars, var);
  sp_os_clear_env_var(key);
}

void sp_test_env_manager_cleanup(sp_test_env_manager_t* manager) {
  sp_dyn_array_for(manager->vars, i) {
    sp_test_env_var_t var = manager->vars[i];
    if (!sp_str_empty(var.before)) {
      sp_os_export_env_var(var.key, var.before, SP_ENV_EXPORT_OVERWRITE_DUPES);
    } else {
      sp_os_clear_env_var(var.key);
    }
  }
  sp_dyn_array_free(manager->vars);
}

/////////////////
// BYTE BUFFER //
/////////////////

void sp_byte_buffer_zero(sp_byte_buffer_t* buffer) {
  sp_mem_zero(buffer->data, buffer->len);
}
#endif
#endif
