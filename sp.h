/*
  sp.h -- the single header nonstandard library for C

  ▗▖ ▗▖ ▗▄▄▖ ▗▄▖  ▗▄▄▖▗▄▄▄▖
  ▐▌ ▐▌▐▌   ▐▌ ▐▌▐▌   ▐▌
  ▐▌ ▐▌ ▝▀▚▖▐▛▀▜▌▐▌▝▜▌▐▛▀▀▘
  ▝▚▄▞▘▗▄▄▞▘▐▌ ▐▌▝▚▄▞▘▐▙▄▄▖
  @usage

  Define the following before you include sp.h in exactly one C or C++ file[^1]:

    #define SP_IMPLEMENTATION

  You can change the linkage of sp.h by optionally defining the following macros
  alongside SP_IMPLEMENTATION. Several modules provide their own module-specific
  configurations.

    SP_API
    SP_EXPORT
    SP_IMPORT
    SP_PRIVATE
    SP_GLOBAL
    SP_SHARED_LIB


  ▗▖  ▗▖ ▗▄▖ ▗▄▄▄ ▗▖ ▗▖▗▖   ▗▄▄▄▖ ▗▄▄▖
  ▐▛▚▞▜▌▐▌ ▐▌▐▌  █▐▌ ▐▌▐▌   ▐▌   ▐▌
  ▐▌  ▐▌▐▌ ▐▌▐▌  █▐▌ ▐▌▐▌   ▐▛▀▀▘ ▝▀▚▖
  ▐▌  ▐▌▝▚▄▞▘▐▙▄▄▀▝▚▄▞▘▐▙▄▄▖▐▙▄▄▖▗▄▄▞▘
  @modules

  + marks a module which is particularly important
  - marks a module which is kind of shitty, in implementation or design
  @ marks a module which mostly exists as a wrapper

      sp_app           minimal game-style main loop
      sp_asset         multithreaded asset registry, importers
    @ sp_atomic        compiler intrinsic atomics
    + sp_context       thread-local allocator, scratch memory
    + sp_dyn_array     stb-style resizable array (intrusive T* + macros)
      sp_elf           minimal elf reading + writing + modification
      sp_env           environment variables
      sp_err           thread-local errno style error system
      sp_fixed_array   fixed size array, from stack or heap (void*)
    + sp_format        "a type-safe {:fg cyan} replacement", SP_FMT_CSTR("printf")
      sp_fmon          os native filesystem watching
    + sp_fs            path manipulation, filesystem, common system paths (e.g. appdata)
    - sp_future        pollable, heap allocated values
      sp_hash          pseudorandom hashing, terrible and stolen
    + sp_hash_table    stb-style hash table (macros)
    + sp_io            read and write to files and buffers
      sp_mem           fundamental memory APIs, allocators, scratch storage
    @ sp_mutex         os native mutex wrappers
    - sp_os            grab bag of platform bullshit
    + sp_ps            subprocesses
      sp_ring_buffer   single threaded ring buffer (intrusive T* + macros)
    @ sp_semaphore     os native semaphore wrappers
    @ sp_spin          efficient spin lock with pausing
    + sp_str           ptr + len strings, no null termination, fundamental c string APIs
    @ sp_thread        os native thread wrappers
      sp_time          high resolution timers, dates and times, epochs
      sp_utf8          encode, decode, validation, iteration


    SP_RT_MAX_CONTEXT
    SP_RT_NUM_SPIN_LOCK
    SP_MEM_ARENA_BLOCK_SIZE
    SP_PS_MAX_ARGS
    SP_PS_MAX_ENV


  ▗▄▄▄▖ ▗▄▖  ▗▄▖▗▄▄▄▖▗▖  ▗▖ ▗▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖
  ▐▌   ▐▌ ▐▌▐▌ ▐▌ █  ▐▛▚▖▐▌▐▌ ▐▌ █  ▐▌   ▐▌
  ▐▛▀▀▘▐▌ ▐▌▐▌ ▐▌ █  ▐▌ ▝▜▌▐▌ ▐▌ █  ▐▛▀▀▘ ▝▀▚▖
  ▐▌   ▝▚▄▞▘▝▚▄▞▘ █  ▐▌  ▐▌▝▚▄▞▘ █  ▐▙▄▄▖▗▄▄▞▘
  @footnotes

  [^1]: C and C++ compile your program in translation units (TUs); roughly, an atomic
  unit as far as the linker is concerned. Usually, it's accurate enough to think of a
  TU as a C file. The linker doesn't have to link functions inside the same C file,
  but it does have to link functions that live in a different C file, or a library.

  sp.h does not, of course, have a C file. And yet, since we'd like to use it by
  compiling it alongside our own program, it needs to go *somewhere*. There must be
  *some* TU which has all its symbols.

  This preprocessor guard lets you "turn on" the C file (the implementation) and jam
  it into an actual C file. You can even make an sp.c in your project which solely
  does this; or, you could compile sp.h into a genuine static or shared library.

*/
#ifndef SP_SP_H
#define SP_SP_H



//  ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗
// ██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝
// ██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗
// ██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║
// ╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝
// @config
//////////////
// PLATFORM //
//////////////
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

#ifdef __COSMOPOLITAN__
  #define SP_COSMO
  #define SP_POSIX
#endif


//////////////
// COMPILER //
//////////////
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

//////////////////
// ARCHITECTURE //
//////////////////
#if defined(__x86_64__) || defined(_M_X64)
  #define SP_AMD64
  #define SP_AMD
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
  #define SP_ARM64
  #define SP_ARM
#endif

//////////////
// LANGUAGE //
//////////////
#ifdef __cplusplus
  #define SP_CPP
#endif

/////////////
// LINKAGE //
/////////////
#if !defined(SP_PRIVATE)
  #define SP_PRIVATE static
#endif

#if !defined(SP_IMPORT)
  #if defined(SP_WIN32)
    #define SP_IMPORT __declspec(__dllimport)
  #else
    #define SP_IMPORT
  #endif
#endif

#if !defined(SP_EXPORT)
  #if defined(SP_WIN32)
    #define SP_EXPORT __declspec(__dllexport)
  #else
    #define SP_EXPORT __attribute__((visibility("default")))
  #endif
#endif


#if !defined(SP_API)
  #if defined(SP_SHARED_LIB)
    #if defined (SP_IMPLEMENTATION)
      #define SP_API SP_EXPORT
    #else
      #define SP_API SP_IMPORT
    #endif
  #else
    #define SP_API extern
  #endif
#endif

#if !defined(SP_GLOBAL)
  #if defined(SP_IMPLEMENTATION)
    #define SP_GLOBAL
  #else
    #define SP_GLOBAL extern
  #endif
#endif


// ███████╗███████╗ █████╗ ████████╗██╗   ██╗██████╗ ███████╗███████╗
// ██╔════╝██╔════╝██╔══██╗╚══██╔══╝██║   ██║██╔══██╗██╔════╝██╔════╝
// █████╗  █████╗  ███████║   ██║   ██║   ██║██████╔╝█████╗  ███████╗
// ██╔══╝  ██╔══╝  ██╔══██║   ██║   ██║   ██║██╔══██╗██╔══╝  ╚════██║
// ██║     ███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║███████╗███████║
// ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝
// @features
// #define SP_ELF_DISABLE
// #define SP_ELF_ENABLE

#if defined(SP_ELF_ENABLE)
  #define SP_ELF
#endif

#if defined(SP_FMON_ENABLE)
  #define SP_FMON
#endif

// LINUX
#ifdef SP_LINUX
  #ifndef SP_ELF_DISABLE
    #define SP_ELF
  #endif

  #ifndef SP_FMON_DISABLE
    #define SP_FMON
  #endif
#endif

// MACOS
#if defined(SP_MACOS)
  #if !defined(SP_FMON_DISABLE)
    #define SP_FMON
  #endif
#endif

// WIN32
#ifdef SP_WIN32
#endif



// ███╗   ███╗ █████╗  ██████╗██████╗  ██████╗ ███████╗
// ████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔═══██╗██╔════╝
// ██╔████╔██║███████║██║     ██████╔╝██║   ██║███████╗
// ██║╚██╔╝██║██╔══██║██║     ██╔══██╗██║   ██║╚════██║
// ██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║╚██████╔╝███████║
// ╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝
// @macro
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

#define sp_rval(T) SP_RVAL(T)
#define sp_begin_extern_c() SP_BEGIN_EXTERN_C()
#define sp_end_extern_c() SP_END_EXTERN_C()
#define sp_zero_initialize() SP_ZERO_INITIALIZE()

#define SP_FALLTHROUGH() ((void)0)
#define sp_fallthrough() SP_FALLTHROUGH()

#define SP_ZERO_STRUCT(t) SP_RVAL(t) SP_ZERO_INITIALIZE()
#define sp_zero_struct(t) SP_ZERO_STRUCT(t)
#define SP_ZERO_RETURN(t) { t __SP_ZERO_RETURN = SP_ZERO_STRUCT(t); return __dn_zero_return; }
#define sp_zero_return(t) SP_ZERO_RETURN(t)

