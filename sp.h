#ifndef SP_SPACE_H
#define SP_SPACE_H

#ifdef _WIN32
  #undef UNICODE
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #define _CRT_RAND_S
  #define _AMD64_

  #ifdef SP_NO_WINDOWS_H
    #include "windef.h"
    #include "winbase.h"
    #include "wingdi.h"
    #include "winuser.h"
    #include "synchapi.h"
    #include "fileapi.h"
    #include "handleapi.h"
    #include "shellapi.h"
    #include "stringapiset.h"
  #else
    #include "windows.h"
    #include "shlobj.h"
    #include "commdlg.h"
    #include "shellapi.h"
  #endif

  #include "threads.h"
#endif // _WIN32

#ifdef __APPLE__
  #include "pthread.h"
#endif

#ifdef __LINUX__
  #include "threads.h"
#endif

#ifdef SP_NO_STDLIB
  typedef signed char        int8_t;
  typedef short              int16_t;
  typedef int                int32_t;
  typedef long long          int64_t;
  typedef unsigned char      uint8_t;
  typedef unsigned short     uint16_t;
  typedef unsigned int       uint32_t;
  typedef unsigned long long uint64_t;
  typedef unsigned long      uintptr_t;  // Add for pointer conversions

  #ifndef __cplusplus
    #define bool  _Bool
    #define false 0
    #define true  1
  #endif

  #include "assert.h"
#else
  #include "assert.h"
  #include "stdbool.h"
  #include "stddef.h"
  #include "stdint.h" 
  #include "stdio.h"
  #include "stdlib.h"
  #include "string.h"
  #include "wchar.h"
#endif


///////////
// TYPES //
///////////
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef char     c8;
typedef wchar_t  c16;

typedef struct {
  u64 s;
  u64 ns;
} sp_precise_epoch_time_t;

typedef sp_precise_epoch_time_t sp_time_t;

typedef u64 sp_precise_time_t;

typedef void* sp_opaque_ptr;

////////////
// MACROS //
////////////
#define SP_API
#define SP_IMP

#ifdef __cplusplus
  #define SP_LVAL(T)
  #define SP_THREAD_LOCAL thread_local
  #define SP_BEGIN_EXTERN_C() extern "C" {
  #define SP_END_EXTERN_C() }
#else
  #define SP_LVAL(T) (T)
  #define SP_THREAD_LOCAL _Thread_local
  #define SP_BEGIN_EXTERN_C()
  #define SP_END_EXTERN_C()
#endif

#define SP_ZERO_INITIALIZE() {0}
#define SP_ZERO_STRUCT(t) SP_LVAL(t) SP_ZERO_INITIALIZE()
#define SP_ZERO_RETURN(t) { t __SP_ZERO_RETURN = SP_ZERO_STRUCT(t); return __dn_zero_return; }

#define SP_ASSERT(condition) assert((condition))
#define SP_UNREACHABLE() SP_ASSERT(false)
#define SP_UNREACHABLE_CASE() SP_ASSERT(false); break;
#define SP_UNREACHABLE_RETURN(v) SP_ASSERT(false); return (v)
#define SP_FATAL() SP_ASSERT(false)
#define SP_BROKEN() SP_ASSERT(false)

#define SP_TYPEDEF_FN(return_type, name, ...) typedef return_type(*name)(__VA_ARGS__)

#define SP_PRINTF_U8 "%hhu"
#define SP_PRINTF_U16 "%hu"
#define SP_PRINTF_U32 "%u"
#define SP_PRINTF_U64 "%lu"
#define SP_PRINTF_S8 "%hhd"
#define SP_PRINTF_S16 "%hd"
#define SP_PRINTF_S32 "%d"
#define SP_PRINTF_S64 "%ld"
#define SP_PRINTF_F32 "%f"
#define SP_PRINTF_F64 "%f"

#define _SP_MACRO_STR(x) #x
#define SP_MACRO_STR(x) _SP_MACRO_STR(x)
#define _SP_MACRO_CAT(x, y) x##y
#define SP_MACRO_CAT(x, y) _SP_MACRO_CAT(x, y)

#define SP_UNIQUE_ID() SP_MACRO_CAT(__sp_unique_name__, __LINE__)

#define SP_MAX(a, b) (a) > (b) ? (a) : (b)
#define SP_MIN(a, b) (a) > (b) ? (b) : (a)
#define SP_SWAP(t, a, b) { t SP_UNIQUE_ID() = (a); (a) = (b); (b) = SP_UNIQUE_ID(); }

#define SP_QSORT_A_FIRST -1
#define SP_QSORT_B_FIRST 1
#define SP_QSORT_EQUAL 0

#define SP_COLOR_255(RED, GREEN, BLUE) { .r = (RED) / 255.f, .g = (GREEN) / 255.f, .b = (BLUE) / 255.f, .a = 1.0 }

#define SP_MAX_PATH_LEN 260

#define SP_ENUM_NAME_CASE(qualified_name) case qualified_name: { return SP_MACRO_STR(qualified_name); }

#define SP_CARR_LEN(CARR) (sizeof((CARR)) / sizeof((CARR)[0]))
#define SP_CARR_FOR(CARR, IT) for (u32 IT = 0; IT < SP_CARR_LEN(CARR); IT++)


SP_BEGIN_EXTERN_C()

//  ███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗
//  ████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗╚██╗ ██╔╝
//  ██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝ ╚████╔╝
//  ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗  ╚██╔╝
//  ██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║   ██║
//  ╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝
typedef enum {
  SP_ALLOCATOR_MODE_ALLOC,
  SP_ALLOCATOR_MODE_FREE,
  SP_ALLOCATOR_MODE_RESIZE,
} sp_allocator_mode_t;

SP_TYPEDEF_FN(
  void*,
  sp_alloc_fn_t,
  void* user_data, sp_allocator_mode_t mode, u32 size, void* ptr
);

typedef struct sp_allocator_t {
  sp_alloc_fn_t on_alloc;
  void* user_data;
} sp_allocator_t;

typedef struct {
  u8* buffer;
  u32 capacity;
  u32 bytes_used;
} sp_bump_allocator_t;

typedef struct {
  u32 size;
} sp_malloc_metadata_t;


typedef struct {
  sp_allocator_t allocator;
} sp_allocator_malloc_t;

typedef struct {
  sp_allocator_t* allocator;

  struct {
    const c8* file;
    u32 line;
  } with;
} sp_context_t;
       
#define SP_MAX_CONTEXT 16
SP_THREAD_LOCAL sp_context_t sp_context_stack [SP_MAX_CONTEXT] = SP_ZERO_INITIALIZE();
SP_THREAD_LOCAL sp_context_t* sp_context = SP_ZERO_INITIALIZE();

void                  sp_context_set(sp_context_t context);
void                  sp_context_push(sp_context_t context);
void                  sp_context_push_allocator(sp_allocator_t* allocator);
void                  sp_context_pop();
void*                 sp_allocator_alloc(sp_allocator_t allocator, u32 size);
void*                 sp_allocator_realloc(sp_allocator_t allocator, void* memory, u32 size);
void                  sp_allocator_free(sp_allocator_t allocator, void* buffer);
sp_allocator_t        sp_bump_allocator_init(sp_bump_allocator_t* allocator, u32 capacity);
void                  sp_bump_allocator_clear(sp_bump_allocator_t* allocator);
void                  sp_bump_allocator_destroy(sp_bump_allocator_t* allocator);
void*                 sp_bump_allocator_on_alloc(void* allocator, sp_allocator_mode_t mode, u32 size, void* old_memory);
sp_allocator_t        sp_allocator_malloc_init(sp_allocator_malloc_t* allocator);
void*                 sp_allocator_malloc_on_alloc(void* user_data, sp_allocator_mode_t mode, u32 size, void* ptr);
sp_malloc_metadata_t* sp_malloc_allocator_get_metadata(void* ptr);
void*                 sp_alloc(u32 size);
void*                 sp_realloc(void* memory, u32 size);
void                  sp_free(void* memory);


//  ██╗  ██╗ █████╗ ███████╗██╗  ██╗██╗███╗   ██╗ ██████╗
//  ██║  ██║██╔══██╗██╔════╝██║  ██║██║████╗  ██║██╔════╝
//  ███████║███████║███████╗███████║██║██╔██╗ ██║██║  ███╗
//  ██╔══██║██╔══██║╚════██║██╔══██║██║██║╚██╗██║██║   ██║
//  ██║  ██║██║  ██║███████║██║  ██║██║██║ ╚████║╚██████╔╝
//  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
typedef u64 sp_hash_t;

sp_hash_t sp_hash_cstr(const c8* str);


//  ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗
//  ██╔════╝╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝
//  ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗
//  ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║
//  ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝
//  ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
typedef struct {
  u32 len;
  c8* data;
} sp_str_t;

// sp_str_buffer_t is for defining fixed size, stack allocated strings that have a length attached. Then, you
// can use sp_str_buffer_view() or sp_str_buffer_view_ptr() to turn it into a sp_str_t and use the normal
// string APIs.
#define sp_str_buffer_t(n) struct { u32 len; c8 data [n]; }
#define sp_str_buffer_capacity(buffer) (sizeof((buffer)->data))
#define sp_str_buffer_view(buffer) SP_LVAL(sp_str_t) { .len = (buffer).len, .data = (buffer).data }
#define sp_str_buffer_view_ptr(buffer) ((sp_str_t*)(buffer))
#define sp_str_copy_to_str_buffer(str, buffer) { sp_str_copy_to((str), (buffer)->data, sp_str_buffer_capacity(buffer)); (buffer)->len = (str).len; }

typedef c8 sp_path_t [SP_MAX_PATH_LEN];

typedef struct {
  c8* data;
  u32 count;
  u32 capacity;
} sp_str_builder_buffer_t;

typedef struct {
  sp_str_builder_buffer_t buffer;
} sp_str_builder_t;

#define sp_str_cstr(STR)   sp_str((STR), strlen(STR))
#define sp_str_lit(STR)    sp_str((STR), sizeof(STR) - 1)
#define sp_str(STR, LEN) SP_LVAL(sp_str_t) { .len = (u32)(LEN), .data = (c8*)(STR) }

SP_API void     sp_str_builder_grow(sp_str_builder_t* builder, u32 requested_capacity);
SP_API void     sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str);
SP_API void     sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str);
SP_API void     sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c);
SP_API sp_str_t sp_str_builder_write(sp_str_builder_t* builder);
SP_API c8*      sp_str_builder_write_cstr(sp_str_builder_t* builder);

