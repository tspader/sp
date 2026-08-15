#ifndef SP_TEST_H
#define SP_TEST_H

#include "sp.h"
#include "sp_glob.h"

typedef struct sp_test_t sp_test_t;

SP_TYPEDEF_FN(sp_err_t, sp_test_fn_t, sp_test_t* t);
SP_TYPEDEF_FN(sp_err_t, sp_test_each_fn_t, sp_test_t* t, const void* it);
SP_TYPEDEF_FN(sp_err_t, sp_test_setup_fn_t, sp_test_t* t);
SP_TYPEDEF_FN(void, sp_test_teardown_fn_t, sp_test_t* t);
SP_TYPEDEF_FN(const c8*, sp_test_axis_name_fn_t, s64 value);

#define SP_TEST_MAX_AXES 4

typedef enum {
  SP_TEST_AXIS_NONE,
  SP_TEST_AXIS_VALUES,
  SP_TEST_AXIS_RANGE,
} sp_test_axis_kind_t;

typedef struct {
  sp_test_axis_kind_t kind;
  const c8* field;
  u64 offset;
  u64 size;
  const s64* values;
  u64 count;
  sp_test_axis_name_fn_t name;
  s64 from;
  s64 to;
  s64 step;
} sp_test_axis_t;

typedef struct {
  const c8* name;
  sp_test_fn_t fn;
  sp_test_each_fn_t each;
  const void* cases;
  u64 stride;
  u64 count;
  u64 case_name_offset;
  sp_test_axis_t axes [SP_TEST_MAX_AXES];
  sp_test_setup_fn_t setup;
  sp_test_teardown_fn_t teardown;
  const void* user;
  bool serial;
} sp_test_decl_t;

typedef struct {
  const c8* name;
  const sp_test_decl_t* tests;
  bool serial;
} sp_test_suite_t;

typedef struct {
  const c8* suite;
  sp_test_decl_t decl;
} sp_test_reg_t;

typedef struct {
  const c8* suite;
  bool serial;
} sp_test_suite_attr_t;

typedef struct {
  const c8* name;
  const sp_test_decl_t* decl;
  const void* arg;
  bool serial;
} sp_test_instance_t;

#define __sp_test_axis_field(TYPE, FIELD)      \
    .field = #FIELD,                           \
    .offset = offsetof(TYPE, FIELD),           \
    .size = sizeof(((TYPE*)0)->FIELD)

#define sp_test_axis(TYPE, FIELD, ...)                                       \
  {                                                                          \
    .kind = SP_TEST_AXIS_VALUES,                                             \
    __sp_test_axis_field(TYPE, FIELD),                                       \
    .values = (const s64 []) { __VA_ARGS__ },                                \
    .count = sizeof((const s64 []) { __VA_ARGS__ }) / sizeof(s64),           \
  }

#define sp_test_axis_named(TYPE, FIELD, FN, ...)                                \
  {                                                                          \
    .kind = SP_TEST_AXIS_VALUES,                                             \
    __sp_test_axis_field(TYPE, FIELD),                                       \
    .values = (const s64 []) { __VA_ARGS__ },                                \
    .count = sizeof((const s64 []) { __VA_ARGS__ }) / sizeof(s64),           \
    .name = (FN),                                                            \
  }

#define sp_test_axis_range(TYPE, FIELD, FROM, TO, STEP)  \
  {                                                      \
    .kind = SP_TEST_AXIS_RANGE,                          \
    __sp_test_axis_field(TYPE, FIELD),                   \
    .from = (FROM),                                      \
    .to = (TO),                                          \
    .step = (STEP),                                      \
  }

#define __sp_test_fn(SUITE, NAME)    sp_mcat(sp_mcat(sp_test_fn_, SUITE), sp_mcat(_, NAME))
#define __sp_test_thunk(SUITE, NAME) sp_mcat(__sp_test_fn(SUITE, NAME), _thunk)

#if defined(__ELF__) || defined(SP_WASM) || (defined(SP_TCC) && defined(SP_LINUX))
  #define SP_TEST_AUTOREG 1

  #if defined(__ELF__) && defined(__has_attribute)
    #if __has_attribute(retain)
      #define SP_TEST_RETAIN __attribute__((retain))
    #endif
  #endif
  #if !defined(SP_TEST_RETAIN)
    #define SP_TEST_RETAIN
  #endif

  #define __sp_test_reg_section   __attribute__((used, section("sp_test"))) SP_TEST_RETAIN
  #define __sp_test_suite_section __attribute__((used, section("sp_test_suite"))) SP_TEST_RETAIN
#elif defined(SP_MACOS)
  #define SP_TEST_AUTOREG 1

  #define __sp_test_reg_section   __attribute__((used, section("__DATA,sp_test")))
  #define __sp_test_suite_section __attribute__((used, section("__DATA,sp_test_suite")))
#elif defined(SP_WIN32) && defined(SP_GNUC)
  #define SP_TEST_AUTOREG 1

  #define __sp_test_reg_section   __attribute__((used, section("sp_test")))
  #define __sp_test_suite_section __attribute__((used, section("sp_suite")))
#elif defined(SP_WIN32) && defined(SP_MSVC)
  #define SP_TEST_AUTOREG 1

  #pragma section("sp_test", read)
  #pragma section("sp_suite", read)
  #define __sp_test_reg_section   __declspec(allocate("sp_test"))
  #define __sp_test_suite_section __declspec(allocate("sp_suite"))
#else
  #define SP_TEST_AUTOREG 0
#endif

#if SP_TEST_AUTOREG
  #define __sp_test_reg(ID, SUITE, ...)                                       \
    static const sp_test_reg_t sp_mcat(ID, _v) = {                            \
      .suite = #SUITE,                                                        \
      .decl = __VA_ARGS__                                                     \
    };                                                                        \
    static __sp_test_reg_section const sp_test_reg_t* ID = &sp_mcat(ID, _v)

  #define sp_test_reg(SUITE, ...) __sp_test_reg(sp_mcat(sp_test_reg_, __COUNTER__), SUITE, __VA_ARGS__)

  #define __sp_test_reg_suite(ID, SUITE, ...)                                 \
    static const sp_test_suite_attr_t sp_mcat(ID, _v) = {                     \
      .suite = #SUITE,                                                        \
      __VA_ARGS__                                                             \
    };                                                                        \
    static __sp_test_suite_section const sp_test_suite_attr_t* ID = &sp_mcat(ID, _v)

  #define sp_test_suite(SUITE, ...) __sp_test_reg_suite(sp_mcat(sp_test_suite_reg_, __COUNTER__), SUITE, __VA_ARGS__)

  #define sp_test(SUITE, NAME, ...)                                         \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t);                \
    sp_test_reg(SUITE, {                                                    \
      .name = #NAME,                                                        \
      .fn = __sp_test_fn(SUITE, NAME),                                      \
      __VA_ARGS__                                                           \
    });                                                                     \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t)

  #define __sp_test_each_thunk_def(SUITE, NAME, TYPE, TARGET)                              \
    static sp_err_t __sp_test_thunk(SUITE, NAME)(sp_test_t* t, const void* sp_test_arg) {  \
      TYPE sp_test_row = *(const TYPE*)sp_test_arg;                                        \
      return TARGET(t, &sp_test_row);                                                      \
    }

  #define __sp_test_each_def(SUITE, NAME, TYPE, ARR)                                       \
    sp_static_assert(sizeof(TYPE) == sizeof((ARR)[0]), sp_test_each_row_type_mismatch);    \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it);                     \
    __sp_test_each_thunk_def(SUITE, NAME, TYPE, __sp_test_fn(SUITE, NAME))

  #define __sp_test_each_name_check(ARR)                                                 \
    sp_static_assert(                                                                    \
      sizeof((ARR)[0].name) == sizeof(const c8*) && sizeof(*(ARR)[0].name) == sizeof(c8),\
      sp_test_each_name_not_a_cstr)

  #define sp_test_each(SUITE, NAME, TYPE, ARR, ...)                         \
    __sp_test_each_def(SUITE, NAME, TYPE, ARR)                              \
    __sp_test_each_name_check(ARR);                                         \
    sp_test_reg(SUITE, {                                                    \
      .name = #NAME,                                                        \
      .each = __sp_test_thunk(SUITE, NAME),                                 \
      .cases = (ARR),                                                       \
      .stride = sizeof((ARR)[0]),                                      \
      .count = sp_carr_len(ARR),                                       \
      .case_name_offset = offsetof(TYPE, name) + 1,                    \
      __VA_ARGS__                                                           \
    });                                                                     \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)

  #define sp_test_each_anon(SUITE, NAME, TYPE, ARR, ...)                    \
    __sp_test_each_def(SUITE, NAME, TYPE, ARR)                              \
    sp_test_reg(SUITE, {                                                    \
      .name = #NAME,                                                        \
      .each = __sp_test_thunk(SUITE, NAME),                                 \
      .cases = (ARR),                                                       \
      .stride = sizeof((ARR)[0]),                                      \
      .count = sp_carr_len(ARR),                                       \
      __VA_ARGS__                                                           \
    });                                                                     \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)

  #define sp_test_each_fn(SUITE, NAME, TYPE, ARR, FN, ...)                  \
    sp_static_assert(sizeof(TYPE) == sizeof((ARR)[0]), sp_test_each_row_type_mismatch); \
    __sp_test_each_name_check(ARR);                                         \
    __sp_test_each_thunk_def(SUITE, NAME, TYPE, FN)                         \
    sp_test_reg(SUITE, {                                                    \
      .name = #NAME,                                                        \
      .each = __sp_test_thunk(SUITE, NAME),                                 \
      .cases = (ARR),                                                       \
      .stride = sizeof((ARR)[0]),                                      \
      .count = sp_carr_len(ARR),                                       \
      .case_name_offset = offsetof(TYPE, name) + 1,                    \
      __VA_ARGS__                                                           \
    })

  #define sp_test_sweep(SUITE, NAME, TYPE, ...)                             \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it);      \
    __sp_test_each_thunk_def(SUITE, NAME, TYPE, __sp_test_fn(SUITE, NAME))  \
    sp_test_reg(SUITE, {                                                    \
      .name = #NAME,                                                        \
      .each = __sp_test_thunk(SUITE, NAME),                                 \
      .stride = sizeof(TYPE),                                               \
      __VA_ARGS__                                                           \
    });                                                                     \
    static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)
#else
  #define __sp_test_unsupported() \
    sp_static_assert(false, no_sp_test_autoreg_for_this_object_format__pass_suites_to_sp_test_main)

  #define sp_test_reg(SUITE, ...)                          __sp_test_unsupported()
  #define sp_test_suite(SUITE, ...)                        __sp_test_unsupported()
  #define sp_test(SUITE, NAME, ...)                        __sp_test_unsupported(); static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t)
  #define sp_test_each(SUITE, NAME, TYPE, ARR, ...)        __sp_test_unsupported(); static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)
  #define sp_test_each_anon(SUITE, NAME, TYPE, ARR, ...)   __sp_test_unsupported(); static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)
  #define sp_test_each_fn(SUITE, NAME, TYPE, ARR, FN, ...) __sp_test_unsupported()
  #define sp_test_sweep(SUITE, NAME, TYPE, ...)            __sp_test_unsupported(); static sp_err_t __sp_test_fn(SUITE, NAME)(sp_test_t* t, TYPE* it)