#define SP_EXIT_SUCCESS() exit(0)
#define sp_exit_success() SP_EXIT_SUCCESS()
#define SP_EXIT_FAILURE() exit(1)
#define sp_exit_failure() SP_EXIT_FAILURE()
#define SP_ASSERT(condition) assert((condition))
#define SP_FATAL(FMT, ...) \
  do { \
    sp_str_t message = sp_format((FMT), ##__VA_ARGS__); \
    SP_LOG("{:color red}: {}", SP_FMT_CSTR("SP_FATAL()"), SP_FMT_STR(message)); \
    SP_EXIT_FAILURE(); \
  } while (0)
#define sp_fatal(FMT, ...) SP_FATAL(FMT, ##__VA_ARGS__)

#define SP_UNREACHABLE() SP_ASSERT(false)
#define sp_unreachable() SP_UNREACHABLE()
#define SP_UNREACHABLE_CASE() SP_ASSERT(false); break;
#define sp_unreachable_case() SP_UNREACHABLE_CASE()
#define SP_UNREACHABLE_RETURN(v) SP_ASSERT(false); return (v)
#define sp_unreachable_return(v) SP_UNREACHABLE_RETURN(v)
//#define SP_BROKEN() SP_ASSERT(false)
#define SP_BROKEN()
#define sp_broken() SP_BROKEN()
#define SP_ASSERT_FMT(COND, FMT, ...) \
  do { \
    if (!(COND)) { \
      const c8* condition = SP_MACRO_STR(COND); \
      sp_str_t message = sp_format((FMT), ##__VA_ARGS__); \
      SP_LOG("SP_ASSERT({:color red}): {}", SP_FMT_CSTR(condition), SP_FMT_STR(message)); \
      SP_EXIT_FAILURE(); \
    } \
  } while (0)
#define sp_assert_fmt(COND, FMT, ...) SP_ASSERT_FMT(COND, FMT, ##__VA_ARGS__)
#define SP_UNTESTED()
#define sp_untested() SP_UNTESTED()
#define SP_INCOMPLETE()
#define sp_incomplete() SP_INCOMPLETE()

#define sp_assert(expr) SP_ASSERT(expr)

#define SP_TYPEDEF_FN(return_type, name, ...) typedef return_type(*name)(__VA_ARGS__)
#define sp_typedef_fn(return_type, name, ...) SP_TYPEDEF_FN(return_type, name, __VA_ARGS__)

#define SP_UNUSED(x) ((void)(x))
#define sp_unused(x) SP_UNUSED(x)

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
#define sp_macro_str(x) SP_MACRO_STR(x)
#define _SP_MACRO_CAT(x, y) x##y
#define SP_MACRO_CAT(x, y) _SP_MACRO_CAT(x, y)
#define sp_macro_cat(x, y) SP_MACRO_CAT(x, y)

#define SP_UNIQUE_ID() SP_MACRO_CAT(__sp_unique_name__, __LINE__)
#define sp_unique_id() SP_UNIQUE_ID()

#define SP_MAX(a, b) (a) > (b) ? (a) : (b)
#define sp_max(a, b) SP_MAX(a, b)
#define SP_MIN(a, b) (a) > (b) ? (b) : (a)
#define sp_min(a, b) SP_MIN(a, b)
#define SP_SWAP(t, a, b) { t SP_UNIQUE_ID() = (a); (a) = (b); (b) = SP_UNIQUE_ID(); }
#define sp_swap(t, a, b) SP_SWAP(t, a, b)

#define SP_QSORT_A_FIRST -1
#define SP_QSORT_B_FIRST 1
#define SP_QSORT_EQUAL 0

#define SP_COLOR_RGB(RED, GREEN, BLUE) { .r = (RED) / 255.f, .g = (GREEN) / 255.f, .b = (BLUE) / 255.f, .a = 1.0 }
#define sp_color_rgb_lit(RED, GREEN, BLUE) SP_COLOR_RGB(RED, GREEN, BLUE)
#define SP_COLOR_HSV(H, S, V) { .h = (H), .s = (S), .v = (V) }
#define sp_color_hsv_lit(H, S, V) SP_COLOR_HSV(H, S, V)

#define SP_MAX_PATH_LEN 260

#define SP_X_ENUM_CASE_TO_CSTR(ID)         case ID: { return SP_MACRO_STR(ID); }
#define SP_X_ENUM_CASE_TO_STRING(ID)       case ID: { return SP_LIT(SP_MACRO_STR(ID)); }
#define SP_X_ENUM_CASE_TO_STRING_UPPER(ID) case ID: { return sp_str_to_upper(SP_LIT(SP_MACRO_STR(ID))); }
#define SP_X_ENUM_CASE_TO_STRING_LOWER(ID) case ID: { return sp_str_to_lower(SP_LIT(SP_MACRO_STR(ID))); }
#define SP_X_ENUM_DEFINE(ID) ID,

#define SP_X_NAMED_ENUM_CASE_TO_CSTR(ID, NAME)         case ID: { return (NAME); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING(ID, NAME)       case ID: { return sp_str_lit(NAME); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING_UPPER(ID, NAME) case ID: { return sp_str_to_upper(sp_str_lit(NAME)); }
#define SP_X_NAMED_ENUM_CASE_TO_STRING_LOWER(ID, NAME) case ID: { return sp_str_to_lower(sp_str_lit(NAME)); }
#define SP_X_NAMED_ENUM_DEFINE(ID, NAME) ID,
#define SP_X_NAMED_ENUM_STR_TO_ENUM(ID, NAME) if (sp_str_equal(str, SP_LIT(NAME))) return ID;

#define SP_CARR_LEN(CARR) (sizeof((CARR)) / sizeof((CARR)[0]))
#define SP_CARR_FOR(CARR, IT) for (u32 IT = 0; IT < SP_CARR_LEN(CARR); IT++)
#define sp_carr_for(CARR, IT) SP_CARR_FOR(CARR, IT)
#define sp_carr_len(CARR) (sizeof((CARR)) / sizeof((CARR)[0]))

#define sp_for(it, n) for (u32 it = 0; it < n; it++)
#define sp_for_range(it, low, high) for (u32 it = (low); it < (high); it++)

#define SP_SIZE_TO_INDEX(size) ((size) ? ((size) - 1) : 0)
#define sp_size_to_index(size) SP_SIZE_TO_INDEX(size)

#define SP_MEM_ALIGNMENT 16

#define sp_align_up(ptr, align) ((void*)(((uintptr_t)(ptr) + ((align) - 1)) & ~((align) - 1)))
#define sp_align_offset(val, align) ((((val) + ((align) - 1)) & ~((align) - 1)))

#if defined(SP_USE_ASSERTS)
  #define sp_try(expr) sp_assert(!expr)
  #define sp_try_as(expr, err) sp_assert(!expr)
  #define sp_try_as_void(expr) sp_assert(!expr)
#else
  #define sp_try(expr) do { s32 _sp_result = (expr); if (_sp_result) return _sp_result; } while (0)
  #define sp_try_as(expr, err) do { if (expr) return err; } while (0)
  #define sp_try_as_void(expr) do { if (expr) return; } while (0)
#endif

#define sp_try_as_null(expr) sp_try_as((expr), SP_NULLPTR)
#define sp_require(expr) sp_try_as_void(!(expr))
#define sp_require_as(expr, err) sp_try_as(!(expr), err)
#define sp_require_as_null(expr) sp_try_as(!(expr), SP_NULLPTR)



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


// ███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗
// ██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║
// ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║
// ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║
// ███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║
// ╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝
// @system
SP_BEGIN_EXTERN_C()
#if defined(SP_COSMO)
  #ifndef _COSMO_SOURCE
    #define _COSMO_SOURCE
  #endif
#endif

#if defined(SP_LINUX)
  #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
  #endif
#endif

#if defined(SP_MACOS)
  #ifndef _DARWIN_C_SOURCE
    #define _DARWIN_C_SOURCE
  #endif
#endif

#if defined(SP_POSIX)
  #ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
  #endif
#endif

#if defined(SP_WIN32)
  #if defined(UNICODE)
    #undef UNICODE
  #endif

  #if !defined(WIN32_LEAN_AND_MEAN)
    #define WIN32_LEAN_AND_MEAN
  #endif

  #if !defined(NOMINMAX)
    #define NOMINMAX
  #endif

  #if !defined(_CRT_RAND_S)
    #define _CRT_RAND_S
  #endif
#endif

#if defined(SP_WIN32)
  #include <windows.h>
  #include <shlobj.h>
  #include <commdlg.h>
  #include <shellapi.h>
  #include <threads.h>
#endif

#if defined(SP_COSMO)
  #include "libc/dce.h"
#endif

#if defined(SP_LINUX)
  #include <poll.h>
  #include <sys/inotify.h>
#endif

#if defined(SP_MACOS)
  #include <dispatch/dispatch.h>
  #include <mach-o/dyld.h>
  #include <sys/event.h>
  #if defined(SP_FMON_MACOS_USE_FSEVENTS)
    #include <CoreServices/CoreServices.h>
  #endif
#endif

#if defined(SP_POSIX)
  #include <dirent.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <limits.h>
  #include <pthread.h>
  #include <semaphore.h>
  #include <signal.h>
  #include <spawn.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <time.h>
#endif

#if !defined(SP_NO_LIBM)
  #include <math.h>
  #define SP_SINF sinf
  #define SP_COSF cosf
  #define SP_TANF tanf
  #define SP_SQRTF sqrtf
  #define SP_ACOSF acosf
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

extern char** environ;

SP_END_EXTERN_C()

#ifdef SP_CPP
  #include <atomic>
#endif


SP_BEGIN_EXTERN_C()


// ████████╗██╗   ██╗██████╗ ███████╗███████╗
// ╚══██╔══╝╚██╗ ██╔╝██╔══██╗██╔════╝██╔════╝
//    ██║    ╚████╔╝ ██████╔╝█████╗  ███████╗
//    ██║     ╚██╔╝  ██╔═══╝ ██╔══╝  ╚════██║
//    ██║      ██║   ██║     ███████╗███████║
//    ╚═╝      ╚═╝   ╚═╝     ╚══════╝╚══════╝
// @types
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

#define SP_LIMIT_S8_MIN   INT8_MIN
#define SP_LIMIT_S8_MAX   INT8_MAX
#define SP_LIMIT_S16_MIN  INT16_MIN
#define SP_LIMIT_S16_MAX  INT16_MAX
#define SP_LIMIT_S32_MIN  INT32_MIN
#define SP_LIMIT_S32_MAX  INT32_MAX
#define SP_LIMIT_S64_MIN  INT64_MIN
#define SP_LIMIT_S64_MAX  INT64_MAX
#define SP_LIMIT_U8_MAX   UINT8_MAX
#define SP_LIMIT_U16_MAX  UINT16_MAX
#define SP_LIMIT_U32_MAX  UINT32_MAX
#define SP_LIMIT_U64_MAX  UINT64_MAX
#define SP_LIMIT_F32_MIN  FLT_MIN
#define SP_LIMIT_F32_MAX  FLT_MAX
#define SP_LIMIT_F64_MIN  DBL_MIN
#define SP_LIMIT_F64_MAX  DBL_MAX

#define SP_LIMIT_EPOCH_MIN SP_ZERO_STRUCT(sp_tm_epoch_t)
#define SP_LIMIT_EPOCH_MAX SP_RVAL(sp_tm_epoch_t) { .s = SP_LIMIT_U64_MAX, .ns = SP_LIMIT_U32_MAX }

typedef enum {
  SP_OPT_NONE = 0,
  SP_OPT_SOME = 1,
} sp_optional_t;

#define sp_opt(T) struct { \
  T value; \
  sp_optional_t some; \
}

#define sp_opt_set(O, V)  do { (O).value = (V); (O).some = SP_OPT_SOME; } while (0)
#define sp_opt_get(O)     (O).value
#define sp_opt_some(V)    { .value = V, .some = SP_OPT_SOME }
#define sp_opt_none()    { .some = SP_OPT_NONE }
#define sp_opt_is_null(V) ((V).some == SP_OPT_NONE)

// ███╗   ███╗ █████╗ ████████╗██╗  ██╗
// ████╗ ████║██╔══██╗╚══██╔══╝██║  ██║
// ██╔████╔██║███████║   ██║   ███████║
// ██║╚██╔╝██║██╔══██║   ██║   ██╔══██║
// ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║
// ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝
// @math
// Totally ripped from Handmade Math and tweaked to follow sp.h's style conventions. It's public domain, but here's
// the credits section from the version I ripped. Any usefulness of any of the vector code in this or derived
// libraries is solely due to the excellent work done by the following folks:
//
// https://github.com/HandmadeMath/HandmadeMath
//
//   CREDITS
//
//   Originally written by Zakary Strange.
//
//   Functionality:
//    Zakary Strange (strangezak@protonmail.com && @strangezak)
//    Matt Mascarenhas (@miblo_)
//    Aleph
//    FieryDrake (@fierydrake)
//    Gingerbill (@TheGingerBill)
//    Ben Visness (@bvisness)
//    Trinton Bullard (@Peliex_Dev)
//    @AntonDan
//    Logan Forman (@dev_dwarf)
//
//   Fixes:
//    Jeroen van Rijn (@J_vanRijn)
//    Kiljacken (@Kiljacken)
//    Insofaras (@insofaras)
//    Daniel Gibson (@DanielGibson)
#define SP_ABS(a) ((a) > 0 ? (a) : -(a))
#define sp_abs(a) SP_ABS(a)
#define SP_MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define sp_mod(a, m) SP_MOD(a, m)
#define SP_SQUARE(x) ((x) * (x))
#define sp_square(x) SP_SQUARE(x)

typedef union sp_vec2 {
  struct {
    f32 x, y;
  };

  struct {
    f32 u, v;
  };

  struct {
    f32 left, right;
  };

  struct {
    f32 width, height;
  };

  f32 elements[2];
} sp_vec2_t;

typedef union sp_vec3 {
  struct {
    f32 x, y, z;
  };

  struct {
    f32 u, v, w;
  };

  struct {
    f32 R, G, B;
  };

  struct {
    sp_vec2_t xy;
    f32 unused0;
  };

  struct {
    f32 unused1;
    sp_vec2_t yz;
  };

  struct {
    sp_vec2_t uv;
    f32 unused2;
  };

  struct {
    f32 unused3;
    sp_vec2_t vw;
  };

  f32 elements[3];
} sp_vec3_t;

typedef union sp_vec4 {
  struct {
    union {
      sp_vec3_t xyz;
      struct { f32 x, y, z; };
    };

    f32 w;
  };

  struct {
    union {
      sp_vec3_t rgb;
      struct { f32 r, g, b; };

      sp_vec3_t hsv;
      struct { f32 h, s, v; };
    };

    f32 a;
  };

  struct {
    sp_vec2_t xy;
    f32 unused0;
    f32 unused1;
  };

  struct {
    f32 unused2;
    sp_vec2_t yz;
    f32 unused3;
  };

  struct {
    f32 unused4;
    f32 unused5;
    sp_vec2_t zw;
  };

  f32 elements[4];
} sp_vec4_t;

typedef union sp_mat2 {
  f32 elements[2][2];
  sp_vec2_t columns[2];
} sp_mat2_t;

typedef union sp_mat3 {
  f32 elements[3][3];
  sp_vec3_t columns[3];
} sp_mat3_t;

typedef union sp_mat4 {
  f32 elements[4][4];
  sp_vec4_t columns[4];
} sp_mat4_t;

typedef union sp_quat {
  struct {
    union {
      sp_vec3_t xyz;
      struct { f32 x, y, z; };
    };

    f32 w;
  };

  f32 elements[4];
} sp_quat_t;

typedef sp_vec4_t sp_color_t;

typedef enum sp_interp_mode_t {
  SP_INTERP_MODE_LERP,
  SP_INTERP_MODE_EASE_IN,
  SP_INTERP_MODE_EASE_OUT,
  SP_INTERP_MODE_EASE_INOUT,
  SP_INTERP_MODE_EASE_INOUT_BOUNCE,
  SP_INTERP_MODE_EXPONENTIAL,
  SP_INTERP_MODE_PARABOLIC,
  SP_INTERP_MODE_COUNT
} sp_interp_mode_t;

typedef struct sp_interp_t {
  f32 start;
  f32 delta;
  f32 t;
  f32 time_scale;
} sp_interp_t;

SP_API sp_color_t   sp_color_rgb_255(u8 r, u8 g, u8 b);
SP_API sp_color_t   sp_color_rgb_to_hsv(sp_color_t color);
SP_API sp_color_t   sp_color_hsv_to_rgb(sp_color_t color);
SP_API f32          sp_inv_sqrtf(f32 value);
SP_API f32          sp_lerp(f32 a, f32 t, f32 b);
SP_API f32          sp_clamp(f32 low, f32 value, f32 high);
SP_API sp_vec2_t    sp_vec2(f32 x, f32 y);
SP_API sp_vec3_t    sp_vec3(f32 x, f32 y, f32 z);
SP_API sp_vec4_t    sp_vec4(f32 x, f32 y, f32 z, f32 w);
SP_API sp_vec4_t    sp_vec4V(sp_vec3_t xyz, f32 w);
SP_API sp_vec2_t    sp_vec2_add(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec3_t    sp_vec3_add(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec4_t    sp_vec4_add(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec2_t    sp_vec2_sub(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec3_t    sp_vec3_sub(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec4_t    sp_vec4_sub(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec2_t    sp_vec2_mul(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec2_t    sp_vec2_scale(sp_vec2_t left, f32 right);
SP_API sp_vec3_t    sp_vec3_mul(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec3_t    sp_vec3_scale(sp_vec3_t left, f32 right);
SP_API sp_vec4_t    sp_vec4_mul(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec4_t    sp_vec4_scale(sp_vec4_t left, f32 right);
SP_API sp_vec2_t    sp_vec2_div(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec2_t    sp_vec2_divf(sp_vec2_t left, f32 right);
SP_API sp_vec3_t    sp_vec3_div(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec3_t    sp_vec3_divf(sp_vec3_t left, f32 right);
SP_API sp_vec4_t    sp_vec4_div(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec4_t    sp_vec4_divf(sp_vec4_t left, f32 right);
SP_API bool         sp_vec2_eq(sp_vec2_t left, sp_vec2_t right);
SP_API bool         sp_vec3_eq(sp_vec3_t left, sp_vec3_t right);
SP_API bool         sp_vec4_eq(sp_vec4_t left, sp_vec4_t right);
SP_API f32          sp_vec2_dot(sp_vec2_t left, sp_vec2_t right);
SP_API f32          sp_vec3_dot(sp_vec3_t left, sp_vec3_t right);
SP_API f32          sp_vec4_dot(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec3_t    sp_vec3_cross(sp_vec3_t left, sp_vec3_t right);
SP_API f32          sp_vec2_len_sqr(sp_vec2_t v);
SP_API f32          sp_vec3_len_sqr(sp_vec3_t v);
SP_API f32          sp_vec4_len_sqr(sp_vec4_t v);
SP_API f32          sp_vec2_len(sp_vec2_t v);
SP_API f32          sp_vec3_len(sp_vec3_t v);
SP_API f32          sp_vec4_len(sp_vec4_t v);
SP_API sp_vec2_t    sp_vec2_norm(sp_vec2_t v);
SP_API sp_vec3_t    sp_vec3_norm(sp_vec3_t v);
SP_API sp_vec4_t    sp_vec4_norm(sp_vec4_t v);
SP_API sp_vec2_t    sp_vec2_lerp(sp_vec2_t a, f32 t, sp_vec2_t b);
SP_API sp_vec3_t    sp_vec3_lerp(sp_vec3_t a, f32 t, sp_vec3_t b);
SP_API sp_vec4_t    sp_vec4_lerp(sp_vec4_t a, f32 t, sp_vec4_t b);
SP_API sp_interp_t  sp_interp_build(f32 start, f32 target, f32 time);
SP_API bool         sp_interp_update(sp_interp_t* interp, f32 dt);
SP_API f32          sp_interp_lerp(sp_interp_t* interp);
SP_API f32          sp_interp_ease_in(sp_interp_t* interp);
SP_API f32          sp_interp_ease_out(sp_interp_t* interp);
SP_API f32          sp_interp_ease_inout(sp_interp_t* interp);
SP_API f32          sp_interp_ease_inout_bounce(sp_interp_t* interp);
SP_API f32          sp_interp_exponential(sp_interp_t* interp);
SP_API f32          sp_interp_parabolic(sp_interp_t* interp);



// ███████╗██████╗ ██████╗  ██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔══██╗
// █████╗  ██████╔╝██████╔╝██║   █║██████╔╝
// ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══██╗
// ███████╗██║  ██║██║  ██║╚██████╔╝██║  ██
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
// @error
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
  SP_ERR_OS,
} sp_err_t;

SP_API void sp_err_set(sp_err_t err);
SP_API sp_err_t sp_err_get();



// ███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗
// ████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗╚██╗ ██╔╝
// ██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝ ╚████╔╝
// ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗  ╚██╔╝
// ██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║   ██║
// ╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝
// @memory
#ifndef SP_MEM_ARENA_BLOCK_SIZE
  #define SP_MEM_ARENA_BLOCK_SIZE 4096
#endif

typedef enum {
  SP_ALLOCATOR_MODE_ALLOC,
  SP_ALLOCATOR_MODE_FREE,
  SP_ALLOCATOR_MODE_RESIZE,
} sp_mem_alloc_mode_t;

SP_TYPEDEF_FN(
  void*,
  sp_allocator_fn_t,
  void* user_data, sp_mem_alloc_mode_t mode, u32 size, void* ptr
);

typedef struct sp_allocator_t {
  sp_allocator_fn_t on_alloc;
  void* user_data;
} sp_allocator_t;

typedef struct sp_mem_arena_block_t {
  struct sp_mem_arena_block_t* next;
  u8* buffer;
  u32 capacity;
  u32 bytes_used;
} sp_mem_arena_block_t;

typedef struct {
  sp_allocator_t allocator;
  sp_mem_arena_block_t* head;
  sp_mem_arena_block_t* current;
  u32 block_size;
} sp_mem_arena_t;

typedef struct {
  u32 size;
  u8 padding [12];
} sp_mem_libc_metadata_t;

typedef struct {
  sp_mem_arena_t* arena;
  sp_mem_arena_block_t* block;
  u32 mark;
} sp_mem_arena_marker_t;

typedef struct {
  sp_mem_arena_marker_t marker;
  sp_allocator_t old_allocator;
} sp_mem_scratch_t;

SP_API void*                   sp_alloc(u32 size);
SP_API void*                   sp_realloc(void* memory, u32 size);
SP_API void                    sp_free(void* memory);
SP_API void*                   sp_mem_os_alloc(u32 size);
SP_API void*                   sp_mem_os_alloc_zero(u32 size);
SP_API void*                   sp_mem_os_realloc(void* ptr, u32 size);
SP_API void                    sp_mem_os_free(void* ptr);
SP_API void                    sp_mem_copy(const void* source, void* dest, u32 num_bytes);
SP_API void                    sp_mem_move(const void* source, void* dest, u32 num_bytes);
SP_API bool                    sp_mem_is_equal(const void* a, const void* b, size_t len);
SP_API void                    sp_mem_fill(void* buffer, u32 bsize, void* fill, u32 fsize);
SP_API void                    sp_mem_fill_u8(void* buffer, u32 buffer_size, u8 fill);
SP_API void                    sp_mem_zero(void* buffer, u32 buffer_size);
SP_API void*                   sp_mem_allocator_alloc(sp_allocator_t arena, u32 size);
SP_API void*                   sp_mem_allocator_realloc(sp_allocator_t arena, void* ptr, u32 size);
SP_API void                    sp_mem_allocator_free(sp_allocator_t arena, void* buffer);
SP_API sp_mem_arena_t*         sp_mem_arena_new(u32 default_block_size);
SP_API sp_allocator_t          sp_mem_arena_as_allocator(sp_mem_arena_t* arena);
SP_API void                    sp_mem_arena_clear(sp_mem_arena_t* arena);
SP_API void                    sp_mem_arena_destroy(sp_mem_arena_t* arena);
SP_API void*                   sp_mem_arena_on_alloc(void* ptr, sp_mem_alloc_mode_t mode, u32 n, void* old);
SP_API sp_mem_arena_marker_t   sp_mem_arena_mark(sp_mem_arena_t* a);
SP_API void                    sp_mem_arena_pop(sp_mem_arena_marker_t marker);
SP_API u32                     sp_mem_arena_capacity(sp_mem_arena_t* arena);
SP_API u32                     sp_mem_arena_bytes_used(sp_mem_arena_t* arena);
SP_API void*                   sp_mem_arena_alloc(sp_mem_arena_t* arena, u32 size);
SP_API void*                   sp_mem_arena_realloc(sp_mem_arena_t* arena, void* ptr, u32 size);
SP_API void                    sp_mem_arena_free(sp_mem_arena_t* arena, void* ptr);
SP_API sp_allocator_t          sp_mem_libc_new();
SP_API void*                   sp_mem_libc_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u32 size, void* ptr);
SP_API sp_mem_libc_metadata_t* sp_mem_libc_get_metadata(void* ptr);
SP_API sp_mem_arena_t*         sp_mem_get_scratch_arena();
SP_API sp_mem_scratch_t        sp_mem_begin_scratch();
SP_API void                    sp_mem_end_scratch(sp_mem_scratch_t scratch);

#define SP_ALLOC(T) (T*)sp_alloc(sizeof(T))
#define SP_ALLOC_N(T, n) (T*)sp_alloc((n) * sizeof(T))
#define SP_OS_ALLOC(T) (T*)sp_mem_os_alloc(sizeof(T))
#define sp_alloc_type(T) (T*)sp_alloc(sizeof(T))
#define sp_alloc_n(T, n) (T*)sp_alloc((n) * sizeof(T))
#define sp_os_alloc_type(T) (T*)sp_mem_os_alloc(sizeof(T))
#define sp_mem_allocator_alloc_type(a, T) (T*)sp_mem_allocator_alloc(a, sizeof(T))
#define sp_mem_allocator_alloc_n(a, T, n) (T*)sp_mem_allocator_alloc(a, (n) * sizeof(T))


// ██╗  ██╗ █████╗ ███████╗██╗  ██╗
// ██║  ██║██╔══██╗██╔════╝██║  ██║
// ███████║███████║███████╗███████║
// ██╔══██║██╔══██║╚════██║██╔══██║
// ██║  ██║██║  ██║███████║██║  ██║
// ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝
// @hash
typedef u64 sp_hash_t;

SP_API sp_hash_t sp_hash_cstr(const c8* str);
SP_API sp_hash_t sp_hash_combine(sp_hash_t* hashes, u32 num_hashes);
SP_API sp_hash_t sp_hash_bytes(const void* p, u64 len, u64 seed);


// ███████╗██╗██╗  ██╗███████╗██████╗      █████╗ ██████╗ ██████╗  █████╗ ██╗   ██╗
// ██╔════╝██║╚██╗██╔╝██╔════╝██╔══██╗    ██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝
// █████╗  ██║ ╚███╔╝ █████╗  ██║  ██║    ███████║██████╔╝██████╔╝███████║ ╚████╔╝
// ██╔══╝  ██║ ██╔██╗ ██╔══╝  ██║  ██║    ██╔══██║██╔══██╗██╔══██╗██╔══██║  ╚██╔╝
// ██║     ██║██╔╝ ██╗███████╗██████╔╝    ██║  ██║██║  ██║██║  ██║██║  ██║   ██║
// ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚═════╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝
// @fixed_array
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


// ██████╗ ██╗   ██╗███╗   ██╗     █████╗ ██████╗ ██████╗  █████╗ ██╗   ██╗
// ██╔══██╗╚██╗ ██╔╝████╗  ██║    ██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝
// ██║  ██║ ╚████╔╝ ██╔██╗ ██║    ███████║██████╔╝██████╔╝███████║ ╚████╔╝
// ██║  ██║  ╚██╔╝  ██║╚██╗██║    ██╔══██║██╔══██╗██╔══██╗██╔══██║  ╚██╔╝
// ██████╔╝   ██║   ██║ ╚████║    ██║  ██║██║  ██║██║  ██║██║  ██║   ██║
// ╚═════╝    ╚═╝   ╚═╝  ╚═══╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝
// @dyn_array
typedef struct sp_dyn_array {
  u32 size;
  u32 capacity;
} sp_dyn_array;

#define sp_dyn_array(T)             T*
#define sp_da(T)                    T*
SP_API void*                        sp_dyn_array_resize_impl(void* arr, u32 sz, u32 amount);
SP_API void**                       sp_dyn_array_init(void** arr, u32 val_len);
SP_API void                         sp_dyn_array_push_f(void** arr, void* val, u32 val_len);
#define sp_da_rfor(__ARR, __IT)     sp_dyn_array_rfor(__ARR, __IT)
#define sp_da_for(__ARR, __IT)      sp_dyn_array_for(__ARR, __IT)
#define sp_da_head(__ARR)           sp_dyn_array_head(__ARR)
#define sp_da_size(__ARR)           sp_dyn_array_size(__ARR)
#define sp_da_capacity(__ARR)       sp_dyn_array_capacity(__ARR)
#define sp_da_empty(__ARR)          sp_dyn_array_empty(__ARR)
#define sp_da_full(__ARR)           sp_dyn_array_full(__ARR)
#define sp_da_clear(__ARR)          sp_dyn_array_clear(__ARR)
#define sp_da_free(__ARR)           sp_dyn_array_free(__ARR)
#define sp_da_need_grow(__ARR, __N) sp_dyn_array_need_grow(__ARR, __N)
#define sp_da_grow(__ARR)           sp_dyn_array_grow(__ARR)
#define sp_da_push(__ARR, __VAL)    sp_dyn_array_push(__ARR, __VAL)
#define sp_da_reserve(__ARR, __N)   sp_dyn_array_reserve(__ARR, __N)
#define sp_da_pop(__ARR)            sp_dyn_array_pop(__ARR)
#define sp_da_back(__ARR)           sp_dyn_array_back(__ARR)
#define sp_da_bounds_ok(arr, it)    sp_dyn_array_bounds_ok(arr, it)
#define sp_da_new(__T)              sp_dyn_array_new(__T)

#define sp_dyn_array_for(__ARR, __IT)  for (u32 __IT = 0; __IT < sp_dyn_array_size((__ARR)); __IT++)
#define sp_dyn_array_rfor(__ARR, __IT) for (u32 __IT = sp_dyn_array_size(__ARR); __IT-- > 0; )

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

#define sp_dyn_array_sort(arr, fn) qsort(arr, sp_dyn_array_size(arr), sizeof((arr)[0]), fn)
#define sp_dyn_array_bounds_ok(arr, it) ((it) < sp_dyn_array_size(arr))


// ██████╗ ██╗███╗   ██╗ ██████╗     ██████╗ ██╗   ██╗███████╗███████╗███████╗██████╗
// ██╔══██╗██║████╗  ██║██╔════╝     ██╔══██╗██║   ██║██╔════╝██╔════╝██╔════╝██╔══██╗
// ██████╔╝██║██╔██╗ ██║██║  ███╗    ██████╔╝██║   ██║█████╗  █████╗  █████╗  ██████╔╝
// ██╔══██╗██║██║╚██╗██║██║   ██║    ██╔══██╗██║   ██║██╔══╝  ██╔══╝  ██╔══╝  ██╔══██╗
// ██║  ██║██║██║ ╚████║╚██████╔╝    ██████╔╝╚██████╔╝██║     ██║     ███████╗██║  ██║
// ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝     ╚═════╝  ╚═════╝ ╚═╝     ╚═╝     ╚══════╝╚═╝  ╚═╝
// @ring_buffer @rb
typedef enum sp_rb_mode {
    SP_RQ_MODE_GROW = 0,
    SP_RQ_MODE_OVERWRITE,
} sp_rb_mode;

typedef struct sp_ring_buffer {
    s32 head;
    s32 size;
    s32 capacity;
    sp_rb_mode mode;
} sp_ring_buffer_t;

#define sp_rb(T) T*
SP_API void* sp_rb_grow_impl(void* arr, u32 elem_size, u32 new_cap);

#define sp_rb_head(__ARR)\
    ((sp_ring_buffer_t*)((u8*)(__ARR) - sizeof(sp_ring_buffer_t)))

#define sp_rb_size(__ARR)\
    ((__ARR) == SP_NULLPTR ? 0 : sp_rb_head((__ARR))->size)

#define sp_rb_capacity(__ARR)\
    ((__ARR) == SP_NULLPTR ? 0 : sp_rb_head((__ARR))->capacity)

#define sp_rb_empty(__ARR)\
    (sp_rb_size(__ARR) == 0)

#define sp_rb_full(__ARR)\
    (sp_rb_size((__ARR)) == sp_rb_capacity((__ARR)))

#define sp_rb_clear(__ARR)\
    do {\
        if (__ARR) {\
            sp_rb_head(__ARR)->head = 0;\
            sp_rb_head(__ARR)->size = 0;\
        }\
    } while (0)

#define sp_rb_free(__ARR)\
    do {\
        if (__ARR) {\
            sp_free(sp_rb_head(__ARR));\
            (__ARR) = SP_NULLPTR;\
        }\
    } while (0)

#define sp_rb_mode(__ARR)\
    ((__ARR) == SP_NULLPTR ? SP_RQ_MODE_GROW : sp_rb_head(__ARR)->mode)

#define sp_rb_set_mode(__ARR, __MODE)\
    do {\
        if ((__ARR) == SP_NULLPTR) {\
            *((void**)&(__ARR)) = sp_rb_grow_impl(SP_NULLPTR, sizeof(*(__ARR)), 8);\
        }\
        sp_rb_head(__ARR)->mode = (__MODE);\
    } while (0)

#define sp_rb_at(__ARR, __IDX)\
    ((__ARR)[(sp_rb_head(__ARR)->head + (__IDX)) % sp_rb_capacity(__ARR)])

#define sp_rb_peek(__ARR)\
    (sp_rb_empty(__ARR) ? SP_NULLPTR : &sp_rb_at(__ARR, 0))

#define sp_rb_back(__ARR)\
    (sp_rb_empty(__ARR) ? SP_NULLPTR : &sp_rb_at(__ARR, sp_rb_size(__ARR) - 1))

#define sp_rb_push(__ARR, __VAL)\
    do {\
        if ((__ARR) == SP_NULLPTR) {\
            *((void**)&(__ARR)) = sp_rb_grow_impl(SP_NULLPTR, sizeof(*(__ARR)), 8);\
        } else if (sp_rb_full(__ARR)) {\
            if (sp_rb_mode(__ARR) == SP_RQ_MODE_OVERWRITE) {\
                sp_rb_head(__ARR)->head = (sp_rb_head(__ARR)->head + 1) % sp_rb_capacity(__ARR);\
                sp_rb_head(__ARR)->size--;\
            } else {\
                *((void**)&(__ARR)) = sp_rb_grow_impl(__ARR, sizeof(*(__ARR)), sp_rb_capacity(__ARR) * 2);\
            }\
        }\
        s32 __sp_rb_tail = (sp_rb_head(__ARR)->head + sp_rb_head(__ARR)->size) % sp_rb_capacity(__ARR);\
        (__ARR)[__sp_rb_tail] = (__VAL);\
        sp_rb_head(__ARR)->size++;\
    } while (0)

#define sp_rb_pop(__ARR)\
    do {\
        if ((__ARR) && !sp_rb_empty(__ARR)) {\
            sp_rb_head(__ARR)->head = (sp_rb_head(__ARR)->head + 1) % sp_rb_capacity(__ARR);\
            sp_rb_head(__ARR)->size--;\
        }\
    } while (0)

#define sp_rb_for(__ARR, __IT)  for (s32 __IT = 0; __IT < sp_rb_size(__ARR); __IT++)
#define sp_rb_rfor(__ARR, __IT) for (s32 __IT = sp_rb_size(__ARR) - 1; __IT >= 0; __IT--)


// ██╗  ██╗ █████╗ ███████╗██╗  ██╗    ████████╗ █████╗ ██████╗ ██╗     ███████╗
// ██║  ██║██╔══██╗██╔════╝██║  ██║    ╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝
// ███████║███████║███████╗███████║       ██║   ███████║██████╔╝██║     █████╗
// ██╔══██║██╔══██║╚════██║██╔══██║       ██║   ██╔══██║██╔══██╗██║     ██╔══╝
// ██║  ██║██║  ██║███████║██║  ██║       ██║   ██║  ██║██████╔╝███████╗███████╗
// ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝       ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝
// @hash_table @ht
#define SP_HT_HASH_SEED         0x31415296
#define SP_HT_INVALID_INDEX     UINT32_MAX

typedef u64 sp_ht_it_t;
SP_TYPEDEF_FN(sp_hash_t, sp_ht_hash_key_fn_t, void*, u32);
SP_TYPEDEF_FN(bool, sp_ht_compare_key_fn_t, void*, void*, u32);

typedef enum sp_ht_entry_state {
  SP_HT_ENTRY_INACTIVE = 0,
  SP_HT_ENTRY_ACTIVE,
  SP_HT_ENTRY_DELETED,
} sp_ht_entry_state;

typedef struct {
  struct {
    sp_ht_hash_key_fn_t hash;
    sp_ht_compare_key_fn_t compare;
  } fn;
  struct {
    u64 key;
    u64 value;
  } size;
  struct {
    u64 entry;
    u64 kv;
    u64 value;
  } stride;
  struct {
    u64 size;
    u64 capacity;
  } header;
  sp_allocator_t allocator;
  u32 tmp_idx;
} sp_ht_info_t;

#if defined(SP_CPP)
SP_END_EXTERN_C()
template<typename T> static T* sp_ht_alloc_type(T* key, size_t size) {
  (void)key;
  return (T*)sp_alloc(size);
}
SP_BEGIN_EXTERN_C()

#else
#define sp_ht_alloc_type(key, size) sp_alloc(size)
#endif



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
        u64 size;                              \
        u64 capacity;                          \
        sp_ht_info_t info;                     \
    }*

#define sp_ht_new(__K, __V) SP_NULLPTR

#define sp_ht_set_fns(ht, hash_fn, cmp_fn) \
  sp_ht_ensure(ht);                        \
  (ht)->info.fn.hash = (hash_fn);          \
  (ht)->info.fn.compare = (cmp_fn)

#define sp_ht_size(ht) \
  ((ht) ? (ht)->size : 0)

#define sp_ht_capacity(ht) \
  ((ht) ? (ht)->capacity : 0)

#define sp_ht_empty(ht) \
  ((ht) ? (ht)->size == 0 : true)

#define sp_ht_clear(ht)                                    \
  do {                                                     \
    if ((ht)) {                                            \
      for (u32 i = 0; i < (ht)->capacity; ++i) {           \
        (ht)->data[i].state = SP_HT_ENTRY_INACTIVE;        \
      }                                                    \
      (ht)->size = 0;                                      \
    }                                                      \
  } while (0)

#define sp_ht_free(ht)        \
  do {                        \
    if ((ht)) {               \
      sp_free((ht)->data);    \
      (ht)->data = SP_NULLPTR;\
      sp_free(ht);            \
      (ht) = SP_NULLPTR;      \
    }                         \
  } while (0)

#define sp_ht_ensure(ht) \
  if (!(ht)) { sp_ht_init(ht); }

#define sp_ht_data_u8_n(ht, n) ((u8*)(&((ht)->data[n])))
#define sp_ht_data_u8(ht) sp_ht_data_u8_n(ht, 0)
#define sp_ht_data_offset_u8(ht, field) (((u8*)(&((ht)->data[0].field))) - sp_ht_data_u8(ht))
#define sp_ht_field_as_u8(ht, field) ((u8*)&((ht)->field))
#define sp_ht_as_u8(ht) ((u8*)(ht))
#define sp_ht_field_offset_u8(ht, field) (sp_ht_field_as_u8(ht, field) - sp_ht_as_u8(ht))

#define sp_ht_init(ht)                                                                    \
  do {                                                                                    \
    (ht)                       = sp_ht_alloc_type(ht, sizeof(*(ht)));                     \
    (ht)->data                 = sp_ht_alloc_type((ht)->data, 2 * sizeof((ht)->data[0])); \
    (ht)->info.allocator       = sp_context_get()->allocator;                             \
    (ht)->size                 = 0;                                                       \
    (ht)->capacity             = 2;                                                       \
    (ht)->info.size.key        = sizeof((ht)->data[0].key);                               \
    (ht)->info.size.value      = sizeof((ht)->data[0].val);                               \
    (ht)->info.stride.entry    = sp_ht_data_u8_n(ht, 1) - sp_ht_data_u8_n(ht, 0);         \
    (ht)->info.stride.value    = sp_ht_data_offset_u8(ht, val);                           \
    (ht)->info.stride.kv       = sp_ht_data_offset_u8(ht, state);                         \
    (ht)->info.header.size     = sp_ht_field_offset_u8(ht, size);                         \
    (ht)->info.header.capacity = sp_ht_field_offset_u8(ht, capacity);                     \
    (ht)->info.fn.hash         = sp_ht_on_hash_key;                                       \
    (ht)->info.fn.compare      = sp_ht_on_compare_key;                                    \
  } while (0)

#define sp_ht_insert_ex(ht, k, v)                                      \
  do {                                                                 \
    (ht)->tmp_key = (k);                                               \
    (ht)->tmp_val = (v);                                               \
    sp_ht_insert_impl(ht, &(ht)->tmp_key, &(ht)->tmp_val, (ht)->info); \
  } while (0)

#define sp_ht_insert(ht, k, v)  \
  do {                          \
    sp_ht_ensure(ht);           \
    sp_ht_insert_ex(ht, (k), v);  \
  } while (0)

#define sp_ht_get_key_n(ht, n) \
  (&(ht)->data[(n)].key)

#define sp_ht_get_n(ht, n) \
  (&(ht)->data[(n)].val)

#define sp_ht_get_tmp_n(ht) \
  sp_ht_get_n(ht, (ht)->info.tmp_idx)

#define sp_ht_getp(ht, key) \
  (!(ht) ? SP_NULLPTR : (   \
    (ht)->tmp_key = (key),  \
    (ht)->info.tmp_idx = sp_ht_tmp_key_index(ht), \
    (ht)->info.tmp_idx == SP_HT_INVALID_INDEX ? SP_NULLPTR : sp_ht_get_tmp_n(ht)))

#define sp_ht_key_exists(ht, key) \
  ((ht) && ((ht)->tmp_key = (key), sp_ht_tmp_key_index(ht) != SP_HT_INVALID_INDEX))

#define sp_ht_exists(ht, key) \
  sp_ht_key_exists((ht), (key))

#define sp_ht_erase(ht, k)                          \
  do {                                              \
    if ((ht)) {                                     \
      (ht)->tmp_key = (k);                          \
      u32 idx = sp_ht_tmp_key_index(ht);            \
      if (idx != SP_HT_INVALID_INDEX) {             \
        (ht)->data[idx].state = SP_HT_ENTRY_DELETED;\
        if ((ht)->size) (ht)->size--;               \
      }                                             \
    }                                               \
  } while (0)

#define sp_ht_it_valid(ht, it) \
  ((ht) && (it) < sp_ht_capacity(ht) && (ht)->data[(it)].state == SP_HT_ENTRY_ACTIVE)

#define sp_ht_it_advance(ht, it) \
  ((ht) ? sp_ht_it_advance_fn((void**)&(ht)->data, (ht)->capacity, &(it), (ht)->info) : (void)0)

#define sp_ht_it_getp(ht, it) \
  (!(ht) ? SP_NULLPTR : sp_ht_get_n(ht, it))

#define sp_ht_it_getkp(ht, it) \
  (!(ht) ? SP_NULLPTR : sp_ht_get_key_n(ht, it))

#define sp_ht_it_init(ht) \
  (!(ht) ? 0 : sp_ht_it_init_fn((void**)&(ht)->data, (ht)->capacity, (ht)->info))

#define sp_ht_for(ht, it) \
  for (sp_ht_it_t it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it))

#define sp_ht_key_t(ht)   __typeof__((ht)->data[0].key)*
#define sp_ht_value_t(ht) __typeof__((ht)->data[0].val)*

#define sp_ht_for_kv(ht, it)                                                       \
  for (                                                                            \
    struct { sp_ht_it_t idx; sp_ht_key_t(ht) key; sp_ht_value_t(ht) val; } it = {  \
      sp_ht_it_init(ht),                                                           \
      sp_ht_it_getkp(ht, sp_ht_it_init(ht)),                                       \
      sp_ht_it_getp(ht, sp_ht_it_init(ht))                                         \
    };                                                                             \
    sp_ht_it_valid(ht, it.idx);                                                    \
    (sp_ht_it_advance(ht, it.idx),                                                 \
     it.key = sp_ht_it_getkp(ht, it.idx),                                          \
     it.val = sp_ht_it_getp(ht, it.idx))                                           \
  )

#define sp_ht_front(ht) \
  (!(ht) || !sp_ht_it_valid(ht, sp_ht_it_init(ht)) ? SP_NULLPTR : sp_ht_it_getp(ht, sp_ht_it_init(ht)))

#define sp_ht_tmp_key_index(ht) \
  sp_ht_get_key_index_fn((void**)&(ht)->data, (void*)&(ht)->tmp_key, (ht)->capacity, (ht)->info)


// STR_HT
#define sp_str_ht(t) sp_ht(sp_str_t, t)

#define sp_str_ht_ensure(ht) \
  if (!(ht)) sp_str_ht_init(ht)

#define sp_str_ht_init(ht)                       \
  sp_ht_init(ht);                                \
  (ht)->info.fn.hash = sp_ht_on_hash_str_key;    \
  (ht)->info.fn.compare = sp_ht_on_compare_str_key

#define sp_str_ht_insert(ht, key, value)  \
  do {                                    \
    sp_str_ht_ensure(ht);                 \
    sp_ht_insert_ex(ht, key, value);      \
  } while (0)

#define sp_str_ht_get(ht, key)      sp_ht_getp(ht, key)
#define sp_str_ht_exists(ht, key)   sp_ht_exists(ht, key)
#define sp_str_ht_erase(ht, key)    sp_ht_erase(ht, key)
#define sp_str_ht_size(ht)          sp_ht_size(ht)
#define sp_str_ht_capacity(ht)      sp_ht_capacity(ht)
#define sp_str_ht_empty(ht)         sp_ht_empty(ht)
#define sp_str_ht_clear(ht)         sp_ht_clear(ht)
#define sp_str_ht_free(ht)          sp_ht_free(ht)
#define sp_str_ht_front(ht)         sp_ht_front(ht)
#define sp_str_ht_for(ht, it)       sp_ht_for(ht, it)
#define sp_str_ht_for_kv(ht, it)    sp_ht_for_kv(ht, it)
#define sp_str_ht_it_init(ht)       sp_ht_it_init(ht)
#define sp_str_ht_it_valid(ht, it)  sp_ht_it_valid(ht, it)
#define sp_str_ht_it_advance(ht, it) sp_ht_it_advance(ht, it)
#define sp_str_ht_it_getp(ht, it)   sp_ht_it_getp(ht, it)
#define sp_str_ht_it_getkp(ht, it)  sp_ht_it_getkp(ht, it)

// CSTR_HT
#define sp_cstr_ht(t) sp_ht(const c8*, t)

#define sp_cstr_ht_ensure(ht) \
  if (!(ht)) sp_cstr_ht_init(ht)

#define sp_cstr_ht_init(ht)                       \
  sp_ht_init(ht);                                 \
  (ht)->info.fn.hash = sp_ht_on_hash_cstr_key;    \
  (ht)->info.fn.compare = sp_ht_on_compare_cstr_key

#define sp_cstr_ht_insert(ht, key, value)  \
  do {                                     \
    sp_cstr_ht_ensure(ht);                 \
    sp_ht_insert_ex(ht, key, value);       \
  } while (0)

#define sp_cstr_ht_get(ht, key)       sp_ht_getp(ht, key)
#define sp_cstr_ht_exists(ht, key)    sp_ht_exists(ht, key)
#define sp_cstr_ht_erase(ht, key)     sp_ht_erase(ht, key)
#define sp_cstr_ht_size(ht)           sp_ht_size(ht)
#define sp_cstr_ht_capacity(ht)       sp_ht_capacity(ht)
#define sp_cstr_ht_empty(ht)          sp_ht_empty(ht)
#define sp_cstr_ht_clear(ht)          sp_ht_clear(ht)
#define sp_cstr_ht_free(ht)           sp_ht_free(ht)
#define sp_cstr_ht_front(ht)          sp_ht_front(ht)
#define sp_cstr_ht_for(ht, it)        sp_ht_for(ht, it)
#define sp_cstr_ht_for_kv(ht, it)     sp_ht_for_kv(ht, it)
#define sp_cstr_ht_it_init(ht)        sp_ht_it_init(ht)
#define sp_cstr_ht_it_valid(ht, it)   sp_ht_it_valid(ht, it)
#define sp_cstr_ht_it_advance(ht, it) sp_ht_it_advance(ht, it)
#define sp_cstr_ht_it_getp(ht, it)    sp_ht_it_getp(ht, it)
#define sp_cstr_ht_it_getkp(ht, it)   sp_ht_it_getkp(ht, it)

SP_API u64         sp_ht_get_key_index_fn(void** data, void* key, u64 capacity, sp_ht_info_t info);
SP_API void        sp_ht_resize_impl(void** data, u64 old_cap, u64 new_cap, sp_ht_info_t info);
SP_API void        sp_ht_insert_impl(void* ht, void* key, void* val, sp_ht_info_t info);
SP_API sp_ht_it_t  sp_ht_it_init_fn(void** data, u64 capacity, sp_ht_info_t info);
SP_API void        sp_ht_it_advance_fn(void** data, u64 capacity, u64* it, sp_ht_info_t info);
SP_API sp_hash_t   sp_ht_on_hash_key(void* key, u32 size);
SP_API bool        sp_ht_on_compare_key(void* ka, void* kb, u32 size);
SP_API sp_hash_t   sp_ht_on_hash_str_key(void* key, u32 size);
SP_API bool        sp_ht_on_compare_str_key(void* ka, void* kb, u32 size);
SP_API sp_hash_t   sp_ht_on_hash_cstr_key(void* key, u32 size);
SP_API bool        sp_ht_on_compare_cstr_key(void* ka, void* kb, u32 size);

// ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗
// ██╔════╝╚══██╔══╝██╔═██╗██║████╗  ██║██╔════╝
// ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗
// ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║
// ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
// @string
typedef struct {
  const c8* data;
  u32 len;
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

typedef struct {
  sp_str_t str;
  u32 index;
  c8 c;
} sp_str_it_t;

#define SP_UTF8_REPLACEMENT   0xFFFD
#define SP_UTF8_SURROGATE_MIN 0xD800
#define SP_UTF8_SURROGATE_MAX 0xDFFF
#define SP_UTF8_2_BYTE_MIN    0x80
#define SP_UTF8_3_BYTE_MIN    0x800
#define SP_UTF8_4_BYTE_MIN    0x10000
#define SP_UTF8_MAX           0x10FFFF
#define SP_UTF8_ASCII_MAX     0x80

typedef struct {
  struct { const u8* data; u32 len; } str;
  s32 index;
  u32 codepoint;
  u8 codepoint_len;
} sp_utf8_it_t;

SP_TYPEDEF_FN(sp_str_t, sp_str_map_fn_t, sp_str_map_context_t* context);
SP_TYPEDEF_FN(void, sp_str_reduce_fn_t, sp_str_reduce_context_t* context);

#define sp_str(STR, LEN) sp_rval(sp_str_t) { .data = (const c8*)(STR), .len = (u32)(LEN) }
#define SP_STR(STR, LEN) sp_str(STR, LEN)
#define sp_str_lit(STR)  sp_str((STR), sizeof(STR) - 1)
#define SP_LIT(STR)      sp_str_lit(STR)
#define sp_lit(STR)      SP_LIT(STR)
#define SP_CSTR(STR)     sp_str((STR), sp_cstr_len(STR))
#define sp_cstr(STR)     SP_CSTR(STR)

#define sp_str_for(str, it) for (u32 it = 0; it < str.len; it++)
#define sp_str_for_it(str, it) for (sp_str_it_t it = sp_str_it(str); sp_str_it_valid(&it); sp_str_it_next(&it))
#define sp_str_for_utf8(str, it) for (sp_utf8_it_t it = sp_utf8_it(str); sp_utf8_it_valid(&it); sp_utf8_it_next(&it))
#define sp_str_rfor_utf8(str, it) for (sp_utf8_it_t it = sp_utf8_rit(str); sp_utf8_it_valid(&it); sp_utf8_it_prev(&it))

sp_str_it_t            sp_str_it(sp_str_t str);
bool                   sp_str_it_valid(sp_str_it_t* it);
void                   sp_str_it_next(sp_str_it_t* it);
SP_API u32             sp_utf8_decode(const u8* ptr);
SP_API u8              sp_utf8_encode(u32 codepoint, u8* out);
SP_API u8              sp_utf8_num_bytes_from_codepoint(u32 codepoint);
SP_API u8              sp_utf8_num_bytes_from_ptr(const u8* ptr);
SP_API u8              sp_utf8_num_bytes_from_byte(u8 byte);
SP_API sp_utf8_it_t    sp_utf8_it(sp_str_t str);
SP_API sp_utf8_it_t    sp_utf8_rit(sp_str_t str);
SP_API bool            sp_utf8_it_valid(sp_utf8_it_t* it);
SP_API void            sp_utf8_it_next(sp_utf8_it_t* it);
SP_API void            sp_utf8_it_prev(sp_utf8_it_t* it);
SP_API bool            sp_utf8_validate(sp_str_t str);
SP_API bool            sp_utf8_is_byte_ascii(u8 b);
SP_API bool            sp_utf8_is_codepoint_ascii(u32 codepoint);
SP_API u32             sp_utf8_to_upper(u32 codepoint);
SP_API u32             sp_utf8_to_lower(u32 codepoint);
SP_API u32             sp_utf8_num_codepoints(sp_str_t str);
SP_API c8              sp_c8_to_upper(c8 c);
SP_API c8              sp_c8_to_lower(c8 c);
SP_API c8*             sp_cstr_copy(const c8* str);
SP_API void            sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length);
SP_API c8*             sp_cstr_copy_sized(const c8* str, u32 length);
SP_API void            sp_cstr_copy_to_sized(const c8* str, u32 n, c8* buffer, u32 bn);
SP_API bool            sp_cstr_equal(const c8* a, const c8* b);
SP_API u32             sp_cstr_len(const c8* str);
SP_API u32             sp_cstr_len_sized(const c8* str, u32 n);
SP_API c8*             sp_wstr_to_cstr(c16* str, u32 len);
SP_API c8*             sp_str_to_cstr(sp_str_t str);
SP_API c8*             sp_str_to_cstr_double_nt(sp_str_t str);
SP_API sp_str_t        sp_str_copy(sp_str_t str);
SP_API void            sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity);
SP_API sp_str_t        sp_str_null_terminate(sp_str_t str);
SP_API sp_str_t        sp_str_from_cstr(const c8* str);
SP_API sp_str_t        sp_str_from_cstr_sized(const c8* str, u32 length);
SP_API sp_str_t        sp_str_from_cstr_null(const c8* str);
SP_API sp_str_t        sp_str_alloc(u32 capacity);
SP_API sp_str_t        sp_str_view(const c8* cstr);
SP_API bool            sp_str_empty(sp_str_t);
SP_API bool            sp_str_equal(sp_str_t a, sp_str_t b);
SP_API bool            sp_str_equal_cstr(sp_str_t a, const c8* b);
SP_API bool            sp_str_starts_with(sp_str_t str, sp_str_t prefix);
SP_API bool            sp_str_ends_with(sp_str_t str, sp_str_t suffix);
SP_API bool            sp_str_contains(sp_str_t str, sp_str_t needle);
SP_API bool            sp_str_valid(sp_str_t str);
SP_API c8              sp_str_at(sp_str_t str, s32 index);
SP_API c8              sp_str_at_reverse(sp_str_t str, s32 index);
SP_API c8              sp_str_back(sp_str_t str);
SP_API s32             sp_str_compare_alphabetical(sp_str_t a, sp_str_t b);
SP_API sp_str_t        sp_str_prefix(sp_str_t str, s32 len);
SP_API sp_str_t        sp_str_suffix(sp_str_t str, s32 len);
SP_API sp_str_t        sp_str_sub(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t        sp_str_sub_reverse(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t        sp_str_concat(sp_str_t a, sp_str_t b);
SP_API sp_str_t        sp_str_replace_c8(sp_str_t str, c8 from, c8 to);
SP_API sp_str_t        sp_str_pad(sp_str_t str, u32 n);
SP_API sp_str_t        sp_str_trim_left(sp_str_t str);
SP_API sp_str_t        sp_str_trim_right(sp_str_t str);
SP_API sp_str_t        sp_str_trim(sp_str_t str);
SP_API sp_str_t        sp_str_strip_left(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_strip_right(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_strip(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_truncate(sp_str_t str, u32 n, sp_str_t trailer);
SP_API sp_str_t        sp_str_join(sp_str_t a, sp_str_t b, sp_str_t join);
SP_API sp_str_t        sp_str_join_cstr_n(const c8** strings, u32 num_strings, sp_str_t join);
SP_API sp_str_t        sp_str_to_upper(sp_str_t str);
SP_API sp_str_t        sp_str_to_lower(sp_str_t str);
SP_API sp_str_t        sp_str_to_pascal_case(sp_str_t str);
SP_API sp_str_pair_t   sp_str_cleave_c8(sp_str_t str, c8 delimiter);
SP_API sp_da(sp_str_t) sp_str_split_c8(sp_str_t, c8 c);
SP_API bool            sp_str_contains_n(sp_str_t* strs, u32 n, sp_str_t needle);
SP_API sp_str_t        sp_str_join_n(sp_str_t* strs, u32 n, sp_str_t joiner);
SP_API u32             sp_str_count_n(sp_str_t* strs, u32 n, sp_str_t needle);
SP_API sp_str_t        sp_str_find_longest_n(sp_str_t* strs, u32 n);
SP_API sp_str_t        sp_str_find_shortest_n(sp_str_t* strs, u32 n);
SP_API sp_da(sp_str_t) sp_str_pad_to_longest(sp_str_t* strs, u32 n);
SP_API sp_str_t        sp_str_reduce(sp_str_t* strs, u32 n, void* ud, sp_str_reduce_fn_t fn);
SP_API void            sp_str_reduce_kernel_join(sp_str_reduce_context_t* context);
SP_API void            sp_str_reduce_kernel_contains(sp_str_reduce_context_t* context);
SP_API void            sp_str_reduce_kernel_count(sp_str_reduce_context_t* context);
SP_API void            sp_str_reduce_kernel_longest(sp_str_reduce_context_t* context);
SP_API void            sp_str_reduce_kernel_shortest(sp_str_reduce_context_t* context);
SP_API sp_da(sp_str_t) sp_str_map(sp_str_t* s, u32 n, sp_opaque_ptr ud, sp_str_map_fn_t fn);
SP_API sp_str_t        sp_str_map_kernel_prepend(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_append(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_prefix(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_trim(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_pad(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_to_upper(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_to_lower(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_pascal_case(sp_str_map_context_t* context);
SP_API s32             sp_str_sort_kernel_alphabetical(const void* a, const void* b);
SP_API void            sp_str_builder_append_utf8(sp_str_builder_t* builder, u32 codepoint);
SP_API void            sp_str_builder_grow(sp_str_builder_t* builder, u32 requested_capacity);
SP_API void            sp_str_builder_add_capacity(sp_str_builder_t* builder, u32 amount);
SP_API void            sp_str_builder_indent(sp_str_builder_t* builder);
SP_API void            sp_str_builder_dedent(sp_str_builder_t* builder);
SP_API void            sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str);
SP_API void            sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str);
SP_API void            sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c);
SP_API void            sp_str_builder_append_fmt_str(sp_str_builder_t* builder, sp_str_t fmt, ...);
SP_API void            sp_str_builder_append_fmt(sp_str_builder_t* builder, const c8* fmt, ...);
SP_API void            sp_str_builder_new_line(sp_str_builder_t* builder);
SP_API sp_str_t        sp_str_builder_move(sp_str_builder_t* builder);
SP_API sp_str_t        sp_str_builder_write(sp_str_builder_t* builder);
SP_API c8*             sp_str_builder_write_cstr(sp_str_builder_t* builder);


// ██╗      ██████╗  ██████╗
// ██║     ██╔═══██╗██╔════╝
// ██║     ██║   ██║██║  ███╗
// ██║     ██║   ██║██║   ██║
// ███████╗╚██████╔╝╚██████╔╝
// ╚══════╝ ╚═════╝  ╚═════╝
// @log
#define SP_LOG(CSTR, ...)    sp_log(SP_CSTR((CSTR)), ##__VA_ARGS__)
#define SP_LOG_STR(STR, ...) sp_log((STR),           ##__VA_ARGS__)
#define sp_log_str(STR, ...) SP_LOG_STR(STR, ##__VA_ARGS__)
SP_API void sp_log(sp_str_t fmt, ...);


// ███████╗███╗   ██╗██╗   ██╗
// ██╔════╝████╗  ██║██║   ██║
// █████╗  ██╔██╗ ██║██║   ██║
// ██╔══╝  ██║╚██╗██║╚██╗ ██╔╝
// ███████╗██║ ╚████║ ╚████╔╝
// ╚══════╝╚═╝  ╚═══╝  ╚═══╝
// @env
typedef struct {
  sp_str_t key;
  sp_str_t value;
} sp_env_var_t;

typedef sp_ht(sp_str_t, sp_str_t) sp_env_table_t;

typedef struct {
  sp_env_table_t vars;
} sp_env_t;

SP_API void     sp_env_init(sp_env_t* env);
SP_API sp_env_t sp_env_capture();
SP_API sp_env_t sp_env_copy(sp_env_t* env);
SP_API sp_str_t sp_env_get(sp_env_t* env, sp_str_t name);
SP_API void     sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value);
SP_API void     sp_env_erase(sp_env_t* env, sp_str_t name);
SP_API void     sp_env_destroy(sp_env_t* env);


//  ██████╗ ███████╗
// ██╔═══██╗██╔════╝
// ██║   ██║███████╗
// ██║   ██║╚════██║
// ╚██████╔╝███████║
//  ╚═════╝ ╚══════╝
// @os
typedef enum {
  SP_OS_LINUX,
  SP_OS_WIN32,
  SP_OS_MACOS,
} sp_os_kind_t;

typedef enum {
  SP_OS_LIB_SHARED,
  SP_OS_LIB_STATIC,
} sp_os_lib_kind_t;

typedef enum {
  SP_FS_LINK_HARD,
  SP_FS_LINK_SYMBOLIC,
  SP_FS_LINK_COPY,
} sp_os_link_kind_t;

typedef enum {
  SP_ENV_EXPORT_OVERWRITE_DUPES,
  SP_ENV_EXPORT_SKIP_DUPES,
} sp_env_export_t;

SP_API sp_os_kind_t sp_os_get_kind();
SP_API sp_str_t     sp_os_get_name();
SP_API void         sp_os_sleep_ms(f64 ms);
SP_API void         sp_os_sleep_ns(u64 ns);
SP_API void         sp_sleep_ns(u64 ns);
SP_API void         sp_sleep_ms(f64 ms);
SP_API c8*          sp_os_wstr_to_cstr(c16* str, u32 len);
SP_API void         sp_os_print(sp_str_t message);
SP_API sp_str_t     sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind);
SP_API sp_str_t     sp_os_lib_to_file_name(sp_str_t lib, sp_os_lib_kind_t kind);
SP_API sp_str_t     sp_os_get_env_var(sp_str_t key);
SP_API sp_str_t     sp_os_get_env_as_path(sp_str_t key);
SP_API void         sp_os_clear_env_var(sp_str_t var);
SP_API void         sp_os_export_env_var(sp_str_t k, sp_str_t v, sp_env_export_t mode);
SP_API void         sp_os_export_env(sp_env_t* env, sp_env_export_t mode);


// ████████╗██╗███╗   ███╗███████╗
// ╚══██╔══╝██║████╗ ████║██╔════╝
//    ██║   ██║██╔████╔██║█████╗
//    ██║   ██║██║╚██╔╝██║██╔══╝
//    ██║   ██║██║ ╚═╝ ██║███████╗
//    ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝
// @time
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

typedef struct {
  s32 year;
  s32 month;
  s32 day;
  s32 hour;
  s32 minute;
  s32 second;
  s32 millisecond;
} sp_os_date_time_t;

SP_API sp_tm_epoch_t     sp_tm_now_epoch();
SP_API sp_str_t          sp_tm_epoch_to_iso8601(sp_tm_epoch_t time);
SP_API sp_tm_point_t     sp_tm_now_point();
SP_API u64               sp_tm_point_diff(sp_tm_point_t newer, sp_tm_point_t older);
SP_API sp_tm_timer_t     sp_tm_start_timer();
SP_API u64               sp_tm_read_timer(sp_tm_timer_t* timer);
SP_API u64               sp_tm_lap_timer(sp_tm_timer_t* timer);
SP_API void              sp_tm_reset_timer(sp_tm_timer_t* timer);
SP_API sp_tm_date_time_t sp_tm_get_date_time();
SP_API u64               sp_tm_fps_to_ns(u64 fps);
SP_API u64               sp_tm_s_to_ms(u64 s);
SP_API u64               sp_tm_s_to_us(u64 s);
SP_API u64               sp_tm_s_to_ns(u64 s);
SP_API u64               sp_tm_ms_to_s(u64 ms);
SP_API u64               sp_tm_ms_to_us(u64 ms);
SP_API u64               sp_tm_ms_to_ns(u64 ms);
SP_API u64               sp_tm_us_to_s(u64 us);
SP_API u64               sp_tm_us_to_ms(u64 us);
SP_API u64               sp_tm_us_to_ns(u64 us);
SP_API u64               sp_tm_ns_to_s(u64 ns);
SP_API u64               sp_tm_ns_to_ms(u64 ns);
SP_API u64               sp_tm_ns_to_us(u64 ns);
SP_API f64               sp_tm_s_to_ms_f(f64 s);
SP_API f64               sp_tm_s_to_us_f(f64 s);
SP_API f64               sp_tm_s_to_ns_f(f64 s);
SP_API f64               sp_tm_ms_to_s_f(f64 ms);
SP_API f64               sp_tm_ms_to_us_f(f64 ms);
SP_API f64               sp_tm_ms_to_ns_f(f64 ms);
SP_API f64               sp_tm_us_to_s_f(f64 us);
SP_API f64               sp_tm_us_to_ms_f(f64 us);
SP_API f64               sp_tm_us_to_ns_f(f64 us);
SP_API f64               sp_tm_ns_to_s_f(f64 ns);
SP_API f64               sp_tm_ns_to_ms_f(f64 ns);
SP_API f64               sp_tm_ns_to_us_f(f64 ns);


// ███████╗███████╗
// ██╔════╝██╔════╝
// █████╗  ███████╗
// ██╔══╝  ╚════██║
// ██║     ███████║
// ╚═╝     ╚══════╝
// @fs
typedef enum {
  SP_OS_FILE_ATTR_NONE = 0,
  SP_OS_FILE_ATTR_REGULAR_FILE = (1 << 0),
  SP_OS_FILE_ATTR_DIRECTORY    = (1 << 1),
  SP_OS_FILE_ATTR_SYMLINK      = (1 << 2),
} sp_os_file_attr_t;

typedef struct {
  sp_str_t file_path;
  sp_str_t file_name;
  sp_os_file_attr_t attributes;
} sp_os_dir_ent_t;

typedef s32 sp_os_file_handle_t;

SP_API bool                   sp_fs_is_regular_file(sp_str_t path);
SP_API bool                   sp_fs_is_symlink(sp_str_t path);
SP_API bool                   sp_fs_is_dir(sp_str_t path);
SP_API bool                   sp_fs_is_target_regular_file(sp_str_t path);
SP_API bool                   sp_fs_is_target_dir(sp_str_t path);
SP_API bool                   sp_fs_is_root(sp_str_t path);
SP_API bool                   sp_fs_is_glob(sp_str_t path);
SP_API bool                   sp_fs_is_on_path(sp_str_t program);
SP_API bool                   sp_fs_exists(sp_str_t path);
SP_API void                   sp_fs_create_dir(sp_str_t path);
SP_API void                   sp_fs_remove_dir(sp_str_t path);
SP_API void                   sp_fs_create_file(sp_str_t path);
SP_API void                   sp_fs_remove_file(sp_str_t path);
SP_API sp_err_t               sp_fs_copy(sp_str_t from, sp_str_t to);
SP_API void                   sp_fs_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to);
SP_API void                   sp_fs_copy_file(sp_str_t from, sp_str_t to);
SP_API void                   sp_fs_copy_dir(sp_str_t from, sp_str_t to);
SP_API sp_err_t               sp_fs_link(sp_str_t from, sp_str_t to, sp_os_link_kind_t kind);
SP_API sp_err_t               sp_fs_create_hard_link(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t               sp_fs_create_sym_link(sp_str_t target, sp_str_t link_path);
SP_API sp_da(sp_os_dir_ent_t) sp_fs_collect(sp_str_t path);
SP_API sp_da(sp_os_dir_ent_t) sp_fs_collect_recursive(sp_str_t path);
SP_API sp_str_t               sp_fs_normalize_path(sp_str_t path);
SP_API void                   sp_fs_normalize_path_soft(sp_str_t* path);
SP_API sp_str_t               sp_fs_canonicalize_path(sp_str_t path);
SP_API sp_str_t               sp_fs_parent_path(sp_str_t path);
SP_API sp_str_t               sp_fs_join_path(sp_str_t a, sp_str_t b);
SP_API sp_str_t               sp_fs_get_ext(sp_str_t path);
SP_API sp_str_t               sp_fs_get_stem(sp_str_t path);
SP_API sp_str_t               sp_fs_get_name(sp_str_t path);
SP_API sp_str_t               sp_fs_get_cwd();
SP_API sp_str_t               sp_fs_get_exe_path();
SP_API sp_str_t               sp_fs_get_storage_path();
SP_API sp_str_t               sp_fs_get_config_path();
SP_API sp_tm_epoch_t          sp_fs_get_mod_time(sp_str_t path);
SP_API sp_os_file_attr_t      sp_fs_get_file_attrs(sp_str_t path);




//  █████╗ ████████╗ ██████╗ ███╗   ███╗██╗██████╗
// ██╔══██╗╚══██╔══╝██╔═══██╗████╗ ████║██║██╔════╝
// ███████║   ██║   ██║   ██║██╔████╔██║██║██║
// ██╔══██║   ██║   ██║   ██║██║╚██╔╝██║██║██║
// ██║  ██║   ██║   ╚██████╔╝██║ ╚═╝ ██║██║╚██████╗
// ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝╚═╝ ╚═════╝
// @atomic
typedef s32 sp_atomic_s32;

SP_API bool sp_atomic_s32_cmp_and_swap(sp_atomic_s32* value, s32 current, s32 desired);
SP_API s32  sp_atomic_s32_set(sp_atomic_s32* value, s32 desired);
SP_API s32  sp_atomic_s32_add(sp_atomic_s32* value, s32 add);
SP_API s32  sp_atomic_s32_get(sp_atomic_s32* value);


// ███╗   ███╗██╗   ██╗████████╗███████╗██╗  ██╗
// ████╗ ████║██║   ██║╚══██╔══╝██╔════╝╚██╗██╔╝
// ██╔████╔██║██║   ██║   ██║   █████╗   ╚███╔╝
// ██║╚██╔╝██║██║   ██║   ██║   ██╔══╝   ██╔██╗
// ██║ ╚═╝ ██║╚██████╔╝   ██║   ███████╗██╔╝ ██╗
// ╚═╝     ╚═╝ ╚═════╝    ╚═╝   ╚══════╝╚═╝  ╚═╝
// @mutex
typedef enum {
  SP_MUTEX_NONE = 0,
  SP_MUTEX_PLAIN = 1,
  SP_MUTEX_TIMED = 2,
  SP_MUTEX_RECURSIVE = 4
} sp_mutex_kind_t;

#if defined(SP_WIN32)
  typedef mtx_t sp_mutex_t;

#elif defined(SP_POSIX)
  typedef pthread_mutex_t sp_mutex_t;
#endif

SP_API void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind);
SP_API void sp_mutex_lock(sp_mutex_t* mutex);
SP_API void sp_mutex_unlock(sp_mutex_t* mutex);
SP_API void sp_mutex_destroy(sp_mutex_t* mutex);
SP_API s32  sp_mutex_kind_to_c11(sp_mutex_kind_t kind);


#if defined(SP_POSIX)
typedef pthread_cond_t sp_cv_t;
#elif defined(SP_WIN32)
typedef CONDITION_VARIABLE sp_cv_t;
#endif

SP_API void sp_cv_init(sp_cv_t* cv);
SP_API void sp_cv_destroy(sp_cv_t* cv);
SP_API void sp_cv_wait(sp_cv_t* cv, sp_mutex_t* mutex);
SP_API bool sp_cv_wait_for(sp_cv_t* cv, sp_mutex_t* mutex, u32 ms);
SP_API void sp_cv_notify_one(sp_cv_t* cv);
SP_API void sp_cv_notify_all(sp_cv_t* cv);


// ███████╗███████╗███╗   ███╗ █████╗ ██████╗ ██╗  ██╗ ██████╗ ██████╗ ███████╗
// ██╔════╝██╔════╝████╗ ████║██╔══██╗██╔══██╗██║  ██║██╔═══██╗██╔══██╗██╔════╝
// ███████╗█████╗  ██╔████╔██║███████║██████╔╝███████║██║   ██║██████╔╝█████╗
// ╚════██║██╔══╝  ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔══██║██║   ██║██╔══██╗██╔══╝
// ███████║███████╗██║ ╚═╝ ██║██║  ██║██║     ██║  ██║╚██████╔╝██║  ██║███████╗
// ╚══════╝╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
// @semaphore
#if defined(SP_WIN32)
  typedef HANDLE sp_semaphore_t;

#elif defined(SP_MACOS)
  typedef dispatch_semaphore_t sp_semaphore_t;

#elif defined(SP_POSIX)
  typedef sem_t sp_semaphore_t;
#endif

SP_API void sp_semaphore_init(sp_semaphore_t* semaphore);
SP_API void sp_semaphore_destroy(sp_semaphore_t* semaphore);
SP_API void sp_semaphore_wait(sp_semaphore_t* semaphore);
SP_API bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms);
SP_API void sp_semaphore_signal(sp_semaphore_t* semaphore);


// ███████╗██╗   ██╗████████╗██╗   ██╗██████╗ ███████╗
// ██╔════╝██║   ██║╚══██╔══╝██║   ██║██╔══██╗██╔════╝
// █████╗  ██║   ██║   ██║   ██║   ██║██████╔╝█████╗
// ██╔══╝  ██║   ██║   ██║   ██║   ██║██╔══██╗██╔══╝
// ██║     ╚██████╔╝   ██║   ╚██████╔╝██║  ██║███████╗
// ╚═╝      ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝
// @future
typedef struct {
  sp_allocator_t allocator;
  void* value;
  sp_atomic_s32 ready;
  u32 size;
} sp_future_t;

SP_API sp_future_t* sp_future_create(u32 size);
SP_API void         sp_future_set_value(sp_future_t* future, void* data);
SP_API void         sp_future_destroy(sp_future_t* future);


// ███████╗██████╗ ██╗███╗   ██╗
// ██╔════╝██╔══██╗██║████╗  ██║
// ███████╗██████╔╝██║██╔██╗ ██║
// ╚════██║██╔═══╝ ██║██║╚██╗██║
// ███████║██║     ██║██║ ╚████║
// ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝
// @spin
typedef s32 sp_spin_lock_t;

SP_API void sp_spin_pause();
SP_API bool sp_spin_try_lock(sp_spin_lock_t* lock);
SP_API void sp_spin_lock(sp_spin_lock_t* lock);
SP_API void sp_spin_unlock(sp_spin_lock_t* lock);


// ████████╗██╗  ██╗██████╗ ███████╗ █████╗ ██████╗
// ╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗██╔══██╗
//    ██║   ███████║██████╔╝█████╗  ███████║██║  ██║
//    ██║   ██╔══██║██╔══██╗██╔══╝  ██╔══██║██║  ██║
//    ██║   ██║  ██║██║  ██║███████╗██║  ██║██████╔╝
//    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝
// @thread
SP_TYPEDEF_FN(s32, sp_thread_fn_t, void*);

typedef struct {
  sp_thread_fn_t fn;
  void* userdata;
  sp_semaphore_t semaphore;
} sp_thread_launch_t;

#if defined(SP_WIN32)
  typedef thrd_t sp_thread_t;

#elif defined(SP_MACOS)
  typedef pthread_t sp_thread_t;

#elif defined(SP_POSIX)
  typedef pthread_t sp_thread_t;
#endif

SP_API void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata);
SP_API void sp_thread_join(sp_thread_t* thread);
SP_API s32  sp_thread_launch(void* userdata);


//  ██████╗ ██████╗ ███╗   ██╗████████╗███████╗██╗  ██╗████████╗
// ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝
// ██║     ██║   ██║██╔██╗ ██║   ██║   █████╗   ╚███╔╝    ██║
// ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══╝   ██╔██╗    ██║
// ╚██████╗╚██████╔╝██║ ╚████║   ██║   ███████╗██╔╝ ██╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝   ╚═╝
// @context
#ifndef SP_RT_MAX_CONTEXT
  #define SP_RT_MAX_CONTEXT 16
#endif

#ifndef SP_RT_NUM_SPIN_LOCKS
  #define SP_RT_NUM_SPIN_LOCKS 32
#endif

typedef struct {
  sp_allocator_t allocator;
} sp_context_t;

typedef struct {
  sp_context_t contexts [SP_RT_MAX_CONTEXT];
  sp_mem_arena_t* scratch;
  u32 index;
} sp_tls_rt_t;

typedef struct {
  sp_mutex_t mutex;
  sp_spin_lock_t locks [SP_RT_NUM_SPIN_LOCKS];
  struct {
    pthread_key_t key;
    pthread_once_t once;
  } tls;
} sp_rt_t;

extern sp_rt_t sp_rt;

sp_tls_rt_t*  sp_tls_rt_get();
sp_context_t* sp_context_get();
void          sp_context_set(sp_context_t context);
void          sp_context_push(sp_context_t context);
void          sp_context_push_allocator(sp_allocator_t allocator);
void          sp_context_push_arena(sp_mem_arena_t* arena);
void          sp_context_pop();


// ██╗ ██████╗
// ██║██╔═══█╗
// ██║██║   ██║
// ██║██║   ██║
// ██║╚██████╔╝
// ╚═╝ ╚═════╝
// @io
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
SP_TYPEDEF_FN(u64, sp_io_pad_fn, sp_io_stream_t* stream, u64 size);
SP_TYPEDEF_FN(void, sp_io_close_fn, sp_io_stream_t* stream);

typedef struct {
  sp_io_size_fn  size;
  sp_io_seek_fn  seek;
  sp_io_read_fn  read;
  sp_io_write_fn write;
  sp_io_pad_fn   pad;
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

typedef struct {
  u8* base;
  u8* here;
  u8* stop;
  u8* max;
} sp_io_buffer_data_t;

struct sp_io_stream_t {
  sp_io_callbacks_t callbacks;
  union {
    sp_io_file_data_t file;
    sp_io_memory_data_t memory;
    sp_io_buffer_data_t buffer;
  };
  sp_allocator_t allocator;
};

SP_API sp_io_stream_t sp_io_from_file(sp_str_t path, sp_io_mode_t mode);
SP_API sp_io_stream_t sp_io_from_memory(void* memory, u64 size);
SP_API sp_io_stream_t sp_io_from_buffer();
SP_API sp_io_stream_t sp_io_from_file_handle(sp_os_file_handle_t handle, sp_io_file_close_mode_t close_mode);
SP_API u64            sp_io_read(sp_io_stream_t* stream, void* ptr, u64 size);
SP_API u64            sp_io_write(sp_io_stream_t* stream, const void* ptr, u64 size);
SP_API u64            sp_io_write_str(sp_io_stream_t* stream, sp_str_t str);
SP_API u64            sp_io_pad(sp_io_stream_t* stream, u64 size);
SP_API s64            sp_io_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence);
SP_API s64            sp_io_size(sp_io_stream_t* stream);
SP_API void           sp_io_close(sp_io_stream_t* stream);
SP_API sp_str_t       sp_io_read_file(sp_str_t path);
SP_API sp_str_t       sp_io_buffer_to_str(sp_io_stream_t* stream);


// ██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗
// ██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝
// ██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗
// ██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║
// ██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝
// @ps
#ifndef SP_PS_MAX_ARGS
  #define SP_PS_MAX_ARGS 16
#endif

#ifndef SP_PS_MAX_ENV
  #define SP_PS_MAX_ENV 16
#endif

typedef enum {
  SP_PS_IO_FILENO_NONE,
  SP_PS_IO_FILENO_STDIN = 0,
  SP_PS_IO_FILENO_STDOUT = 1,
  SP_PS_IO_FILENO_STDERR = 2,
} sp_ps_io_file_number_t;

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
  sp_io_stream_t stream;
  sp_ps_io_mode_t mode;
  sp_ps_io_blocking_t block;
} sp_ps_io_stream_config_t;

typedef struct {
  sp_ps_io_stream_config_t in;
  sp_ps_io_stream_config_t out;
  sp_ps_io_stream_config_t err;
} sp_ps_io_config_t;

typedef sp_ps_io_config_t sp_ps_io_t;

typedef struct {
  sp_env_t env;
  sp_env_var_t extra [SP_PS_MAX_ENV];
  sp_ps_env_mode_t mode;
} sp_ps_env_config_t;

typedef struct {
  sp_str_t command;
  sp_str_t args [SP_PS_MAX_ARGS];
  sp_da(sp_str_t) dyn_args;
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

#if defined(SP_WIN32)
typedef struct {
  void* placeholder;
} sp_ps_platform_t;

#elif defined(SP_POSIX)
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

typedef struct {
  sp_ps_io_t io;
  sp_ps_platform_t platform;
  sp_allocator_t allocator;
  pid_t pid;
} sp_ps_t;

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


// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║ ██║   ██║
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝
// @format
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
  union {
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES
  };

  sp_format_id_t id;
} sp_format_arg_t;

SP_TYPEDEF_FN(void, sp_format_fn_t, sp_str_builder_t*, sp_format_arg_t*);

typedef struct sp_formatter {
  sp_format_fn_t fn;
  sp_format_id_t id;
} sp_formatter_t;


#define SP_FMT_ARG(T, V) SP_RVAL(sp_format_arg_t) { .SP_FMT_UNION(T) = (V), .id = SP_FMT_ID(T) }

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

SP_API sp_str_t sp_format_str(sp_str_t fmt, ...);
SP_API sp_str_t sp_format(const c8* fmt, ...);
SP_API sp_str_t sp_format_v(sp_str_t fmt, va_list args);
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


// app
typedef enum {
  SP_APP_CONTINUE = 0,
  SP_APP_ERR = 1,
  SP_APP_QUIT = 2
} sp_app_result_t;

typedef struct sp_app sp_app_t;

sp_typedef_fn(sp_app_result_t, sp_app_init_fn_t, sp_app_t*);
sp_typedef_fn(sp_app_result_t, sp_app_poll_fn_t, sp_app_t*);
sp_typedef_fn(sp_app_result_t, sp_app_update_fn_t, sp_app_t*);
sp_typedef_fn(sp_app_result_t, sp_app_deinit_fn_t, sp_app_t*);

typedef struct {
  void* user_data;
  sp_app_init_fn_t on_init;
  sp_app_poll_fn_t on_poll;
  sp_app_update_fn_t on_update;
  sp_app_deinit_fn_t on_deinit;
  u32 fps;
} sp_app_config_t;

struct sp_app {
  void* user_data;
  sp_app_init_fn_t on_init;
  sp_app_poll_fn_t on_poll;
  sp_app_update_fn_t on_update;
  sp_app_deinit_fn_t on_deinit;

  s32 return_code;
  sp_atomic_s32 shutdown;

  u32 fps;

  struct {
    sp_tm_timer_t timer;
    u64 target;
    u64 accumulated;
    u64 num;
  } frame;
};

extern sp_app_config_t sp_main(s32 num_args, const c8** args);
SP_API sp_app_t*       sp_app_new(sp_app_config_t config);
SP_API s32             sp_app_run(sp_app_config_t config);

//
// ███╗   ███╗ ██████╗ ███╗   ██╗██╗████████╗ ██████╗ ██████╗
// ████╗ ████║██╔═══██╗████╗  ██║██║╚══██╔══╝██╔═══██╗██╔══██╗
// ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║   ██║   ██║██████╔╝
// ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║   ██║   ██║██╔══██╗
// ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
// @file_monitor
typedef enum sp_file_change_event_t {
	SP_FILE_CHANGE_EVENT_NONE = 0,
	SP_FILE_CHANGE_EVENT_ADDED = 1 << 0,
	SP_FILE_CHANGE_EVENT_MODIFIED = 1 << 1,
	SP_FILE_CHANGE_EVENT_REMOVED = 1 << 2,
} sp_fmon_event_kind_t;

typedef struct {
	sp_str_t file_path;
	sp_str_t file_name;
	sp_fmon_event_kind_t events;
	f32 time;
} sp_fmon_event_t;

typedef struct sp_fmon sp_fmon_t;
SP_TYPEDEF_FN(void, sp_fmon_fn_t, sp_fmon_t*, sp_fmon_event_t*, void*);

typedef struct {
	sp_hash_t hash;
	f64 last_event_time;
} sp_fmon_cache_t;

#if defined(SP_WIN32)
///////////
// WIN32 //
///////////
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

#elif defined(SP_LINUX)
///////////
// LINUX //
///////////
typedef struct {
  sp_da(s32) watch_descs;
  sp_da(sp_str_t) watch_paths;
  u8 buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  s32 fd;
} sp_fmon_os_t;

#elif defined(SP_MACOS)
///////////
// MACOS //
///////////
#ifndef SP_FMON_ARENA_SIZE
#define SP_FMON_ARENA_SIZE (64 * 1024)
#endif

#if defined(SP_FMON_MACOS_USE_FSEVENTS)
typedef struct {
  FSEventStreamRef stream;
  dispatch_queue_t queue;
  sp_da(sp_str_t) watch_paths;
  sp_da(sp_str_t) watch_files;
  sp_fmon_t* monitor;
  sp_mutex_t mutex;
  sp_mem_arena_t* watch_arena;
  sp_mem_arena_t* event_arena;
} sp_fmon_os_t;
#else
typedef struct {
  s32 kq;
  sp_da(s32) fds;
  sp_da(sp_str_t) watch_paths;
} sp_fmon_os_t;
#endif

///////////
// COSMO //
///////////
#elif defined(SP_COSMO)
typedef s32 sp_os_file_handle_t;

typedef struct {
  s32 dummy;
} sp_fmon_os_t;
#endif

#define SP_FILE_MONITOR_BUFFER_SIZE 4092

struct sp_fmon {
	sp_fmon_fn_t callback;
	void* userdata;
	sp_da(sp_fmon_event_t) changes;
	sp_da(sp_fmon_cache_t) cache;
	sp_fmon_os_t* os;
	sp_fmon_event_kind_t events_to_watch;
	u32 debounce_time_ms;
  sp_mem_arena_t* arena;
};

SP_API void sp_fmon_init(sp_fmon_t* m, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* user_data);
SP_API void sp_fmon_init_ex(sp_fmon_t* m, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* user_data, u32 debounce);
SP_API void sp_fmon_deinit(sp_fmon_t* monitor);
SP_API void sp_fmon_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_API void sp_fmon_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_API void sp_fmon_process_changes(sp_fmon_t* monitor);
SP_API void sp_fmon_emit_changes(sp_fmon_t* monitor);
SP_API sp_fmon_cache_t* sp_fmon_find_cache_entry(sp_fmon_t* monitor, sp_str_t file_path);


//  █████╗ ███████╗███████╗███████╗████████╗
// ██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝
// ███████║███████╗███████╗█████╗     ██║
// ██╔══██║╚════██║╚════██║██╔══╝     ██║
// ██║  ██║███████║███████║███████╗   ██║
// ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝
// @asset
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
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_kind_t kind;
} sp_asset_importer_config_t;

typedef struct {
  sp_asset_import_fn_t on_import;
  sp_asset_completion_fn_t on_completion;
  sp_asset_registry_t* registry;
  sp_asset_kind_t kind;
} sp_asset_importer_t;

struct sp_asset_import_context {
  sp_asset_registry_t* registry;
  sp_asset_importer_t* importer;
  sp_future_t* future;
  void* user_data;
  u32 asset_index;
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
  sp_da(sp_asset_t) assets;
  sp_da(sp_asset_importer_t) importers;
  sp_rb(sp_asset_import_context_t) import_queue;
  sp_rb(sp_asset_import_context_t) completion_queue;
  bool shutdown_requested;
};

SP_API void                 sp_asset_registry_init(sp_asset_registry_t* r, sp_asset_registry_config_t config);
SP_API void                 sp_asset_registry_shutdown(sp_asset_registry_t* r);
SP_API sp_future_t*         sp_asset_registry_import(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
SP_API sp_asset_t*          sp_asset_registry_add(sp_asset_registry_t* r, sp_asset_kind_t k, sp_str_t name, void* user_data);
SP_API sp_asset_t*          sp_asset_registry_find(sp_asset_registry_t* r, sp_asset_kind_t kind, sp_str_t name);
SP_API void                 sp_asset_registry_process_completions(sp_asset_registry_t* r);
SP_API sp_asset_t*          sp_asset_registry_reserve(sp_asset_registry_t* r);
SP_API sp_asset_importer_t* sp_asset_registry_find_importer(sp_asset_registry_t* r, sp_asset_kind_t kind);
SP_API s32                  sp_asset_registry_thread_fn(void* user_data);

// ███████╗██╗     ███████╗
// ██╔════╝██║     ██╔════╝
// █████╗  ██║     █████╗
// ██╔══╝  ██║     ██╔══╝
// ███████╗███████╗██║
// ╚══════╝╚══════╝╚═╝
// @elf
#if defined(SP_ELF)

#if !defined(SP_ELF_NO_VENDOR) && !defined(_ELF_H)
  typedef u64 Elf64_Addr;
  typedef u64 Elf64_Off;
  typedef u16 Elf64_Half;
  typedef u32 Elf64_Word;
  typedef s32 Elf64_Sword;
  typedef u64 Elf64_Xword;
  typedef s64 Elf64_Sxword;

  #define EI_NIDENT 16

  typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
  } Elf64_Ehdr;

  typedef struct {
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
  } Elf64_Shdr;

  typedef struct {
    Elf64_Word    st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half    st_shndx;
    Elf64_Addr    st_value;
    Elf64_Xword   st_size;
  } Elf64_Sym;

  typedef struct {
    Elf64_Addr   r_offset;
    Elf64_Xword  r_info;
    Elf64_Sxword r_addend;
  } Elf64_Rela;

  #define EI_MAG0    0
  #define EI_MAG1    1
  #define EI_MAG2    2
  #define EI_MAG3    3
  #define EI_CLASS   4
  #define EI_DATA    5
  #define EI_VERSION 6
  #define EI_OSABI   7
  #define EI_PAD     8

  #define ELFCLASS32 1
  #define ELFCLASS64 2

  #define ELFDATA2LSB 1

  #define EV_CURRENT 1

  #define ET_REL 1

  #define EM_X86_64 62

  #define SHT_NULL     0
  #define SHT_PROGBITS 1
  #define SHT_SYMTAB   2
  #define SHT_STRTAB   3
  #define SHT_RELA     4
  #define SHT_NOBITS   8

  #define SHF_WRITE     0x1
  #define SHF_ALLOC     0x2
  #define SHF_EXECINSTR 0x4

  #define SHN_UNDEF 0

  #define STB_LOCAL  0
  #define STB_GLOBAL 1

  #define STT_NOTYPE 0
  #define STT_OBJECT 1
  #define STT_FUNC   2

  #define R_X86_64_PC32 2

  #define ELFMAG0 0x7f
  #define ELFMAG1 'E'
  #define ELFMAG2 'L'
  #define ELFMAG3 'F'

  #define ELFOSABI_NONE 0

  #define ELF64_ST_BIND(i)   ((i) >> 4)
  #define ELF64_ST_TYPE(i)   ((i) & 0xf)
  #define ELF64_ST_INFO(b,t) (((b) << 4) | ((t) & 0xf))

  #define ELF64_R_SYM(i)    ((i) >> 32)
  #define ELF64_R_TYPE(i)   ((i) & 0xffffffff)
  #define ELF64_R_INFO(s,t) (((Elf64_Xword)(s) << 32) | (t))
#endif

#define SP_ELF_DEFAULT_BUFFER_SIZE 64
#define SP_ELF_NULL_INDEX 0

typedef struct {
  u8* data;
  u64 size;
  u64 capacity;
} sp_elf_buffer_t;

typedef struct {
  sp_str_t name;
  u32 index;
  u32 type;
  u64 flags;
  u64 addr;
  u64 alignment;
  u64 entry_size;
  u32 link;
  u32 info;
  sp_elf_buffer_t buffer;
} sp_elf_section_t;

typedef struct {
  sp_da(sp_elf_section_t) sections;
  sp_ht(sp_str_t, u32) section_map;
  sp_allocator_t allocator;
} sp_elf_t;

sp_elf_t*         sp_elf_new();
sp_elf_t*         sp_elf_new_alloc(sp_allocator_t a);
sp_elf_t*         sp_elf_new_with_null_section();
void              sp_elf_free(sp_elf_t* elf);
sp_elf_section_t* sp_elf_add_section(sp_elf_t* elf, sp_str_t name, u32 type, u64 addralign);
sp_elf_section_t* sp_elf_find_section_by_index(sp_elf_t* elf, u32 index);
sp_elf_section_t* sp_elf_find_section_by_name(sp_elf_t* elf, sp_str_t name);
u32               sp_elf_num_sections(sp_elf_t* elf);
u32               sp_elf_section_num_entries(sp_elf_section_t* section);
void*             sp_elf_section_reserve_aligned(sp_elf_section_t* section, u64 size, u64 align);
void*             sp_elf_section_reserve_entry(sp_elf_section_t* section);
u8*               sp_elf_section_reserve_bytes(sp_elf_section_t* section, u64 size);
Elf64_Sym*        sp_elf_symtab_get(sp_elf_section_t* symtab, u32 index);
void              sp_elf_symtab_sort(sp_elf_section_t* symtab, sp_elf_t* elf);
Elf64_Rela*       sp_elf_rela_get(sp_elf_section_t* rela, u32 index);
u32               sp_elf_strtab_size(sp_elf_section_t* strtab);
sp_str_t          sp_elf_strtab_get(sp_elf_section_t* strtab, u32 offset);
sp_err_t          sp_elf_write(sp_elf_t* elf, sp_io_stream_t* out);
sp_err_t          sp_elf_write_to_file(sp_elf_t* elf, sp_str_t path);
#endif


SP_END_EXTERN_C()

#ifdef SP_CPP
SP_API sp_str_t operator/(const sp_str_t& a, const sp_str_t& b);
SP_API sp_str_t operator/(const sp_str_t& a, const c8* b);

template <typename T>
SP_API sp_format_arg_t sp_make_format_arg(sp_format_id_t id, T&& data) {
  sp_format_arg_t result = SP_ZERO_STRUCT(sp_format_arg_t);
  result.id = id;
  sp_os_copy_memory(&data, &result.u8_value, sizeof(data));

  return result;
}
#endif

#endif








#ifndef SP_SP_C
#define SP_SP_C

#ifdef SP_IMPLEMENTATION
// @implementation

SP_BEGIN_EXTERN_C()

sp_rt_t sp_rt = {
  .mutex = PTHREAD_MUTEX_INITIALIZER,
  .tls = { .once = PTHREAD_ONCE_INIT }
};

//  ███╗   ███╗ █████╗ ████████╗██╗  ██╗
//  ████╗ ████║██╔══██╗╚══██╔══╝██║  ██║
//  ██╔████╔██║███████║   ██║   ███████║
//  ██║╚██╔╝██║██╔══██║   ██║   ██╔══██║
//  ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║
//  ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝
//  @math

sp_color_t sp_color_rgb_255(u8 r, u8 g, u8 b) {
  return (sp_color_t) SP_COLOR_RGB(r, g, b);
}

sp_color_t sp_color_rgb_to_hsv(sp_color_t color) {
  f32 r = color.r;
  f32 g = color.g;
  f32 b = color.b;

  f32 max = SP_MAX(r, SP_MAX(g, b));
  f32 min = SP_MIN(r, SP_MIN(g, b));
  f32 delta = max - min;

  f32 h = 0.0f;
  f32 s = 0.0f;
  f32 v = max;

  if (delta > 1e-6f && max > 1e-6f) {
    s = delta / max;

    if (max - r < 1e-6f) {
      h = (g - b) / delta;
    } else if (max - g < 1e-6f) {
      h = 2.0f + (b - r) / delta;
    } else {
      h = 4.0f + (r - g) / delta;
    }

    h = h / 6.0f;
    if (h < 0.0f) {
      h += 1.0f;
    }
  }

  return (sp_color_t){
    .h = h * 360.0f,
    .s = s * 100.0f,
    .v = v * 100.0f,
    .a = color.a
  };
}

sp_color_t sp_color_hsv_to_rgb(sp_color_t color) {
  f32 h = color.h / 360.0f;
  f32 s = color.s / 100.0f;
  f32 v = color.v / 100.0f;

  f32 r = v;
  f32 g = v;
  f32 b = v;

  if (s > 1e-6f) {
    f32 h6 = h * 6.0f;
    if (h6 >= 6.0f) {
      h6 = 0.0f;
    }
    s32 sector = (s32)h6;
    f32 f = h6 - (f32)sector;

    f32 p = v * (1.0f - s);
    f32 q = v * (1.0f - s * f);
    f32 t = v * (1.0f - s * (1.0f - f));

    switch (sector) {
      case 0: { r = v; g = t; b = p; break; }
      case 1: { r = q; g = v; b = p; break; }
      case 2: { r = p; g = v; b = t; break; }
      case 3: { r = p; g = q; b = v; break; }
      case 4: { r = t; g = p; b = v; break; }
      case 5: { r = v; g = p; b = q; break; }
    }
  }

  return (sp_color_t){
    .r = r,
    .g = g,
    .b = b,
    .a = color.a
  };
}

f32 sp_inv_sqrtf(f32 value) {
  return 1.0f / SP_SQRTF(value);
}

f32 sp_lerp(f32 a, f32 t, f32 b) {
  return (1.0f - t) * a + t * b;
}

f32 sp_clamp(f32 low, f32 value, f32 high) {
  f32 result = value;
  if (result < low) {
    result = low;
  }
  if (result > high) {
    result = high;
  }
  return result;
}

sp_vec2_t sp_vec2(f32 x, f32 y) {
  return (sp_vec2_t){ .x = x, .y = y };
}

sp_vec3_t sp_vec3(f32 x, f32 y, f32 z) {
  return (sp_vec3_t){ .x = x, .y = y, .z = z };
}

sp_vec4_t sp_vec4(f32 x, f32 y, f32 z, f32 w) {
  return (sp_vec4_t){ .x = x, .y = y, .z = z, .w = w };
}

sp_vec4_t sp_vec4V(sp_vec3_t xyz, f32 w) {
  return (sp_vec4_t){ .xyz = xyz, .w = w };
}

sp_vec2_t sp_vec2_add(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x + right.x, .y = left.y + right.y };
}

sp_vec3_t sp_vec3_add(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x + right.x, .y = left.y + right.y, .z = left.z + right.z };
}

sp_vec4_t sp_vec4_add(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x + right.x, .y = left.y + right.y, .z = left.z + right.z, .w = left.w + right.w };
}

sp_vec2_t sp_vec2_sub(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x - right.x, .y = left.y - right.y };
}

sp_vec3_t sp_vec3_sub(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x - right.x, .y = left.y - right.y, .z = left.z - right.z };
}

sp_vec4_t sp_vec4_sub(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x - right.x, .y = left.y - right.y, .z = left.z - right.z, .w = left.w - right.w };
}

sp_vec2_t sp_vec2_mul(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x * right.x, .y = left.y * right.y };
}

sp_vec2_t sp_vec2_scale(sp_vec2_t left, f32 right) {
  return (sp_vec2_t){ .x = left.x * right, .y = left.y * right };
}

sp_vec3_t sp_vec3_mul(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x * right.x, .y = left.y * right.y, .z = left.z * right.z };
}

sp_vec3_t sp_vec3_scale(sp_vec3_t left, f32 right) {
  return (sp_vec3_t){ .x = left.x * right, .y = left.y * right, .z = left.z * right };
}

sp_vec4_t sp_vec4_mul(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x * right.x, .y = left.y * right.y, .z = left.z * right.z, .w = left.w * right.w };
}

sp_vec4_t sp_vec4_scale(sp_vec4_t left, f32 right) {
  return (sp_vec4_t){ .x = left.x * right, .y = left.y * right, .z = left.z * right, .w = left.w * right };
}

sp_vec2_t sp_vec2_div(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x / right.x, .y = left.y / right.y };
}

sp_vec2_t sp_vec2_divf(sp_vec2_t left, f32 right) {
  return (sp_vec2_t){ .x = left.x / right, .y = left.y / right };
}

sp_vec3_t sp_vec3_div(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x / right.x, .y = left.y / right.y, .z = left.z / right.z };
}

