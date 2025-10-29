#ifndef SP_SPACE_H
#define SP_SPACE_H

//  ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗██║   ██║██████╔╝███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║██║   ██║██╔══██╗██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

////////////////////////
// PLATFORM SELECTION //
////////////////////////
#ifdef _WIN32
  #define SP_WIN32
#endif

#ifdef __APPLE__
  #define SP_MACOS
  #define SP_POSIX
#endif

#ifdef __linux__
  #define SP_LINUX
  #define SP_POSIX
#endif


////////////////////////
// COMPILER SELECTION //
////////////////////////
#if defined(_MSC_VER)
  #define SP_MSVC
#endif

#if defined(__TINYC__)
  #define SP_TCC
#elif defined(__clang__)
  #define SP_CLANG
  #define SP_GNUISH
#elif defined(__GNUC__) && !defined(SP_CLANG)
  #define SP_GCC
  #define SP_GNUISH
#endif


////////////////////////////
// ARCHITECTURE SELECTION //
////////////////////////////
#if defined(__x86_64__) || defined(_M_X64)
  #define SP_AMD64
  #define SP_AMD
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
  #define SP_ARM64
  #define SP_ARM
#endif

////////////////////////
// LANGUAGE SELECTION //
////////////////////////
#ifdef __cplusplus
  #define SP_CPP
#endif


//////////////////////
// PLATFORM HEADERS //
//////////////////////
#ifdef SP_WIN32
  #undef UNICODE
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #define _CRT_RAND_S

  #ifdef SP_WIN32_NO_WINDOWS_H
    #include "windef.h"
    #include "winbase.h"
    #include "wingdi.h"
    #include "winuser.h"
    #include "synchapi.h"
    #include "fileapi.h"
    #include "handleapi.h"
    #include "shellapi.h"
    #include "stringapiset.h"
    #include "threads.h"
  #else
    #include "windows.h"
    #include "shlobj.h"
    #include "commdlg.h"
    #include "shellapi.h"
    #include "threads.h"
  #endif
#endif

#ifdef SP_MACOS
  #ifndef _DARWIN_SOURCE
    #define _DARWIN_SOURCE
  #endif

  #include "pthread.h"
  #include <dispatch/dispatch.h>
  #include <mach-o/dyld.h>
  #include <spawn.h>
#endif

#ifdef SP_LINUX
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif

  #include "pthread.h"
  #include <sys/inotify.h>
  #include <poll.h>
  #include <spawn.h>
#endif

#ifdef SP_POSIX
  #ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
  #endif

  #include <dirent.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <limits.h>
  #include <pthread.h>
  #include <semaphore.h>
  #include <signal.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <time.h>
#endif

#ifdef SP_CPP
  #include <atomic>
#endif

#include "assert.h"
#include "stdarg.h"
#include "stdbool.h"
#include "string.h"

#if defined(SP_POSIX) && defined(strnlen)
  #undef SP_STRNLEN
#endif

extern char** environ;

// ███╗   ███╗ █████╗  ██████╗██████╗  ██████╗ ███████╗
// ████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔═══██╗██╔════╝
// ██╔████╔██║███████║██║     ██████╔╝██║   ██║███████╗
// ██║╚██╔╝██║██╔══██║██║     ██╔══██╗██║   ██║╚════██║
// ██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║╚██████╔╝███████║
// ╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝
#ifdef SP_CPP
  #define SP_RVAL(T) (T)
  #define SP_THREAD_LOCAL thread_local
  #define SP_BEGIN_EXTERN_C() extern "C" {
  #define SP_END_EXTERN_C() }
  #define SP_ZERO_INITIALIZE() {}
  #define SP_NULL 0
  #define SP_NULLPTR nullptr
#else
  #define SP_RVAL(T) (T)
  #define SP_THREAD_LOCAL _Thread_local
  #define SP_BEGIN_EXTERN_C()
  #define SP_END_EXTERN_C()
  #define SP_ZERO_INITIALIZE() {0}
  #define SP_NULL 0
  #define SP_NULLPTR ((void*)0)
#endif

#ifdef SP_WIN32
  #define SP_IMPORT declspec(__dllimport)
  #define SP_EXPORT declspec(__dllexport)
#endif

#ifdef SP_POSIX
  #define SP_IMPORT
  #define SP_EXPORT __attribute__((visibility("default")))
#endif

#define SP_FALLTHROUGH() ((void)0)

#define SP_ZERO_STRUCT(t) SP_RVAL(t) SP_ZERO_INITIALIZE()
#define SP_ZERO_RETURN(t) { t __SP_ZERO_RETURN = SP_ZERO_STRUCT(t); return __dn_zero_return; }

#define SP_EXIT_SUCCESS() exit(0)
#define SP_EXIT_FAILURE() exit(1)
#define SP_ASSERT(condition) assert((condition))
#define SP_ASSERT_FMT(COND, FMT, ...) \
  do { \
    if (!(COND)) { \
      const c8* condition = SP_MACRO_STR(COND); \
      sp_str_t message = sp_format((FMT), ##__VA_ARGS__); \
      SP_LOG("SP_ASSERT({:color red}): {}", SP_FMT_CSTR(condition), SP_FMT_STR(message)); \
      SP_EXIT_FAILURE(); \
    } \
  } while (0)
#define SP_FATAL(FMT, ...) \
  do { \
    sp_str_t message = sp_format((FMT), ##__VA_ARGS__); \
    SP_LOG("{:color red}: {}", SP_FMT_CSTR("SP_FATAL()"), SP_FMT_STR(message)); \
    SP_EXIT_FAILURE(); \
  } while (0)

#define SP_UNREACHABLE() SP_ASSERT(false)
#define SP_UNREACHABLE_CASE() SP_ASSERT(false); break;
#define SP_UNREACHABLE_RETURN(v) SP_ASSERT(false); return (v)
#define SP_BROKEN() SP_ASSERT(false)
#define SP_UNTESTED()
#define SP_INCOMPLETE()

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

#define SP_X_ENUM_CASE_TO_CSTR(ID)         case ID: { return SP_MACRO_STR(ID); }
#define SP_X_ENUM_CASE_TO_STRING(ID)       case ID: { return SP_LIT(SP_MACRO_STR(ID)); }
#define SP_X_ENUM_CASE_TO_STRING_UPPER(ID) case ID: { return sp_str_to_upper(SP_LIT(SP_MACRO_STR(ID))); }
#define SP_X_ENUM_CASE_TO_STRING_LOWER(ID) case ID: { return sp_str_to_lower(SP_LIT(SP_MACRO_STR(ID))); }
#define SP_X_ENUM_DEFINE(ID) ID,

#define SP_X_NAMED_ENUM_CASE_TO_CSTR(ID, NAME)         case ID: { return (NAME); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING(ID, NAME)       case ID: { return sp_str_lit(NAME); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING_UPPER(ID, NAME) case ID: { return sp_str_to_upper(sp_str_lit(NAME)); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING_LOWER(ID, NAME) case ID: { return sp_str_to_lower(sp_str_LIT(NAME)); }
#define SP_X_NAMED_ENUM_DEFINE(ID, NAME) ID,
#define SP_X_NAMED_ENUM_STR_TO_ENUM(ID, NAME) if (sp_str_equal(str, SP_LIT(NAME))) return ID;

#define SP_CARR_LEN(CARR) (sizeof((CARR)) / sizeof((CARR)[0]))
#define SP_CARR_FOR(CARR, IT) for (u32 IT = 0; IT < SP_CARR_LEN(CARR); IT++)

#define SP_SIZE_TO_INDEX(size) ((size) ? ((size) - 1) : 0)

#ifndef SP_API
  #define SP_API
#endif

#ifndef SP_IMP
  #define SP_IMP
#endif

#define SP_UNUSED(x) ((void)(x))

#define SP_ANSI_RESET             "\033[0m"
#define SP_ANSI_BOLD              "\033[1m"
#define SP_ANSI_DIM               "\033[2m"
#define SP_ANSI_ITALIC            "\033[3m"
#define SP_ANSI_UNDERLINE         "\033[4m"
#define SP_ANSI_BLINK             "\033[5m"
#define SP_ANSI_REVERSE           "\033[7m"
#define SP_ANSI_HIDDEN            "\033[8m"
#define SP_ANSI_STRIKETHROUGH     "\033[9m"
#define SP_ANSI_FG_BLACK          "\033[30m"
#define SP_ANSI_FG_RED            "\033[31m"
#define SP_ANSI_FG_GREEN          "\033[32m"
#define SP_ANSI_FG_YELLOW         "\033[33m"
#define SP_ANSI_FG_BLUE           "\033[34m"
#define SP_ANSI_FG_MAGENTA        "\033[35m"
#define SP_ANSI_FG_CYAN           "\033[36m"
#define SP_ANSI_FG_WHITE          "\033[37m"
#define SP_ANSI_BG_BLACK          "\033[40m"
#define SP_ANSI_BG_RED            "\033[41m"
#define SP_ANSI_BG_GREEN          "\033[42m"
#define SP_ANSI_BG_YELLOW         "\033[43m"
#define SP_ANSI_BG_BLUE           "\033[44m"
#define SP_ANSI_BG_MAGENTA        "\033[45m"
#define SP_ANSI_BG_CYAN           "\033[46m"
#define SP_ANSI_BG_WHITE          "\033[47m"
#define SP_ANSI_FG_BRIGHT_BLACK   "\033[90m"
#define SP_ANSI_FG_BRIGHT_RED     "\033[91m"
#define SP_ANSI_FG_BRIGHT_GREEN   "\033[92m"
#define SP_ANSI_FG_BRIGHT_YELLOW  "\033[93m"
#define SP_ANSI_FG_BRIGHT_BLUE    "\033[94m"
#define SP_ANSI_FG_BRIGHT_MAGENTA "\033[95m"
#define SP_ANSI_FG_BRIGHT_CYAN    "\033[96m"
#define SP_ANSI_FG_BRIGHT_WHITE   "\033[97m"
#define SP_ANSI_BG_BRIGHT_BLACK   "\033[100m"
#define SP_ANSI_BG_BRIGHT_RED     "\033[101m"
#define SP_ANSI_BG_BRIGHT_GREEN   "\033[102m"
#define SP_ANSI_BG_BRIGHT_YELLOW  "\033[103m"
#define SP_ANSI_BG_BRIGHT_BLUE    "\033[104m"
#define SP_ANSI_BG_BRIGHT_MAGENTA "\033[105m"
#define SP_ANSI_BG_BRIGHT_CYAN    "\033[106m"
#define SP_ANSI_BG_BRIGHT_WHITE   "\033[107m"


SP_BEGIN_EXTERN_C()

// ████████╗██╗   ██╗██████╗ ███████╗███████╗
// ╚══██╔══╝╚██╗ ██╔╝██╔══██╗██╔════╝██╔════╝
//    ██║    ╚████╔╝ ██████╔╝█████╗  ███████╗
//    ██║     ╚██╔╝  ██╔═══╝ ██╔══╝  ╚════██║
//    ██║      ██║   ██║     ███████╗███████║
//    ╚═╝      ╚═╝   ╚═╝     ╚══════╝╚══════╝
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
typedef size_t   sp_size_t;
typedef void*    sp_opaque_ptr;


// ███████╗██████╗ ██████╗  ██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔══██╗
// █████╗  ██████╔╝██████╔╝██║   ██║██████╔╝
// ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══██╗
// ███████╗██║  ██║██║  ██║╚██████╔╝██║  ██║
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
typedef enum {
  SP_ERR_OK = 0,
  SP_ERR_IO_EOF,
  SP_ERR_IO,
  SP_ERR_IO_SEEK_INVALID,
  SP_ERR_IO_SEEK_FAILED,
  SP_ERR_IO_WRITE_FAILED,
  SP_ERR_IO_CLOSE_FAILED,
  SP_ERR_IO_READ_FAILED,
  SP_ERR_LAZY,
} sp_err_t;

void sp_err_set(sp_err_t err);


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
} sp_malloc_allocator_t;

sp_allocator_t        sp_allocator_default();
void*                 sp_allocator_alloc(sp_allocator_t allocator, u32 size);
void*                 sp_allocator_realloc(sp_allocator_t allocator, void* memory, u32 size);
void                  sp_allocator_free(sp_allocator_t allocator, void* buffer);
sp_allocator_t        sp_bump_allocator_init(sp_bump_allocator_t* allocator, u32 capacity);
void                  sp_bump_allocator_clear(sp_bump_allocator_t* allocator);
void                  sp_bump_allocator_destroy(sp_bump_allocator_t* allocator);
void*                 sp_bump_allocator_on_alloc(void* allocator, sp_allocator_mode_t mode, u32 size, void* old_memory);
sp_allocator_t        sp_malloc_allocator_init();
void*                 sp_malloc_allocator_on_alloc(void* user_data, sp_allocator_mode_t mode, u32 size, void* ptr);
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
sp_hash_t sp_hash_combine(sp_hash_t* hashes, u32 num_hashes);
sp_hash_t sp_hash_bytes(const void* p, u64 len, u64 seed);


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

///////////////////
// DYNAMIC ARRAY //
///////////////////
typedef struct sp_dyn_array {
    s32 size;
    s32 capacity;
} sp_dyn_array;

void* sp_dyn_array_resize_impl(void* arr, u32 sz, u32 amount);
void** sp_dyn_array_init(void** arr, u32 val_len);
void sp_dyn_array_push_f(void** arr, void* val, u32 val_len);

#define sp_dyn_array(T) T*
#define sp_da(T) T*
#define sp_dyn_array_for(__ARR, __IT) for (u32 __IT = 0; __IT < sp_dyn_array_size((__ARR)); __IT++)

#define sp_dyn_array_head(__ARR)\
    ((sp_dyn_array*)((u8*)(__ARR) - sizeof(sp_dyn_array)))

#define sp_dyn_array_size(__ARR)\
    (__ARR == NULL ? 0 : sp_dyn_array_head((__ARR))->size)

#define sp_dyn_array_capacity(__ARR)\
    (__ARR == NULL ? 0 : sp_dyn_array_head((__ARR))->capacity)

#define sp_dyn_array_empty(__ARR)\
    (sp_dyn_array_size(__ARR) == 0)

#define sp_dyn_array_full(__ARR)\
    ((sp_dyn_array_size((__ARR)) == sp_dyn_array_capacity((__ARR))))

#define sp_dyn_array_clear(__ARR)\
    do {\
        if (__ARR) {\
            sp_dyn_array_head(__ARR)->size = 0;\
        }\
    } while (0)

#define sp_dyn_array_free(__ARR)\
    do {\
        if (__ARR) {\
            sp_free(sp_dyn_array_head(__ARR));\
            (__ARR) = NULL;\
        }\
    } while (0)

#define sp_dyn_array_need_grow(__ARR, __N)\
    ((__ARR) == 0 || sp_dyn_array_size(__ARR) + (__N) >= sp_dyn_array_capacity(__ARR))

#define sp_dyn_array_grow(__ARR)\
    sp_dyn_array_resize_impl((__ARR), sizeof(*(__ARR)), sp_dyn_array_capacity(__ARR) ? sp_dyn_array_capacity(__ARR) * 2 : 1)

#define sp_dyn_array_push(__ARR, __VAL)\
    do {\
        sp_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));\
        if (!(__ARR) || ((__ARR) && sp_dyn_array_need_grow(__ARR, 1))) {\
            *((void **)&(__ARR)) = sp_dyn_array_grow(__ARR); \
        }\
        (__ARR)[sp_dyn_array_size(__ARR)] = (__VAL);\
        sp_dyn_array_head(__ARR)->size++;\
    } while(0)

#define sp_dyn_array_reserve(__ARR, __AMOUNT)\
    do {\
        if ((!__ARR)) sp_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));\
        if ((!__ARR) || (u32)__AMOUNT > sp_dyn_array_capacity(__ARR)) {\
            *((void **)&(__ARR)) = sp_dyn_array_resize_impl(__ARR, sizeof(*__ARR), __AMOUNT);\
        }\
    } while(0)

#define sp_dyn_array_pop(__ARR)\
    do {\
        if (__ARR && !sp_dyn_array_empty(__ARR)) {\
            sp_dyn_array_head(__ARR)->size -= 1;\
        }\
    } while (0)

#define sp_dyn_array_back(__ARR)\
    (__ARR + (sp_dyn_array_size(__ARR) ? sp_dyn_array_size(__ARR) - 1 : 0))

#define sp_dyn_array_new(__T)\
    ((__T*)sp_dyn_array_resize_impl(NULL, sizeof(__T), 0))


////////////////
// HASH TABLE //
////////////////
#define SP_HT_HASH_SEED         0x31415296
#define SP_HT_INVALID_INDEX     UINT32_MAX

typedef u32 sp_ht_it;
SP_TYPEDEF_FN(sp_hash_t, sp_ht_hash_key_fn_t, void*, u32);
SP_TYPEDEF_FN(bool, sp_ht_compare_key_fn_t, void*, void*, u32);

typedef enum sp_ht_entry_state {
    SP_HT_ENTRY_INACTIVE = 0x00,
    SP_HT_ENTRY_ACTIVE = 0x01
} sp_ht_entry_state;

typedef struct {
  u32 stride;
  u32 klpvl;
  u32 tmp_idx;
  struct {
    u32 key;
    u32 value;
  } size;
  struct {
    sp_ht_hash_key_fn_t hash;
    sp_ht_compare_key_fn_t compare;
  } fn;
} sp_ht_info_t;

#define __sp_ht_entry(__K, __V)\
    struct\
    {\
        __K key;\
        __V val;\
        sp_ht_entry_state state;\
    }
#define sp_ht(__K, __V)                \
    struct {                                   \
        __sp_ht_entry(__K, __V)* data; \
        __K tmp_key;                           \
        __V tmp_val;                           \
        sp_ht_info_t info; \
    }*

#define sp_ht_new(__K, __V) SP_NULLPTR


#define sp_ht_init(ht)\
    do {\
        *((void**)(&(ht))) = sp_alloc(sizeof(*ht));                    \
        sp_os_zero_memory((ht), sizeof(*ht));                          \
                                                                       \
        sp_dyn_array_reserve(ht->data, 2);                             \
        ht->data[0].state = SP_HT_ENTRY_INACTIVE;              \
        ht->data[1].state = SP_HT_ENTRY_INACTIVE;              \
                                                                       \
        u8* d0 = (u8*)&((ht)->data[0]);                                \
        u8* d1 = (u8*)&((ht)->data[1]);                                \
        u32 diff = (d1 - d0);                                          \
                                                                       \
        u32 klpvl = (u8*)&(ht->data[0].state) - (u8*)(&ht->data[0]);   \
                                                                       \
        (ht)->info.size.key = (u32)(sizeof((ht)->data[0].key));        \
        (ht)->info.size.value = (u32)(sizeof((ht)->data[0].val));      \
        (ht)->info.stride = (u32)(diff);                               \
        (ht)->info.klpvl = (u32)(klpvl);                               \
        (ht)->info.fn.hash = sp_ht_on_hash_key;                \
        (ht)->info.fn.compare = sp_ht_on_compare_key;          \
    } while (0)

#define sp_ht_set_fns(ht, hash_fn, cmp_fn) \
  if (!(ht)) sp_ht_init(ht);             \
  (ht)->info.fn.hash = (hash_fn);                   \
  (ht)->info.fn.compare = (cmp_fn);

#define sp_ht_size(ht)\
    ((ht) != NULL ? sp_dyn_array_size((ht)->data) : 0)

#define sp_ht_capacity(ht)\
    ((ht) != NULL ? sp_dyn_array_capacity((ht)->data) : 0)

#define sp_ht_empty(ht)\
    ((ht) != NULL ? sp_dyn_array_size((ht)->data) == 0 : true)

#define sp_ht_clear(ht)\
    do {\
        if ((ht) != NULL) {\
            u32 capacity = sp_dyn_array_capacity((ht)->data);\
            for (u32 i = 0; i < capacity; ++i) {\
                (ht)->data[i].state = SP_HT_ENTRY_INACTIVE;\
            }\
            sp_dyn_array_clear((ht)->data);\
        }\
    } while (0)

#define sp_ht_free(ht)\
    do {\
        if ((ht) != NULL) {\
            sp_dyn_array_free((ht)->data);\
            (ht)->data = NULL;\
            sp_free(ht);\
            (ht) = NULL;\
        }\
    } while (0)

#define sp_ht_insert(__HT, __K, __V)\
    do {\
        if ((__HT) == SP_NULLPTR) {\
            sp_ht_init((__HT));\
        }\
        sp_ht_info_t __INFO = (__HT)->info;\
        u32 __CAP = sp_ht_capacity(__HT);\
        f32 __LF = __CAP ? (f32)(sp_ht_size(__HT)) / (f32)(__CAP) : 0.f;\
        if (__LF >= 0.5f || !__CAP)\
        {\
            u32 NEW_CAP = __CAP ? __CAP * 2 : 2;\
            sp_dyn_array_reserve((__HT)->data, NEW_CAP);\
            for (u32 __I = __CAP; __I < NEW_CAP; ++__I) {\
                (__HT)->data[__I].state = SP_HT_ENTRY_INACTIVE;\
            }\
            __CAP = sp_ht_capacity(__HT);\
        }\
        (__HT)->tmp_key = (__K);\
        u32 __EXISTING_IDX = sp_ht_tmp_key_index(__HT);\
        bool __KEY_EXISTS = (__EXISTING_IDX != SP_HT_INVALID_INDEX);\
        sp_hash_t __HSH = __INFO.fn.hash((void*)(&(__HT)->tmp_key), __INFO.size.key);\
        u32 __HSH_IDX = __HSH % __CAP;\
        (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;\
        u32 c = 0;\
        while (\
            c < __CAP\
            && __HSH != __INFO.fn.hash((void*)(&(__HT)->tmp_key), __INFO.size.key)\
            && (__HT)->data[__HSH_IDX].state == SP_HT_ENTRY_ACTIVE)\
        {\
            __HSH_IDX = ((__HSH_IDX + 1) % __CAP);\
            (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;\
            ++c;\
        }\
        (__HT)->data[__HSH_IDX].key = (__K);\
        (__HT)->data[__HSH_IDX].val = (__V);\
        (__HT)->data[__HSH_IDX].state = SP_HT_ENTRY_ACTIVE;\
        if (!__KEY_EXISTS) sp_dyn_array_head((__HT)->data)->size++;\
    } while (0)

#define sp_ht_getp(__HT, __K)\
    (\
        (__HT) == SP_NULLPTR ? SP_NULLPTR :\
        ((__HT)->tmp_key = (__K), \
        ((__HT)->info.tmp_idx = sp_ht_tmp_key_index(__HT), \
        ((__HT)->info.tmp_idx != SP_HT_INVALID_INDEX ? &(__HT)->data[(__HT)->info.tmp_idx].val : NULL))) \
    )

#define sp_ht_exists(__HT, __K) ((__HT) && ((__HT)->tmp_key = (__K), sp_ht_tmp_key_index(__HT) != SP_HT_INVALID_INDEX))

#define sp_ht_key_exists(__HT, __K) sp_ht_exists((__HT), (__K))

#define sp_ht_erase(__HT, __K)\
    do {\
        if ((__HT))\
        {\
            (__HT)->tmp_key = (__K);\
            u32 __IDX = sp_ht_tmp_key_index(__HT);\
            if (__IDX != SP_HT_INVALID_INDEX) {\
                (__HT)->data[__IDX].state = SP_HT_ENTRY_INACTIVE;\
                if (sp_dyn_array_head((__HT)->data)->size) sp_dyn_array_head((__HT)->data)->size--;\
            }\
        }\
    } while (0)

#define sp_ht_it_valid(__ht, __it)\
  ((__ht) == NULL ? false : (((__it) < sp_ht_capacity((__ht))) && ((__ht)->data[(__it)].state == SP_HT_ENTRY_ACTIVE)))

#define sp_ht_it_advance(__ht, __it)\
  ((__ht) == NULL ? (void)0 : (sp_ht_it_advance_fn((void**)&(__ht)->data, &(__it), (__ht)->info)))

#define sp_ht_it_getp(__ht, __it)\
  ((__ht) == NULL ? NULL : (&((__ht)->data[(__it)].val)))

#define sp_ht_it_getkp(__ht, __it)\
  ((__ht) == NULL ? NULL : (&((__ht)->data[(__it)].key)))

#define sp_ht_it_init(__ht)\
  ((__ht) == NULL ? 0 : (sp_ht_it_init_fn((void**)&(__ht)->data, (__ht)->info)))

#define sp_ht_for(__ht, __it)\
  for (sp_ht_it __it = sp_ht_it_init(__ht); sp_ht_it_valid(__ht, __it); sp_ht_it_advance(__ht, __it))

#define sp_ht_tmp_key_index(__HT) sp_ht_get_key_index_fn((void**)&((__HT)->data), (void*)&((__HT)->tmp_key), (__HT)->info)

u32        sp_ht_get_key_index_fn(void** data, void* key, sp_ht_info_t info);
sp_ht_it sp_ht_it_init_fn(void** data, sp_ht_info_t info);
void       sp_ht_it_advance_fn(void** data, u32* it, sp_ht_info_t info);

sp_hash_t  sp_ht_on_hash_key(void* key, u32 size);
bool       sp_ht_on_compare_key(void* ka, void* kb, u32 size);
sp_hash_t sp_ht_on_hash_str_key(void* key, u32 size);
bool      sp_ht_on_compare_str_key(void* ka, void* kb, u32 size);



///////////////
// RING BUFFER/
///////////////
typedef struct {
  u8* data;
  u32 element_size;
  u32 head;
  u32 size;
  u32 capacity;
} sp_ring_buffer_t;

#define sp_ring_buffer(t) sp_ring_buffer_t

typedef struct {
  s32 index;
  bool reverse;
  sp_ring_buffer_t* buffer;
} sp_ring_buffer_iterator_t;

SP_API void*                     sp_ring_buffer_at(sp_ring_buffer_t* buffer, u32 index);
SP_API void                      sp_ring_buffer_init(sp_ring_buffer_t* buffer, u32 capacity, u32 element_size);
SP_API void*                     sp_ring_buffer_back(sp_ring_buffer_t* buffer);
SP_API void*                     sp_ring_buffer_push(sp_ring_buffer_t* buffer, void* data);
SP_API void*                     sp_ring_buffer_push_zero(sp_ring_buffer_t* buffer);
SP_API void*                     sp_ring_buffer_push_overwrite(sp_ring_buffer_t* buffer, void* data);
SP_API void*                     sp_ring_buffer_push_overwrite_zero(sp_ring_buffer_t* buffer);
SP_API void*                     sp_ring_buffer_pop(sp_ring_buffer_t* buffer);
SP_API u32                       sp_ring_buffer_bytes(sp_ring_buffer_t* buffer);
SP_API void                      sp_ring_buffer_clear(sp_ring_buffer_t* buffer);
SP_API void                      sp_ring_buffer_destroy(sp_ring_buffer_t* buffer);
SP_API bool                      sp_ring_buffer_is_full(sp_ring_buffer_t* buffer);
SP_API bool                      sp_ring_buffer_is_empty(sp_ring_buffer_t* buffer);
SP_API void*                     sp_ring_buffer_iter_deref(sp_ring_buffer_iterator_t* it);
SP_API void                      sp_ring_buffer_iter_next(sp_ring_buffer_iterator_t* it);
SP_API void                      sp_ring_buffer_iter_prev(sp_ring_buffer_iterator_t* it);
SP_API bool                      sp_ring_buffer_iter_done(sp_ring_buffer_iterator_t* it);
SP_API sp_ring_buffer_iterator_t sp_ring_buffer_iter(sp_ring_buffer_t* buffer);
SP_API sp_ring_buffer_iterator_t sp_ring_buffer_riter(sp_ring_buffer_t* buffer);

#define sp_ring_buffer_for(rb, it)  for (sp_ring_buffer_iterator_t (it) = sp_ring_buffer_iter(&(rb)); !sp_ring_buffer_iter_done(&(it)); sp_ring_buffer_iter_next(&(it)))
#define sp_ring_buffer_rfor(rb, it) for (sp_ring_buffer_iterator_t (it) = sp_ring_buffer_riter(&(rb)); !sp_ring_buffer_iter_done(&(it)); sp_ring_buffer_iter_prev(&(it)))
#define sp_rb_it(it, t) ((t*)sp_ring_buffer_iter_deref(&(it)))

#define sp_ring_buffer_push_literal(__RB_PTR, __TYPE, __VALUE) \
    do { \
        __TYPE __sp_rb_tmp = (__VALUE); \
        sp_ring_buffer_push((__RB_PTR), &__sp_rb_tmp); \
    } while (0)


//  ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗
//  ██╔════╝╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝
//  ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗
//  ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║
//  ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝
//  ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
typedef struct {
  u32 len;
  const c8* data;
} sp_str_t;

typedef struct {
  struct {
   c8* data;
   u32 len;
   u32 capacity;
  } buffer;

  struct {
    sp_str_t word;
    u32 level;
  } indent;
} sp_str_builder_t;

typedef struct {
  void* user_data;

  sp_str_builder_t builder;
  struct {
    sp_str_t* data;
    u32 len;
  } elements;

  sp_str_t str;
  u32 index;
} sp_str_reduce_context_t;

typedef struct {
  sp_str_t str;
  sp_opaque_ptr user_data;
} sp_str_map_context_t;

typedef struct {
  sp_str_t needle;
  bool found;
} sp_str_contains_context_t;

typedef struct {
  sp_str_t needle;
  u32 count;
} sp_str_count_context_t;

typedef struct {
  sp_str_t first;
  sp_str_t second;
} sp_str_pair_t;

SP_TYPEDEF_FN(sp_str_t, sp_str_map_fn_t, sp_str_map_context_t* context);
SP_TYPEDEF_FN(void, sp_str_reduce_fn_t, sp_str_reduce_context_t* context);

#define SP_CSTR(STR)   sp_str((STR), sp_cstr_len(STR))
#define sp_str_lit(STR)    sp_str((STR), sizeof(STR) - 1)
#define sp_str(STR, LEN) SP_RVAL(sp_str_t) { .len = (u32)(LEN), .data = (const c8*)(STR) }
#define SP_STR(STR, LEN) sp_str(STR, LEN)
#define SP_LIT(STR) sp_str_lit(STR)

SP_API void     sp_str_builder_grow(sp_str_builder_t* builder, u32 requested_capacity);
SP_API void     sp_str_builder_add_capacity(sp_str_builder_t* builder, u32 amount);
SP_API void     sp_str_builder_indent(sp_str_builder_t* builder);
SP_API void     sp_str_builder_dedent(sp_str_builder_t* builder);
SP_API void     sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str);
SP_API void     sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str);
SP_API void     sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c);
SP_API void     sp_str_builder_append_fmt_str(sp_str_builder_t* builder, sp_str_t fmt, ...);
SP_API void     sp_str_builder_append_fmt(sp_str_builder_t* builder, const c8* fmt, ...);
SP_API void     sp_str_builder_new_line(sp_str_builder_t* builder);
SP_API sp_str_t sp_str_builder_move(sp_str_builder_t* builder);
SP_API sp_str_t sp_str_builder_write(sp_str_builder_t* builder);
SP_API c8*      sp_str_builder_write_cstr(sp_str_builder_t* builder);