#endif

SP_API s32 sp_test_main(s32 argc, const c8** argv, const sp_test_suite_t* suites);

SP_API bool sp_test_filtered(sp_glob_t* filter, const c8* name);

SP_API void sp_test_expand(sp_mem_t mem, const c8* suite, const sp_test_decl_t* decl, bool serial, sp_glob_t* filter, sp_da(sp_test_instance_t)* out);


typedef struct {
  sp_str_t key;
  sp_str_t value;
} sp_test_kv_t;

typedef struct {
  sp_str_t file;
  u32 line;
  sp_str_t message;
  sp_str_t expected;
  sp_str_t actual;
  sp_da(sp_test_kv_t) kvs;
} sp_test_failure_t;

#if defined(SP_FREESTANDING)
  #define sp_test_skip_on_freestanding() return sp_test_skip(t, "skipped on freestanding");
#else
  #define sp_test_skip_on_freestanding()
#endif

#if defined(SP_WIN32)
  #define sp_test_skip_on_win32() return sp_test_skip(t, "skipped on win32");
#else
  #define sp_test_skip_on_win32()
#endif

#if defined(SP_MACOS)
  #define sp_test_skip_on_macos() return sp_test_skip(t, "skipped on macos");
#else
  #define sp_test_skip_on_macos()
#endif

#if defined(SP_WASM)
  #define sp_test_skip_on_wasm() return sp_test_skip(t, "skipped on wasm");
#else
  #define sp_test_skip_on_wasm()
#endif

#if defined(SP_LINUX)
  #define sp_test_skip_on_linux() return sp_test_skip(t, "skipped on linux");
#else
  #define sp_test_skip_on_linux()
#endif

SP_API sp_err_t    sp_test_skip(sp_test_t* t, const c8* fmt, ...);
SP_API void        sp_test_fail(sp_test_t* t, const c8* fmt, ...);
SP_API sp_str_t    sp_test_get_name(sp_test_t* t);

SP_API void        sp_test_kv(sp_test_t* t, const c8* key, sp_str_t value);
SP_API void        sp_test_kv_c(sp_test_t* t, const c8* key, const c8* value);
SP_API void        sp_test_kv_clear(sp_test_t* t, const c8* key);

SP_API void        sp_test_set_state(sp_test_t* t, void* state);
SP_API void*       sp_test_state(sp_test_t* t);

SP_API void        sp_test_log(sp_test_t* t, const c8* fmt, ...);
SP_API void        sp_test_note(sp_test_t* t, const c8* fmt, ...);

SP_API sp_mem_t    sp_test_mem(sp_test_t* t);
SP_API sp_mem_t    sp_test_arena(sp_test_t* t);
SP_API sp_str_t    sp_test_dir(sp_test_t* t);
SP_API const void* sp_test_user(sp_test_t* t);

SP_API void        sp_test_record(sp_test_t* t, sp_test_failure_t failure);
SP_API sp_str_t    sp_test_format(sp_test_t* t, const c8* fmt, ...);
SP_API sp_str_t    sp_test_err_str(sp_test_t* t, sp_err_t err);

SP_API bool        sp_test_mem_eq(sp_test_t* t, const void* lhs, const void* rhs, u64 len, const c8* sl, const c8* sr, sp_str_t file, u32 line);
SP_API bool        sp_test_strs_eq(sp_test_t* t, const sp_str_t* actual, u64 count, const c8* const* expect, const c8* sa, const c8* se, sp_str_t file, u32 line);

SP_API void            sp_test_golden(sp_test_t* t, sp_str_t path, sp_str_t actual, sp_str_t file, u32 line);
SP_API void            sp_test_golden_abs(sp_test_t* t, sp_str_t path, sp_str_t actual, sp_str_t file, u32 line);
SP_API sp_da(sp_str_t) sp_test_resolve_roots(sp_mem_t mem, sp_str_t cwd, sp_str_t exe_dir);

typedef struct {
  sp_atomic_s32_t state;
  sp_err_t err;
} sp_test_once_t;

SP_TYPEDEF_FN(sp_err_t, sp_test_once_fn_t, void* user);
SP_API sp_err_t    sp_test_once(sp_test_once_t* once, sp_test_once_fn_t fn, void* user);


typedef enum {
  SP_TEST_ALLOC_LIVE,
  SP_TEST_ALLOC_FREED,
} sp_test_alloc_state_t;

typedef struct {
  sp_test_alloc_state_t state;
  u64 size;
  u32 id;
} sp_test_alloc_t;

typedef struct {
  sp_mem_t backing;
  sp_ht(void*, sp_test_alloc_t) allocs;
  u64 live_bytes;
  u32 live_count;
  u32 double_frees;
  u32 wild_frees;
  u32 bad_sizes;
  u32 next_id;
} sp_test_tracking_t;

SP_API void        sp_test_tracking_init(sp_test_tracking_t* k, sp_mem_t meta, sp_mem_t backing);
SP_API sp_mem_t    sp_test_tracking_as_allocator(sp_test_tracking_t* k);
SP_API void        sp_test_tracking_deinit(sp_test_tracking_t* k);


// stream: nonce frame
// frame: tag len payload
// str: len bytes
#define SP_TEST_WIRE_VERSION    1
#define SP_TEST_WIRE_NONCE_SIZE 8
#define SP_TEST_WIRE_MAX_FRAME  (1u << 24)

typedef enum {
  SP_TEST_WIRE_PLAN = 1,
  SP_TEST_WIRE_START = 2,
  SP_TEST_WIRE_FAILURE = 3,
  SP_TEST_WIRE_RESULT = 4,
  SP_TEST_WIRE_SUMMARY = 5,
} sp_test_wire_tag_t;

typedef enum {
  SP_TEST_WIRE_OK = 0,
  SP_TEST_WIRE_FAIL = 1,
  SP_TEST_WIRE_SKIP = 2,
  SP_TEST_WIRE_UPDATE = 3,
} sp_test_wire_status_t;

typedef struct {
  u32 v;
  sp_str_t arch;
  sp_str_t os;
  sp_str_t abi;
  sp_str_t* tests;
  u32 num_tests;
} sp_test_wire_plan_t;

typedef struct {
  u32 id;
} sp_test_wire_start_t;

typedef struct {
  u32 id;
  u32 line;
  sp_str_t file;
  sp_str_t message;
  sp_str_t expected;
  sp_str_t actual;
  sp_test_kv_t* kvs;
  u32 num_kvs;
} sp_test_wire_failure_t;

typedef struct {
  u32 id;
  sp_test_wire_status_t status;
  u64 dur_ns;
  sp_str_t reason;
  sp_str_t* notes;
  u32 num_notes;
  sp_str_t* logs;
  u32 num_logs;
} sp_test_wire_result_t;

typedef struct {
  u32 passed;
  u32 failed;
  u32 skipped;
  u32 updated;
  u64 dur_ns;
  sp_str_t capture;
} sp_test_wire_summary_t;

typedef struct {
  sp_test_wire_tag_t tag;
  union {
    sp_test_wire_plan_t plan;
    sp_test_wire_start_t start;
    sp_test_wire_failure_t failure;
    sp_test_wire_result_t result;
    sp_test_wire_summary_t summary;
  };
} sp_test_wire_event_t;

SP_API sp_err_t    sp_test_wire_write_nonce(sp_io_writer_t* io, const u8 nonce [SP_TEST_WIRE_NONCE_SIZE]);
SP_API sp_err_t    sp_test_wire_write(sp_io_writer_t* io, const sp_test_wire_event_t* event);
SP_API sp_err_t    sp_test_wire_scan_nonce(sp_io_reader_t* io, const u8 nonce [SP_TEST_WIRE_NONCE_SIZE]);
SP_API sp_err_t    sp_test_wire_read(sp_io_reader_t* io, sp_mem_t mem, sp_test_wire_event_t* out);


/////////////////////
// VALUE FORMATTING //
/////////////////////
SP_API sp_str_t sp_test_value_bool(sp_test_t* t, bool value);
SP_API sp_str_t sp_test_value_c8(sp_test_t* t, c8 value);
SP_API sp_str_t sp_test_value_s8(sp_test_t* t, s8 value);
SP_API sp_str_t sp_test_value_u8(sp_test_t* t, u8 value);
SP_API sp_str_t sp_test_value_s16(sp_test_t* t, s16 value);
SP_API sp_str_t sp_test_value_u16(sp_test_t* t, u16 value);
SP_API sp_str_t sp_test_value_s32(sp_test_t* t, s32 value);
SP_API sp_str_t sp_test_value_u32(sp_test_t* t, u32 value);
SP_API sp_str_t sp_test_value_s64(sp_test_t* t, s64 value);
SP_API sp_str_t sp_test_value_u64(sp_test_t* t, u64 value);
SP_API sp_str_t sp_test_value_f32(sp_test_t* t, f32 value);
SP_API sp_str_t sp_test_value_f64(sp_test_t* t, f64 value);
SP_API sp_str_t sp_test_value_cstr(sp_test_t* t, const c8* value);
SP_API sp_str_t sp_test_value_ptr(sp_test_t* t, void* value);
SP_API sp_str_t sp_test_value_str(sp_test_t* t, sp_str_t value);
SP_API sp_str_t sp_test_value_opaque(sp_test_t* t, ...);