sp_vec3_t sp_vec3_divf(sp_vec3_t left, f32 right) {
  return (sp_vec3_t){ .x = left.x / right, .y = left.y / right, .z = left.z / right };
}

sp_vec4_t sp_vec4_div(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x / right.x, .y = left.y / right.y, .z = left.z / right.z, .w = left.w / right.w };
}

sp_vec4_t sp_vec4_divf(sp_vec4_t left, f32 right) {
  return (sp_vec4_t){ .x = left.x / right, .y = left.y / right, .z = left.z / right, .w = left.w / right };
}

bool sp_vec2_eq(sp_vec2_t left, sp_vec2_t right) {
  return left.x == right.x && left.y == right.y;
}

bool sp_vec3_eq(sp_vec3_t left, sp_vec3_t right) {
  return left.x == right.x && left.y == right.y && left.z == right.z;
}

bool sp_vec4_eq(sp_vec4_t left, sp_vec4_t right) {
  return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

f32 sp_vec2_dot(sp_vec2_t left, sp_vec2_t right) {
  return (left.x * right.x) + (left.y * right.y);
}

f32 sp_vec3_dot(sp_vec3_t left, sp_vec3_t right) {
  return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
}

f32 sp_vec4_dot(sp_vec4_t left, sp_vec4_t right) {
  return ((left.x * right.x) + (left.z * right.z)) + ((left.y * right.y) + (left.w * right.w));
}

sp_vec3_t sp_vec3_cross(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){
    .x = (left.y * right.z) - (left.z * right.y),
    .y = (left.z * right.x) - (left.x * right.z),
    .z = (left.x * right.y) - (left.y * right.x)
  };
}