SP_API c8*                    sp_cstr_copy(const c8* str);
SP_API void                   sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length);
SP_API c8*                    sp_cstr_copy_sized(const c8* str, u32 length);
SP_API void                   sp_cstr_copy_to_sized(const c8* str, u32 length, c8* buffer, u32 buffer_length);
SP_API bool                   sp_cstr_equal(const c8* a, const c8* b);
SP_API u32                    sp_cstr_len(const c8* str);
SP_API u32                    sp_cstr_len_sized(const c8* str, u32 n);
SP_API c8*                    sp_wstr_to_cstr(c16* str, u32 len);
SP_API c8*                    sp_str_to_cstr(sp_str_t str);
SP_API c8*                    sp_str_to_cstr_double_nt(sp_str_t str);
SP_API sp_str_t               sp_str_copy(sp_str_t str);
SP_API void                   sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity);
SP_API sp_str_t               sp_str_null_terminate(sp_str_t str);
SP_API sp_str_t               sp_str_from_cstr(const c8* str);
SP_API sp_str_t               sp_str_from_cstr_sized(const c8* str, u32 length);
SP_API sp_str_t               sp_str_from_cstr_null(const c8* str);
SP_API sp_str_t               sp_str_alloc(u32 capacity);
SP_API sp_str_t               sp_str_view(const c8* cstr);
SP_API bool                   sp_str_empty(sp_str_t);
SP_API bool                   sp_str_equal(sp_str_t a, sp_str_t b);
SP_API bool                   sp_str_equal_cstr(sp_str_t a, const c8* b);
SP_API bool                   sp_str_starts_with(sp_str_t str, sp_str_t prefix);
SP_API bool                   sp_str_ends_with(sp_str_t str, sp_str_t suffix);
SP_API bool                   sp_str_contains(sp_str_t str, sp_str_t needle);
SP_API bool                   sp_str_valid(sp_str_t str);
SP_API c8                     sp_str_at(sp_str_t str, s32 index);
SP_API c8                     sp_str_at_reverse(sp_str_t str, s32 index);
SP_API c8                     sp_str_back(sp_str_t str);
SP_API s32                    sp_str_compare_alphabetical(sp_str_t a, sp_str_t b);
SP_API sp_str_t               sp_str_sub(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t               sp_str_sub_reverse(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t               sp_str_concat(sp_str_t a, sp_str_t b);
SP_API sp_str_t               sp_str_replace_c8(sp_str_t str, c8 from, c8 to);
SP_API sp_str_t               sp_str_pad(sp_str_t str, u32 n);
SP_API sp_str_t               sp_str_trim(sp_str_t str);
SP_API sp_str_t               sp_str_trim_right(sp_str_t str);
SP_API sp_str_t               sp_str_truncate(sp_str_t str, u32 n, sp_str_t trailer);
SP_API sp_str_t               sp_str_join(sp_str_t a, sp_str_t b, sp_str_t join);
SP_API sp_str_t               sp_str_join_cstr_n(const c8** strings, u32 num_strings, sp_str_t join);
SP_API sp_str_t               sp_str_to_upper(sp_str_t str);
SP_API sp_str_t               sp_str_to_lower(sp_str_t str);
SP_API sp_str_t               sp_str_capitalize_words(sp_str_t str);
SP_API sp_str_pair_t          sp_str_cleave_c8(sp_str_t str, c8 delimiter);
SP_API sp_dyn_array(sp_str_t) sp_str_split_c8(sp_str_t, c8 c);
SP_API bool                   sp_str_contains_n(sp_str_t* strs, u32 n, sp_str_t needle);
SP_API sp_str_t               sp_str_join_n(sp_str_t* strs, u32 n, sp_str_t joiner);
SP_API u32                    sp_str_count_n(sp_str_t* strs, u32 n, sp_str_t needle);
SP_API sp_str_t               sp_str_find_longest_n(sp_str_t* strs, u32 n);
SP_API sp_str_t               sp_str_find_shortest_n(sp_str_t* strs, u32 n);
SP_API sp_dyn_array(sp_str_t) sp_str_pad_to_longest(sp_str_t* strs, u32 n);
SP_API sp_str_t               sp_str_reduce(sp_str_t* strs, u32 n, void* user_data, sp_str_reduce_fn_t fn);
SP_API void                   sp_str_reduce_kernel_join(sp_str_reduce_context_t* context);
SP_API void                   sp_str_reduce_kernel_contains(sp_str_reduce_context_t* context);
SP_API void                   sp_str_reduce_kernel_count(sp_str_reduce_context_t* context);
SP_API void                   sp_str_reduce_kernel_longest(sp_str_reduce_context_t* context);
SP_API void                   sp_str_reduce_kernel_shortest(sp_str_reduce_context_t* context);
SP_API sp_dyn_array(sp_str_t) sp_str_map(sp_str_t* s, u32 n, sp_opaque_ptr user_data, sp_str_map_fn_t fn);
SP_API sp_str_t               sp_str_map_kernel_prepend(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_append(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_prefix(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_trim(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_pad(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_to_upper(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_to_lower(sp_str_map_context_t* context);
SP_API sp_str_t               sp_str_map_kernel_capitalize_words(sp_str_map_context_t* context);
SP_API s32                    sp_str_sort_kernel_alphabetical(const void* a, const void* b);


// ████████╗██╗███╗   ███╗███████╗
// ╚══██╔══╝██║████╗ ████║██╔════╝
//    ██║   ██║██╔████╔██║█████╗
//    ██║   ██║██║╚██╔╝██║██╔══╝
//    ██║   ██║██║ ╚═╝ ██║███████╗
//    ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝
typedef struct {
  u64 s;
  u32 ns;
} sp_tm_epoch_t;

typedef u64 sp_tm_point_t;

typedef struct {
  s32 year;
  s32 month;
  s32 day;
  s32 hour;
  s32 minute;
  s32 second;
  s32 millisecond;
} sp_tm_date_time_t;

typedef struct {
  sp_tm_point_t start;
  sp_tm_point_t previous;
} sp_tm_timer_t;

SP_API sp_tm_epoch_t             sp_tm_now_epoch();
SP_API sp_str_t                  sp_tm_to_iso8601(sp_tm_epoch_t time);
SP_API sp_tm_point_t             sp_tm_now_point();
SP_API u64                       sp_tm_point_diff(sp_tm_point_t newer, sp_tm_point_t older);
SP_API sp_tm_timer_t             sp_tm_start_timer();
SP_API u64                       sp_tm_read_timer(sp_tm_timer_t* timer);
SP_API u64                       sp_tm_lap_timer(sp_tm_timer_t* timer);
SP_API void                      sp_tm_reset_timer(sp_tm_timer_t* timer);
SP_API sp_tm_date_time_t         sp_tm_get_date_time();


// ███████╗██████╗ ██████╗  █████╗ ████████╗ █████╗
// ██╔════╝██╔══██╗██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// █████╗  ██████╔╝██████╔╝███████║   ██║   ███████║
// ██╔══╝  ██╔══██╗██╔══██╗██╔══██║   ██║   ██╔══██║
// ███████╗██║  ██║██║  ██║██║  ██║   ██║   ██║  ██║
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝
#define SP_TERNARY_X(X) \
  X(SP_TERNARY_FALSE) \
  X(SP_TERNARY_TRUE) \
  X(SP_TERNARY_NULL)

typedef enum {
  SP_TERNARY_X(SP_X_ENUM_DEFINE)
} sp_ternary_t;

SP_API sp_str_t sp_ternary_to_str(sp_ternary_t ternary);


// ██╗      ██████╗  ██████╗
// ██║     ██╔═══██╗██╔════╝
// ██║     ██║   ██║██║  ███╗
// ██║     ██║   ██║██║   ██║
// ███████╗╚██████╔╝╚██████╔╝
// ╚══════╝ ╚═════╝  ╚═════╝
#define SP_LOG(CSTR, ...)    sp_log(SP_CSTR((CSTR)), ##__VA_ARGS__)
#define SP_LOG_STR(STR, ...) sp_log((STR),           ##__VA_ARGS__)
void sp_log(sp_str_t fmt, ...);


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

struct sp_file_monitor {
	sp_file_change_callback_t callback;
	sp_file_change_event_t events_to_watch;
	void* userdata;
	u32 debounce_time_ms;
	sp_dynamic_array_t changes;
	sp_dynamic_array_t cache;
  sp_opaque_ptr os;
};

SP_API void              sp_file_monitor_init(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata);
SP_API void              sp_file_monitor_init_debounce(sp_file_monitor_t* monitor, sp_file_change_callback_t callback, sp_file_change_event_t events, void* userdata, u32 debounce_ms);
SP_API void              sp_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path);
SP_IMP void              sp_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path);
SP_API void              sp_file_monitor_process_changes(sp_file_monitor_t* monitor);
SP_API void              sp_file_monitor_emit_changes(sp_file_monitor_t* monitor);
SP_API bool              sp_file_monitor_check_cache(sp_file_monitor_t* monitor, sp_str_t file_path, f64 time);
SP_API sp_cache_entry_t* sp_file_monitor_find_cache_entry(sp_file_monitor_t* monitor, sp_str_t file_path);


//   ██████╗ ███████╗
//  ██╔═══██╗██╔════╝
//  ██║   ██║███████╗
//  ██║   ██║╚════██║
//  ╚██████╔╝███████║
//   ╚═════╝ ╚══════╝
// @sp_os

//////////////
// OS TYPES //
//////////////
#ifdef SP_WIN32
  typedef HANDLE              sp_win32_handle_t;
  typedef DWORD               sp_win32_dword_t;
  typedef WIN32_FIND_DATA     sp_win32_find_data_t;
  typedef CRITICAL_SECTION    sp_win32_critical_section_t;
  typedef OVERLAPPED          sp_win32_overlapped_t;
  typedef PROCESS_INFORMATION sp_win32_process_information_t;
  typedef STARTUPINFO         sp_win32_startup_info_t;
  typedef SECURITY_ATTRIBUTES sp_win32_security_attributes_t;
  typedef HANDLE              sp_os_file_handle_t;

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
#endif

#if defined(SP_POSIX)
  typedef s32 sp_os_file_handle_t;
#endif

#ifdef SP_LINUX
  typedef struct {
    s32 fd;
    sp_dynamic_array_t watch_descs;
    sp_dynamic_array_t watch_paths;
    u8 buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  } sp_os_linux_file_monitor_t;

  typedef sp_os_linux_file_monitor_t sp_os_file_monitor_t;
#endif

#ifdef SP_MACOS
  typedef struct {
    void* placeholder;
  } sp_os_file_monitor_t;
#endif

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

typedef enum {
  SP_OS_PLATFORM_LINUX,
  SP_OS_PLATFORM_WIN32,
  SP_OS_PLATFORM_MACOS,
} sp_os_platform_kind_t;

typedef enum {
  SP_OS_LIB_SHARED,
  SP_OS_LIB_STATIC,
} sp_os_lib_kind_t;

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
} sp_os_dir_entry_t;

SP_API void*                    sp_os_allocate_memory(u32 size);
SP_API void*                    sp_os_reallocate_memory(void* ptr, u32 size);
SP_API void                     sp_os_free_memory(void* ptr);
SP_API void                     sp_os_copy_memory(const void* source, void* dest, u32 num_bytes);
SP_API void                     sp_os_move_memory(const void* source, void* dest, u32 num_bytes);
SP_API bool                     sp_os_is_memory_equal(const void* a, const void* b, size_t len);
SP_API void                     sp_os_fill_memory(void* buffer, u32 buffer_size, void* fill, u32 fill_size);
SP_API void                     sp_os_fill_memory_u8(void* buffer, u32 buffer_size, u8 fill);
SP_API void                     sp_os_zero_memory(void* buffer, u32 buffer_size);
SP_API sp_os_platform_kind_t    sp_os_platform_kind();
SP_API sp_str_t                 sp_os_platform_name();
SP_API void                     sp_os_sleep_ms(f64 ms);
SP_API c8*                      sp_os_wstr_to_cstr(c16* str, u32 len);
SP_API void                     sp_os_print(sp_str_t message);
SP_API void                     sp_os_log(sp_str_t message);
SP_API sp_str_t                 sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind);
SP_API sp_str_t                 sp_os_lib_to_file_name(sp_str_t lib, sp_os_lib_kind_t kind);

SP_API bool                     sp_os_is_regular_file(sp_str_t path);
SP_API bool                     sp_os_is_directory(sp_str_t path);
SP_API bool                     sp_os_is_path_root(sp_str_t path);
SP_API bool                     sp_os_is_glob(sp_str_t path);
SP_API bool                     sp_os_is_program_on_path(sp_str_t program);
SP_API bool                     sp_os_does_path_exist(sp_str_t path);
SP_API void                     sp_os_create_directory(sp_str_t path);
SP_API void                     sp_os_remove_directory(sp_str_t path);
SP_API void                     sp_os_create_file(sp_str_t path);
SP_API void                     sp_os_remove_file(sp_str_t path);
SP_API void                     sp_os_copy(sp_str_t from, sp_str_t to);
SP_API void                     sp_os_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to);
SP_API void                     sp_os_copy_file(sp_str_t from, sp_str_t to);
SP_API void                     sp_os_copy_directory(sp_str_t from, sp_str_t to);
SP_API sp_da(sp_os_dir_entry_t) sp_os_scan_directory(sp_str_t path);
SP_API sp_str_t                 sp_os_normalize_path(sp_str_t path);
SP_API void                     sp_os_normalize_path_soft(sp_str_t* path);
SP_API sp_str_t                 sp_os_parent_path(sp_str_t path);
SP_API sp_str_t                 sp_os_join_path(sp_str_t a, sp_str_t b);
SP_API sp_str_t                 sp_os_extract_extension(sp_str_t path);
SP_API sp_str_t                 sp_os_extract_stem(sp_str_t path);
SP_API sp_str_t                 sp_os_extract_file_name(sp_str_t path);
SP_API sp_str_t                 sp_os_get_cwd();
SP_API sp_str_t                 sp_os_get_executable_path();
SP_API sp_str_t                 sp_os_get_storage_path();
SP_API sp_str_t                 sp_os_get_config_path();
SP_API sp_str_t                 sp_os_canonicalize_path(sp_str_t path);
SP_API sp_tm_epoch_t            sp_os_file_mod_time_precise(sp_str_t path);
SP_API sp_os_file_attr_t        sp_os_file_attributes(sp_str_t path);
SP_IMP sp_os_file_attr_t        sp_os_winapi_attr_to_sp_attr(u32 attr);
SP_IMP void                     sp_os_file_monitor_init(sp_file_monitor_t* monitor);
SP_IMP void                     sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path);
SP_IMP void                     sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path);
SP_IMP void                     sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor);


// ████████╗██╗  ██╗██████╗ ███████╗ █████╗ ██████╗ ██╗███╗   ██╗ ██████╗
// ╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗██╔══██╗██║████╗  ██║██╔════╝
//    ██║   ███████║██████╔╝█████╗  ███████║██║  ██║██║██╔██╗ ██║██║  ███╗
//    ██║   ██╔══██║██╔══██╗██╔══╝  ██╔══██║██║  ██║██║██║╚██╗██║██║   ██║
//    ██║   ██║  ██║██║  ██║███████╗██║  ██║██████╔╝██║██║ ╚████║╚██████╔╝
//    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝
#if defined(SP_WIN32)
  typedef thrd_t               sp_thread_t;
  typedef mtx_t                sp_mutex_t;
  typedef HANDLE               sp_semaphore_t;
#elif defined(SP_LINUX)
  typedef pthread_t            sp_thread_t;
  typedef pthread_mutex_t      sp_mutex_t;
  typedef sem_t                sp_semaphore_t;
#elif defined(SP_MACOS)
  typedef pthread_t            sp_thread_t;
  typedef pthread_mutex_t      sp_mutex_t;
  typedef dispatch_semaphore_t sp_semaphore_t;
#endif

typedef s32 sp_spin_lock_t;
typedef s32 sp_atomic_s32;

SP_TYPEDEF_FN(s32, sp_thread_fn_t, void*);

typedef struct {
  sp_thread_fn_t fn;
  void* userdata;
  sp_semaphore_t semaphore;
} sp_thread_launch_t;

typedef struct {
  sp_allocator_t allocator;
  sp_atomic_s32 ready;
  void* value;
  u32 size;
} sp_future_t;

SP_API void         sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata);
SP_API void         sp_thread_join(sp_thread_t* thread);
SP_API s32          sp_thread_launch(void* userdata);
SP_API void         sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind);
SP_API void         sp_mutex_lock(sp_mutex_t* mutex);
SP_API void         sp_mutex_unlock(sp_mutex_t* mutex);
SP_API void         sp_mutex_destroy(sp_mutex_t* mutex);
SP_API s32          sp_mutex_kind_to_c11(sp_mutex_kind_t kind);
SP_API void         sp_semaphore_init(sp_semaphore_t* semaphore);
SP_API void         sp_semaphore_destroy(sp_semaphore_t* semaphore);
SP_API void         sp_semaphore_wait(sp_semaphore_t* semaphore);
SP_API bool         sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms);
SP_API void         sp_semaphore_signal(sp_semaphore_t* semaphore);
SP_API sp_future_t* sp_future_create(u32 size);
SP_API void         sp_future_set_value(sp_future_t* future, void* data);
SP_API void         sp_future_destroy(sp_future_t* future);
SP_API void         sp_spin_pause();
SP_API bool         sp_spin_try_lock(sp_spin_lock_t* lock);
SP_API void         sp_spin_lock(sp_spin_lock_t* lock);
SP_API void         sp_spin_unlock(sp_spin_lock_t* lock);
SP_API bool         sp_atomic_s32_cmp_and_swap(sp_atomic_s32* value, s32 current, s32 desired);
SP_API s32          sp_atomic_s32_set(sp_atomic_s32* value, s32 desired);
SP_API s32          sp_atomic_s32_add(sp_atomic_s32* value, s32 add);
SP_API s32          sp_atomic_s32_get(sp_atomic_s32* value);


//  ██████╗ ██████╗ ███╗   ██╗████████╗███████╗██╗  ██╗████████╗
// ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝
// ██║     ██║   ██║██╔██╗ ██║   ██║   █████╗   ╚███╔╝    ██║
// ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══╝   ██╔██╗    ██║
// ╚██████╗╚██████╔╝██║ ╚████║   ██║   ███████╗██╔╝ ██╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝   ╚═╝
#define SP_MAX_CONTEXT 16

typedef struct {
  sp_allocator_t allocator;
  sp_mutex_t mutex;
} sp_context_t;

#define SP_GLOBAL_NUM_SPIN_LOCKS 32
typedef struct {
  u8 initted;
  sp_mutex_t mutex;
  sp_spin_lock_t locks [SP_GLOBAL_NUM_SPIN_LOCKS];
} sp_sp_t;
static sp_sp_t sp_global = SP_ZERO_INITIALIZE();

typedef struct {
  sp_allocator_t allocator;
} sp_config_t;

extern pthread_key_t sp_context_stack_key;
extern pthread_key_t sp_context_key;
extern pthread_once_t sp_context_keys_once;

sp_context_t* sp_context_get();
void          sp_context_set(sp_context_t context);
void          sp_context_push(sp_context_t context);
void          sp_context_push_allocator(sp_allocator_t allocator);
void          sp_context_pop();
void          sp_context_ensure();
void          sp_init(sp_config_t config);


// ██╗ ██████╗
// ██║██╔═══██╗
// ██║██║   ██║
// ██║██║   ██║
// ██║╚██████╔╝
// ╚═╝ ╚═════╝
typedef enum {
  SP_IO_SEEK_SET,
  SP_IO_SEEK_CUR,
  SP_IO_SEEK_END,
} sp_io_whence_t;

typedef enum {
  SP_IO_MODE_READ   = 1 << 0,
  SP_IO_MODE_WRITE  = 1 << 1,
  SP_IO_MODE_APPEND = 1 << 2,
} sp_io_mode_t;

typedef enum {
  SP_IO_FILE_CLOSE_MODE_NONE,
  SP_IO_FILE_CLOSE_MODE_AUTO,
} sp_io_file_close_mode_t;

typedef struct sp_io_stream_t sp_io_stream_t;

SP_TYPEDEF_FN(s64, sp_io_size_fn, sp_io_stream_t* stream);
SP_TYPEDEF_FN(s64, sp_io_seek_fn, sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence);
SP_TYPEDEF_FN(u64, sp_io_read_fn, sp_io_stream_t* stream, void* ptr, u64 size);
SP_TYPEDEF_FN(u64, sp_io_write_fn, sp_io_stream_t* stream, const void* ptr, u64 size);
SP_TYPEDEF_FN(void, sp_io_close_fn, sp_io_stream_t* stream);

typedef struct {
  sp_io_size_fn  size;
  sp_io_seek_fn  seek;
  sp_io_read_fn  read;
  sp_io_write_fn write;
  sp_io_close_fn close;
} sp_io_callbacks_t;

typedef struct {
  s32 fd;
  sp_io_file_close_mode_t close_mode;
} sp_io_file_data_t;