#if defined(SP_CPP)
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, bool value) { return sp_test_value_bool(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, c8 value) { return sp_test_value_c8(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, s8 value) { return sp_test_value_s8(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, u8 value) { return sp_test_value_u8(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, s16 value) { return sp_test_value_s16(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, u16 value) { return sp_test_value_u16(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, s32 value) { return sp_test_value_s32(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, u32 value) { return sp_test_value_u32(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, s64 value) { return sp_test_value_s64(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, u64 value) { return sp_test_value_u64(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, f32 value) { return sp_test_value_f32(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, f64 value) { return sp_test_value_f64(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, const c8* value) { return sp_test_value_cstr(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, c8* value) { return sp_test_value_cstr(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, void* value) { return sp_test_value_ptr(t, value); }
  SP_INLINE sp_str_t sp_test_value(sp_test_t* t, sp_str_t value) { return sp_test_value_str(t, value); }
  template <typename T> SP_INLINE sp_str_t sp_test_value(sp_test_t* t, const T& value) { SP_UNUSED(value); return sp_test_value_opaque(t); }
#elif defined(__GNUC__) || defined(__TINYC__)
  #define sp_test_value(T, V) _Generic((V), \
    bool:        sp_test_value_bool,        \
    c8:          sp_test_value_c8,          \
    s8:          sp_test_value_s8,          \
    u8:          sp_test_value_u8,          \
    s16:         sp_test_value_s16,         \
    u16:         sp_test_value_u16,         \
    s32:         sp_test_value_s32,         \
    u32:         sp_test_value_u32,         \
    s64:         sp_test_value_s64,         \
    u64:         sp_test_value_u64,         \
    f32:         sp_test_value_f32,         \
    f64:         sp_test_value_f64,         \
    const c8*:   sp_test_value_cstr,        \
    c8*:         sp_test_value_cstr,        \
    void*:       sp_test_value_ptr,         \
    sp_str_t:    sp_test_value_str,         \
    default:     sp_test_value_opaque       \
  )(T, V)
#else
  #define sp_test_value(T, V) (SP_UNUSED(T), SP_UNUSED(V), sp_str_lit("?"))
#endif


////////////////
// ASSERTIONS //
////////////////
#if defined(SP_CPP)
  #define sp_test_auto(X) auto
#elif (defined(__clang__) || defined(__GNUC__)) && !defined(__TINYC__)
  #define sp_test_auto(X) __auto_type
#else
  #define sp_test_auto(X) __typeof__((X) + 0)
#endif

#define sp_test_cmp(T, A, B, SA, SB, OP, FAIL)                 \
  do {                                                         \
    sp_test_t* sp_test_it = (T);                               \
    sp_test_auto(A) sp_test_lhs = (A);                         \
    sp_test_auto(B) sp_test_rhs = (B);                         \
    if (!(sp_test_lhs OP sp_test_rhs)) {                       \
      sp_test_record(sp_test_it, (sp_test_failure_t) {         \
        .file = sp_cstr_as_str(__FILE__),                      \
        .line = (u32)__LINE__,                                 \
        .expected = sp_test_format(sp_test_it, "{} {} {}",     \
          sp_fmt_cstr(SA), sp_fmt_cstr(#OP), sp_fmt_cstr(SB)), \
        .actual = sp_test_format(sp_test_it, "{} vs {}",       \
          sp_fmt_str(sp_test_value(sp_test_it, sp_test_lhs)),  \
          sp_fmt_str(sp_test_value(sp_test_it, sp_test_rhs))), \
      });                                                      \
      FAIL;                                                    \
    }                                                          \
  } while (0)

#define sp_test_bool(T, X, SX, WANT, FAIL)                          \
  do {                                                              \
    sp_test_t* sp_test_it = (T);                                    \
    bool sp_test_got = !!(X);                                       \
    if (sp_test_got != (WANT)) {                                    \
      sp_test_record(sp_test_it, (sp_test_failure_t) {              \
        .file = sp_cstr_as_str(__FILE__),                           \
        .line = (u32)__LINE__,                                      \
        .expected = sp_test_format(sp_test_it, "{} is {}",          \
          sp_fmt_cstr(SX), sp_fmt_cstr((WANT) ? "true" : "false")), \
        .actual = sp_cstr_as_str(sp_test_got ? "true" : "false"),   \
      });                                                           \
      FAIL;                                                         \
    }                                                               \
  } while (0)

#define sp_test_streq(T, A, B, FAIL)                                                 \
  do {                                                                               \
    sp_test_t* sp_test_it = (T);                                                     \
    sp_str_t sp_test_lhs = (A);                                                      \
    sp_str_t sp_test_rhs = (B);                                                      \
    if (!sp_str_equal(sp_test_lhs, sp_test_rhs)) {                                   \
      sp_test_record(sp_test_it, (sp_test_failure_t) {                               \
        .file = sp_cstr_as_str(__FILE__),                                            \
        .line = (u32)__LINE__,                                                       \
        .expected = sp_test_format(sp_test_it, "{.quote}", sp_fmt_str(sp_test_rhs)), \
        .actual = sp_test_format(sp_test_it, "{.quote}", sp_fmt_str(sp_test_lhs)),   \
      });                                                                            \
      FAIL;                                                                          \
    }                                                                                \
  } while (0)

#define sp_test_arr_cmp(T, A, B, COUNT, SA, SB, FAIL)                                        \
  do {                                                                                       \
    sp_test_t* sp_test_it = (T);                                                             \
    sp_test_auto((A) + 0) sp_test_lhs = (A) + 0;                                             \
    sp_test_auto((B) + 0) sp_test_rhs = (B) + 0;                                             \
    u64 sp_test_count = (u64)(COUNT);                                                        \
    u64 sp_test_head = sp_test_count;                                                        \
    u64 sp_test_diffs = 0;                                                                   \
    for (u64 sp_test_index = 0; sp_test_index < sp_test_count; sp_test_index++) {            \
      if (sp_test_lhs[sp_test_index] == sp_test_rhs[sp_test_index]) continue;                \
      if (sp_test_head == sp_test_count) sp_test_head = sp_test_index;                       \
      sp_test_diffs++;                                                                       \
    }                                                                                        \
    if (sp_test_diffs) {                                                                     \
      sp_test_record(sp_test_it, (sp_test_failure_t) {                                       \
        .file = sp_cstr_as_str(__FILE__),                                                    \
        .line = (u32)__LINE__,                                                               \
        .message = sp_test_format(sp_test_it, "{} and {} differ at [{}], {} of {} elements", \
          sp_fmt_cstr(SA), sp_fmt_cstr(SB), sp_fmt_uint(sp_test_head),                       \
          sp_fmt_uint(sp_test_diffs), sp_fmt_uint(sp_test_count)),                           \
        .actual = sp_test_format(sp_test_it, "{} vs {}",                                     \
          sp_fmt_str(sp_test_value(sp_test_it, sp_test_lhs[sp_test_head])),                  \
          sp_fmt_str(sp_test_value(sp_test_it, sp_test_rhs[sp_test_head]))),                 \
      });                                                                                    \
      FAIL;                                                                                  \
    }                                                                                        \
  } while (0)

#define sp_test_err_ok(T, ERR, SERR, FAIL)                                        \
  do {                                                                            \
    sp_test_t* sp_test_it = (T);                                                  \
    sp_err_t sp_test_err = (ERR);                                                 \
    if (sp_test_err != SP_OK) {                                                   \
      sp_test_record(sp_test_it, (sp_test_failure_t) {                            \
        .file = sp_cstr_as_str(__FILE__),                                         \
        .line = (u32)__LINE__,                                                    \
        .expected = sp_test_format(sp_test_it, "{} is SP_OK", sp_fmt_cstr(SERR)), \
        .actual = sp_test_err_str(sp_test_it, sp_test_err),                       \
      });                                                                         \
      FAIL;                                                                       \
    }                                                                             \
  } while (0)

#define sp_test_err_eq(T, A, B, SA, SB, FAIL)                    \
  do {                                                           \
    sp_test_t* sp_test_it = (T);                                 \
    sp_err_t sp_test_lhs = (A);                                  \
    sp_err_t sp_test_rhs = (B);                                  \
    if (sp_test_lhs != sp_test_rhs) {                            \
      sp_test_record(sp_test_it, (sp_test_failure_t) {           \
        .file = sp_cstr_as_str(__FILE__),                        \
        .line = (u32)__LINE__,                                   \
        .expected = sp_test_format(sp_test_it, "{} == {}",       \
          sp_fmt_cstr(SA), sp_fmt_cstr(SB)),                     \
        .actual = sp_test_format(sp_test_it, "{} vs {}",         \
          sp_fmt_str(sp_test_err_str(sp_test_it, sp_test_lhs)),  \
          sp_fmt_str(sp_test_err_str(sp_test_it, sp_test_rhs))), \
      });                                                        \
      FAIL;                                                      \
    }                                                            \
  } while (0)

#define sp_test_soft                ((void)0)
#define sp_test_stop                return SP_ERR

#define sp_expect(T, COND)          sp_test_bool(T, COND, #COND, true, sp_test_soft)
#define sp_must(T, COND)            sp_test_bool(T, COND, #COND, true, sp_test_stop)

#define sp_expect_eq(T, A, B)       sp_test_cmp(T, A, B, #A, #B, ==, sp_test_soft)
#define sp_must_eq(T, A, B)         sp_test_cmp(T, A, B, #A, #B, ==, sp_test_stop)
#define sp_expect_ne(T, A, B)       sp_test_cmp(T, A, B, #A, #B, !=, sp_test_soft)
#define sp_must_ne(T, A, B)         sp_test_cmp(T, A, B, #A, #B, !=, sp_test_stop)
#define sp_expect_lt(T, A, B)       sp_test_cmp(T, A, B, #A, #B, <,  sp_test_soft)
#define sp_must_lt(T, A, B)         sp_test_cmp(T, A, B, #A, #B, <,  sp_test_stop)
#define sp_expect_le(T, A, B)       sp_test_cmp(T, A, B, #A, #B, <=, sp_test_soft)
#define sp_must_le(T, A, B)         sp_test_cmp(T, A, B, #A, #B, <=, sp_test_stop)
#define sp_expect_gt(T, A, B)       sp_test_cmp(T, A, B, #A, #B, >,  sp_test_soft)
#define sp_must_gt(T, A, B)         sp_test_cmp(T, A, B, #A, #B, >,  sp_test_stop)
#define sp_expect_ge(T, A, B)       sp_test_cmp(T, A, B, #A, #B, >=, sp_test_soft)
#define sp_must_ge(T, A, B)         sp_test_cmp(T, A, B, #A, #B, >=, sp_test_stop)

#define sp_expect_str_eq(T, A, B)   sp_test_streq(T, A, B, sp_test_soft)
#define sp_must_str_eq(T, A, B)     sp_test_streq(T, A, B, sp_test_stop)
#define sp_expect_str_eq_c(T, A, B) sp_test_streq(T, A, sp_cstr_as_str(B), sp_test_soft)
#define sp_must_str_eq_c(T, A, B)   sp_test_streq(T, A, sp_cstr_as_str(B), sp_test_stop)

#define sp_expect_arr_eq(T, A, B, COUNT) sp_test_arr_cmp(T, A, B, COUNT, #A, #B, sp_test_soft)
#define sp_must_arr_eq(T, A, B, COUNT)   sp_test_arr_cmp(T, A, B, COUNT, #A, #B, sp_test_stop)

#define sp_expect_mem_eq(T, A, B, LEN)   ((void)sp_test_mem_eq(T, A, B, LEN, #A, #B, sp_cstr_as_str(__FILE__), (u32)__LINE__))
#define sp_must_mem_eq(T, A, B, LEN)     do { if (!sp_test_mem_eq(T, A, B, LEN, #A, #B, sp_cstr_as_str(__FILE__), (u32)__LINE__)) sp_test_stop; } while (0)

#define sp_expect_strs_eq(T, A, COUNT, B) ((void)sp_test_strs_eq(T, A, COUNT, B, #A, #B, sp_cstr_as_str(__FILE__), (u32)__LINE__))
#define sp_must_strs_eq(T, A, COUNT, B)   do { if (!sp_test_strs_eq(T, A, COUNT, B, #A, #B, sp_cstr_as_str(__FILE__), (u32)__LINE__)) sp_test_stop; } while (0)

#define sp_expect_ok(T, ERR)        sp_test_err_ok(T, ERR, #ERR, sp_test_soft)
#define sp_must_ok(T, ERR)          sp_test_err_ok(T, ERR, #ERR, return sp_test_err)

#define sp_expect_err_eq(T, A, B)   sp_test_err_eq(T, A, B, #A, #B, sp_test_soft)
#define sp_must_err_eq(T, A, B)     sp_test_err_eq(T, A, B, #A, #B, sp_test_stop)

#define sp_expect_golden(T, PATH, ACTUAL)     sp_test_golden(T, PATH, ACTUAL, sp_cstr_as_str(__FILE__), (u32)__LINE__)
#define sp_expect_golden_abs(T, PATH, ACTUAL) sp_test_golden_abs(T, PATH, ACTUAL, sp_cstr_as_str(__FILE__), (u32)__LINE__)

#endif

////////////////////
// IMPLEMENTATION //
////////////////////
#if defined(SP_IMPLEMENTATION) && !defined(SP_TEST_IMPLEMENTATION)
  #define SP_TEST_IMPLEMENTATION
#endif

#if defined(SP_TEST_IMPLEMENTATION) && !defined(SP_TEST_H_IMPL)
#define SP_TEST_H_IMPL

#include "sp_cli.h"

typedef struct sp_test_runner_t sp_test_runner_t;

struct sp_test_t {
  const c8* name;
  const sp_test_decl_t* decl;
  const void* arg;
  const void* user;
  void* state;
  sp_test_runner_t* runner;

  sp_mem_arena_t* bookkeeping;
  sp_mem_t mem;

  sp_da(sp_test_failure_t) failures;
  sp_da(sp_test_kv_t) kvs;
  sp_da(sp_str_t) logs;
  sp_da(sp_str_t) notes;

  sp_str_t skip_reason;
  bool skipped;

  u32 updated;

  sp_mem_arena_t* scratch;
  sp_mem_t scratch_mem;

  sp_test_tracking_t tracking;
  sp_mem_heap_t* tracking_heap;
  sp_mem_t tracked_mem;
  bool tracking_live;

  sp_str_t dir;
};

struct sp_test_runner_t {
  sp_mem_t mem;
  sp_io_stream_writer_t out;
  u8 out_buffer [8192];
  sp_mutex_t mutex;
  sp_da(sp_test_instance_t) queue;
  sp_atomic_s32_t cursor;
  sp_da(const c8*) failed;
  sp_da(const c8*) skipped;
  sp_da(const c8*) updated;
  sp_str_t dir_root;
  sp_str_t golden_root;
  bool update;
  bool color;
};


static void* sp_test_tracking_do_alloc(sp_test_tracking_t* k, u64 size) {
  if (!size) return SP_NULLPTR;
  if (!k->backing.on_alloc) return SP_NULLPTR;

  void* ptr = sp_alloc(k->backing, size);
  if (!ptr) return SP_NULLPTR;

  sp_ht_insert(k->allocs, ptr, ((sp_test_alloc_t) {
    .state = SP_TEST_ALLOC_LIVE,
    .size = size,
    .id = ++k->next_id,
  }));
  k->live_count++;
  k->live_bytes += size;
  return ptr;
}

static void sp_test_tracking_do_free(sp_test_tracking_t* k, void* ptr, u64 size) {
  if (!ptr) return;

  sp_test_alloc_t* alloc = sp_ht_getp(k->allocs, ptr);
  if (!alloc) {
    k->wild_frees++;
    return;
  }
  if (alloc->state == SP_TEST_ALLOC_FREED) {
    k->double_frees++;
    return;
  }

  if (alloc->size != size) k->bad_sizes++;
  alloc->state = SP_TEST_ALLOC_FREED;
  k->live_count--;
  k->live_bytes -= alloc->size;
  sp_free(k->backing, ptr, alloc->size);
}

static void* sp_test_tracking_do_realloc(sp_test_tracking_t* k, void* old, u64 size, u64 old_size) {
  if (!old)  return sp_test_tracking_do_alloc(k, size);
  if (!size) {
    sp_test_tracking_do_free(k, old, old_size);
    return SP_NULLPTR;
  }

  sp_test_alloc_t* alloc = sp_ht_getp(k->allocs, old);
  if (!alloc) {
    k->wild_frees++;
    return SP_NULLPTR;
  }
  if (alloc->state == SP_TEST_ALLOC_FREED) {
    k->double_frees++;
    return SP_NULLPTR;
  }

  if (alloc->size != old_size) k->bad_sizes++;
  if (alloc->size == size) return old;

  u64 have = alloc->size;
  void* fresh = sp_test_tracking_do_alloc(k, size);
  if (!fresh) return SP_NULLPTR;
  sp_mem_copy(fresh, old, sp_min(have, size));
  sp_test_tracking_do_free(k, old, have);
  return fresh;
}

static void* sp_test_tracking_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u64 size, void* ptr, u64 old_size) {
  sp_test_tracking_t* k = (sp_test_tracking_t*)ud;
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC:
    case SP_ALLOCATOR_MODE_ALLOC_UNINITIALIZED:  return sp_test_tracking_do_alloc(k, size);
    case SP_ALLOCATOR_MODE_RESIZE:
    case SP_ALLOCATOR_MODE_RESIZE_UNINITIALIZED: return sp_test_tracking_do_realloc(k, ptr, size, old_size);
    case SP_ALLOCATOR_MODE_FREE:       sp_test_tracking_do_free(k, ptr, old_size); return SP_NULLPTR;
  }
  return SP_NULLPTR;
}

void sp_test_tracking_init(sp_test_tracking_t* k, sp_mem_t meta, sp_mem_t backing) {
  sp_mem_zero(k, sizeof(*k));
  k->backing = backing;
  sp_ht_init(meta, k->allocs);
}

sp_mem_t sp_test_tracking_as_allocator(sp_test_tracking_t* k) {
  return (sp_mem_t) {
    .on_alloc = sp_test_tracking_on_alloc,
    .user_data = k,
  };
}

void sp_test_tracking_deinit(sp_test_tracking_t* k) {
  sp_ht_for_kv(k->allocs, it) {
    if (it.val->state == SP_TEST_ALLOC_LIVE) sp_free(k->backing, *it.key, it.val->size);
  }
  sp_ht_free(k->allocs);
  sp_mem_zero(k, sizeof(*k));
}


///////////////
// FORMATTING //
///////////////
sp_str_t sp_test_format(sp_test_t* t, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_r result = sp_fmt_mem_v(t->mem, sp_cstr_as_str(fmt), args);
  va_end(args);
  return result.value;
}

sp_str_t sp_test_err_str(sp_test_t* t, sp_err_t err) {
  sp_str_t name = sp_rt.err_str(err);
  if (!sp_str_empty(name)) return name;
  return sp_test_format(t, "err {}", sp_fmt_int(err));
}

sp_str_t sp_test_value_bool(sp_test_t* t, bool value) { return sp_test_format(t, "{}", sp_fmt_cstr(value ? "true" : "false")); }
sp_str_t sp_test_value_c8(sp_test_t* t, c8 value) { return sp_test_format(t, "{}", sp_fmt_char(value)); }
sp_str_t sp_test_value_s8(sp_test_t* t, s8 value) { return sp_test_format(t, "{}", sp_fmt_int(value)); }
sp_str_t sp_test_value_u8(sp_test_t* t, u8 value) { return sp_test_format(t, "{}", sp_fmt_uint(value)); }
sp_str_t sp_test_value_s16(sp_test_t* t, s16 value) { return sp_test_format(t, "{}", sp_fmt_int(value)); }
sp_str_t sp_test_value_u16(sp_test_t* t, u16 value) { return sp_test_format(t, "{}", sp_fmt_uint(value)); }
sp_str_t sp_test_value_s32(sp_test_t* t, s32 value) { return sp_test_format(t, "{}", sp_fmt_int(value)); }
sp_str_t sp_test_value_u32(sp_test_t* t, u32 value) { return sp_test_format(t, "{}", sp_fmt_uint(value)); }
sp_str_t sp_test_value_s64(sp_test_t* t, s64 value) { return sp_test_format(t, "{}", sp_fmt_int(value)); }
sp_str_t sp_test_value_u64(sp_test_t* t, u64 value) { return sp_test_format(t, "{}", sp_fmt_uint(value)); }
sp_str_t sp_test_value_f32(sp_test_t* t, f32 value) { return sp_test_format(t, "{:.6}", sp_fmt_float(value)); }
sp_str_t sp_test_value_f64(sp_test_t* t, f64 value) { return sp_test_format(t, "{:.6}", sp_fmt_float(value)); }
sp_str_t sp_test_value_cstr(sp_test_t* t, const c8* value) { return value ? sp_test_format(t, "{.quote}", sp_fmt_cstr(value)) : sp_str_lit("(null)"); }
sp_str_t sp_test_value_ptr(sp_test_t* t, void* value) { return sp_test_format(t, "{}", sp_fmt_ptr(value)); }
sp_str_t sp_test_value_str(sp_test_t* t, sp_str_t value) { return sp_test_format(t, "{.quote}", sp_fmt_str(value)); }

sp_str_t sp_test_value_opaque(sp_test_t* t, ...) {
  SP_UNUSED(t);
  return sp_str_lit("?");
}


//////////////////
// CONTEXT CORE //
//////////////////
sp_str_t sp_test_get_name(sp_test_t* t) {
  return sp_cstr_as_str(t->name);
}

const void* sp_test_user(sp_test_t* t) {
  return t->user;
}

static sp_da(sp_test_kv_t) sp_test_kv_snapshot(sp_test_t* t) {
  sp_da(sp_test_kv_t) copy = sp_da_new(t->mem, sp_test_kv_t);
  sp_da_for(t->kvs, it) {
    sp_da_push(copy, t->kvs[it]);
  }
  return copy;
}

void sp_test_record(sp_test_t* t, sp_test_failure_t failure) {
  sp_str_t here = sp_str_lit("./");
  if (sp_str_starts_with(failure.file, here)) {
    failure.file = sp_str_sub(failure.file, (s32)here.len, (s32)(failure.file.len - here.len));
  }

  failure.kvs = sp_test_kv_snapshot(t);
  sp_da_push(t->failures, failure);
}

void sp_test_fail(sp_test_t* t, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_r message = sp_fmt_mem_v(t->mem, sp_cstr_as_str(fmt), args);
  va_end(args);

  sp_test_record(t, (sp_test_failure_t) { .message = message.value });
}

sp_err_t sp_test_skip(sp_test_t* t, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_r reason = sp_fmt_mem_v(t->mem, sp_cstr_as_str(fmt), args);
  va_end(args);

  t->skip_reason = reason.value;
  t->skipped = true;
  return SP_ERR;
}

void sp_test_kv(sp_test_t* t, const c8* key, sp_str_t value) {
  sp_str_t owned_key = sp_str_copy(t->mem, sp_cstr_as_str(key));
  sp_str_t owned_value = sp_str_copy(t->mem, value);

  sp_da_for(t->kvs, it) {
    if (sp_str_equal(t->kvs[it].key, owned_key)) {
      t->kvs[it].value = owned_value;
      return;
    }
  }

  sp_da_push(t->kvs, ((sp_test_kv_t) {
    .key = owned_key,
    .value = owned_value,
  }));
}

void sp_test_kv_c(sp_test_t* t, const c8* key, const c8* value) {
  sp_test_kv(t, key, sp_cstr_as_str(value));
}

void sp_test_kv_clear(sp_test_t* t, const c8* key) {
  if (!key) {
    sp_da_clear(t->kvs);
    return;
  }

  sp_str_t target = sp_cstr_as_str(key);
  sp_da(sp_test_kv_t) kept = sp_da_new(t->mem, sp_test_kv_t);
  sp_da_for(t->kvs, it) {
    if (sp_str_equal(t->kvs[it].key, target)) continue;
    sp_da_push(kept, t->kvs[it]);
  }
  t->kvs = kept;
}

void sp_test_set_state(sp_test_t* t, void* state) {
  t->state = state;
}

void* sp_test_state(sp_test_t* t) {
  return t->state;
}

void sp_test_log(sp_test_t* t, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_r line = sp_fmt_mem_v(t->mem, sp_cstr_as_str(fmt), args);
  va_end(args);
  sp_da_push(t->logs, line.value);
}

void sp_test_note(sp_test_t* t, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_r line = sp_fmt_mem_v(t->mem, sp_cstr_as_str(fmt), args);
  va_end(args);
  sp_da_push(t->notes, line.value);
}


//////////////
// SERVICES //
//////////////
sp_mem_t sp_test_arena(sp_test_t* t) {
  if (!t->scratch) {
    t->scratch = sp_mem_arena_new(sp_mem_os_new());
    t->scratch_mem = sp_mem_arena_as_allocator(t->scratch);
  }
  return t->scratch_mem;
}

sp_mem_t sp_test_mem(sp_test_t* t) {
  if (!t->tracking_live) {
    t->tracking_heap = sp_mem_heap_new();
    sp_test_tracking_init(&t->tracking, t->mem, sp_mem_heap_as_allocator(t->tracking_heap));
    t->tracked_mem = sp_test_tracking_as_allocator(&t->tracking);
    t->tracking_live = true;
  }
  return t->tracked_mem;
}

sp_str_t sp_test_dir(sp_test_t* t) {
  if (sp_str_empty(t->dir)) {
    sp_str_t leaf = sp_str_replace_c8(t->mem, sp_cstr_as_str(t->name), '/', '_');
    t->dir = sp_fs_join_path(t->mem, t->runner->dir_root, leaf);
    if (sp_fs_exists(t->dir)) sp_fs_remove_dir(t->dir);
    sp_fs_create_dir(t->dir);
  }
  return t->dir;
}

sp_err_t sp_test_once(sp_test_once_t* once, sp_test_once_fn_t fn, void* user) {
  if (sp_atomic_s32_cas(&once->state, 0, 1, SP_ATOMIC_ACQUIRE)) {
    once->err = fn(user);
    sp_atomic_s32_store(&once->state, 2, SP_ATOMIC_RELEASE);
  }
  else {
    while (sp_atomic_s32_load(&once->state, SP_ATOMIC_ACQUIRE) != 2) {
      sp_os_sleep_ms(1);
    }
  }
  return once->err;
}


////////////
// GOLDEN //
////////////
sp_da(sp_str_t) sp_test_resolve_roots(sp_mem_t mem, sp_str_t cwd, sp_str_t exe_dir) {
  sp_da(sp_str_t) roots = sp_da_new(mem, sp_str_t);

  sp_str_t anchors [] = { cwd, exe_dir };
  sp_carr_for(anchors, at) {
    if (sp_str_empty(anchors[at])) continue;

    sp_str_t dir = sp_fs_normalize_path(mem, anchors[at]);
    while (!sp_str_empty(dir)) {
      bool seen = false;
      sp_da_for(roots, it) {
        if (sp_str_equal(roots[it], dir)) { seen = true; break; }
      }
      if (!seen) sp_da_push(roots, dir);

      sp_str_t parent = sp_fs_parent_path(dir);
      if (sp_str_equal(parent, dir)) break;
      dir = parent;
    }
  }

  return roots;
}

static sp_str_t sp_test_golden_root(sp_test_t* t, sp_str_t file) {
  sp_test_runner_t* runner = t->runner;

  sp_mutex_lock(&runner->mutex);
  sp_str_t root = runner->golden_root;
  sp_mutex_unlock(&runner->mutex);
  if (!sp_str_empty(root)) return root;

  sp_str_t cwd = sp_fs_get_cwd(t->mem);
  sp_str_t exe_dir = sp_fs_parent_path(sp_fs_get_exe_path(t->mem));
  sp_da(sp_str_t) roots = sp_test_resolve_roots(t->mem, cwd, exe_dir);

  sp_da_for(roots, it) {
    if (!sp_fs_is_target_file(sp_fs_join_path(t->mem, roots[it], file))) continue;
    root = roots[it];
    break;
  }
  if (sp_str_empty(root)) return root;

  sp_mutex_lock(&runner->mutex);
  if (sp_str_empty(runner->golden_root)) {
    runner->golden_root = sp_str_copy(runner->mem, root);
  }
  root = runner->golden_root;
  sp_mutex_unlock(&runner->mutex);
  return root;
}

static void sp_test_golden_at(sp_test_t* t, sp_str_t path, sp_str_t actual, sp_str_t file, u32 line) {
  sp_str_t actual_path = sp_test_format(t, "{}.actual", sp_fmt_str(path));

  if (t->runner->update) {
    sp_str_t parent = sp_fs_parent_path(path);
    if (!sp_str_empty(parent) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    sp_err_t err = sp_fs_create_file_str(path, actual);
    if (err) {
      sp_test_record(t, (sp_test_failure_t) {
        .file = file,
        .line = line,
        .message = sp_test_format(t, "failed to update golden {}: err {}",
          sp_fmt_str(path), sp_fmt_int(err)),
      });
      return;
    }

    if (sp_fs_exists(actual_path)) sp_fs_remove_file(actual_path);
    t->updated++;
    return;
  }

  if (!sp_fs_exists(path)) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "golden {} does not exist; run with --update to create it",
        sp_fmt_str(path)),
    });
    return;
  }

  sp_str_t want = sp_zero;
  sp_err_t err = sp_io_read_file(t->mem, path, &want);
  if (err) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "failed to read golden {}: err {}",
        sp_fmt_str(path), sp_fmt_int(err)),
    });
    return;
  }

  if (sp_str_equal(want, actual)) {
    if (sp_fs_exists(actual_path)) sp_fs_remove_file(actual_path);
    return;
  }

  sp_fs_create_file_str(actual_path, actual);

  sp_da(sp_str_t) want_lines = sp_str_split_c8(t->mem, want, '\n');
  sp_da(sp_str_t) got_lines = sp_str_split_c8(t->mem, actual, '\n');
  u64 shared = sp_min(sp_da_size(want_lines), sp_da_size(got_lines));

  u64 diff = 0;
  while (diff < shared && sp_str_equal(want_lines[diff], got_lines[diff])) {
    diff++;
  }

  sp_str_t want_line = diff < sp_da_size(want_lines) ? want_lines[diff] : sp_str_lit("<end of file>");
  sp_str_t got_line  = diff < sp_da_size(got_lines)  ? got_lines[diff]  : sp_str_lit("<end of file>");

  sp_test_record(t, (sp_test_failure_t) {
    .file = file,
    .line = line,
    .message = sp_test_format(t, "golden mismatch at {}:{}; wrote {}",
      sp_fmt_str(path), sp_fmt_uint(diff + 1), sp_fmt_str(actual_path)),
    .expected = sp_test_format(t, "{.quote}", sp_fmt_str(want_line)),
    .actual = sp_test_format(t, "{.quote}", sp_fmt_str(got_line)),
  });
}

void sp_test_golden(sp_test_t* t, sp_str_t path, sp_str_t actual, sp_str_t file, u32 line) {
  if (sp_fs_is_absolute_for(path, SP_FS_PATH_WINDOWS)) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "golden path {} is absolute; paths resolve against the calling source dir (use sp_expect_golden_abs)",
        sp_fmt_str(path)),
    });
    return;
  }

  file = sp_fs_normalize_path(t->mem, file);

  sp_str_t src = file;
  if (!sp_fs_is_absolute_for(file, SP_FS_PATH_WINDOWS)) {
    sp_str_t root = sp_test_golden_root(t, file);
    if (sp_str_empty(root)) {
      sp_test_record(t, (sp_test_failure_t) {
        .file = file,
        .line = line,
        .message = sp_test_format(t, "cannot locate {} from the cwd or the test binary; pass --golden-root",
          sp_fmt_str(file)),
      });
      return;
    }
    src = sp_fs_join_path(t->mem, root, file);
  }

  if (!sp_fs_is_target_file(src)) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "cannot locate {}; goldens resolve against the calling source dir (--golden-root overrides)",
        sp_fmt_str(src)),
    });
    return;
  }

  sp_test_golden_at(t, sp_fs_join_path(t->mem, sp_fs_parent_path(src), path), actual, file, line);
}