SP_API c8*      sp_cstr_copy(const c8* str);
SP_API c8*      sp_cstr_copy_n(const c8* str, u32 length);
SP_API c8*      sp_cstr_copy_c8(const c8* str, u32 length);
SP_API void     sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length);
SP_API void     sp_cstr_copy_to_n(const c8* str, u32 length, c8* buffer, u32 buffer_length);
SP_API bool     sp_cstr_equal(const c8* a, const c8* b);
SP_API u32      sp_cstr_len(const c8* str);

SP_API c8*      sp_wstr_to_cstr(c16* str, u32 len);

SP_API c8*      sp_str_to_cstr(sp_str_t str);
SP_API c8*      sp_str_to_double_null_terminated(sp_str_t str);
SP_API c8*      sp_str_to_cstr_ex(sp_str_t str);
SP_API sp_str_t sp_str_copy(sp_str_t str);
SP_API sp_str_t sp_str_copy_cstr_n(const c8* str, u32 length);
SP_API sp_str_t sp_str_copy_cstr(const c8* str);
SP_API void     sp_str_copy_to_str(sp_str_t str, sp_str_t* dest, u32 capacity);
SP_API void     sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity);
SP_API sp_str_t sp_str_alloc(u32 capacity);
SP_API bool     sp_str_equal(sp_str_t a, sp_str_t b);
SP_API bool     sp_str_equal_cstr(sp_str_t a, const c8* b);
SP_API s32      sp_str_sort_kernel_alphabetical(const void* a, const void* b);
SP_API s32      sp_str_compare_alphabetical(sp_str_t a, sp_str_t b);
SP_API bool     sp_str_valid(sp_str_t str);
SP_API c8       sp_str_at(sp_str_t str, u32 index);


//   ██████╗ ██████╗ ███╗   ██╗████████╗ █████╗ ██╗███╗   ██╗███████╗██████╗ ███████╗
//  ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝██╔══██╗██╔════╝
//  ██║     ██║   ██║██╔██╗ ██║   ██║   ███████║██║██╔██╗ ██║█████╗  ██████╔╝███████╗
//  ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██║██║██║╚██╗██║██╔══╝  ██╔══██╗╚════██║
//  ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║██║██║ ╚████║███████╗██║  ██║███████║
//   ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝╚══════╝
/////////////////
// FIXED ARRAY //
/////////////////
typedef struct {
  u8* data;
  u32 size;
  u32 capacity;
  u32 element_size;
} sp_fixed_array_t;

#define sp_fixed_array(t, n) sp_fixed_array_t
#define sp_fixed_array_for(arr, it, t) for (t* it = (t*)arr.data; (it - (t*)arr.data) < arr.size; it++)
#define SP_FIXED_ARRAY_RUNTIME_SIZE

SP_API void sp_fixed_array_init(sp_fixed_array_t* fixed_array, u32 capacity, u32 element_size);
SP_API u8*  sp_fixed_array_push(sp_fixed_array_t* fixed_array, void* data, u32 count);
SP_API u8*  sp_fixed_array_reserve(sp_fixed_array_t* fixed_array, u32 count);
SP_API void sp_fixed_array_clear(sp_fixed_array_t* fixed_array);
SP_API u32  sp_fixed_array_byte_size(sp_fixed_array_t* fixed_array);
SP_API u8*  sp_fixed_array_at(sp_fixed_array_t* fixed_array, u32 index);

///////////////////
// DYNAMIC ARRAY //
///////////////////
typedef struct {
  u8* data;
  u32 size;
  u32 capacity;
  u32 element_size;
} sp_dynamic_array_t;

#define sp_dynamic_array(t) sp_dynamic_array_t

SP_API void sp_dynamic_array_init(sp_dynamic_array_t* arr, u32 element_size);
SP_API u8*  sp_dynamic_array_push(sp_dynamic_array_t* arr, void* data);
SP_API u8*  sp_dynamic_array_push_n(sp_dynamic_array_t* arr, void* data, u32 count);
SP_API u8*  sp_dynamic_array_reserve(sp_dynamic_array_t* arr, u32 count);
SP_API void sp_dynamic_array_clear(sp_dynamic_array_t* arr);
SP_API u32  sp_dynamic_array_byte_size(sp_dynamic_array_t* arr);
SP_API u8*  sp_dynamic_array_at(sp_dynamic_array_t* arr, u32 index);
SP_API void sp_dynamic_array_grow(sp_dynamic_array_t* arr, u32 capacity);


// ███████╗██╗██╗     ███████╗    ███╗   ███╗ ██████╗ ███╗   ██╗██╗████████╗ ██████╗ ██████╗ 
// ██╔════╝██║██║     ██╔════╝    ████╗ ████║██╔═══██╗████╗  ██║██║╚══██╔══╝██╔═══██╗██╔══██╗
// █████╗  ██║██║     █████╗      ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║   ██║   ██║██████╔╝
// ██╔══╝  ██║██║     ██╔══╝      ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║   ██║   ██║██╔══██╗
// ██║     ██║███████╗███████╗    ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝╚══════╝╚══════╝    ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝

typedef enum sp_file_change_event_t {
	SP_FILE_CHANGE_EVENT_NONE = 0,
	SP_FILE_CHANGE_EVENT_ADDED = 1 << 0,
	SP_FILE_CHANGE_EVENT_MODIFIED = 1 << 1,
	SP_FILE_CHANGE_EVENT_REMOVED = 1 << 2,
} sp_file_change_event_t;

typedef struct {
	sp_str_t file_path;
	sp_str_t file_name;
	sp_file_change_event_t events;
	f32 time;
} sp_file_change_t;

typedef struct sp_file_monitor sp_file_monitor_t;
SP_TYPEDEF_FN(void, sp_file_change_callback_t, sp_file_monitor_t*, sp_file_change_t*, void*);

typedef struct {
	sp_hash_t hash;
	f64 last_event_time;
} sp_cache_entry_t;

#define SP_FILE_MONITOR_BUFFER_SIZE 4092

typedef struct sp_file_monitor {
	sp_file_change_callback_t callback;
	sp_file_change_event_t events_to_watch;
	void* userdata;
	u32 debounce_time_ms;
	sp_dynamic_array_t changes;
	sp_dynamic_array_t cache;
  sp_opaque_ptr os;
} sp_file_monitor_t; 

SP_API void              sp_file_monitor_init(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata);
SP_API void              sp_file_monitor_init_debounce(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata, u32 debounce_ms);
SP_API void              sp_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path);
SP_IMP void              sp_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path);
SP_API void              sp_file_monitor_process_changes(sp_file_monitor_t* monitor);
SP_API void              sp_file_monitor_emit_changes(sp_file_monitor_t* monitor);
SP_API bool              sp_file_monitor_check_cache(sp_file_monitor_t* monitor, sp_str_t file_path, f64 time);
SP_API sp_cache_entry_t* sp_file_monitor_find_cache_entry(sp_file_monitor_t* monitor, sp_str_t file_path);


// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║   
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║   
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║   ██║   
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝
typedef struct sp_format_arg_t {
  sp_hash_t id;
  void* data;
} sp_format_arg_t; 

typedef struct sp_formatter sp_formatter_t;
SP_TYPEDEF_FN(void, sp_format_fn_t, sp_str_builder_t*, sp_format_arg_t*);

typedef struct sp_formatter {
    sp_hash_t id;
    sp_format_fn_t fn;
} sp_formatter_t;

#define SP_FMT_ID(T) sp_hash_cstr(SP_MACRO_STR(T))

#ifdef __cplusplus 
  #define SP_FMT_ARG(T, V) sp_make_format_arg(SP_FMT_ID(T), (V))
#else
  #define SP_FMT_ARG(T, V) SP_LVAL(sp_format_arg_t) { .id =  SP_FMT_ID(T), .data = (void*)&(V) }
#endif

#define SP_FMT_PTR(V) SP_FMT_ARG(ptr, V)
#define SP_FMT_STR(V) SP_FMT_ARG(str, V)
#define SP_FMT_CSTR(V) SP_FMT_ARG(cstr, V)
#define SP_FMT_S8(V) SP_FMT_ARG(s8, V)
#define SP_FMT_S16(V) SP_FMT_ARG(s16, V)
#define SP_FMT_S32(V) SP_FMT_ARG(s32, V)
#define SP_FMT_S64(V) SP_FMT_ARG(s64, V)
#define SP_FMT_U8(V) SP_FMT_ARG(u8, V)
#define SP_FMT_U16(V) SP_FMT_ARG(u16, V)
#define SP_FMT_U32(V) SP_FMT_ARG(u32, V)
#define SP_FMT_U64(V) SP_FMT_ARG(u64, V)
#define SP_FMT_F32(V) SP_FMT_ARG(f32, V)
#define SP_FMT_F64(V) SP_FMT_ARG(f64, V)
#define SP_FMT_C8(V) SP_FMT_ARG(c8, V)
#define SP_FMT_C16(V) SP_FMT_ARG(c16, V)
#define SP_FMT_CONTEXT(V) SP_FMT_ARG(context, V)
#define SP_FMT_HASH(V) SP_FMT_ARG(hash, V)
#define SP_FMT_STR_BUILDER(V) SP_FMT_ARG(str_builder, V)
#define SP_FMT_DATE_TIME(V) SP_FMT_ARG(date_time, V)
#define SP_FMT_THREAD(V) SP_FMT_ARG(thread, V)
#define SP_FMT_MUTEX(V) SP_FMT_ARG(mutex, V)
#define SP_FMT_SEMAPHORE(V) SP_FMT_ARG(semaphore, V)
#define SP_FMT_FIXED_ARRAY(V) SP_FMT_ARG(fixed_array, V)
#define SP_FMT_DYNAMIC_ARRAY(V) SP_FMT_ARG(dynamic_array, V)
 