f32 sp_vec2_len_sqr(sp_vec2_t v) {
  return sp_vec2_dot(v, v);
}

f32 sp_vec3_len_sqr(sp_vec3_t v) {
  return sp_vec3_dot(v, v);
}

f32 sp_vec4_len_sqr(sp_vec4_t v) {
  return sp_vec4_dot(v, v);
}

f32 sp_vec2_len(sp_vec2_t v) {
  return sqrtf(sp_vec2_len_sqr(v));
}

f32 sp_vec3_len(sp_vec3_t v) {
  return sqrtf(sp_vec3_len_sqr(v));
}

f32 sp_vec4_len(sp_vec4_t v) {
  return sqrtf(sp_vec4_len_sqr(v));
}

sp_vec2_t sp_vec2_norm(sp_vec2_t v) {
  return sp_vec2_scale(v, sp_inv_sqrtf(sp_vec2_dot(v, v)));
}

sp_vec3_t sp_vec3_norm(sp_vec3_t v) {
  return sp_vec3_scale(v, sp_inv_sqrtf(sp_vec3_dot(v, v)));
}

sp_vec4_t sp_vec4_norm(sp_vec4_t v) {
  return sp_vec4_scale(v, sp_inv_sqrtf(sp_vec4_dot(v, v)));
}

sp_vec2_t sp_vec2_lerp(sp_vec2_t a, f32 t, sp_vec2_t b) {
  return sp_vec2_add(sp_vec2_scale(a, 1.0f - t), sp_vec2_scale(b, t));
}

sp_vec3_t sp_vec3_lerp(sp_vec3_t a, f32 t, sp_vec3_t b) {
  return sp_vec3_add(sp_vec3_scale(a, 1.0f - t), sp_vec3_scale(b, t));
}

sp_vec4_t sp_vec4_lerp(sp_vec4_t a, f32 t, sp_vec4_t b) {
  return sp_vec4_add(sp_vec4_scale(a, 1.0f - t), sp_vec4_scale(b, t));
}

sp_interp_t sp_interp_build(f32 start, f32 target, f32 time) {
  return (sp_interp_t){ .start = start, .delta = target - start, .t = 0, .time_scale = 1.0f / time };
}

bool sp_interp_update(sp_interp_t* interp, f32 dt) {
  interp->t += dt * interp->time_scale;
  if (interp->t > 1.0f) { interp->t = 1.0f; }
  return interp->t >= 1.0f;
}

f32 sp_interp_lerp(sp_interp_t* interp) {
  return interp->start + interp->delta * interp->t;
}

f32 sp_interp_ease_in(sp_interp_t* interp) {
  f32 eased = interp->t * interp->t;
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_out(sp_interp_t* interp) {
  f32 eased = 1.0f - (1.0f - interp->t) * (1.0f - interp->t);
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_inout(sp_interp_t* interp) {
  f32 eased;
  if (interp->t < 0.5f) {
    eased = 2.0f * interp->t * interp->t;
  } else {
    eased = 1.0f - (-2.0f * interp->t + 2.0f) * (-2.0f * interp->t + 2.0f) / 2.0f;
  }
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_inout_bounce(sp_interp_t* interp) {
  f32 c1 = 1.70158f;
  f32 c2 = c1 * 1.525f;
  f32 eased;
  if (interp->t < 0.5f) {
    f32 x = 2.0f * interp->t;
    eased = 0.5f * (x * x * ((c2 + 1.0f) * x - c2));
  } else {
    f32 x = 2.0f * interp->t - 2.0f;
    eased = 0.5f * (x * x * ((c2 + 1.0f) * x + c2) + 2.0f);
  }
  return interp->start + interp->delta * eased;
}

f32 sp_interp_exponential(sp_interp_t* interp) {
  f32 k = 5.0f;
  f32 e_k = 148.413159f;
  f32 eased = (expf(k * interp->t) - 1.0f) / (e_k - 1.0f);
  return interp->start + interp->delta * eased;
}

f32 sp_interp_parabolic(sp_interp_t* interp) {
  f32 x = 2.0f * interp->t - 1.0f;
  f32 eased = 1.0f - x * x;
  return interp->start + interp->delta * eased;
}

//  ██╗  ██╗ █████╗ ███████╗██╗  ██╗██╗███╗   ██╗ ██████╗
//  ██║  ██║██╔══██╗██╔════╝██║  ██║██║████╗  ██║██╔════╝
//  ███████║███████║███████╗███████║██║██╔██╗ ██║██║  ███╗
//  ██╔══██║██╔══██║╚════██║██╔══██║██║██║╚██╗██║██║   ██║
//  ██║  ██║██║  ██║███████║██║  ██║██║██║ ╚████║╚██████╔╝
//  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ═════╝
//  @hash
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

// ██╗  ██╗ █████╗ ███████╗██╗  ██╗    ████████╗ █████╗ ██████╗ ██╗     ███████╗
// ██║  ██║██╔══██╗██╔════╝██║  ██║    ╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝
// ███████║███████║███████╗███████║       ██║   ███████║██████╔╝██║     █████╗
// ██╔══██║██╔══██║╚════██║██╔══██║       ██║   ██╔══██║██╔══██╗██║     ██╔══╝
// ██║  ██║██║  ██║███████║██║  ██║       ██║   ██║  ██║██████╔╝███████╗███████╗
// ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝       ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝
// @hash_table @ht
bool sp_ht_on_compare_key(void* ka, void* kb, u32 size) {
  return sp_mem_is_equal(ka, kb, size);
}

sp_hash_t sp_ht_on_hash_key(void *key, u32 size) {
  return sp_hash_bytes(key, size, SP_HT_HASH_SEED);
}

sp_hash_t sp_ht_on_hash_str_key(void* key, u32 size) {
  (void)size;
  sp_str_t* str = (sp_str_t*)key;
  return sp_hash_str(*str);
}

bool sp_ht_on_compare_str_key(void* ka, void* kb, u32 size) {
  (void)size;
  sp_str_t* sa = (sp_str_t*)ka;
  sp_str_t* sb = (sp_str_t*)kb;
  return sp_str_equal(*sa, *sb);
}

sp_hash_t sp_ht_on_hash_cstr_key(void* key, u32 size) {
  (void)size;
  const c8** str = (const c8**)key;
  return sp_hash_cstr(*str);
}

bool sp_ht_on_compare_cstr_key(void* ka, void* kb, u32 size) {
  (void)size;
  const c8** sa = (const c8**)ka;
  const c8** sb = (const c8**)kb;
  return sp_cstr_equal(*sa, *sb);
}

u64 sp_ht_get_key_index_fn(void** data, void* key, u64 capacity, sp_ht_info_t info) {
  if (!data || !*data || !key || !capacity) return SP_HT_INVALID_INDEX;

  sp_hash_t hash = info.fn.hash(key, info.size.key);
  u64 hash_idx = hash % capacity;

  for (u64 c = 0; c < capacity; ++c) {
    u64 i = (hash_idx + c) % capacity;
    u64 offset = i * info.stride.entry;
    sp_ht_entry_state state = *(sp_ht_entry_state*)((c8*)(*data) + offset + info.stride.kv);

    if (state == SP_HT_ENTRY_INACTIVE) {
      break;
    }
    if (state == SP_HT_ENTRY_DELETED) {
      continue;
    }
    void* k = (c8*)(*data) + offset;
    if (info.fn.compare(k, key, info.size.key)) {
      return i;
    }
  }
  return SP_HT_INVALID_INDEX;
}

void sp_ht_resize_impl(void** data, u64 old_cap, u64 new_cap, sp_ht_info_t info) {
  if (!data || new_cap <= old_cap) return;

  sp_context_push_allocator(info.allocator);

  void* old_data = *data;
  void* new_data = sp_alloc(new_cap * info.stride.entry);

  for (u64 i = 0; i < old_cap; ++i) {
    u64 offset = i * info.stride.entry;
    sp_ht_entry_state state = *(sp_ht_entry_state*)((c8*)old_data + offset + info.stride.kv);
    if (state != SP_HT_ENTRY_ACTIVE) continue;

    void* old_key = (c8*)old_data + offset;
    sp_hash_t hash = info.fn.hash(old_key, info.size.key);
    u64 new_idx = hash % new_cap;

    while (*(sp_ht_entry_state*)((c8*)new_data + new_idx * info.stride.entry + info.stride.kv) == SP_HT_ENTRY_ACTIVE) {
      new_idx = (new_idx + 1) % new_cap;
    }

    sp_mem_copy((c8*)old_data + offset, (c8*)new_data + new_idx * info.stride.entry, info.stride.kv);
    *(sp_ht_entry_state*)((c8*)new_data + new_idx * info.stride.entry + info.stride.kv) = SP_HT_ENTRY_ACTIVE;
  }

  sp_free(old_data);
  *data = new_data;
  sp_context_pop();
}

void sp_ht_insert_impl(void* ht, void* key, void* val, sp_ht_info_t info) {
  sp_context_push_allocator(info.allocator);
  u8* base = (u8*)ht;
  void** data = (void**)base;
  u64* size = (u64*)(base + info.header.size);
  u64* capacity = (u64*)(base + info.header.capacity);

  u64 cap = *capacity;
  if (*size * 4 >= cap * 3) {
    u64 new_cap = cap ? cap * 2 : 2;
    sp_ht_resize_impl(data, cap, new_cap, info);
    *capacity = new_cap;
    cap = new_cap;
  }

  u64 existing = sp_ht_get_key_index_fn(data, key, cap, info);
  if (existing != SP_HT_INVALID_INDEX) {
    u8* entry = (u8*)(*data) + existing * info.stride.entry;
    sp_mem_copy(val, entry + info.stride.value, info.size.value);
  }
  else {
    sp_hash_t hash = info.fn.hash(key, info.size.key);
    u64 idx = hash % cap;
    while (*(sp_ht_entry_state*)((u8*)(*data) + idx * info.stride.entry + info.stride.kv) == SP_HT_ENTRY_ACTIVE) {
      idx = (idx + 1) % cap;
    }
    u8* entry = (u8*)(*data) + idx * info.stride.entry;
    sp_mem_copy(key, entry, info.size.key);
    sp_mem_copy(val, entry + info.stride.value, info.size.value);
    *(sp_ht_entry_state*)(entry + info.stride.kv) = SP_HT_ENTRY_ACTIVE;
    (*size)++;
  }
  sp_context_pop();
}

sp_ht_it_t sp_ht_it_init_fn(void** data, u64 capacity, sp_ht_info_t info) {
  if (!data || !*data) return 0;
  sp_ht_it_t it = 0;
  for (; it < capacity; ++it) {
    u64 offset = it * info.stride.entry;
    sp_ht_entry_state state = *(sp_ht_entry_state*)((u8*)*data + offset + info.stride.kv);
    if (state == SP_HT_ENTRY_ACTIVE) {
      break;
    }
  }
  return it;
}

void sp_ht_it_advance_fn(void** data, u64 capacity, u64* it, sp_ht_info_t info) {
  if (!data || !*data) return;
  (*it)++;
  for (; *it < capacity; ++*it) {
    u64 offset = *it * info.stride.entry;
    sp_ht_entry_state state = *(sp_ht_entry_state*)((u8*)*data + offset + info.stride.kv);
    if (state == SP_HT_ENTRY_ACTIVE) {
      break;
    }
  }
}

// ██████╗ ██╗   ██╗███╗   ██╗   █████╗ ██████╗ ██████╗  █████╗ ██╗   ██╗
// ██╔══██╗╚██╗ ██╔╝████╗  ██║  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝
// ██║  ██║ ╚████╔╝ ██╔██╗ ██║  ███████║██████╔╝██████╔╝███████║ ╚████╔╝
// ██║  ██║  ╚██╔╝  ██║╚██╗██║  ██╔══██║██╔══██╗██╔══██╗██╔══██║  ╚██╔╝
// ██████╔╝   ██║   ██║ ╚████║  ██║  ██║██║  ██║██║  ██║██║  ██║   ██║
// ╚═════╝    ╚═╝   ╚═╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝
// @dyn_array @da
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
    sp_mem_copy(val, ((u8*)(*arr)) + sp_dyn_array_size(*arr) * val_len, val_len);
    sp_dyn_array_head(*arr)->size++;
  }
}

// ██████╗ ██╗███╗   ██╗ ██████╗      ██████╗ ██╗   ██╗███████╗██╗   ██╗███████╗
// ██╔══██╗██║████╗  ██║██╔════╝     ██╔═══██╗██║   ██║██╔════╝██║   ██║██╔════╝
// ██████╔╝██║██╔██╗ ██║██║  ███╗    ██║   ██║██║   ██║█████╗  ██║   ██║█████╗
// ██╔══██╗██║██║╚██╗██║██║   ██║    ██║▄▄ ██║██║   ██║██╔══╝  ██║   ██║██╔══╝
// ██║  ██║██║██║ ╚████║╚██████╔╝    ╚██████╔╝╚██████╔╝███████╗╚██████╔╝███████╗
// ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚══▀▀═╝  ╚═════╝ ╚══════╝ ╚═════╝ ╚══════╝
// @ring_buffer @rb
void* sp_rb_grow_impl(void* arr, u32 elem_size, u32 new_cap) {
  sp_ring_buffer_t* new_data = (sp_ring_buffer_t*)sp_alloc(new_cap * elem_size + sizeof(sp_ring_buffer_t));
  if (!new_data) return SP_NULLPTR;

  new_data->head = 0;
  new_data->capacity = (s32)new_cap;
  new_data->mode = SP_RQ_MODE_GROW;

  if (arr) {
    sp_ring_buffer_t* old = sp_rb_head(arr);
    s32 old_size = old->size;
    s32 old_cap = old->capacity;
    s32 old_head = old->head;
    new_data->size = old_size;
    new_data->mode = old->mode;

    u8* new_arr = (u8*)new_data + sizeof(sp_ring_buffer_t);
    u8* old_arr = (u8*)arr;

    s32 first_chunk = old_cap - old_head;
    if (first_chunk > old_size) first_chunk = old_size;
    sp_mem_copy(old_arr + old_head * elem_size, new_arr, first_chunk * elem_size);

    s32 second_chunk = old_size - first_chunk;
    if (second_chunk > 0) {
      sp_mem_copy(old_arr, new_arr + first_chunk * elem_size, second_chunk * elem_size);
    }

    sp_free(old);
  }
  else {
    new_data->size = 0;
  }

  return (u8*)new_data + sizeof(sp_ring_buffer_t);
}

// ███████╗██╗██╗  ██╗███████╗██████╗      █████╗ ██████╗ ██████╗  █████╗ ██╗   ██╗
// ██╔════╝██║╚██╗██╔╝██╔════╝██╔══██╗    ██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝
// █████╗  ██║ ╚███╔╝ █████╗  ██║  ██║    ███████║██████╔╝██████╔╝███████║ ╚███╔╝
// ██╔══╝  ██║ ██╔██╗ ██╔══╝  ██║  ██║    ██╔══██║██╔══██╗██╔══██╗██╔══██║  ╚██╔╝
// ██║     ██║██╔╝ ██╗███████╗██████╔╝    ██║  ██║██║  ██║██║  ██║██║  ██║   ██║
// ╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚═════╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝
// @fixed_array
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
  if (data) sp_mem_copy(data, reserved, buffer->element_size * count);
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



// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║   ██║
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═     ╚═╝╚═╝  ╚═╝   ╚═╝
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
  sp_tls_rt_t* state = sp_tls_rt_get();
  if (state) {
    sp_fmt_format_unsigned(builder, state->index, 10);
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
  sp_str_t color;
  u32 flags;
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
  #define SP_FMT_X(ID, t) (sp_formatter_t) { .fn = SP_FMT_FN(ID), .id = SP_FMT_ID(ID) },

  static sp_formatter_t formatters [] = {
    SP_FORMAT_TYPES
  };

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_format_parser_t parser = {
    .fmt = fmt,
    .it = 0
  };

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

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

// ██╗      ██████╗  ██████╗
// ██║     ██╔═══██╗██╔════╝
// ██║     ██║   ██║██║  ███╗
// ██║     ██║   ██║██║   ██║
// ███████╗╚██████╔╝╚██████╔
// ╚══════╝ ╚═════╝  ╚═════╝
// @log
void sp_log(sp_str_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t formatted = sp_format_v(fmt, args);
  va_end(args);

  sp_os_print(formatted);
  sp_os_print(sp_str_lit("\n"));
}


//  ██████╗ ██████╗ ███╗   ██╗████████╗███████╗██╗  ██╗████████╗
// ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝
// ██║     ██║   ██║██╔██╗ ██║   ██║   █████╗   ╚███╔╝    ██║
// ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══╝   ██╔██╗    ██║
// ╚██████╗╚██████╔╝██║ ╚████║   ██║   ███████╗██╔╝ ██╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ═╝   ╚═╝
//  @context
void sp_context_check_index() {
  sp_tls_rt_t* state = sp_tls_rt_get();
  SP_ASSERT(state->index > 0);
  SP_ASSERT(state->index < SP_RT_MAX_CONTEXT);
}

void sp_context_set(sp_context_t context) {
  sp_tls_rt_t* state = sp_tls_rt_get();
  state->contexts[state->index] = context;
}

void sp_context_push(sp_context_t context) {
  sp_tls_rt_t* state = sp_tls_rt_get();
  SP_ASSERT(state->index + 1 < SP_RT_MAX_CONTEXT);
  state->index++;
  state->contexts[state->index] = context;
}

void sp_context_push_allocator(sp_allocator_t allocator) {
  sp_tls_rt_t* state = sp_tls_rt_get();
  sp_context_t context = state->contexts[state->index];
  context.allocator = allocator;
  sp_context_push(context);
}

void sp_context_push_arena(sp_mem_arena_t* arena) {
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));
}