void sp_test_golden_abs(sp_test_t* t, sp_str_t path, sp_str_t actual, sp_str_t file, u32 line) {
  sp_test_golden_at(t, path, actual, file, line);
}


/////////////
// BUFFERS //
/////////////
static sp_str_t sp_test_mem_eq_window(sp_test_t* t, const u8* bytes, u64 len, u64 mark) {
  const c8* digits = "0123456789abcdef";
  u64 begin = mark >= 8 ? mark - 8 : 0;
  u64 end = sp_min(len, mark + 8);

  c8 buf [96];
  u32 at = 0;
  if (begin > 0) { buf[at++] = '.'; buf[at++] = '.'; buf[at++] = ' '; }
  for (u64 it = begin; it < end; it++) {
    if (it > begin) buf[at++] = ' ';
    if (it == mark) buf[at++] = '[';
    buf[at++] = digits[bytes[it] >> 4];
    buf[at++] = digits[bytes[it] & 15];
    if (it == mark) buf[at++] = ']';
  }
  if (end < len) { buf[at++] = ' '; buf[at++] = '.'; buf[at++] = '.'; }
  return sp_str_copy(t->mem, sp_str(buf, at));
}

bool sp_test_mem_eq(sp_test_t* t, const void* lhs, const void* rhs, u64 len, const c8* sl, const c8* sr, sp_str_t file, u32 line) {
  const u8* a = (const u8*)lhs;
  const u8* b = (const u8*)rhs;

  if (len && (!a || !b)) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "{} is null", sp_fmt_cstr(a ? sr : sl)),
    });
    return false;
  }

  u64 first = len;
  u64 diffs = 0;
  for (u64 it = 0; it < len; it++) {
    if (a[it] == b[it]) continue;
    if (first == len) first = it;
    diffs++;
  }
  if (!diffs) return true;

  sp_str_t message = sp_test_format(t, "{} and {} differ at byte {} of {}",
    sp_fmt_cstr(sl), sp_fmt_cstr(sr), sp_fmt_uint(first), sp_fmt_uint(len));
  if (diffs > 1) {
    message = sp_test_format(t, "{}, {} bytes differ", sp_fmt_str(message), sp_fmt_uint(diffs));
  }

  sp_test_record(t, (sp_test_failure_t) {
    .file = file,
    .line = line,
    .message = message,
    .expected = sp_test_mem_eq_window(t, b, len, first),
    .actual = sp_test_mem_eq_window(t, a, len, first),
  });
  return false;
}