typedef struct {
  u8* base;
  u8* here;
  u8* stop;
} sp_io_memory_data_t;

struct sp_io_stream_t {
  sp_io_callbacks_t callbacks;
  union {
    sp_io_file_data_t file;
    sp_io_memory_data_t memory;
  };
  sp_allocator_t allocator;
};

SP_API sp_io_stream_t sp_io_from_file(sp_str_t path, sp_io_mode_t mode);
SP_API sp_io_stream_t sp_io_from_memory(void* memory, u64 size);
SP_API sp_io_stream_t sp_io_from_file_handle(sp_os_file_handle_t handle, sp_io_file_close_mode_t close_mode);
SP_API u64            sp_io_read(sp_io_stream_t* stream, void* ptr, u64 size);
SP_API u64            sp_io_write(sp_io_stream_t* stream, const void* ptr, u64 size);
SP_API u64            sp_io_write_str(sp_io_stream_t* stream, sp_str_t str);
SP_API s64            sp_io_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence);
SP_API s64            sp_io_size(sp_io_stream_t* stream);
SP_API void           sp_io_close(sp_io_stream_t* stream);
SP_API sp_str_t       sp_io_read_file(sp_str_t path);


// ██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗
// ██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝
// ██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗
// ██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║
// ██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝
#define SP_PS_MAX_ARGS 8
#define SP_PS_MAX_ENV 16

typedef enum {
  SP_PS_IO_FILENO_NONE,
  SP_PS_IO_FILENO_STDIN = 0,
  SP_PS_IO_FILENO_STDOUT = 1,
  SP_PS_IO_FILENO_STDERR = 2,
} sp_ps_io_file_number_t;

typedef enum {
  SP_ENV_EXPORT_OVERWRITE_DUPES,
  SP_ENV_EXPORT_SKIP_DUPES,
} sp_env_export_overwrite_t;

typedef enum {
  SP_PS_IO_MODE_INHERIT,
  SP_PS_IO_MODE_NULL,
  SP_PS_IO_MODE_CREATE,
  SP_PS_IO_MODE_EXISTING,
  SP_PS_IO_MODE_REDIRECT,
} sp_ps_io_mode_t;

typedef enum {
  SP_PS_IO_NONBLOCKING,
  SP_PS_IO_BLOCKING,
} sp_ps_io_blocking_t;

typedef enum {
  SP_PS_ENV_INHERIT,
  SP_PS_ENV_CLEAN,
  SP_PS_ENV_EXISTING,
} sp_ps_env_mode_t;

typedef enum {
  SP_PS_STATE_RUNNING,
  SP_PS_STATE_DONE
} sp_ps_state_t;

#define SP_PS_NO_STDIO (sp_ps_io_config_t) { \
  .in = { .mode = SP_PS_IO_NULL }, \
  .out = { .mode = SP_PS_IO_NULL }, \
  .err = { .mode = SP_PS_IO_NULL }, \
}
typedef struct {
  sp_str_t key;
  sp_str_t value;
} sp_env_var_t;

typedef sp_ht(sp_str_t, sp_str_t) sp_env_table_t;

typedef struct {
  sp_env_table_t vars;
} sp_env_t;

typedef struct {
  sp_ps_io_mode_t mode;
  sp_io_stream_t stream;
  sp_ps_io_blocking_t block;
} sp_ps_io_stream_config_t;

typedef struct {
  sp_ps_io_stream_config_t in;
  sp_ps_io_stream_config_t out;
  sp_ps_io_stream_config_t err;
} sp_ps_io_config_t;

typedef sp_ps_io_config_t sp_ps_io_t;

typedef struct {
  sp_ps_env_mode_t mode;
  sp_env_t env;
  sp_env_var_t extra [SP_PS_MAX_ENV];
} sp_ps_env_config_t;

typedef struct {
  sp_str_t command;
  sp_str_t args [SP_PS_MAX_ARGS];
  sp_str_t cwd;
  sp_ps_env_config_t env;
  sp_ps_io_config_t io;
} sp_ps_config_t;

typedef struct {
  sp_str_t data;
  u64 size;
  s32 exit_code;
} sp_ps_read_result_t;

typedef struct {
  sp_ps_state_t state;
  s32 exit_code;
} sp_ps_status_t;

typedef struct {
  sp_str_t out;
  sp_str_t err;
  sp_ps_status_t status;
} sp_ps_output_t;

#if defined(SP_POSIX)
#define SP_POSIX_WAITPID_BLOCK 0
#define SP_POSIX_WAITPID_NO_BLOCK WNOHANG

typedef struct {
  posix_spawn_file_actions_t* fa;
  sp_ps_io_file_number_t file_number;
  s32 flag;
  s32 mode;
  struct {
    s32 read;
    s32 write;
  } pipes;
} sp_ps_posix_stdio_stream_config_t;

typedef struct {
  sp_ps_posix_stdio_stream_config_t in;
  sp_ps_posix_stdio_stream_config_t out;
  sp_ps_posix_stdio_stream_config_t err;
} sp_ps_posix_stdio_config_t;

typedef struct {
  s32 read;
  s32 write;
} sp_ps_pipe_t;

typedef struct {
  c8** argv;
  c8** envp;
  sp_ps_env_mode_t env_mode;
} sp_ps_platform_t;
#endif

// WIN32
#if defined(SP_WIN32)
typedef struct {
  void* placeholder;
} sp_ps_platform_t;
#endif

typedef struct {
  pid_t pid;
  sp_ps_io_t io;
  sp_ps_platform_t platform;
  sp_allocator_t allocator;
} sp_ps_t;

SP_API void            sp_env_init(sp_env_t* env);
SP_API sp_env_t        sp_env_capture();
SP_API sp_env_t        sp_env_copy(sp_env_t* env);
SP_API sp_str_t        sp_env_get(sp_env_t* env, sp_str_t name);
SP_API void            sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value);
SP_API void            sp_env_erase(sp_env_t* env, sp_str_t name);
SP_API void            sp_env_destroy(sp_env_t* env);
SP_API sp_str_t        sp_os_get_env_var(sp_str_t key);
SP_API sp_str_t        sp_os_get_env_as_path(sp_str_t key);
SP_API void            sp_os_clear_env_var(sp_str_t var);
SP_API void            sp_os_export_env_var(sp_str_t key, sp_str_t value, sp_env_export_overwrite_t overwrite);
SP_API void            sp_os_export_env(sp_env_t* env, sp_env_export_overwrite_t overwrite);
SP_API sp_ps_config_t  sp_ps_config_copy(const sp_ps_config_t* src);
SP_API void            sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg);
SP_API sp_ps_t         sp_ps_create(sp_ps_config_t config);
SP_API sp_ps_output_t  sp_ps_run(sp_ps_config_t config);
SP_API sp_io_stream_t* sp_ps_io_in(sp_ps_t* proc);
SP_API sp_io_stream_t* sp_ps_io_out(sp_ps_t* proc);
SP_API sp_io_stream_t* sp_ps_io_err(sp_ps_t* proc);
SP_API sp_ps_status_t  sp_ps_wait(sp_ps_t* proc);
SP_API sp_ps_status_t  sp_ps_poll(sp_ps_t* proc, u32 timeout_ms);
SP_API sp_ps_output_t  sp_ps_output(sp_ps_t* proc);
SP_API bool            sp_ps_kill(sp_ps_t* proc);

#if defined(SP_POSIX)
SP_IMP void sp_ps_set_cwd(posix_spawn_file_actions_t* fa, sp_str_t cwd);
SP_IMP bool sp_ps_create_pipes(s32 pipes [2]);
SP_IMP c8** sp_ps_build_posix_args(sp_ps_config_t* config);
SP_IMP void sp_ps_free_posix_args(c8** argv);
SP_IMP c8** sp_ps_build_posix_env(sp_ps_env_config_t* env_config);
SP_IMP void sp_ps_set_nonblocking(s32 fd);
SP_IMP void sp_ps_set_blocking(s32 fd);
#endif


// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║   ██║
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝

#define SP_FORMAT_TYPES \
 SP_FMT_X(ptr, void*) \
 SP_FMT_X(str, sp_str_t) \
 SP_FMT_X(cstr, const c8*) \
 SP_FMT_X(s8, s8) \
 SP_FMT_X(s16, s16) \
 SP_FMT_X(s32, s32) \
 SP_FMT_X(s64, s64) \
 SP_FMT_X(u8, u8) \
 SP_FMT_X(u16, u16) \
 SP_FMT_X(u32, u32) \
 SP_FMT_X(u64, u64) \
 SP_FMT_X(f32, f32) \
 SP_FMT_X(f64, f64) \
 SP_FMT_X(c8, c8) \
 SP_FMT_X(c16, c16) \
 SP_FMT_X(context, sp_context_t*) \
 SP_FMT_X(hash, sp_hash_t) \
 SP_FMT_X(hash_short, sp_hash_t) \
 SP_FMT_X(str_builder, sp_str_builder_t) \
 SP_FMT_X(fixed_array, sp_fixed_array_t) \
 SP_FMT_X(dynamic_array, sp_dynamic_array_t) \
 SP_FMT_X(quoted_str, sp_str_t) \
 SP_FMT_X(color, const c8*) \

#define SP_FMT_ID(id) SP_MACRO_CAT(sp_format_id_, id)
#define SP_FMT_FN(id) SP_MACRO_CAT(sp_fmt_format_, id)
#define SP_FMT_UNION(T) SP_MACRO_CAT(T, _value)

typedef enum {
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES
} sp_format_id_t;

typedef struct sp_format_arg_t {
  sp_format_id_t id;

  union {
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES
  };

} sp_format_arg_t;

SP_TYPEDEF_FN(void, sp_format_fn_t, sp_str_builder_t*, sp_format_arg_t*);

typedef struct sp_formatter {
  sp_format_id_t id;
  sp_format_fn_t fn;
} sp_formatter_t;


#define SP_FMT_ARG(T, V) SP_RVAL(sp_format_arg_t) { .id =  SP_FMT_ID(T), .SP_FMT_UNION(T) = (V) }

#define SP_FMT_PTR(V)           SP_FMT_ARG(ptr, V)
#define SP_FMT_STR(V)           SP_FMT_ARG(str, V)
#define SP_FMT_CSTR(V)          SP_FMT_ARG(cstr, V)
#define SP_FMT_S8(V)            SP_FMT_ARG(s8, V)
#define SP_FMT_S16(V)           SP_FMT_ARG(s16, V)
#define SP_FMT_S32(V)           SP_FMT_ARG(s32, V)
#define SP_FMT_S64(V)           SP_FMT_ARG(s64, V)
#define SP_FMT_U8(V)            SP_FMT_ARG(u8, V)
#define SP_FMT_U16(V)           SP_FMT_ARG(u16, V)
#define SP_FMT_U32(V)           SP_FMT_ARG(u32, V)
#define SP_FMT_U64(V)           SP_FMT_ARG(u64, V)
#define SP_FMT_F32(V)           SP_FMT_ARG(f32, V)
#define SP_FMT_F64(V)           SP_FMT_ARG(f64, V)
#define SP_FMT_C8(V)            SP_FMT_ARG(c8, V)
#define SP_FMT_C16(V)           SP_FMT_ARG(c16, V)
#define SP_FMT_CONTEXT(V)       SP_FMT_ARG(context, V)
#define SP_FMT_HASH(V)          SP_FMT_ARG(hash, V)
#define SP_FMT_SHORT_HASH(V)    SP_FMT_ARG(hash_short, V)
#define SP_FMT_STR_BUILDER(V)   SP_FMT_ARG(str_builder, V)
#define SP_FMT_DATE_TIME(V)     SP_FMT_ARG(date_time, V)
#define SP_FMT_THREAD(V)        SP_FMT_ARG(thread, V)
#define SP_FMT_MUTEX(V)         SP_FMT_ARG(mutex, V)
#define SP_FMT_SEMAPHORE(V)     SP_FMT_ARG(semaphore, V)
#define SP_FMT_FIXED_ARRAY(V)   SP_FMT_ARG(fixed_array, V)
#define SP_FMT_DYNAMIC_ARRAY(V) SP_FMT_ARG(dynamic_array, V)
#define SP_FMT_QUOTED_STR(V)    SP_FMT_ARG(quoted_str, V)
#define SP_FMT_COLOR(V)         SP_FMT_ARG(color, V)
#define SP_FMT_YELLOW()         SP_FMT_COLOR(SP_ANSI_FG_YELLOW)
#define SP_FMT_CYAN()           SP_FMT_COLOR(SP_ANSI_FG_CYAN)
#define SP_FMT_CLEAR()          SP_FMT_COLOR(SP_ANSI_FG_RESET)

#undef SP_FMT_X
#define SP_FMT_X(name, type) void sp_fmt_format_##name(sp_str_builder_t* builder, sp_format_arg_t* buffer);
SP_FORMAT_TYPES

sp_str_t sp_format_str(sp_str_t fmt, ...);
sp_str_t sp_format(const c8* fmt, ...);
sp_str_t sp_format_v(sp_str_t fmt, va_list args);

SP_API u8        sp_parse_u8(sp_str_t str);
SP_API u16       sp_parse_u16(sp_str_t str);
SP_API u32       sp_parse_u32(sp_str_t str);
SP_API u64       sp_parse_u64(sp_str_t str);
SP_API s8        sp_parse_s8(sp_str_t str);
SP_API s16       sp_parse_s16(sp_str_t str);
SP_API s32       sp_parse_s32(sp_str_t str);
SP_API s64       sp_parse_s64(sp_str_t str);
SP_API f32       sp_parse_f32(sp_str_t str);
SP_API f64       sp_parse_f64(sp_str_t str);
SP_API c8        sp_parse_c8(sp_str_t str);
SP_API c16       sp_parse_c16(sp_str_t str);
SP_API void*     sp_parse_ptr(sp_str_t str);
SP_API bool      sp_parse_bool(sp_str_t str);
SP_API sp_hash_t sp_parse_hash(sp_str_t str);
SP_API u64       sp_parse_hex(sp_str_t str);
SP_API bool      sp_parse_u8_ex(sp_str_t str, u8* out);
SP_API bool      sp_parse_u16_ex(sp_str_t str, u16* out);
SP_API bool      sp_parse_u32_ex(sp_str_t str, u32* out);
SP_API bool      sp_parse_u64_ex(sp_str_t str, u64* out);
SP_API bool      sp_parse_s8_ex(sp_str_t str, s8* out);
SP_API bool      sp_parse_s16_ex(sp_str_t str, s16* out);
SP_API bool      sp_parse_s32_ex(sp_str_t str, s32* out);
SP_API bool      sp_parse_s64_ex(sp_str_t str, s64* out);
SP_API bool      sp_parse_f32_ex(sp_str_t str, f32* out);
SP_API bool      sp_parse_f64_ex(sp_str_t str, f64* out);
SP_API bool      sp_parse_c8_ex(sp_str_t str, c8* out);
SP_API bool      sp_parse_c16_ex(sp_str_t str, c16* out);
SP_API bool      sp_parse_ptr_ex(sp_str_t str, void** out);
SP_API bool      sp_parse_bool_ex(sp_str_t str, bool* out);
SP_API bool      sp_parse_hash_ex(sp_str_t str, sp_hash_t* out);
SP_API bool      sp_parse_hex_ex(sp_str_t str, u64* out);
SP_API bool      sp_parse_is_digit(c8 c);


//  █████╗ ███████╗███████╗███████╗████████╗███████╗
// ██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝██╔════╝
// ███████║███████╗███████╗█████╗     ██║   ███████╗
// ██╔══██║╚════██║╚════██║██╔══╝     ██║   ╚════██║
// ██║  ██║███████║███████║███████╗   ██║   ███████║
// ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝   ╚══════╝

#ifdef SP_APP
typedef enum {
  SP_ASSET_STATE_QUEUED,
  SP_ASSET_STATE_IMPORTED,
  SP_ASSET_STATE_COMPLETED,
} sp_asset_state_t;

typedef enum {
  SP_ASSET_KIND_NONE,
} sp_builtin_asset_kind_t;

typedef u32 sp_asset_kind_t;

typedef struct sp_asset_registry sp_asset_registry_t;
typedef struct sp_asset_import_context sp_asset_import_context_t;

SP_TYPEDEF_FN(void, sp_asset_import_fn_t, sp_asset_import_context_t* context);
SP_TYPEDEF_FN(void, sp_asset_completion_fn_t, sp_asset_import_context_t* context);

typedef struct {
  sp_asset_kind_t kind;
  sp_asset_state_t state;
  sp_str_t name;
  void* data;
} sp_asset_t;

typedef struct {
  sp_asset_kind_t kind;
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
} sp_asset_importer_config_t;

typedef struct {
  sp_asset_kind_t kind;
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_registry_t* registry;
} sp_asset_importer_t;

struct sp_asset_import_context {
  sp_asset_registry_t* registry;
  sp_asset_importer_t* importer;
  u32 asset_index;
  sp_future_t* future;
  void* user_data;
};

#define sp_asset_import_context_get_asset(ctx) (&(ctx)->registry->assets[(ctx)->asset_index])

#define SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS 32
typedef struct {
  sp_asset_importer_config_t importers [SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS];
} sp_asset_registry_config_t;

struct sp_asset_registry {
  sp_mutex_t mutex;
  sp_mutex_t import_mutex;
  sp_mutex_t completion_mutex;
  sp_semaphore_t semaphore;
  sp_thread_t thread;
  bool shutdown_requested;

  sp_dyn_array(sp_asset_t) assets;
  sp_dyn_array(sp_asset_importer_t) importers;
  sp_ring_buffer(sp_asset_import_context_t) import_queue;
  sp_ring_buffer(sp_asset_import_context_t) completion_queue;
};

