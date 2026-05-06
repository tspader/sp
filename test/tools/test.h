#ifndef SP_TEST_H
#define SP_TEST_H

#include "sp.h"

#if defined(SP_TEST_AMALGAMATION)
  #define SP_TEST_MAIN()
#else
  #define SP_TEST_MAIN() UTEST_MAIN()
#endif


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
    sp_str_t formatted = sp_fmt_a(utest_state.mem, fmt, ##__VA_ARGS__).value; \
    UTEST_PRINTF("{}", sp_fmt_str(formatted)); \
  } while (0)

#define SP_TEST_REPORT_STR(str) \
  do { \
    UTEST_PRINTF("{}", sp_fmt_str(str)); \
  } while (0)

#define SP_TEST_STREQ(a, b, is_assert) \
  UTEST_SURPRESS_WARNING_BEGIN do { \
    if (!sp_str_equal((a), (b))) { \
      const c8* __file = __FILE__; \
      const u32 __line = __LINE__; \
      sp_str_t __msg = sp_fmt_a( \
        utest_state.mem, \
        "{}:{} Failure:\n  {.quote} != {.quote}",     \
        sp_fmt_cstr(__file), sp_fmt_uint(__line), \
        sp_fmt_str((a)),                          \
        sp_fmt_str((b))                           \
      ).value;                                    \
      SP_TEST_REPORT_STR(__msg);                  \
      *utest_result = UTEST_TEST_FAILURE;         \
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
  sp_mem_t mem;
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


typedef struct sp_mem_tracking_node_t {
  struct sp_mem_tracking_node_t* prev;
  struct sp_mem_tracking_node_t* next;
  u64 size;
  u32 id;
  u32 magic;
} sp_mem_tracking_node_t;

#define SP_MEM_TRACKING_LIVE_MAGIC  0xA110CA7Eu
#define SP_MEM_TRACKING_FREED_MAGIC 0xDEADBEEFu

typedef struct sp_mem_tracking_t {
  sp_mem_t backing;
  sp_mem_tracking_node_t* live;
  sp_mem_tracking_node_t* freed;
  u64 live_bytes;
  u32 live_count;
  u32 double_frees;
  u32 wild_frees;
  u32 next_id;
} sp_mem_tracking_t;

void     sp_mem_tracking_init(sp_mem_tracking_t* t);
void     sp_mem_tracking_init_ex(sp_mem_tracking_t* t, sp_mem_t backing);
sp_mem_t sp_mem_tracking_as_allocator(sp_mem_tracking_t* t);
void     sp_mem_tracking_dump(sp_mem_tracking_t* t);
void     sp_mem_tracking_deinit(sp_mem_tracking_t* t);
void*    sp_mem_tracking_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u64 size, void* ptr);
bool     sp_mem_tracking_ok(sp_mem_tracking_t* mem);

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
  sp_str_t sanitized = sp_str_replace_c8_a(a, iso, ':', '-');
  sp_str_t root = sp_fs_join_path_a(a, tmp, sanitized);

  if (!sp_fs_exists_a(root)) {
    sp_fs_create_dir_a(root);
  }

  // Cache in a long-lived allocator (os) so the result outlives `a`, which is
  // typically a per-test arena that gets destroyed in cleanup.
  sp_test_file_manager_top_level = sp_fs_canonicalize_path_a(sp_mem_os_new(), root);
  return sp_test_file_manager_top_level;
}

void sp_test_file_manager_init(sp_test_file_manager_t* fs) {
  fs->arena = sp_mem_arena_new(sp_mem_os_new());
  fs->mem = sp_mem_arena_as_allocator(fs->arena);
  fs->paths.bin = sp_fs_get_exe_path_a(fs->mem);
  fs->paths.root = sp_test_file_manager_get_repo_root(fs->mem);
  fs->paths.build = fs->paths.root;
  fs->paths.test = sp_test_file_manager_get_top_level(fs->mem, fs->paths.root);

  if (!sp_fs_exists_a(fs->paths.test)) {
    sp_fs_create_dir_a(fs->paths.test);
  }
}