void sp_format_ptr(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_str(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_cstr(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_s8(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_s16(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_s32(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_s64(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_u8(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_u16(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_u32(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_u64(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_f32(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_f64(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_c8(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_c16(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_context(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_hash(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_str_builder(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_date_time(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_thread(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_mutex(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_semaphore(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_fixed_array(sp_str_builder_t* builder, sp_format_arg_t* buffer);
void sp_format_dynamic_array(sp_str_builder_t* builder, sp_format_arg_t* buffer);

sp_str_t sp_fmt(sp_str_t fmt, ...);

#define SP_BUILTIN_FORMATTERS \
  SP_FORMATTER(ptr, sp_format_ptr), \
  SP_FORMATTER(str, sp_format_str), \
  SP_FORMATTER(cstr, sp_format_cstr), \
  SP_FORMATTER(s8, sp_format_s8), \
  SP_FORMATTER(s16, sp_format_s16), \
  SP_FORMATTER(s32, sp_format_s32), \
  SP_FORMATTER(s64, sp_format_s64), \
  SP_FORMATTER(u8, sp_format_u8), \
  SP_FORMATTER(u16, sp_format_u16), \
  SP_FORMATTER(u32, sp_format_u32), \
  SP_FORMATTER(u64, sp_format_u64), \
  SP_FORMATTER(f32, sp_format_f32), \
  SP_FORMATTER(f64, sp_format_f64), \
  SP_FORMATTER(c8, sp_format_c8), \
  SP_FORMATTER(c16, sp_format_c16), \
  SP_FORMATTER(context, sp_format_context), \
  SP_FORMATTER(hash, sp_format_hash), \
  SP_FORMATTER(str_builder, sp_format_str_builder), \
  SP_FORMATTER(date_time, sp_format_date_time), \
  SP_FORMATTER(thread, sp_format_thread), \
  SP_FORMATTER(mutex, sp_format_mutex), \
  SP_FORMATTER(semaphore, sp_format_semaphore), \
  SP_FORMATTER(fixed_array, sp_format_fixed_array), \
  SP_FORMATTER(dynamic_array, sp_format_dynamic_array)


//   ██████╗ ███████╗
//  ██╔═══██╗██╔════╝
//  ██║   ██║███████╗
//  ██║   ██║╚════██║
//  ╚██████╔╝███████║
//   ╚═════╝ ╚══════╝
typedef enum {
  SP_OS_FILE_ATTR_NONE = 0,
  SP_OS_FILE_ATTR_REGULAR_FILE = 1,
  SP_OS_FILE_ATTR_DIRECTORY = 2,
} sp_os_file_attr_t;

typedef enum {
  SP_MUTEX_NONE = 0,
  SP_MUTEX_PLAIN = 1,
  SP_MUTEX_TIMED = 2,
  SP_MUTEX_RECURSIVE = 4
} sp_mutex_kind_t;

// typedef FILE*              sp_os_file_t;

#ifdef _WIN32
  typedef HANDLE              sp_win32_handle_t;
  typedef DWORD               sp_win32_dword_t;
  typedef WIN32_FIND_DATA     sp_win32_find_data_t;
  typedef CRITICAL_SECTION    sp_win32_critical_section_t;
  typedef OVERLAPPED          sp_win32_overlapped_t;
  typedef PROCESS_INFORMATION sp_win32_process_information_t;
  typedef STARTUPINFO         sp_win32_startup_info_t;
  typedef SECURITY_ATTRIBUTES sp_win32_security_attributes_t;
  typedef thrd_t              sp_thread_t;
  typedef mtx_t               sp_mutex_t;
  typedef HANDLE              sp_semaphore_t;

  typedef struct {
  	sp_str_t path;
  	sp_win32_overlapped_t overlapped;
  	sp_win32_handle_t handle;
  	void* notify_information;
  	s32 bytes_returned;
  } sp_monitored_dir_t;

  typedef struct {
	  sp_dynamic_array_t directory_infos;
  } sp_os_win32_file_monitor_t;
  
  SP_IMP void sp_os_win32_file_monitor_add_change(sp_file_monitor_t* monitor, sp_str_t file_path, sp_str_t file_name, sp_file_change_event_t events);
  SP_IMP void sp_os_win32_file_monitor_issue_one_read(sp_file_monitor_t* monitor, sp_monitored_dir_t* info);
#else
  typedef void*               sp_thread_t;
  typedef void*               sp_mutex_t;
  typedef void*               sp_semaphore_t;

  typedef struct {

  } sp_os_file_monitor_t;
#endif

SP_TYPEDEF_FN(s32,         sp_thread_fn_t, void*);

typedef struct {
  s32 year;
  s32 month;
  s32 day;
  s32 hour;
  s32 minute;
  s32 second;
  s32 millisecond;
} sp_os_date_time_t;

typedef struct {
  sp_str_t file_path;
  sp_str_t file_name;
  sp_os_file_attr_t attributes;
} sp_os_directory_entry_t;

typedef struct {
  sp_os_directory_entry_t* data;
  u32 count;
} sp_os_directory_entry_list_t;

typedef struct {
  sp_thread_fn_t fn;
  void* userdata;
  sp_context_t context;
  sp_semaphore_t semaphore;
} sp_thread_launch_t;

SP_API void*                        sp_os_allocate_memory(u32 size);
SP_API void*                        sp_os_reallocate_memory(void* ptr, u32 size);
SP_API void                         sp_os_free_memory(void* ptr);
SP_API void                         sp_os_memory_copy(const void* source, void* dest, u32 num_bytes);
SP_API bool                         sp_os_is_memory_equal(const void* a, const void* b, size_t len);
SP_API void                         sp_os_fill_memory(void* buffer, u32 buffer_size, void* fill, u32 fill_size);
SP_API void                         sp_os_fill_memory_u8(void* buffer, u32 buffer_size, u8 fill);
SP_API void                         sp_os_zero_memory(void* buffer, u32 buffer_size);
SP_API bool                         sp_os_does_path_exist(sp_str_t path);
SP_API bool                         sp_os_is_regular_file(sp_str_t path);
SP_API bool                         sp_os_is_directory(sp_str_t path);
SP_API void                         sp_os_create_directory(sp_str_t path);
SP_API void                         sp_os_remove_directory(sp_str_t path);
SP_API void                         sp_os_create_file(sp_str_t path);
SP_API void                         sp_os_remove_file(sp_str_t path);
SP_API sp_os_directory_entry_list_t sp_os_scan_directory(sp_str_t path);
SP_API sp_os_directory_entry_list_t sp_os_scan_directory_recursive(sp_str_t path);
SP_API sp_os_date_time_t            sp_os_get_date_time();
SP_API void                         sp_os_normalize_path(sp_str_t path);
SP_API sp_str_t                     sp_os_parent_path(sp_str_t path);
SP_API sp_str_t                     sp_os_path_extension(sp_str_t path);
SP_API sp_str_t                     sp_os_extract_file_name(sp_str_t path);
SP_API void                         sp_os_sleep_ms(f64 ms);
SP_API sp_str_t                     sp_os_get_executable_path();
SP_API sp_str_t                     sp_os_canonicalize_path(sp_str_t path);
SP_API sp_precise_epoch_time_t      sp_os_file_mod_time_precise(sp_str_t path);
SP_API c8*                          sp_os_wstr_to_cstr(c16* str, u32 len);
SP_IMP sp_os_file_attr_t            sp_os_winapi_attr_to_sp_attr(u32 attr);
SP_IMP void                         sp_os_file_monitor_init(sp_file_monitor_t* monitor);
SP_IMP void                         sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path);
SP_IMP void                         sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path);
SP_IMP void                         sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor);
SP_API void                         sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata);
SP_API void                         sp_thread_join(sp_thread_t* thread);
SP_API s32                          sp_thread_launch(void* userdata);
SP_API void                         sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind);
SP_API void                         sp_mutex_lock(sp_mutex_t* mutex);
SP_API void                         sp_mutex_unlock(sp_mutex_t* mutex);
SP_API void                         sp_mutex_destroy(sp_mutex_t* mutex);
SP_API s32                          sp_mutex_kind_to_c11(sp_mutex_kind_t kind);
SP_API void                         sp_semaphore_init(sp_semaphore_t* semaphore);
SP_API void                         sp_semaphore_destroy(sp_semaphore_t* semaphore);
SP_API void                         sp_semaphore_wait(sp_semaphore_t* semaphore);
SP_API bool                         sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms);
SP_API void                         sp_semaphore_signal(sp_semaphore_t* semaphore);


SP_END_EXTERN_C()


//  ██████╗██████╗ ██████╗ 
// ██╔════╝██╔══██╗██╔══██╗
// ██║     ██████╔╝██████╔╝
// ██║     ██╔═══╝ ██╔═══╝ 
// ╚██████╗██║     ██║     
//  ╚═════╝╚═╝     ╚═╝     
#ifdef __cplusplus
  sp_str_t operator/(const sp_str_t& a, const sp_str_t& b);
  sp_str_t operator/(const sp_str_t& a, const c8* b);

  template <typename T> 
  sp_format_arg_t sp_make_format_arg(sp_hash_t id, T&& data) {
    sp_format_arg_t result = SP_ZERO_STRUCT(sp_format_arg_t);
    result.id = id;
    result.data = &data;
    return result;
  }
#endif


#endif // SP_SPACE_H


// ██╗███╗   ███╗██████╗ ██╗     ███████╗███╗   ███╗███████╗███╗   ██╗████████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██║████╗ ████║██╔══██╗██║     ██╔════╝████╗ ████║██╔════╝████╗  ██║╚══██╔══╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║██╔████╔██║██████╔╝██║     █████╗  ██╔████╔██║█████╗  ██╔██╗ ██║   ██║   ███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║██║╚██╔╝██║██╔═══╝ ██║     ██╔══╝  ██║╚██╔╝██║██╔══╝  ██║╚██╗██║   ██║   ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ██║██║ ╚═╝ ██║██║     ███████╗███████╗██║ ╚═╝ ██║███████╗██║ ╚████║   ██║   ██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
// ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
#ifdef SP_IMPLEMENTATION
SP_BEGIN_EXTERN_C()

// Shared formatting utilities
void sp_format_unsigned(sp_str_builder_t* builder, u64 num, u32 max_digits) {
    SP_ASSERT(builder);
    
    if (num == 0) {
        sp_str_builder_grow(builder, builder->buffer.count + 1);
        sp_str_builder_append_c8(builder, '0');
        return;
    }
    
    c8 digits[20]; // max 20 digits for u64
    s32 digit_count = 0;
    
    while (num > 0) {
        digits[digit_count++] = '0' + (num % 10);
        num /= 10;
    }
    
    SP_ASSERT((u32)digit_count <= max_digits);
    sp_str_builder_grow(builder, builder->buffer.count + digit_count);
    
    for (s32 i = digit_count - 1; i >= 0; i--) {
        sp_str_builder_append_c8(builder, digits[i]);
    }
}

void sp_format_signed(sp_str_builder_t* builder, s64 num, u32 max_digits) {
    SP_ASSERT(builder);
    
    bool negative = num < 0;
    u64 abs_value;
    
    if (negative) {
        // Handle INT_MIN properly by casting to unsigned first
        abs_value = (u64)(-(num + 1)) + 1;
        sp_str_builder_grow(builder, builder->buffer.count + 1);
        sp_str_builder_append_c8(builder, '-');
    } else {
        abs_value = (u64)num;
    }
    
    sp_format_unsigned(builder, abs_value, max_digits);
}

void sp_format_hex(sp_str_builder_t* builder, u64 value, u32 min_width, const c8* prefix) {
    SP_ASSERT(builder);
    
    if (prefix) {
        sp_str_builder_append_cstr(builder, prefix);
    }
    
    if (value == 0) {
        u32 zero_count = min_width > 0 ? min_width : 1;
        sp_str_builder_grow(builder, builder->buffer.count + zero_count);
        for (u32 i = 0; i < zero_count; i++) {
            sp_str_builder_append_c8(builder, '0');
        }
        return;
    }
    
    c8 hex_digits[16]; // max 16 hex digits for 64-bit
    s32 digit_count = 0;
    
    while (value > 0) {
        u8 digit = value & 0xF;
        hex_digits[digit_count++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
        value >>= 4;
    }
    
    // Pad to minimum width
    while (digit_count < (s32)min_width) {
        hex_digits[digit_count++] = '0';
    }
    
    sp_str_builder_grow(builder, builder->buffer.count + digit_count);
    
    for (s32 i = digit_count - 1; i >= 0; i--) {
        sp_str_builder_append_c8(builder, hex_digits[i]);
    }
}
  
void sp_format_str(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_str_t* value = (sp_str_t*)arg->data;
  SP_ASSERT(builder);
  SP_ASSERT(value);
  
  sp_str_builder_append(builder, *value);
}

void sp_format_cstr(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  c8** value = (c8**)arg->data;
  SP_ASSERT(builder);
  SP_ASSERT(value);
  SP_ASSERT(*value);
  
  sp_str_builder_append_cstr(builder, *value);
}

void sp_format_ptr(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  void** value = (void**)arg->data;
  u64 addr = (u64)*value;
  sp_format_hex(builder, addr, 8, "0x");
}

void sp_format_s8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s8* value = (s8*)arg->data;
  sp_format_signed(builder, *value, 3);
}

void sp_format_s16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s16* value = (s16*)arg->data;
  sp_format_signed(builder, *value, 5);
}

void sp_format_s32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s32* value = (s32*)arg->data;
  sp_format_signed(builder, *value, 10);
}

void sp_format_s64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s64* value = (s64*)arg->data;
  sp_format_signed(builder, *value, 20);
}

void sp_format_u8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u8* value = (u8*)arg->data;
  sp_format_unsigned(builder, *value, 3);
}

void sp_format_u16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u16* value = (u16*)arg->data;
  sp_format_unsigned(builder, *value, 5);
}

void sp_format_u32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u32* value = (u32*)arg->data;
  sp_format_unsigned(builder, *value, 10);
}

void sp_format_u64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u64* value = (u64*)arg->data;
  sp_format_unsigned(builder, *value, 20);
}

void sp_format_f32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  f32* value = (f32*)arg->data;
  f32 num = *value;
  
  // Handle negative
  if (num < 0) {
    sp_str_builder_append_c8(builder, '-');
    num = -num;
  }
  
  // Extract integer part
  s32 integer_part = (s32)num;
  f32 fractional_part = num - integer_part;
  
  // Format integer part
  if (integer_part == 0) {
    sp_str_builder_append_c8(builder, '0');
  } else {
    c8 digits[10];
    s32 digit_count = 0;
    s32 temp = integer_part;
    
    while (temp > 0) {
      digits[digit_count++] = '0' + (temp % 10);
      temp /= 10;
    }
    
    for (s32 i = digit_count - 1; i >= 0; i--) {
      sp_str_builder_append_c8(builder, digits[i]);
    }
  }
  
  // Add decimal point and 3 decimal places
  sp_str_builder_append_c8(builder, '.');
  
  for (s32 i = 0; i < 3; i++) {
    fractional_part *= 10;
    c8 digit = (c8)fractional_part;
    sp_str_builder_append_c8(builder, '0' + digit);
    fractional_part -= digit;
  }
}

void sp_format_f64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  f64* value = (f64*)arg->data;
  f64 num = *value;
  
  // Handle negative
  if (num < 0) {
    sp_str_builder_append_c8(builder, '-');
    num = -num;
  }
  
  // Extract integer part
  s64 integer_part = (s64)num;
  f64 fractional_part = num - integer_part;
  
  // Format integer part
  if (integer_part == 0) {
    sp_str_builder_append_c8(builder, '0');
  } else {
    c8 digits[20];
    s32 digit_count = 0;
    s64 temp = integer_part;
    
    while (temp > 0) {
      digits[digit_count++] = '0' + (temp % 10);
      temp /= 10;
    }
    
    for (s32 i = digit_count - 1; i >= 0; i--) {
      sp_str_builder_append_c8(builder, digits[i]);
    }
  }
  
  // Add decimal point and 3 decimal places
  sp_str_builder_append_c8(builder, '.');
  
  for (s32 i = 0; i < 3; i++) {
    fractional_part *= 10;
    s32 digit = (s32)fractional_part;
    sp_str_builder_append_c8(builder, (c8)('0' + digit));
    fractional_part -= digit;
  }
}

void sp_format_c8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  c8* value = (c8*)arg->data;
  SP_ASSERT(builder);
  SP_ASSERT(value);
  
  sp_str_builder_grow(builder, builder->buffer.count + 3); // two quotes + character
  sp_str_builder_append_c8(builder, '\'');
  sp_str_builder_append_c8(builder, *value);
  sp_str_builder_append_c8(builder, '\'');
}

void sp_format_c16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  c16* value = (c16*)arg->data;
  SP_ASSERT(builder);
  SP_ASSERT(value);
  
  // Grow buffer for worst case: 'U+XXXX' (7 chars)
  sp_str_builder_grow(builder, builder->buffer.count + 7);
  sp_str_builder_append_c8(builder, '\'');
  // Convert c16 to c8 if it's in ASCII range
  if (*value < 128) {
    sp_str_builder_append_c8(builder, (c8)*value);
  } else {
    // Just show the numeric value for non-ASCII
    sp_str_builder_append_c8(builder, 'U');
    sp_str_builder_append_c8(builder, '+');
    sp_format_hex(builder, *value, 4, NULL);
  }
  sp_str_builder_append_c8(builder, '\'');
}

void sp_format_context(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  // Context is passed as pointer but we don't use it
  if (sp_context) {
    u32 index = (u32)(sp_context - sp_context_stack);
    sp_format_unsigned(builder, index, 10);
  } else {
    sp_str_builder_append_cstr(builder, "NULL");
  }
}

void sp_format_hash(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_hash_t* value = (sp_hash_t*)arg->data;
  u64 hash = (u64)*value;
  sp_format_hex(builder, hash, 0, NULL);
}

void sp_format_str_builder(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_str_builder_t* sb = (sp_str_builder_t*)arg->data;
  
  sp_str_builder_append_cstr(builder, "{ buffer: (");
  
  // Format data pointer
  u64 addr = (u64)sb->buffer.data;
  sp_format_hex(builder, addr, 8, "0x");
  
  sp_str_builder_append_cstr(builder, ", ");
  
  // Format count
  sp_format_unsigned(builder, sb->buffer.count, 10);
  
  sp_str_builder_append_cstr(builder, "), capacity: ");
  
  // Format capacity
  sp_format_unsigned(builder, sb->buffer.capacity, 10);
  
  sp_str_builder_append_cstr(builder, " }");
}

void sp_format_date_time(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_os_date_time_t* dt = (sp_os_date_time_t*)arg->data;
  
  // Format year
  sp_format_signed(builder, dt->year, 10);
  
  sp_str_builder_append_c8(builder, '-');
  
  // Format month (2 digits)
  if (dt->month < 10) sp_str_builder_append_c8(builder, '0');
  sp_format_signed(builder, dt->month, 10);
  
  sp_str_builder_append_c8(builder, '-');
  
  // Format day (2 digits)
  if (dt->day < 10) sp_str_builder_append_c8(builder, '0');
  sp_format_signed(builder, dt->day, 10);
  
  sp_str_builder_append_c8(builder, 'T');
  
  // Format hour (2 digits)
  if (dt->hour < 10) sp_str_builder_append_c8(builder, '0');
  sp_format_signed(builder, dt->hour, 10);
  
  sp_str_builder_append_c8(builder, ':');
  
  // Format minute (2 digits)
  if (dt->minute < 10) sp_str_builder_append_c8(builder, '0');
  sp_format_signed(builder, dt->minute, 10);
  
  sp_str_builder_append_c8(builder, ':');
  
  // Format second (2 digits)
  if (dt->second < 10) sp_str_builder_append_c8(builder, '0');
  sp_format_signed(builder, dt->second, 10);
  
  // Add milliseconds if non-zero
  if (dt->millisecond > 0) {
    sp_str_builder_append_c8(builder, '.');
    
    // Format milliseconds (3 digits)
    if (dt->millisecond < 100) sp_str_builder_append_c8(builder, '0');
    if (dt->millisecond < 10) sp_str_builder_append_c8(builder, '0');
    sp_format_signed(builder, dt->millisecond, 10);
  }
}

void sp_format_thread(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_thread_t* thread = (sp_thread_t*)arg->data;
  
  // Thread is a thrd_t which is a struct on Windows, cast to pointer size
  u64 tid = (u64)(uintptr_t)thread;
  
  sp_str_builder_append_cstr(builder, "Thread ");
  sp_format_unsigned(builder, tid, 20);
}

void sp_format_mutex(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_mutex_t* mutex = (sp_mutex_t*)arg->data;
  
  // mtx_t is an opaque type, just show pointer address
  u64 addr = (u64)mutex;
  
  sp_str_builder_append_cstr(builder, "Mutex ");
  sp_format_hex(builder, addr, 8, "0x");
}

void sp_format_semaphore(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_semaphore_t* sem = (sp_semaphore_t*)arg->data;
  
  // sp_semaphore_t is HANDLE on Windows
  u64 handle = (u64)*sem;
  
  sp_str_builder_append_cstr(builder, "Semaphore ");
  sp_format_hex(builder, handle, 8, "0x");
}

void sp_format_fixed_array(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_fixed_array_t* arr = (sp_fixed_array_t*)arg->data;
  
  sp_str_builder_append_cstr(builder, "{ size: ");
  sp_format_unsigned(builder, arr->size, 10);
  sp_str_builder_append_cstr(builder, ", capacity: ");
  sp_format_unsigned(builder, arr->capacity, 10);
  sp_str_builder_append_cstr(builder, " }");
}

void sp_format_dynamic_array(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_dynamic_array_t* arr = (sp_dynamic_array_t*)arg->data;
  
  sp_str_builder_append_cstr(builder, "{ size: ");
  sp_format_unsigned(builder, arr->size, 10);
  sp_str_builder_append_cstr(builder, ", capacity: ");
  sp_format_unsigned(builder, arr->capacity, 10);
  sp_str_builder_append_cstr(builder, " }");
}

sp_str_t sp_fmt(sp_str_t fmt, ...) {
  #define SP_FORMATTER(T, FN) SP_LVAL(sp_formatter_t) { .id = sp_hash_cstr(SP_MACRO_STR(T)), .fn = FN }
  sp_formatter_t formatters [] = { 
    SP_BUILTIN_FORMATTERS
  };

  va_list args;
  va_start(args, fmt); {
    sp_str_builder_t builder = SP_ZERO_INITIALIZE();

    u32 index = 0;
    while (true) {
      if (index >= fmt.len) break;

      if (sp_str_at(fmt, index) == '{') {
        sp_format_arg_t arg = va_arg(args, sp_format_arg_t);
        SP_CARR_FOR(formatters, formatter_index) {
          if (formatters[formatter_index].id == arg.id) {
            sp_format_fn_t fn = formatters[formatter_index].fn;
            fn(&builder, &arg);
            break;
          }
        }
        
        index++;
        index++;

      } 
      else {
        sp_str_builder_append_c8(&builder, sp_str_at(fmt, index));
        index++;
      }      
    }

    va_end(args);
    return sp_str_builder_write(&builder);
  }
}

void sp_context_check_index() {
  u32 index = (u32)(sp_context - sp_context_stack);
  SP_ASSERT(index);
  SP_ASSERT(index < SP_MAX_CONTEXT);
}

void sp_context_set(sp_context_t context) {
  sp_context = sp_context ? sp_context : &sp_context_stack[1];
  *sp_context = context;    
}

void sp_context_push(sp_context_t context) {
  sp_context = sp_context ? sp_context : &sp_context_stack[0];
  sp_context++;
  *sp_context = context;
  sp_context_check_index();
}

void sp_context_push_allocator(sp_allocator_t* allocator) {
  sp_context_t context = SP_ZERO_STRUCT(sp_context_t);
  if (sp_context) context = *sp_context;
  context.allocator = allocator;
  sp_context_push(context);
}

void sp_context_pop() {
  SP_ASSERT(sp_context);
  *sp_context = SP_ZERO_STRUCT(sp_context_t); // Not required, just for the debugger
  sp_context--;
  sp_context_check_index();
}

void* sp_alloc(u32 size) {
  return sp_allocator_alloc(*sp_context->allocator, size);
}

void* sp_realloc(void* memory, u32 size) {
  return sp_allocator_realloc(*sp_context->allocator, memory, size);
}

void sp_free(void* memory) {
  sp_allocator_free(*sp_context->allocator, memory);
}

void* sp_allocator_alloc(sp_allocator_t allocator, u32 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
}

void* sp_allocator_realloc(sp_allocator_t allocator, void* memory, u32 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_RESIZE, size, memory);
}

void sp_allocator_free(sp_allocator_t allocator, void* buffer) {
  allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_FREE, 0, buffer);
}

sp_allocator_t sp_bump_allocator_init(sp_bump_allocator_t* allocator, u32 capacity) {
  allocator->buffer = (u8*)sp_os_allocate_memory(capacity);
  allocator->capacity = capacity;
  allocator->bytes_used = 0;
  
  sp_allocator_t result;
  result.on_alloc = sp_bump_allocator_on_alloc;
  result.user_data = allocator;
  return result;
}

void sp_bump_allocator_clear(sp_bump_allocator_t* allocator) {
  memset(allocator->buffer, 0, allocator->bytes_used);
  allocator->bytes_used = 0;
}

void sp_bump_allocator_destroy(sp_bump_allocator_t* allocator) {
  if (allocator->buffer) {
    sp_os_free_memory(allocator->buffer);
    allocator->buffer = NULL;
    allocator->capacity = 0;
    allocator->bytes_used = 0;
  }
}

void* sp_bump_allocator_on_alloc(void* user_data, sp_allocator_mode_t mode, u32 size, void* old_memory) {
  sp_bump_allocator_t* bump = (sp_bump_allocator_t*)user_data;
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      if (bump->bytes_used + size > bump->capacity) {
        // Resize buffer to at least double the capacity or enough for the allocation
        u32 new_capacity = SP_MAX(bump->capacity * 2, bump->bytes_used + size);
        u8* new_buffer = (u8*)sp_os_reallocate_memory(bump->buffer, new_capacity);
        SP_ASSERT(new_buffer != NULL);
        bump->buffer = new_buffer;
        bump->capacity = new_capacity;
      }
      void* memory_block = bump->buffer + bump->bytes_used;
      bump->bytes_used += size;
      return memory_block;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      return NULL;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      void* memory_block = sp_bump_allocator_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      memcpy(memory_block, old_memory, size);
      return memory_block;
    }
    default: {
      SP_UNREACHABLE();
      return NULL;
    }
  }
}

sp_malloc_metadata_t* sp_malloc_allocator_get_metadata(void* ptr) {
  return ((sp_malloc_metadata_t*)ptr) - 1;
}

void* sp_allocator_malloc_on_alloc(void* user_data, sp_allocator_mode_t mode, u32 size, void* ptr) {
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      u32 total_size = size + sizeof(sp_malloc_metadata_t);
      sp_malloc_metadata_t* metadata = (sp_malloc_metadata_t*)sp_os_allocate_memory(total_size);
      metadata->size = size;
      return metadata + 1;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      if (!ptr) {
        return sp_allocator_malloc_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      }

      sp_malloc_metadata_t* metadata = sp_malloc_allocator_get_metadata(ptr);
      if (metadata->size >= size) {
        return ptr;
      }

      void* buffer = sp_allocator_malloc_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      sp_os_memory_copy(ptr, buffer, metadata->size);
      sp_allocator_malloc_on_alloc(user_data, SP_ALLOCATOR_MODE_FREE, 0, ptr);

      return buffer;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      sp_malloc_metadata_t* metadata = sp_malloc_allocator_get_metadata(ptr);
      sp_os_free_memory(metadata);
      return NULL;
    }
    default: {
      return NULL;
    }
  }
}