void                  sp_asset_registry_init(sp_asset_registry_t* registry, sp_asset_registry_config_t config);
void                  sp_asset_registry_shutdown(sp_asset_registry_t* registry);
sp_future_t*          sp_asset_registry_import(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
sp_asset_t*           sp_asset_registry_add(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
sp_asset_t*           sp_asset_registry_find(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name);
void                  sp_asset_registry_process_completions(sp_asset_registry_t* registry);
sp_asset_t*           sp_asset_registry_reserve(sp_asset_registry_t* registry);
sp_asset_importer_t*  sp_asset_registry_find_importer(sp_asset_registry_t* registry, sp_asset_kind_t kind);
s32                   sp_asset_registry_thread_fn(void* user_data);
#endif

SP_END_EXTERN_C()


//  ██████╗██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗
// ██║     ██████╔╝██████╔╝
// ██║     ██╔═══╝ ██╔═══╝
// ╚██████╗██║     ██║
//  ╚═════╝╚═╝     ╚═╝
#ifdef SP_CPP
  sp_str_t operator/(const sp_str_t& a, const sp_str_t& b);
  sp_str_t operator/(const sp_str_t& a, const c8* b);

  template <typename T>
  sp_format_arg_t sp_make_format_arg(sp_format_id_t id, T&& data) {
    sp_format_arg_t result = SP_ZERO_STRUCT(sp_format_arg_t);
    result.id = id;
    sp_os_copy_memory(&data, &result.u8_value, sizeof(data));

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
#ifndef SP_SP_C
#define SP_SP_C

#ifdef SP_IMPLEMENTATION
SP_BEGIN_EXTERN_C()

#define SP_SIZE_T_BITS  ((sizeof(size_t)) * 8)
#define SP_SIPHASH_C_ROUNDS 1
#define SP_SIPHASH_D_ROUNDS 1
#define sp_rotate_left(__V, __N)   (((__V) << (__N)) | ((__V) >> (SP_SIZE_T_BITS - (__N))))
#define sp_rotate_right(__V, __N)  (((__V) >> (__N)) | ((__V) << (SP_SIZE_T_BITS - (__N))))

sp_hash_t sp_hash_cstr(const c8* str) {
  const size_t prime = 31;

  sp_hash_t hash = 0;
  c8 c = 0;

  while ((c = *str++)) {
    hash = c + (hash * prime);
  }

  return hash;
}

sp_hash_t sp_hash_str(sp_str_t str) {
  return sp_hash_bytes(str.data, str.len, 0);
}

sp_hash_t sp_hash_bytes(const void *p, u64 len, u64 seed) {
  unsigned char *d = (unsigned char *) p;
  size_t i,j;
  size_t v0,v1,v2,v3, data;

  v0 = ((((size_t) 0x736f6d65 << 16) << 16) + 0x70736575) ^  seed;
  v1 = ((((size_t) 0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
  v2 = ((((size_t) 0x6c796765 << 16) << 16) + 0x6e657261) ^  seed;
  v3 = ((((size_t) 0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

  #define sp_sipround() \
    do {                   \
      v0 += v1; v1 = sp_rotate_left(v1, 13);  v1 ^= v0; v0 = sp_rotate_left(v0,SP_SIZE_T_BITS/2); \
      v2 += v3; v3 = sp_rotate_left(v3, 16);  v3 ^= v2;                                                 \
      v2 += v1; v1 = sp_rotate_left(v1, 17);  v1 ^= v2; v2 = sp_rotate_left(v2,SP_SIZE_T_BITS/2); \
      v0 += v3; v3 = sp_rotate_left(v3, 21);  v3 ^= v0;                                                 \
    } while (0)

  for (i=0; i+sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
    data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    data |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16;

    v3 ^= data;
    for (j=0; j < SP_SIPHASH_C_ROUNDS; ++j)
      sp_sipround();
    v0 ^= data;
  }
  data = len << (SP_SIZE_T_BITS-8);
  switch (len - i) {
    case 7: data |= ((size_t) d[6] << 24) << 24; SP_FALLTHROUGH();
    case 6: data |= ((size_t) d[5] << 20) << 20; SP_FALLTHROUGH();
    case 5: data |= ((size_t) d[4] << 16) << 16; SP_FALLTHROUGH();
    case 4: data |= (d[3] << 24); SP_FALLTHROUGH();
    case 3: data |= (d[2] << 16); SP_FALLTHROUGH();
    case 2: data |= (d[1] << 8); SP_FALLTHROUGH();
    case 1: data |= d[0]; SP_FALLTHROUGH();
    case 0: break;
  }
  v3 ^= data;
  for (j=0; j < SP_SIPHASH_C_ROUNDS; ++j)
    sp_sipround();
  v0 ^= data;
  v2 ^= 0xff;
  for (j=0; j < SP_SIPHASH_D_ROUNDS; ++j)
    sp_sipround();

  return v1^v2^v3;
}

sp_hash_t sp_hash_combine(sp_hash_t* hashes, u32 num_hashes) {
  return sp_hash_bytes(hashes, num_hashes * sizeof(sp_hash_t), 0);
}

bool sp_ht_on_compare_key(void* ka, void* kb, u32 size) {
  return sp_os_is_memory_equal(ka, kb, size);
}

sp_hash_t sp_ht_on_hash_key(void *key, u32 size) {
  return sp_hash_bytes(key, size, SP_HT_HASH_SEED);
}

sp_hash_t sp_ht_on_hash_str_key(void* key, u32 size) {
  sp_str_t* str = (sp_str_t*) key;
  return sp_hash_str(*str);
}

bool sp_ht_on_compare_str_key(void* ka, void* kb, u32 size) {
  sp_str_t* sa = (sp_str_t*) ka;
  sp_str_t* sb = (sp_str_t*) kb;
  return sp_str_equal(*sa, *sb);
}

u32 sp_ht_get_key_index_fn(void** data, void* key, sp_ht_info_t info) {
    if (!data || !key) return SP_HT_INVALID_INDEX;

    u32 capacity = sp_dyn_array_capacity(*data);
    u32 size = sp_dyn_array_size(*data);
    if (!capacity || !size) return SP_HT_INVALID_INDEX;
    u32 idx = SP_HT_INVALID_INDEX;
    sp_hash_t hash = info.fn.hash(key, info.size.key);
    u32 hash_idx = (hash % capacity);

    for (u32 i = hash_idx, c = 0; c < capacity; ++c, i = ((i + 1) % capacity)) {
        u32 offset = (i * info.stride);
        void* k = ((c8*)(*data) + (offset));
        sp_hash_t kh = info.fn.hash(k, info.size.key);
        bool equal = info.fn.compare(k, key, info.size.key);
        sp_ht_entry_state state = *(sp_ht_entry_state*)((c8*)(*data) + offset + (info.klpvl));
        if (equal && hash == kh && state == SP_HT_ENTRY_ACTIVE) {
            idx = i;
            break;
        }
    }
    return idx;
}

sp_ht_it sp_ht_it_init_fn(void** data, sp_ht_info_t info) {
    if (!data || !*data) return 0;
    sp_ht_it it = 0;
    for (; it < sp_dyn_array_capacity(*data); ++it) {
        u32 offset = (it * info.stride);
        sp_ht_entry_state state = *(sp_ht_entry_state*)((u8*)*data + offset + (info.klpvl));
        if (state == SP_HT_ENTRY_ACTIVE) {
            break;
        }
    }
    return it;
}

void sp_ht_it_advance_fn(void** data, u32* it, sp_ht_info_t info) {
    if (!data || !*data) return;
    (*it)++;
    for (; *it < sp_dyn_array_capacity(*data); ++*it) {
        u32 offset = (*it * info.stride);
        sp_ht_entry_state state = *(sp_ht_entry_state*)((u8*)*data + offset + (info.klpvl));
        if (state == SP_HT_ENTRY_ACTIVE) {
            break;
        }
    }
}


////////////////////////////
// DYN ARRAY IMPLEMENTATION
////////////////////////////
void* sp_dyn_array_resize_impl(void* arr, u32 sz, u32 amount) {
    u32 capacity;

    if (arr) {
        capacity = amount;
    } else {
        capacity = 0;
    }

    sp_dyn_array* data = (sp_dyn_array*)sp_realloc(arr ? sp_dyn_array_head(arr) : 0, capacity * sz + sizeof(sp_dyn_array));

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (s32)capacity;
        return ((s32*)data + 2);
    }

    return NULL;
}

void** sp_dyn_array_init(void** arr, u32 val_len) {
    if (*arr == NULL) {
        sp_dyn_array* data = (sp_dyn_array*)sp_alloc(val_len + sizeof(sp_dyn_array));
        data->size = 0;
        data->capacity = 1;
        *arr = ((s32*)data + 2);
    }
    return arr;
}

void sp_dyn_array_push_f(void** arr, void* val, u32 val_len) {
    sp_dyn_array_init(arr, val_len);
    if (!(*arr) || sp_dyn_array_need_grow(*arr, 1)) {
        u32 new_capacity = sp_dyn_array_capacity(*arr);
        if (new_capacity == 0) {
            new_capacity = 1;
        } else {
            new_capacity *= 2;
        }
        *arr = sp_dyn_array_resize_impl(*arr, val_len, new_capacity);
    }
    if (*arr) {
        sp_os_copy_memory(val, ((u8*)(*arr)) + sp_dyn_array_size(*arr) * val_len, val_len);
        sp_dyn_array_head(*arr)->size++;
    }
}

bool sp_parse_u64_ex(sp_str_t str, u64* out) {
    if (str.len == 0) return false;

    u64 result = 0;
    for (u32 i = 0; i < str.len; i++) {
        c8 ch = str.data[i];
        if (ch < '0' || ch > '9') return false;

        u64 digit = ch - '0';
        if (result > (UINT64_MAX - digit) / 10) return false; // overflow check
        result = result * 10 + digit;
    }

    *out = result;
    return true;
}

bool sp_parse_s64_ex(sp_str_t str, s64* out) {
    if (str.len == 0) return false;

    bool negative = false;
    u32 start = 0;

    if (str.data[0] == '-') {
        negative = true;
        start = 1;
    } else if (str.data[0] == '+') {
        start = 1;
    }

    if (start >= str.len) return false;

    sp_str_t num_str = sp_str(str.data + start, str.len - start);
    u64 abs_value;
    if (!sp_parse_u64_ex(num_str, &abs_value)) return false;

    if (negative) {
        if (abs_value > (u64)INT64_MAX + 1) return false; // overflow
        *out = -(s64)abs_value;
    } else {
        if (abs_value > INT64_MAX) return false; // overflow
        *out = (s64)abs_value;
    }

    return true;
}

bool sp_parse_u32_ex(sp_str_t str, u32* out) {
    u64 val;
    if (!sp_parse_u64_ex(str, &val)) return false;
    if (val > UINT32_MAX) return false;
    *out = (u32)val;
    return true;
}

bool sp_parse_s32_ex(sp_str_t str, s32* out) {
    s64 val;
    if (!sp_parse_s64_ex(str, &val)) return false;
    if (val < INT32_MIN || val > INT32_MAX) return false;
    *out = (s32)val;
    return true;
}

bool sp_parse_u16_ex(sp_str_t str, u16* out) {
    u64 val;
    if (!sp_parse_u64_ex(str, &val)) return false;
    if (val > UINT16_MAX) return false;
    *out = (u16)val;
    return true;
}

bool sp_parse_s16_ex(sp_str_t str, s16* out) {
    s64 val;
    if (!sp_parse_s64_ex(str, &val)) return false;
    if (val < INT16_MIN || val > INT16_MAX) return false;
    *out = (s16)val;
    return true;
}

bool sp_parse_u8_ex(sp_str_t str, u8* out) {
    u64 val;
    if (!sp_parse_u64_ex(str, &val)) return false;
    if (val > UINT8_MAX) return false;
    *out = (u8)val;
    return true;
}

bool sp_parse_s8_ex(sp_str_t str, s8* out) {
    s64 val;
    if (!sp_parse_s64_ex(str, &val)) return false;
    if (val < INT8_MIN || val > INT8_MAX) return false;
    *out = (s8)val;
    return true;
}

bool sp_parse_is_digit(c8 c) {
    return c >= '0' && c <= '9';
}

bool sp_parse_f32_ex(sp_str_t str, f32* out) {
    size_t i = 0;
    int sign = 1;
    f32 value = 0.0f;
    f32 scale = 1.0f;
    int exponent = 0;
    int exp_sign = 1;
    bool has_digits = false;

    if (i < str.len && (str.data[i] == '-' || str.data[i] == '+')) {
        if (str.data[i] == '-') sign = -1;
        i++;
    }

    while (i < str.len && sp_parse_is_digit(str.data[i])) {
        has_digits = true;
        value = value * 10.0f + (f32)(str.data[i] - '0');
        i++;
    }

    if (i < str.len && str.data[i] == '.') {
        i++;
        while (i < str.len && sp_parse_is_digit(str.data[i])) {
            has_digits = true;
            scale /= 10.0f;
            value += (f32)(str.data[i] - '0') * scale;
            i++;
        }
    }

    if (i < str.len && (str.data[i] == 'e' || str.data[i] == 'E')) {
        i++;
        if (i < str.len && (str.data[i] == '-' || str.data[i] == '+')) {
            if (str.data[i] == '-') exp_sign = -1;
            i++;
        }
        if (i >= str.len || !sp_parse_is_digit(str.data[i])) {
            return false;
        }
        while (i < str.len && sp_parse_is_digit(str.data[i])) {
            exponent = exponent * 10 + (str.data[i] - '0');
            i++;
        }
        exponent *= exp_sign;
    }

    if (i != str.len || !has_digits) {
        return false;
    }

    if (exponent > 0) {
        for (int j = 0; j < exponent; j++) {
            value *= 10.0f;
        }
    } else if (exponent < 0) {
        for (int j = 0; j < -exponent; j++) {
            value /= 10.0f;
        }
    }

    *out = sign * value;
    return true;
}

bool sp_parse_f64_ex(sp_str_t str, f64* out) {
  f32 hack = 0.0f;
  bool result = sp_parse_f32_ex(str, &hack);
  *out = hack;
  return result;
}

bool sp_parse_ptr_ex(sp_str_t str, void** out) {
    u64 addr;
    if (!sp_parse_hex_ex(str, &addr)) return false;
    *out = (void*)(uintptr_t)addr;
    return true;
}

bool sp_parse_c8_ex(sp_str_t str, c8* out) {
    // handle 'a' format
    if (str.len == 3 && str.data[0] == '\'' && str.data[2] == '\'') {
        *out = str.data[1];
        return true;
    }
    // handle plain character
    if (str.len == 1) {
        *out = str.data[0];
        return true;
    }
    return false;
}

bool sp_parse_c16_ex(sp_str_t str, c16* out) {
    // handle 'a' format
    if (str.len == 3 && str.data[0] == '\'' && str.data[2] == '\'') {
        *out = (c16)str.data[1];
        return true;
    }
    // handle 'U+XXXX' format
    if (str.len >= 8 && str.data[0] == '\'' && str.data[1] == 'U' &&
        str.data[2] == '+' && str.data[str.len-1] == '\'') {
        sp_str_t hex_str = sp_str(str.data + 3, str.len - 4);
        u64 val;
        if (sp_parse_hex_ex(hex_str, &val) && val <= UINT16_MAX) {
            *out = (c16)val;
            return true;
        }
    }
    return false;
}

bool sp_parse_hex_ex(sp_str_t str, u64* out) {
    if (str.len == 0) return false;

    u32 start = 0;

    // skip 0x prefix if present
    if (str.len >= 2 && str.data[0] == '0' && (str.data[1] == 'x' || str.data[1] == 'X')) {
        start = 2;
    }

    if (start >= str.len) return false;

    u64 result = 0;
    for (u32 i = start; i < str.len; i++) {
        c8 ch = str.data[i];
        u8 digit;

        if (ch >= '0' && ch <= '9') {
            digit = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            digit = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'F') {
            digit = ch - 'A' + 10;
        } else {
            return false;
        }

        if (result > (UINT64_MAX >> 4)) return false; // overflow check
        result = (result << 4) | digit;
    }

    *out = result;
    return true;
}

bool sp_parse_hash_ex(sp_str_t str, sp_hash_t* out) {
    return sp_parse_hex_ex(str, out);
}

bool sp_parse_bool_ex(sp_str_t str, bool* out) {
    if (sp_str_equal(str, SP_LIT("true")) || sp_str_equal(str, SP_LIT("1"))) {
        *out = true;
        return true;
    }
    if (sp_str_equal(str, SP_LIT("false")) || sp_str_equal(str, SP_LIT("0"))) {
        *out = false;
        return true;
    }
    return false;
}

u8 sp_parse_u8(sp_str_t str) {
  u8 value = SP_ZERO_INITIALIZE();
  sp_parse_u8_ex(str, &value);
  return value;
}

u16 sp_parse_u16(sp_str_t str) {
  u16 value = SP_ZERO_INITIALIZE();
  sp_parse_u16_ex(str, &value);
  return value;
}

u32 sp_parse_u32(sp_str_t str) {
  u32 value = SP_ZERO_INITIALIZE();
  sp_parse_u32_ex(str, &value);
  return value;
}

u64 sp_parse_u64(sp_str_t str) {
  u64 value = SP_ZERO_INITIALIZE();
  sp_parse_u64_ex(str, &value);
  return value;
}

s8 sp_parse_s8(sp_str_t str) {
  s8 value = SP_ZERO_INITIALIZE();
  sp_parse_s8_ex(str, &value);
  return value;
}

s16 sp_parse_s16(sp_str_t str) {
  s16 value = SP_ZERO_INITIALIZE();
  sp_parse_s16_ex(str, &value);
  return value;
}

s32 sp_parse_s32(sp_str_t str) {
  s32 value = SP_ZERO_INITIALIZE();
  sp_parse_s32_ex(str, &value);
  return value;
}

s64 sp_parse_s64(sp_str_t str) {
  s64 value = SP_ZERO_INITIALIZE();
  sp_parse_s64_ex(str, &value);
  return value;
}

f32 sp_parse_f32(sp_str_t str) {
  f32 value = SP_ZERO_INITIALIZE();
  sp_parse_f32_ex(str, &value);
  return value;
}

f64 sp_parse_f64(sp_str_t str) {
  f64 value = SP_ZERO_INITIALIZE();
  sp_parse_f64_ex(str, &value);
  return value;
}

c8 sp_parse_c8(sp_str_t str) {
  c8 value = SP_ZERO_INITIALIZE();
  sp_parse_c8_ex(str, &value);
  return value;
}

c16 sp_parse_c16(sp_str_t str) {
  c16 value = SP_ZERO_INITIALIZE();
  sp_parse_c16_ex(str, &value);
  return value;
}

u64 sp_parse_hex(sp_str_t str) {
  u64 value = SP_ZERO_INITIALIZE();
  sp_parse_hex_ex(str, &value);
  return value;
}

void* sp_parse_ptr(sp_str_t str) {
  void* value = SP_ZERO_INITIALIZE();
  sp_parse_ptr_ex(str, &value);
  return value;
}

bool sp_parse_bool(sp_str_t str) {
  bool value = SP_ZERO_INITIALIZE();
  sp_parse_bool_ex(str, &value);
  return value;
}

sp_hash_t sp_parse_hash(sp_str_t str) {
  sp_hash_t value = SP_ZERO_INITIALIZE();
  sp_parse_hash_ex(str, &value);
  return value;
}


void sp_fmt_format_unsigned(sp_str_builder_t* builder, u64 num, u32 max_digits) {
    SP_ASSERT(builder);

    if (num == 0) {
        sp_str_builder_grow(builder, builder->buffer.len + 1);
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
    sp_str_builder_grow(builder, builder->buffer.len + digit_count);

    for (s32 i = digit_count - 1; i >= 0; i--) {
        sp_str_builder_append_c8(builder, digits[i]);
    }
}

void sp_fmt_format_signed(sp_str_builder_t* builder, s64 num, u32 max_digits) {
    SP_ASSERT(builder);

    bool negative = num < 0;
    u64 abs_value;

    if (negative) {
        // Handle INT_MIN properly by casting to unsigned first
        abs_value = (u64)(-(num + 1)) + 1;
        sp_str_builder_grow(builder, builder->buffer.len + 1);
        sp_str_builder_append_c8(builder, '-');
    } else {
        abs_value = (u64)num;
    }

    sp_fmt_format_unsigned(builder, abs_value, max_digits);
}

void sp_fmt_format_hex(sp_str_builder_t* builder, u64 value, u32 min_width, const c8* prefix) {
    SP_ASSERT(builder);

    if (prefix) {
        sp_str_builder_append_cstr(builder, prefix);
    }

    if (value == 0) {
        u32 zero_count = min_width > 0 ? min_width : 1;
        sp_str_builder_grow(builder, builder->buffer.len + zero_count);
        for (u32 i = 0; i < zero_count; i++) {
            sp_str_builder_append_c8(builder, '0');
        }
        return;
    }

    c8 hex_digits[16]; // max 16 hex digits for 64-bit
    s32 digit_count = 0;

    while (value > 0) {
        u8 digit = value & 0xF;
        hex_digits[digit_count++] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
        value >>= 4;
    }

    // Pad to minimum width
    while (digit_count < (s32)min_width) {
        hex_digits[digit_count++] = '0';
    }

    sp_str_builder_grow(builder, builder->buffer.len + digit_count);

    for (s32 i = digit_count - 1; i >= 0; i--) {
        sp_str_builder_append_c8(builder, hex_digits[i]);
    }
}
void sp_fmt_format_color(sp_str_builder_t *builder, sp_format_arg_t *buffer) {
  SP_ASSERT(builder);
  sp_str_builder_append_cstr(builder, buffer->color_value);
}

void sp_fmt_format_str(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_str_t value = arg->str_value;
  SP_ASSERT(builder);

  sp_str_builder_append(builder, value);
}

void sp_fmt_format_cstr(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  const c8* value = arg->cstr_value;
  SP_ASSERT(builder);
  SP_ASSERT(value);

  sp_str_builder_append_cstr(builder, value);
}

void sp_fmt_format_ptr(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  void* value = arg->ptr_value;
  u64 addr = (u64)value;
  sp_fmt_format_hex(builder, addr, 8, "0x");
}

void sp_fmt_format_s8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s8 value = arg->s8_value;
  sp_fmt_format_signed(builder, value, 3);
}

void sp_fmt_format_s16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s16 value = arg->s16_value;
  sp_fmt_format_signed(builder, value, 5);
}

void sp_fmt_format_s32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s32 value = arg->s32_value;
  sp_fmt_format_signed(builder, value, 10);
}

void sp_fmt_format_s64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  s64 value = arg->s64_value;
  sp_fmt_format_signed(builder, value, 20);
}

void sp_fmt_format_u8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u8 value = arg->u8_value;
  sp_fmt_format_unsigned(builder, value, 3);
}

void sp_fmt_format_u16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u16 value = arg->u16_value;
  sp_fmt_format_unsigned(builder, value, 5);
}

void sp_fmt_format_u32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u32 value = arg->u32_value;
  sp_fmt_format_unsigned(builder, value, 10);
}

void sp_fmt_format_u64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  u64 value = arg->u64_value;
  sp_fmt_format_unsigned(builder, value, 20);
}

void sp_fmt_format_f32(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  f32 value = arg->f32_value;
  f32 num = value;

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

void sp_fmt_format_f64(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  f64 value = arg->f64_value;
  f64 num = value;

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

void sp_fmt_format_c8(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  SP_ASSERT(builder);
  sp_str_builder_append_c8(builder, arg->c8_value);
}

void sp_fmt_format_c16(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  c16 value = arg->c16_value;
  SP_ASSERT(builder);

  if (value < 128) {
    sp_str_builder_append_c8(builder, (c8)value);
  }
  else {
    sp_str_builder_append_c8(builder, 'U');
    sp_str_builder_append_c8(builder, '+');
    sp_fmt_format_hex(builder, value, 4, SP_NULL);
  }
}

void sp_fmt_format_context(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  // Context is passed as pointer but we don't use it
  sp_context_t* ctx = sp_context_get();
  if (ctx) {
    sp_context_t* stack = (sp_context_t*)pthread_getspecific(sp_context_stack_key);
    u32 index = (u32)(ctx - stack);
    sp_fmt_format_unsigned(builder, index, 10);
  } else {
    sp_str_builder_append_cstr(builder, "NULL");
  }
}

void sp_fmt_format_hash(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_hash_t value = arg->hash_value;
  u64 hash = (u64)value;
  sp_fmt_format_hex(builder, hash, 0, NULL);
}

void sp_fmt_format_hash_short(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_hash_t value = arg->hash_short_value;
  u64 hash = (u64)value;
  sp_fmt_format_hex(builder, hash >> 32, 0, NULL);
}

void sp_fmt_format_str_builder(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_str_builder_t sb = arg->str_builder_value;

  sp_str_builder_append_cstr(builder, "{ buffer: (");

  // Format data pointer
  u64 addr = (u64)sb.buffer.data;
  sp_fmt_format_hex(builder, addr, 8, "0x");

  sp_str_builder_append_cstr(builder, ", ");

  // Format count
  sp_fmt_format_unsigned(builder, sb.buffer.len, 10);

  sp_str_builder_append_cstr(builder, "), capacity: ");

  // Format capacity
  sp_fmt_format_unsigned(builder, sb.buffer.capacity, 10);

  sp_str_builder_append_cstr(builder, " }");
}

void sp_fmt_format_fixed_array(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_fixed_array_t arr = arg->fixed_array_value;

  sp_str_builder_append_cstr(builder, "{ size: ");
  sp_fmt_format_unsigned(builder, arr.size, 10);
  sp_str_builder_append_cstr(builder, ", capacity: ");
  sp_fmt_format_unsigned(builder, arr.capacity, 10);
  sp_str_builder_append_cstr(builder, " }");
}

void sp_fmt_format_dynamic_array(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_dynamic_array_t arr = arg->dynamic_array_value;

  sp_str_builder_append_cstr(builder, "{ size: ");
  sp_fmt_format_unsigned(builder, arr.size, 10);
  sp_str_builder_append_cstr(builder, ", capacity: ");
  sp_fmt_format_unsigned(builder, arr.capacity, 10);
  sp_str_builder_append_cstr(builder, " }");
}

void sp_fmt_format_quoted_str(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  sp_str_t value = arg->quoted_str_value;
  SP_ASSERT(builder);

  sp_str_builder_append_c8(builder, '"');
  sp_str_builder_append(builder, value);
  sp_str_builder_append_c8(builder, '"');
}

sp_str_t sp_fmt(sp_str_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t str = sp_format_v(fmt, args);
  va_end(args);

  return str;
}

sp_str_t sp_format_str(sp_str_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t str = sp_format_v(fmt, args);
  va_end(args);

  return str;
}

sp_str_t sp_format(const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t str = sp_format_v(SP_CSTR(fmt), args);
  va_end(args);

  return str;
}

typedef enum {
  SP_FORMAT_SPECIFIER_FLAG_NONE = 0,
  SP_FORMAT_SPECIFIER_FLAG_FG_COLOR = 1 << 0,
  SP_FORMAT_SPECIFIER_FLAG_BG_COLOR = 1 << 1,
  SP_FORMAT_SPECIFIER_FLAG_PAD = 1 << 2,
} sp_format_specifier_flag_t;


typedef struct {
  sp_str_t fmt;
  u32 it;
} sp_format_parser_t;

typedef struct {
  u32 flags;
  sp_str_t color;
  u32 pad;
} sp_format_specifier_t;

c8 sp_format_parser_peek(sp_format_parser_t* parser) {
  return sp_str_at(parser->fmt, parser->it);
}

void sp_format_parser_eat(sp_format_parser_t* parser) {
  parser->it++;
}

void sp_format_parser_eat_and_assert(sp_format_parser_t* parser, c8 c) {
  SP_ASSERT(sp_format_parser_peek(parser) == c);
  sp_format_parser_eat(parser);
}

bool sp_format_parser_is_alpha(sp_format_parser_t* parser) {
  c8 c = sp_format_parser_peek(parser);
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  return false;
}

bool sp_format_parser_is_alphanumeric(sp_format_parser_t* parser) {
  c8 c = sp_format_parser_peek(parser);
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  if (c >= '0' && c <= '9') return true;
  return false;
}

bool sp_format_parser_is_done(sp_format_parser_t* parser) {
  return parser->it >= parser->fmt.len;
}

sp_str_t sp_format_parser_id(sp_format_parser_t* parser) {
  sp_str_t id = sp_str_sub(parser->fmt, parser->it, 0);
  while (sp_format_parser_is_alpha(parser)) {
    sp_format_parser_eat(parser);
    id.len++;
  }
  return id;
}

sp_str_t sp_format_parser_value(sp_format_parser_t* parser) {
  sp_str_t value = sp_str_sub(parser->fmt, parser->it, 0);
  while (sp_format_parser_is_alphanumeric(parser)) {
    sp_format_parser_eat(parser);
    value.len++;
  }
  return value;
}

sp_format_specifier_flag_t sp_format_specifier_flag_from_str(sp_str_t id) {
  if (sp_str_equal_cstr(id, "color")) return SP_FORMAT_SPECIFIER_FLAG_FG_COLOR;
  if (sp_str_equal_cstr(id, "fg")) return SP_FORMAT_SPECIFIER_FLAG_FG_COLOR;
  if (sp_str_equal_cstr(id, "bg")) return SP_FORMAT_SPECIFIER_FLAG_BG_COLOR;
  if (sp_str_equal_cstr(id, "pad")) return SP_FORMAT_SPECIFIER_FLAG_PAD;
  return SP_FORMAT_SPECIFIER_FLAG_NONE;
}

sp_str_t sp_format_color_id_to_ansi_fg(sp_str_t id) {
  if (sp_str_equal_cstr(id, "black")) return SP_CSTR(SP_ANSI_FG_BLACK);
  if (sp_str_equal_cstr(id, "red")) return SP_CSTR(SP_ANSI_FG_RED);
  if (sp_str_equal_cstr(id, "green")) return SP_CSTR(SP_ANSI_FG_GREEN);
  if (sp_str_equal_cstr(id, "yellow")) return SP_CSTR(SP_ANSI_FG_YELLOW);
  if (sp_str_equal_cstr(id, "blue")) return SP_CSTR(SP_ANSI_FG_BLUE);
  if (sp_str_equal_cstr(id, "magenta")) return SP_CSTR(SP_ANSI_FG_MAGENTA);
  if (sp_str_equal_cstr(id, "cyan")) return SP_CSTR(SP_ANSI_FG_CYAN);
  if (sp_str_equal_cstr(id, "white")) return SP_CSTR(SP_ANSI_FG_WHITE);
  if (sp_str_equal_cstr(id, "brightblack")) return SP_CSTR(SP_ANSI_FG_BRIGHT_BLACK);
  if (sp_str_equal_cstr(id, "brightred")) return SP_CSTR(SP_ANSI_FG_BRIGHT_RED);
  if (sp_str_equal_cstr(id, "brightgreen")) return SP_CSTR(SP_ANSI_FG_BRIGHT_GREEN);
  if (sp_str_equal_cstr(id, "brightyellow")) return SP_CSTR(SP_ANSI_FG_BRIGHT_YELLOW);
  if (sp_str_equal_cstr(id, "brightblue")) return SP_CSTR(SP_ANSI_FG_BRIGHT_BLUE);
  if (sp_str_equal_cstr(id, "brightmagenta")) return SP_CSTR(SP_ANSI_FG_BRIGHT_MAGENTA);
  if (sp_str_equal_cstr(id, "brightcyan")) return SP_CSTR(SP_ANSI_FG_BRIGHT_CYAN);
  if (sp_str_equal_cstr(id, "brightwhite")) return SP_CSTR(SP_ANSI_FG_BRIGHT_WHITE);
  return SP_CSTR(SP_ANSI_RESET);
}

sp_format_specifier_t sp_format_parser_specifier(sp_format_parser_t* parser) {
  sp_format_specifier_t spec = SP_ZERO_INITIALIZE();

  while (!sp_format_parser_is_done(parser)) {
    c8 c = sp_format_parser_peek(parser);
    if (c != ':') {
      break;
    }

    sp_format_parser_eat_and_assert(parser, ':');
    sp_str_t id = sp_format_parser_id(parser);
    sp_format_parser_eat_and_assert(parser, ' ');
    sp_str_t value = sp_format_parser_value(parser);

    sp_format_specifier_flag_t flag = sp_format_specifier_flag_from_str(id);
    switch (flag) {
      case SP_FORMAT_SPECIFIER_FLAG_FG_COLOR: {
        spec.color = sp_format_color_id_to_ansi_fg(value);
        break;
      }
      case SP_FORMAT_SPECIFIER_FLAG_PAD: {
        spec.pad = sp_parse_u32(value);
        break;
      }
      default: {
        SP_UNREACHABLE_CASE();
      }
    }

    spec.flags |= flag;

    if (!sp_format_parser_is_done(parser) && sp_format_parser_peek(parser) == ' ') {
      sp_format_parser_eat(parser);
    }
  }

  return spec;
}

sp_str_t sp_format_v(sp_str_t fmt, va_list args) {
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) (sp_formatter_t) { .id = SP_FMT_ID(ID), .fn = SP_FMT_FN(ID) },

  static sp_formatter_t formatters [] = {
    SP_FORMAT_TYPES
  };

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_format_parser_t parser = SP_ZERO_INITIALIZE();
  parser.fmt = fmt;
  while (true) {
    if (sp_format_parser_is_done(&parser)) {
      break;
    }

    c8 c = sp_format_parser_peek(&parser);
    switch (c) {
      case '{': {
        sp_format_parser_eat(&parser);
        sp_format_specifier_t specifier = sp_format_parser_specifier(&parser);
        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
          sp_str_builder_append(&builder, specifier.color);
        }
        sp_format_parser_eat_and_assert(&parser, '}');

        sp_format_arg_t arg = va_arg(args, sp_format_arg_t);
        u32 formatted_value_start = builder.buffer.len;
        SP_CARR_FOR(formatters, i) {
          if (arg.id == formatters[i].id) {
            sp_format_fn_t fn = formatters[i].fn;
            fn(&builder, &arg);
            break;
          }
        }
        sp_str_t formatted_value = sp_str(builder.buffer.data + formatted_value_start, builder.buffer.len - formatted_value_start);

        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_PAD) {
          if (formatted_value.len < specifier.pad) {
            u32 spaces_needed = specifier.pad - formatted_value.len;
            for (u32 i = 0; i < spaces_needed; i++) {
              sp_str_builder_append_c8(&builder, ' ');
            }
          }
        }

        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
          sp_str_builder_append_cstr(&builder, SP_ANSI_RESET);
        }
        else if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_BG_COLOR) {
          sp_str_builder_append_cstr(&builder, SP_ANSI_RESET);
        }

        break;
      }
      default: {
        sp_str_builder_append_c8(&builder, c);
        sp_format_parser_eat(&parser);
        break;
      }
    }
  }

  return sp_str_builder_write(&builder);
}