void sp_context_pop() {
  sp_tls_rt_t* state = sp_tls_rt_get();
  SP_ASSERT(state->index > 0);
  state->contexts[state->index] = SP_ZERO_STRUCT(sp_context_t); // Not required, just for the debugger
  state->index--;
}

void sp_context_on_deinit(void* ptr) {
  if (ptr) {
    sp_tls_rt_t* state = (sp_tls_rt_t*)ptr;
    sp_mem_arena_destroy(state->scratch);
    sp_mem_os_free(ptr);
  }
}

void sp_context_on_init() {
  pthread_key_create(&sp_rt.tls.key, sp_context_on_deinit);
}

sp_tls_rt_t* sp_tls_rt_get() {
  pthread_once(&sp_rt.tls.once, sp_context_on_init);

  sp_tls_rt_t* state = (sp_tls_rt_t*)pthread_getspecific(sp_rt.tls.key);
  if (!state) {
    state = (sp_tls_rt_t*)sp_mem_os_alloc(sizeof(sp_tls_rt_t));
    *state = (sp_tls_rt_t) {
      .contexts = {
        { .allocator = sp_mem_libc_new() }
      },
      .index = 0,
    };
    pthread_setspecific(sp_rt.tls.key, state);
    state->scratch = sp_mem_arena_new(SP_MEM_ARENA_BLOCK_SIZE);
  }

  return state;
}

sp_context_t* sp_context_get() {
  sp_tls_rt_t* state = sp_tls_rt_get();
  return &state->contexts[state->index];
}

// ███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗
// ████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗╚██╗ ██╔╝
// ██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝ ╚████╔╝
// ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗  ╚██╔╝
// ██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║   ██║
// ╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝
// @memory
void* sp_alloc(u32 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_alloc(ctx->allocator, size);
}

void* sp_realloc(void* memory, u32 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_realloc(ctx->allocator, memory, size);
}

void sp_free(void* memory) {
  sp_context_t* ctx = sp_context_get();
  sp_mem_allocator_free(ctx->allocator, memory);
}

void* sp_mem_allocator_alloc(sp_allocator_t allocator, u32 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
}

void* sp_mem_allocator_realloc(sp_allocator_t allocator, void* memory, u32 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_RESIZE, size, memory);
}

void sp_mem_allocator_free(sp_allocator_t allocator, void* buffer) {
  allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_FREE, 0, buffer);
}

typedef struct {
  u32 size;
  u8 padding [12];
} sp_arena_alloc_header_t;

sp_mem_arena_block_t* sp_mem_arena_block_new(sp_mem_arena_t* arena, u32 capacity) {
  sp_mem_arena_block_t* block = sp_mem_allocator_alloc_type(arena->allocator, sp_mem_arena_block_t);
  block->next = SP_NULLPTR;
  block->buffer = sp_mem_allocator_alloc_n(arena->allocator, u8, capacity);
  block->capacity = capacity;
  block->bytes_used = 0;
  return block;
}

sp_mem_arena_t* sp_mem_arena_new(u32 block_size) {
  sp_mem_arena_t* arena = sp_alloc_type(sp_mem_arena_t);
  arena->allocator = sp_context_get()->allocator;

  sp_mem_arena_block_t* block = sp_mem_arena_block_new(arena, block_size);
  arena->head = block;
  arena->current = block;
  arena->block_size = block_size;
  return arena;
}

sp_allocator_t sp_mem_arena_as_allocator(sp_mem_arena_t* a) {
  return (sp_allocator_t) {
    .on_alloc = sp_mem_arena_on_alloc,
    .user_data = a
  };
}

void sp_mem_arena_clear(sp_mem_arena_t* arena) {
  arena->head->bytes_used = 0;
  arena->current = arena->head;
}

void sp_mem_arena_destroy(sp_mem_arena_t* arena) {
  sp_mem_arena_block_t* block = arena->head;
  while (block) {
    sp_mem_arena_block_t* next = block->next;
    if (block->buffer) {
      sp_mem_allocator_free(arena->allocator, block->buffer);
    }
    sp_mem_allocator_free(arena->allocator, block);
    block = next;
  }

  arena->head = SP_NULLPTR;
  arena->current = SP_NULLPTR;
  sp_mem_allocator_free(arena->allocator, arena);
}

u32 sp_mem_arena_capacity(sp_mem_arena_t* arena) {
  u32 total = 0;
  for (sp_mem_arena_block_t* block = arena->head; block; block = block->next) {
    total += block->capacity;
  }
  return total;
}

u32 sp_mem_arena_bytes_used(sp_mem_arena_t* arena) {
  u32 total = 0;
  for (sp_mem_arena_block_t* block = arena->head; ; block = block->next) {
    total += block->bytes_used;
    if (block == arena->current) break;
  }
  return total;
}

void* sp_mem_arena_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u32 size, void* old_memory) {
  sp_mem_arena_t* arena = (sp_mem_arena_t*)user_data;

  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      sp_mem_arena_block_t* block = arena->current;
      u32 aligned = sp_align_offset(block->bytes_used, SP_MEM_ALIGNMENT);
      u32 total_bytes = aligned + sizeof(sp_arena_alloc_header_t) + size;

      if (total_bytes > block->capacity) {
        u32 alloc_size = sizeof(sp_arena_alloc_header_t) + size;
        u32 new_capacity = SP_MAX(arena->block_size, alloc_size);

        if (block->next && block->next->capacity >= alloc_size) {
          block = block->next;
        } else {
          sp_mem_arena_block_t* new_block = sp_mem_arena_block_new(arena, new_capacity);
          new_block->next = block->next;
          block->next = new_block;
          block = new_block;
        }
        arena->current = block;
        aligned = 0;
        total_bytes = sizeof(sp_arena_alloc_header_t) + size;
      }

      sp_arena_alloc_header_t* header = (sp_arena_alloc_header_t*)(block->buffer + aligned);
      header->size = size;

      void* ptr = (u8*)header + sizeof(sp_arena_alloc_header_t);
      sp_mem_zero(ptr, size);
      block->bytes_used = total_bytes;

      return ptr;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      void* new_memory = sp_mem_arena_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, SP_NULLPTR);
      if (old_memory) {
        sp_arena_alloc_header_t* header = (sp_arena_alloc_header_t*)((u8*)old_memory - sizeof(sp_arena_alloc_header_t));
        sp_mem_move(old_memory, new_memory, SP_MIN(header->size, size));
      }
      return new_memory;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      return SP_NULLPTR;
    }
  }

  SP_UNREACHABLE_RETURN(SP_NULLPTR);
}

void* sp_mem_arena_alloc(sp_mem_arena_t* arena, u32 size) {
  return sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, size, SP_NULLPTR);
}

void* sp_mem_arena_realloc(sp_mem_arena_t* arena, void* ptr, u32 size) {
  return sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_RESIZE, size, ptr);
}

void sp_mem_arena_free(sp_mem_arena_t* arena, void* ptr) {
  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_FREE, 0, ptr);
}

sp_mem_arena_marker_t sp_mem_arena_mark(sp_mem_arena_t* a) {
  return (sp_mem_arena_marker_t) {
    .arena = a,
    .block = a->current,
    .mark = a->current->bytes_used
  };
}

void sp_mem_arena_pop(sp_mem_arena_marker_t marker) {
  marker.block->bytes_used = marker.mark;
  marker.arena->current = marker.block;
}

sp_mem_arena_t* sp_mem_get_scratch_arena() {
  sp_tls_rt_t* runtime = sp_tls_rt_get();
  return runtime->scratch;
}

sp_mem_scratch_t sp_mem_begin_scratch() {
  sp_tls_rt_t* runtime = sp_tls_rt_get();
  sp_context_t* ctx = sp_context_get();

  sp_mem_scratch_t scratch = {
    .marker = sp_mem_arena_mark(runtime->scratch),
    .old_allocator = ctx->allocator
  };
  sp_context_push_allocator(sp_mem_arena_as_allocator(runtime->scratch));
  return scratch;
}

void sp_mem_end_scratch(sp_mem_scratch_t scratch) {
  sp_context_pop();
  sp_mem_arena_pop(scratch.marker);
}

sp_mem_libc_metadata_t* sp_mem_libc_get_metadata(void* ptr) {
  return ((sp_mem_libc_metadata_t*)ptr) - 1;
}

void* sp_mem_libc_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u32 size, void* ptr) {
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      u32 total_size = size + sizeof(sp_mem_libc_metadata_t);
      sp_mem_libc_metadata_t* metadata = (sp_mem_libc_metadata_t*)sp_mem_os_alloc_zero(total_size);
      metadata->size = size;
      return metadata + 1;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      if (!ptr) {
        return sp_mem_libc_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      }

      sp_mem_libc_metadata_t* metadata = sp_mem_libc_get_metadata(ptr);
      if (metadata->size >= size) {
        return ptr;
      }

      void* buffer = sp_mem_libc_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      sp_mem_copy(ptr, buffer, metadata->size);
      sp_mem_libc_on_alloc(user_data, SP_ALLOCATOR_MODE_FREE, 0, ptr);

      return buffer;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      sp_mem_libc_metadata_t* metadata = sp_mem_libc_get_metadata(ptr);
      sp_mem_os_free(metadata);
      return NULL;
    }
    default: {
      return NULL;
    }
  }
}

sp_allocator_t sp_mem_libc_new() {
  sp_allocator_t allocator;
  allocator.on_alloc = sp_mem_libc_on_alloc;
  allocator.user_data = NULL;
  return allocator;
}

bool sp_mem_is_equal(const void* a, const void* b, size_t len) {
  return !memcmp(a, b, len);
}

void sp_mem_copy(const void* source, void* dest, u32 num_bytes) {
  memcpy(dest, source, num_bytes);
}

void sp_mem_move(const void* source, void* dest, u32 num_bytes) {
  memmove(dest, source, num_bytes);
}

void sp_mem_fill(void* buffer, u32 buffer_size, void* fill, u32 fill_size) {
  u8* current_byte = (u8*)buffer;

  s32 i = 0;
  while (true) {
    if (i + fill_size > buffer_size) return;
    memcpy(current_byte + i, (u8*)fill, fill_size);
    i += fill_size;
  }
}

void sp_mem_fill_u8(void* buffer, u32 buffer_size, u8 fill) {
  sp_mem_fill(buffer, buffer_size, &fill, sizeof(u8));
}

void sp_mem_zero(void* buffer, u32 buffer_size) {
  sp_mem_fill_u8(buffer, buffer_size, 0);
}

#if defined(SP_WIN32)
void* sp_mem_os_alloc(u32 size) {
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void* sp_mem_os_realloc(void* ptr, u32 size) {
  if (!ptr) return sp_mem_os_alloc(size);
  if (!size) { HeapFree(GetProcessHeap(), 0, ptr); return NULL; }
  return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

void sp_mem_os_free(void* ptr) {
  if (!ptr) return;
  HeapFree(GetProcessHeap(), 0, ptr);
}
#elif defined(SP_POSIX)
void* sp_mem_os_alloc(u32 size) {
  return malloc(size);
}

void* sp_mem_os_alloc_zero(u32 size) {
  return calloc(size, 1);
}

void* sp_mem_os_realloc(void* ptr, u32 size) {
  return realloc(ptr, size);
}

void sp_mem_os_free(void* ptr) {
  free(ptr);
}
#endif

// ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗
// ██╔════╝╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝
// ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗
// ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║
// ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
// @string
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
  sp_require(str);
  sp_require(buffer);
  sp_require(buffer_length);

  // @bad
  u32 copy_length = SP_MIN(length, buffer_length - 1);
  for (u32 i = 0; i < copy_length; i++) {
    buffer[i] = str[i];
  }
  buffer[copy_length] = '\0';
}

bool sp_cstr_equal(const c8* a, const c8* b) {
  u32 la = sp_cstr_len(a);
  u32 lb = sp_cstr_len(b);
  if (la != lb) return false;

  return sp_mem_is_equal(a, b, la);
}

u32 sp_cstr_len(const c8* str) {
  sp_require_as(str, 0);

  u32 len = 0;
  while (str[len]) len++;
  return len;
}

u32 sp_cstr_len_sized(const c8* str, u32 n) {
  sp_require_as(str, 0);

  u32 len = 0;
  while (len < n && str[len]) len++;
  return len;
}

c8* sp_wstr_to_cstr(c16* str16, u32 len) {
  return sp_os_wstr_to_cstr(str16, len);
}

c8 sp_c8_to_lower(c8 c) {
  return (c >= 'A' && c <= 'Z') ? (c + 32) : (c);
}

c8 sp_c8_to_upper(c8 c) {
  return (c >= 'a' && c <= 'z') ? (c - 32) : (c);
}

u8 sp_utf8_num_bytes_from_byte(u8 byte) {
  if (byte < SP_UTF8_2_BYTE_MIN) return 1;
  if ((byte & 0xE0) == 0xC0) return 2;
  if ((byte & 0xF0) == 0xE0) return 3;
  if ((byte & 0xF8) == 0xF0) return 4;
  return 0;
}

u8 sp_utf8_num_bytes_from_ptr(const u8* ptr) {
  if (!ptr) return 0;
  return sp_utf8_num_bytes_from_byte(*ptr);
}

u8 sp_utf8_num_bytes_from_codepoint(u32 codepoint) {
  if (codepoint < SP_UTF8_2_BYTE_MIN) return 1;
  if (codepoint < SP_UTF8_3_BYTE_MIN) return 2;
  if (codepoint < SP_UTF8_4_BYTE_MIN) return 3;
  if (codepoint <= SP_UTF8_MAX)       return 4;
  return 0;
}

bool sp_utf8_is_cont(u8 b) {
  return (b & 0xC0) == 0x80;
}

bool sp_utf8_is_surrogate(u32 codepoint) {
  return (codepoint >= SP_UTF8_SURROGATE_MIN) && (codepoint <= SP_UTF8_SURROGATE_MAX);
}

bool sp_utf8_is_bounds_ok(u32 codepoint, u8 len) {
  if (codepoint > SP_UTF8_MAX) return false;
  switch (len) {
    case 2: { return (codepoint >= SP_UTF8_2_BYTE_MIN); }
    case 3: { return (codepoint >= SP_UTF8_3_BYTE_MIN); }
    case 4: { return (codepoint >= SP_UTF8_4_BYTE_MIN); }
  }

  return true;
}

bool sp_utf8_is_byte_ascii(u8 b) {
  return b < SP_UTF8_ASCII_MAX;
}

bool sp_utf8_is_codepoint_ascii(u32 codepoint) {
  return codepoint < SP_UTF8_ASCII_MAX;
}

u32 sp_utf8_to_lower(u32 codepoint) {
  if (!sp_utf8_is_codepoint_ascii(codepoint)) return codepoint;
  return sp_c8_to_lower((c8)codepoint);
}

u32 sp_utf8_to_upper(u32 codepoint) {
  if (!sp_utf8_is_codepoint_ascii(codepoint)) return codepoint;
  return sp_c8_to_upper((c8)codepoint);
}


u32 sp_utf8_mask(u8 byte, u8 mask, u8 shift) {
  return (u32)((byte & mask) << shift);
}

u32 sp_utf8_decode(const u8* ptr) {
  sp_require_as(ptr, SP_UTF8_REPLACEMENT);
  sp_require_as(!sp_utf8_is_byte_ascii(ptr[0]), ptr[0]);

  switch (sp_utf8_num_bytes_from_byte(ptr[0])) {
    case 2: return sp_utf8_mask(ptr[0], 0x1F, 6)  | sp_utf8_mask(ptr[1], 0x3F, 0);
    case 3: return sp_utf8_mask(ptr[0], 0x0F, 12) | sp_utf8_mask(ptr[1], 0x3F, 6)  | sp_utf8_mask(ptr[2], 0x3F, 0);
    case 4: return sp_utf8_mask(ptr[0], 0x07, 18) | sp_utf8_mask(ptr[1], 0x3F, 12) | sp_utf8_mask(ptr[2], 0x3F, 6) | sp_utf8_mask(ptr[3], 0x3f, 0);
    default: return SP_UTF8_REPLACEMENT;
  }

  #undef sp_mask
}

u8 sp_utf8_encode(u32 cp, u8* out) {
  sp_require_as(out, 0);

  u8 len = sp_utf8_num_bytes_from_codepoint(cp);
  switch (len) {
    case 1: { out[0] = (u8)cp; break; }
    case 2: { out[0] = 0xC0 | (cp >> 6); out[1] = 0x80 | (cp & 0x3F); break; }
    case 3: { out[0] = 0xE0 | (cp >> 12); out[1] = 0x80 | ((cp >> 6) & 0x3F); out[2] = 0x80 | (cp & 0x3F); break; }
    case 4: { out[0] = 0xF0 | (cp >> 18); out[1] = 0x80 | ((cp >> 12) & 0x3F); out[2] = 0x80 | ((cp >> 6) & 0x3F); out[3] = 0x80 | (cp & 0x3F); break; }
  }
  return len;
}

sp_utf8_it_t sp_utf8_it(sp_str_t str) {
  sp_require_as(!sp_str_empty(str), sp_zero_struct(sp_utf8_it_t));

  return (sp_utf8_it_t) {
    .str = { .data = (const u8*)str.data, .len = str.len },
    .index = 0,
    .codepoint = sp_utf8_decode((const u8*)str.data),
    .codepoint_len = sp_utf8_num_bytes_from_byte((u8)str.data[0]),
  };
}

sp_utf8_it_t sp_utf8_rit(sp_str_t str) {
  // @spader
  // the cast is objectively wrong here. i just use s32 in the iterator to give me a sentinel when reverse
  // iterating. which in and of itself is stupid. double down.
  sp_require_as(!sp_str_empty(str), sp_zero_struct(sp_utf8_it_t));
  sp_utf8_it_t it = {
    .str = { .data = (const u8*)str.data, .len = str.len },
    .index = (s32)str.len
  };
  sp_utf8_it_prev(&it);
  return it;
}

bool sp_utf8_it_valid(sp_utf8_it_t* it) {
  sp_require_as(it, false);
  return (it->index < (s32)it->str.len) && (it->index >= 0) && (it->codepoint_len);
}

void sp_utf8_it_next(sp_utf8_it_t* it) {
  sp_require(it);

  it->index += it->codepoint_len;
  if (it->index >= (s32)it->str.len) {
    return;
  }

  it->codepoint = sp_utf8_decode(it->str.data + it->index);
  it->codepoint_len = sp_utf8_num_bytes_from_byte(it->str.data[it->index]);
}

void sp_utf8_it_prev(sp_utf8_it_t* it) {
  sp_require(it);

  while (true) {
    it->index--;
    if (it->index < 0) return;
    if (!sp_utf8_is_cont(it->str.data[it->index])) break;
  }

  it->codepoint = sp_utf8_decode(it->str.data + it->index);
  it->codepoint_len = sp_utf8_num_bytes_from_byte(it->str.data[it->index]);
}

bool sp_utf8_validate(sp_str_t str) {
  const u8* ptr = (const u8*)str.data;
  u32 it = 0;

  while (it < str.len) {
    u8 len = sp_utf8_num_bytes_from_byte(ptr[it]);
    if (!len) return false;
    if (it + len > str.len) return false;

    sp_for_range(j, 1, len) {
      if (!sp_utf8_is_cont(ptr[it + j])) {
        return false;
      }
    }

    u32 codepoint = sp_utf8_decode(&ptr[it]);
    if (sp_utf8_is_surrogate(codepoint)) return false;
    if (!sp_utf8_is_bounds_ok(codepoint, len)) return false;

    it += len;
  }
  return true;
}

u32 sp_utf8_num_codepoints(sp_str_t str) {
  u32 count = 0;
  sp_str_for_utf8(str, it) {
    count++;
  }
  return count;
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
    .data = (c8*)sp_alloc(capacity),
    .len = 0,
  };
}

sp_str_t sp_str_view(const c8* cstr) {
  return (sp_str_t) {
    .data = cstr,
    .len = sp_cstr_len(cstr),
  };
}

bool sp_str_empty(sp_str_t str) {
  return !str.len;
}

bool sp_str_equal(sp_str_t a, sp_str_t b) {
  if (a.len != b.len) return false;

  return sp_mem_is_equal(a.data, b.data, a.len);
}