sp_allocator_t sp_allocator_malloc_init(sp_allocator_malloc_t* allocator) {
  allocator->allocator.on_alloc = sp_allocator_malloc_on_alloc;
  allocator->allocator.user_data = NULL;
  return allocator->allocator;
}


////////////
// STRING //
////////////
c8* sp_cstr_copy_n(const c8* str, u32 length) {
  u32 buffer_length = length + 1;
  c8* copy = (c8*)sp_alloc(buffer_length);
  sp_cstr_copy_to_n(str, length, copy, buffer_length);
  return copy;
}

c8* sp_cstr_copy(const c8* str) {
  return sp_cstr_copy_n(str, sp_cstr_len(str));
}

c8* sp_cstr_copy_c8(const c8* str, u32 length) {
  return sp_cstr_copy_n((const c8*)str, length);
}

void sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length) {
  sp_cstr_copy_to_n(str, sp_cstr_len(str), buffer, buffer_length);
}

void sp_cstr_copy_to_n(const c8* str, u32 length, c8* buffer, u32 buffer_length) {
  if (!str) return;
  if (!buffer) return;
  if (!buffer_length) return;

  u32 copy_length = SP_MIN(length, buffer_length - 1);
  for (u32 i = 0; i < copy_length; i++) {
    buffer[i] = str[i];
  }
  buffer[copy_length] = '\0';
}