void sp_log(sp_str_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t formatted = sp_format_v(fmt, args);
  va_end(args);

  sp_os_log(formatted);
}

void sp_context_check_index() {
  sp_context_t* ctx = sp_context_get();
  sp_context_t* stack = (sp_context_t*)pthread_getspecific(sp_context_stack_key);
  u32 index = (u32)(ctx - stack);
  SP_ASSERT(index);
  SP_ASSERT(index < SP_MAX_CONTEXT);
}

void sp_context_ensure() {
  if (!sp_context_get()) sp_init(SP_ZERO_STRUCT(sp_config_t));
}

void sp_context_set(sp_context_t context) {
  sp_context_ensure();
  sp_context_t* ctx = sp_context_get();
  sp_context_t* stack = (sp_context_t*)pthread_getspecific(sp_context_stack_key);
  ctx = ctx ? ctx : &stack[1];
  *ctx = context;
  pthread_setspecific(sp_context_key, ctx);
}

void sp_context_push(sp_context_t context) {
  sp_context_ensure();

  sp_context_t* ctx = sp_context_get();
  ctx++;
  *ctx = context;
  pthread_setspecific(sp_context_key, ctx);
  sp_context_check_index();
}

void sp_context_push_allocator(sp_allocator_t allocator) {
  sp_context_ensure();

  sp_context_t* ctx = sp_context_get();
  sp_context_t context = *ctx;
  context.allocator = allocator;
  sp_context_push(context);
}

void sp_context_pop() {
  sp_context_t* ctx = sp_context_get();
  SP_ASSERT(ctx);
  sp_context_check_index();
  *ctx = SP_ZERO_STRUCT(sp_context_t); // Not required, just for the debugger
  ctx--;
  pthread_setspecific(sp_context_key, ctx);
}

void* sp_alloc(u32 size) {
  sp_context_ensure();
  sp_context_t* ctx = sp_context_get();
  return sp_allocator_alloc(ctx->allocator, size);
}

void* sp_realloc(void* memory, u32 size) {
  sp_context_ensure();
  sp_context_t* ctx = sp_context_get();
  return sp_allocator_realloc(ctx->allocator, memory, size);
}

void sp_free(void* memory) {
  sp_context_ensure();
  sp_context_t* ctx = sp_context_get();
  sp_allocator_free(ctx->allocator, memory);
}

sp_allocator_t sp_allocator_default() {
  return sp_malloc_allocator_init();
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
      sp_os_move_memory(old_memory, memory_block, size);
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

void* sp_malloc_allocator_on_alloc(void* user_data, sp_allocator_mode_t mode, u32 size, void* ptr) {
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      u32 total_size = size + sizeof(sp_malloc_metadata_t);
      sp_malloc_metadata_t* metadata = (sp_malloc_metadata_t*)sp_os_allocate_memory(total_size);
      metadata->size = size;
      return metadata + 1;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      if (!ptr) {
        return sp_malloc_allocator_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      }

      sp_malloc_metadata_t* metadata = sp_malloc_allocator_get_metadata(ptr);
      if (metadata->size >= size) {
        return ptr;
      }

      void* buffer = sp_malloc_allocator_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      sp_os_copy_memory(ptr, buffer, metadata->size);
      sp_malloc_allocator_on_alloc(user_data, SP_ALLOCATOR_MODE_FREE, 0, ptr);

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

sp_allocator_t sp_malloc_allocator_init() {
  sp_allocator_t allocator;
  allocator.on_alloc = sp_malloc_allocator_on_alloc;
  allocator.user_data = NULL;
  return allocator;
}


////////////
// STRING //
////////////
c8* sp_cstr_copy_sized(const c8* str, u32 length) {
  u32 buffer_length = length + 1;
  c8* copy = (c8*)sp_alloc(buffer_length);
  sp_cstr_copy_to_sized(str, length, copy, buffer_length);
  return copy;
}

c8* sp_cstr_copy(const c8* str) {
  return sp_cstr_copy_sized(str, sp_cstr_len(str));
}

void sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length) {
  sp_cstr_copy_to_sized(str, sp_cstr_len(str), buffer, buffer_length);
}

void sp_cstr_copy_to_sized(const c8* str, u32 length, c8* buffer, u32 buffer_length) {
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

u32 sp_cstr_len_sized(const c8* str, u32 n) {
  u32 len = 0;
  if (str) {
    while (len < n && str[len]) len++;
  }

  return len;
}

c8* sp_wstr_to_cstr(c16* str16, u32 len) {
  return sp_os_wstr_to_cstr(str16, len);
}

c8* sp_str_to_cstr(sp_str_t str) {
  return sp_cstr_copy_sized(str.data, str.len);
}

c8* sp_str_to_cstr_double_nt(sp_str_t str) {
  c8* buffer = (c8*)sp_alloc(str.len + 2);
  sp_str_copy_to(str, buffer, str.len);
  return (c8*)buffer;
}

sp_str_t sp_str_alloc(u32 capacity) {
  return SP_RVAL(sp_str_t) {
    .len = 0,
    .data = (c8*)sp_alloc(capacity),
  };
}

sp_str_t sp_str_view(const c8* cstr) {
  return (sp_str_t) {
    .len = sp_cstr_len(cstr),
    .data = cstr
  };
}

bool sp_str_empty(sp_str_t str) {
  return !str.len;
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

bool sp_str_starts_with(sp_str_t str, sp_str_t prefix) {
  if (str.len < prefix.len) return false;

  return sp_str_equal(sp_str_sub(str, 0, prefix.len), prefix);
}

bool sp_str_ends_with(sp_str_t str, sp_str_t suffix) {
  if (str.len < suffix.len) return false;

  return sp_str_equal(sp_str_sub_reverse(str, 0, suffix.len), suffix);
}

bool sp_str_contains(sp_str_t str, sp_str_t needle) {
  if (str.len < needle.len) return false;

  for (u32 i = 0; i <= str.len - needle.len; i++) {
    if (sp_os_is_memory_equal(str.data + i, needle.data, needle.len)) {
      return true;
    }
  }
  return false;
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

c8 sp_str_at(sp_str_t str, s32 index) {
  if (index < 0) {
    index = str.len + index;
  }
  return str.data[index];
}

c8 sp_str_at_reverse(sp_str_t str, s32 index) {
  if (index < 0) {
    index = str.len + index;
  }
  return str.data[str.len - index - 1];
}

c8 sp_str_back(sp_str_t str) {
  return str.data[str.len - 1];
}

sp_str_t sp_str_concat(sp_str_t a, sp_str_t b) {
  return sp_format("{}{}", SP_FMT_STR(a), SP_FMT_STR(b));
}

sp_str_t sp_str_join(sp_str_t a, sp_str_t b, sp_str_t join) {
  return sp_format("{}{}{}", SP_FMT_STR(a), SP_FMT_STR(join), SP_FMT_STR(b));
}

sp_str_t sp_str_join_cstr_n(const c8** strings, u32 num_strings, sp_str_t join) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  for (u32 index = 0; index < num_strings; index++) {
    sp_str_builder_append_cstr(&builder, strings[index]);

    if (index != (num_strings - 1)) {
      sp_str_builder_append(&builder, join);
    }
  }

  return sp_str_builder_write(&builder);
}

sp_str_t sp_str_sub(sp_str_t str, s32 index, s32 len) {
  sp_str_t substr = {
    .len = (u32)len,
    .data = str.data + index
  };
  SP_ASSERT(index + len <= str.len);
  return substr;
}

sp_str_t sp_str_sub_reverse(sp_str_t str, s32 index, s32 len) {
  sp_str_t substr = {
    .len = (u32)len,
    .data = str.data + str.len + index - len
  };
  return substr;
}

sp_str_t sp_str_from_cstr_sized(const c8* str, u32 length) {
  c8* buffer = (c8*)sp_alloc(length);
  u32 len = sp_cstr_len(str);
  len = SP_MIN(len, length);
  sp_os_copy_memory(str, buffer, len);

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr_null(const c8* str) {
  u32 len = sp_cstr_len(str);
  c8* buffer = (c8*)sp_alloc(len + 1);
  sp_os_copy_memory(str, buffer, len);
  buffer[len] = 0;

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr(const c8* str) {
  u32 len = sp_cstr_len(str);
  c8* buffer = (c8*)sp_alloc(len + 1);
  sp_os_copy_memory(str, buffer, len);

  return SP_STR(buffer, len);
}

sp_str_t sp_str_copy(sp_str_t str) {
  c8* buffer = (c8*)sp_alloc(str.len);
  sp_os_copy_memory(str.data, buffer, str.len);

  return SP_STR(buffer, str.len);
}

void sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity) {
  sp_os_copy_memory(str.data, buffer, SP_MIN(str.len, capacity));
}

sp_str_t sp_str_null_terminate(sp_str_t str) {
  c8* buffer = (c8*)sp_alloc(str.len + 1);
  sp_os_copy_memory(str.data, buffer, str.len);
  buffer[str.len] = 0;
  return SP_STR(buffer, str.len);
}

void sp_str_builder_grow(sp_str_builder_t* builder, u32 requested_capacity) {
  while (builder->buffer.capacity < requested_capacity) {
    u32 capacity = SP_MAX(builder->buffer.capacity * 2, 8);
    builder->buffer.data = (c8*)sp_realloc(builder->buffer.data, capacity);
    builder->buffer.capacity = capacity;
  }
}

void sp_str_builder_add_capacity(sp_str_builder_t* builder, u32 amount) {
  sp_str_builder_grow(builder, builder->buffer.len + amount);
}

void sp_str_builder_indent(sp_str_builder_t* builder) {
  builder->indent.level++;
}

void sp_str_builder_dedent(sp_str_builder_t* builder) {
  if (builder->indent.level) {
    builder->indent.level--;
  }
}

void sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str) {
  sp_str_builder_add_capacity(builder, str.len);
  sp_os_copy_memory(str.data, builder->buffer.data + builder->buffer.len, str.len);
  builder->buffer.len += str.len;
}

void sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str) {
  sp_str_builder_append(builder, SP_CSTR(str));
}

void sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c) {
  sp_str_builder_append(builder, SP_STR(&c, 1));
}

void sp_str_builder_append_fmt_str(sp_str_builder_t* builder, sp_str_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t formatted = sp_format_v(fmt, args);
  va_end(args);

  sp_str_builder_append(builder, formatted);
}

void sp_str_builder_append_fmt(sp_str_builder_t* builder, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t formatted = sp_format_v(SP_CSTR(fmt), args);
  va_end(args);

  sp_str_builder_append(builder, formatted);
}

void sp_str_builder_new_line(sp_str_builder_t* builder) {
  sp_str_builder_append_c8(builder, '\n');

  if (!sp_str_valid(builder->indent.word)) {
    builder->indent.word = SP_LIT("  ");
  }

  for (u32 index = 0; index < builder->indent.level; index++) {
    sp_str_builder_append(builder, builder->indent.word);
  }
}

sp_str_t sp_str_builder_move(sp_str_builder_t* builder) {
  sp_str_t str = sp_str(builder->buffer.data, builder->buffer.len);
  return str;
}

sp_str_t sp_str_builder_write(sp_str_builder_t* builder) {
  sp_str_t str = sp_str(builder->buffer.data, builder->buffer.len);
  return sp_str_copy(str);
}

c8* sp_str_builder_write_cstr(sp_str_builder_t* builder) {
  return sp_cstr_copy_sized(builder->buffer.data, builder->buffer.len);
}

sp_str_t sp_str_reduce(sp_str_t* strings, u32 num_strings, void* user_data, sp_str_reduce_fn_t fn) {
  sp_str_reduce_context_t context = {
    .user_data = user_data,
    .builder = { SP_ZERO_INITIALIZE() },
    .elements = {
      .data = strings,
      .len = num_strings
    },
  };

  for (u32 index = 0; index < num_strings; index++) {
    context.str = strings[index];
    context.index = index;
    fn(&context);
  }

  return sp_str_builder_write(&context.builder);
}

void sp_str_reduce_kernel_join(sp_str_reduce_context_t* context) {
  if (sp_str_empty(context->str)) return;

  sp_str_builder_append(&context->builder, context->str);

  if (context->index != (context->elements.len - 1)) {
    sp_str_t joiner = *(sp_str_t*)context->user_data;
    sp_str_builder_append(&context->builder, joiner);
  }
}

sp_str_t sp_str_join_n(sp_str_t* strings, u32 num_strings, sp_str_t joiner) {
  return sp_str_reduce(strings, num_strings, &joiner, sp_str_reduce_kernel_join);
}

sp_str_t sp_str_pad(sp_str_t str, u32 n) {
  s32 delta = (s32)n - (s32)str.len;
  if (delta <= 0) return sp_str_copy(str);

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, str);
  for (u32 index = 0; index < delta; index++) {
    sp_str_builder_append_c8(&builder, ' ');
  }

  return sp_str_builder_write(&builder);
}

sp_str_t sp_str_replace_c8(sp_str_t str, c8 from, c8 to) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    if (c == from) {
      sp_str_builder_append_c8(&builder, to);
    } else {
      sp_str_builder_append_c8(&builder, c);
    }
  }

  return sp_str_builder_write(&builder);
}

sp_dyn_array(sp_str_t) sp_str_split_c8(sp_str_t str, c8 delimiter) {
  if (sp_str_empty(str)) return SP_NULLPTR;

  sp_dyn_array(sp_str_t) result = SP_NULLPTR;

  u32 i = 0, j = 0;
  for (; j < str.len; j++) {
    if (sp_str_at(str, j) == delimiter) {
      sp_dyn_array_push(result, sp_str_sub(str, i, j - i));
      i = j + 1;
    }
  }

  sp_dyn_array_push(result, sp_str_sub(str, i, j - i));

  return result;
}
sp_str_pair_t sp_str_cleave_c8(sp_str_t str, c8 delimiter) {
  sp_str_pair_t result = SP_ZERO_STRUCT(sp_str_pair_t);

  for (u32 i = 0; i < str.len; i++) {
    if (sp_str_at(str, i) == delimiter) {
      result.first = sp_str_sub(str, 0, i);
      result.second = sp_str_sub(str, i + 1, str.len - i - 1);
      return result;
    }
  }

  result.first = str;
  result.second = SP_ZERO_STRUCT(sp_str_t);
  return result;
}

sp_str_t sp_str_trim_right(sp_str_t str) {
  while (str.len) {
    c8 c = sp_str_back(str);

    switch (c) {
      case ' ':
      case '\t':
      case '\r':
      case '\n': {
        str.len--;
        break;
      }
      default: {
        return str;
      }
    }
  }

  return str;
}

sp_str_t sp_str_trim(sp_str_t str) {
  u32 start = 0;
  u32 end = str.len;

  while (start < str.len) {
    c8 c = str.data[start];
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    start++;
  }

  while (end > start) {
    c8 c = str.data[end - 1];
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    end--;
  }

  return sp_str_sub(str, start, end - start);
}

sp_str_t sp_str_to_upper(sp_str_t str) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    if (c >= 'a' && c <= 'z') {
      sp_str_builder_append_c8(&builder, c - 32);
    } else {
      sp_str_builder_append_c8(&builder, c);
    }
  }

  return sp_str_builder_write(&builder);
}

sp_str_t sp_str_to_lower(sp_str_t str) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    if (c >= 'A' && c <= 'Z') {
      sp_str_builder_append_c8(&builder, c + 32);
    } else {
      sp_str_builder_append_c8(&builder, c);
    }
  }

  return sp_str_builder_write(&builder);
}

sp_str_t sp_str_capitalize_words(sp_str_t str) {
  if (str.len == 0) return str;

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  bool next_cap = true;

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      sp_str_builder_append_c8(&builder, c);
      next_cap = true;
    } else if (next_cap && c >= 'a' && c <= 'z') {
      sp_str_builder_append_c8(&builder, c - 32);
      next_cap = false;
    } else if (!next_cap && c >= 'A' && c <= 'Z') {
      sp_str_builder_append_c8(&builder, c + 32);
      next_cap = false;
    } else {
      sp_str_builder_append_c8(&builder, c);
      next_cap = false;
    }
  }

  return sp_str_builder_write(&builder);
}

sp_str_t sp_str_truncate(sp_str_t str, u32 n, sp_str_t trailer) {
  if (!n) return sp_str_copy(str);
  if (str.len <= n) return sp_str_copy(str);
  SP_ASSERT(trailer.len <= n);

  str.len = n - trailer.len;
  return sp_str_concat(str, trailer);
}

sp_dyn_array(sp_str_t) sp_str_map(sp_str_t* strs, u32 num_strs, sp_opaque_ptr user_data, sp_str_map_fn_t fn) {
  sp_dyn_array(sp_str_t) results = SP_NULLPTR;

  for (u32 index = 0; index < num_strs; index++) {
    sp_str_map_context_t context = SP_ZERO_INITIALIZE();
    context.str = strs[index];
    context.user_data = user_data;
    sp_str_t result = fn(&context);
    sp_dyn_array_push(results, result);
  }

  return results;
}

sp_str_t sp_str_map_kernel_prepend(sp_str_map_context_t* context) {
  sp_str_t prefix = *(sp_str_t*)context->user_data;
  return sp_str_concat(prefix, context->str);
}

sp_str_t sp_str_map_kernel_append(sp_str_map_context_t* context) {
  sp_str_t suffix = *(sp_str_t*)context->user_data;
  return sp_str_concat(context->str, suffix);
}

sp_str_t sp_str_map_kernel_prefix(sp_str_map_context_t* context) {
  u32 len = *(u32*)context->user_data;
  return sp_str_sub(context->str, 0, len);
}

sp_str_t sp_str_map_kernel_pad(sp_str_map_context_t* context) {
  u32 len = *(u32*)context->user_data;
  return sp_str_pad(context->str, len);
}

sp_str_t sp_str_map_kernel_trim(sp_str_map_context_t* context) {
  SP_UNUSED(context);
  return sp_str_trim(context->str);
}

sp_str_t sp_str_map_kernel_to_upper(sp_str_map_context_t* context) {
  SP_UNUSED(context);
  return sp_str_to_upper(context->str);
}

sp_str_t sp_str_map_kernel_to_lower(sp_str_map_context_t* context) {
  SP_UNUSED(context);
  return sp_str_to_lower(context->str);
}

sp_str_t sp_str_map_kernel_capitalize_words(sp_str_map_context_t* context) {
  SP_UNUSED(context);
  return sp_str_capitalize_words(context->str);
}

void sp_str_reduce_kernel_contains(sp_str_reduce_context_t* context) {
  sp_str_contains_context_t* data = (sp_str_contains_context_t*)context->user_data;

  if (data->found) return;

  if (sp_str_contains(context->str, data->needle)) {
    data->found = true;
  }
}

void sp_str_reduce_kernel_count(sp_str_reduce_context_t* context) {
  sp_str_count_context_t* data = (sp_str_count_context_t*)context->user_data;

  if (context->str.len >= data->needle.len) {
    for (u32 i = 0; i <= context->str.len - data->needle.len; i++) {
      if (sp_os_is_memory_equal(context->str.data + i, data->needle.data, data->needle.len)) {
        data->count++;
        i += data->needle.len - 1; // Skip past the match
      }
    }
  }
}

void sp_str_reduce_kernel_longest(sp_str_reduce_context_t* context) {
  sp_str_t* longest = (sp_str_t*)context->user_data;
  if (context->str.len > longest->len) {
    *longest = context->str;
  }
}

void sp_str_reduce_kernel_shortest(sp_str_reduce_context_t* context) {
  sp_str_t* shortest = (sp_str_t*)context->user_data;
  if (context->index == 0 || context->str.len < shortest->len) {
    *shortest = context->str;
  }
}

// Wrapper functions for common operations
bool sp_str_contains_n(sp_str_t* strs, u32 n, sp_str_t needle) {
  sp_str_contains_context_t context = {
    .needle = needle,
    .found = false
  };
  sp_str_reduce(strs, n, &context, sp_str_reduce_kernel_contains);
  return context.found;
}

u32 sp_str_count_n(sp_str_t* strs, u32 n, sp_str_t needle) {
  sp_str_count_context_t context = {
    .needle = needle,
    .count = 0
  };
  sp_str_reduce(strs, n, &context, sp_str_reduce_kernel_count);
  return context.count;
}

sp_str_t sp_str_find_longest_n(sp_str_t* strs, u32 n) {
  if (n == 0) return sp_str_lit("");
  sp_str_t longest = sp_str_lit("");
  sp_str_reduce(strs, n, &longest, sp_str_reduce_kernel_longest);
  return longest;
}

sp_str_t sp_str_find_shortest_n(sp_str_t* strs, u32 n) {
  if (n == 0) return sp_str_lit("");
  sp_str_t shortest = sp_str_lit("");
  sp_str_reduce(strs, n, &shortest, sp_str_reduce_kernel_shortest);
  return shortest;
}

sp_dyn_array(sp_str_t) sp_str_pad_to_longest(sp_str_t* strs, u32 n) {
  sp_str_t longest = sp_str_find_longest_n(strs, n);
  return sp_str_map(strs, n, &longest.len, sp_str_map_kernel_pad);
}


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

u8* sp_dynamic_array_push(sp_dynamic_array_t* arr, void* data) {
  return sp_dynamic_array_push_n(arr, data, 1);
}

u8* sp_dynamic_array_push_n(sp_dynamic_array_t* arr, void* data, u32 count) {
  SP_ASSERT(arr);

  u8* reserved = sp_dynamic_array_reserve(arr, count);
  if (data) sp_os_copy_memory(data, reserved, arr->element_size * count);
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
  if (data) sp_os_copy_memory(data, reserved, buffer->element_size * count);
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


////////
// OS //
////////
sp_str_t sp_os_normalize_path(sp_str_t path) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  for (u32 i = 0; i < path.len; i++) {
    if (path.data[i] == '\\') {
      sp_str_builder_append_c8(&builder, '/');
    }
    else {
      sp_str_builder_append_c8(&builder, path.data[i]);
    }
  }


  if (path.len) {
    switch (sp_str_back(path)) {
      case '/':
      case '\\': {
        builder.buffer.len--;
        break;
      }
      default: {
        break;
      }
    }
  }

  return sp_str_builder_write(&builder);
}

void sp_os_normalize_path_soft(sp_str_t* path) {
  if (!sp_str_valid(*path)) return;
  if (sp_str_empty(*path)) return;

  switch (sp_str_back(*path)) {
    case '/':
    case '\\': {
      path->len--;
      break;
    }
    default: {
      break;
    }
  }
}

sp_str_t sp_os_extract_file_name(sp_str_t path) {
  if (sp_str_empty(path)) return SP_LIT("");

  const c8* last_slash = NULL;
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
  const c8* c = path.data + path.len - 1;

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

  sp_str_t result = sp_str_copy(path);
  sp_os_normalize_path_soft(&result);
  return result;
}

sp_str_t sp_os_join_path(sp_str_t a, sp_str_t b) {
  // Remove trailing slash from first path to avoid double slash
  sp_str_t a_copy = a;
  if (a_copy.len > 0 && (a_copy.data[a_copy.len - 1] == '/' || a_copy.data[a_copy.len - 1] == '\\')) {
    a_copy.len--;
  }

  sp_str_t result = sp_str_join(a_copy, b, SP_LIT("/"));
  sp_os_normalize_path_soft(&result);
  return result;
}

sp_str_t sp_os_extract_extension(sp_str_t path) {
  for (u32 index = 0; index < path.len; index++) {
    c8 c = sp_str_at_reverse(path, index);

    switch (c) {
      case '.': return sp_str_sub_reverse(path, 0, index);
      case '/': return sp_str_sub_reverse(path, 0, 0);
      default:  break;
    }
  }

  return sp_str_sub_reverse(path, 0, 0);
}

sp_str_t sp_os_extract_stem(sp_str_t path) {
  sp_str_t file_name = sp_os_extract_file_name(path);
  if (!file_name.len) return path;

  sp_str_t extension = sp_os_extract_extension(path);

  sp_str_t stem = {
    .len = file_name.len - extension.len,
    .data = file_name.data
  };

  while (true) {
    if (!stem.len) break;
    if (sp_str_back(stem) != '.') break;

    stem.len--;
  }

  return stem;
}

void sp_spin_pause() {
  #if defined(SP_AMD64)
    #if defined(SP_MSVC)
      _mm_pause();
    #elif defined(SP_TCC)
      volatile int x = 0; (void)x;
    #elif defined(SP_GNUISH)
      __asm__ __volatile__("pause");
    #endif

  #elif defined(SP_ARM64)
    #if defined(SP_MSVC)
      __yield();
    #elif defined(SP_TCC)
      volatile int x = 0; (void)x;
    #elif defined(SP_GNUISH)
      __asm__ __volatile__("yield");
    #endif
  #endif
}

bool sp_spin_try_lock(sp_spin_lock_t* lock) {
  #if defined(SP_GNUISH)
    return __sync_lock_test_and_set(lock, 1) == 0;
  #elif defined(SP_MSVC)
    return _InterlockedExchange(lock, 1) == 0;
  #else
    sp_context_ensure();
    sp_mutex_lock(&sp_global.mutex);

    if (*lock == 0) {
      *lock = 1;
      sp_mutex_unlock(&sp_global.mutex);
      return true;
    }
    else {
      sp_mutex_unlock(&sp_global.mutex);
      return false;
    }
  #endif
}

void sp_spin_lock(sp_spin_lock_t* lock) {
  while (!sp_spin_try_lock(lock)) {
    while (*lock) {
      sp_spin_pause();
    }
  }
}