bool sp_str_equal_cstr(sp_str_t a, const c8* b) {
  u32 len = sp_cstr_len(b);
  if (a.len != len) return false;

  return sp_mem_is_equal(a.data, b, len);
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
    if (sp_mem_is_equal(str.data + i, needle.data, needle.len)) {
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

sp_str_t sp_str_prefix(sp_str_t str, s32 len) {
  return sp_str_sub(str, 0, len);
}

sp_str_t sp_str_suffix(sp_str_t str, s32 len) {
  return sp_str_sub(str, str.len - len, len);
}

sp_str_t sp_str_sub(sp_str_t str, s32 index, s32 len) {
  sp_str_t substr = sp_str(str.data + index, (u32)len);
  SP_ASSERT(index + len <= (s32)str.len);
  return substr;
}

sp_str_t sp_str_sub_reverse(sp_str_t str, s32 index, s32 len) {
  sp_str_t substr = {
    .data = str.data + str.len + index - len,
    .len = (u32)len,
  };
  return substr;
}

sp_str_t sp_str_from_cstr_sized(const c8* str, u32 length) {
  c8* buffer = (c8*)sp_alloc(length);
  u32 len = sp_cstr_len(str);
  len = SP_MIN(len, length);
  sp_mem_copy(str, buffer, len);

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr_null(const c8* str) {
  u32 len = sp_cstr_len(str);
  c8* buffer = sp_alloc_n(c8, len + 1);
  sp_mem_copy(str, buffer, len);
  buffer[len] = 0;

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr(const c8* str) {
  u32 len = sp_cstr_len(str);
  c8* buffer = sp_alloc_n(c8, len + 1);
  sp_mem_copy(str, buffer, len);

  return sp_str(buffer, len);
}

sp_str_t sp_str_copy(sp_str_t str) {
  c8* buffer = sp_alloc_n(c8, str.len);
  sp_mem_copy(str.data, buffer, str.len);

  return SP_STR(buffer, str.len);
}

void sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity) {
  sp_mem_copy(str.data, buffer, SP_MIN(str.len, capacity));
}

sp_str_t sp_str_null_terminate(sp_str_t str) {
  c8* buffer = (c8*)sp_alloc(str.len + 1);
  sp_mem_copy(str.data, buffer, str.len);
  buffer[str.len] = 0;
  return SP_STR(buffer, str.len);
}

sp_str_it_t sp_str_it(sp_str_t str) {
  return (sp_str_it_t) {
    .str = str,
    .index = 0,
    .c = str.len ? str.data[0] : '\0'
  };
}

bool sp_str_it_valid(sp_str_it_t* it) {
  return it->index < it->str.len;
}

void sp_str_it_next(sp_str_it_t* it) {
  it->index++;
  it->c = sp_str_it_valid(it) ? it->str.data[it->index] : 0;
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
  sp_mem_copy(str.data, builder->buffer.data + builder->buffer.len, str.len);
  builder->buffer.len += str.len;
}

void sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str) {
  sp_str_builder_append(builder, sp_str_view(str));
}

void sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c) {
  sp_str_builder_append(builder, sp_str(&c, 1));
}

void sp_str_builder_append_utf8(sp_str_builder_t* builder, u32 codepoint) {
  u8 buf[4] = SP_ZERO_INITIALIZE();
  u8 len = sp_utf8_encode(codepoint, buf);
  sp_assert(len);

  sp_str_builder_append(builder, sp_str(buf, len));
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
  for (s32 index = 0; index < delta; index++) {
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

sp_str_t sp_str_trim_left(sp_str_t str) {
  sp_str_t trimmed = str;

  sp_str_for_it(str, it) {
    switch (it.c) {
      case ' ':
      case '\t':
      case '\r':
      case '\n': {
        trimmed.data++; trimmed.len--;
        break;
      }
      default: {
        return trimmed;
      }
    }
  }

  return trimmed;
}

sp_str_t sp_str_trim(sp_str_t str) {
  sp_str_t trimmed = str;
  trimmed = sp_str_trim_left(trimmed);
  trimmed = sp_str_trim_right(trimmed);
  return trimmed;
}

sp_str_t sp_str_strip_left(sp_str_t str, sp_str_t strip) {
  if (!sp_str_starts_with(str, strip)) return str;
  return sp_str_suffix(str, str.len - strip.len);
}

sp_str_t sp_str_strip_right(sp_str_t str, sp_str_t strip) {
  if (!sp_str_ends_with(str, strip)) return str;
  return sp_str_prefix(str, str.len - strip.len);
}

sp_str_t sp_str_strip(sp_str_t str, sp_str_t strip) {
  sp_str_t result = sp_str_strip_left(str, strip);
  return sp_str_strip_right(result, strip);
}

sp_str_t sp_str_to_upper(sp_str_t str) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_str_for_utf8(str, it) {
    sp_str_builder_append_utf8(&builder, sp_utf8_to_upper(it.codepoint));
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_str_t sp_str_to_lower(sp_str_t str) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_str_for_utf8(str, it) {
    sp_str_builder_append_utf8(&builder, sp_utf8_to_lower(it.codepoint));
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_str_t sp_str_to_pascal_case(sp_str_t str) {
  if (sp_str_empty(str)) return sp_str_lit("");

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  bool word = true;

  sp_str_for_utf8(str, it) {
    switch (it.codepoint) {
      case ' ':
      case '\t':
      case '\n':
      case '\r': {
        sp_str_builder_append_utf8(&builder, it.codepoint);
        word = true;
        break;
      }
      default: {
        sp_str_builder_append_utf8(&builder, word ?
          sp_utf8_to_upper(it.codepoint) :
          sp_utf8_to_lower(it.codepoint)
        );
        word = false;
        break;
      }
    }
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_write(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_str_t sp_str_truncate(sp_str_t str, u32 max_len, sp_str_t trailer) {
  sp_require_as(max_len, sp_str_copy(str));
  sp_require_as(str.len > max_len, sp_str_copy(str));
  sp_require_as(trailer.len <= max_len, sp_str_copy(str));

  return sp_str_concat(sp_str_prefix(str, max_len - trailer.len), trailer);
}

sp_dyn_array(sp_str_t) sp_str_map(sp_str_t* strs, u32 num_strs, sp_opaque_ptr user_data, sp_str_map_fn_t fn) {
  sp_dyn_array(sp_str_t) results = SP_NULLPTR;

  sp_for(it, num_strs) {
    sp_str_map_context_t context = {
      .str = strs[it],
      .user_data = user_data
    };
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
  return sp_str_trim(context->str);
}

sp_str_t sp_str_map_kernel_to_upper(sp_str_map_context_t* context) {
  return sp_str_to_upper(context->str);
}

sp_str_t sp_str_map_kernel_to_lower(sp_str_map_context_t* context) {
  return sp_str_to_lower(context->str);
}

sp_str_t sp_str_map_kernel_pascal_case(sp_str_map_context_t* context) {
  return sp_str_to_pascal_case(context->str);
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
      if (sp_mem_is_equal(context->str.data + i, data->needle.data, data->needle.len)) {
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


// ███████╗██╗██╗     ███████╗███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗
// ██╔════╝██║██║     ██╔════╝██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║
// █████╗  ██║██║     █████╗  ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║
// ██╔══╝  ██║██║     ██╔══╝  ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║
// ██║     ██║███████╗███████╗███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║
// ╚═╝     ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝
// @filesystem @fs
sp_str_t sp_fs_normalize_path(sp_str_t path) {
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

void sp_fs_normalize_path_soft(sp_str_t* path) {
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

sp_str_t sp_fs_get_name(sp_str_t path) {
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

sp_str_t sp_fs_parent_path(sp_str_t path) {
  if (path.len == 0) return path;

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
    path.len = 0;
  } else {
    path.len = 0;
  }

  sp_str_t result = sp_str_copy(path);
  sp_fs_normalize_path_soft(&result);
  return result;
}

sp_str_t sp_fs_join_path(sp_str_t a, sp_str_t b) {
  // Remove trailing slash from first path to avoid double slash
  sp_str_t a_copy = a;
  if (a_copy.len > 0 && (a_copy.data[a_copy.len - 1] == '/' || a_copy.data[a_copy.len - 1] == '\\')) {
    a_copy.len--;
  }

  sp_str_t result = sp_str_join(a_copy, b, SP_LIT("/"));
  sp_fs_normalize_path_soft(&result);
  return result;
}

sp_str_t sp_fs_get_ext(sp_str_t path) {
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

sp_str_t sp_fs_get_stem(sp_str_t path) {
  sp_str_t file_name = sp_fs_get_name(path);
  if (!file_name.len) return path;

  sp_str_t extension = sp_fs_get_ext(path);

  sp_str_t stem = {
    .data = file_name.data,
    .len = file_name.len - extension.len,
  };

  while (true) {
    if (!stem.len) break;
    if (sp_str_back(stem) != '.') break;

    stem.len--;
  }

  return stem;
}

sp_err_t sp_fs_link(sp_str_t from, sp_str_t to, sp_os_link_kind_t kind) {
  switch (kind) {
    case SP_FS_LINK_HARD:     return sp_fs_create_hard_link(from, to);
    case SP_FS_LINK_SYMBOLIC: return sp_fs_create_sym_link(from, to);
    case SP_FS_LINK_COPY:     return sp_fs_copy(from, to);
  }

  SP_UNREACHABLE_RETURN(SP_ERR_OK);
}

#if defined(SP_WIN32)
SP_PRIVATE sp_os_file_attr_t sp_fs_winapi_attr_to_sp_attr(u32 attr);

bool sp_fs_exists(sp_str_t path) {
  sp_win32_dword_t attributes = GetFileAttributesA(sp_str_to_cstr(path));
  return (attributes != INVALID_FILE_ATTRIBUTES);
}

bool sp_fs_is_regular_file(sp_str_t path) {
  sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
  if (attribute == INVALID_FILE_ATTRIBUTES) return false;
  return !(attribute & FILE_ATTRIBUTE_DIRECTORY);
}

bool sp_fs_is_dir(sp_str_t path) {
  sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
  if (attribute == INVALID_FILE_ATTRIBUTES) return false;
  return attribute & FILE_ATTRIBUTE_DIRECTORY;
}

bool sp_fs_is_symlink(sp_str_t path) {
  sp_win32_dword_t attr = GetFileAttributesA(sp_str_to_cstr(path));
  if (attr == INVALID_FILE_ATTRIBUTES) return false;
  return (attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

bool sp_fs_is_target_regular_file(sp_str_t path) {
  sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
  if (attribute == INVALID_FILE_ATTRIBUTES) return false;
  return !(attribute & FILE_ATTRIBUTE_DIRECTORY);
}

bool sp_fs_is_target_dir(sp_str_t path) {
  sp_win32_dword_t attribute = GetFileAttributesA(sp_str_to_cstr(path));
  if (attribute == INVALID_FILE_ATTRIBUTES) return false;
  return attribute & FILE_ATTRIBUTE_DIRECTORY;
}

bool sp_fs_is_root(sp_str_t path) {
  if (path.len == 0) return true;
  if (path.len == 1 && path.data[0] == '/') return true;
  if (path.len == 2 && path.data[1] == ':') return true;
  if (path.len == 3 && path.data[1] == ':' && (path.data[2] == '/' || path.data[2] == '\\')) return true;
  return false;
}

void sp_fs_remove_dir(sp_str_t path) {
  SHFILEOPSTRUCTA file_op = SP_ZERO_INITIALIZE();
  file_op.wFunc = FO_DELETE;
  file_op.pFrom = sp_str_to_double_null_terminated(path);
  file_op.fFlags = FOF_NO_UI;
  SHFileOperationA(&file_op);
}

void sp_os_create_dir(sp_str_t path) {
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

  if (sp_fs_is_root(normalized)) return;

  sp_str_t parent = sp_os_parent_path(normalized);
  if (parent.len > 0 && !sp_str_equal(parent, normalized)) {
    sp_os_create_directory(parent);
    CreateDirectoryA(path_cstr, NULL);
  }
}

void sp_fs_create_file(sp_str_t path) {
  sp_win32_handle_t handle = CreateFileA(sp_str_to_cstr(path), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  CloseHandle(handle);
}

void sp_fs_remove_file(sp_str_t path) {
  sp_win32_dword_t attr = GetFileAttributesA(sp_str_to_cstr(path));
  if (attr != INVALID_FILE_ATTRIBUTES &&
      (attr & FILE_ATTRIBUTE_REPARSE_POINT) &&
      (attr & FILE_ATTRIBUTE_DIRECTORY)) {
    RemoveDirectoryA(sp_str_to_cstr(path));
  } else {
    DeleteFileA(sp_str_to_cstr(path));
  }
}

bool sp_fs_is_glob(sp_str_t path) {
  sp_str_t file_name = sp_os_extract_file_name(path);
  return sp_str_contains_n(&file_name, 1, sp_str_lit("*"));
}

bool sp_fs_is_on_path(sp_str_t program) {
  sp_ps_config_t config = SP_ZERO_INITIALIZE();
  config.command = SP_LIT("where");
  sp_ps_config_add_arg(&config, program);
  config.io.out.mode = SP_PS_IO_MODE_NULL;
  config.io.err.mode = SP_PS_IO_MODE_NULL;

  sp_ps_output_t output = sp_ps_run(config);

  return output.status.exit_code == 0;
}

sp_err_t sp_fs_copy(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_glob(from)) {
    sp_fs_copy_glob(sp_os_parent_path(from), sp_os_extract_file_name(from), to);
  }
  else if (sp_fs_is_target_dir(from)) {
    SP_ASSERT(sp_fs_is_target_dir(to));
    sp_fs_copy_glob(from, sp_str_lit("*"), sp_os_join_path(to, sp_os_extract_file_name(from)));
  }
  else if (sp_fs_is_target_regular_file(from)) {
    sp_fs_copy_file(from, to);
  }

  return SP_ERR_OK;
}

void sp_fs_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to) {
  sp_fs_create_dir(to);

  sp_da(sp_fs_dir_ent_t) entries = sp_fs_collect(from);

  for (u32 i = 0; i < entries.count; i++) {
    sp_fs_dir_ent_t* entry = &entries.data[i];
    sp_str_t entry_name = entry->file_name;

    bool matches = sp_str_equal(glob, sp_str_lit("*"));
    if (!matches) {
      matches = sp_str_equal(entry_name, glob);
    }

    if (matches) {
      sp_str_t entry_path = sp_fs_join_path(from, entry_name);
      sp_os_copy(entry_path, to);
    }
  }
}

void sp_fs_copy_file(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    sp_fs_create_dir(to);
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  sp_win32_dword_t src_attrs = GetFileAttributesA(sp_str_to_cstr(from));

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

  if (src_attrs != INVALID_FILE_ATTRIBUTES) {
    SetFileAttributesA(sp_str_to_cstr(to), src_attrs);
  }
}

void sp_fs_copy_dir(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  sp_fs_copy_glob(from, sp_str_lit("*"), to);
}

sp_da(sp_fs_dir_ent_t) sp_fs_collect(sp_str_t path) {
  if (!sp_fs_is_dir(path) || !sp_fs_exists(path)) {
    return SP_NULLPTR;
  }

  sp_dyn_array(sp_fs_dir_ent_t) entries = SP_NULLPTR;

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
    sp_fs_normalize_path(file_path);

    sp_gs_dir_ent_t entry = SP_RVAL(sp_fs_dir_ent_t) {
      .file_path = file_path,
      .file_name = sp_str_from_cstr(find_data.cFileName),
      .attributes = sp_fs_get_file_attrs(file_path),
    };
    sp_dyn_array_push(entries, entry);
  } while (FindNextFile(handle, &find_data));

  FindClose(handle);

  return entries;
}

sp_fs_file_attr_t sp_fs_get_file_attrs(sp_str_t path) {
  return sp_fs_winapi_attr_to_sp_attr(GetFileAttributesA(sp_str_to_cstr(path)));
}

sp_fs_file_attr_t sp_fs_winapi_attr_to_sp_attr(u32 attr) {
  if (attr == INVALID_FILE_ATTRIBUTES) return SP_OS_FILE_ATTR_NONE;
  if (attr & FILE_ATTRIBUTE_REPARSE_POINT) return SP_OS_FILE_ATTR_SYMLINK;
  if (attr & FILE_ATTRIBUTE_DIRECTORY) return SP_OS_FILE_ATTR_DIRECTORY;
  return SP_OS_FILE_ATTR_REGULAR_FILE;
}

sp_str_t sp_fs_get_exe_path() {
  c8 exe_path[SP_MAX_PATH_LEN];
  GetModuleFileNameA(NULL, exe_path, SP_MAX_PATH_LEN);

  sp_str_t exe_path_str = SP_CSTR(exe_path);
  sp_fs_normalize_path(exe_path_str);

  return sp_str_copy(exe_path_str);
}

sp_str_t sp_fs_canonicalize_path(sp_str_t path) {
  c8* path_cstr = sp_str_to_cstr(path);
  c8 canonical_path[SP_MAX_PATH_LEN];

  if (GetFullPathNameA(path_cstr, SP_MAX_PATH_LEN, canonical_path, NULL) == 0) {
    return sp_str_copy(path);
  }

  sp_str_t result = sp_str_from_cstr(canonical_path);
  result = sp_fs_normalize_path(result);

  // Remove trailing slash if present
  if (result.len > 0 && result.data[result.len - 1] == '/') {
    result.len--;
  }

  return sp_str_copy(result);
}
#endif

#if defined(SP_POSIX)
sp_str_t sp_fs_get_cwd() {
  c8 path [PATH_MAX];
  if (!getcwd(path, PATH_MAX - 1)) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  sp_str_t cwd = sp_str_from_cstr(path);
  return sp_fs_normalize_path(cwd);
}

bool sp_fs_exists(sp_str_t path) {
  struct stat st;
  c8* path_cstr = sp_str_to_cstr(path);
  s32 result = stat(path_cstr, &st);
  return result == 0;
}

bool sp_fs_is_regular_file(sp_str_t path) {
  struct stat st;
  c8* path_cstr = sp_str_to_cstr(path);
  s32 result = lstat(path_cstr, &st);
  if (result != 0) return false;
  return S_ISREG(st.st_mode);
}

bool sp_fs_is_symlink(sp_str_t path) {
  struct stat st;
  c8* path_cstr = sp_str_to_cstr(path);
  s32 result = lstat(path_cstr, &st);
  if (result != 0) return false;
  return S_ISLNK(st.st_mode);
}

bool sp_fs_is_dir(sp_str_t path) {
  struct stat st;
  c8* path_cstr = sp_str_to_cstr(path);
  s32 result = lstat(path_cstr, &st);
  if (result != 0) return false;
  return S_ISDIR(st.st_mode);
}

bool sp_fs_is_target_regular_file(sp_str_t path) {
  struct stat st;
  if (stat(sp_str_to_cstr(path), &st) != 0) return false;
  return S_ISREG(st.st_mode);
}

bool sp_fs_is_target_dir(sp_str_t path) {
  struct stat st;
  if (stat(sp_str_to_cstr(path), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

bool sp_fs_is_root(sp_str_t path) {
  if (sp_str_empty(path)) return true;
  if (sp_str_equal_cstr(path, "/")) return true;

  return false;
}

void sp_fs_remove_dir(sp_str_t path) {
  sp_da(sp_os_dir_ent_t) entries = sp_fs_collect(path);

  sp_dyn_array_for(entries, i) {
    sp_os_dir_ent_t* entry = &entries[i];

    if (sp_fs_is_symlink(entry->file_path)) {
      sp_fs_remove_file(entry->file_path);
    } else if (sp_fs_is_dir(entry->file_path)) {
      sp_fs_remove_dir(entry->file_path);
    } else if (sp_fs_is_regular_file(entry->file_path)) {
      sp_fs_remove_file(entry->file_path);
    }
  }

  c8* path_cstr = sp_str_to_cstr(path);
  rmdir(path_cstr);
}

void sp_fs_create_dir(sp_str_t path) {
  sp_fs_normalize_path_soft(&path);
  c8* path_cstr = sp_str_to_cstr(path);

  if (sp_str_empty(path)) return;
  if (sp_fs_is_root(path)) return;
  if (sp_fs_exists(path)) return;

  s32 result = mkdir(path_cstr, 0755);
  if (!result) return;

  sp_fs_create_dir(sp_fs_parent_path(path));

  result = mkdir(path_cstr, 0755);
  if (!result) return;
}

void sp_fs_create_file(sp_str_t path) {
  c8* path_cstr = sp_str_to_cstr(path);
  s32 fd = open(path_cstr, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd >= 0) close(fd);
}

void sp_fs_remove_file(sp_str_t path) {
  c8* path_cstr = sp_str_to_cstr(path);
  unlink(path_cstr);
}

bool sp_fs_is_glob(sp_str_t path) {
  SP_INCOMPLETE()
  sp_str_t file_name = sp_fs_get_name(path);
  return sp_str_contains_n(&file_name, 1, sp_str_lit("*"));
}

bool sp_fs_is_on_path(sp_str_t program) {
  SP_INCOMPLETE()
  sp_ps_config_t config = SP_ZERO_INITIALIZE();
  config.command = SP_LIT("which");
  sp_ps_config_add_arg(&config, program);
  config.io.out.mode = SP_PS_IO_MODE_NULL;
  config.io.err.mode = SP_PS_IO_MODE_NULL;

  sp_ps_output_t output = sp_ps_run(config);

  return output.status.exit_code == 0;
}

sp_err_t sp_fs_copy(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_glob(from)) {
    sp_fs_copy_glob(sp_fs_parent_path(from), sp_fs_get_name(from), to);
  }
  else if (sp_fs_is_target_dir(from)) {
    SP_ASSERT(sp_fs_is_target_dir(to));
    sp_fs_copy_glob(from, sp_str_lit("*"), sp_fs_join_path(to, sp_fs_get_name(from)));
  }
  else if (sp_fs_is_target_regular_file(from)) {
    sp_fs_copy_file(from, to);
  }

  return SP_ERR_OK;
}

void sp_fs_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to) {
  sp_fs_create_dir(to);

  sp_da(sp_os_dir_ent_t) entries = sp_fs_collect(from);

  sp_dyn_array_for(entries, i) {
    sp_os_dir_ent_t* entry = &entries[i];
    sp_str_t entry_name = entry->file_name;

    bool matches = sp_str_equal(glob, sp_str_lit("*"));
    if (!matches) {
      matches = sp_str_equal(entry_name, glob);
    }

    if (matches) {
      sp_str_t entry_path = sp_fs_join_path(from, entry_name);
      sp_fs_copy(entry_path, to);
    }
  }
}

void sp_fs_copy_file(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    sp_fs_create_dir(to);
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  struct stat stf;
  if (stat(sp_str_to_cstr(from), &stf)) return;

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

  chmod(sp_str_to_cstr(to), stf.st_mode);
}

void sp_fs_copy_dir(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  sp_fs_copy_glob(from, sp_str_lit("*"), to);
}

sp_da(sp_os_dir_ent_t) sp_fs_collect(sp_str_t path) {
  if (!sp_fs_is_dir(path) || !sp_fs_exists(path)) {
    return SP_NULLPTR;
  }

  sp_dyn_array(sp_os_dir_ent_t) entries = SP_NULLPTR;

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
    sp_fs_normalize_path(file_path);

    sp_os_dir_ent_t dir_ent = {
      .file_path = file_path,
      .file_name = sp_str_from_cstr(entry->d_name),
      .attributes = sp_fs_get_file_attrs(file_path),
    };
    sp_dyn_array_push(entries, dir_ent);
  }

  closedir(dir);

  return entries;
}

sp_da(sp_os_dir_ent_t) sp_fs_collect_recursive(sp_str_t path) {
  if (!sp_fs_is_dir(path) || !sp_fs_exists(path)) {
    return SP_NULLPTR;
  }

  sp_rb(sp_str_t) queue = SP_NULLPTR;
  sp_da(sp_os_dir_ent_t) results = SP_NULLPTR;

  sp_rb_push(queue, path);

  while (!sp_rb_empty(queue)) {
    sp_str_t current = *sp_rb_peek(queue);
    sp_rb_pop(queue);

    sp_da(sp_os_dir_ent_t) entries = sp_fs_collect(current);
    sp_dyn_array_for(entries, i) {
      sp_os_dir_ent_t* entry = &entries[i];
      sp_dyn_array_push(results, *entry);
      if (entry->attributes == SP_OS_FILE_ATTR_DIRECTORY) {
        sp_rb_push(queue, entry->file_path);
      }
    }
  }

  sp_rb_free(queue);
  return results;
}

sp_str_t sp_fs_canonicalize_path(sp_str_t path) {
  sp_str_t result = path;
  c8 canonical_path[SP_MAX_PATH_LEN] = SP_ZERO_INITIALIZE();
  if (sp_fs_exists(path)) {
    c8* path_cstr = sp_str_to_cstr(path);
    if (realpath(path_cstr, canonical_path)) {
      result = SP_CSTR(canonical_path);
    }
  }

  result = sp_fs_normalize_path(result);

  if (result.len > 0 && result.data[result.len - 1] == '/') {
    result.len--;
  }

  return sp_str_copy(result);
}


sp_os_file_attr_t sp_fs_get_file_attrs(sp_str_t path) {
  struct stat st;
  if (lstat(sp_str_to_cstr(path), &st) == 0) {
    if (S_ISLNK(st.st_mode)) {
      return SP_OS_FILE_ATTR_SYMLINK;
    } else if (S_ISDIR(st.st_mode)) {
      return SP_OS_FILE_ATTR_DIRECTORY;
    } else if (S_ISREG(st.st_mode)) {
      return SP_OS_FILE_ATTR_REGULAR_FILE;
    }
  }
  return SP_OS_FILE_ATTR_NONE;
}

sp_err_t sp_fs_create_hard_link(sp_str_t target, sp_str_t link_path) {
  if (link(sp_str_to_cstr(target), sp_str_to_cstr(link_path))) {
    return SP_ERR_OS;
  }
  return SP_ERR_OK;
}

sp_err_t sp_fs_create_sym_link(sp_str_t target, sp_str_t link_path) {
  if (symlink(sp_str_to_cstr(target), sp_str_to_cstr(link_path)) != 0) {
    return SP_ERR_OS;
  }
  return SP_ERR_OK;
}

sp_str_t sp_os_try_xdg_or_home(sp_str_t xdg, sp_str_t home_suffix) {
  sp_str_t path =  sp_os_get_env_var(xdg);
  if (sp_str_valid(path)) return path;

  path = sp_os_get_env_var(SP_LIT("HOME"));
  if (sp_str_valid(path)) return sp_fs_join_path(path, home_suffix);

  return SP_ZERO_STRUCT(sp_str_t);
}

sp_str_t sp_fs_get_storage_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_DATA_HOME"), SP_LIT(".local/share"));
}

sp_str_t sp_fs_get_config_path() {
  return sp_os_try_xdg_or_home(SP_LIT("XDG_CONFIG_HOME"), SP_LIT(".config"));
}
#endif

#if defined(SP_COSMO)
extern char* program_invocation_name;

sp_str_t sp_fs_get_exe_path() {
  c8 exe_path [PATH_MAX] = SP_ZERO_INITIALIZE();
  if (realpath(program_invocation_name, exe_path)) {
    return sp_str_copy(sp_fs_parent_path(SP_CSTR(exe_path)));
  }
  return sp_str_lit("");
}
#endif

#if defined(SP_LINUX)
sp_str_t sp_fs_get_exe_path() {
  c8 exe_path [PATH_MAX];
  s64 len = readlink("/proc/self/exe", exe_path, PATH_MAX - 1);
  if (len < 0) {
    return sp_str_lit("");
  }

  sp_str_t file_path = {
    .data = exe_path,
    .len = (u32)len,
  };

  return sp_str_copy(sp_fs_parent_path(file_path));
}
#endif

#if defined(SP_MACOS)
sp_str_t sp_fs_get_exe_path() {
  c8 exe_path[PATH_MAX];
  u32 size = PATH_MAX;
  if (_NSGetExecutablePath(exe_path, &size) == 0) {
    c8 real_path[PATH_MAX];
    if (realpath(exe_path, real_path)) {
      return sp_str_copy(sp_fs_parent_path(SP_CSTR(real_path)));
    }
  }
  return sp_str_lit("");
}
#endif


//  ██████╗ ███████╗
// ██╔═══██╗██╔════╝
// ██║   ██║███████╗
// ██║   ██║╚════██║
// ╚██████╔╝███████║
//  ╚═════╝ ╚══════╝
//  @os
#if defined(SP_WIN32)
void sp_os_sleep_ns(u64 ns) {
  // Windows stub: just use millisecond Sleep
  Sleep((DWORD)(ns / 1000000));
}

void sp_os_sleep_ms(f64 ms) {
  sp_os_sleep_ns((u64)(ms * 1000000.0));
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


#elif defined(SP_POSIX)
void sp_os_sleep_ns(u64 ns) {
  struct timespec remaining = {
    .tv_sec = (time_t)(ns / 1000000000ULL),
    .tv_nsec = (long)(ns % 1000000000ULL)
  };
  int was_error;
  do {
    errno = 0;
    was_error = nanosleep(&remaining, &remaining);
  } while (was_error == -1 && errno == EINTR);
}

void sp_os_sleep_ms(f64 ms) {
  sp_os_sleep_ns((u64)sp_tm_ms_to_ns_f(ms));
}

void sp_sleep_ns(u64 target) {
  const u64 coarse = sp_tm_ms_to_ns(1);
  u64 fine = coarse;

  u64 elapsed = 0;
  sp_tm_timer_t timer = sp_tm_start_timer();

  // coarse sleep until we get pretty close
  while (elapsed + fine < target) {
    u64 before = sp_tm_read_timer(&timer);
    sp_os_sleep_ns(coarse);
    elapsed = sp_tm_read_timer(&timer);
    fine = SP_MAX(fine, elapsed - before);
  }

  // fine sleep until we get really close
  // guard against elapsed > target (can happen with debugger pauses or OS overshoots)
  if (elapsed < target) {
    u64 margin = fine - coarse;
    u64 remaining = target - elapsed;
    if (remaining > margin) {
      sp_os_sleep_ns(remaining - margin);
      elapsed = sp_tm_read_timer(&timer);
    }
  }

  // spin until we get so god damn close we can taste it
  while (elapsed < target) {
    sp_spin_pause();
    elapsed = sp_tm_read_timer(&timer);
  }
}

void sp_sleep_ms(f64 ms) {
  sp_sleep_ns((u64)sp_tm_ms_to_ns_f(ms));
}


c8* sp_os_wstr_to_cstr(c16* str16, u32 len) {
  if (!str16 || len == 0) {
    c8* result = (c8*)sp_alloc(1);
    result[0] = '\0';
    return result;
  }

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

void sp_os_print(sp_str_t message) {
  write(STDOUT_FILENO, message.data, message.len);
}
#endif

#if defined(SP_MACOS)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_MACOS;
}

sp_str_t sp_os_get_name() {
  return sp_str_lit("macos");
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
#endif

#if defined(SP_COSMO)
sp_os_kind_t sp_os_get_kind() {
  if (IsLinux()) {
    return SP_OS_LINUX;
  }
  else if (IsWindows()) {
    return SP_OS_WIN32;
  }
  else if (IsXnu()) {
    return SP_OS_MACOS;
  }

  SP_UNREACHABLE_RETURN(SP_OS_LINUX);
}

sp_str_t sp_os_get_name() {
  switch (sp_os_get_kind()) {
    case SP_OS_LINUX: return sp_str_lit("linux");
    case SP_OS_WIN32: return sp_str_lit("windows");
    case SP_OS_MACOS: return sp_str_lit("macos");
  }

  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

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
#endif

#if defined(SP_LINUX)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_LINUX;
}

sp_str_t sp_os_get_name() {
  return sp_str_lit("linux");
}

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
#endif

// ████████╗██╗███╗   ███╗███████╗
// ╚══██╔══╝██║████╗ ████║██╔════╝
//    ██║   ██║██╔████╔██║█████╗
//    ██║   ██║██║╚██╔╝██║██╔══╝
//    ██║   ██║██║ ╚═╝ ██║███████╗
//    ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝
#if defined(SP_WIN32)
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

sp_str_t sp_tm_epoch_to_iso8601(sp_tm_epoch_t time) {
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

  u64 seconds = counter.QuadPart / freq.QuadPart;
  u64 remainder = counter.QuadPart % freq.QuadPart;
  u64 ns = seconds * 1000000000ULL + (remainder * 1000000000ULL) / freq.QuadPart;

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

sp_tm_epoch_t sp_fs_get_mod_time(sp_str_t file_path) {
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

#elif defined(SP_POSIX)
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

sp_str_t sp_tm_epoch_to_iso8601(sp_tm_epoch_t time) {
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
  return sp_tm_point_diff(current, timer->start);
}

u64 sp_tm_lap_timer(sp_tm_timer_t* timer) {
  sp_tm_point_t current = sp_tm_now_point();
  u64 elapsed = sp_tm_point_diff(current, timer->previous);
  timer->previous = current;
  return elapsed;
}

void sp_tm_reset_timer(sp_tm_timer_t* timer) {
  sp_tm_point_t now = sp_tm_now_point();
  timer->start = now;
  timer->previous = now;
}
#endif

#define SP_TM_S_TO_MS  1000ULL
#define SP_TM_S_TO_US  1000000ULL
#define SP_TM_S_TO_NS  1000000000ULL
#define SP_TM_MS_TO_US 1000ULL
#define SP_TM_MS_TO_NS 1000000ULL
#define SP_TM_US_TO_NS 1000ULL

#define SP_TM_S_TO_MS_MAX  (SP_LIMIT_U64_MAX / SP_TM_S_TO_MS)
#define SP_TM_S_TO_US_MAX  (SP_LIMIT_U64_MAX / SP_TM_S_TO_US)
#define SP_TM_S_TO_NS_MAX  (SP_LIMIT_U64_MAX / SP_TM_S_TO_NS)
#define SP_TM_MS_TO_US_MAX (SP_LIMIT_U64_MAX / SP_TM_MS_TO_US)
#define SP_TM_MS_TO_NS_MAX (SP_LIMIT_U64_MAX / SP_TM_MS_TO_NS)
#define SP_TM_US_TO_NS_MAX (SP_LIMIT_U64_MAX / SP_TM_US_TO_NS)

u64 sp_tm_s_to_ms(u64 s) {
  SP_ASSERT(s <= SP_TM_S_TO_MS_MAX);
  return s * SP_TM_S_TO_MS;
}

u64 sp_tm_s_to_us(u64 s) {
  SP_ASSERT(s <= SP_TM_S_TO_US_MAX);
  return s * SP_TM_S_TO_US;
}

u64 sp_tm_s_to_ns(u64 s) {
  SP_ASSERT(s <= SP_TM_S_TO_NS_MAX);
  return s * SP_TM_S_TO_NS;
}

u64 sp_tm_ms_to_s(u64 ms) {
  return ms / SP_TM_S_TO_MS;
}

u64 sp_tm_ms_to_us(u64 ms) {
  SP_ASSERT(ms <= SP_TM_MS_TO_US_MAX);
  return ms * SP_TM_MS_TO_US;
}

u64 sp_tm_ms_to_ns(u64 ms) {
  SP_ASSERT(ms <= SP_TM_MS_TO_NS_MAX);
  return ms * SP_TM_MS_TO_NS;
}

u64 sp_tm_us_to_s(u64 us) {
  return us / SP_TM_S_TO_US;
}

u64 sp_tm_us_to_ms(u64 us) {
  return us / SP_TM_MS_TO_US;
}

u64 sp_tm_us_to_ns(u64 us) {
  SP_ASSERT(us <= SP_TM_US_TO_NS_MAX);
  return us * SP_TM_US_TO_NS;
}

u64 sp_tm_ns_to_s(u64 ns) {
  return ns / SP_TM_S_TO_NS;
}

u64 sp_tm_ns_to_ms(u64 ns) {
  return ns / SP_TM_MS_TO_NS;
}

u64 sp_tm_ns_to_us(u64 ns) {
  return ns / SP_TM_US_TO_NS;
}

f64 sp_tm_s_to_ms_f(f64 s) {
  return s * (f64)SP_TM_S_TO_MS;
}

f64 sp_tm_s_to_us_f(f64 s) {
  return s * (f64)SP_TM_S_TO_US;
}

f64 sp_tm_s_to_ns_f(f64 s) {
  return s * (f64)SP_TM_S_TO_NS;
}

f64 sp_tm_ms_to_s_f(f64 ms) {
  return ms / (f64)SP_TM_S_TO_MS;
}

f64 sp_tm_ms_to_us_f(f64 ms) {
  return ms * (f64)SP_TM_MS_TO_US;
}

f64 sp_tm_ms_to_ns_f(f64 ms) {
  return ms * (f64)SP_TM_MS_TO_NS;
}

f64 sp_tm_us_to_s_f(f64 us) {
  return us / (f64)SP_TM_S_TO_US;
}

f64 sp_tm_us_to_ms_f(f64 us) {
  return us / (f64)SP_TM_MS_TO_US;
}

f64 sp_tm_us_to_ns_f(f64 us) {
  return us * (f64)SP_TM_US_TO_NS;
}

f64 sp_tm_ns_to_s_f(f64 ns) {
  return ns / (f64)SP_TM_S_TO_NS;
}

f64 sp_tm_ns_to_ms_f(f64 ns) {
  return ns / (f64)SP_TM_MS_TO_NS;
}

f64 sp_tm_ns_to_us_f(f64 ns) {
  return ns / (f64)SP_TM_US_TO_NS;
}

u64 sp_tm_fps_to_ns(u64 fps) {
  f64 s = (1.0) / (f64)(fps);
  f64 ns = sp_tm_s_to_ns_f(s);
  return (u64)ns;
}

#if defined(SP_MACOS)
  sp_tm_epoch_t sp_fs_get_mod_time(sp_str_t file_path) {
    struct stat st;
    c8* path_cstr = sp_str_to_cstr(file_path);
    s32 result = stat(path_cstr, &st);

    if (result != 0) {
      return SP_ZERO_STRUCT(sp_tm_epoch_t);
    }

    return SP_RVAL(sp_tm_epoch_t) {
      .s = (u64)st.st_mtime,
      .ns = (u32)st.st_mtimespec.tv_nsec
    };
  }
#endif

#if defined(SP_LINUX) || defined(SP_COSMO)
sp_tm_epoch_t sp_fs_get_mod_time(sp_str_t file_path) {
  struct stat st;
  c8* path_cstr = sp_str_to_cstr(file_path);
  s32 result = stat(path_cstr, &st);

  if (result != 0) {
    return SP_ZERO_STRUCT(sp_tm_epoch_t);
  }

  return SP_RVAL(sp_tm_epoch_t) {
    .s = (u64)st.st_mtime,
    .ns = (u32)st.st_mtim.tv_nsec
  };
}
#endif


// ███████╗██████╗ ██╗███╗   ██╗
// ██╔════╝██╔══██╗██║████╗  ██║
// ███████╗██████╔╝██║██╔██╗ ██║
// ╚════██║██╔═══╝ ██║██║╚██╗██║
// ███████║██║     ██║██║ ╚████║
// ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝
// @spin
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
    sp_mutex_lock(&sp_rt.mutex);

    if (*lock == 0) {
      *lock = 1;
      sp_mutex_unlock(&sp_rt.mutex);
      return true;
    }
    else {
      sp_mutex_unlock(&sp_rt.mutex);
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
    sp_mutex_lock(&sp_rt.mutex);
    *lock = 0;
    sp_mutex_unlock(&sp_rt.mutex);
  #endif
}

//  █████╗ ████████╗ ██████╗ ███╗   ███╗██╗ ██████╗
// ██╔══██╗╚══██╔══╝██╔═══██╗████╗ ████║██║██╔════╝
// ███████║   ██║   ██║   ██║██╔████╔██║██║██║
// ██╔══██║   ██║   ██║   ██║██║╚██╔╝██║██║██║
// ██║  ██║   ██║   ╚██████╔╝██║ ╚═╝ ██║██║╚██████╗
// ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝     ═╝╚═╝ ╚═════╝
// @atomic
bool sp_atomic_s32_cmp_and_swap(sp_atomic_s32* value, s32 current, s32 desired) {
  #if defined(SP_MSVC)
    return _InterlockedCompareExchange(&value, desired, current) == current;
  #elif defined(SP_GNUISH)
    return __sync_bool_compare_and_swap(value, current, desired);
  #else
    bool result = false;
    size_t index = ((((size_t)value) >> 3) & 0x1f);
    sp_spin_lock(&sp_rt.locks[index]);
    if (*value == current) {
      *value = desired;
      result = true;
    }
    sp_spin_unlock(&sp_rt.locks[index]);
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
// ███████╗███████╗███╗   ███╗ █████╗ ██████╗ ██╗  ██╗ ██████╗ ██████╗ ███████╗
// ██╔════╝██╔════╝████╗ ████║██╔══██╗██╔══██╗██║  ██║██╔═══██╗██╔══██╗██╔════╝
// ███████╗█████╗  ██╔████╔██║███████║██████╔╝███████║██║   ██║██████╔╝█████╗
// ╚════██║██╔══╝  ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔══██║██║   ██║██╔══██╗██╔══╝
// ███████║███████╗██║ ╚═╝ ██║██║  ██║██║     ██║  ██║╚██████╔╝██║  ██║███████╗
// ╚══════╝╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
// @semaphore
#if defined(SP_WIN32)
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
#elif defined(SP_MACOS)
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
#elif defined(SP_POSIX)
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
#endif

// ███╗   ███╗██╗   ██╗████████╗███████╗██╗  ██╗
// ████╗ ████║██║   ██║╚══██╔══╝██╔════╝╚██╗██╔╝
// ██╔████╔██║██║   ██║   ██║   █████╗   ╚███╔╝
// ██║╚██╔╝██║██║   ██║   ██║   ██╔══╝   ██╔██╗
// ██║ ╚═╝ ██║╚██████╔╝   ██║   ███████╗██╔╝ ██╗
// ╚═╝     ╚═╝ ╚═════╝    ╚╝   ╚══════╝╚═╝   ╚═╝
// #mutex
#if defined(SP_WIN32)
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
#elif defined(SP_POSIX)
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
#endif

#if defined(SP_POSIX)
void sp_cv_init(sp_cv_t* cond) {
  pthread_cond_init(cond, NULL);
}

void sp_cv_destroy(sp_cv_t* cond) {
  pthread_cond_destroy(cond);
}

void sp_cv_wait(sp_cv_t* cond, sp_mutex_t* mutex) {
  pthread_cond_wait(cond, mutex);
}

bool sp_cv_wait_for(sp_cv_t* cond, sp_mutex_t* mutex, u32 ms) {
  sp_tm_epoch_t now = sp_tm_now_epoch();

  struct timespec ts = {
    .tv_sec = (time_t)(now.s + (ms / 1000)),
    .tv_nsec = now.ns + ((ms % 1000) * 1000000),
  };

  if (ts.tv_nsec >= 1000000000) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000;
  }

  return pthread_cond_timedwait(cond, mutex, &ts) == 0;
}

void sp_cv_notify_one(sp_cv_t* cond) {
  pthread_cond_signal(cond);
}

void sp_cv_notify_all(sp_cv_t* cond) {
  pthread_cond_broadcast(cond);
}

#elif defined(SP_WIN32)
void sp_cv_init(sp_cv_t* cond) {
  InitializeConditionVariable(cond);
}

void sp_cv_destroy(sp_cv_t* cond) {
  SP_UNUSED(cond);
}

void sp_cv_wait(sp_cv_t* cond, sp_mutex_t* mutex) {
  SleepConditionVariableCS(cond, mutex, INFINITE);
}

bool sp_cv_wait_for(sp_cv_t* cond, sp_mutex_t* mutex, u32 ms) {
  return SleepConditionVariableCS(cond, mutex, (DWORD)ms) != 0;
}

void sp_cv_notify_one(sp_cv_t* cond) {
  WakeConditionVariable(cond);
}

void sp_cv_notify_all(sp_cv_t* cond) {
  WakeAllConditionVariable(cond);
}
#endif

// ███████╗██╗   ██╗████████╗██╗   ██╗██████╗ ███████╗
// ██╔════╝██║   ██║╚══██╔══╝██║   ██║██╔══██╗██╔════╝
// █████╗  ██║   ██║   ██║   ██║   ██║██████╔╝█████╗
// ██╔══╝  ██║   ██║   ██║   ██║   ██║██╔══██╗██╔══╝
// ██║     ╚██████╔╝   ██║   ╚██████╔╝██║  ██║███████╗
// ╚═╝      ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝
sp_future_t* sp_future_create(u32 size) {
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
  sp_mem_copy(value, future->value, future->size);
  sp_atomic_s32_set(&future->ready, 1);
}


// ████████╗██╗  ██╗██████╗ ███████╗ █████╗ ██████╗
// ╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗██╔══██╗
//    ██║   ███████║██████╔╝█████╗  ███████║██║  ██║
//    ██║   ██╔══██║██╔══██╗██╔══╝  ██╔══██║██║  ██║
//    ██║   ██║  ██║██║  ██║███████╗██║  ██║██████╔╝
//    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝
// @thread
#if defined(SP_WIN32)
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
#elif defined(SP_POSIX)
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
#endif


// ███████╗███╗   ██╗██╗   ██╗
// ██╔════╝████╗  ██║██║   ██║
// █████╗  ██╔██╗ ██║██║   ██║
// ██╔══╝  ██║╚██╗██║╚██╗ ██╔╝
// ███████╗██║ ╚████║ ╚████╔╝
// ╚══════╝╚═╝  ╚═══╝  ╚═══╝
// @env
#if defined(SP_POSIX)
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
  return sp_fs_normalize_path(path);
}

void sp_os_clear_env_var(sp_str_t key) {
  unsetenv(sp_str_to_cstr(key));
}

void sp_os_export_env(sp_env_t* env, sp_env_export_t overwrite) {
  sp_ht_for(env->vars, it) {
    sp_os_export_env_var(*sp_ht_it_getkp(env->vars, it), *sp_ht_it_getp(env->vars, it), overwrite);
  }
}

void sp_os_export_env_var(sp_str_t key, sp_str_t value, sp_env_export_t overwrite) {
  const c8* k = sp_str_to_cstr(key);
  const c8* v = sp_str_to_cstr(value);
  setenv(k, v, overwrite);
}
#endif

// ██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗
// ██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝
// ██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗
// ██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║
// ██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝
#if defined(SP_POSIX)
SP_PRIVATE void sp_ps_set_cwd(posix_spawn_file_actions_t* fa, sp_str_t cwd);
SP_PRIVATE bool sp_ps_create_pipes(s32 pipes [2]);
SP_PRIVATE sp_da(c8*) sp_ps_build_posix_args(sp_ps_config_t* config);
SP_PRIVATE void sp_ps_free_posix_args(c8** argv);
SP_PRIVATE c8** sp_ps_build_posix_env(sp_ps_env_config_t* env_config);
SP_PRIVATE void sp_ps_set_nonblocking(s32 fd);
SP_PRIVATE void sp_ps_set_blocking(s32 fd);


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

sp_da(c8*) sp_ps_build_posix_args(sp_ps_config_t* config) {
  sp_da(c8*) args = SP_NULLPTR;

  sp_dyn_array_push(args, sp_str_to_cstr(config->command));

  sp_carr_for(config->args, it) {
    sp_str_t arg = config->args[it];
    if (sp_str_empty(arg)) break;

    sp_dyn_array_push(args, sp_str_to_cstr(arg));
  }

  sp_dyn_array_for(config->dyn_args, it) {
    sp_dyn_array_push(args, sp_str_to_cstr(config->dyn_args[it]));
  }

  sp_dyn_array_push(args, SP_NULLPTR);
  return args;
}

void sp_ps_free_posix_args(c8** args) {
  sp_dyn_array_free(args);
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

  // Copy dynamic args
  sp_dyn_array_for(src->dyn_args, i) {
    sp_dyn_array_push(dst.dyn_args, sp_str_copy(src->dyn_args[i]));
  }

  dst.env.mode = src->env.mode;

  sp_env_table_t ht = src->env.env.vars;
  for (sp_ht_it_t it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
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
  SP_ASSERT(config);

  if (!sp_str_empty(arg)) {
    sp_dyn_array_push(config->dyn_args, arg);
  }
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
  if (config.io.out.mode == SP_PS_IO_MODE_EXISTING || config.io.out.mode == SP_PS_IO_MODE_REDIRECT) {
    SP_FATAL(
      "You called sp_ps_run() but your config redirected stdout. sp_ps_run() always creates a new "
      "file descriptor for stdout, since its purpose is to run a command and capture output without "
      "the command polluting the parent command's output. If you really wanted this, use sp_ps_create() "
      "and sp_ps_output(). The failing command was {:fg brightyellow}",
      SP_FMT_STR(config.command)
    );
  }
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


// ███████╗███╗   ███╗ ██████╗ ███╗   ██╗
// ██╔════╝████╗ ████║██╔═══██╗████╗  ██║
// █████╗  ██╔████╔██║██║   ██║██╔██╗ ██║
// ██╔══╝  ██║╚██╔╝██║██║   ██║██║╚██╗██║
// ██║     ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║
// ╚═╝     ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
// @fmon
SP_PRIVATE void             sp_fmon_os_init(sp_fmon_t* monitor);
SP_PRIVATE void             sp_fmon_os_deinit(sp_fmon_t* monitor);
SP_PRIVATE void             sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_PRIVATE void             sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_PRIVATE void             sp_fmon_os_process_changes(sp_fmon_t* monitor);

void sp_fmon_init(sp_fmon_t* monitor, sp_fmon_fn_t callback, sp_fmon_event_kind_t events, void* userdata) {
  sp_fmon_init_ex(monitor, callback, events, userdata, 0);
}

void sp_fmon_init_ex(sp_fmon_t* monitor, sp_fmon_fn_t callback, sp_fmon_event_kind_t events, void* userdata, u32 debounce_ms) {
  monitor->callback = callback;
  monitor->events_to_watch = events;
  monitor->userdata = userdata;
  monitor->debounce_time_ms = debounce_ms;
  sp_fmon_os_init(monitor);
}

void sp_fmon_deinit(sp_fmon_t* monitor) {
  sp_fmon_os_deinit(monitor);
}

void sp_fmon_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  sp_fmon_os_add_dir(monitor, path);
}

void sp_fmon_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_fmon_os_add_file(monitor, file_path);
}

void sp_fmon_process_changes(sp_fmon_t* monitor) {
  sp_fmon_os_process_changes(monitor);
}

void sp_fmon_emit_changes(sp_fmon_t* monitor) {
  sp_da_for(monitor->changes, it) {
    sp_fmon_event_t* change = &monitor->changes[it];
    monitor->callback(monitor, change, monitor->userdata);
  }

  sp_da_clear(monitor->changes);
}

sp_fmon_cache_t* sp_fmon_find_cache_entry(sp_fmon_t* monitor, sp_str_t file_path) {
  c8* file_path_cstr = sp_str_to_cstr(file_path);
  sp_hash_t file_hash = sp_hash_cstr(file_path_cstr);

  sp_fmon_cache_t* found = NULL;
  for (u32 i = 0; i < sp_da_size(monitor->cache); i++) {
    sp_fmon_cache_t* entry = &monitor->cache[i];
    if (entry->hash == file_hash) {
      found = entry;
      break;
    }
  }

  if (!found) {
    sp_da_push(monitor->cache, SP_ZERO_STRUCT(sp_fmon_cache_t));
    found = &monitor->cache[sp_da_size(monitor->cache) - 1];
    found->hash = file_hash;
  }

  return found;
}

#ifdef SP_WIN32
SP_PRIVATE void sp_os_win32_file_monitor_add_change(sp_fmon_t* monitor, sp_str_t file_path, sp_str_t file_name, sp_fmon_event_t events);
SP_PRIVATE void sp_os_win32_file_monitor_issue_one_read(sp_fmon_t * monitor, sp_fmon_dir_t* info);

void sp_fmon_os_init(sp_file_monitor_t* monitor) {
  sp_os_win32_file_monitor_t* os = (sp_os_win32_file_monitor_t*)sp_alloc(sizeof(sp_os_win32_file_monitor_t));
  sp_dynamic_array_init(&os->directory_infos, sizeof(sp_monitored_dir_t));
  monitor->os = os;
}

void sp_fmon_os_deinit(sp_file_monitor_t* monitor) {
  (void)monitor;
}

void sp_fmon_os_add_dir(sp_file_monitor_t* monitor, sp_str_t directory_path) {
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

void sp_fmon_os_add_file(sp_file_monitor_t* monitor, sp_str_t file_path) {
}

void sp_fmon_os_process_changes(sp_file_monitor_t* monitor) {
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

  sp_fmon_emit_changes(monitor);
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

#elif defined(SP_LINUX)
void sp_fmon_os_init(sp_fmon_t* monitor) {
  sp_fmon_os_t* linux_monitor = SP_ALLOC(sp_fmon_os_t);

  linux_monitor->fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (linux_monitor->fd == -1) {
    // Handle error but don't crash
    linux_monitor->fd = 0;
  }

  monitor->os = linux_monitor;
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  if (!monitor->os) return;
  sp_fmon_os_t* linux_monitor = (sp_fmon_os_t*)monitor->os;

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
    sp_dyn_array_push(linux_monitor->watch_descs, wd);
    sp_dyn_array_push(linux_monitor->watch_paths, sp_str_copy(path));
  }

}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  if (os->fd > 0) {
    close(os->fd);
  }
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_str_t dir_path = sp_fs_parent_path(file_path);
  if (dir_path.len > 0) {
    sp_fmon_os_add_dir(monitor, dir_path);
  }
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  if (!monitor->os) return;

  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  if (os->fd <= 0) return;

  size_t len = read(os->fd, os->buffer, sizeof(os->buffer));
  if (len <= 0) return;

  // Process all events in buffer
  char* ptr = (char*)os->buffer;
  while (ptr < (char*)os->buffer + len) {
    struct inotify_event* event = (struct inotify_event*)ptr;

    // Find which path this watch descriptor corresponds to
    sp_dyn_array_for(os->watch_descs, it) {
      s32 wd = os->watch_descs[it];
      if (wd == event->wd) {
        sp_str_t dir_path = os->watch_paths[it];

        // Build full path if there's a filename
        sp_str_t file_name = SP_ZERO_STRUCT(sp_str_t);
        sp_str_t file_path = SP_ZERO_STRUCT(sp_str_t);

        if (event->len > 0 && event->name[0] != '\0') {
          file_name = sp_str(event->name, sp_cstr_len(event->name));

          // Build full path
          sp_str_builder_t builder = SP_ZERO_INITIALIZE();
          sp_str_builder_append(&builder, dir_path);
          sp_str_builder_append(&builder, sp_str_lit("/"));
          sp_str_builder_append(&builder, file_name);
          file_path = sp_str_builder_write(&builder);
        } else {
          file_path = sp_str_copy(dir_path);
          file_name = sp_fs_get_name(file_path);
        }

        // Convert inotify mask to our events
        sp_fmon_event_kind_t events = SP_FILE_CHANGE_EVENT_NONE;
        if (event->mask & (IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_MODIFIED);
        }
        if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_ADDED);
        }
        if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_REMOVED);
        }

        // Add change to monitor's change list
        if (events != SP_FILE_CHANGE_EVENT_NONE) {
          sp_fmon_event_t change = {
            .file_path = file_path,
            .file_name = file_name,
            .events = events,
            .time = 0  // TODO: get actual time
          };
          sp_dyn_array_push(monitor->changes, change);
        }
        break;
      }
    }

    ptr += sizeof(struct inotify_event) + event->len;
  }

  // Emit changes with debouncing
  sp_fmon_emit_changes(monitor);
}

#elif defined(SP_MACOS)

#if defined(SP_FMON_MACOS_USE_FSEVENTS)
SP_PRIVATE void sp_fmon_fsevents_callback(
    ConstFSEventStreamRef stream,
    void* user_data,
    size_t num_events,
    void* event_paths,
    const FSEventStreamEventFlags event_flags[],
    const FSEventStreamEventId event_ids[]) {
  (void)stream;
  (void)event_ids;

  sp_fmon_t* monitor = (sp_fmon_t*)user_data;
  sp_fmon_os_t* os = monitor->os;
  c8** paths = (c8**)event_paths;

  sp_mutex_lock(&os->mutex);
  sp_context_push_allocator(sp_mem_arena_as_allocator(os->event_arena));

  sp_for(it, num_events) {
    sp_str_t file_path = sp_str_view(paths[it]);

    bool watching_file = false;
    sp_da_for(os->watch_files, wf_it) {
      if (sp_str_equal(os->watch_files[wf_it], file_path)) {
        watching_file = true;
        break;
      }
    }

    if (sp_da_size(os->watch_files) > 0 && !watching_file) {
      continue;
    }

    sp_fmon_event_kind_t kind = SP_FILE_CHANGE_EVENT_NONE;

    if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_ADDED) &&
        (event_flags[it] & kFSEventStreamEventFlagItemCreated)) {
      kind = (sp_fmon_event_kind_t)(kind | SP_FILE_CHANGE_EVENT_ADDED);
    }
    if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) &&
        (event_flags[it] & kFSEventStreamEventFlagItemModified)) {
      kind = (sp_fmon_event_kind_t)(kind | SP_FILE_CHANGE_EVENT_MODIFIED);
    }
    if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) &&
        (event_flags[it] & kFSEventStreamEventFlagItemRemoved)) {
      kind = (sp_fmon_event_kind_t)(kind | SP_FILE_CHANGE_EVENT_REMOVED);
    }

    if (kind != SP_FILE_CHANGE_EVENT_NONE) {
      sp_fmon_event_t change = {
        .file_path = sp_str_copy(file_path),
        .file_name = sp_fs_get_name(file_path),
        .events = kind,
      };
      sp_da_push(monitor->changes, change);
    }
  }

  sp_context_pop();
  sp_mutex_unlock(&os->mutex);
}

void sp_fmon_os_destroy_stream(sp_fmon_os_t* os) {
  FSEventStreamStop(os->stream);
  FSEventStreamInvalidate(os->stream);
  dispatch_sync(os->queue, ^{});
  FSEventStreamRelease(os->stream);
  os->stream = SP_NULLPTR;
}

SP_PRIVATE void sp_fmon_fsevents_recreate_stream(sp_fmon_t* monitor) {
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  CFArrayRef paths = SP_ZERO_INITIALIZE();

  u32 num_paths = sp_da_size(os->watch_paths);
  if (!num_paths) return;

  if (os->stream) {
    sp_fmon_os_destroy_stream(os);
  }

  {
    sp_mem_scratch_t scratch = sp_mem_begin_scratch();
    sp_mutex_lock(&os->mutex);

    CFStringRef* cf_paths = sp_alloc_n(CFStringRef, num_paths);
    sp_da_for(os->watch_paths, it) {
      cf_paths[it] = CFStringCreateWithCString(
        kCFAllocatorDefault,
        sp_str_to_cstr(os->watch_paths[it]),
        kCFStringEncodingUTF8
      );
    }

    paths = CFArrayCreate(NULL, (const void**)cf_paths, num_paths, &kCFTypeArrayCallBacks);

    sp_for(it, num_paths) {
      CFRelease(cf_paths[it]);
    }

    sp_mutex_unlock(&os->mutex);
    sp_mem_end_scratch(scratch);
  }

  FSEventStreamContext context = {
    .version = 0,
    .info = monitor,
    .retain = NULL,
    .release = NULL,
    .copyDescription = NULL
  };

  os->stream = FSEventStreamCreate(
    kCFAllocatorDefault,
    sp_fmon_fsevents_callback,
    &context,
    paths,
    kFSEventStreamEventIdSinceNow,
    0.0,
    kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer
  );

  CFRelease(paths);

  if (os->stream) {
    FSEventStreamSetDispatchQueue(os->stream, os->queue);
    FSEventStreamStart(os->stream);
  }
}

void sp_fmon_os_init(sp_fmon_t* monitor) {
  sp_fmon_os_t* os = sp_alloc_type(sp_fmon_os_t);
  os->queue = dispatch_queue_create("sp.fmon", DISPATCH_QUEUE_SERIAL);
  os->monitor = monitor;
  sp_mutex_init(&os->mutex, SP_MUTEX_PLAIN);
  os->watch_arena = sp_mem_arena_new(SP_FMON_ARENA_SIZE);
  os->event_arena = sp_mem_arena_new(SP_FMON_ARENA_SIZE);
  monitor->os = os;
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  sp_require(monitor);
  sp_require(monitor->os);

  sp_fmon_os_t* os = monitor->os;

  if (os->stream) {
    sp_fmon_os_destroy_stream(os);
  }

  if (os->queue) {
    dispatch_release(os->queue);
  }

  sp_mutex_destroy(&os->mutex);
  sp_mem_arena_destroy(os->watch_arena);
  sp_mem_arena_destroy(os->event_arena);
  *os = sp_zero_struct(sp_fmon_os_t);
}

void sp_fmon_os_push_dir(sp_fmon_os_t* os, sp_str_t dir) {
  sp_mutex_lock(&os->mutex);
  sp_context_push_allocator(sp_mem_arena_as_allocator(os->watch_arena));
  sp_da_push(os->watch_paths, sp_str_copy(dir));
  sp_context_pop();
  sp_mutex_unlock(&os->mutex);
}

void sp_fmon_os_push_file(sp_fmon_os_t* os, sp_str_t file) {
  sp_mutex_lock(&os->mutex);
  sp_context_push_allocator(sp_mem_arena_as_allocator(os->watch_arena));
  sp_da_push(os->watch_files, sp_fs_canonicalize_path(file));
  sp_context_pop();
  sp_mutex_unlock(&os->mutex);
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  sp_require(monitor);
  sp_require(monitor->os);

  sp_fmon_os_push_dir(monitor->os, path);
  sp_fmon_fsevents_recreate_stream(monitor);
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t path) {
  sp_require(monitor);
  sp_require(monitor->os);

  sp_fmon_os_t* os = monitor->os;

  sp_fmon_os_push_file(os, path);

  {
    sp_mem_scratch_t scratch = sp_mem_begin_scratch();

    sp_str_t dir = sp_fs_parent_path(sp_fs_canonicalize_path(path));

    bool found = false;
    sp_da_for(os->watch_paths, it) {
      if (sp_str_equal(os->watch_paths[it], dir)) {
        found = true;
        break;
      }
    }

    if (!found) {
      sp_fmon_os_push_dir(os, dir);
      sp_fmon_fsevents_recreate_stream(monitor);
    }

    sp_mem_end_scratch(scratch);
  }
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  sp_require(monitor);
  sp_require(monitor->os);
  sp_fmon_os_t* os = monitor->os;

  dispatch_sync(os->queue, ^{});

  sp_mutex_lock(&os->mutex);
  sp_fmon_emit_changes(monitor);
  sp_mem_arena_clear(os->event_arena);
  sp_mutex_unlock(&os->mutex);
}

#else

#include <sys/event.h>
#include <fcntl.h>

void sp_fmon_os_init(sp_fmon_t* monitor) {
  sp_fmon_os_t* os = SP_ALLOC(sp_fmon_os_t);

  os->kq = kqueue();
  if (os->kq == -1) {
    os->kq = 0;
  }

  monitor->os = os;
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;

  sp_da_for(os->fds, it) {
    close(os->fds[it]);
  }
  if (os->kq > 0) {
    close(os->kq);
  }
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;

  if (os->kq <= 0) return;

  s32 fd = open(sp_str_to_cstr(path), O_RDONLY | O_EVTONLY);
  if (fd == -1) return;

  u32 fflags = 0;
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) {
    fflags |= NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB;
  }
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_ADDED) {
    fflags |= NOTE_WRITE | NOTE_LINK;
  }
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) {
    fflags |= NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE;
  }

  struct kevent change;
  EV_SET(&change, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, fflags, 0, NULL);

  if (kevent(os->kq, &change, 1, NULL, 0, NULL) != -1) {
    sp_da_push(os->fds, fd);
    sp_da_push(os->watch_paths, sp_str_copy(path));
  } else {
    close(fd);
  }
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t path) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;

  if (os->kq <= 0) return;

  s32 fd = open(sp_str_to_cstr(path), O_RDONLY | O_EVTONLY);
  if (fd == -1) return;

  u32 fflags = 0;
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) {
    fflags |= NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB;
  }
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) {
    fflags |= NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE;
  }

  struct kevent change;
  EV_SET(&change, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, fflags, 0, NULL);

  if (kevent(os->kq, &change, 1, NULL, 0, NULL) != -1) {
    sp_da_push(os->fds, fd);
    sp_da_push(os->watch_paths, sp_str_copy(path));
  } else {
    close(fd);
  }
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  if (!monitor->os) return;

  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  if (os->kq <= 0) return;

  struct kevent events[16];
  struct timespec timeout = {0, 0};

  s32 nev = kevent(os->kq, NULL, 0, events, 16, &timeout);
  if (nev <= 0) return;

  for (s32 i = 0; i < nev; i++) {
    struct kevent* ev = &events[i];
    s32 fd = (s32)ev->ident;

    sp_da_for(os->fds, it) {
      if (os->fds[it] == fd) {
        sp_str_t path = os->watch_paths[it];

        sp_fmon_event_kind_t event_kind = SP_FILE_CHANGE_EVENT_NONE;
        if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) &&
            (ev->fflags & (NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB))) {
          event_kind = (sp_fmon_event_kind_t)(event_kind | SP_FILE_CHANGE_EVENT_MODIFIED);
        }
        if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_ADDED) &&
            (ev->fflags & NOTE_LINK)) {
          event_kind = (sp_fmon_event_kind_t)(event_kind | SP_FILE_CHANGE_EVENT_ADDED);
        }
        if ((monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) &&
            (ev->fflags & (NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE))) {
          event_kind = (sp_fmon_event_kind_t)(event_kind | SP_FILE_CHANGE_EVENT_REMOVED);
        }

        if (event_kind != SP_FILE_CHANGE_EVENT_NONE) {
          sp_fmon_event_t change = {
            .file_path = sp_str_copy(path),
            .file_name = sp_fs_get_name(path),
            .events = event_kind,
            .time = 0
          };
          sp_dyn_array_push(monitor->changes, change);
        }
        break;
      }
    }
  }

  sp_fmon_emit_changes(monitor);
}