bool sp_test_strs_eq(sp_test_t* t, const sp_str_t* actual, u64 count, const c8* const* expect, const c8* sa, const c8* se, sp_str_t file, u32 line) {
  if (count && !actual) {
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "{} is null", sp_fmt_cstr(sa)),
    });
    return false;
  }

  u64 want = 0;
  while (expect[want]) want++;

  if (count != want) {
    sp_str_t message = sp_test_format(t, "{} has {} strings, {} has {}",
      sp_fmt_cstr(sa), sp_fmt_uint(count), sp_fmt_cstr(se), sp_fmt_uint(want));
    if (count > want) {
      message = sp_test_format(t, "{}\nfirst extra: {.quote}", sp_fmt_str(message), sp_fmt_str(actual[want]));
    }
    else {
      message = sp_test_format(t, "{}\nfirst missing: {.quote}", sp_fmt_str(message), sp_fmt_cstr(expect[count]));
    }
    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = message,
    });
    return false;
  }

  for (u64 it = 0; it < want; it++) {
    sp_str_t rhs = sp_cstr_as_str(expect[it]);
    if (sp_str_equal(actual[it], rhs)) continue;

    sp_test_record(t, (sp_test_failure_t) {
      .file = file,
      .line = line,
      .message = sp_test_format(t, "{} and {} differ at [{}]",
        sp_fmt_cstr(sa), sp_fmt_cstr(se), sp_fmt_uint(it)),
      .expected = sp_test_format(t, "{.quote}", sp_fmt_str(rhs)),
      .actual = sp_test_format(t, "{.quote}", sp_fmt_str(actual[it])),
    });
    return false;
  }
  return true;
}