void sp_spin_unlock(sp_spin_lock_t* lock) {
  #if defined(SP_GNUISH)
    __sync_lock_release(lock);
  #elif defined(SP_MSVC)
    _InterlockedExchange(lock, 0);
  #else
    sp_context_ensure();
    sp_mutex_lock(&sp_global.mutex);
    *lock = 0;
    sp_mutex_unlock(&sp_global.mutex);
  #endif
}

bool sp_atomic_s32_cmp_and_swap(sp_atomic_s32* value, s32 current, s32 desired) {
  #if defined(SP_MSVC)
    return _InterlockedCompareExchange(&value, desired, current) == current;
  #elif defined(SP_GNUISH)
    return __sync_bool_compare_and_swap(value, current, desired);
  #else
    bool result = false;
    size_t index = ((((size_t)value) >> 3) & 0x1f);
    sp_spin_lock(&sp_global.locks[index]);
    if (*value == current) {
      *value = desired;
      result = true;
    }
    sp_spin_unlock(&sp_global.locks[index]);
    return result;
  #endif
}

s32 sp_atomic_s32_set(sp_atomic_s32* value, s32 desired) {
  #if defined(SP_MSVC)
    return _InterlockedExchange((long*)value, desired);
  #elif defined(SP_GNUISH)
    return __sync_lock_test_and_set(value, desired);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cmp_and_swap(value, old, desired));
    return old;
  #endif
}

s32 sp_atomic_s32_add(sp_atomic_s32* value, s32 add) {
  #if defined(SP_MSVC)
    return _InterlockedExchangeAdd((long*)value, add);
  #elif defined(SP_GNUISH)
    return __sync_fetch_and_add(value, add);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cmp_and_swap(value, old, old + add));
    return old;
  #endif
}

s32 sp_atomic_s32_get(sp_atomic_s32* value) {
  #if defined(SP_MSVC)
    return _InterlockedOr((long*)value, 0);
  #elif defined(SP_GNUISH)
    return __sync_or_and_fetch(value, 0);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cmp_and_swap(value, old, old));
    return old;
  #endif
}

#if defined(SP_WIN32)
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

  bool sp_os_is_path_root(sp_str_t path) {
    if (path.len == 0) return true;
    if (path.len == 1 && path.data[0] == '/') return true;
    if (path.len == 2 && path.data[1] == ':') return true;
    if (path.len == 3 && path.data[1] == ':' && (path.data[2] == '/' || path.data[2] == '\\')) return true;
    return false;
  }

  void sp_os_remove_directory(sp_str_t path) {
    SHFILEOPSTRUCTA file_op = SP_ZERO_INITIALIZE();
    file_op.wFunc = FO_DELETE;
    file_op.pFrom = sp_str_to_double_null_terminated(path);
    file_op.fFlags = FOF_NO_UI;
    SHFileOperationA(&file_op);
  }

  void sp_os_create_directory(sp_str_t path) {
    if (path.len == 0) return;

    sp_str_t normalized = sp_os_normalize_path(path);
    while (normalized.len > 0 && (normalized.data[normalized.len - 1] == '/' || normalized.data[normalized.len - 1] == '\\')) {
      normalized = sp_str(normalized.data, normalized.len - 1);
    }

    if (sp_os_does_path_exist(normalized)) return;

    c8* path_cstr = sp_str_to_cstr(normalized);
    if (CreateDirectoryA(path_cstr, NULL)) {
      return;
    }

    if (sp_os_is_path_root(normalized)) return;

    sp_str_t parent = sp_os_parent_path(normalized);
    if (parent.len > 0 && !sp_str_equal(parent, normalized)) {
      sp_os_create_directory(parent);
      CreateDirectoryA(path_cstr, NULL);
    }
  }

  void sp_os_create_file(sp_str_t path) {
    sp_win32_handle_t handle = CreateFileA(sp_str_to_cstr(path), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(handle);
  }

  void sp_os_remove_file(sp_str_t path) {
    DeleteFileA(sp_str_to_cstr(path));
  }

  bool sp_os_is_glob(sp_str_t path) {
    sp_str_t file_name = sp_os_extract_file_name(path);
    return sp_str_contains_n(&file_name, 1, sp_str_lit("*"));
  }

  bool sp_os_is_program_on_path(sp_str_t program) {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("where");
    sp_ps_config_add_arg(&config, program);
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_output_t output = sp_ps_run(config);

    return output.status.exit_code == 0;
  }

  void sp_os_copy(sp_str_t from, sp_str_t to) {
    if (sp_os_is_glob(from)) {
      sp_os_copy_glob(sp_os_parent_path(from), sp_os_extract_file_name(from), to);
    }
    else if (sp_os_is_directory(from)) {
      SP_ASSERT(sp_os_is_directory(to));
      sp_os_copy_glob(from, sp_str_lit("*"), sp_os_join_path(to, sp_os_extract_stem(from)));
    }
    else if (sp_os_is_regular_file(from)) {
      sp_os_copy_file(from, to);
    }
  }

  void sp_os_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to) {
    sp_os_create_directory(to);

    sp_os_directory_entry_list_t entries = sp_os_scan_directory(from);

    for (u32 i = 0; i < entries.count; i++) {
      sp_os_directory_entry_t* entry = &entries.data[i];
      sp_str_t entry_name = entry->file_name;

      bool matches = sp_str_equal(glob, sp_str_lit("*"));
      if (!matches) {
        matches = sp_str_equal(entry_name, glob);
      }

      if (matches) {
        sp_str_t entry_path = sp_os_join_path(from, entry_name);
        sp_os_copy(entry_path, to);
      }
    }
  }

  void sp_os_copy_file(sp_str_t from, sp_str_t to) {
    if (sp_os_is_directory(to)) {
      sp_os_create_directory(to);
      to = sp_os_join_path(to, sp_os_extract_file_name(from));
    }

    sp_io_stream_t src = sp_io_from_file(from, SP_IO_MODE_READ);
    if (!src.file.fd) return;

    sp_io_stream_t dst = sp_io_from_file(to, SP_IO_MODE_WRITE);
    if (!dst.file.fd) {
      sp_io_close(&src);
      return;
    }

    u8 buffer[4096];
    u64 bytes_read;
    while ((bytes_read = sp_io_read(&src, buffer, sizeof(buffer))) > 0) {
      sp_io_write(&dst, buffer, bytes_read);
    }

    sp_io_close(&src);
    sp_io_close(&dst);
  }

  void sp_os_copy_directory(sp_str_t from, sp_str_t to) {
    if (sp_os_is_directory(to)) {
      to = sp_os_join_path(to, sp_os_extract_file_name(from));
    }

    sp_os_copy_glob(from, sp_str_lit("*"), to);
  }

  sp_da(sp_os_dir_entry_t) sp_os_scan_directory(sp_str_t path) {
    if (!sp_os_is_directory(path) || !sp_os_does_path_exist(path)) {
      return SP_NULLPTR;
    }

    sp_dyn_array(sp_os_dir_entry_t) entries = SP_NULLPTR;

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append(&builder, path);
    sp_str_builder_append(&builder, sp_str_lit("/*"));
    sp_str_t glob = sp_str_builder_write(&builder);

    sp_win32_find_data_t find_data;
    sp_win32_handle_t handle = FindFirstFile(sp_str_to_cstr(glob), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
      return SP_NULLPTR;
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

      sp_os_dir_entry_t entry = SP_RVAL(sp_os_dir_entry_t) {
        .file_path = file_path,
        .file_name = sp_str_from_cstr(find_data.cFileName),
        .attributes = sp_os_file_attributes(file_path),
      };
      sp_dyn_array_push(entries, entry);
    } while (FindNextFile(handle, &find_data));

    FindClose(handle);

    return entries;
  }

  sp_os_directory_entry_list_t sp_os_scan_directory_recursive(sp_str_t path) {
    SP_BROKEN();
    return SP_ZERO_STRUCT(sp_os_directory_entry_list_t);
  }

  sp_tm_epoch_t sp_tm_now_epoch() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert to Unix epoch
    u64 windows_100ns = ((u64)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    u64 unix_100ns = windows_100ns - 116444736000000000ULL;

    return SP_RVAL(sp_tm_epoch_t) {
      .seconds = unix_100ns / 10000000,
      .nanoseconds = (u32)((unix_100ns % 10000000) * 100)
    };
  }

  sp_str_t sp_tm_to_iso8601(sp_tm_epoch_t time) {
    FILETIME ft;
    SYSTEMTIME st;

    // Convert Unix epoch back to Windows FILETIME
    u64 unix_100ns = time.seconds * 10000000ULL + time.nanoseconds / 100ULL;
    u64 windows_100ns = unix_100ns + 116444736000000000ULL;

    ft.dwHighDateTime = (u32)(windows_100ns >> 32);
    ft.dwLowDateTime = (u32)(windows_100ns & 0xFFFFFFFF);

    FileTimeToSystemTime(&ft, &st);

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append_fmt(&builder, "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
      SP_FMT_S32(st.wYear),
      SP_FMT_S32(st.wMonth),
      SP_FMT_S32(st.wDay),
      SP_FMT_S32(st.wHour),
      SP_FMT_S32(st.wMinute),
      SP_FMT_S32(st.wSecond),
      SP_FMT_U32(time.nanoseconds / 1000000));

    return sp_str_builder_write(&builder);
  }

  sp_tm_point_t sp_tm_now_point() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);

    // Convert to nanoseconds
    u64 ns = (counter.QuadPart * 1000000000ULL) / freq.QuadPart;
    return (sp_tm_point_t)ns;
  }

  u64 sp_tm_point_diff(sp_tm_point_t newer, sp_tm_point_t older) {
    return newer - older;
  }

  sp_tm_timer_t sp_tm_start_timer() {
    sp_tm_point_t now = sp_tm_now_point();
    return SP_RVAL(sp_tm_timer_t) {
      .start = now,
      .previous = now
    };
  }

  u64 sp_tm_read_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t current = sp_tm_now_point();
    if (current < timer->previous) {
      timer->previous = current;
    }
    return sp_tm_point_diff(current, timer->start);
  }

  u64 sp_tm_lap_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t current = sp_tm_now_point();
    if (current < timer->previous) {
      timer->previous = current;
    }
    u64 elapsed = sp_tm_point_diff(current, timer->previous);
    timer->previous = current;
    return elapsed;
  }

  void sp_tm_reset_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t now = sp_tm_now_point();
    timer->start = now;
    timer->previous = now;
  }

  sp_tm_date_time_t sp_tm_get_date_time() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return SP_RVAL(sp_tm_date_time_t) {
      .year = st.wYear,
      .month = st.wMonth,
      .day = st.wDay,
      .hour = st.wHour,
      .minute = st.wMinute,
      .second = st.wSecond,
      .millisecond = st.wMilliseconds
    };
  }

  sp_tm_epoch_t sp_os_file_mod_time_precise(sp_str_t file_path) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(sp_str_to_cstr(file_path), GetFileExInfoStandard, &fad)) {
      return SP_ZERO_STRUCT(sp_tm_epoch_t);
    }

    if (fad.nFileSizeHigh == 0 && fad.nFileSizeLow == 0) {
      return SP_ZERO_STRUCT(sp_tm_epoch_t);
    }

    LARGE_INTEGER time;
    time.HighPart = fad.ftLastWriteTime.dwHighDateTime;
    time.LowPart = fad.ftLastWriteTime.dwLowDateTime;

    // Convert to Unix epoch
    u64 unix_100ns = time.QuadPart - 116444736000000000LL;

    return SP_RVAL(sp_tm_epoch_t) {
      unix_100ns / 10000000,           // seconds
      (unix_100ns % 10000000) * 100    // remainder to nanoseconds
    };
  }

  sp_os_file_attr_t sp_os_file_attributes(sp_str_t path) {
    return sp_os_winapi_attr_to_sp_attr(GetFileAttributesA(sp_str_to_cstr(path)));
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

  void sp_os_copy_memory(const void* source, void* dest, u32 num_bytes) {
    memcpy(dest, source, num_bytes);
  }

  void sp_os_move_memory(const void* source, void* dest, u32 num_bytes) {
    memmove(dest, source, num_bytes);
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

    sp_str_t exe_path_str = SP_CSTR(exe_path);
    sp_os_normalize_path(exe_path_str);

    return sp_str_copy(exe_path_str);
  }

  sp_str_t sp_os_canonicalize_path(sp_str_t path) {
    c8* path_cstr = sp_str_to_cstr(path);
    c8 canonical_path[SP_MAX_PATH_LEN];

    if (GetFullPathNameA(path_cstr, SP_MAX_PATH_LEN, canonical_path, NULL) == 0) {
      return sp_str_copy(path);
    }

    sp_str_t result = sp_str_from_cstr(canonical_path);
    result = sp_os_normalize_path(result);

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

  void sp_os_print(sp_str_t message) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    WriteConsoleA(console, message.data, message.len, &written, NULL);
  }

  void sp_os_log(sp_str_t message) {
    sp_os_print(message);
    sp_os_print(SP_LIT("\n"));
  }

  void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
    sp_thread_launch_t launch = SP_RVAL(sp_thread_launch_t) {
      .fn = fn,
      .userdata = userdata,
      .semaphore = SP_ZERO_STRUCT(sp_semaphore_t)
    };
    sp_semaphore_init(&launch.semaphore);

    thrd_create(thread, sp_thread_launch, &launch);
    sp_semaphore_wait(&launch.semaphore);
  }

  s32 sp_thread_launch(void* args) {
    sp_thread_launch_t* launch = (sp_thread_launch_t*)args;
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
#endif

#if defined(SP_POSIX)
  sp_str_t sp_os_get_cwd() {
    c8 path [PATH_MAX];
    if (!getcwd(path, PATH_MAX - 1)) {
      return SP_ZERO_STRUCT(sp_str_t);
    }

    sp_str_t cwd = sp_str_from_cstr(path);
    return sp_os_normalize_path(cwd);
  }

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

  void sp_os_copy_memory(const void* source, void* dest, u32 num_bytes) {
    memcpy(dest, source, num_bytes);
  }

  void sp_os_move_memory(const void* source, void* dest, u32 num_bytes) {
    memmove(dest, source, num_bytes);
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
    struct stat st;
    c8* path_cstr = sp_str_to_cstr(path);
    s32 result = stat(path_cstr, &st);
    return result == 0;
  }

  bool sp_os_is_regular_file(sp_str_t path) {
    struct stat st;
    c8* path_cstr = sp_str_to_cstr(path);
    s32 result = stat(path_cstr, &st);
    if (result != 0) return false;
    return S_ISREG(st.st_mode);
  }

  bool sp_os_is_directory(sp_str_t path) {
    struct stat st;
    c8* path_cstr = sp_str_to_cstr(path);
    s32 result = stat(path_cstr, &st);
    if (result != 0) return false;
    return S_ISDIR(st.st_mode);
  }

  bool sp_os_is_path_root(sp_str_t path) {
    if (sp_str_empty(path)) return true;
    if (sp_str_equal_cstr(path, "/")) return true;

    return false;
  }

  void sp_os_remove_directory(sp_str_t path) {
    sp_da(sp_os_dir_entry_t) entries = sp_os_scan_directory(path);

    sp_dyn_array_for(entries, i) {
      sp_os_dir_entry_t* entry = &entries[i];

      if (sp_os_is_directory(entry->file_path)) {
        sp_os_remove_directory(entry->file_path);
      }
      if (sp_os_is_regular_file(entry->file_path)) {
        sp_os_remove_file(entry->file_path);
      }
    }

    c8* path_cstr = sp_str_to_cstr(path);
    rmdir(path_cstr);
  }

  void sp_os_create_directory(sp_str_t path) {
    sp_os_normalize_path_soft(&path);
    c8* path_cstr = sp_str_to_cstr(path);

    if (sp_str_empty(path)) return;
    if (sp_os_is_path_root(path)) return;
    if (sp_os_does_path_exist(path)) return;

    s32 result = mkdir(path_cstr, 0755);
    if (!result) return;

    sp_os_create_directory(sp_os_parent_path(path));

    result = mkdir(path_cstr, 0755);
    if (!result) return;
  }

  void sp_os_create_file(sp_str_t path) {
    c8* path_cstr = sp_str_to_cstr(path);
    s32 fd = open(path_cstr, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) close(fd);
  }

  void sp_os_remove_file(sp_str_t path) {
    c8* path_cstr = sp_str_to_cstr(path);
    unlink(path_cstr);
  }

  bool sp_os_is_glob(sp_str_t path) {
    SP_INCOMPLETE()
    sp_str_t file_name = sp_os_extract_file_name(path);
    return sp_str_contains_n(&file_name, 1, sp_str_lit("*"));
  }

  bool sp_os_is_program_on_path(sp_str_t program) {
    SP_INCOMPLETE()
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("which");
    sp_ps_config_add_arg(&config, program);
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_output_t output = sp_ps_run(config);

    return output.status.exit_code == 0;
  }

  void sp_os_copy(sp_str_t from, sp_str_t to) {
    if (sp_os_is_glob(from)) {
      sp_os_copy_glob(sp_os_parent_path(from), sp_os_extract_file_name(from), to);
    }
    else if (sp_os_is_directory(from)) {
      SP_ASSERT(sp_os_is_directory(to));
      sp_os_copy_glob(from, sp_str_lit("*"), sp_os_join_path(to, sp_os_extract_stem(from)));
    }
    else if (sp_os_is_regular_file(from)) {
      sp_os_copy_file(from, to);
    }
  }

  void sp_os_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to) {
    sp_os_create_directory(to);

    sp_da(sp_os_dir_entry_t) entries = sp_os_scan_directory(from);

    sp_dyn_array_for(entries, i) {
      sp_os_dir_entry_t* entry = &entries[i];
      sp_str_t entry_name = entry->file_name;

      bool matches = sp_str_equal(glob, sp_str_lit("*"));
      if (!matches) {
        matches = sp_str_equal(entry_name, glob);
      }

      if (matches) {
        sp_str_t entry_path = sp_os_join_path(from, entry_name);
        sp_os_copy(entry_path, to);
      }
    }
  }

  void sp_os_copy_file(sp_str_t from, sp_str_t to) {
    if (sp_os_is_directory(to)) {
      sp_os_create_directory(to);
      to = sp_os_join_path(to, sp_os_extract_file_name(from));
    }

    sp_io_stream_t src = sp_io_from_file(from, SP_IO_MODE_READ);
    if (!src.file.fd) return;

    sp_io_stream_t dst = sp_io_from_file(to, SP_IO_MODE_WRITE);
    if (!dst.file.fd) {
      sp_io_close(&src);
      return;
    }

    u8 buffer[4096];
    u64 bytes_read;
    while ((bytes_read = sp_io_read(&src, buffer, sizeof(buffer))) > 0) {
      sp_io_write(&dst, buffer, bytes_read);
    }

    sp_io_close(&src);
    sp_io_close(&dst);
  }

  void sp_os_copy_directory(sp_str_t from, sp_str_t to) {
    if (sp_os_is_directory(to)) {
      to = sp_os_join_path(to, sp_os_extract_file_name(from));
    }

    sp_os_copy_glob(from, sp_str_lit("*"), to);
  }

  sp_da(sp_os_dir_entry_t) sp_os_scan_directory(sp_str_t path) {
    if (!sp_os_is_directory(path) || !sp_os_does_path_exist(path)) {
      return SP_NULLPTR;
    }

    sp_dyn_array(sp_os_dir_entry_t) entries = SP_NULLPTR;

    c8* path_cstr = sp_str_to_cstr(path);
    DIR* dir = opendir(path_cstr);

    if (!dir) {
      return entries;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (sp_cstr_equal(entry->d_name, ".")) continue;
      if (sp_cstr_equal(entry->d_name, "..")) continue;

      sp_str_builder_t entry_builder = SP_ZERO_INITIALIZE();
      sp_str_builder_append(&entry_builder, path);
      sp_str_builder_append(&entry_builder, sp_str_lit("/"));
      sp_str_builder_append_cstr(&entry_builder, entry->d_name);
      sp_str_t file_path = sp_str_builder_write(&entry_builder);
      sp_os_normalize_path(file_path);

      sp_os_dir_entry_t dir_entry = {
        .file_path = file_path,
        .file_name = sp_str_from_cstr(entry->d_name),
        .attributes = sp_os_file_attributes(file_path),
      };
      sp_dyn_array_push(entries, dir_entry);
    }

    closedir(dir);

    return entries;
  }

  sp_tm_date_time_t sp_tm_get_date_time() {
    time_t raw_time;
    struct tm* time_info;
    struct timeval tv;

    time(&raw_time);
    time_info = localtime(&raw_time);
    gettimeofday(&tv, NULL);

    return SP_RVAL(sp_tm_date_time_t) {
      .year = time_info->tm_year + 1900,
      .month = time_info->tm_mon + 1,
      .day = time_info->tm_mday,
      .hour = time_info->tm_hour,
      .minute = time_info->tm_min,
      .second = time_info->tm_sec,
      .millisecond = (s32)(tv.tv_usec / 1000)
    };
  }

  sp_tm_epoch_t sp_tm_now_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return SP_RVAL(sp_tm_epoch_t) {
      .s = (u64)ts.tv_sec,
      .ns = (u32)ts.tv_nsec
    };
  }

  sp_str_t sp_tm_to_iso8601(sp_tm_epoch_t time) {
    struct tm* time_info;
    time_t raw_time = (time_t)time.s;
    time_info = gmtime(&raw_time);

    c8 buffer[32];
    size_t len = strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", time_info);

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append(&builder, sp_str(buffer, len));
    sp_str_builder_append_c8(&builder, '.');

    u32 ms = time.ns / 1000000;
    if (ms < 100) sp_str_builder_append_c8(&builder, '0');
    if (ms < 10) sp_str_builder_append_c8(&builder, '0');
    sp_str_builder_append_fmt(&builder, "{}", SP_FMT_U32(ms));
    sp_str_builder_append_c8(&builder, 'Z');

    return sp_str_builder_write(&builder);
  }

  sp_tm_point_t sp_tm_now_point() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (sp_tm_point_t)(ts.tv_sec * 1000000000ULL + ts.tv_nsec);
  }

  u64 sp_tm_point_diff(sp_tm_point_t newer, sp_tm_point_t older) {
    return newer - older;
  }

  sp_tm_timer_t sp_tm_start_timer() {
    sp_tm_point_t now = sp_tm_now_point();
    return SP_RVAL(sp_tm_timer_t) {
      .start = now,
      .previous = now
    };
  }

  u64 sp_tm_read_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t current = sp_tm_now_point();
    if (current < timer->previous) {
      timer->previous = current;
    }
    return sp_tm_point_diff(current, timer->start);
  }

  u64 sp_tm_lap_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t current = sp_tm_now_point();
    if (current < timer->previous) {
      timer->previous = current;
    }
    u64 elapsed = sp_tm_point_diff(current, timer->previous);
    timer->previous = current;
    return elapsed;
  }

  void sp_tm_reset_timer(sp_tm_timer_t* timer) {
    sp_tm_point_t now = sp_tm_now_point();
    timer->start = now;
    timer->previous = now;
  }

  sp_tm_epoch_t sp_os_file_mod_time_precise(sp_str_t file_path) {
    struct stat st;
    c8* path_cstr = sp_str_to_cstr(file_path);
    s32 result = stat(path_cstr, &st);

    if (result != 0) {
      return SP_ZERO_STRUCT(sp_tm_epoch_t);
    }

    if (st.st_size == 0) {
      return SP_ZERO_STRUCT(sp_tm_epoch_t);
    }

    return SP_RVAL(sp_tm_epoch_t) {
      .s = (u64)st.st_mtime,
      .ns = 0
    };
  }

  void sp_os_sleep_ms(f64 ms) {
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000.0);
    ts.tv_nsec = (long)((ms - (ts.tv_sec * 1000.0)) * 1000000.0);
    nanosleep(&ts, NULL);
  }

  sp_str_t sp_os_canonicalize_path(sp_str_t path) {
    c8* path_cstr = sp_str_to_cstr(path);
    c8 canonical_path[SP_MAX_PATH_LEN] = SP_ZERO_INITIALIZE();
    realpath(path_cstr, canonical_path);

    sp_str_t result = SP_CSTR(canonical_path);
    result = sp_os_normalize_path(result);

    if (result.len > 0 && result.data[result.len - 1] == '/') {
      result.len--;
    }

    return sp_str_copy(result);
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

  sp_os_file_attr_t sp_os_file_attributes(sp_str_t path) {
    struct stat st;
    if (stat(sp_str_to_cstr(path), &st) == 0) {
      if (S_ISDIR(st.st_mode)) {
        return SP_OS_FILE_ATTR_DIRECTORY;
      } else if (S_ISREG(st.st_mode)) {
        return SP_OS_FILE_ATTR_REGULAR_FILE;
      }
    }
    return SP_OS_FILE_ATTR_NONE;
  }

  void sp_os_print(sp_str_t message) {
    write(STDOUT_FILENO, message.data, message.len);
  }

  void sp_os_log(sp_str_t message) {
    sp_os_print(message);
    sp_os_print(SP_LIT("\n"));
  }

  void* sp_posix_thread_launch(void* args) {
    return (void*)(intptr_t)sp_thread_launch(args);
  }

  s32 sp_thread_launch(void* args) {
    sp_thread_launch_t* launch = (sp_thread_launch_t*)args;
    void* userdata = launch->userdata;
    sp_thread_fn_t fn = launch->fn;

    sp_semaphore_signal(&launch->semaphore);
    s32 result = fn(userdata);

    return result;
  }

  void sp_thread_join(sp_thread_t* thread) {
    pthread_join(*thread, NULL);
  }

  void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
    sp_thread_launch_t launch = SP_ZERO_INITIALIZE();
    launch.fn = fn;
    launch.userdata = userdata;
    sp_semaphore_init(&launch.semaphore);

    pthread_create(thread, NULL, sp_posix_thread_launch, &launch);
    sp_semaphore_wait(&launch.semaphore);
  }

  void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    if (kind & SP_MUTEX_RECURSIVE) {
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    }

    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
  }

  void sp_mutex_lock(sp_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
  }

  void sp_mutex_unlock(sp_mutex_t* mutex) {
    pthread_mutex_unlock(mutex);
  }

  void sp_mutex_destroy(sp_mutex_t* mutex) {
    pthread_mutex_destroy(mutex);
  }