#endif

#elif defined(SP_COSMO)
void sp_fmon_os_init(sp_fmon_t* monitor) {
  (void)monitor;
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  (void)monitor;
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t directory_path) {
  (void)monitor; (void)directory_path;
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  (void)monitor; (void)file_path;
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  (void)monitor;
}
#endif


// ███████╗██████╗ ██████╗  ██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔══██╗
// █████╗  ██████╔╝██████╔╝██║   ██║██████╔╝
// ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══██╗
// ███████╗██║  ██║██║  ██║╚██████╔╝██║  ██║
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
// @error
void sp_err_set(sp_err_t err) {
  (void)err;
}

sp_err_t sp_err_get() {
  return SP_ERR_OK;
}


// ██╗ ██████╗
// ██║██╔═══██╗
// ██║██║   ██║
// ██║██║   ██║
// ██║╚██████╔╝
// ╚═╝ ╚═════╝
// @io
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

  sp_mem_copy(data->here, ptr, to_read);
  data->here += to_read;
  return to_read;
}

u64 sp_io_memory_write(sp_io_stream_t* stream, const void* ptr, u64 size) {
  sp_io_memory_data_t* data = &stream->memory;
  u64 available = data->stop - data->here;
  u64 to_write = SP_MIN(size, available);

  sp_mem_copy(ptr, data->here, to_write);
  data->here += to_write;
  return to_write;
}