bool sp_cstr_equal(const c8* a, const c8* b) {
  u32 len_a = sp_cstr_len(a);
  u32 len_b = sp_cstr_len(b);
  if (len_a != len_b) return false;

  return sp_os_is_memory_equal(a, b, len_a);
}

u32 sp_cstr_len(const c8* str) {
  u32 len = 0;
  if (str) {
    while (str[len]) len++;
  }

  return len;
}

c8* sp_wstr_to_cstr(c16* str16, u32 len) {
  return sp_os_wstr_to_cstr(str16, len);
}

c8* sp_str_to_cstr(sp_str_t str) {
  return sp_str_to_cstr_ex(str);
}

c8* sp_str_to_cstr_ex(sp_str_t str) {
  return sp_cstr_copy_n((c8*)str.data, str.len);
}

c8* sp_str_to_double_null_terminated(sp_str_t str) {
  c8* buffer = (c8*)sp_alloc(str.len + 2);
  sp_str_copy_to(str, buffer, str.len);
  return (c8*)buffer;
}

sp_str_t sp_str_alloc(u32 capacity) {
  return SP_LVAL(sp_str_t) {
    .len = 0,
    .data = (c8*)sp_alloc(capacity),
  };
}

bool sp_str_equal(sp_str_t a, sp_str_t b) {
  if (a.len != b.len) return false;

  return sp_os_is_memory_equal(a.data, b.data, a.len);
}

bool sp_str_equal_cstr(sp_str_t a, const c8* b) {
  u32 len = sp_cstr_len(b);
  if (a.len != len) return false;

  return sp_os_is_memory_equal(a.data, b, len);
}

s32 sp_str_sort_kernel_alphabetical(const void* a, const void* b) {
  return sp_str_compare_alphabetical(*(sp_str_t*)a, *(sp_str_t*)b);
}

s32 sp_str_compare_alphabetical(sp_str_t a, sp_str_t b) {
  u32 i = 0;
  while (true) {
    if (i >= a.len && i >= b.len) return SP_QSORT_EQUAL;
    if (i >= a.len)               return SP_QSORT_A_FIRST;
    if (i >= b.len)               return SP_QSORT_B_FIRST;
    
    if (a.data[i] == b.data[i]) {
      i++;
      continue;
    }
    if (a.data[i] > b.data[i]) return SP_QSORT_B_FIRST;
    if (b.data[i] > a.data[i]) return SP_QSORT_A_FIRST;
  }

}

bool sp_str_valid(sp_str_t str) {
  return str.data != NULL;
}

c8 sp_str_at(sp_str_t str, u32 index) {
  return str.data[index];
}

sp_str_t sp_str_copy_cstr_n(const c8* str, u32 length) {
  sp_str_t copy;
  u32 str_len = sp_cstr_len(str);
  copy.len = SP_MIN(str_len, length);
  copy.data = (c8*)sp_alloc(copy.len);

  sp_os_memory_copy(str, copy.data, copy.len);
  return copy;
}

sp_str_t sp_str_copy_cstr(const c8* str) {
  sp_str_t copy;
  copy.len = sp_cstr_len(str);
  copy.data = (c8*)sp_alloc(copy.len);

  sp_os_memory_copy(str, copy.data, copy.len);
  return copy;
}

sp_str_t sp_str_copy(sp_str_t str) {
  sp_str_t copy = {
    .len = str.len,
    .data = (c8*)sp_alloc(str.len),
  };

  sp_os_memory_copy(str.data, copy.data, str.len);
  return copy;
}

void sp_str_copy_to_str(sp_str_t source, sp_str_t* dest, u32 capacity) {
  dest->len = SP_MIN(source.len, capacity);
  sp_os_memory_copy(source.data, dest->data, dest->len);
}

void sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity) {
  sp_os_memory_copy(str.data, buffer, SP_MIN(str.len, capacity));
}