sp_str_t sp_test_file_path(sp_test_file_manager_t* manager, sp_str_t name) {
  return sp_fs_join_path_a(manager->mem, manager->paths.test, name);
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

/////////////////
// BYTE BUFFER //
/////////////////

void sp_byte_buffer_zero(sp_byte_buffer_t* buffer) {
  sp_mem_zero(buffer->data, buffer->len);
}

///////////////////////
// TRACKING ALLOCATOR //
///////////////////////
static sp_mem_tracking_node_t* sp_mem_tracking_header(void* ptr) {
  return (sp_mem_tracking_node_t*)((u8*)ptr - sizeof(sp_mem_tracking_node_t));
}

static void* sp_mem_tracking_user_ptr(sp_mem_tracking_node_t* node) {
  return (u8*)node + sizeof(sp_mem_tracking_node_t);
}

static void sp_mem_tracking_link(sp_mem_tracking_node_t** list, sp_mem_tracking_node_t* node) {
  node->prev = SP_NULLPTR;
  node->next = *list;
  if (*list) (*list)->prev = node;
  *list = node;
}

static void sp_mem_tracking_unlink(sp_mem_tracking_node_t** list, sp_mem_tracking_node_t* node) {
  if (node->prev) node->prev->next = node->next;
  else            *list             = node->next;
  if (node->next) node->next->prev = node->prev;
  node->prev = SP_NULLPTR;
  node->next = SP_NULLPTR;
}

static void* sp_mem_tracking_do_alloc(sp_mem_tracking_t* t, u64 size) {
  if (!size) return SP_NULLPTR;
  void* raw = sp_alloc_a(t->backing, size + sizeof(sp_mem_tracking_node_t));
  if (!raw) return SP_NULLPTR;

  sp_mem_tracking_node_t* node = (sp_mem_tracking_node_t*)raw;
  node->size = size;
  node->magic = SP_MEM_TRACKING_LIVE_MAGIC;
  node->id = ++t->next_id;
  sp_mem_tracking_link(&t->live, node);
  t->live_count++;
  t->live_bytes += size;
  return sp_mem_tracking_user_ptr(node);
}

static void sp_mem_tracking_do_free(sp_mem_tracking_t* t, void* ptr) {
  if (!ptr) return;
  sp_mem_tracking_node_t* node = sp_mem_tracking_header(ptr);

  if (node->magic == SP_MEM_TRACKING_LIVE_MAGIC) {
    sp_mem_tracking_unlink(&t->live, node);
    node->magic = SP_MEM_TRACKING_FREED_MAGIC;
    t->live_count--;
    t->live_bytes -= node->size;
    // Retain the node on the freed list (don't actually return it to the
    // backing) so that subsequent frees of the same pointer can be detected
    // as double-frees. Released in deinit.
    sp_mem_tracking_link(&t->freed, node);
  }
  else if (node->magic == SP_MEM_TRACKING_FREED_MAGIC) {
    t->double_frees++;
  }
  else {
    t->wild_frees++;
  }
}

static void* sp_mem_tracking_do_realloc(sp_mem_tracking_t* t, void* old, u64 size) {
  if (!old)  return sp_mem_tracking_do_alloc(t, size);
  if (!size) { sp_mem_tracking_do_free(t, old); return SP_NULLPTR; }

  sp_mem_tracking_node_t* node = sp_mem_tracking_header(old);
  if (node->magic != SP_MEM_TRACKING_LIVE_MAGIC) {
    if (node->magic == SP_MEM_TRACKING_FREED_MAGIC) t->double_frees++;
    else                                            t->wild_frees++;
    return SP_NULLPTR;
  }
  if (node->size >= size) return old;

  void* fresh = sp_mem_tracking_do_alloc(t, size);
  if (!fresh) return SP_NULLPTR;
  sp_mem_copy(old, fresh, node->size);
  sp_mem_tracking_do_free(t, old);
  return fresh;
}

void* sp_mem_tracking_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u64 size, void* ptr) {
  sp_mem_tracking_t* t = (sp_mem_tracking_t*)ud;
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC:  return sp_mem_tracking_do_alloc(t, size);
    case SP_ALLOCATOR_MODE_RESIZE: return sp_mem_tracking_do_realloc(t, ptr, size);
    case SP_ALLOCATOR_MODE_FREE:   sp_mem_tracking_do_free(t, ptr); return SP_NULLPTR;
    default:                       return SP_NULLPTR;
  }
}

void sp_mem_tracking_init_ex(sp_mem_tracking_t* t, sp_mem_t backing) {
  sp_mem_zero(t, sizeof(*t));
  t->backing = backing;
}

void sp_mem_tracking_init(sp_mem_tracking_t* t) {
  sp_mem_tracking_init_ex(t, sp_mem_os_new());
}

sp_mem_t sp_mem_tracking_as_allocator(sp_mem_tracking_t* t) {
  return (sp_mem_t) {
    .on_alloc = sp_mem_tracking_on_alloc,
    .user_data = t,
  };
}

u32 sp_mem_tracking_live_count(sp_mem_tracking_t* t)   { return t->live_count; }
u64 sp_mem_tracking_live_bytes(sp_mem_tracking_t* t)   { return t->live_bytes; }
u32 sp_mem_tracking_double_frees(sp_mem_tracking_t* t) { return t->double_frees; }
u32 sp_mem_tracking_wild_frees(sp_mem_tracking_t* t)   { return t->wild_frees; }

void sp_mem_tracking_dump(sp_mem_tracking_t* t) {
  sp_log_a("tracking: live={} bytes={} double_frees={} wild_frees={}",
    sp_fmt_uint(t->live_count),
    sp_fmt_uint(t->live_bytes),
    sp_fmt_uint(t->double_frees),
    sp_fmt_uint(t->wild_frees));
  for (sp_mem_tracking_node_t* n = t->live; n; n = n->next) {
    sp_log_a("  #{} size={}", sp_fmt_uint(n->id), sp_fmt_uint(n->size));
  }
}

void sp_mem_tracking_deinit(sp_mem_tracking_t* t) {
  sp_mem_tracking_node_t* n = t->live;
  while (n) {
    sp_mem_tracking_node_t* next = n->next;
    sp_free_a(t->backing, n);
    n = next;
  }
  n = t->freed;
  while (n) {
    sp_mem_tracking_node_t* next = n->next;
    sp_free_a(t->backing, n);
    n = next;
  }
  sp_mem_zero(t, sizeof(*t));
}

bool sp_mem_tracking_ok(sp_mem_tracking_t* mem) {
  return
    (mem->live_count == 0) &&
    (mem->double_frees == 0) &&
    (mem->wild_frees == 0);
}
#endif
#endif
