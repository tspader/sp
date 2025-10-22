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


////////////////////
// MEMORY TRACKER //
////////////////////
typedef struct sp_test_memory_tracker {
  sp_bump_allocator_t* bump;
  sp_allocator_t allocator;
} sp_test_memory_tracker;

void sp_test_use_bump_allocator(u32 capacity);
void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity);
void sp_test_memory_tracker_deinit(sp_test_memory_tracker* tracker);
u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker);
void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker);

#if defined(SP_TEST_IMPLEMENTATION)
void sp_test_file_manager_init(sp_test_file_manager_t* manager) {
  manager->paths.bin = sp_os_get_executable_path();
  manager->paths.build = sp_os_parent_path(manager->paths.bin);
  manager->paths.root = sp_os_parent_path(manager->paths.build);
  manager->paths.test = sp_os_join_path(manager->paths.build, sp_str_lit("test"));

  sp_os_remove_directory(manager->paths.test);
  sp_os_create_directory(manager->paths.test);
}

sp_str_t sp_test_file_path(sp_test_file_manager_t* manager, sp_str_t name) {
  return sp_os_join_path(manager->paths.test, name);
}

void sp_test_file_create_ex(sp_test_file_config_t config) {
  sp_os_remove_file(config.path);

  sp_io_stream_t stream = sp_io_from_file(config.path, SP_IO_MODE_WRITE);
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
  sp_os_remove_directory(manager->paths.test);
}

////////////////////
// MEMORY TRACKER //
////////////////////
void sp_test_use_bump_allocator(u32 capacity) {
  static sp_bump_allocator_t bump_allocator;

  bump_allocator = SP_ZERO_STRUCT(sp_bump_allocator_t);

  sp_allocator_t allocator = sp_bump_allocator_init(&bump_allocator, capacity);
  sp_context_push_allocator(allocator);
}

void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity) {
  sp_test_use_bump_allocator(capacity);
  sp_context_t* ctx = sp_context_get();
  tracker->bump = (sp_bump_allocator_t*)ctx->allocator.user_data;
  tracker->allocator = ctx->allocator;
}

void sp_test_memory_tracker_deinit(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_destroy(tracker->bump);
  sp_context_pop();
}

u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker) {
  return tracker->bump->bytes_used;
}

void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_clear(tracker->bump);
}

/////////////////
// BYTE BUFFER //
/////////////////

void sp_byte_buffer_zero(sp_byte_buffer_t* buffer) {
  sp_os_zero_memory(buffer->data, buffer->len);
}
#endif