void sp_str_builder_grow(sp_str_builder_t* builder, u32 requested_capacity) {
  while (builder->buffer.capacity < requested_capacity) {
    u32 capacity = SP_MAX(builder->buffer.capacity * 2, 8);
    builder->buffer.data = (c8*)sp_realloc(builder->buffer.data, capacity);
    builder->buffer.capacity = capacity;
  }
}

void sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str) {
  sp_str_builder_grow(builder, builder->buffer.count + str.len);
  sp_os_memory_copy(str.data, builder->buffer.data + builder->buffer.count, str.len);
  builder->buffer.count += str.len;
}

void sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str) {
  u32 len = sp_cstr_len(str);
  sp_str_builder_grow(builder, builder->buffer.count + len);
  sp_os_memory_copy(str, builder->buffer.data + builder->buffer.count, len);
  builder->buffer.count += len;
}

void sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c) {
  sp_str_builder_append(builder, sp_str(&c, 1));
}

sp_str_t sp_str_builder_write(sp_str_builder_t* builder) {
  sp_str_t string = {
    .len = builder->buffer.count,
    .data = (c8*)sp_alloc(builder->buffer.count),
  };

  sp_os_memory_copy(builder->buffer.data, string.data, builder->buffer.count);
  return string;
}

c8* sp_str_builder_write_cstr(sp_str_builder_t* builder) {
  return sp_cstr_copy_n((c8*)builder->buffer.data, builder->buffer.count);
}

////////
// OS //
////////
void sp_os_normalize_path(sp_str_t path) {
  for (u32 i = 0; i < path.len; i++) {
    if (path.data[i] == '\\') {
      path.data[i] = '/';
    }
  }
}

sp_str_t sp_os_extract_file_name(sp_str_t path) {
  if (path.len == 0) return sp_str_lit("");
  
  c8* last_slash = NULL;
  for (u32 i = 0; i < path.len; i++) {
    if (path.data[i] == '/' || path.data[i] == '\\') {
      last_slash = &path.data[i];
    }
  }
  
  if (!last_slash) {
    return path;
  }
  
  u32 filename_start = (u32)(last_slash - path.data) + 1;
  if (filename_start >= path.len) return sp_str_lit("");
  
  u32 filename_len = path.len - filename_start;
  return sp_str(path.data + filename_start, filename_len);
}


sp_str_t sp_os_parent_path(sp_str_t path) {
  if (path.len == 0) return path;
  
  // Start from the end of the string
  c8* c = path.data + path.len - 1;
  
  // Skip any trailing slashes
  while (c > path.data && *c == '/') {
    c--;
  }
  
  // Now find the next slash
  while (c > path.data && *c != '/') {
    c--;
  }
  
  // If we found a slash and it's not the only character, exclude it
  if (c > path.data) {
    path.len = (u32)(c - path.data);
  } else if (c == path.data && *c == '/') {
    // Root path case: "/" -> ""
    path.len = 0;
  } else {
    // No parent found
    path.len = 0;
  }
  
  return path;
}

sp_str_t sp_os_path_extension(sp_str_t path) {
  if (!sp_os_is_regular_file(path)) {
    return sp_str_lit("");
  }

  c8* c = path.data + path.len;
  while (true) {
    if ((*c == '/') || (c == path.data)) {
      return sp_str_lit("");
    }

    if (*c == '.') break;
    c--;
  }

  return sp_str(c, (path.data + path.len) - (c));
}

#ifdef _WIN32
  void* sp_os_allocate_memory(u32 size) {
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
  }

  void* sp_os_reallocate_memory(void* ptr, u32 size) {
    if (!ptr) return sp_os_allocate_memory(size);
    if (!size) { HeapFree(GetProcessHeap(), 0, ptr); return NULL; }
    return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
  }

  void sp_os_free_memory(void* ptr) {
    if (!ptr) return;
    HeapFree(GetProcessHeap(), 0, ptr);
  }

  bool sp_os_does_path_exist(sp_str_t path) {
    sp_win32_dword_t attributes = GetFileAttributesA(sp_str_to_cstr(path));
    return (attributes != INVALID_FILE_ATTRIBUTES);
  }

  bool sp_os_is_regular_file(sp_str_t path) {
    sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
    if (attribute == INVALID_FILE_ATTRIBUTES) return false;
    return !(attribute & FILE_ATTRIBUTE_DIRECTORY);
  }

  bool sp_os_is_directory(sp_str_t path) {
    sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
    if (attribute == INVALID_FILE_ATTRIBUTES) return false;
    return attribute & FILE_ATTRIBUTE_DIRECTORY;
  }

  void sp_os_remove_directory(sp_str_t path) {
    SHFILEOPSTRUCTA file_op = SP_ZERO_INITIALIZE();
    file_op.wFunc = FO_DELETE;
    file_op.pFrom = sp_str_to_double_null_terminated(path);
    file_op.fFlags = FOF_NO_UI;
    SHFileOperationA(&file_op);
  }

  void sp_os_create_directory(sp_str_t path) {
    CreateDirectoryA(sp_str_to_cstr(path), NULL);
  }

  void sp_os_create_file(sp_str_t path) {
    sp_win32_handle_t handle = CreateFileA(sp_str_to_cstr(path), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(handle);
  }

  void sp_os_remove_file(sp_str_t path) {
    DeleteFileA(sp_str_to_cstr(path));
  }

  sp_os_directory_entry_list_t sp_os_scan_directory(sp_str_t path) {
    if (!sp_os_is_directory(path) || !sp_os_does_path_exist(path)) {
      return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
    }

    sp_dynamic_array(sp_os_directory_entry_t) entries;
    sp_dynamic_array_init(&entries, sizeof(sp_os_directory_entry_t));

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append(&builder, path);
    sp_str_builder_append(&builder, sp_str_lit("/*"));
    sp_str_t glob = sp_str_builder_write(&builder);

    sp_win32_find_data_t find_data;
    sp_win32_handle_t handle = FindFirstFile(sp_str_to_cstr(glob), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
      return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
    }

    do {
      if (sp_cstr_equal(find_data.cFileName, ".")) continue;
      if (sp_cstr_equal(find_data.cFileName, "..")) continue;
      
      sp_str_builder_t entry_builder = SP_ZERO_INITIALIZE();
      sp_str_builder_append(&entry_builder, path);
      sp_str_builder_append(&entry_builder, sp_str_lit("/"));
      sp_str_builder_append_cstr(&entry_builder, find_data.cFileName);
      sp_str_t file_path = sp_str_builder_write(&entry_builder);
      sp_os_normalize_path(file_path);

      sp_os_directory_entry_t entry  = SP_LVAL(sp_os_directory_entry_t) {
        .file_path = file_path,
        .file_name = sp_str_copy_cstr(find_data.cFileName),
        .attributes = sp_os_winapi_attr_to_sp_attr(GetFileAttributesA(sp_str_to_cstr(file_path))),
      };
      sp_dynamic_array_push(&entries, &entry);
    } while (FindNextFile(handle, &find_data));

    FindClose(handle);

    return SP_LVAL(sp_os_directory_entry_list_t) {
      .data = (sp_os_directory_entry_t*)entries.data,
      .count = entries.size
    };
  }

  sp_os_directory_entry_list_t sp_os_scan_directory_recursive(sp_str_t path) {
    SP_BROKEN();
    return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
  }

  sp_os_date_time_t sp_os_get_date_time() {
    SP_BROKEN();
    return SP_ZERO_STRUCT(sp_os_date_time_t);
  }

  sp_precise_epoch_time_t sp_os_file_mod_time_precise(sp_str_t file_path) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(sp_str_to_cstr(file_path), GetFileExInfoStandard, &fad)) {
      return SP_ZERO_STRUCT(sp_precise_epoch_time_t);
    }

    if (fad.nFileSizeHigh == 0 && fad.nFileSizeLow == 0) {
      return SP_ZERO_STRUCT(sp_precise_epoch_time_t);
    }

    LARGE_INTEGER time;
    time.HighPart = fad.ftLastWriteTime.dwHighDateTime;
    time.LowPart = fad.ftLastWriteTime.dwLowDateTime;

    // Convert to Unix epoch
    u64 unix_100ns = time.QuadPart - 116444736000000000LL;
    
    return SP_LVAL(sp_precise_epoch_time_t) {
      unix_100ns / 10000000,           // seconds
      (unix_100ns % 10000000) * 100    // remainder to nanoseconds
    };
  }

  sp_os_file_attr_t sp_os_winapi_attr_to_sp_attr(u32 attr) {
    u32 result = SP_OS_FILE_ATTR_NONE;
    if ( (attr & FILE_ATTRIBUTE_DIRECTORY)) result |= SP_OS_FILE_ATTR_DIRECTORY;
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) result |= SP_OS_FILE_ATTR_REGULAR_FILE;
    return (sp_os_file_attr_t)result;
  }

  bool sp_os_is_memory_equal(const void* a, const void* b, size_t len) {
    return !memcmp(a, b, len);
  }

  void sp_os_memory_copy(const void* source, void* dest, u32 num_bytes) {
    memcpy(dest, source, num_bytes);
  }

  void sp_os_fill_memory(void* buffer, u32 buffer_size, void* fill, u32 fill_size) {
    u8* current_byte = (u8*)buffer;

    s32 i = 0;
    while (true) {
      if (i + fill_size > buffer_size) return;
      memcpy(current_byte + i, (u8*)fill, fill_size);
      i += fill_size;
    }
  }

  void sp_os_fill_memory_u8(void* buffer, u32 buffer_size, u8 fill) {
  sp_os_fill_memory(buffer, buffer_size, &fill, sizeof(u8));
  }

  void sp_os_zero_memory(void* buffer, u32 buffer_size) {
  sp_os_fill_memory_u8(buffer, buffer_size, 0);
  }

  void sp_os_sleep_ms(f64 ms) {
    Sleep((DWORD)ms);
  }

  sp_str_t sp_os_get_executable_path() {
    c8 exe_path[SP_MAX_PATH_LEN];
    GetModuleFileNameA(NULL, exe_path, SP_MAX_PATH_LEN);
    
    sp_str_t exe_path_str = sp_str_cstr(exe_path);
    sp_os_normalize_path(exe_path_str);
    
    return sp_str_copy(exe_path_str);
  }

  sp_str_t sp_os_canonicalize_path(sp_str_t path) {
    c8* path_cstr = sp_str_to_cstr(path);
    c8 canonical_path[SP_MAX_PATH_LEN];
    
    if (GetFullPathNameA(path_cstr, SP_MAX_PATH_LEN, canonical_path, NULL) == 0) {
      sp_free(path_cstr);
      return sp_str_copy(path);
    }
    
    sp_free(path_cstr);
    
    sp_str_t result = sp_str_cstr(canonical_path);
    sp_os_normalize_path(result);
    
    // Remove trailing slash if present
    if (result.len > 0 && result.data[result.len - 1] == '/') {
      result.len--;
    }
    
    return sp_str_copy(result);
  }

  c8* sp_os_wstr_to_cstr(c16* str16, u32 len) {
    s32 size_needed = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)str16, (s32)len, NULL, 0, NULL, NULL);
    c8* str8 = (c8*)sp_alloc((u32)(size_needed + 1));
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)str16, (s32)len, str8, size_needed, NULL, NULL);
    str8[size_needed] = '\0';
    return str8;
  }