sp_err_t sp_test_wire_write_nonce(sp_io_writer_t* io, const u8 nonce [SP_TEST_WIRE_NONCE_SIZE]) {
  SP_UNUSED(io);
  SP_UNUSED(nonce);
  return SP_ERR;
}

sp_err_t sp_test_wire_write(sp_io_writer_t* io, const sp_test_wire_event_t* event) {
  SP_UNUSED(io);
  SP_UNUSED(event);
  return SP_ERR;
}

sp_err_t sp_test_wire_scan_nonce(sp_io_reader_t* io, const u8 nonce [SP_TEST_WIRE_NONCE_SIZE]) {
  SP_UNUSED(io);
  SP_UNUSED(nonce);
  return SP_ERR;
}

sp_err_t sp_test_wire_read(sp_io_reader_t* io, sp_mem_t mem, sp_test_wire_event_t* out) {
  SP_UNUSED(io);
  SP_UNUSED(mem);
  SP_UNUSED(out);
  return SP_ERR;
}


static sp_fmt_argv_t sp_test_style(bool color, sp_fmt_style_t style) {
  return sp_fmt_style(color ? style : sp_fmt_style_none);
}

static sp_str_t sp_test_duration(sp_mem_t mem, u64 ns) {
  const c8* const units [] = { "ns", "us", "ms", "s" };
  u32 unit = 0;
  u64 time = ns;
  while (unit < sp_carr_len(units) - 1 && time >= 10000) {
    time /= 1000;
    unit++;
  }
  return sp_fmt(mem, "{}{}", sp_fmt_uint(time), sp_fmt_cstr(units[unit])).value;
}

static sp_str_t sp_test_report_bar(sp_mem_t mem, bool color) {
  return sp_fmt(mem, "{.$}", sp_test_style(color, sp_fmt_style_red), sp_fmt_cstr("▐ ")).value;
}

static void sp_test_report_attr(sp_io_writer_t* io, sp_mem_t mem, bool color, sp_str_t bar, sp_str_t key, sp_str_t value, u32 width) {
  value = sp_str_trim_right(value);
  sp_da(sp_str_t) lines = sp_str_split_c8(mem, value, '\n');

  if (sp_str_empty(key)) {
    sp_da_for(lines, it) {
      sp_fmt_io(io, "  {}{}\n", sp_fmt_str(bar), sp_fmt_str(lines[it]));
    }
    return;
  }

  if (sp_da_size(lines) <= 1) {
    sp_fmt_io(io, "  {}{.$} {}\n",
      sp_fmt_str(bar),
      sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(sp_str_pad(mem, key, width)),
      sp_fmt_str(value));
    return;
  }

  sp_fmt_io(io, "  {}{.$}\n", sp_fmt_str(bar), sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(key));
  sp_da_for(lines, it) {
    sp_fmt_io(io, "  {}  {}\n", sp_fmt_str(bar), sp_fmt_str(lines[it]));
  }
}

static void sp_test_report_failure(sp_io_writer_t* io, sp_mem_t mem, bool color, sp_test_failure_t* failure) {
  sp_str_t bar = sp_test_report_bar(mem, color);

  u32 width = (u32)sizeof("expected") - 1;
  sp_da_for(failure->kvs, it) {
    width = sp_max(width, failure->kvs[it].key.len);
  }

  if (!sp_str_empty(failure->file)) {
    sp_fmt_io(io, "  {}{.$}:{.$}\n",
      sp_fmt_str(bar),
      sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(failure->file),
      sp_test_style(color, sp_fmt_style_gray), sp_fmt_uint(failure->line));
  }
  if (!sp_str_empty(failure->message)) {
    sp_test_report_attr(io, mem, color, bar, sp_zero_s(sp_str_t), failure->message, width);
  }
  if (!sp_str_empty(failure->expected)) {
    sp_test_report_attr(io, mem, color, bar, sp_str_lit("expected"), failure->expected, width);
  }
  if (!sp_str_empty(failure->actual)) {
    sp_test_report_attr(io, mem, color, bar, sp_str_lit("actual"), failure->actual, width);
  }
  sp_da_for(failure->kvs, it) {
    sp_test_report_attr(io, mem, color, bar, failure->kvs[it].key, failure->kvs[it].value, width);
  }
}

static void sp_test_report_log(sp_io_writer_t* io, sp_mem_t mem, bool color, sp_test_t* t) {
  if (sp_da_empty(t->logs)) return;

  sp_str_t bar = sp_test_report_bar(mem, color);
  sp_fmt_io(io, "  {}{.$}\n", sp_fmt_str(bar), sp_test_style(color, sp_fmt_style_gray), sp_fmt_cstr("log"));
  sp_da_for(t->logs, it) {
    sp_fmt_io(io, "  {}  {}\n", sp_fmt_str(bar), sp_fmt_str(t->logs[it]));
  }
}

static sp_test_wire_status_t sp_test_status(sp_test_t* t) {
  if (!sp_da_empty(t->failures)) return SP_TEST_WIRE_FAIL;
  if (t->updated)                return SP_TEST_WIRE_UPDATE;
  if (t->skipped)                return SP_TEST_WIRE_SKIP;
  return SP_TEST_WIRE_OK;
}

static void sp_test_report(sp_test_t* t, sp_io_writer_t* io, sp_test_wire_status_t status, u64 ns) {
  bool color = t->runner->color;
  sp_str_t duration = sp_test_duration(t->mem, ns);

  switch (status) {
    case SP_TEST_WIRE_FAIL: {
      sp_fmt_io(io, "{} {.$} {.$}\n",
        sp_fmt_cstr(t->name),
        sp_test_style(color, sp_fmt_style_red), sp_fmt_cstr("failed"),
        sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(duration));
      break;
    }
    case SP_TEST_WIRE_UPDATE: {
      sp_fmt_io(io, "{} {.$} {.$}\n",
        sp_fmt_cstr(t->name),
        sp_test_style(color, sp_fmt_style_cyan), sp_fmt_cstr("updated"),
        sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(duration));
      break;
    }
    case SP_TEST_WIRE_SKIP: {
      sp_fmt_io(io, "{} {.$} {.$} {.$}\n",
        sp_fmt_cstr(t->name),
        sp_test_style(color, sp_fmt_style_yellow), sp_fmt_cstr("skipped"),
        sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(t->skip_reason),
        sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(duration));
      break;
    }
    case SP_TEST_WIRE_OK: {
      sp_fmt_io(io, "{} {.$} {.$}\n",
        sp_fmt_cstr(t->name),
        sp_test_style(color, sp_fmt_style_green), sp_fmt_cstr("ok"),
        sp_test_style(color, sp_fmt_style_gray), sp_fmt_str(duration));
      break;
    }
  }

  sp_da_for(t->notes, it) {
    sp_fmt_io(io, "  {.$} {}\n", sp_test_style(color, sp_fmt_style_gray), sp_fmt_cstr("note"), sp_fmt_str(t->notes[it]));
  }

  sp_da_for(t->failures, it) {
    sp_test_report_failure(io, t->mem, color, &t->failures[it]);
  }

  if (!sp_da_empty(t->failures)) {
    sp_test_report_log(io, t->mem, color, t);
  }
}


static sp_test_t* sp_test_context_new(sp_test_runner_t* runner, sp_test_instance_t* instance) {
  sp_mem_arena_t* arena = sp_mem_arena_new(sp_mem_os_new());
  sp_mem_t mem = sp_mem_arena_as_allocator(arena);

  sp_test_t* t = sp_alloc_type(mem, sp_test_t);
  sp_mem_zero(t, sizeof(*t));

  t->name = instance->name;
  t->decl = instance->decl;
  t->arg = instance->arg;
  t->user = instance->decl->user;
  t->runner = runner;
  t->bookkeeping = arena;
  t->mem = mem;
  t->failures = sp_da_new(mem, sp_test_failure_t);
  t->kvs = sp_da_new(mem, sp_test_kv_t);
  t->logs = sp_da_new(mem, sp_str_t);
  t->notes = sp_da_new(mem, sp_str_t);
  return t;
}

static void sp_test_context_destroy(sp_test_t* t) {
  sp_mem_arena_destroy(t->bookkeeping);
}

static sp_err_t sp_test_invoke(sp_test_t* t) {
  if (!t->decl->fn && !t->decl->each) {
    sp_test_fail(t, "decl has no test function");
    return SP_ERR;
  }

  if (t->decl->setup) {
    sp_err_t err = t->decl->setup(t);
    if (err) {
      if (!t->skipped && sp_da_empty(t->failures)) {
        sp_test_fail(t, "setup failed: {}", sp_fmt_str(sp_test_err_str(t, err)));
      }
      return err;
    }
  }

  sp_err_t err = SP_OK;
  if (t->decl->fn) err = t->decl->fn(t);
  else             err = t->decl->each(t, t->arg);

  if (t->decl->teardown) {
    t->decl->teardown(t);
  }
  return err;
}

static s32 sp_test_leak_order(const void* a, const void* b) {
  const sp_test_alloc_t* lhs = (const sp_test_alloc_t*)a;
  const sp_test_alloc_t* rhs = (const sp_test_alloc_t*)b;
  return (lhs->id > rhs->id) - (lhs->id < rhs->id);
}