void sp_env_init(sp_env_t* env) {
  sp_ht_set_fns(env->vars, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);
}

sp_env_t sp_env_capture() {
  sp_env_t env = SP_ZERO_INITIALIZE();
  sp_env_init(&env);

  for (c8** envp = (c8**)environ; *envp != SP_NULLPTR; envp++) {
    sp_str_pair_t pair = sp_str_cleave_c8(sp_str_view(*envp), '=');
    sp_ht_insert(env.vars, pair.first, pair.second);
  }

  return env;
}

sp_env_t sp_env_copy(sp_env_t* env) {
  sp_env_t copy = SP_ZERO_INITIALIZE();
  sp_env_init(&copy);

  sp_ht_for(env->vars, it) {
    sp_str_t key = *sp_ht_it_getkp(env->vars, it);
    sp_str_t val = *sp_ht_it_getp(env->vars, it);
    sp_env_insert(&copy, key, val);
  }

  return copy;
}

sp_str_t sp_env_get(sp_env_t* env, sp_str_t name) {
  sp_str_t* val = sp_ht_getp(env->vars, name);
  return val ? *val : SP_ZERO_STRUCT(sp_str_t);
}

void sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value) {
  sp_ht_insert(env->vars, name, value);
}

void sp_env_erase(sp_env_t* env, sp_str_t name) {
  sp_ht_erase(env->vars, name);
}

void sp_env_destroy(sp_env_t* env) {
  sp_ht_free(env->vars);
  env->vars = SP_NULLPTR;
}

sp_str_t sp_os_get_env_var(sp_str_t key) {
  const c8* value = getenv(sp_str_to_cstr(key));
  return sp_str_view(value);
}

sp_str_t sp_os_get_env_as_path(sp_str_t key) {
  SP_UNTESTED()
  const c8* value = getenv(sp_str_to_cstr(key));
  sp_str_t path = sp_str_view(value);
  return sp_os_normalize_path(path);
}

void sp_os_clear_env_var(sp_str_t key) {
  unsetenv(sp_str_to_cstr(key));
}

void sp_os_export_env(sp_env_t* env, sp_env_export_overwrite_t overwrite) {
  sp_ht_for(env->vars, it) {
    sp_os_export_env_var(*sp_ht_it_getkp(env->vars, it), *sp_ht_it_getp(env->vars, it), overwrite);
  }
}

void sp_os_export_env_var(sp_str_t key, sp_str_t value, sp_env_export_overwrite_t overwrite) {
  const c8* k = sp_str_to_cstr(key);
  const c8* v = sp_str_to_cstr(value);
  setenv(k, v, overwrite);
}

bool sp_ps_create_pipes(s32 pipes [2]) {
  if (pipe(pipes) < 0) {
    return false;
  }

  fcntl(pipes[0], F_SETFD, fcntl(pipes[0], F_GETFD) | FD_CLOEXEC);
  fcntl(pipes[1], F_SETFD, fcntl(pipes[1], F_GETFD) | FD_CLOEXEC);

  signal(SIGPIPE, SIG_IGN);

  return true;
}

void sp_ps_set_nonblocking(s32 fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void sp_ps_set_blocking(s32 fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
}

c8** sp_ps_build_posix_args(sp_ps_config_t* config) {
  u32 arg_count = 1;
  for (u32 i = 0; i < SP_PS_MAX_ARGS; i++) {
    if (sp_str_empty(config->args[i])) break;
    arg_count++;
  }

  c8** args = (c8**)sp_alloc(sizeof(c8*) * (arg_count + 1));
  args[0] = (c8*)sp_str_to_cstr(config->command);

  for (u32 i = 0; i < arg_count - 1; i++) {
    args[i + 1] = (c8*)sp_str_to_cstr(config->args[i]);
  }
  args[arg_count] = SP_NULLPTR;

  return args;
}

void sp_ps_free_posix_args(c8** args) {
  if (!args) return;
  for (u32 i = 0; args[i] != SP_NULLPTR; i++) {
    sp_free((void*)args[i]);
  }
  sp_free(args);
}

c8** sp_ps_build_posix_env(sp_ps_env_config_t* config) {
  sp_dyn_array(c8*) envp = SP_NULLPTR;

  sp_env_t env = SP_ZERO_INITIALIZE();
  sp_env_init(&env);

  // Apply the base environment
  switch (config->mode) {
    case SP_PS_ENV_INHERIT: {
      for (c8** kvp = (c8**)environ; *kvp != SP_NULLPTR; kvp++) {
        sp_str_pair_t pair = sp_str_cleave_c8(sp_str_view(*kvp), '=');
        sp_ht_insert(env.vars, pair.first, pair.second);
      }

      break;
    }
    case SP_PS_ENV_EXISTING: {
      env = sp_env_copy(&config->env);
      break;
    }
    case SP_PS_ENV_CLEAN: {
    }
  }

  // Clean just means the base environment, so always apply extras
  for (u32 i = 0; i < SP_PS_MAX_ENV; i++) {
    if (sp_str_empty(config->extra[i].key)) break;

    sp_str_t key = config->extra[i].key;
    sp_str_t val = config->extra[i].value;
    sp_ht_insert(env.vars, key, val);
  }

  sp_ht_for(env.vars, it) {
    sp_str_t key = *sp_ht_it_getkp(env.vars, it);
    sp_str_t val = *sp_ht_it_getp(env.vars, it);

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append_fmt(&builder, "{}={}", SP_FMT_STR(key), SP_FMT_STR(val));
    sp_dyn_array_push(envp, sp_str_builder_write_cstr(&builder));
  }

  sp_dyn_array_push(envp, SP_NULLPTR);
  return envp;
}

void sp_ps_free_envp(c8** env, sp_ps_env_mode_t mode) {
  if (!env) return;

  u32 start_index = 0;
  if (mode == SP_PS_ENV_INHERIT) {
    for (c8** env = (c8**)environ; *env != SP_NULLPTR; env++) {
      start_index++;
    }
  }

  for (u32 i = start_index; env[i] != SP_NULLPTR; i++) {
    sp_free(env[i]);
  }
  sp_dyn_array_free(env);
}

sp_ps_config_t sp_ps_config_copy(const sp_ps_config_t* src) {
  sp_ps_config_t dst = SP_ZERO_INITIALIZE();

  dst.command = sp_str_copy(src->command);
  dst.cwd = sp_str_copy(src->cwd);

  for (u32 i = 0; i < SP_PS_MAX_ARGS; i++) {
    if (sp_str_empty(src->args[i])) break;
    dst.args[i] = sp_str_copy(src->args[i]);
  }

  dst.env.mode = src->env.mode;

  sp_env_table_t ht = src->env.env.vars;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    sp_str_t key = *sp_ht_it_getkp(ht, it);
    sp_str_t val = *sp_ht_it_getp(ht, it);
    sp_env_insert(&dst.env.env, key, val);
  }

  for (u32 i = 0; i < SP_PS_MAX_ENV; i++) {
    if (sp_str_empty(src->env.extra[i].key)) break;
    dst.env.extra[i].key = sp_str_copy(src->env.extra[i].key);
    dst.env.extra[i].value = sp_str_copy(src->env.extra[i].value);
  }

  dst.io = src->io;

  return dst;
}

void sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg) {
  SP_ASSERT(config != SP_NULLPTR);

  if (sp_str_empty(arg)) return;

  for (u32 i = 0; i < SP_PS_MAX_ARGS; i++) {
    if (sp_str_empty(config->args[i])) {
      config->args[i] = arg;
      if (i + 1 < SP_PS_MAX_ARGS) {
        config->args[i + 1] = SP_ZERO_STRUCT(sp_str_t);
      }
      return;
    }
  }

  SP_FATAL("sp_ps_config_add_arg: exceeded SP_PS_MAX_ARGS ({})", SP_FMT_U32(SP_PS_MAX_ARGS));
}

void sp_ps_configure_io_stream(sp_ps_io_stream_config_t* io, sp_ps_posix_stdio_stream_config_t* p) {
  switch (io->mode) {
    case SP_PS_IO_MODE_NULL: {
      SP_ASSERT(posix_spawn_file_actions_addopen(p->fa, p->file_number, "/dev/null", p->flag, p->mode) == 0);
      break;
    }
    case SP_PS_IO_MODE_CREATE: {
      s32 pipes [2] = { -1, -1 };
      SP_ASSERT(sp_ps_create_pipes(pipes));
      p->pipes.read = pipes[0];
      p->pipes.write = pipes[1];

      s32 duped = p->file_number == STDIN_FILENO ? p->pipes.read : p->pipes.write;
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, duped, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_EXISTING: {
      SP_ASSERT(io->stream.file.fd);
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, io->stream.file.fd, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_REDIRECT: {
      s32 redirect = p->file_number == SP_PS_IO_FILENO_STDOUT ? SP_PS_IO_FILENO_STDERR : SP_PS_IO_FILENO_STDOUT;
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, redirect, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_INHERIT: {
      break;
    }
  }
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  sp_context_ensure();

  sp_ps_t proc = SP_ZERO_STRUCT(sp_ps_t);
  proc.allocator = sp_context_get()->allocator;
  proc.io = config.io;

  SP_ASSERT(!sp_str_empty(config.command));

  c8** argv = sp_ps_build_posix_args(&config);
  c8** envp = sp_ps_build_posix_env(&config.env);

  posix_spawnattr_t attr;
  posix_spawn_file_actions_t fa;

  SP_ASSERT(posix_spawnattr_init(&attr) == 0);
  SP_ASSERT(posix_spawn_file_actions_init(&fa) == 0);

  if (!sp_str_empty(config.cwd)) {
    sp_ps_set_cwd(&fa, config.cwd);
  }

  sp_ps_posix_stdio_config_t io = {
    .in = (sp_ps_posix_stdio_stream_config_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDIN,
      .flag = O_RDONLY,
      .mode = 0x000,
      .pipes = { .read = -1, .write = -1 }
    },
    .out = (sp_ps_posix_stdio_stream_config_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDOUT,
      .flag = O_WRONLY,
      .mode = 0x644,
      .pipes = { .read = -1, .write = -1 },
    },
    .err = (sp_ps_posix_stdio_stream_config_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDERR,
      .flag = O_WRONLY,
      .mode = 0x644,
      .pipes = { .read = -1, .write = -1 },
    },
  };

  sp_ps_configure_io_stream(&proc.io.in, &io.in);

  if (proc.io.out.mode == SP_PS_IO_MODE_REDIRECT) {
    sp_ps_configure_io_stream(&proc.io.err, &io.err);
    sp_ps_configure_io_stream(&proc.io.out, &io.out);
  }
  else {
    sp_ps_configure_io_stream(&proc.io.out, &io.out);
    sp_ps_configure_io_stream(&proc.io.err, &io.err);
  }

  pid_t pid;
  if (posix_spawnp(&pid, argv[0], &fa, &attr, argv, envp) != 0) {
    posix_spawn_file_actions_destroy(&fa);
    posix_spawnattr_destroy(&attr);
    sp_ps_free_posix_args(argv);
    sp_ps_free_envp(envp, config.env.mode);
    if (io.in.pipes.read >= 0) { close(io.in.pipes.read); close(io.in.pipes.write); }
    if (io.out.pipes.read >= 0) { close(io.out.pipes.read); close(io.out.pipes.write); }
    if (io.err.pipes.read >= 0) { close(io.err.pipes.read); close(io.err.pipes.write); }

    return SP_ZERO_STRUCT(sp_ps_t);
  }

  proc.pid = pid;

  // For the 3 IO streams, we have two pipes; one for reading, one for writing. The parent only needs one end of each
  // (e.g. it doesn't make sense to read from the child's stdin), so close the unused end
  if (io.in.pipes.read >= 0) {
    close(io.in.pipes.read);

    switch (config.io.in.block) {
      case SP_PS_IO_NONBLOCKING: sp_ps_set_nonblocking(io.in.pipes.write);
      case SP_PS_IO_BLOCKING: sp_ps_set_blocking(io.in.pipes.write);
    }
    proc.io.in.stream = sp_io_from_file_handle(io.in.pipes.write, SP_IO_FILE_CLOSE_MODE_AUTO);
  }

  if (io.out.pipes.read >= 0) {
    close(io.out.pipes.write);

    switch (config.io.in.block) {
      case SP_PS_IO_NONBLOCKING: sp_ps_set_nonblocking(io.in.pipes.write);
      case SP_PS_IO_BLOCKING: sp_ps_set_blocking(io.in.pipes.write);
    }
    proc.io.out.stream = sp_io_from_file_handle(io.out.pipes.read, SP_IO_FILE_CLOSE_MODE_AUTO);
  }

  if (io.err.pipes.read >= 0) {
    close(io.err.pipes.write);

    switch (config.io.in.block) {
      case SP_PS_IO_NONBLOCKING: sp_ps_set_nonblocking(io.in.pipes.write);
      case SP_PS_IO_BLOCKING: sp_ps_set_blocking(io.in.pipes.write);
    }
    proc.io.err.stream = sp_io_from_file_handle(io.err.pipes.read, SP_IO_FILE_CLOSE_MODE_AUTO);
  }

  posix_spawn_file_actions_destroy(&fa);
  posix_spawnattr_destroy(&attr);
  sp_ps_free_posix_args(argv);
  sp_ps_free_envp(envp, config.env.mode);

  return proc;
}

sp_ps_output_t sp_ps_run(sp_ps_config_t config) {
  config.io.out = (sp_ps_io_stream_config_t) {
    .mode = SP_PS_IO_MODE_CREATE
  };
  sp_ps_t ps = sp_ps_create(config);
  return sp_ps_output(&ps);
}

void sp_ps_set_cwd(posix_spawn_file_actions_t* fa, sp_str_t cwd) {
  const c8* cwd_cstr = sp_str_to_cstr(cwd);
  SP_ASSERT(posix_spawn_file_actions_addchdir_np(fa, cwd_cstr) == 0);
}

sp_io_stream_t* sp_ps_io_redirect(sp_ps_t* proc, sp_ps_io_file_number_t file_number) {
  switch (file_number) {
    case SP_PS_IO_FILENO_STDIN: return &proc->io.in.stream;
    case SP_PS_IO_FILENO_STDOUT: return &proc->io.out.stream;
    case SP_PS_IO_FILENO_STDERR: return &proc->io.err.stream;
  }
  SP_UNREACHABLE_RETURN(SP_NULLPTR);
}

sp_io_stream_t* sp_ps_io(sp_ps_t* proc, sp_ps_io_stream_config_t* config) {
  SP_ASSERT(proc != SP_NULLPTR);
  switch (config->mode) {
    case SP_PS_IO_MODE_CREATE:
    case SP_PS_IO_MODE_EXISTING: {
      return &config->stream;
    }
    case SP_PS_IO_MODE_REDIRECT:
    case SP_PS_IO_MODE_INHERIT:
    case SP_PS_IO_MODE_NULL: {
      return SP_NULLPTR;
    }
  }

  SP_UNREACHABLE_RETURN(SP_NULLPTR);
}

sp_io_stream_t* sp_ps_io_in(sp_ps_t* proc) {
  return sp_ps_io(proc, &proc->io.in);
}

sp_io_stream_t* sp_ps_io_out(sp_ps_t* proc) {
  return sp_ps_io(proc, &proc->io.out);
}

sp_io_stream_t* sp_ps_io_err(sp_ps_t* proc) {
  return sp_ps_io(proc, &proc->io.err);
}

sp_ps_status_t sp_ps_poll(sp_ps_t* ps, u32 timeout_ms) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  s32 wait_status = 0;
  s32 wait_result = 0;
  s32 time_remaining = timeout_ms;

  do {
    wait_result = waitpid(ps->pid, &wait_status, SP_POSIX_WAITPID_NO_BLOCK);
    if (wait_result == 0) {
      result.state = SP_PS_STATE_RUNNING;
    }
    else if (wait_result > 0) {
      result.state = SP_PS_STATE_DONE;

      if (WIFEXITED(wait_status)) {
        result.exit_code = WEXITSTATUS(wait_status);
      }
      else if (WIFSIGNALED(wait_status)) {
        result.exit_code = -1 * WTERMSIG(wait_status);
      }
      else {
        result.exit_code = -255;
      }

      return result;
    }
    else if (wait_result < 0 && errno == EINTR) {
      continue;
    }
    else if (wait_result < 0) {
      sp_err_set(SP_ERR_LAZY);
      result.state = SP_PS_STATE_DONE;
      return result;
    }

    s32 poll_wait = SP_MIN(time_remaining, 10);
    if (poll_wait > 0) {
      sp_os_sleep_ms(poll_wait);
      time_remaining -= poll_wait;
    }
  } while (time_remaining > 0);

  return result;
}

sp_ps_status_t sp_ps_wait(sp_ps_t* ps) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  s32 wait_status = 0;
  s32 wait_result = 0;

  do {
    wait_result = waitpid(ps->pid, &wait_status, SP_POSIX_WAITPID_BLOCK);
  } while (wait_result == -1 && errno == EINTR);

  if (wait_result < 0) {
    sp_err_set(SP_ERR_LAZY);
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  result.state = SP_PS_STATE_DONE;

  if (WIFEXITED(wait_status)) {
    result.exit_code = WEXITSTATUS(wait_status);
  }
  else if (WIFSIGNALED(wait_status)) {
    result.exit_code = -1 * WTERMSIG(wait_status);
  }
  else {
    result.exit_code = -255;
  }

  return result;
}

sp_ps_output_t sp_ps_output(sp_ps_t* proc) {
  sp_ps_output_t result = SP_ZERO_INITIALIZE();

  result.status = sp_ps_wait(proc);

  u8 buffer [4096] = SP_ZERO_INITIALIZE();

  sp_io_stream_t* out = sp_ps_io_out(proc);
  if (out) {
    sp_str_builder_t builder = SP_ZERO_INITIALIZE();

    while (true) {
      u64 num_bytes = sp_io_read(out, buffer, sizeof(buffer));
      if (!num_bytes) {
        break;
      }

      sp_str_t chunk = sp_str(buffer, num_bytes);
      sp_str_builder_append(&builder, chunk);
    }

    result.out = sp_str_builder_move(&builder);
  }

  sp_io_stream_t* err = sp_ps_io_err(proc);
  if (err) {
    sp_str_builder_t builder = SP_ZERO_INITIALIZE();

    while (true) {
      u64 num_bytes = sp_io_read(err, buffer, sizeof(buffer));
      if (!num_bytes) {
        break;
      }

      sp_str_t chunk = sp_str(buffer, num_bytes);
      sp_str_builder_append(&builder, chunk);
    }

    result.err = sp_str_builder_move(&builder);
  }

  return result;
}
#endif

#if defined(SP_MACOS)
  void sp_semaphore_init(sp_semaphore_t* semaphore) {
      *semaphore = dispatch_semaphore_create(0);
  }

  void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
      //dispatch_release(*semaphore);
  }

  void sp_semaphore_wait(sp_semaphore_t* semaphore) {
      dispatch_semaphore_wait(*semaphore, DISPATCH_TIME_FOREVER);
  }

  bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
      dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, ms * NSEC_PER_MSEC);
      return dispatch_semaphore_wait(*semaphore, timeout) == 0;
  }

  void sp_semaphore_signal(sp_semaphore_t* semaphore) {
      dispatch_semaphore_signal(*semaphore);
  }

  sp_str_t sp_os_get_executable_path() {
    c8 exe_path[PATH_MAX];
    u32 size = PATH_MAX;
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
      c8 real_path[PATH_MAX];
      if (realpath(exe_path, real_path)) {
        return sp_str_copy(sp_os_parent_path(sp_str_from_cstr(real_path)));
      }
    }
    return sp_str_lit("");
  }
#endif

#if defined(SP_LINUX)
sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) {
  switch (kind) {
    case SP_OS_LIB_SHARED: return SP_LIT("so");
    case SP_OS_LIB_STATIC: return SP_LIT("a");
  }

  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
  return sp_format("lib{}.{}", SP_FMT_STR(lib_name), SP_FMT_STR(sp_os_lib_kind_to_extension(kind)));
}

void sp_semaphore_init(sp_semaphore_t* semaphore) {
  sem_init(semaphore, 0, 0);
}

void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
  sem_destroy(semaphore);
}

void sp_semaphore_wait(sp_semaphore_t* semaphore) {
  sem_wait(semaphore);
}

bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += ms / 1000;
  ts.tv_nsec += (ms % 1000) * 1000000;
  if (ts.tv_nsec >= 1000000000) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000;
  }
  return sem_timedwait(semaphore, &ts) == 0;
}

void sp_semaphore_signal(sp_semaphore_t* semaphore) {
  sem_post(semaphore);
}

sp_str_t sp_os_get_executable_path() {
  c8 exe_path [PATH_MAX];
  sp_str_t file_path = {
    .len = (u32)readlink("/proc/self/exe", exe_path, PATH_MAX - 1),
    .data = exe_path
  };

  if (!file_path.len) {
    return sp_str_lit("");
  }

  return sp_str_copy(sp_os_parent_path(file_path));
}

sp_str_t sp_os_try_xdg_or_home(sp_str_t xdg, sp_str_t home_suffix) {
  sp_str_t path =  sp_os_get_env_var(xdg);
  if (sp_str_valid(path)) return path;

  path = sp_os_get_env_var(SP_LIT("HOME"));
  if (sp_str_valid(path)) return sp_os_join_path(path, home_suffix);

  return SP_ZERO_STRUCT(sp_str_t);
}

sp_str_t sp_os_get_storage_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_DATA_HOME"), SP_LIT(".local/share"));
}

sp_str_t sp_os_get_config_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_CONFIG_HOME"), SP_LIT(".config"));
}
#endif


//////////////
// PLATFORM //
//////////////
#ifdef SP_WIN32
  sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) { switch (kind) {
      case SP_OS_LIB_SHARED: return SP_LIT("dll");
      case SP_OS_LIB_STATIC: return SP_LIT("lib");
    }

    SP_UNREACHABLE();
  }

  sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
    return sp_format("{}.{}", SP_FMT_STR(lib_name), SP_FMT_STR(sp_os_lib_kind_to_extension(kind)));
  }

  sp_os_platform_kind_t sp_os_platform_kind() {
    return SP_OS_PLATFORM_WIN32;
  }

  sp_str_t sp_os_platform_name() {
    return sp_str_lit("win32");
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
        sp_str_t partial_path_str = SP_CSTR(partial_path_cstr);

        sp_str_builder_t builder = SP_ZERO_INITIALIZE();
        sp_str_builder_append(&builder, info->path);
        sp_str_builder_append(&builder, sp_str_lit("/"));
        sp_str_builder_append(&builder, partial_path_str);
        sp_str_t full_path = sp_str_builder_write(&builder);

        sp_os_normalize_path(full_path);

        sp_str_t file_name = sp_os_extract_file_name(full_path);
        sp_os_win32_file_monitor_add_change(monitor, full_path, file_name, events);


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

#ifdef SP_LINUX
  sp_os_platform_kind_t sp_os_platform_kind() {
    return SP_OS_PLATFORM_LINUX;
  }

  sp_str_t sp_os_platform_name() {
    return sp_str_lit("linux");
  }

  void sp_os_file_monitor_init(sp_file_monitor_t* monitor) {
    sp_os_linux_file_monitor_t* linux_monitor = (sp_os_linux_file_monitor_t*)sp_alloc(sizeof(sp_os_linux_file_monitor_t));

    linux_monitor->fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (linux_monitor->fd == -1) {
      // Handle error but don't crash
      linux_monitor->fd = 0;
    }

    sp_dynamic_array_init(&linux_monitor->watch_descs, sizeof(s32));
    sp_dynamic_array_init(&linux_monitor->watch_paths, sizeof(sp_str_t));

    monitor->os = linux_monitor;
  }

  void sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t path) {
    if (!monitor->os) return;
    sp_os_linux_file_monitor_t* linux_monitor = (sp_os_linux_file_monitor_t*)monitor->os;

    if (linux_monitor->fd <= 0) return;

    c8* path_cstr = sp_str_to_cstr(path);

    // Build mask based on what events we want to watch
    u32 mask = 0;
    if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) {
      mask |= IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE;
    }
    if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_ADDED) {
      mask |= IN_CREATE | IN_MOVED_TO;
    }
    if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) {
      mask |= IN_DELETE | IN_MOVED_FROM;
    }

    s32 wd = inotify_add_watch(linux_monitor->fd, path_cstr, mask);

    if (wd != -1) {
      sp_dynamic_array_push(&linux_monitor->watch_descs, &wd);
      sp_str_t path_copy = sp_str_copy(path);
      sp_dynamic_array_push(&linux_monitor->watch_paths, &path_copy);
    }

  }

  void sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
    // For inotify, we need to watch the directory containing the file
    sp_str_t dir_path = sp_os_parent_path(file_path);
    if (dir_path.len > 0) {
      sp_os_file_monitor_add_directory(monitor, dir_path);
    }
  }

  void sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor) {
    if (!monitor->os) return;

    sp_os_linux_file_monitor_t* linux_monitor = (sp_os_linux_file_monitor_t*)monitor->os;
    if (linux_monitor->fd <= 0) return;

    size_t len = read(linux_monitor->fd, linux_monitor->buffer, sizeof(linux_monitor->buffer));
    if (len <= 0) return;

    // Process all events in buffer
    char* ptr = (char*)linux_monitor->buffer;
    while (ptr < (char*)linux_monitor->buffer + len) {
      struct inotify_event* event = (struct inotify_event*)ptr;

      // Find which path this watch descriptor corresponds to
      for (u32 i = 0; i < linux_monitor->watch_descs.size; i++) {
        s32* wd = (s32*)sp_dynamic_array_at(&linux_monitor->watch_descs, i);
        if (*wd == event->wd) {
          sp_str_t* dir_path = (sp_str_t*)sp_dynamic_array_at(&linux_monitor->watch_paths, i);

          // Build full path if there's a filename
          sp_str_t file_name = SP_ZERO_STRUCT(sp_str_t);
          sp_str_t file_path = SP_ZERO_STRUCT(sp_str_t);

          if (event->len > 0 && event->name[0] != '\0') {
            file_name = sp_str(event->name, sp_cstr_len(event->name));

            // Build full path
            sp_str_builder_t builder = SP_ZERO_INITIALIZE();
            sp_str_builder_append(&builder, *dir_path);
            sp_str_builder_append(&builder, sp_str_lit("/"));
            sp_str_builder_append(&builder, file_name);
            file_path = sp_str_builder_write(&builder);
          } else {
            file_path = sp_str_copy(*dir_path);
            file_name = sp_os_extract_file_name(file_path);
          }

          // Convert inotify mask to our events
          sp_file_change_event_t events = SP_FILE_CHANGE_EVENT_NONE;
          if (event->mask & (IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE)) {
            events = (sp_file_change_event_t)(events | SP_FILE_CHANGE_EVENT_MODIFIED);
          }
          if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
            events = (sp_file_change_event_t)(events | SP_FILE_CHANGE_EVENT_ADDED);
          }
          if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
            events = (sp_file_change_event_t)(events | SP_FILE_CHANGE_EVENT_REMOVED);
          }

          // Add change to monitor's change list
          if (events != SP_FILE_CHANGE_EVENT_NONE) {
            sp_file_change_t change = {
              .file_path = file_path,
              .file_name = file_name,
              .events = events,
              .time = 0  // TODO: get actual time
            };
            sp_dynamic_array_push(&monitor->changes, &change);
          }
          break;
        }
      }

      ptr += sizeof(struct inotify_event) + event->len;
    }

    // Emit changes with debouncing
    sp_file_monitor_emit_changes(monitor);
  }