#endif // _WIN32

#ifdef __APPLE__
  void* sp_os_allocate_memory(u32 size) {
    void* ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
  }

  void* sp_os_reallocate_memory(void* ptr, u32 size) {
    if (!ptr) return sp_os_allocate_memory(size);
    if (!size) { free(ptr); return NULL; }
    return realloc(ptr, size);
  }

  void sp_os_free_memory(void* ptr) {
    if (!ptr) return;
    free(ptr);
  }

  bool sp_os_is_memory_equal(const void* a, const void* b, size_t len) {
    return !memcmp(a, b, len);
  }

  void sp_os_memory_copy(const void* source, void* dest, u32 num_bytes) {
    memcpy(dest, source, num_bytes);
  }

  void sp_os_fill_memory(void* buffer, u32 buffer_size, void* fill, u32 fill_size) {
    u8* current_byte = (u8*)buffer;

    s32 i = 0;
    while (true) {
      if (i + fill_size > buffer_size) return;
      memcpy(current_byte + i, (u8*)fill, fill_size);
      i += fill_size;
    }
  }

  void sp_os_fill_memory_u8(void* buffer, u32 buffer_size, u8 fill) {
  sp_os_fill_memory(buffer, buffer_size, &fill, sizeof(u8));
  }

  void sp_os_zero_memory(void* buffer, u32 buffer_size) {
  sp_os_fill_memory_u8(buffer, buffer_size, 0);
  }

  bool sp_os_does_path_exist(sp_str_t path) {
    (void)path;
    return false; // Minimal implementation for now
  }

  bool sp_os_is_regular_file(sp_str_t path) {
    (void)path;
    return false; // Minimal implementation for now
  }

  bool sp_os_is_directory(sp_str_t path) {
    (void)path;
    return false; // Minimal implementation for now
  }

  void sp_os_remove_directory(sp_str_t path) {
    (void)path; // Minimal implementation for now
  }

  void sp_os_create_directory(sp_str_t path) {
    (void)path; // Minimal implementation for now
  }

  void sp_os_create_file(sp_str_t path) {
    (void)path; // Minimal implementation for now
  }

  void sp_os_remove_file(sp_str_t path) {
    (void)path; // Minimal implementation for now
  }

  sp_os_directory_entry_list_t sp_os_scan_directory(sp_str_t path) {
    (void)path;
    return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
  }

  sp_os_directory_entry_list_t sp_os_scan_directory_recursive(sp_str_t path) {
    (void)path;
    return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
  }

  sp_os_date_time_t sp_os_get_date_time() {
    return SP_ZERO_STRUCT(sp_os_date_time_t);
  }

  sp_precise_epoch_time_t sp_os_file_mod_time_precise(sp_str_t file_path) {
    (void)file_path;
    return SP_ZERO_STRUCT(sp_precise_epoch_time_t);
  }

  void sp_os_sleep_ms(f64 ms) {
    (void)ms; // Minimal implementation for now
  }

  sp_str_t sp_os_get_executable_path() {
    return sp_str_lit(""); // Minimal implementation for now
  }

  sp_str_t sp_os_canonicalize_path(sp_str_t path) {
    return sp_str_copy(path); // Minimal implementation for now
  }

  c8* sp_os_wstr_to_cstr(c16* str16, u32 len) {
    if (!str16 || len == 0) {
      c8* result = (c8*)sp_alloc(1);
      result[0] = '\0';
      return result;
    }
    
    // Simple conversion for ASCII characters
    c8* result = (c8*)sp_alloc(len + 1);
    for (u32 i = 0; i < len; i++) {
      if (str16[i] < 128) {
        result[i] = (c8)str16[i];
      } else {
        result[i] = '?';  // Replace non-ASCII with '?'
      }
    }
    result[len] = '\0';
    return result;
  }
#endif // __APPLE__



///////////////////
// DYNAMIC ARRAY //
///////////////////
void sp_dynamic_array_init(sp_dynamic_array_t* arr, u32 element_size) {
  SP_ASSERT(arr);

  arr->size = 0;
  arr->capacity = 2;
  arr->element_size = element_size;
  arr->data = (u8*)sp_alloc(arr->capacity * element_size);
}

u8* sp_dynamic_array_at(sp_dynamic_array_t* arr, u32 index) {
  SP_ASSERT(arr);
  return arr->data + (index * arr->element_size);
}

u8*  sp_dynamic_array_push(sp_dynamic_array_t* arr, void* data) {
  return sp_dynamic_array_push_n(arr, data, 1);
}

u8* sp_dynamic_array_push_n(sp_dynamic_array_t* arr, void* data, u32 count) {
  SP_ASSERT(arr);

  u8* reserved = sp_dynamic_array_reserve(arr, count);
  if (data) sp_os_memory_copy(data, reserved, arr->element_size * count);
  return reserved;
}

u8* sp_dynamic_array_reserve(sp_dynamic_array_t* arr, u32 count) {
  SP_ASSERT(arr);

  sp_dynamic_array_grow(arr, arr->size + count);

  u8* element = sp_dynamic_array_at(arr, arr->size);
  arr->size += count;
  return element;
}

void sp_dynamic_array_clear(sp_dynamic_array_t* arr) {
  SP_ASSERT(arr);

  arr->size = 0;
}

u32 sp_dynamic_array_byte_size(sp_dynamic_array_t* arr) {
  SP_ASSERT(arr);

  return arr->size * arr->element_size;
}

void sp_dynamic_array_grow(sp_dynamic_array_t* arr, u32 capacity) {
  SP_ASSERT(arr);

  if (arr->capacity >= capacity) return;
  arr->capacity = SP_MAX(arr->capacity * 2, capacity);
  arr->data = (u8*)sp_realloc(arr->data, arr->capacity * arr->element_size);
}


/////////////////
// FIXED ARRAY //
/////////////////
void sp_fixed_array_init(sp_fixed_array_t* buffer, u32 max_vertices, u32 element_size) {
  SP_ASSERT(buffer);

  buffer->size = 0;
  buffer->capacity = max_vertices;
  buffer->element_size = element_size;
  buffer->data = (u8*)sp_alloc(max_vertices * element_size);
}

u8* sp_fixed_array_at(sp_fixed_array_t* buffer, u32 index) {
  SP_ASSERT(buffer);
  return buffer->data + (index * buffer->element_size);
}

u8* sp_fixed_array_push(sp_fixed_array_t* buffer, void* data, u32 count) {
  SP_ASSERT(buffer);
  SP_ASSERT(buffer->size < buffer->capacity);

  u8* reserved = sp_fixed_array_reserve(buffer, count);
  if (data) sp_os_memory_copy(data, reserved, buffer->element_size * count);
  return reserved;
}

u8* sp_fixed_array_reserve(sp_fixed_array_t* buffer, u32 count) {
  SP_ASSERT(buffer);

  u8* element = sp_fixed_array_at(buffer, buffer->size);
  buffer->size += count;
  return element;
}

void sp_fixed_array_clear(sp_fixed_array_t* buffer) {
  SP_ASSERT(buffer);

  buffer->size = 0;
}

u32 sp_fixed_array_byte_size(sp_fixed_array_t* buffer) {
  SP_ASSERT(buffer);

  return buffer->size * buffer->element_size;
}