static void sp_test_report_leaks(sp_test_t* t) {
  sp_test_tracking_t* k = &t->tracking;
  if (!k->live_count && !k->double_frees && !k->wild_frees && !k->bad_sizes) return;

  sp_io_dyn_mem_writer_t message = sp_zero;
  sp_io_dyn_mem_writer_init(t->mem, &message);

  if (k->live_count) {
    sp_fmt_io(&message.base, "leaked {} {}, {} bytes\n",
      sp_fmt_uint(k->live_count),
      sp_fmt_cstr(k->live_count == 1 ? "allocation" : "allocations"),
      sp_fmt_uint(k->live_bytes));

    sp_da(sp_test_alloc_t) leaks = sp_da_new(t->mem, sp_test_alloc_t);
    sp_ht_for_kv(k->allocs, it) {
      if (it.val->state == SP_TEST_ALLOC_LIVE) sp_da_push(leaks, *it.val);
    }
    sp_os_qsort(leaks, sp_da_size(leaks), sizeof(sp_test_alloc_t), sp_test_leak_order);

    sp_da_for(leaks, it) {
      sp_fmt_io(&message.base, "  #{} {} bytes\n", sp_fmt_uint(leaks[it].id), sp_fmt_uint(leaks[it].size));
    }
  }
  if (k->double_frees) sp_fmt_io(&message.base, "{} double frees\n", sp_fmt_uint(k->double_frees));
  if (k->wild_frees)   sp_fmt_io(&message.base, "{} wild frees\n", sp_fmt_uint(k->wild_frees));
  if (k->bad_sizes)    sp_fmt_io(&message.base, "{} frees with a bad size\n", sp_fmt_uint(k->bad_sizes));

  sp_test_record(t, (sp_test_failure_t) {
    .message = sp_io_dyn_mem_writer_as_str(&message),
  });
}

static void sp_test_teardown(sp_test_t* t) {
  if (t->tracking_live) {
    if (!t->skipped) sp_test_report_leaks(t);
    sp_test_tracking_deinit(&t->tracking);
    sp_mem_heap_destroy(t->tracking_heap);
    t->tracking_heap = SP_NULLPTR;
    t->tracking_live = false;
  }

  if (!sp_str_empty(t->dir) && sp_fs_exists(t->dir)) {
    sp_fs_remove_dir(t->dir);
  }

  if (t->scratch) {
    sp_mem_arena_destroy(t->scratch);
    t->scratch = SP_NULLPTR;
  }
}

static void sp_test_run_instance(sp_test_runner_t* runner, sp_test_instance_t* instance) {
  sp_test_t* t = sp_test_context_new(runner, instance);

  sp_tm_point_t start = sp_tm_now_point();
  sp_err_t err = sp_test_invoke(t);
  sp_test_teardown(t);
  u64 ns = sp_tm_point_diff(sp_tm_now_point(), start);

  if (err && !t->skipped && sp_da_empty(t->failures)) {
    sp_test_record(t, (sp_test_failure_t) {
      .message = sp_test_format(t, "test returned err {} with no failure recorded", sp_fmt_int(err)),
    });
  }

  sp_test_wire_status_t status = sp_test_status(t);

  sp_io_dyn_mem_writer_t report = sp_zero;
  sp_io_dyn_mem_writer_init(t->mem, &report);
  sp_test_report(t, &report.base, status, ns);

  sp_str_t text = sp_io_dyn_mem_writer_as_str(&report);

  sp_mutex_lock(&runner->mutex);
  sp_io_write_str(&runner->out.base, text, SP_NULLPTR);
  sp_io_flush(&runner->out.base);
  switch (status) {
    case SP_TEST_WIRE_FAIL:   sp_da_push(runner->failed, instance->name); break;
    case SP_TEST_WIRE_SKIP:   sp_da_push(runner->skipped, instance->name); break;
    case SP_TEST_WIRE_UPDATE: sp_da_push(runner->updated, instance->name); break;
    case SP_TEST_WIRE_OK: break;
  }
  sp_mutex_unlock(&runner->mutex);

  sp_test_context_destroy(t);
}

static s32 sp_test_worker(void* userdata) {
  sp_test_runner_t* runner = (sp_test_runner_t*)userdata;
  while (true) {
    s32 slot = sp_atomic_s32_add(&runner->cursor, 1, SP_ATOMIC_RELAXED);
    if ((u64)slot >= sp_da_size(runner->queue)) break;
    sp_test_run_instance(runner, &runner->queue[slot]);
  }
  return 0;
}


bool sp_test_filtered(sp_glob_t* filter, const c8* name) {
  if (!filter) return false;

  sp_str_t path = sp_cstr_as_str(name);
  if (sp_glob_match(filter, path)) return false;

  sp_for(it, path.len) {
    if (path.data[it] != '.' && path.data[it] != '/') continue;
    if (sp_glob_match(filter, sp_str_prefix(path, (s32)it))) return false;
  }

  return true;
}

static u32 sp_test_num_cpus(void) {
#if defined(SP_SYSCALL_NUM_SCHED_GETAFFINITY)
  u64 mask [16] = sp_zero;
  s64 bytes = sp_syscall(SP_SYSCALL_NUM_SCHED_GETAFFINITY, 0, sizeof(mask), mask);
  if (bytes <= 0) return 1;

  u32 count = 0;
  sp_for(it, (u32)bytes / (u32)sizeof(u64)) {
    u64 word = mask[it];
    while (word) {
      count += (u32)(word & 1);
      word >>= 1;
    }
  }
  return count ? count : 1;
#else
  return 1;
#endif
}

static bool sp_test_axis_fits(s64 value, u64 size) {
  if (size == 8) return true;
  s64 bits = (s64)size * 8;
  return value >= -((s64)1 << (bits - 1)) && value < ((s64)1 << bits);
}

static u64 sp_test_axis_count(const sp_test_axis_t* axis) {
  switch (axis->size) {
    case 1: case 2: case 4: case 8: break;
    default: SP_ASSERT(false);
  }

  switch (axis->kind) {
    case SP_TEST_AXIS_VALUES: {
      SP_ASSERT(axis->count);
      return axis->count;
    }
    case SP_TEST_AXIS_RANGE: {
      SP_ASSERT(axis->from <= axis->to);
      SP_ASSERT(axis->step > 0);
      return (u64)(axis->to - axis->from) / (u64)axis->step + 1;
    }
    default: SP_UNREACHABLE_RETURN(0);
  }
}

static s64 sp_test_axis_value(const sp_test_axis_t* axis, u64 pick) {
  switch (axis->kind) {
    case SP_TEST_AXIS_VALUES: return axis->values[pick];
    case SP_TEST_AXIS_RANGE: return axis->from + (s64)pick * axis->step;
    default: SP_UNREACHABLE_RETURN(0);
  }
}

static void sp_test_axis_apply(const sp_test_axis_t* axis, s64 value, u8* row) {
  SP_ASSERT(sp_test_axis_fits(value, axis->size));
  sp_mem_copy(row + axis->offset, &value, axis->size);
}

static const c8* sp_test_instance_base(sp_mem_t mem, const c8* suite, const sp_test_decl_t* decl, u32 row) {
  if (!decl->each || !decl->cases) {
    return sp_fmt_mem_cstr(mem, "{}.{}", sp_fmt_cstr(suite), sp_fmt_cstr(decl->name));
  }

  if (decl->case_name_offset) {
    const u8* row_base = (const u8*)decl->cases + row * decl->stride;
    const c8* case_name = *(const c8* const*)(row_base + decl->case_name_offset - 1);
    if (case_name) {
      return sp_fmt_mem_cstr(mem, "{}.{}.{}",
        sp_fmt_cstr(suite),
        sp_fmt_cstr(decl->name),
        sp_fmt_cstr(case_name));
    }
  }

  return sp_fmt_mem_cstr(mem, "{}.{}.{}",
    sp_fmt_cstr(suite),
    sp_fmt_cstr(decl->name),
    sp_fmt_uint(row));
}

void sp_test_expand(sp_mem_t mem, const c8* suite, const sp_test_decl_t* decl, bool serial, sp_glob_t* filter, sp_da(sp_test_instance_t)* out) {
  u64 counts [SP_TEST_MAX_AXES];
  u32 num_axes = 0;
  u64 combos = 1;
  sp_carr_for(decl->axes, it) {
    if (decl->axes[it].kind == SP_TEST_AXIS_NONE) break;
    counts[num_axes] = sp_test_axis_count(&decl->axes[it]);
    combos *= counts[num_axes];
    num_axes++;
  }
  SP_ASSERT(!num_axes || decl->each);

  u64 rows = decl->each && decl->cases ? decl->count : 1;
  sp_for(row, rows) {
    const u8* source = SP_NULLPTR;
    if (decl->cases) source = (const u8*)decl->cases + row * decl->stride;
    else if (decl->each) source = (const u8*)sp_alloc(mem, decl->stride);
    const c8* base = sp_test_instance_base(mem, suite, decl, row);

    sp_for(combo, combos) {
      const c8* name = base;
      const void* arg = source;

      if (num_axes) {
        u64 picks [SP_TEST_MAX_AXES];
        u64 rest = combo;
        for (u32 it = num_axes; it-- > 0;) {
          picks[it] = rest % counts[it];
          rest /= counts[it];
        }

        u8* copy = (u8*)sp_alloc(mem, decl->stride);
        sp_mem_copy(copy, source, decl->stride);
        sp_for(it, num_axes) {
          const sp_test_axis_t* axis = &decl->axes[it];
          s64 value = sp_test_axis_value(axis, picks[it]);
          sp_test_axis_apply(axis, value, copy);
          if (axis->name) {
            name = sp_fmt_mem_cstr(mem, "{}.{}={}",
              sp_fmt_cstr(name),
              sp_fmt_cstr(axis->field),
              sp_fmt_cstr(axis->name(value)));
          }
          else {
            name = sp_fmt_mem_cstr(mem, "{}.{}={}",
              sp_fmt_cstr(name),
              sp_fmt_cstr(axis->field),
              sp_fmt_int(value));
          }
        }
        arg = copy;
      }

      if (sp_test_filtered(filter, name)) continue;

      sp_da_push(*out, ((sp_test_instance_t) {
        .name = name,
        .decl = decl,
        .arg = arg,
        .serial = serial || decl->serial,
      }));
    }
  }
}

#if SP_TEST_AUTOREG
#if defined(SP_WIN32)
extern IMAGE_DOS_HEADER __ImageBase;

typedef struct {
  const void* begin;
  const void* end;
} sp_test_pe_bounds_t;

static sp_test_pe_bounds_t sp_test_pe_bounds(const c8* name) {
  sp_test_pe_bounds_t bounds = sp_zero;

  const u8* base = (const u8*)&__ImageBase;
  const IMAGE_NT_HEADERS* nt = (const IMAGE_NT_HEADERS*)(base + ((const IMAGE_DOS_HEADER*)base)->e_lfanew);
  const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);

  sp_for(it, nt->FileHeader.NumberOfSections) {
    const IMAGE_SECTION_HEADER* section = &sections[it];

    const c8* want = name;
    bool match = true;
    sp_for(n, (u32)sizeof(section->Name)) {
      if ((c8)section->Name[n] != *want) { match = false; break; }
      if (*want) want++;
    }
    if (!match) continue;

    bounds.begin = base + section->VirtualAddress;
    bounds.end   = base + section->VirtualAddress + section->Misc.VirtualSize;
    break;
  }

  return bounds;
}