u64 sp_io_memory_pad(sp_io_stream_t* stream, u64 size) {
  sp_io_memory_data_t* data = &stream->memory;
  u64 available = data->stop - data->here;
  u64 to_pad = SP_MIN(size, available);

  sp_mem_zero(data->here, to_pad);
  data->here += to_pad;
  return to_pad;
}

void sp_io_memory_close(sp_io_stream_t* stream) {
  (void)stream;
}

s64 sp_io_buffer_size(sp_io_stream_t* stream) {
  sp_io_buffer_data_t* data = &stream->buffer;
  return data->stop - data->base;
}

s64 sp_io_buffer_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence) {
  sp_io_buffer_data_t* data = &stream->buffer;
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

u64 sp_io_buffer_read(sp_io_stream_t* stream, void* ptr, u64 size) {
  sp_io_buffer_data_t* data = &stream->buffer;
  u64 available = data->stop - data->here;
  u64 to_read = SP_MIN(size, available);

  sp_mem_copy(data->here, ptr, to_read);
  data->here += to_read;
  return to_read;
}

u64 sp_io_buffer_write(sp_io_stream_t* stream, const void* ptr, u64 size) {
  sp_io_buffer_data_t* data = &stream->buffer;
  u64 pos = data->here - data->base;
  u64 required = pos + size;
  u64 capacity = data->max - data->base;

  if (required > capacity) {
    u64 new_capacity = capacity ? capacity : 64;
    while (new_capacity < required) {
      new_capacity *= 2;
    }
    u64 content_size = data->stop - data->base;
    data->base = (u8*)sp_realloc(data->base, (u32)new_capacity);
    data->here = data->base + pos;
    data->stop = data->base + content_size;
    data->max = data->base + new_capacity;
  }

  sp_mem_copy(ptr, data->here, size);
  data->here += size;
  if (data->here > data->stop) {
    data->stop = data->here;
  }
  return size;
}

u64 sp_io_buffer_pad(sp_io_stream_t* stream, u64 size) {
  sp_io_buffer_data_t* data = &stream->buffer;
  u64 pos = data->here - data->base;
  u64 required = pos + size;
  u64 capacity = data->max - data->base;

  if (required > capacity) {
    u64 new_capacity = capacity ? capacity : 64;
    while (new_capacity < required) {
      new_capacity *= 2;
    }
    u64 content_size = data->stop - data->base;
    data->base = (u8*)sp_realloc(data->base, (u32)new_capacity);
    data->here = data->base + pos;
    data->stop = data->base + content_size;
    data->max = data->base + new_capacity;
  }

  sp_mem_zero(data->here, size);
  data->here += size;
  if (data->here > data->stop) {
    data->stop = data->here;
  }
  return size;
}

void sp_io_buffer_close(sp_io_stream_t* stream) {
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

u64 sp_io_file_pad(sp_io_stream_t* stream, u64 size) {
  static const u8 zeros[64] = {0};
  u64 written = 0;
  while (written < size) {
    u64 chunk = SP_MIN(size - written, 64);
    u64 result = sp_io_file_write(stream, zeros, chunk);
    if (result == 0) break;
    written += result;
  }
  return written;
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
    .pad = sp_io_file_pad,
    .close = sp_io_file_close,
  };
  stream.file.fd = fd;
  stream.file.close_mode = SP_IO_FILE_CLOSE_MODE_AUTO;

  if (fd < 0) {
    sp_err_set(SP_ERR_IO);
    return stream;
  }
  SP_ASSERT(sp_fs_is_target_regular_file(path));

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
  stream.callbacks.pad = sp_io_memory_pad;
  stream.callbacks.close = sp_io_memory_close;

  return stream;
}

sp_io_stream_t sp_io_from_buffer() {
  return (sp_io_stream_t) {
    .callbacks = {
      .size = sp_io_buffer_size,
      .seek = sp_io_buffer_seek,
      .read = sp_io_buffer_read,
      .write = sp_io_buffer_write,
      .pad = sp_io_buffer_pad,
      .close = sp_io_buffer_close,
    }
  };
}

sp_str_t sp_io_buffer_to_str(sp_io_stream_t* stream) {
  SP_ASSERT(stream);
  sp_io_buffer_data_t* data = &stream->buffer;
  return (sp_str_t) {
    .data = (c8*)data->base,
    .len = (u32)(data->stop - data->base),
  };
}

sp_io_stream_t sp_io_from_file_handle(sp_os_file_handle_t handle, sp_io_file_close_mode_t close_mode) {
  sp_io_stream_t stream = SP_ZERO_INITIALIZE();
  stream.callbacks = (sp_io_callbacks_t) {
    .size = sp_io_file_size,
    .seek = sp_io_file_seek,
    .read = sp_io_file_read,
    .write = sp_io_file_write,
    .pad = sp_io_file_pad,
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

u64 sp_io_pad(sp_io_stream_t* stream, u64 size) {
  SP_ASSERT(stream); SP_ASSERT(stream->callbacks.pad);
  return stream->callbacks.pad(stream, size);
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

//  █████╗ ███████╗███████╗███████╗████████╗
// ██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝
// ███████║███████╗███████╗█████╗     ██║
// ██╔══██║╚════██║╚════██║██╔══╝     ██║
// ██║  ██║███████║███████║███████╗   ██║
// ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝
// @asset
void sp_asset_registry_init(sp_asset_registry_t* registry, sp_asset_registry_config_t config) {
  sp_mutex_init(&registry->mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->import_mutex, SP_MUTEX_PLAIN);
  sp_mutex_init(&registry->completion_mutex, SP_MUTEX_PLAIN);

  sp_semaphore_init(&registry->semaphore);
  registry->shutdown_requested = false;

  registry->import_queue = SP_NULLPTR;
  registry->completion_queue = SP_NULLPTR;

  for (u32 index = 0; index < SP_ASSET_REGISTRY_CONFIG_MAX_IMPORTERS; index++) {
    sp_asset_importer_config_t* cfg = &config.importers[index];
    if (cfg->kind == SP_ASSET_KIND_NONE) break;

    sp_asset_importer_t importer = (sp_asset_importer_t) {
      .on_import = cfg->on_import,
      .on_completion = cfg->on_completion,
      .registry = registry,
      .kind = cfg->kind,
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
  while (!sp_rb_empty(registry->completion_queue)) {
    sp_asset_import_context_t context = *sp_rb_peek(registry->completion_queue);
    sp_rb_pop(registry->completion_queue);
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
    .future = sp_future_create(sizeof(sp_asset_t*)),
    .user_data = user_data,
    .asset_index = asset_index,
  };

  sp_mutex_lock(&registry->import_mutex);
  sp_rb_push(registry->import_queue, context);
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

    while (!sp_rb_empty(registry->import_queue)) {
      sp_asset_import_context_t context = *sp_rb_peek(registry->import_queue);
      sp_rb_pop(registry->import_queue);

      sp_mutex_unlock(&registry->import_mutex);

      context.importer->on_import(&context);

      sp_mutex_lock(&registry->mutex);
      sp_asset_t* asset = &registry->assets[context.asset_index];
      asset->state = SP_ASSET_STATE_IMPORTED;
      sp_mutex_unlock(&registry->mutex);

      sp_mutex_lock(&registry->completion_mutex);
      sp_rb_push(registry->completion_queue, context);
      sp_mutex_unlock(&registry->completion_mutex);

      sp_mutex_lock(&registry->import_mutex);
    }

    sp_mutex_unlock(&registry->import_mutex);
  }

  return 0;
}

sp_app_t* sp_app_new(sp_app_config_t config) {
  sp_app_t* app = SP_ALLOC(sp_app_t);
  *app = (sp_app_t) {
    .user_data = config.user_data,
    .on_init = config.on_init,
    .on_poll = config.on_poll,
    .on_update = config.on_update,
    .on_deinit = config.on_deinit,
    .fps = config.fps,
    .frame = {
      .target = sp_tm_fps_to_ns(config.fps),
    }
  };
  return app;
}

SP_API s32 sp_app_run(sp_app_config_t config) {
  sp_app_t* sp = sp_app_new(config);

  if (sp->on_init) {
    if (sp->on_init(sp) != SP_APP_CONTINUE) {
      goto deinit;
    };
  }

  sp->frame.timer = sp_tm_start_timer();
  while (true) {
    if (sp->on_poll) {
      sp->on_poll(sp);
    }

    sp->frame.accumulated += sp_tm_lap_timer(&sp->frame.timer);
    if (sp->frame.accumulated >= sp->frame.target) {
      sp->frame.accumulated -= sp->frame.target;
      sp->frame.num++;
      if (sp->on_update(sp) != SP_APP_CONTINUE) {
        break;
      };
    }
    else {
      sp_sleep_ns(sp->frame.target - sp->frame.accumulated);
    }
  }

deinit:
  if (sp->on_deinit) {
    sp->on_deinit(sp);
  }

  return sp->return_code;
}

#if defined(SP_MAIN)
s32 main(s32 num_args, const c8** args) {
  return sp_app_run(sp_main(num_args, args));
}
#endif

#if defined(SP_ELF)
bool sp_elf_is_align_valid(u64 v) {
  return v == 0 || (v & (v - 1)) == 0;
}

sp_elf_t* sp_elf_new() {
  return sp_elf_new_alloc(sp_context_get()->allocator);
}

sp_elf_t* sp_elf_new_alloc(sp_allocator_t allocator) {
  sp_context_push_allocator(allocator);
  sp_elf_t* elf = sp_alloc_type(sp_elf_t);
  elf->allocator = allocator;
  sp_ht_set_fns(elf->section_map, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);
  sp_context_pop();

  return elf;
}

sp_elf_t* sp_elf_new_with_null_section() {
  sp_elf_t* elf = sp_elf_new();

  sp_da_push(elf->sections, ((sp_elf_section_t){
    .name = sp_str_lit(""),
    .index = 0,
    .type = SHT_NULL,
    .alignment = 0,
  }));

  return elf;
}

void sp_elf_free(sp_elf_t* elf) {
  if (!elf) return;
  sp_allocator_t alloc = elf->allocator;

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    if (!sp_str_empty(sec->name) && sec->name.data != SP_NULLPTR) {
      sp_mem_allocator_free(alloc, (void*)sec->name.data);
    }
    if (sec->buffer.data) {
      sp_mem_allocator_free(alloc, sec->buffer.data);
    }
  }

  sp_da_free(elf->sections);
  sp_ht_free(elf->section_map);
  sp_mem_allocator_free(alloc, elf);
}

sp_elf_section_t* sp_elf_add_section(sp_elf_t* elf, sp_str_t name, u32 kind, u64 alignment) {
  sp_require_as_null(elf);
  sp_require_as_null(!sp_str_empty(name));
  sp_require_as_null(sp_elf_is_align_valid(alignment));

  sp_context_push_allocator(elf->allocator);

  sp_elf_section_t section = {
    .name = sp_str_copy(name),
    .index = sp_da_size(elf->sections),
    .type = kind,
    .alignment = alignment,
  };

  sp_da_push(elf->sections, section);
  sp_ht_insert(elf->section_map, section.name, section.index);

  sp_context_pop();

  return sp_da_back(elf->sections);
}

sp_elf_section_t* sp_elf_find_section_by_index(sp_elf_t* elf, u32 index) {
  sp_require_as_null(elf);
  sp_require_as_null(sp_da_bounds_ok(elf->sections, index));

  return &elf->sections[index];
}

sp_elf_section_t* sp_elf_find_section_by_name(sp_elf_t* elf, sp_str_t name) {
  sp_require_as_null(elf);
  sp_require_as_null(sp_ht_key_exists(elf->section_map, name));

  u32 index = *sp_ht_getp(elf->section_map, name);
  return &elf->sections[index];
}

u32 sp_elf_num_sections(sp_elf_t* elf) {
  return elf ? sp_da_size(elf->sections) : 0;
}

void sp_elf_buffer_grow(sp_elf_buffer_t* buffer, u64 size) {
  if (size <= buffer->capacity) {
    return;
  }

  u64 capacity = SP_MAX(buffer->capacity, SP_ELF_DEFAULT_BUFFER_SIZE);
  while (capacity < size) {
    capacity *= 2;
  }

  buffer->capacity = capacity;
  buffer->data = (u8*)sp_realloc(buffer->data, capacity);
}

void* sp_elf_section_reserve_aligned(sp_elf_section_t* section, u64 size, u64 align) {
  sp_assert(section);

  u64 offset = sp_align_offset(section->buffer.size, align);
  u64 total = offset + size;
  sp_elf_buffer_grow(&section->buffer, total);

  section->buffer.size = total;
  return section->buffer.data + offset;
}

u8* sp_elf_section_reserve_bytes(sp_elf_section_t* section, u64 size) {
  return (u8*)sp_elf_section_reserve_aligned(section, size, 1);
}

void* sp_elf_section_reserve_entry(sp_elf_section_t* section) {
  return sp_elf_section_reserve_aligned(section, section->entry_size, 1);
}

u32 sp_elf_hash(sp_str_t name) {
  u32 h = 0;
  for (u32 i = 0; i < name.len; i++) {
    h = (h << 4) + (u8)name.data[i];
    u32 g = h & 0xf0000000;
    if (g) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

u32 sp_elf_add_string(sp_elf_section_t* strtab, sp_str_t str) {
  if (!strtab) return SP_ELF_NULL_INDEX;

  if (!strtab->buffer.size) {
    u8* p = sp_elf_section_reserve_bytes(strtab, 1);
    *p = 0;
  }

  if (sp_str_empty(str)) {
    return 0;
  }

  u32 index = (u32)strtab->buffer.size;
  u8* p = sp_elf_section_reserve_bytes(strtab, str.len + 1);
  sp_mem_copy(str.data, p, str.len);
  p[str.len] = 0;

  return index;
}

sp_elf_section_t* sp_elf_symtab_new(sp_elf_t* elf) {
  if (!elf) return SP_NULLPTR;

  sp_elf_section_t* strtab = sp_elf_add_section(elf, sp_str_lit(".strtab"), SHT_STRTAB, 1);
  if (!strtab) return SP_NULLPTR;

  sp_elf_section_t* symtab = sp_elf_add_section(elf, sp_str_lit(".symtab"), SHT_SYMTAB, 8);
  if (!symtab) return SP_NULLPTR;

  symtab->link = strtab->index;
  symtab->entry_size = sizeof(Elf64_Sym);

  sp_elf_add_string(strtab, sp_str_lit(""));

  Elf64_Sym* sym0 = (Elf64_Sym*)sp_elf_section_reserve_entry(symtab);
  sp_mem_zero(sym0, sizeof(Elf64_Sym));

  return symtab;
}

u32 sp_elf_section_num_entries(sp_elf_section_t* symtab) {
  return (u32)(symtab->buffer.size / symtab->entry_size);
}

Elf64_Sym* sp_elf_symtab_get(sp_elf_section_t* symtab, u32 index) {
  SP_ASSERT(symtab && symtab->type == SHT_SYMTAB);
  SP_ASSERT(index < sp_elf_section_num_entries(symtab));
  return (Elf64_Sym*)symtab->buffer.data + index;
}

Elf64_Rela* sp_elf_rela_get(sp_elf_section_t* rela, u32 index) {
  SP_ASSERT(rela && rela->type == SHT_RELA);
  SP_ASSERT(index < sp_elf_section_num_entries(rela));
  return (Elf64_Rela*)rela->buffer.data + index;
}

u32 sp_elf_strtab_size(sp_elf_section_t* strtab) {
  SP_ASSERT(strtab && strtab->type == SHT_STRTAB);
  return (u32)strtab->buffer.size;
}

sp_str_t sp_elf_strtab_get(sp_elf_section_t* strtab, u32 offset) {
  SP_ASSERT(strtab && strtab->type == SHT_STRTAB);
  SP_ASSERT(offset < strtab->buffer.size);
  return sp_str_view((const c8*)strtab->buffer.data + offset);
}

u32 sp_elf_add_symbol(sp_elf_section_t* symbols, sp_elf_t* elf, sp_str_t name, u64 value, u64 size, u8 bind, u8 type, u16 shndx) {
  sp_require_as(symbols, SP_ELF_NULL_INDEX);
  sp_require_as(elf, SP_ELF_NULL_INDEX);

  sp_elf_section_t* strings = sp_elf_find_section_by_index(elf, symbols->link);
  u32 sym_idx = sp_elf_section_num_entries(symbols);

  Elf64_Sym* sym = (Elf64_Sym*)sp_elf_section_reserve_entry(symbols);
  sym->st_name = sp_elf_add_string(strings, name);
  sym->st_value = value;
  sym->st_size = size;
  sym->st_info = ELF64_ST_INFO(bind, type);
  sym->st_other = 0;
  sym->st_shndx = shndx;

  return sym_idx;
}

sp_elf_section_t* sp_elf_rela_new(sp_elf_t* elf, sp_elf_section_t* target) {
  if (!elf) return SP_NULLPTR;
  if (!target) return SP_NULLPTR;

  sp_elf_section_t* symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
  if (!symtab) return SP_NULLPTR;

  sp_str_t name = sp_format(".rela{}", SP_FMT_STR(target->name));
  sp_elf_section_t* rela = sp_elf_add_section(elf, name, SHT_RELA, 8);
  if (!rela) return SP_NULLPTR;
  rela->link = symtab->index;
  rela->info = target->index;
  rela->entry_size = sizeof(Elf64_Rela);

  return rela;
}

void sp_elf_add_relocation(sp_elf_section_t* rela, u64 offset, u32 sym_idx, u32 type, s64 addend) {
  if (!rela) return;

  Elf64_Rela* rel = (Elf64_Rela*)sp_elf_section_reserve_entry(rela);
  rel->r_offset = offset;
  rel->r_info = ELF64_R_INFO(sym_idx, type);
  rel->r_addend = addend;
}

void sp_elf_symtab_sort(sp_elf_section_t* symtab, sp_elf_t* elf) {
  if (!symtab) return;
  SP_ASSERT(symtab->type == SHT_SYMTAB);

  u32 count = sp_elf_section_num_entries(symtab);
  if (count <= 1) {
    symtab->info = count;
    return;
  }

  sp_da(u32) old_to_new = SP_NULLPTR;
  for (u32 i = 0; i < count; i++) {
    sp_da_push(old_to_new, 0);
  }

  Elf64_Sym* syms = (Elf64_Sym*)symtab->buffer.data;
  Elf64_Sym* new_syms = sp_alloc_n(Elf64_Sym, count);
  u32 local_idx = 0;
  (void)count;

  for (u32 i = 0; i < count; i++) {
    if (ELF64_ST_BIND(syms[i].st_info) == STB_LOCAL) {
      old_to_new[i] = local_idx;
      new_syms[local_idx++] = syms[i];
    }
  }

  u32 first_nonlocal = local_idx;

  for (u32 i = 0; i < count; i++) {
    if (ELF64_ST_BIND(syms[i].st_info) != STB_LOCAL) {
      old_to_new[i] = local_idx;
      new_syms[local_idx++] = syms[i];
    }
  }

  sp_mem_copy(new_syms, syms, count * sizeof(Elf64_Sym));
  symtab->info = first_nonlocal;

  if (elf) {
    for (u32 i = 0; i < sp_elf_num_sections(elf); i++) {
      sp_elf_section_t* sec = sp_elf_find_section_by_index(elf, i);
      if (sec->type == SHT_RELA && sec->link == symtab->index) {
        u32 rela_count = sp_elf_section_num_entries(sec);
        for (u32 j = 0; j < rela_count; j++) {
          Elf64_Rela* rel = sp_elf_rela_get(sec, j);
          u32 sym_idx = ELF64_R_SYM(rel->r_info);
          u32 type = ELF64_R_TYPE(rel->r_info);
          rel->r_info = ELF64_R_INFO(old_to_new[sym_idx], type);
        }
      }
    }
  }

  sp_da_free(old_to_new);
}

sp_elf_section_t* sp_elf_find_or_create_shstrtab(sp_elf_t* elf) {
  sp_elf_section_t* table = sp_elf_find_section_by_name(elf, sp_str_lit(".shstrtab"));
  if (table) {
    return table;
  }

  table = sp_elf_add_section(elf, sp_str_lit(".shstrtab"), SHT_STRTAB, 1);
  sp_elf_add_string(table, sp_str_lit(""));
  return table;
}

sp_err_t sp_elf_write(sp_elf_t* elf, sp_io_stream_t* out) {
  sp_require_as(elf, SP_ERR_LAZY);
  sp_require_as(out, SP_ERR_LAZY);

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    if (sec->type == SHT_SYMTAB) {
      sp_elf_symtab_sort(sec, elf);
    }
  }

  sp_elf_section_t* shstrtab = sp_elf_find_or_create_shstrtab(elf);
  u32 num_sections = sp_elf_num_sections(elf);

  sp_da(Elf64_Shdr) headers = SP_NULLPTR;
  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    sp_da_push(headers, ((Elf64_Shdr) {
      .sh_name = sp_elf_add_string(shstrtab, sec->name),
      .sh_type = sec->type,
      .sh_flags = sec->flags,
      .sh_addr = sec->addr,
      .sh_size = sec->buffer.size,
      .sh_link = sec->link,
      .sh_info = sec->info,
      .sh_addralign = sec->alignment,
      .sh_entsize = sec->entry_size,
    }));
  }

  u64 shoff = sp_align_offset(sizeof(Elf64_Ehdr), 4);
  u64 file_offset = shoff + num_sections * sizeof(Elf64_Shdr);

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    switch (sec->type) {
      case SHT_NULL:
      case SHT_NOBITS: {
        headers[i].sh_offset = 0;
        break;
      }
      default: {
        file_offset = sp_align_offset(file_offset, 16);
        headers[i].sh_offset = file_offset;
        headers[i].sh_size = sec->buffer.size;
        file_offset += sec->buffer.size;
        break;
      }
    }
  }

  Elf64_Ehdr ehdr = {
    .e_type = ET_REL,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,
    .e_entry = 0,
    .e_phoff = 0,
    .e_shoff = shoff,
    .e_flags = 0,
    .e_ehsize = sizeof(Elf64_Ehdr),
    .e_phentsize = 0,
    .e_phnum = 0,
    .e_shentsize = sizeof(Elf64_Shdr),
    .e_shnum = (u16)num_sections,
    .e_shstrndx = (u16)shstrtab->index,
  };
  ehdr.e_ident[EI_MAG0] = ELFMAG0;
  ehdr.e_ident[EI_MAG1] = ELFMAG1;
  ehdr.e_ident[EI_MAG2] = ELFMAG2;
  ehdr.e_ident[EI_MAG3] = ELFMAG3;
  ehdr.e_ident[EI_CLASS] = ELFCLASS64;
  ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
  ehdr.e_ident[EI_VERSION] = EV_CURRENT;
  ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;

  u64 written = 0;
  written += sp_io_write(out, &ehdr, sizeof(Elf64_Ehdr));
  written += sp_io_pad(out, shoff - written);

  sp_da_for(headers, i) {
    written += sp_io_write(out, &headers[i], sizeof(Elf64_Shdr));
  }

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    switch (sec->type) {
      case SHT_NULL:
      case SHT_NOBITS: {
        break;
      }
      default: {
        written += sp_io_pad(out, headers[i].sh_offset - written);
        written += sp_io_write(out, sec->buffer.data, sec->buffer.size);
        break;
      }
    }
  }

  sp_da_free(headers);
  return SP_ERR_OK;
}

sp_err_t sp_elf_write_to_file(sp_elf_t* elf, sp_str_t path) {
  sp_io_stream_t f = sp_io_from_file(path, SP_IO_MODE_WRITE);
  sp_err_t err = sp_elf_write(elf, &f);
  sp_io_close(&f);
  return err;
}

sp_elf_t* sp_elf_read(sp_io_stream_t* in) {
  sp_require_as_null(in);

  Elf64_Ehdr ehdr = SP_ZERO_INITIALIZE();
  u64 bytes_read = sp_io_read(in, &ehdr, sizeof(Elf64_Ehdr));
  if (bytes_read != sizeof(Elf64_Ehdr)) {
    return SP_NULLPTR;
  }

  u16 num_sections = ehdr.e_shnum;
  sp_require_as_null(ehdr.e_shstrndx < num_sections);

  if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
      ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
      ehdr.e_ident[EI_MAG3] != ELFMAG3) {
    sp_err_set(SP_ERR_LAZY);
    return SP_NULLPTR;
  }

  if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) {
    sp_err_set(SP_ERR_LAZY);
    return SP_NULLPTR;
  }

  if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
    sp_err_set(SP_ERR_LAZY);
    return SP_NULLPTR;
  }


  if (!num_sections) {
    return sp_elf_new();
  }

  Elf64_Shdr* section_headers = sp_alloc_n(Elf64_Shdr, num_sections);
  sp_io_seek(in, (s64)ehdr.e_shoff, SP_IO_SEEK_SET);
  bytes_read = sp_io_read(in, section_headers, num_sections * sizeof(Elf64_Shdr));
  if (bytes_read != num_sections * sizeof(Elf64_Shdr)) {
    return SP_NULLPTR;
  }

  Elf64_Shdr* string_header = &section_headers[ehdr.e_shstrndx];

  c8* string_table = SP_NULLPTR;
  if (string_header->sh_size) {
    string_table = sp_alloc_n(c8, string_header->sh_size);
    sp_io_seek(in, string_header->sh_offset, SP_IO_SEEK_SET);
    sp_io_read(in, string_table, string_header->sh_size);
  }

  sp_elf_t* elf = sp_elf_new();

  sp_assert(section_headers[0].sh_type == SHT_NULL);
  sp_da_push(elf->sections, ((sp_elf_section_t) {
    .type = SHT_NULL
  }));

  for (u32 it = 1; it < num_sections; it++) {
    if (it == ehdr.e_shstrndx) continue;

    Elf64_Shdr* header = &section_headers[it];

    sp_assert(header->sh_name < string_header->sh_size);
    sp_str_t name = sp_str_view(string_table + header->sh_name);

    sp_elf_section_t* section = sp_elf_add_section(elf, name, header->sh_type, header->sh_addralign);
    section->flags = header->sh_flags;
    section->addr = header->sh_addr;
    section->link = header->sh_link;
    section->info = header->sh_info;
    section->entry_size = header->sh_entsize;

    if (header->sh_size) {
      switch (header->sh_type) {
        case SHT_NOBITS: {
          section->buffer.size = header->sh_size;
          break;
        }
        default: {
          sp_io_seek(in, header->sh_offset, SP_IO_SEEK_SET);
          u8* ptr = sp_elf_section_reserve_bytes(section, header->sh_size);
          sp_io_read(in, ptr, header->sh_size);
          break;
        }
      }
    }
  }

  return elf;
}

sp_elf_t* sp_elf_read_from_file(sp_str_t path) {
  sp_io_stream_t f = sp_io_from_file(path, SP_IO_MODE_READ);
  if (f.file.fd < 0) {
    sp_err_set(SP_ERR_LAZY);
    return SP_NULLPTR;
  }
  sp_elf_t* elf = sp_elf_read(&f);
  sp_io_close(&f);
  return elf;
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
  return sp_fs_normalize_path(result);
}

sp_str_t operator/(const sp_str_t& a, const c8* b) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append_cstr(&builder, b);
  sp_str_t result = sp_str_builder_write(&builder);
  return sp_fs_normalize_path(result);
}
#endif

#endif // SP_SP_C
#endif // SP_IMPLEMENTATION