///////////////
// THREADING //
///////////////
#ifdef _WIN32
  void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
    sp_thread_launch_t launch = SP_LVAL(sp_thread_launch_t) {
      .fn = fn,
      .userdata = userdata,
      .context = *sp_context,
      .semaphore = SP_ZERO_STRUCT(sp_semaphore_t)
    };
    sp_semaphore_init(&launch.semaphore);
  
    thrd_create(thread, sp_thread_launch, &launch);
    sp_semaphore_wait(&launch.semaphore);
  }
  
  s32 sp_thread_launch(void* args) {
    sp_thread_launch_t* launch = (sp_thread_launch_t*)args;
    sp_context_push(launch->context);
    void* userdata = launch->userdata;
    sp_thread_fn_t fn = launch->fn;
    sp_semaphore_signal(&launch->semaphore);
    return fn(userdata);
  }
  
  void sp_thread_join(sp_thread_t* thread) {
    s32 result = 0;
    thrd_join(*thread, &result);
  }
  
  s32 sp_mutex_kind_to_c11(sp_mutex_kind_t kind) {
    s32 c11_kind;
    if (kind & SP_MUTEX_PLAIN) c11_kind = mtx_plain; 
    if (kind & SP_MUTEX_TIMED) c11_kind = mtx_timed; 
    if (kind & SP_MUTEX_RECURSIVE) c11_kind |= mtx_recursive; 
  
    return c11_kind;
  } 
  
  void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
    mtx_init(mutex, sp_mutex_kind_to_c11(kind));
  }
  
  void sp_mutex_lock(sp_mutex_t* mutex) {
    mtx_lock(mutex);
  }
  
  void sp_mutex_unlock(sp_mutex_t* mutex) {
    mtx_unlock(mutex);
  }
  
  void sp_mutex_destroy(sp_mutex_t* mutex) {
    mtx_destroy(mutex);
  }
      
  
  void sp_semaphore_init(sp_semaphore_t* semaphore) {
    *semaphore = CreateSemaphoreW(NULL, 0, 1, NULL); 
  }
  
  void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
    CloseHandle(*semaphore);
  }
  
  void sp_semaphore_wait(sp_semaphore_t* semaphore) {
    WaitForSingleObject(*semaphore, INFINITE);
  }    
   
  
  bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
    sp_win32_dword_t result = WaitForSingleObject(*semaphore, ms);
    return result == WAIT_OBJECT_0;
  }    
  
  void sp_semaphore_signal(sp_semaphore_t* semaphore) {
    ReleaseSemaphore(*semaphore, 1, NULL);
  }  
  
  void sp_os_file_monitor_init(sp_file_monitor_t* monitor) {
    sp_os_win32_file_monitor_t* os = (sp_os_win32_file_monitor_t*)sp_alloc(sizeof(sp_os_win32_file_monitor_t));
    sp_dynamic_array_init(&os->directory_infos, sizeof(sp_monitored_dir_t));
    monitor->os = os;
  }
  
  void sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t directory_path) {
    sp_os_win32_file_monitor_t* os = (sp_os_win32_file_monitor_t*)monitor->os;
  
    sp_win32_handle_t event = CreateEventW(NULL, false, false, NULL);
    if (!event) return;
    
    c8* directory_cstr = sp_str_to_cstr(directory_path);
    sp_win32_handle_t handle = CreateFileA(
      directory_cstr, 
      FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      NULL
    );
    
    if (handle == INVALID_HANDLE_VALUE) {
      CloseHandle(event);
      sp_free(directory_cstr);
      return;
    }
  
    sp_monitored_dir_t* info = (sp_monitored_dir_t*)sp_dynamic_array_push(&os->directory_infos, NULL);
    sp_os_zero_memory(&info->overlapped, sizeof(sp_win32_overlapped_t));
    info->overlapped.hEvent = event;
    info->handle = handle;
    info->path = sp_str_copy(directory_path);
    info->notify_information = sp_alloc(SP_FILE_MONITOR_BUFFER_SIZE);
    sp_os_zero_memory(info->notify_information, SP_FILE_MONITOR_BUFFER_SIZE);
  
    sp_os_win32_file_monitor_issue_one_read(monitor, info);
    sp_free(directory_cstr);
  }
  
  void sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
  }
  
  void sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor) {
    sp_os_win32_file_monitor_t* os = (sp_os_win32_file_monitor_t*)monitor->os;
  
    for (u32 i = 0; i < os->directory_infos.size; i++) {
      sp_monitored_dir_t* info = (sp_monitored_dir_t*)sp_dynamic_array_at(&os->directory_infos, i);
      assert(info->handle != INVALID_HANDLE_VALUE);
  
      if (!HasOverlappedIoCompleted(&info->overlapped)) continue;
  
      s32 bytes_written = 0;
      bool success = GetOverlappedResult(info->handle, &info->overlapped, (LPDWORD) &bytes_written, false);
      if (!success || bytes_written == 0) break;
  
      FILE_NOTIFY_INFORMATION* notify = (FILE_NOTIFY_INFORMATION*)info->notify_information;
      while (true) {
        sp_file_change_event_t events = SP_FILE_CHANGE_EVENT_NONE;
        if (notify->Action == FILE_ACTION_MODIFIED) {
          events = SP_FILE_CHANGE_EVENT_MODIFIED;
        }
        else if (notify->Action == FILE_ACTION_ADDED) {
          events = SP_FILE_CHANGE_EVENT_ADDED;
        }
        else if (notify->Action == FILE_ACTION_REMOVED) {
          events = SP_FILE_CHANGE_EVENT_REMOVED;
        }
        else if (notify->Action == FILE_ACTION_RENAMED_OLD_NAME) {
          
        }
        else if (notify->Action == FILE_ACTION_RENAMED_NEW_NAME) {
  
        }
        else {
          continue;
        }
  
        c8* partial_path_cstr = sp_wstr_to_cstr(&notify->FileName[0], (u32)(notify->FileNameLength / 2));
        sp_str_t partial_path_str = sp_str_cstr(partial_path_cstr);
        
        sp_str_builder_t builder = SP_ZERO_INITIALIZE();
        sp_str_builder_append(&builder, info->path);
        sp_str_builder_append(&builder, sp_str_lit("/"));
        sp_str_builder_append(&builder, partial_path_str);
        sp_str_t full_path = sp_str_builder_write(&builder);
        
        sp_os_normalize_path(full_path);
  
        sp_str_t file_name = sp_os_extract_file_name(full_path);
        sp_os_win32_file_monitor_add_change(monitor, full_path, file_name, events);
        
        sp_free(partial_path_cstr);
  
        if (notify->NextEntryOffset == 0) break;
        notify = (FILE_NOTIFY_INFORMATION*)((char*)notify + notify->NextEntryOffset);
      }
  
      sp_os_win32_file_monitor_issue_one_read(monitor, info);
    }
  
    sp_file_monitor_emit_changes(monitor);
  }
  
  void sp_os_win32_file_monitor_add_change(sp_file_monitor_t* monitor, sp_str_t file_path, sp_str_t file_name, sp_file_change_event_t events) {
    f32 time = (f32)(GetTickCount64() / 1000.0);
  
    if (sp_os_is_directory(file_path)) return;
  
    if (file_name.data && file_name.len > 0) {
      if (file_name.data[0] == '.' && file_name.len > 1 && file_name.data[1] == '#') return;
      if (file_name.data[0] ==  '#') return;
    }
  
    if (!sp_file_monitor_check_cache(monitor, file_path, time)) return;
  
    for (u32 i = 0; i < monitor->changes.size; i++) {
      sp_file_change_t* change = (sp_file_change_t*)sp_dynamic_array_at(&monitor->changes, i);
      if (sp_str_equal(change->file_path, file_path)) {
        if (monitor->debounce_time_ms > 0) {
          f32 time_diff_ms = (time - change->time) * 1000.0f;
          if (time_diff_ms < (f32)monitor->debounce_time_ms) {
            return;
          }
        }
        change->events = (sp_file_change_event_t)(change->events | events);
        change->time = time;
        return;
      }
    }
  
    sp_file_change_t* change = (sp_file_change_t*)sp_dynamic_array_push(&monitor->changes, NULL);
    change->file_path = sp_str_copy(file_path);
    change->file_name = sp_str_copy(file_name);
    change->events = events;
    change->time = time;
  }
  
  void sp_os_win32_file_monitor_issue_one_read(sp_file_monitor_t* monitor, sp_monitored_dir_t* info) {
    SP_ASSERT(info->handle != INVALID_HANDLE_VALUE);
  
    s32 notify_filter = 0;
    if (monitor->events_to_watch & (SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_REMOVED)) {
      notify_filter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION;
    }
    if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) {
      notify_filter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
    }
  
    info->bytes_returned = 0;
  
    ReadDirectoryChangesW(info->handle, info->notify_information, SP_FILE_MONITOR_BUFFER_SIZE, true, notify_filter, NULL, &info->overlapped, NULL);
  }
#endif

#ifdef __APPLE__
  void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
    (void)thread; (void)fn; (void)userdata;
  }
  
  s32 sp_thread_launch(void* args) {
    (void)args;
    return 0;
  }
  
  void sp_thread_join(sp_thread_t* thread) {
    (void)thread;
  }
  
  void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
    (void)mutex; (void)kind;
  }
  
  void sp_mutex_lock(sp_mutex_t* mutex) {
    (void)mutex;
  }
  
  void sp_mutex_unlock(sp_mutex_t* mutex) {
    (void)mutex;
  }
  
  void sp_mutex_destroy(sp_mutex_t* mutex) {
    (void)mutex;
  }
  
  void sp_semaphore_init(sp_semaphore_t* semaphore) {
    (void)semaphore;
  }
  
  void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
    (void)semaphore;
  }
  
  void sp_semaphore_wait(sp_semaphore_t* semaphore) {
    (void)semaphore;
  }
  
  bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
    (void)semaphore; (void)ms;
    return true;
  }
  
  void sp_semaphore_signal(sp_semaphore_t* semaphore) {
    (void)semaphore;
  }
  
  void sp_os_file_monitor_init(sp_file_monitor_t* monitor) {
    (void)monitor;
  }
  
  void sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t directory_path) {
    (void)monitor; (void)directory_path;
  }
  
  void sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
    (void)monitor; (void)file_path;
  }
  
  void sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor) {
    (void)monitor;
  }
#endif // __APPLE__

//////////////////
// FILE MONITOR //
//////////////////
void sp_file_monitor_init(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata) {
  sp_file_monitor_init_debounce(monitor, callback, events, userdata, 0);
}

void sp_file_monitor_init_debounce(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata, u32 debounce_ms) {
  monitor->callback = callback;
  monitor->events_to_watch = events;
  monitor->userdata = userdata;
  monitor->debounce_time_ms = debounce_ms;
  sp_dynamic_array_init(&monitor->changes, sizeof(sp_file_change_t));
  sp_dynamic_array_init(&monitor->cache, sizeof(sp_cache_entry_t));
  sp_os_file_monitor_init(monitor);
}

void sp_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path) {
  sp_os_file_monitor_add_directory(monitor, path);
}

void sp_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
  sp_os_file_monitor_add_file(monitor, file_path);
}

void sp_file_monitor_process_changes(sp_file_monitor_t* monitor) {
  sp_os_file_monitor_process_changes(monitor);
}

void sp_file_monitor_emit_changes(sp_file_monitor_t* monitor) {
  for (u32 i = 0; i < monitor->changes.size; i++) {
    sp_file_change_t* change = (sp_file_change_t*)sp_dynamic_array_at(&monitor->changes, i);
    monitor->callback(monitor, change, monitor->userdata);
  }

  sp_dynamic_array_clear(&monitor->changes);
}

sp_cache_entry_t* sp_file_monitor_find_cache_entry(sp_file_monitor_t* monitor, sp_str_t file_path) {
  c8* file_path_cstr = sp_str_to_cstr(file_path);
  sp_hash_t file_hash = sp_hash_cstr(file_path_cstr);
  sp_free(file_path_cstr);
  
  sp_cache_entry_t* found = NULL;
  for (u32 i = 0; i < monitor->cache.size; i++) {
    sp_cache_entry_t* entry = (sp_cache_entry_t*)sp_dynamic_array_at(&monitor->cache, i);
    if (entry->hash == file_hash) {
      found = entry;
      break;
    }
  }

  if (!found) {
    found = (sp_cache_entry_t*)sp_dynamic_array_push(&monitor->cache, NULL);
    found->hash = file_hash;
  }
  
  return found;
}

bool sp_file_monitor_check_cache(sp_file_monitor_t* monitor, sp_str_t file_path, f64 time) {
  sp_cache_entry_t* entry = sp_file_monitor_find_cache_entry(monitor, file_path);
  f64 delta = time - entry->last_event_time;
  entry->last_event_time = time;

  return delta > (monitor->debounce_time_ms / 1000.0);
}

sp_hash_t sp_hash_cstr(const c8* str) {
  const size_t prime = 31;

  sp_hash_t hash = 0;
  c8 c = 0;
  
  while ((c = *str++)) {
    hash = c + (hash * prime);
  }

  return hash;
}

SP_END_EXTERN_C()

#ifdef __cplusplus
sp_str_t operator/(const sp_str_t& a, const sp_str_t& b) {
  sp_str_builder_t builder = {0};
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append(&builder, b);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_os_normalize_path(result);
  return result;
}

sp_str_t operator/(const sp_str_t& a, const c8* b) {
  sp_str_builder_t builder = {0};
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append_cstr(&builder, b);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_os_normalize_path(result);
  return result;
}
#endif

#endif // SP_IMPLEMENTATION