#define __sp_test_reg_begin   ((const sp_test_reg_t* const*)sp_test_pe_bounds("sp_test").begin)
#define __sp_test_reg_end     ((const sp_test_reg_t* const*)sp_test_pe_bounds("sp_test").end)
#define __sp_test_suite_begin ((const sp_test_suite_attr_t* const*)sp_test_pe_bounds("sp_suite").begin)
#define __sp_test_suite_end   ((const sp_test_suite_attr_t* const*)sp_test_pe_bounds("sp_suite").end)
#else
#if defined(SP_MACOS)
extern const sp_test_reg_t* const __start_sp_test[] __asm("section$start$__DATA$sp_test");
extern const sp_test_reg_t* const __stop_sp_test[] __asm("section$end$__DATA$sp_test");
extern const sp_test_suite_attr_t* const __start_sp_test_suite[] __asm("section$start$__DATA$sp_test_suite");
extern const sp_test_suite_attr_t* const __stop_sp_test_suite[] __asm("section$end$__DATA$sp_test_suite");
#else
extern const sp_test_reg_t* const __start_sp_test[] __attribute__((weak));
extern const sp_test_reg_t* const __stop_sp_test[] __attribute__((weak));
extern const sp_test_suite_attr_t* const __start_sp_test_suite[] __attribute__((weak));
extern const sp_test_suite_attr_t* const __stop_sp_test_suite[] __attribute__((weak));
#endif
#define __sp_test_reg_begin   (__start_sp_test)
#define __sp_test_reg_end     (__stop_sp_test)
#define __sp_test_suite_begin (__start_sp_test_suite)
#define __sp_test_suite_end   (__stop_sp_test_suite)
#endif
#endif

static bool sp_test_suite_serial(const c8* suite) {
#if SP_TEST_AUTOREG
  for (const sp_test_suite_attr_t* const* it = __sp_test_suite_begin, * const* end = __sp_test_suite_end; it < end; it++) {
    if (!*it) continue;
    if (sp_cstr_equal((*it)->suite, suite)) return (*it)->serial;
  }
#else
  SP_UNUSED(suite);
#endif
  return false;
}

static void sp_test_collect(sp_mem_t mem, const sp_test_suite_t* suites, sp_glob_t* filter, sp_da(sp_test_instance_t)* out) {
  if (suites) {
    for (const sp_test_suite_t* suite = suites; suite->name; suite++) {
      bool serial = suite->serial || sp_test_suite_serial(suite->name);
      for (const sp_test_decl_t* decl = suite->tests; decl->name; decl++) {
        sp_test_expand(mem, suite->name, decl, serial, filter, out);
      }
    }
  }

#if SP_TEST_AUTOREG
  for (const sp_test_reg_t* const* it = __sp_test_reg_begin, * const* end = __sp_test_reg_end; it < end; it++) {
    if (!*it) continue;
    sp_test_expand(mem, (*it)->suite, &(*it)->decl, sp_test_suite_serial((*it)->suite), filter, out);
  }
#endif
}

static sp_str_t sp_test_abi_name(void) {
#if defined(SP_LIBC_MSVC)
  return sp_str_lit("msvc");
#elif defined(SP_LIBC_GNU)
  return sp_str_lit("gnu");
#elif defined(SP_LIBC_NONE)
  return sp_str_lit("none");
#elif defined(SP_MACOS)
  return sp_str_lit("apple");
#elif defined(SP_LINUX)
  return sp_str_lit("musl");
#else
  return sp_str_lit("unknown");
#endif
}

static sp_str_t sp_test_arch_name(void) {
#if defined(SP_AMD64)
  return sp_str_lit("x86_64");
#elif defined(SP_ARM64)
  return sp_str_lit("aarch64");
#elif defined(SP_WASM)
  return sp_str_lit("wasm32");
#else
  return sp_str_lit("unknown");
#endif
}

static sp_cli_result_t sp_test_cli_handler(sp_cli_t* cli) {
  SP_UNUSED(cli);
  return SP_CLI_CONTINUE;
}

s32 sp_test_main(s32 argc, const c8** argv, const sp_test_suite_t* suites) {
  const c8* filter = SP_NULLPTR;
  const c8* golden_root = SP_NULLPTR;
  u32 jobs = 1;
  bool list = false;
  bool update = false;

  sp_cli_cmd_t cli = {
    .name = "sp_test",
    .summary = "run registered tests",
    .opts = {
      {
        .name = "list",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "print instance names, one per line",
        .ptr = &list,
      },
      {
        .name = "filter",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "run only instances matching a glob",
        .placeholder = "glob",
        .ptr = &filter,
      },
      {
        .name = "jobs",
        .kind = SP_CLI_OPT_U32,
        .summary = "run on n threads; 0 uses one per processor",
        .placeholder = "n",
        .ptr = &jobs,
      },
      {
        .name = "update",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "write golden files from actual values instead of comparing",
        .ptr = &update,
      },
      {
        .name = "golden-root",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "resolve golden files against this directory",
        .placeholder = "dir",
        .ptr = &golden_root,
      },
    },
    .handler = sp_test_cli_handler,
  };

  switch (sp_cli_run((sp_cli_desc_t) { .root = &cli, .args = argv, .num_args = argc })) {
    case SP_CLI_OK:
    case SP_CLI_CONTINUE: break;
    case SP_CLI_HELP:     return 0;
    case SP_CLI_ERR:      return 1;
  }

  sp_mem_arena_t* arena = sp_mem_arena_new(sp_mem_os_new());

  sp_test_runner_t* runner = sp_alloc_type(sp_mem_arena_as_allocator(arena), sp_test_runner_t);
  sp_mem_zero(runner, sizeof(*runner));
  runner->mem = sp_mem_arena_as_allocator(arena);
  runner->queue = sp_da_new(runner->mem, sp_test_instance_t);
  runner->failed = sp_da_new(runner->mem, const c8*);
  runner->skipped = sp_da_new(runner->mem, const c8*);
  runner->updated = sp_da_new(runner->mem, const c8*);
  runner->update = update;

  sp_str_t root = sp_os_env_get(sp_str_lit("SP_TEST_GOLDEN_ROOT"));
  if (golden_root && *golden_root) root = sp_cstr_as_str(golden_root);
  if (!sp_str_empty(root)) {
    runner->golden_root = sp_fs_normalize_path(runner->mem, root);
  }

  sp_mutex_init(&runner->mutex);
  runner->color = sp_sys_is_tty(sp_sys_stdout);
  sp_io_stream_writer_from_fd(&runner->out, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
  sp_io_writer_set_buffer(&runner->out.base, runner->out_buffer, sizeof(runner->out_buffer));

  sp_glob_t* glob = SP_NULLPTR;
  if (filter && *filter) {
    glob = sp_glob_new(runner->mem, filter);
    if (!glob) {
      sp_fmt_io(&runner->out.base, "invalid filter {.quote}\n", sp_fmt_cstr(filter));
      sp_io_flush(&runner->out.base);
      return 1;
    }
  }

  sp_test_collect(runner->mem, suites, glob, &runner->queue);

  sp_cstr_ht(bool) seen = SP_NULLPTR;
  sp_cstr_ht_init(runner->mem, seen);
  sp_da_for(runner->queue, it) {
    const c8* name = runner->queue[it].name;
    if (sp_cstr_ht_get(seen, name)) {
      sp_fmt_io(&runner->out.base, "duplicate test {.quote}\n", sp_fmt_cstr(name));
      sp_io_flush(&runner->out.base);
      return 1;
    }
    sp_cstr_ht_insert(seen, name, true);
  }

  if (list) {
    sp_da_for(runner->queue, it) {
      sp_fmt_io(&runner->out.base, "{}\n", sp_fmt_cstr(runner->queue[it].name));
    }
    sp_io_flush(&runner->out.base);
    return 0;
  }

  sp_str_t iso = sp_tm_epoch_to_iso8601(runner->mem, sp_tm_now_epoch());
  runner->dir_root = sp_fs_join_path(runner->mem,
    sp_fs_join_path(runner->mem, sp_fs_get_cwd(runner->mem), sp_str_lit(".spn/test")),
    sp_str_replace_c8(runner->mem, iso, ':', '-'));

  if (jobs == 0) jobs = sp_test_num_cpus();

  sp_da(sp_test_instance_t) parallel = sp_da_new(runner->mem, sp_test_instance_t);
  sp_da(sp_test_instance_t) serial = sp_da_new(runner->mem, sp_test_instance_t);
  sp_da_for(runner->queue, it) {
    if (runner->queue[it].serial) sp_da_push(serial, runner->queue[it]);
    else                          sp_da_push(parallel, runner->queue[it]);
  }

  sp_fmt_io(&runner->out.base, "> running {.$} test cases on {}-{}-{}\n",
    sp_test_style(runner->color, sp_fmt_style_black), sp_fmt_uint(sp_da_size(runner->queue)),
    sp_fmt_str(sp_test_arch_name()),
    sp_fmt_str(sp_os_get_name()),
    sp_fmt_str(sp_test_abi_name()));
  sp_io_flush(&runner->out.base);

  runner->queue = parallel;
  jobs = (u32)sp_min((u64)jobs, sp_da_size(parallel));
  if (jobs <= 1) {
    sp_da_for(parallel, it) {
      sp_test_run_instance(runner, &parallel[it]);
    }
  }
  else {
    sp_thread_t* threads = sp_alloc_n(runner->mem, sp_thread_t, jobs);
    sp_for(it, jobs) {
      sp_thread_init(&threads[it], sp_test_worker, runner);
    }
    sp_for(it, jobs) {
      sp_thread_join(&threads[it]);
    }
  }

  sp_da_for(serial, it) {
    sp_test_run_instance(runner, &serial[it]);
  }

  u64 total = sp_da_size(parallel) + sp_da_size(serial);
  u64 failed = sp_da_size(runner->failed);
  u64 skipped = sp_da_size(runner->skipped);
  u64 updated = sp_da_size(runner->updated);

  sp_fmt_io(&runner->out.base, "> {.$} passed, {.$} failed, {.$} skipped",
    sp_test_style(runner->color, sp_fmt_style_green), sp_fmt_uint(total - failed - skipped - updated),
    sp_test_style(runner->color, sp_fmt_style_red), sp_fmt_uint(failed),
    sp_test_style(runner->color, sp_fmt_style_yellow), sp_fmt_uint(skipped));
  if (runner->update) {
    sp_fmt_io(&runner->out.base, ", {.$} updated", sp_test_style(runner->color, sp_fmt_style_cyan), sp_fmt_uint(updated));
  }
  sp_fmt_io(&runner->out.base, "\n");

  sp_da_for(runner->failed, it) {
    sp_fmt_io(&runner->out.base, "  {.$} {}\n", sp_test_style(runner->color, sp_fmt_style_red), sp_fmt_cstr("failed"), sp_fmt_cstr(runner->failed[it]));
  }
  sp_io_flush(&runner->out.base);

  if (sp_fs_exists(runner->dir_root)) {
    sp_fs_remove_dir(runner->dir_root);
  }

  sp_mutex_destroy(&runner->mutex);
  sp_mem_arena_destroy(arena);
  return failed ? 1 : 0;
}

#endif