#endif

#ifdef SP_MACOS
sp_str_t sp_os_try_xdg_or_home(sp_str_t xdg, sp_str_t home_suffix) {
  sp_str_t path =  sp_os_get_env_var(xdg);
  if (sp_str_valid(path)) return path;

  path = sp_os_get_env_var(SP_LIT("HOME"));
  if (sp_str_valid(path)) return sp_os_join_path(path, home_suffix);

  return SP_ZERO_STRUCT(sp_str_t);
}

sp_str_t sp_os_get_storage_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_DATA_HOME"), SP_LIT(".local/share"));
}

sp_str_t sp_os_get_config_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_CONFIG_HOME"), SP_LIT(".config"));
}


  sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) {
    switch (kind) {
      case SP_OS_LIB_SHARED: return SP_LIT("dylib");
      case SP_OS_LIB_STATIC: return SP_LIT("a");
    }

    SP_UNREACHABLE();
  }

  sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
    return sp_format("lib{}.{}", SP_FMT_STR(lib_name), SP_FMT_STR(sp_os_lib_kind_to_extension(kind)));
  }

  sp_os_platform_kind_t sp_os_platform_kind() {
    return SP_OS_PLATFORM_MACOS;
  }

  sp_str_t sp_os_platform_name() {
    return sp_str_lit("macos");
  }

  void sp_os_file_monitor_init(sp_file_monitor_t* monitor) {
    (void)monitor;
    SP_BROKEN();
  }

  void sp_os_file_monitor_add_directory(sp_file_monitor_t* monitor, sp_str_t directory_path) {
    (void)monitor; (void)directory_path;
    SP_BROKEN();
  }

  void sp_os_file_monitor_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
    (void)monitor; (void)file_path;
    SP_BROKEN();
  }

  void sp_os_file_monitor_process_changes(sp_file_monitor_t* monitor) {
    (void)monitor;
    SP_BROKEN();
  }
#endif

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

////////////////////////////
// RING BUFFER IMPLEMENTATION
////////////////////////////
void* sp_ring_buffer_at(sp_ring_buffer_t* buffer, u32 index) {
    return buffer->data + ((buffer->head + buffer->element_size * index) % (buffer->capacity * buffer->element_size));
}

void sp_ring_buffer_init(sp_ring_buffer_t* buffer, u32 capacity, u32 element_size) {
    buffer->size = 0;
    buffer->head = 0;
    buffer->capacity = capacity;
    buffer->element_size = element_size;
    buffer->data = (u8*)sp_alloc(capacity * element_size);
    sp_os_zero_memory(buffer->data, capacity * element_size);
}

void* sp_ring_buffer_back(sp_ring_buffer_t* buffer) {
    SP_ASSERT(buffer->size);
    return sp_ring_buffer_at(buffer, buffer->size - 1);
}

void* sp_ring_buffer_push(sp_ring_buffer_t* buffer, void* data) {
    SP_ASSERT(buffer->size < buffer->capacity);

    u32 index = (buffer->head + buffer->size * buffer->element_size) % (buffer->capacity * buffer->element_size);
    sp_os_copy_memory(data, buffer->data + index, buffer->element_size);
    buffer->size += 1;
    return sp_ring_buffer_back(buffer);
}

void* sp_ring_buffer_push_zero(sp_ring_buffer_t* buffer) {
    SP_ASSERT(buffer->size < buffer->capacity);

    u32 index = (buffer->head + buffer->size * buffer->element_size) % (buffer->capacity * buffer->element_size);
    sp_os_zero_memory(buffer->data + index, buffer->element_size);
    buffer->size += 1;
    return sp_ring_buffer_back(buffer);
}

void* sp_ring_buffer_push_overwrite(sp_ring_buffer_t* buffer, void* data) {
    if (buffer->size == buffer->capacity) sp_ring_buffer_pop(buffer);
    return sp_ring_buffer_push(buffer, data);
}

void* sp_ring_buffer_push_overwrite_zero(sp_ring_buffer_t* buffer) {
    if (buffer->size == buffer->capacity) sp_ring_buffer_pop(buffer);
    return sp_ring_buffer_push_zero(buffer);
}

void* sp_ring_buffer_pop(sp_ring_buffer_t* buffer) {
    SP_ASSERT(buffer->size);

    void* element = buffer->data + buffer->head;
    buffer->head = (buffer->head + buffer->element_size) % (buffer->capacity * buffer->element_size);
    buffer->size--;
    return element;
}

u32 sp_ring_buffer_bytes(sp_ring_buffer_t* buffer) {
    return buffer->capacity * buffer->element_size;
}

void sp_ring_buffer_clear(sp_ring_buffer_t* buffer) {
    sp_os_zero_memory(buffer->data, sp_ring_buffer_bytes(buffer));
    buffer->size = 0;
    buffer->head = 0;
}

void sp_ring_buffer_destroy(sp_ring_buffer_t* buffer) {
    if (buffer->data) {
        buffer->data = NULL;
        buffer->size = 0;
        buffer->capacity = 0;
        buffer->head = 0;
    }
}

bool sp_ring_buffer_is_full(sp_ring_buffer_t* buffer) {
    return buffer->capacity == buffer->size;
}

bool sp_ring_buffer_is_empty(sp_ring_buffer_t* buffer) {
    return buffer->size == 0;
}

void* sp_ring_buffer_iter_deref(sp_ring_buffer_iterator_t* it) {
    return sp_ring_buffer_at(it->buffer, it->index);
}

void sp_ring_buffer_iter_next(sp_ring_buffer_iterator_t* it) {
    SP_ASSERT(it->index < (s32)it->buffer->size);
    it->index++;
}

void sp_ring_buffer_iter_prev(sp_ring_buffer_iterator_t* it) {
    SP_ASSERT(it->index >= 0 && it->index < (s32)it->buffer->size);
    it->index--;
}

bool sp_ring_buffer_iter_done(sp_ring_buffer_iterator_t* it) {
    if (it->reverse) return it->index < 0;
    return it->index >= (s32)it->buffer->size;
}

sp_ring_buffer_iterator_t sp_ring_buffer_iter(sp_ring_buffer_t* buffer) {
    sp_ring_buffer_iterator_t iterator;
    iterator.index = 0;
    iterator.reverse = false;
    iterator.buffer = buffer;
    return iterator;
}

sp_ring_buffer_iterator_t sp_ring_buffer_riter(sp_ring_buffer_t* buffer) {
    sp_ring_buffer_iterator_t iterator;
    iterator.index = buffer->size - 1;
    iterator.reverse = true;
    iterator.buffer = buffer;
    return iterator;
}

sp_future_t* sp_future_create(u32 size) {
  sp_context_ensure();

  sp_future_t* future = (sp_future_t*)sp_alloc(sizeof(sp_future_t));
  future->allocator = sp_context_get()->allocator;
  sp_atomic_s32_set(&future->ready, 0);
  future->value = sp_alloc(size);
  future->size = size;
  return future;
}

void sp_future_destroy(sp_future_t* future) {
  sp_context_push_allocator(future->allocator);
  sp_free(future);
  sp_context_pop();
}

void sp_future_set_value(sp_future_t* future, void* value) {
  sp_os_copy_memory(value, future->value, future->size);
  sp_atomic_s32_set(&future->ready, 1);
}


pthread_key_t sp_context_stack_key;
pthread_key_t sp_context_key;
pthread_once_t sp_context_keys_once = PTHREAD_ONCE_INIT;

void sp_context_stack_cleanup(void* ptr) {
  if (ptr) {
    free(ptr);
  }
}

void sp_context_keys_create_once() {
  pthread_key_create(&sp_context_stack_key, sp_context_stack_cleanup);
  pthread_key_create(&sp_context_key, NULL);
}

void sp_context_thread_init() {
  pthread_once(&sp_context_keys_once, sp_context_keys_create_once);

  if (pthread_getspecific(sp_context_stack_key) != NULL) {
    return;
  }

  sp_context_t* stack = (sp_context_t*)malloc(sizeof(sp_context_t) * SP_MAX_CONTEXT);
  memset(stack, 0, sizeof(sp_context_t) * SP_MAX_CONTEXT);
  pthread_setspecific(sp_context_stack_key, stack);

  pthread_setspecific(sp_context_key, &stack[0]);

  stack[0].allocator = sp_malloc_allocator_init();
}

sp_context_t* sp_context_get() {
  pthread_once(&sp_context_keys_once, sp_context_keys_create_once);

  sp_context_t* ctx = (sp_context_t*)pthread_getspecific(sp_context_key);
  if (ctx == NULL) {
    sp_context_thread_init();
    ctx = (sp_context_t*)pthread_getspecific(sp_context_key);
  }
  return ctx;
}

void sp_init(sp_config_t config) {
  sp_context_t* ctx = sp_context_get();
  *ctx = (sp_context_t) {
    .allocator = sp_allocator_default()
  };

  if (!sp_global.initted) {
    sp_mutex_init(&sp_global.mutex, SP_MUTEX_PLAIN);
  }
}

void sp_err_set(sp_err_t err) {
  (void)err;
}

s64 sp_io_memory_size(sp_io_stream_t* stream) {
  sp_io_memory_data_t* data = &stream->memory;
  return data->stop - data->base;
}

s64 sp_io_memory_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence) {
  sp_io_memory_data_t* data = &stream->memory;
  u8* new_pos;

  switch (whence) {
    case SP_IO_SEEK_SET: {
      new_pos = data->base + offset;
      break;
    }
    case SP_IO_SEEK_CUR: {
      new_pos = data->here + offset;
      break;
    }
    case SP_IO_SEEK_END: {
      new_pos = data->stop + offset;
      break;
    }
    default: {
      SP_UNREACHABLE();
    }
  }

  if (new_pos < data->base || new_pos > data->stop) {
    sp_err_set(SP_ERR_IO_SEEK_INVALID);
    return -1;
  }
  data->here = new_pos;
  return data->here - data->base;
}

u64 sp_io_memory_read(sp_io_stream_t* stream, void* ptr, u64 size) {
  sp_io_memory_data_t* data = &stream->memory;
  u64 available = data->stop - data->here;
  u64 to_read = SP_MIN(size, available);

  sp_os_copy_memory(data->here, ptr, to_read);
  data->here += to_read;
  return to_read;
}

u64 sp_io_memory_write(sp_io_stream_t* stream, const void* ptr, u64 size) {
  sp_io_memory_data_t* data = &stream->memory;
  u64 available = data->stop - data->here;
  u64 to_write = SP_MIN(size, available);

  sp_os_copy_memory(ptr, data->here, to_write);
  data->here += to_write;
  return to_write;
}

void sp_io_memory_close(sp_io_stream_t* stream) {
  (void)stream;
}

s64 sp_io_file_size(sp_io_stream_t* stream) {
  sp_io_file_data_t* data = &stream->file;
  s64 current = lseek(data->fd, 0, SEEK_CUR);
  if (current < 0) {
    sp_err_set(SP_ERR_IO);
    return -1;
  }
  s64 size = lseek(data->fd, 0, SEEK_END);
  if (size < 0) {
    sp_err_set(SP_ERR_IO);
    return -1;
  }
  lseek(data->fd, current, SEEK_SET);
  return size;
}

s64 sp_io_file_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence) {
  sp_io_file_data_t* data = &stream->file;
  int posix_whence;

  switch (whence) {
    case SP_IO_SEEK_SET: {
      posix_whence = SEEK_SET;
      break;
    }
    case SP_IO_SEEK_CUR: {
      posix_whence = SEEK_CUR;
      break;
    }
    case SP_IO_SEEK_END: {
      posix_whence = SEEK_END;
      break;
    }
    default: {
      SP_UNREACHABLE();
    }
  }

  s64 pos = lseek(data->fd, offset, posix_whence);
  if (pos < 0) {
    sp_err_set(SP_ERR_IO);
  }
  return pos;
}

u64 sp_io_file_read(sp_io_stream_t* stream, void* ptr, u64 size) {
  sp_io_file_data_t* data = &stream->file;
  s64 result = read(data->fd, ptr, size);
  if (result < 0) {
    sp_err_set(SP_ERR_IO);
    return 0;
  }
  return (u64)result;
}

u64 sp_io_file_write(sp_io_stream_t* stream, const void* ptr, u64 size) {
  sp_io_file_data_t* data = &stream->file;
  s64 result = write(data->fd, ptr, size);
  if (result < 0) {
    sp_err_set(SP_ERR_IO);
    return 0;
  }
  return (u64)result;
}

void sp_io_file_close(sp_io_stream_t* stream) {
  sp_io_file_data_t* data = &stream->file;
  if (data->close_mode == SP_IO_FILE_CLOSE_MODE_AUTO) {
    if (close(data->fd) < 0) {
      sp_err_set(SP_ERR_IO);
    }
  }
}

sp_io_stream_t sp_io_from_file(sp_str_t path, sp_io_mode_t mode) {
  int flags = 0;

  if ((mode & SP_IO_MODE_READ) && (mode & SP_IO_MODE_WRITE)) {
    flags = O_RDWR | O_CREAT;
  } else if ((mode & SP_IO_MODE_READ) && (mode & SP_IO_MODE_APPEND)) {
    flags = O_RDWR | O_CREAT | O_APPEND;
  } else if (mode & SP_IO_MODE_READ) {
    flags = O_RDONLY;
  } else if (mode & SP_IO_MODE_WRITE) {
    flags = O_WRONLY | O_CREAT | O_TRUNC;
  } else if (mode & SP_IO_MODE_APPEND) {
    flags = O_WRONLY | O_CREAT | O_APPEND;
  }
  const char* cpath = sp_str_to_cstr(path);
  int fd = open(cpath, flags, 0644);

  sp_io_stream_t stream = SP_ZERO_INITIALIZE();
  stream.callbacks = (sp_io_callbacks_t) {
    .size = sp_io_file_size,
    .seek = sp_io_file_seek,
    .read = sp_io_file_read,
    .write = sp_io_file_write,
    .close = sp_io_file_close,
  };
  stream.file.fd = fd;
  stream.file.close_mode = SP_IO_FILE_CLOSE_MODE_AUTO;

  if (fd < 0) {
    sp_err_set(SP_ERR_IO);
    return stream;
  }

  SP_ASSERT(sp_os_is_regular_file(path));

  return stream;
}

sp_io_stream_t sp_io_from_memory(void* memory, u64 size) {
  sp_io_stream_t stream = SP_ZERO_INITIALIZE();
  stream.memory.base = (u8*)memory;
  stream.memory.here = (u8*)memory;
  stream.memory.stop = (u8*)memory + size;
  stream.callbacks.size = sp_io_memory_size;
  stream.callbacks.seek = sp_io_memory_seek;
  stream.callbacks.read = sp_io_memory_read;
  stream.callbacks.write = sp_io_memory_write;
  stream.callbacks.close = sp_io_memory_close;

  return stream;
}

sp_io_stream_t sp_io_from_file_handle(sp_os_file_handle_t handle, sp_io_file_close_mode_t close_mode) {
  sp_io_stream_t stream = SP_ZERO_INITIALIZE();
  stream.callbacks = (sp_io_callbacks_t) {
    .size = sp_io_file_size,
    .seek = sp_io_file_seek,
    .read = sp_io_file_read,
    .write = sp_io_file_write,
    .close = sp_io_file_close,
  };
  stream.file.fd = handle;
  stream.file.close_mode = close_mode;

  return stream;
}

u64 sp_io_read(sp_io_stream_t* stream, void* ptr, u64 size) {
  SP_ASSERT(stream); SP_ASSERT(ptr); SP_ASSERT(stream->callbacks.read);
  u64 bytes = stream->callbacks.read(stream, ptr, size);
  if (bytes < size) sp_err_set(SP_ERR_IO_EOF);
  return bytes;
}

u64 sp_io_write(sp_io_stream_t* stream, const void* ptr, u64 size) {
  SP_ASSERT(stream); SP_ASSERT(ptr); SP_ASSERT(stream->callbacks.write);
  return stream->callbacks.write(stream, ptr, size);
}

u64 sp_io_write_str(sp_io_stream_t* stream, sp_str_t str) {
  SP_ASSERT(stream);
  SP_ASSERT(sp_str_valid(str));
  return sp_io_write(stream, str.data, str.len);
}

u64 sp_io_write_cstr(sp_io_stream_t* stream, const c8* cstr) {
  SP_ASSERT(stream);
  SP_ASSERT(cstr);
  return sp_io_write(stream, cstr, sp_cstr_len(cstr));
}

s64 sp_io_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence) {
  SP_ASSERT(stream); SP_ASSERT(stream->callbacks.seek);
  return stream->callbacks.seek(stream, offset, whence);
}

s64 sp_io_size(sp_io_stream_t* stream) {
  SP_ASSERT(stream); SP_ASSERT(stream->callbacks.size);
  return stream->callbacks.size(stream);
}

void sp_io_close(sp_io_stream_t* stream) {
  SP_ASSERT(stream); SP_ASSERT(stream->callbacks.close);
  stream->callbacks.close(stream);
}

sp_str_t sp_io_read_file(sp_str_t path) {
  sp_io_stream_t stream = sp_io_from_file(path, SP_IO_MODE_READ);
  if (stream.file.fd < 0) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  s64 size = sp_io_size(&stream);
  if (size < 0) {
    sp_io_close(&stream);
    return SP_ZERO_STRUCT(sp_str_t);
  }

  sp_str_t result = sp_str_alloc((u32)size);
  u64 bytes = sp_io_read(&stream, (void*)result.data, (u64)size);
  sp_io_close(&stream);

  if (bytes != (u64)size) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  result.len = (u32)size;
  return result;
}

#ifdef SP_APP
////////////////////
// ASSET REGISTRY //
////////////////////
void sp_asset_registry_init(sp_asset_registry_t* registry, sp_asset_registry_config_t config) {
  sp_mutex_init(&registry->mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->import_mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->completion_mutex, SP_MUTEX_PLAIN);

  sp_semaphore_init(&registry->semaphore);
  registry->shutdown_requested = false;

  sp_ring_buffer_init(&registry->import_queue, 128, sizeof(sp_asset_import_context_t));
  sp_ring_buffer_init(&registry->completion_queue, 128, sizeof(sp_asset_import_context_t));

  for (u32 index = 0; index < SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS; index++) {
    sp_asset_importer_config_t* cfg = &config.importers[index];
    if (cfg->kind == SP_ASSET_KIND_NONE) break;

    sp_asset_importer_t importer = (sp_asset_importer_t) {
      .kind = cfg->kind,
      .on_import = cfg->on_import,
      .on_completion = cfg->on_completion,
      .registry = registry
    };
    sp_dyn_array_push(registry->importers, importer);
  }

  sp_thread_init(&registry->thread, sp_asset_registry_thread_fn, registry);
}

void sp_asset_registry_shutdown(sp_asset_registry_t* registry) {
  sp_mutex_lock(&registry->mutex);
  registry->shutdown_requested = true;
  sp_mutex_unlock(&registry->mutex);

  sp_semaphore_signal(&registry->semaphore);

  sp_thread_join(&registry->thread);

  sp_mutex_destroy(&registry->mutex);
  sp_mutex_destroy(&registry->import_mutex);
  sp_mutex_destroy(&registry->completion_mutex);
  sp_semaphore_destroy(&registry->semaphore);
}

void sp_asset_registry_process_completions(sp_asset_registry_t* registry) {
  sp_mutex_lock(&registry->completion_mutex);
  while (!sp_ring_buffer_is_empty(&registry->completion_queue)) {
    sp_asset_import_context_t context = *((sp_asset_import_context_t*)sp_ring_buffer_pop(&registry->completion_queue));
    sp_mutex_unlock(&registry->completion_mutex);

    context.importer->on_completion(&context);

    sp_mutex_lock(&registry->mutex);
    sp_asset_t* asset = &registry->assets[context.asset_index];
    asset->state = SP_ASSET_STATE_COMPLETED;
    sp_future_set_value(context.future, &asset);
    sp_mutex_unlock(&registry->mutex);

    sp_mutex_lock(&registry->completion_mutex);
  }
  sp_mutex_unlock(&registry->completion_mutex);
}

sp_asset_t* sp_asset_registry_reserve(sp_asset_registry_t* registry) {
  sp_asset_t memory = SP_ZERO_INITIALIZE();
  sp_dyn_array_push(registry->assets, memory);
  return sp_dyn_array_back(registry->assets);
}

sp_asset_t* sp_asset_registry_add(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name, void* user_data) {
  sp_mutex_lock(&registry->mutex);
  sp_asset_t* asset = sp_asset_registry_reserve(registry);
  asset->kind = kind;
  asset->name = sp_str_copy(name);
  asset->state = SP_ASSET_STATE_COMPLETED;
  asset->data = user_data;
  sp_mutex_unlock(&registry->mutex);
  return asset;
}

sp_future_t* sp_asset_registry_import(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name, void* user_data) {
  sp_asset_importer_t* importer = sp_asset_registry_find_importer(registry, kind);
  SP_ASSERT(importer);

  sp_mutex_lock(&registry->mutex);
  sp_asset_t* asset = sp_asset_registry_reserve(registry);
  asset->kind = kind;
  asset->name = sp_str_copy(name);
  asset->state = SP_ASSET_STATE_QUEUED;
  u32 asset_index = sp_dyn_array_size(registry->assets) - 1;  // Get index of the asset we just added
  sp_mutex_unlock(&registry->mutex);

  sp_asset_import_context_t context = {
    .registry = registry,
    .importer = importer,
    .asset_index = asset_index,
    .future = sp_future_create(sizeof(sp_asset_t*)),
    .user_data = user_data,
  };

  sp_mutex_lock(&registry->import_mutex);
  sp_ring_buffer_push(&registry->import_queue, &context);
  sp_mutex_unlock(&registry->import_mutex);

  sp_semaphore_signal(&registry->semaphore);

  return context.future;
}

sp_asset_t* sp_asset_registry_find(sp_asset_registry_t* registry, sp_asset_kind_t kind, sp_str_t name) {
  sp_mutex_lock(&registry->mutex);
  sp_asset_t* found = SP_NULLPTR;
  sp_dyn_array_for(registry->assets, index) {
    sp_asset_t* asset = registry->assets + index;
    if (asset->kind == kind && sp_str_equal(asset->name, name)) {
      found = asset;
      break;
    }
  }
  sp_mutex_unlock(&registry->mutex);
  return found;
}

sp_asset_importer_t*  sp_asset_registry_find_importer(sp_asset_registry_t* registry, sp_asset_kind_t kind) {
  sp_mutex_lock(&registry->mutex);
  sp_dyn_array_for(registry->importers, index) {
    sp_asset_importer_t* importer = &registry->importers[index];
    if (importer->kind == kind) {
      sp_mutex_unlock(&registry->mutex);
      return importer;
    }
  }

  sp_mutex_unlock(&registry->mutex);
  return SP_NULLPTR;
}

s32 sp_asset_registry_thread_fn(void* user_data) {
  sp_asset_registry_t* registry = (sp_asset_registry_t*)user_data;
  while (true) {
    sp_semaphore_wait(&registry->semaphore);

    sp_mutex_lock(&registry->mutex);
    bool shutdown = registry->shutdown_requested;
    sp_mutex_unlock(&registry->mutex);
    if (shutdown) break;

    sp_mutex_lock(&registry->import_mutex);

    while (!sp_ring_buffer_is_empty(&registry->import_queue)) {
      sp_asset_import_context_t context = *((sp_asset_import_context_t*)sp_ring_buffer_pop(&registry->import_queue));

      sp_mutex_unlock(&registry->import_mutex);

      context.importer->on_import(&context);

      sp_mutex_lock(&registry->mutex);
      sp_asset_t* asset = &registry->assets[context.asset_index];
      asset->state = SP_ASSET_STATE_IMPORTED;
      sp_mutex_unlock(&registry->mutex);

      sp_mutex_lock(&registry->completion_mutex);
      sp_ring_buffer_push(&registry->completion_queue, &context);
      sp_mutex_unlock(&registry->completion_mutex);

      sp_mutex_lock(&registry->import_mutex);
    }

    sp_mutex_unlock(&registry->import_mutex);
  }

  return 0;
}
#endif

SP_END_EXTERN_C()

#ifdef SP_CPP
sp_str_t operator/(const sp_str_t& a, const sp_str_t& b) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append(&builder, b);
  sp_str_t result = sp_str_builder_write(&builder);
  return sp_os_normalize_path(result);
}

sp_str_t operator/(const sp_str_t& a, const c8* b) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append_cstr(&builder, b);
  sp_str_t result = sp_str_builder_write(&builder);
  return sp_os_normalize_path(result);
}
#endif

#endif // SP_SP_C
#endif // SP_IMPLEMENTATION
