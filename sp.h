/*
    █████████  ███████████     █████   █████
   ███░░░░░███░░███░░░░░███   ░░███   ░░███
  ░███    ░░░  ░███    ░███    ░███    ░███
  ░░█████████  ░██████████     ░███████████
   ░░░░░░░░███ ░███░░░░░░      ░███░░░░░███
   ███    ░███ ░███            ░███    ░███
  ░░█████████  █████        ██ █████   █████
   ░░░░░░░░░  ░░░░░        ░░ ░░░░░   ░░░░░

  >> sp.h
  the single header nonstandard library for C

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
    @ sp_atomic        compiler intrinsic atomics
    + sp_context       thread-local allocator, scratch memory
    + sp_da     stb-style resizable array (intrusive T* + macros)
      sp_env           environment variables
      sp_err           thread-local errno style error system
    + sp_fmt           "a type-safe {.cyan .italic} replacement", sp_fmt_cstr("printf()")
      sp_fmon          os native filesystem watching
    + sp_fs            path manipulation, filesystem, common system paths (e.g. appdata)
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

    The following modules are extensions in separate headers

      sp_asset         multithreaded asset registry, importers
      sp_elf           minimal elf reading + writing + modification
      sp_glob

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

//  ██████████   ██████████ ███████████ ██████████   █████████  ███████████ █████    ███████    ██████   █████
// ░░███░░░░███ ░░███░░░░░█░█░░░███░░░█░░███░░░░░█  ███░░░░░███░█░░░███░░░█░░███   ███░░░░░███ ░░██████ ░░███
//  ░███   ░░███ ░███  █ ░ ░   ░███  ░  ░███  █ ░  ███     ░░░ ░   ░███  ░  ░███  ███     ░░███ ░███░███ ░███
//  ░███    ░███ ░██████       ░███     ░██████   ░███             ░███     ░███ ░███      ░███ ░███░░███░███
//  ░███    ░███ ░███░░█       ░███     ░███░░█   ░███             ░███     ░███ ░███      ░███ ░███ ░░██████
//  ░███    ███  ░███ ░   █    ░███     ░███ ░   █░░███     ███    ░███     ░███ ░░███     ███  ░███  ░░█████
//  ██████████   ██████████    █████    ██████████ ░░█████████     █████    █████ ░░░███████░   █████  ░░█████
// ░░░░░░░░░░   ░░░░░░░░░░    ░░░░░    ░░░░░░░░░░   ░░░░░░░░░     ░░░░░    ░░░░░    ░░░░░░░    ░░░░░    ░░░░░
// @detection
//////////////
// PLATFORM //
//////////////
#if defined(_WIN32)
  #define SP_WIN32

#elif defined(__linux__)
  #define SP_LINUX
  #define SP_UNIX

#elif defined(__APPLE__)
  #define SP_MACOS
  #define SP_UNIX

#elif __COSMOPOLITAN__
  #define SP_COSMO
  #define SP_UNIX

#elif defined(__wasm__)
  #define SP_WASM

  #if defined(__wasi__)
    #define SP_WASM_WASI
  #else
    #define SP_WASM_FREESTANDING
  #endif
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
  #define SP_GNUC
#elif defined(__GNUC__) && !defined(SP_CLANG)
  #define SP_GCC
  #define SP_GNUC
#endif

//////////
// LIBC //
//////////
#if defined(__GLIBC__)
  #define SP_LIBC_GNU
#elif defined(__MINGW64__)
  #define SP_LIBC_GNU
#elif defined(_MSC_VER)
  #define SP_LIBC_MSVC
#elif defined(SP_FREESTANDING)
  #define SP_LIBC_NONE
#elif defined(SP_MACOS)
  #define SP_LIBC_APPLE
#elif defined(SP_LINUX)
  #define SP_LIBC_MUSL
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
  #define SP_IMP static
#endif

#if !defined(SP_IMPORT)
  #if defined(SP_WIN32)
    #define SP_IMPORT __declspec(dllimport)
  #else
    #define SP_IMPORT
  #endif
#endif

#if !defined(SP_EXPORT)
  #if defined(SP_WIN32)
    #define SP_EXPORT __declspec(dllexport)
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


//  ██████   ██████   █████████     █████████  ███████████      ███████     █████████
// ░░██████ ██████   ███░░░░░███   ███░░░░░███░░███░░░░░███   ███░░░░░███  ███░░░░░███
//  ░███░█████░███  ░███    ░███  ███     ░░░  ░███    ░███  ███     ░░███░███    ░░░
//  ░███░░███ ░███  ░███████████ ░███          ░██████████  ░███      ░███░░█████████
//  ░███ ░░░  ░███  ░███░░░░░███ ░███          ░███░░░░░███ ░███      ░███ ░░░░░░░░███
//  ░███      ░███  ░███    ░███ ░░███     ███ ░███    ░███ ░░███     ███  ███    ░███
//  █████     █████ █████   █████ ░░█████████  █████   █████ ░░░███████░  ░░█████████
// ░░░░░     ░░░░░ ░░░░░   ░░░░░   ░░░░░░░░░  ░░░░░   ░░░░░    ░░░░░░░     ░░░░░░░░░
// @macro
///////////////
// SP_LIKELY //
///////////////
#if defined(SP_GNUC)
  #define SP_LIKELY(x)   __builtin_expect(!!(x), 1)
  #define SP_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define SP_LIKELY(x)   (x)
  #define SP_UNLIKELY(x) (x)
#endif

//////////////////////
// SP_HAS_ATTRIBUTE //
//////////////////////
#if !defined(SP_MSVC)
  #define SP_HAS_ATTRIBUTE(attr) __has_attribute(attr)
#else
  #define SP_HAS_ATTRIBUTE(attr) 0
#endif

//////////////
// SP_ALIGN //
//////////////
#if defined(SP_MSVC)
  #define sp_align(n) __declspec(align(n))
#elif defined(SP_GNUC) || defined(SP_TCC)
  #define sp_align(n) __attribute__((aligned(n)))
#else
  // @spader Too lazy to actually think about this
  #define sp_align(n) _Alignas(n)
#endif

#ifdef SP_CPP
  #define SP_RVAL(T) T
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
#define sp_zero() SP_ZERO_INITIALIZE()

#if SP_HAS_ATTRIBUTE(fallthrough)
  #define SP_FALLTHROUGH() __attribute__((fallthrough))
  #define sp_fallthrough() SP_FALLTHROUGH()
#else
  #define SP_FALLTHROUGH() ((void)0)
  #define sp_fallthrough() ((void)0)
#endif

#define SP_ZERO_STRUCT(t) SP_RVAL(t) SP_ZERO_INITIALIZE()
#define sp_zero_struct(t) SP_ZERO_STRUCT(t)
#define SP_ZERO_RETURN(t) { t __SP_ZERO_RETURN = SP_ZERO_STRUCT(t); return __SP_ZERO_RETURN; }
#define sp_zero_return(t) SP_ZERO_RETURN(t)

#define SP_EXIT_SUCCESS() exit(0)
#define sp_exit_success() SP_EXIT_SUCCESS()
#define SP_EXIT_FAILURE() exit(1)
#define sp_exit_failure() SP_EXIT_FAILURE()

// sp_assert() is platform dependent, so it's defined with the other platform stuff. The macros
// that just wrap it with some goodies are agnostic, though, so they're here
#define SP_ASSERT(condition) sp_assert((condition))
#define sp_assert(x) sp_sys_assert(__FILE__, SP_MACRO_STR(__LINE__), __func__, #x, (bool)(x))

#define sp_fatal(FMT, ...) do { sp_log_a((FMT), ##__VA_ARGS__); sp_assert(false); } while (0)

#define SP_UNREACHABLE() SP_ASSERT(false)
#define sp_unreachable() SP_UNREACHABLE()

#define SP_UNREACHABLE_CASE() SP_ASSERT(false); break;
#define sp_unreachable_case() SP_UNREACHABLE_CASE()

#define SP_UNREACHABLE_RETURN(v) SP_ASSERT(false); return (v)
#define sp_unreachable_return(v) SP_UNREACHABLE_RETURN(v)

#ifndef SP_UNIMPLEMENTED
  #define SP_UNIMPLEMENTED() SP_ASSERT(false)
#endif

#define SP_UNTESTED()
#define sp_untested() SP_UNTESTED()
#define SP_INCOMPLETE()
#define sp_incomplete() SP_INCOMPLETE()

#define SP_TYPEDEF_FN(return_type, name, ...) typedef return_type(*name)(__VA_ARGS__)
#define sp_typedef_fn(return_type, name, ...) SP_TYPEDEF_FN(return_type, name, __VA_ARGS__)

#define SP_UNUSED(x) ((void)(x))
#define sp_unused(x) SP_UNUSED(x)

#define _SP_MACRO_STR(x) #x
#define SP_MACRO_STR(x) _SP_MACRO_STR(x)
#define sp_macro_str(x) SP_MACRO_STR(x)
#define sp_mstr(x) SP_MACRO_STR(x)

#define _SP_MACRO_CAT(x, y) x##y
#define SP_MACRO_CAT(x, y) _SP_MACRO_CAT(x, y)
#define sp_macro_cat(x, y) SP_MACRO_CAT(x, y)
#define sp_mcat(x, y) SP_MACRO_CAT(x, y)

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
#define sp_aligned() sp_align(SP_MEM_ALIGNMENT)
#define sp_align_up(ptr, align) ((void*)(((uintptr_t)(ptr) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1)))
#define sp_align_offset(val, align) ((((val) + ((u64)(align) - 1)) & ~((u64)(align) - 1)))

#define sp_try(expr) \
  do { \
    s32 _sp_result = (expr); \
    if (_sp_result) return _sp_result; \
  } while (0)

#define sp_try_as(expr, err) \
  do { \
    if (expr) return err; \
  } while (0)

#define sp_try_as_void(expr) \
  do { \
    if (expr) return; \
  } while (0)

#define sp_try_goto(expr, err, label) \
  do { \
    err = (expr); \
    if (err) goto label; \
  } while (0)

#define sp_try_goto_r(__expr, __result, __label) \
  do { \
    __result.err = (__expr); \
    if (__result.err) goto __label; \
  } while (0)

#define sp_try_as_null(expr) \
  sp_try_as((expr), SP_NULLPTR)

#define sp_require(expr) sp_try_as_void(!(expr))
#define sp_require_as(expr, err) sp_try_as(!(expr), err)
#define sp_require_as_null(expr) sp_try_as(!(expr), SP_NULLPTR)


//  █████ ██████   █████   █████████  █████       █████  █████ ██████████   ██████████
// ░░███ ░░██████ ░░███   ███░░░░░███░░███       ░░███  ░░███ ░░███░░░░███ ░░███░░░░░█
//  ░███  ░███░███ ░███  ███     ░░░  ░███        ░███   ░███  ░███   ░░███ ░███  █ ░
//  ░███  ░███░░███░███ ░███          ░███        ░███   ░███  ░███    ░███ ░██████
//  ░███  ░███ ░░██████ ░███          ░███        ░███   ░███  ░███    ░███ ░███░░█
//  ░███  ░███  ░░█████ ░░███     ███ ░███      █ ░███   ░███  ░███    ███  ░███ ░   █
//  █████ █████  ░░█████ ░░█████████  ███████████ ░░████████   ██████████   ██████████
// ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░  ░░░░░░░░░░░   ░░░░░░░░   ░░░░░░░░░░   ░░░░░░░░░░
// @include
SP_BEGIN_EXTERN_C()

#if defined(SP_UNIX)
  #if defined(SP_MACOS)
    #ifndef _DARWIN_C_SOURCE
      #define _DARWIN_C_SOURCE
    #endif
    #define SP_POSIX
  #elif defined(SP_COSMO)
    #ifndef _COSMO_SOURCE
      #define _COSMO_SOURCE
    #endif
    #define SP_POSIX
  #elif defined(SP_FREESTANDING)

  #elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
    #define SP_POSIX
  #elif !defined(_FEATURES_H)
    #define _POSIX_C_SOURCE 200809L
    #define SP_POSIX
  #else
    #error "sp.h needs _POSIX_C_SOURCE >= 200809L; make sure that sp.h is included before anything else"
  #endif
#endif

#if defined(SP_COSMO)
  #ifndef _COSMO_SOURCE
    #define _COSMO_SOURCE
  #endif
#endif

#if defined(SP_MACOS)
  #ifndef _DARWIN_C_SOURCE
    #define _DARWIN_C_SOURCE
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

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(SP_FREESTANDING)

#elif defined(SP_LINUX) // @preprocessor
  #include <assert.h>
  #include <fcntl.h>
  #include <poll.h>
  #include <pthread.h>
  #include <semaphore.h> // sem_t
  #include <spawn.h>
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h> // calloc, realloc, free
  #include <time.h>
  #include <sys/inotify.h>
  #include <sys/ioctl.h>
  #include <sys/time.h> // gettimeofday

  #include <errno.h>  // errno
  #include <signal.h> // signal
  #include <unistd.h> // pipe

#elif defined(SP_MACOS)
  #include <dispatch/dispatch.h>
  #include <mach-o/dyld.h>
  #include <sys/event.h>
  #include <poll.h>
  #if defined(SP_FMON_MACOS_USE_FSEVENTS)
    #include <CoreServices/CoreServices.h>
  #endif

  #include <assert.h>
  #include <fcntl.h>
  #include <poll.h>
  #include <dirent.h>
  #include <errno.h>
  #include <limits.h>
  #include <pthread.h>
  #include <semaphore.h>
  #include <signal.h>
  #include <spawn.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <termios.h>
  #include <time.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <sys/mman.h>

#elif defined(SP_WIN32)
  #include <windows.h>
  #include <assert.h>
  #include <direct.h>
  #include <fcntl.h>
  #include <io.h>
  #include <signal.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
  #include <shlobj.h>
  #include <commdlg.h>
  #include <shellapi.h>
  #include <errno.h>

#elif defined(SP_COSMO)
  #include "libc/dce.h"
  #include <sys/resource.h>
  #include <poll.h>
  #include <pthread.h>

  #include <assert.h>
  #include <fcntl.h>
  #include <poll.h>
  #include <dirent.h>
  #include <errno.h>
  #include <limits.h>
  #include <pthread.h>
  #include <semaphore.h>
  #include <signal.h>
  #include <spawn.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <termios.h>
  #include <time.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <sys/mman.h>
#endif


//  ███████████ █████ █████ ███████████  ██████████  █████████
// ░█░░░███░░░█░░███ ░░███ ░░███░░░░░███░░███░░░░░█ ███░░░░░███
// ░   ░███  ░  ░░███ ███   ░███    ░███ ░███  █ ░ ░███    ░░░
//     ░███      ░░█████    ░██████████  ░██████   ░░█████████
//     ░███       ░░███     ░███░░░░░░   ░███░░█    ░░░░░░░░███
//     ░███        ░███     ░███         ░███ ░   █ ███    ░███
//     █████       █████    █████        ██████████░░█████████
//    ░░░░░       ░░░░░    ░░░░░        ░░░░░░░░░░  ░░░░░░░░░
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

typedef struct {
  const c8* data;
  u32 len;
} sp_str_t;

typedef struct {
  u16* data;
  u32 len;
} sp_wide_str_t;

typedef enum {
  SP_OK                   = 0,
  SP_ERR                = 1,
  SP_ERR_IO               = 1001,
  SP_ERR_IO_OPEN_FAILED   = 1002,
  SP_ERR_IO_SEEK_INVALID  = 1003,
  SP_ERR_IO_SEEK_FAILED   = 1004,
  SP_ERR_IO_WRITE_FAILED  = 1005,
  SP_ERR_IO_CLOSE_FAILED  = 1006,
  SP_ERR_IO_READ_FAILED   = 1007,
  SP_ERR_IO_READ_ONLY     = 1008,
  SP_ERR_IO_NO_SPACE      = 1009,
  SP_ERR_IO_EOF           = 1010,
  SP_ERR_FMT_TOO_MANY_RENDERERS = 1100,
  SP_ERR_FMT_WRONG_PARAM_KIND = 1101,
  SP_ERR_FMT_UNKNOWN_DIRECTIVE = 1102,
  SP_ERR_FMT_BAD_DIRECTIVE = 1103,
  SP_ERR_FMT_TOO_MANY_DIRECTIVES = 1104,
  SP_ERR_FMT_BAD_PRECISION = 1105,
  SP_ERR_FMT_BAD_PLACEHOLDER = 1106,
  SP_ERR_FMT_DIRECTIVE_ARG_MISSING = 1107,
  SP_ERR_FMT_DIRECTIVE_ARG_UNEXPECTED = 1108,
  SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND = 1109,
  SP_ERR_FMT_UNTERMINATED_PLACEHOLDER = 1111,
  SP_ERR_FMT_CUSTOM_WITHOUT_FN = 1112,
  SP_ERR_LAZY,
  SP_ERR_OS,
} sp_err_t;

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

#define sp_result(T) struct { \
  T value; \
  sp_err_t err; \
}

#define sp_ok(__result, __value) ((__result).value = (__value), __result)
#define sp_err(__result, __value) ((__result).err = (__value), __result)

typedef sp_result(sp_str_t) sp_str_r;

typedef struct sp_io_reader sp_io_reader_t;
typedef struct sp_io_writer sp_io_writer_t;

#if defined(SP_WIN32)
typedef HANDLE           sp_win32_handle_t;
typedef DWORD            sp_win32_dword_t;
typedef OVERLAPPED       sp_win32_overlapped_t;
typedef WIN32_FIND_DATAW sp_win32_find_data_t;
#endif



//    █████████     ███████    ██████   █████  █████████  ███████████   █████████   ██████   █████ ███████████  █████████
//   ███░░░░░███  ███░░░░░███ ░░██████ ░░███  ███░░░░░███░█░░░███░░░█  ███░░░░░███ ░░██████ ░░███ ░█░░░███░░░█ ███░░░░░███
//  ███     ░░░  ███     ░░███ ░███░███ ░███ ░███    ░░░ ░   ░███  ░  ░███    ░███  ░███░███ ░███ ░   ░███  ░ ░███    ░░░
// ░███         ░███      ░███ ░███░░███░███ ░░█████████     ░███     ░███████████  ░███░░███░███     ░███    ░░█████████
// ░███         ░███      ░███ ░███ ░░██████  ░░░░░░░░███    ░███     ░███░░░░░███  ░███ ░░██████     ░███     ░░░░░░░░███
// ░░███     ███░░███     ███  ░███  ░░█████  ███    ░███    ░███     ░███    ░███  ░███  ░░█████     ░███     ███    ░███
//  ░░█████████  ░░░███████░   █████  ░░█████░░█████████     █████    █████   █████ █████  ░░█████    █████   ░░█████████
//   ░░░░░░░░░     ░░░░░░░    ░░░░░    ░░░░░  ░░░░░░░░░     ░░░░░    ░░░░░   ░░░░░ ░░░░░    ░░░░░    ░░░░░     ░░░░░░░░░
// @constants
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

#if defined(SP_AMD64)
  #define SP_ARCH_SET_FS 0x1002
  #define SP_O_DIRECTORY 0200000
#elif defined(SP_ARM64)
  #define SP_O_DIRECTORY          040000
#endif

#if defined(SP_LINUX)
  #define SP_EINTR                4

  #define SP_AT_FDCWD             (-100)
  #define SP_AT_SYMLINK_NOFOLLOW  0x100
  #define SP_AT_REMOVEDIR         0x200
  #define SP_AT_SYMLINK_FOLLOW    0x400
  #define SP_AT_EACCESS           0x200
  #define SP_AT_EMPTY_PATH        0x1000

  #define SP_O_RDONLY             0
  #define SP_O_WRONLY             1
  #define SP_O_RDWR               2
  #define SP_O_CREAT              0100
  #define SP_O_EXCL               0200
  #define SP_O_TRUNC              01000
  #define SP_O_APPEND             02000
  #define SP_O_NONBLOCK           04000
  #define SP_O_CLOEXEC            02000000
  #define SP_O_BINARY             0

  #define SP_SEEK_SET             0
  #define SP_SEEK_CUR             1
  #define SP_SEEK_END             2

  #define SP_F_DUPFD              0
  #define SP_F_GETFD              1
  #define SP_F_SETFD              2
  #define SP_F_GETFL              3
  #define SP_F_SETFL              4
  #define SP_FD_CLOEXEC           1

  #define SP_PROT_NONE            0
  #define SP_PROT_READ            1
  #define SP_PROT_WRITE           2
  #define SP_PROT_EXEC            4

  #define SP_MAP_SHARED           0x01
  #define SP_MAP_PRIVATE          0x02
  #define SP_MAP_FIXED            0x10
  #define SP_MAP_ANONYMOUS        0x20
  #define SP_MAP_FAILED           ((void*)-1)

  #define SP_DT_UNKNOWN   0
  #define SP_DT_FIFO      1
  #define SP_DT_CHR       2
  #define SP_DT_DIR       4
  #define SP_DT_BLK       6
  #define SP_DT_REG       8
  #define SP_DT_LNK       10
  #define SP_DT_SOCK      12

  #define SP_IN_ACCESS        0x00000001
  #define SP_IN_MODIFY        0x00000002
  #define SP_IN_ATTRIB        0x00000004
  #define SP_IN_CLOSE_WRITE   0x00000008
  #define SP_IN_CLOSE_NOWRITE 0x00000010
  #define SP_IN_OPEN          0x00000020
  #define SP_IN_MOVED_FROM    0x00000040
  #define SP_IN_MOVED_TO      0x00000080
  #define SP_IN_CREATE        0x00000100
  #define SP_IN_DELETE        0x00000200
  #define SP_IN_DELETE_SELF   0x00000400
  #define SP_IN_MOVE_SELF     0x00000800
  #define SP_IN_NONBLOCK      0x00000800
  #define SP_IN_CLOEXEC       0x00080000

  #define SP_CLOCK_REALTIME          0
  #define SP_CLOCK_MONOTONIC         1
  #define SP_TIMER_ABSTIME           1

  #define SP_BRKINT                  0x0002
  #define SP_ICRNL                   0x0100
  #define SP_INPCK                   0x0010
  #define SP_ISTRIP                  0x0020
  #define SP_IXON                    0x0400
  #define SP_OPOST                   0x0001
  #define SP_CS8                     0x0030
  #define SP_ECHO                    0x0008
  #define SP_ICANON                  0x0002
  #define SP_IEXTEN                  0x8000
  #define SP_ISIG                    0x0001
  #define SP_VTIME                   6
  #define SP_VMIN                    7
  #define SP_TCSANOW                 0
  #define SP_TCSADRAIN               1
  #define SP_TCSAFLUSH               2
  #define SP_TCGETS                  0x5401
  #define SP_TCSETS                  0x5402
  #define SP_TIOCGWINSZ              0x5413

#elif defined(SP_WIN32)
  #define SP_O_RDONLY             _O_RDONLY
  #define SP_O_WRONLY             _O_WRONLY
  #define SP_O_RDWR               _O_RDWR
  #define SP_O_CREAT              _O_CREAT
  #define SP_O_EXCL               _O_EXCL
  #define SP_O_TRUNC              _O_TRUNC
  #define SP_O_APPEND             _O_APPEND
  #define SP_O_BINARY             _O_BINARY

  #define SP_SEEK_SET             SEEK_SET
  #define SP_SEEK_CUR             SEEK_CUR
  #define SP_SEEK_END             SEEK_END
  #define SP_EINTR                EINTR

  #define SP_CLOCK_REALTIME       0
  #define SP_CLOCK_MONOTONIC      1

#elif defined(SP_MACOS) || defined(SP_COSMO)
  #define SP_EINTR                EINTR

  #define SP_AT_FDCWD             AT_FDCWD
  #define SP_AT_SYMLINK_NOFOLLOW  AT_SYMLINK_NOFOLLOW
  #define SP_AT_REMOVEDIR         AT_REMOVEDIR
  #define SP_AT_SYMLINK_FOLLOW    AT_SYMLINK_FOLLOW
  #define SP_AT_EACCESS           AT_EACCESS

  #define SP_O_RDONLY             O_RDONLY
  #define SP_O_WRONLY             O_WRONLY
  #define SP_O_RDWR               O_RDWR
  #define SP_O_CREAT              O_CREAT
  #define SP_O_EXCL               O_EXCL
  #define SP_O_TRUNC              O_TRUNC
  #define SP_O_APPEND             O_APPEND
  #define SP_O_NONBLOCK           O_NONBLOCK
  #define SP_O_CLOEXEC            O_CLOEXEC
  #define SP_O_BINARY             0

  #define SP_SEEK_SET             SEEK_SET
  #define SP_SEEK_CUR             SEEK_CUR
  #define SP_SEEK_END             SEEK_END

  #define SP_F_DUPFD              F_DUPFD
  #define SP_F_GETFD              F_GETFD
  #define SP_F_SETFD              F_SETFD
  #define SP_F_GETFL              F_GETFL
  #define SP_F_SETFL              F_SETFL
  #define SP_FD_CLOEXEC           FD_CLOEXEC

  #define SP_PROT_NONE            PROT_NONE
  #define SP_PROT_READ            PROT_READ
  #define SP_PROT_WRITE           PROT_WRITE
  #define SP_PROT_EXEC            PROT_EXEC

  #define SP_MAP_SHARED           MAP_SHARED
  #define SP_MAP_PRIVATE          MAP_PRIVATE
  #define SP_MAP_FIXED            MAP_FIXED
  #define SP_MAP_ANONYMOUS        MAP_ANONYMOUS
  #define SP_MAP_FAILED           MAP_FAILED

  #define SP_DT_UNKNOWN           DT_UNKNOWN
  #define SP_DT_FIFO              DT_FIFO
  #define SP_DT_CHR               DT_CHR
  #define SP_DT_DIR               DT_DIR
  #define SP_DT_BLK               DT_BLK
  #define SP_DT_REG               DT_REG
  #define SP_DT_LNK               DT_LNK
  #define SP_DT_SOCK              DT_SOCK

  #define SP_IN_ACCESS        IN_ACCESS
  #define SP_IN_MODIFY        IN_MODIFY
  #define SP_IN_ATTRIB        IN_ATTRIB
  #define SP_IN_CLOSE_WRITE   IN_CLOSE_WRITE
  #define SP_IN_CLOSE_NOWRITE IN_CLOSE_NOWRITE
  #define SP_IN_OPEN          IN_OPEN
  #define SP_IN_MOVED_FROM    IN_MOVED_FROM
  #define SP_IN_MOVED_TO      IN_MOVED_TO
  #define SP_IN_CREATE        IN_CREATE
  #define SP_IN_DELETE        IN_DELETE
  #define SP_IN_DELETE_SELF   IN_DELETE_SELF
  #define SP_IN_MOVE_SELF     IN_MOVE_SELF
  #define SP_IN_NONBLOCK      IN_NONBLOCK
  #define SP_IN_CLOEXEC       IN_CLOEXEC

  #define SP_CLOCK_REALTIME          CLOCK_REALTIME
  #define SP_CLOCK_MONOTONIC         CLOCK_MONOTONIC
  #define SP_TIMER_ABSTIME           TIMER_ABSTIME

  #define SP_BRKINT                  BRKINT
  #define SP_ICRNL                   ICRNL
  #define SP_INPCK                   INPCK
  #define SP_ISTRIP                  ISTRIP
  #define SP_IXON                    IXON
  #define SP_OPOST                   OPOST
  #define SP_CS8                     CS8
  #define SP_ECHO                    ECHO
  #define SP_ICANON                  ICANON
  #define SP_IEXTEN                  IEXTEN
  #define SP_ISIG                    ISIG
  #define SP_VTIME                   VTIME
  #define SP_VMIN                    VMIN
  #define SP_TCSANOW                 TCSANOW
  #define SP_TCSADRAIN               TCSADRAIN
  #define SP_TCSAFLUSH               TCSAFLUSH
  #define SP_TCGETS                  TCGETS
  #define SP_TCSETS                  TCSETS
  #define SP_TIOCGWINSZ              TIOCGWINSZ
#endif

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

#define sp_ansi_u8_to_str(__code) "\033[" sp_mstr(__code) "m"

#define SP_ANSI_RESET_U8             0
#define SP_ANSI_BOLD_U8              1
#define SP_ANSI_DIM_U8               2
#define SP_ANSI_ITALIC_U8            3
#define SP_ANSI_UNDERLINE_U8         4
#define SP_ANSI_BLINK_U8             5
#define SP_ANSI_REVERSE_U8           7
#define SP_ANSI_HIDDEN_U8            8
#define SP_ANSI_STRIKETHROUGH_U8     9
#define SP_ANSI_FG_BLACK_U8          30
#define SP_ANSI_FG_RED_U8            31
#define SP_ANSI_FG_GREEN_U8          32
#define SP_ANSI_FG_YELLOW_U8         33
#define SP_ANSI_FG_BLUE_U8           34
#define SP_ANSI_FG_MAGENTA_U8        35
#define SP_ANSI_FG_CYAN_U8           36
#define SP_ANSI_FG_WHITE_U8          37
#define SP_ANSI_BG_BLACK_U8          40
#define SP_ANSI_BG_RED_U8            41
#define SP_ANSI_BG_GREEN_U8          42
#define SP_ANSI_BG_YELLOW_U8         43
#define SP_ANSI_BG_BLUE_U8           44
#define SP_ANSI_BG_MAGENTA_U8        45
#define SP_ANSI_BG_CYAN_U8           46
#define SP_ANSI_BG_WHITE_U8          47
#define SP_ANSI_FG_BRIGHT_BLACK_U8   90
#define SP_ANSI_FG_BRIGHT_RED_U8     91
#define SP_ANSI_FG_BRIGHT_GREEN_U8   92
#define SP_ANSI_FG_BRIGHT_YELLOW_U8  93
#define SP_ANSI_FG_BRIGHT_BLUE_U8    94
#define SP_ANSI_FG_BRIGHT_MAGENTA_U8 95
#define SP_ANSI_FG_BRIGHT_CYAN_U8    96
#define SP_ANSI_FG_BRIGHT_WHITE_U8   97
#define SP_ANSI_BG_BRIGHT_BLACK_U8   100
#define SP_ANSI_BG_BRIGHT_RED_U8     101
#define SP_ANSI_BG_BRIGHT_GREEN_U8   102
#define SP_ANSI_BG_BRIGHT_YELLOW_U8  103
#define SP_ANSI_BG_BRIGHT_BLUE_U8    104
#define SP_ANSI_BG_BRIGHT_MAGENTA_U8 105
#define SP_ANSI_BG_BRIGHT_CYAN_U8    106
#define SP_ANSI_BG_BRIGHT_WHITE_U8   107

#define SP_ANSI_RESET             sp_ansi_u8_to_str(SP_ANSI_RESET_U8)
#define SP_ANSI_BOLD              sp_ansi_u8_to_str(SP_ANSI_BOLD_U8)
#define SP_ANSI_DIM               sp_ansi_u8_to_str(SP_ANSI_DIM_U8)
#define SP_ANSI_ITALIC            sp_ansi_u8_to_str(SP_ANSI_ITALIC_U8)
#define SP_ANSI_UNDERLINE         sp_ansi_u8_to_str(SP_ANSI_UNDERLINE_U8)
#define SP_ANSI_BLINK             sp_ansi_u8_to_str(SP_ANSI_BLINK_U8)
#define SP_ANSI_REVERSE           sp_ansi_u8_to_str(SP_ANSI_REVERSE_U8)
#define SP_ANSI_HIDDEN            sp_ansi_u8_to_str(SP_ANSI_HIDDEN_U8)
#define SP_ANSI_STRIKETHROUGH     sp_ansi_u8_to_str(SP_ANSI_STRIKETHROUGH_U8)
#define SP_ANSI_FG_BLACK          sp_ansi_u8_to_str(SP_ANSI_FG_BLACK_U8)
#define SP_ANSI_FG_RED            sp_ansi_u8_to_str(SP_ANSI_FG_RED_U8)
#define SP_ANSI_FG_GREEN          sp_ansi_u8_to_str(SP_ANSI_FG_GREEN_U8)
#define SP_ANSI_FG_YELLOW         sp_ansi_u8_to_str(SP_ANSI_FG_YELLOW_U8)
#define SP_ANSI_FG_BLUE           sp_ansi_u8_to_str(SP_ANSI_FG_BLUE_U8)
#define SP_ANSI_FG_MAGENTA        sp_ansi_u8_to_str(SP_ANSI_FG_MAGENTA_U8)
#define SP_ANSI_FG_CYAN           sp_ansi_u8_to_str(SP_ANSI_FG_CYAN_U8)
#define SP_ANSI_FG_WHITE          sp_ansi_u8_to_str(SP_ANSI_FG_WHITE_U8)
#define SP_ANSI_BG_BLACK          sp_ansi_u8_to_str(SP_ANSI_BG_BLACK_U8)
#define SP_ANSI_BG_RED            sp_ansi_u8_to_str(SP_ANSI_BG_RED_U8)
#define SP_ANSI_BG_GREEN          sp_ansi_u8_to_str(SP_ANSI_BG_GREEN_U8)
#define SP_ANSI_BG_YELLOW         sp_ansi_u8_to_str(SP_ANSI_BG_YELLOW_U8)
#define SP_ANSI_BG_BLUE           sp_ansi_u8_to_str(SP_ANSI_BG_BLUE_U8)
#define SP_ANSI_BG_MAGENTA        sp_ansi_u8_to_str(SP_ANSI_BG_MAGENTA_U8)
#define SP_ANSI_BG_CYAN           sp_ansi_u8_to_str(SP_ANSI_BG_CYAN_U8)
#define SP_ANSI_BG_WHITE          sp_ansi_u8_to_str(SP_ANSI_BG_WHITE_U8)
#define SP_ANSI_FG_BRIGHT_BLACK   sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_BLACK_U8)
#define SP_ANSI_FG_BRIGHT_RED     sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_RED_U8)
#define SP_ANSI_FG_BRIGHT_GREEN   sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_GREEN_U8)
#define SP_ANSI_FG_BRIGHT_YELLOW  sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_YELLOW_U8)
#define SP_ANSI_FG_BRIGHT_BLUE    sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_BLUE_U8)
#define SP_ANSI_FG_BRIGHT_MAGENTA sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_MAGENTA_U8)
#define SP_ANSI_FG_BRIGHT_CYAN    sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_CYAN_U8)
#define SP_ANSI_FG_BRIGHT_WHITE   sp_ansi_u8_to_str(SP_ANSI_FG_BRIGHT_WHITE_U8)
#define SP_ANSI_BG_BRIGHT_BLACK   sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_BLACK_U8)
#define SP_ANSI_BG_BRIGHT_RED     sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_RED_U8)
#define SP_ANSI_BG_BRIGHT_GREEN   sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_GREEN_U8)
#define SP_ANSI_BG_BRIGHT_YELLOW  sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_YELLOW_U8)
#define SP_ANSI_BG_BRIGHT_BLUE    sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_BLUE_U8)
#define SP_ANSI_BG_BRIGHT_MAGENTA sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_MAGENTA_U8)
#define SP_ANSI_BG_BRIGHT_CYAN    sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_CYAN_U8)
#define SP_ANSI_BG_BRIGHT_WHITE   sp_ansi_u8_to_str(SP_ANSI_BG_BRIGHT_WHITE_U8)


//   █████████  █████ █████  █████████
//  ███░░░░░███░░███ ░░███  ███░░░░░███
// ░███    ░░░  ░░███ ███  ░███    ░░░
// ░░█████████   ░░█████   ░░█████████
//  ░░░░░░░░███   ░░███     ░░░░░░░░███
//  ███    ░███    ░███     ███    ░███
// ░░█████████     █████   ░░█████████
//  ░░░░░░░░░     ░░░░░     ░░░░░░░░░
// @sys

// @spader
// These types should be the same in the public API; we can obviously typedef to platform types in the implementation
// section, but they shouldn't be here
#if defined(SP_WIN32)
  typedef intptr_t sp_sys_fd_t;
  #define SP_SYS_INVALID_FD ((sp_sys_fd_t)-1)
  #define sp_sys_stdin  ((sp_sys_fd_t)GetStdHandle(STD_INPUT_HANDLE))
  #define sp_sys_stdout ((sp_sys_fd_t)GetStdHandle(STD_OUTPUT_HANDLE))
  #define sp_sys_stderr ((sp_sys_fd_t)GetStdHandle(STD_ERROR_HANDLE))
#else
  typedef s32 sp_sys_fd_t;
  #define SP_SYS_INVALID_FD ((sp_sys_fd_t)-1)
  #define sp_sys_stdin  ((sp_sys_fd_t)0)
  #define sp_sys_stdout ((sp_sys_fd_t)1)
  #define sp_sys_stderr ((sp_sys_fd_t)2)
#endif

#if defined(SP_WIN32)
  typedef struct {
    DWORD input_mode;
    DWORD output_mode;
  } sp_tty_mode_t;
#elif defined(SP_LINUX)
  typedef struct {
    u32 c_iflag;
    u32 c_oflag;
    u32 c_cflag;
    u32 c_lflag;
    u8  c_cc[20];
    u32 _c_ispeed;
    u32 _c_ospeed;
  } sp_sys_termios_t;

  typedef sp_sys_termios_t sp_tty_mode_t;
#elif defined(SP_MACOS) || defined(SP_COSMO)
  typedef struct termios sp_tty_mode_t;
#elif defined(SP_WASM)
  typedef struct {
    u32 dummy;
  } sp_tty_mode_t;

#endif

typedef struct {
  s64 tv_sec;
  s64 tv_nsec;
} sp_sys_timespec_t;

typedef enum {
  SP_FS_KIND_NONE = 0,
  SP_FS_KIND_FILE,
  SP_FS_KIND_DIR,
  SP_FS_KIND_SYMLINK,
} sp_fs_kind_t;

typedef struct {
  sp_fs_kind_t      kind;
  s64               size;
  sp_sys_timespec_t atime;
  sp_sys_timespec_t mtime;
  sp_sys_timespec_t btime;
  u64               id;
  u64               device;
  u64               nlink;
  u32               raw_attrs;
} sp_sys_stat_t;

sp_typedef_fn(int, sp_qsort_fn_t, const void *, const void *);

s64         sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count);
s64         sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count);
sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode);
s32         sp_sys_close(sp_sys_fd_t fd);
s32         sp_sys_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end);
s64         sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence);
s32         sp_sys_stat(const c8* path, sp_sys_stat_t* st);
s32         sp_sys_lstat(const c8* path, sp_sys_stat_t* st);
s32         sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st);
s32         sp_sys_mkdir(const c8* path, s32 mode);
s32         sp_sys_rmdir(const c8* path);
s32         sp_sys_unlink(const c8* path);
s32         sp_sys_rename(const c8* oldpath, const c8* newpath);
s32         sp_sys_chdir(const c8* path);
s64         sp_sys_getcwd(char* buf, u64 size);
s32         sp_sys_link(const c8* oldpath, const c8* newpath);
s32         sp_sys_symlink(const c8* target, const c8* linkpath);
s32         sp_sys_chmod(const c8* path, const sp_sys_stat_t* st);
s32         sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts);
s32         sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem);
s64         sp_sys_canonicalize_path(const c8* path, c8* buf, u64 size);
s64         sp_sys_get_exe_path(c8* buf, u64 size);
s64         sp_sys_get_storage_path(c8* buf, u64 size);
s64         sp_sys_get_config_path(c8* buf, u64 size);
s32         sp_sys_fd_ready(sp_sys_fd_t fd, u8* ready);
s32         sp_sys_fd_wait(sp_sys_fd_t fd);
s32         sp_sys_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds);
void*       sp_sys_alloc(u64 size);
void        sp_sys_free(void* ptr, u64 size);
void*       sp_sys_memcpy(void* dest, const void* src, u64 n);
void*       sp_sys_memmove(void* dest, const void* src, u64 n);
void*       sp_sys_memset(void* dest, s32 c, u64 n);
s32         sp_sys_memcmp(const void* a, const void* b, u64 n);
void        sp_sys_assert(const c8* file, const c8* line, const c8* func, const c8* expr, bool cond);


//  ██████   ██████   █████████   █████ ██████   █████
// ░░██████ ██████   ███░░░░░███ ░░███ ░░██████ ░░███
//  ░███░█████░███  ░███    ░███  ░███  ░███░███ ░███
//  ░███░░███ ░███  ░███████████  ░███  ░███░░███░███
//  ░███ ░░░  ░███  ░███░░░░░███  ░███  ░███ ░░██████
//  ░███      ░███  ░███    ░███  ░███  ░███  ░░█████
//  █████     █████ █████   █████ █████ █████  ░░█████
// ░░░░░     ░░░░░ ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░
// @main
typedef s32 (*sp_entry_fn_t)(s32, const c8**);

#define SP_MAIN_FREESTANDING_AMD64(fn) \
  __attribute__((naked))               \
  void _start() {                      \
    __asm__ volatile (                 \
      /* The SysV ABI requires that we zero the frame pointer to mark the outermost stack frame (e.g. for a debugger) */ \
      "xor %%rbp, %%rbp"          "\n" \
      /*
       * Next, we need to set up the arguments for sp_main(). There are three:
       * - The number of command line args (u32 num_args)
       * - An array of command line args (c8** args)
       * - The function the user gave us to use as the entry point
       *
       * The kernel gives us the first two by putting them contiguously in memory and putting the
       * base address in rsp. We just need to move them to rdi and rsi, the argument registers, so
       * that sp_main() receives them as arguments. Here's the stack layout:
       *
       *   rsp[0] == num_args (e.g. 3)
       *   rsp[1] == args[0]  (e.g. 0xDEADBEEF -> "hello")
       *   rsp[2] == args[1]  (e.g. 0xCAFEBABE -> "world")
       *   ...
       *
       * For the function, we can't just use the function pointer as an immediate for the third argument. It would
       * force a relocation if compiled as PIE, but since this is freestanding there's nothing to apply the
       * relocation.
       *
       * Instead, we use `lea fn(rip)` to encode it as an offset from the instruction pointer, which avoids
       * a relocation entirely.
       */ \
      "mov (%%rsp),        %%rdi" "\n" \
      "lea 8(%%rsp),       %%rsi" "\n" \
      "lea " #fn "(%%rip), %%rdx" "\n" \
      /*
       * Now that the arguments are set up, call into our entry point, sp_main(). Freestanding builds require a few
       * things to be initialized; we could do that initialization here and jump directly to the user's function,
       * but I can barely write Assembly as it is, so I prefer to just write it in C.
       */ \
      "call sp_main"              "\n" \
      ::: "memory"                     \
    );                                 \
  }

#define SP_MAIN_FREESTANDING_ARM64(fn) \
  __attribute__((naked))               \
  void _start() {                      \
    __asm__ volatile (                 \
      /* Same as x86_64, pretty much. Load the arguments. */ \
      "ldr x0, [sp]"              "\n" \
      "add x1, sp, #8"            "\n" \
      /* Calculate the function pointer as relative to the instruction pointer */ \
      "adrp x2, " #fn             "\n" \
      "add x2, x2, :lo12:" #fn    "\n" \
      "bl sp_main"                "\n" \
      ::: "memory"                     \
    ); \
  }

#define SP_MAIN_WASM_FREESTANDING(fn)                              \
  __attribute__((export_name("_start")))                           \
  void _start() {                                                  \
    struct { uint8_t** data; __wasi_size_t len; } args;            \
    struct { uint8_t** data; __wasi_size_t len; } env;             \
    struct {                                                       \
      struct { uint8_t* data; __wasi_size_t len; } args;           \
      struct { uint8_t* data; __wasi_size_t len; } env;            \
    } buffers;                                                     \
                                                                   \
    (void)__wasi_args_sizes_get(&args.len, &buffers.args.len);     \
    args.data = sp_sys_alloc((args.len + 1) * sizeof(uint8_t*));   \
    buffers.args.data = sp_sys_alloc(buffers.args.len);            \
    (void)__wasi_args_get(args.data, buffers.args.data);           \
                                                                   \
    (void)__wasi_environ_sizes_get(&env.len, &buffers.env.len);    \
    env.data = sp_sys_alloc((env.len + 1) * sizeof(uint8_t*));     \
    buffers.env.data = sp_sys_alloc(buffers.env.len);              \
    (void)__wasi_environ_get(env.data, buffers.env.data);          \
                                                                   \
    environ = (c8**)env.data;                                \
    sp_main((s32)args.len, (const c8**)args.data, fn);             \
  }

#define SP_MAIN_HOSTED(fn) \
  s32 main(s32 num_args, const c8** args) { \
    return fn(num_args, args); \
  }

#if defined(SP_FREESTANDING) && defined(SP_AMD64)
  #define SP_MAIN(fn) SP_MAIN_FREESTANDING_AMD64(fn)
#elif defined(SP_FREESTANDING) && defined(SP_ARM64)
  #define SP_MAIN(fn) SP_MAIN_FREESTANDING_ARM64(fn)
#elif defined(SP_WASM_FREESTANDING)
  #define SP_MAIN(fn) SP_MAIN_WASM_FREESTANDING(fn)
#else
  #define SP_MAIN(fn) SP_MAIN_HOSTED(fn)
#endif

#if defined(SP_FREESTANDING)
  extern c8** environ;
  extern s32 errno;
#elif defined(SP_WASM)
  extern c8** environ;
#endif


//  ██████████ ███████████   ███████████      ███████    ███████████
// ░░███░░░░░█░░███░░░░░███ ░░███░░░░░███   ███░░░░░███ ░░███░░░░░███
//  ░███  █ ░  ░███    ░███  ░███    ░███  ███     ░░███ ░███    ░███
//  ░██████    ░██████████   ░██████████  ░███      ░███ ░██████████
//  ░███░░█    ░███░░░░░███  ░███░░░░░███ ░███      ░███ ░███░░░░░███
//  ░███ ░   █ ░███    ░███  ░███    ░███ ░░███     ███  ░███    ░███
//  ██████████ █████   █████ █████   █████ ░░░███████░   █████   █████
// ░░░░░░░░░░ ░░░░░   ░░░░░ ░░░░░   ░░░░░    ░░░░░░░    ░░░░░   ░░░░░
// @error


//  ██████   ██████ ██████████ ██████   ██████    ███████    ███████████   █████ █████
// ░░██████ ██████ ░░███░░░░░█░░██████ ██████   ███░░░░░███ ░░███░░░░░███ ░░███ ░░███
//  ░███░█████░███  ░███  █ ░  ░███░█████░███  ███     ░░███ ░███    ░███  ░░███ ███
//  ░███░░███ ░███  ░██████    ░███░░███ ░███ ░███      ░███ ░██████████    ░░█████
//  ░███ ░░░  ░███  ░███░░█    ░███ ░░░  ░███ ░███      ░███ ░███░░░░░███    ░░███
//  ░███      ░███  ░███ ░   █ ░███      ░███ ░░███     ███  ░███    ░███     ░███
//  █████     █████ ██████████ █████     █████ ░░░███████░   █████   █████    █████
// ░░░░░     ░░░░░ ░░░░░░░░░░ ░░░░░     ░░░░░    ░░░░░░░    ░░░░░   ░░░░░    ░░░░░
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
  void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr
);

typedef struct sp_allocator_t {
  sp_allocator_fn_t on_alloc;
  void* user_data;
} sp_mem_t;

typedef enum {
  SP_MEM_ARENA_MODE_DEFAULT,
  SP_MEM_ARENA_MODE_NO_REALLOC,
} sp_mem_arena_mode_t;

typedef struct sp_mem_arena_block_t {
  struct sp_mem_arena_block_t* next;
  u8* buffer;
  u64 capacity;
  u64 bytes_used;
} sp_mem_arena_block_t;

typedef struct {
  sp_mem_t allocator;
  sp_mem_arena_block_t* head;
  sp_mem_arena_block_t* current;
  u64 block_size;
  sp_mem_arena_mode_t mode;
  u8 alignment;
} sp_mem_arena_t;

typedef struct sp_aligned() {
  u64 size;
} sp_mem_os_header_t;

typedef struct {
  sp_mem_arena_t* arena;
  sp_mem_arena_block_t* block;
  u64 mark;
  sp_mem_t mem;
} sp_mem_arena_marker_t;

typedef struct {
  sp_mem_arena_marker_t marker;
} sp_mem_scratch_t;

SP_API void*                 sp_alloc(u64 size);
SP_API void*                 sp_realloc(void* memory, u64 size);
SP_API void                  sp_free(void* memory);
SP_API void                  sp_mem_copy(const void* source, void* dest, u64 num_bytes);
SP_API void                  sp_mem_move(const void* source, void* dest, u64 num_bytes);
SP_API bool                  sp_mem_is_equal(const void* a, const void* b, u64 len);
SP_API void                  sp_mem_fill(void* buffer, u64 bsize, void* fill, u64 fsize);
SP_API void                  sp_mem_fill_u8(void* buffer, u64 buffer_size, u8 fill);
SP_API void                  sp_mem_zero(void* buffer, u64 buffer_size);
SP_API void*                 sp_mem_allocator_alloc(sp_mem_t arena, u64 size);
SP_API void*                 sp_mem_allocator_realloc(sp_mem_t arena, void* ptr, u64 size);
SP_API void                  sp_mem_allocator_free(sp_mem_t arena, void* buffer);
SP_API void*                 sp_mem_os_alloc(u64 size);
SP_API void*                 sp_mem_os_alloc_zero(u64 size);
SP_API void*                 sp_mem_os_realloc(void* ptr, u64 size);
SP_API void                  sp_mem_os_free(void* ptr);
SP_API void*                 sp_mem_os_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u64 size, void* ptr);
SP_API sp_mem_os_header_t*   sp_mem_os_get_header(void* ptr);
SP_API sp_mem_arena_t*       sp_mem_arena_new();
SP_API sp_mem_arena_t*       sp_mem_arena_new_ex(u64 block_size, sp_mem_arena_mode_t mode, u8 alignment);
SP_API sp_mem_arena_t*       sp_mem_arena_new_with_allocator(sp_mem_t allocator, u64 block_size, sp_mem_arena_mode_t mode, u8 alignment);
SP_API void                  sp_mem_arena_clear(sp_mem_arena_t* arena);
SP_API void                  sp_mem_arena_destroy(sp_mem_arena_t* arena);
SP_API void*                 sp_mem_arena_on_alloc(void* ptr, sp_mem_alloc_mode_t mode, u64 n, void* old);
SP_API sp_mem_arena_marker_t sp_mem_arena_mark(sp_mem_arena_t* a);
SP_API void                  sp_mem_arena_pop(sp_mem_arena_marker_t marker);
SP_API u64                   sp_mem_arena_capacity(sp_mem_arena_t* arena);
SP_API u64                   sp_mem_arena_bytes_used(sp_mem_arena_t* arena);
SP_API void*                 sp_mem_arena_alloc(sp_mem_arena_t* arena, u64 size);
SP_API void*                 sp_mem_arena_realloc(sp_mem_arena_t* arena, void* ptr, u64 size);
SP_API void                  sp_mem_arena_free(sp_mem_arena_t* arena, void* ptr);
SP_API sp_mem_arena_t*       sp_mem_get_scratch_arena();
SP_API sp_mem_scratch_t      sp_mem_begin_scratch();
SP_API void                  sp_mem_end_scratch(sp_mem_scratch_t scratch);

#define SP_ALLOC(T) (T*)sp_alloc(sizeof(T))
#define SP_ALLOC_N(T, n) (T*)sp_alloc((n) * sizeof(T))
#define sp_alloc_n(T, n) (T*)sp_alloc((n) * sizeof(T))
#define sp_alloc_type(T) sp_alloc_n(T, 1)
#define sp_sys_alloc_n(T, n) (T*)sp_sys_alloc((n) * sizeof(T))
#define sp_sys_alloc_type(T) sp_sys_alloc_n(T, 1)
#define sp_mem_allocator_alloc_n(a, T, n) (T*)sp_mem_allocator_alloc(a, (n) * sizeof(T))
#define sp_mem_allocator_alloc_type(a, T) sp_mem_allocator_alloc_n(a, T, 1)
#define sp_mem_arena_alloc_n(a, T, n) (T*)sp_mem_arena_alloc((a), (n) * sizeof(T))
#define sp_mem_arena_alloc_type(a, T) sp_mem_arena_alloc_n(a, T, 1)


typedef struct {
  u8* data;
  u64 len;
  u64 capacity;
} sp_mem_buffer_t;

typedef struct {
  u8* data;
  u64 len;
} sp_mem_slice_t;

typedef struct {
  sp_mem_slice_t slice;
  u64 index;
  u8 byte;
} sp_mem_slice_it_t;

#define sp_mem_slice_for(slice, it) for (u64 it = 0; it < (slice).len; it++)
#define sp_mem_slice_for_it(slice, it) for (sp_mem_slice_it_t it = sp_mem_slice_it(slice); sp_mem_slice_it_valid(&it); sp_mem_slice_it_next(&it))

SP_API sp_mem_slice_t    sp_mem_slice(u8* ptr, u64 len);
SP_API sp_mem_slice_t    sp_mem_slice_copy(sp_mem_slice_t slice);
SP_API sp_mem_slice_t    sp_mem_slice_sub(sp_mem_slice_t slice, u64 start, u64 len);
SP_API sp_mem_slice_t    sp_mem_slice_prefix(sp_mem_slice_t slice, u64 len);
SP_API sp_mem_slice_t    sp_mem_slice_suffix(sp_mem_slice_t slice, u64 len);
SP_API bool              sp_mem_slice_empty(sp_mem_slice_t slice);
SP_API u8                sp_mem_slice_at(sp_mem_slice_t slice, u64 index);
SP_API sp_mem_slice_it_t sp_mem_slice_it(sp_mem_slice_t slice);
SP_API bool              sp_mem_slice_it_valid(sp_mem_slice_it_t* it);
SP_API void              sp_mem_slice_it_next(sp_mem_slice_it_t* it);
SP_API sp_str_t          sp_mem_buffer_as_str(sp_mem_buffer_t* buffer);
SP_API c8*               sp_mem_buffer_as_cstr(sp_mem_buffer_t* buffer);



//  █████   █████   █████████    █████████  █████   █████
// ░░███   ░░███   ███░░░░░███  ███░░░░░███░░███   ░░███
//  ░███    ░███  ░███    ░███ ░███    ░░░  ░███    ░███
//  ░███████████  ░███████████ ░░█████████  ░███████████
//  ░███░░░░░███  ░███░░░░░███  ░░░░░░░░███ ░███░░░░░███
//  ░███    ░███  ░███    ░███  ███    ░███ ░███    ░███
//  █████   █████ █████   █████░░█████████  █████   █████
// ░░░░░   ░░░░░ ░░░░░   ░░░░░  ░░░░░░░░░  ░░░░░   ░░░░░
// @hash
typedef u64 sp_hash_t;

SP_API sp_hash_t sp_hash_cstr(const c8* str);
SP_API sp_hash_t sp_hash_combine(sp_hash_t* hashes, u32 num_hashes);
SP_API sp_hash_t sp_hash_bytes(const void* p, u64 len, u64 seed);


//  ██████████   █████ █████ ██████   █████      █████████   ███████████   ███████████     █████████   █████ █████
// ░░███░░░░███ ░░███ ░░███ ░░██████ ░░███      ███░░░░░███ ░░███░░░░░███ ░░███░░░░░███   ███░░░░░███ ░░███ ░░███
//  ░███   ░░███ ░░███ ███   ░███░███ ░███     ░███    ░███  ░███    ░███  ░███    ░███  ░███    ░███  ░░███ ███
//  ░███    ░███  ░░█████    ░███░░███░███     ░███████████  ░██████████   ░██████████   ░███████████   ░░█████
//  ░███    ░███   ░░███     ░███ ░░██████     ░███░░░░░███  ░███░░░░░███  ░███░░░░░███  ░███░░░░░███    ░░███
//  ░███    ███     ░███     ░███  ░░█████     ░███    ░███  ░███    ░███  ░███    ░███  ░███    ░███     ░███
//  ██████████      █████    █████  ░░█████    █████   █████ █████   █████ █████   █████ █████   █████    █████
// ░░░░░░░░░░      ░░░░░    ░░░░░    ░░░░░    ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░   ░░░░░    ░░░░░
// @array @dyn_array @da

typedef struct sp_aligned() {
  u64 size;
  u64 capacity;
  sp_mem_t allocator;
} sp_da_header_t;

#define sp_da(T) T*
SP_API void* sp_da_resize(void* arr, u32 stride, u64 len);
SP_API void* sp_da_grow_ex(void* arr, u32 stride, u64 addlen);
SP_API void  sp_da_push_ex(void** arr, void* val, u32 stride);

#define sp_da_for(__ARR, __IT)  for (u64 __IT = 0; __IT < sp_da_size((__ARR)); __IT++)
#define sp_da_rfor(__ARR, __IT) for (u64 __IT = sp_da_size(__ARR); __IT-- > 0; )

#define sp_da_head(__ARR)\
  ((sp_da_header_t*)((u8*)(__ARR) - sizeof(sp_da_header_t)))

#define sp_da_size(__ARR)\
  (__ARR ? sp_da_head((__ARR))->size : 0)

#define sp_da_allocator(__arr) \
  (__arr ? sp_da_head((__arr))->allocator : sp_zero_struct(sp_mem_t))

#define sp_da_stride(__ARR) \
  sizeof(*(__ARR))

#define sp_da_capacity(__ARR)\
  (__ARR ? sp_da_head(__ARR)->capacity : 0)

#define sp_da_empty(__ARR)\
  (sp_da_size(__ARR) == 0)

#define sp_da_full(__ARR)\
  ((sp_da_size(__ARR) == sp_da_capacity(__ARR)))

#define sp_da_clear(__ARR) \
  sp_unused((__ARR) ? sp_da_head(__ARR)->size = 0 : 0)

#define sp_da_vp(__arr) \
  ((void**)&(__arr))

#define sp_da_init(__a, __arr) \
  sp_da_init_ex((__a), sp_da_vp(__arr), sp_da_stride(__arr))

#define sp_da_free(__ARR)         \
  do {                            \
    if (__ARR) {                  \
      sp_free(sp_da_head(__ARR)); \
      (__ARR) = NULL;\
    }\
  } while (0)

#define sp_da_grow(__ARR, __N)\
  sp_da_grow_ex((__ARR), sp_da_stride(__ARR), (__N))

#define sp_da_push(__ARR, __VAL)\
    do {\
        *((void **)&(__ARR)) = sp_da_grow(__ARR, 1);\
        (__ARR)[sp_da_head(__ARR)->size++] = (__VAL);\
    } while(0)

#define sp_da_reserve(__ARR, __AMOUNT)\
    do {\
        if ((u64)(__AMOUNT) > sp_da_capacity(__ARR)) {\
            *((void **)&(__ARR)) = sp_da_resize(__ARR, sizeof(*(__ARR)), __AMOUNT);\
        }\
    } while(0)

#define sp_da_pop(__ARR)\
    do {\
        if (__ARR && !sp_da_empty(__ARR)) {\
            sp_da_head(__ARR)->size -= 1;\
        }\
    } while (0)

#define sp_da_back(__ARR)\
    (__ARR + (sp_da_size(__ARR) ? sp_da_size(__ARR) - 1 : 0))

#define sp_da_sort(arr, fn) sp_os_qsort(arr, sp_da_size(arr), sizeof((arr)[0]), fn)
#define sp_da_bounds_ok(arr, it) ((it) < sp_da_size(arr))


//  ███████████   █████ ██████   █████   █████████     ███████████  █████  █████ ███████████ ███████████ ██████████ ███████████
// ░░███░░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███   ░░███░░░░░███░░███  ░░███ ░░███░░░░░░█░░███░░░░░░█░░███░░░░░█░░███░░░░░███
//  ░███    ░███  ░███  ░███░███ ░███  ███     ░░░     ░███    ░███ ░███   ░███  ░███   █ ░  ░███   █ ░  ░███  █ ░  ░███    ░███
//  ░██████████   ░███  ░███░░███░███ ░███             ░██████████  ░███   ░███  ░███████    ░███████    ░██████    ░██████████
//  ░███░░░░░███  ░███  ░███ ░░██████ ░███    █████    ░███░░░░░███ ░███   ░███  ░███░░░█    ░███░░░█    ░███░░█    ░███░░░░░███
//  ░███    ░███  ░███  ░███  ░░█████ ░░███  ░░███     ░███    ░███ ░███   ░███  ░███  ░     ░███  ░     ░███ ░   █ ░███    ░███
//  █████   █████ █████ █████  ░░█████ ░░█████████     ███████████  ░░████████   █████       █████       ██████████ █████   █████
// ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░     ░░░░░░░░░░░    ░░░░░░░░   ░░░░░       ░░░░░       ░░░░░░░░░░ ░░░░░   ░░░░░
// @ring_buffer @rb
typedef enum sp_rb_mode {
    SP_RQ_MODE_GROW = 0,
    SP_RQ_MODE_OVERWRITE,
} sp_rb_mode;

typedef struct sp_ring_buffer {
    u32 head;
    u32 size;
    u32 capacity;
    sp_rb_mode mode;
} sp_ring_buffer_t;

#define sp_rb(T) T*
SP_API void* sp_rb_grow_impl(void* arr, u32 elem_size, u32 new_cap);

#define sp_rb_head(__ARR)\
    ((sp_ring_buffer_t*)((u8*)(__ARR) - sizeof(sp_ring_buffer_t)))

#define sp_rb_size(__ARR)\
    ((__ARR) == SP_NULLPTR ? 0u : sp_rb_head((__ARR))->size)

#define sp_rb_capacity(__ARR)\
    ((__ARR) == SP_NULLPTR ? 0u : sp_rb_head((__ARR))->capacity)

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
            *((void**)&(__ARR)) = sp_rb_grow_impl(SP_NULLPTR, sizeof(*(__ARR)), 8u);\
        } else if (sp_rb_full(__ARR)) {\
            if (sp_rb_mode(__ARR) == SP_RQ_MODE_OVERWRITE) {\
                sp_rb_head(__ARR)->head = (sp_rb_head(__ARR)->head + 1) % sp_rb_capacity(__ARR);\
                sp_rb_head(__ARR)->size--;\
            } else {\
                *((void**)&(__ARR)) = sp_rb_grow_impl(__ARR, sizeof(*(__ARR)), sp_rb_capacity(__ARR) * 2u);\
            }\
        }\
        u32 __sp_rb_tail = (sp_rb_head(__ARR)->head + sp_rb_head(__ARR)->size) % sp_rb_capacity(__ARR);\
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

#define sp_rb_for(__ARR, __IT)  for (u32 __IT = 0; __IT < sp_rb_size(__ARR); __IT++)
#define sp_rb_rfor(__ARR, __IT) for (u32 __IT = sp_rb_size(__ARR); __IT-- > 0;)


//  █████   █████   █████████    █████████  █████   █████    ███████████   █████████   ███████████  █████       ██████████
// ░░███   ░░███   ███░░░░░███  ███░░░░░███░░███   ░░███    ░█░░░███░░░█  ███░░░░░███ ░░███░░░░░███░░███       ░░███░░░░░█
//  ░███    ░███  ░███    ░███ ░███    ░░░  ░███    ░███    ░   ░███  ░  ░███    ░███  ░███    ░███ ░███        ░███  █ ░
//  ░███████████  ░███████████ ░░█████████  ░███████████        ░███     ░███████████  ░██████████  ░███        ░██████
//  ░███░░░░░███  ░███░░░░░███  ░░░░░░░░███ ░███░░░░░███        ░███     ░███░░░░░███  ░███░░░░░███ ░███        ░███░░█
//  ░███    ░███  ░███    ░███  ███    ░███ ░███    ░███        ░███     ░███    ░███  ░███    ░███ ░███      █ ░███ ░   █
//  █████   █████ █████   █████░░█████████  █████   █████       █████    █████   █████ ███████████  ███████████ ██████████
// ░░░░░   ░░░░░ ░░░░░   ░░░░░  ░░░░░░░░░  ░░░░░   ░░░░░       ░░░░░    ░░░░░   ░░░░░ ░░░░░░░░░░░  ░░░░░░░░░░░ ░░░░░░░░░░
// @hash_table @ht
//
// sp_ht is a hash table in the spirit of Sean Barrett's stb_ds.h
// - Keys and values may be of any type (primitive scalars, structs, strings, etc.)
// - You can provide a custom hash() and compare() function
//
// # API
// Like everything in sp.h, hash tables must be zero initialized. Once declared,
// they can be used immediately, without further initialization.
//
//   sp_ht(s32, s32) ht = sp_zero()
//   sp_ht_insert(ht, 69, 420);
//   s32* value = sp_ht_get(ht, 69, 420);
//   sp_ht_erase(ht, 69);
//   sp_ht_free(ht);
//
// ## ITERATORS
// Hash tables also come with two kinds of iterators. The first, more convenient
// form provides an iterator struct with a typed key and value.
//
// sp_ht_for_kv(ht, it) {
//   s32* key = it.key;
//   s32* value = it.value;
// }
//
// This, however, depends on a GNU extension, so a plain iterator which must be
// resolved into a key and value is also provided:
//
// sp_ht_for(ht, it) {
//   s32* key = sp_ht_it_get(ht, it);
//   s32* value = sp_ht_it_getk(ht, it);
// }
//
// ## KEY TYPES
// Unlike stb_ds.h, sp_ht does not require a type with fields named "key" and "value". You
// declare a hash table with a complex key type the same as anything else:
//
//   typedef struct { u32 x; u32 y; } cell_t;
//   sp_ht(cell_t, f32) cells = sp_zero();
//
// For key types where memcmp() is not correct for hashing or comparison, you need to
// initialize the hash table with a hash() and compare() function:
//
//   sp_ht_set_fns(ht, on_hash, on_compare)
//
// This can very much be a footgun. To combat this, sp.h provides specializations for string
// (both sp_str_t and const c8*) keys which extend the lazy initialization to the hash and
// compare functions, too. The only functions which need to be specialized are those which
// actually perform the lazy initialization. See sp_str_ht and sp_cstr_ht.
//
// Practically, you'll only need these custom functions for two types of keys:
// 1. Strings
// 2. Structs into which the compiler inserts padding
//
// # TYPES
// sp_ht(K, V) defines a strongly-typed struct. A hash table is a pointer to such
// a struct which is lazily allocated on first use. Internally, a hash table is laid
// out array-of-structs style:
//
//    [KEY VALUE STATE] [KEY VALUE STATE] [KEY VALUE STATE]
//
// In other words, sp_ht() is not designed for high performance linear iteration
// over a tightly packed array of keys or values
#define SP_HT_HASH_SEED         0x31415296
#define SP_HT_INVALID_INDEX     UINT32_MAX

typedef u64 sp_ht_it_t;
SP_TYPEDEF_FN(sp_hash_t, sp_ht_hash_key_fn_t, void*, u64);
SP_TYPEDEF_FN(bool, sp_ht_compare_key_fn_t, void*, void*, u64);

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
  sp_mem_t allocator;
  u64 tmp_idx;
} sp_ht_info_t;

#if defined(SP_CPP)
SP_END_EXTERN_C()
template<typename T> static T* sp_ht_alloc_type(T* key, size_t size) {
  (void)key;
  return (T*)sp_alloc(size);
}
template<typename T> static T* sp_ht_alloc_type_a(sp_mem_t a, T* key, size_t size) {
  (void)key;
  return (T*)sp_mem_allocator_alloc(a, size);
}
SP_BEGIN_EXTERN_C()
#else
#define sp_ht_alloc_type(key, size) sp_ht_alloc(size)
#define sp_ht_alloc_type_a(a, key, size) sp_mem_allocator_alloc((a), (size))
#endif

#define __sp_ht_entry(__K, __V)                \
    struct                                     \
    {                                          \
        __K key;                               \
        __V val;                               \
        sp_ht_entry_state state;               \
    }

#define sp_ht_a(__K, __V)                        \
    struct {                                   \
        __sp_ht_entry(__K, __V)* data;         \
        __K tmp_key;                           \
        __V tmp_val;                           \
        u64 size;                              \
        u64 capacity;                          \
        sp_ht_info_t info;                     \
    }*

#define sp_ht(__K, __V) struct sp_mcat(refactor_me, sp_mcat(__, sp_mcat(__K, sp_mcat(_, __V)))) {}*

#define sp_ht_new(__K, __V) SP_NULLPTR

#define sp_ht_set_fns(ht, hash_fn, cmp_fn) \
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
      sp_mem_allocator_free((ht)->info.allocator, (ht)->data);    \
      (ht)->data = SP_NULLPTR;\
      sp_mem_allocator_free((ht)->info.allocator, (ht));            \
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

#define sp_ht_init_a(mem, ht)                                                                  \
  do {                                                                                         \
    (ht)                       = sp_ht_alloc_type_a((mem), ht, sizeof(*(ht)));                 \
    (ht)->data                 = sp_ht_alloc_type_a((mem), (ht)->data, 2 * sizeof((ht)->data[0])); \
    (ht)->info.allocator       = (mem);                                                        \
    (ht)->size                 = 0;                                                            \
    (ht)->capacity             = 2;                                                            \
    (ht)->info.size.key        = sizeof((ht)->data[0].key);                                    \
    (ht)->info.size.value      = sizeof((ht)->data[0].val);                                    \
    (ht)->info.stride.entry    = sp_ht_data_u8_n(ht, 1) - sp_ht_data_u8_n(ht, 0);              \
    (ht)->info.stride.value    = sp_ht_data_offset_u8(ht, val);                                \
    (ht)->info.stride.kv       = sp_ht_data_offset_u8(ht, state);                              \
    (ht)->info.header.size     = sp_ht_field_offset_u8(ht, size);                              \
    (ht)->info.header.capacity = sp_ht_field_offset_u8(ht, capacity);                          \
    (ht)->info.fn.hash         = sp_ht_on_hash_key;                                            \
    (ht)->info.fn.compare      = sp_ht_on_compare_key;                                         \
  } while (0)

#define sp_ht_insert_ex(ht, k, v)                                      \
  do {                                                                 \
    (ht)->tmp_key = (k);                                               \
    (ht)->tmp_val = (v);                                               \
    sp_ht_insert_impl(ht, &(ht)->tmp_key, &(ht)->tmp_val, (ht)->info); \
  } while (0)

#define sp_ht_insert(ht, k, v)                                      \
  sp_ht_insert_ex(ht, k, v)

#define sp_ht_get_key_n(ht, n) \
  (&(ht)->data[(n)].key)

#define sp_ht_get_n(ht, n) \
  (&(ht)->data[(n)].val)

#define sp_ht_get_tmp_n(ht) \
  sp_ht_get_n(ht, (ht)->info.tmp_idx)

#define sp_ht_getp(ht, key) \
  (!(ht) ? SP_NULLPTR : ( \
    (ht)->tmp_key = (key), \
    (ht)->info.tmp_idx = sp_ht_tmp_key_index(ht), \
    (ht)->info.tmp_idx == SP_HT_INVALID_INDEX ? \
      SP_NULLPTR : \
      sp_ht_get_tmp_n(ht) \
    ) \
  )

#define sp_ht_get_ex(ht, key, idx) \
  (!(ht) ? SP_NULLPTR : (   \
    (idx) = sp_ht_key_index(ht, key), \
    (idx) == SP_HT_INVALID_INDEX ?  \
      SP_NULLPTR :  \
      sp_ht_get_n(ht, idx) \
    ) \
  )

#define sp_ht_erase(ht, k)                               \
  do {                                                   \
    if ((ht)) {                                          \
      (ht)->tmp_key = (k);                               \
      u64 _ht_idx = sp_ht_tmp_key_index(ht);             \
      if (_ht_idx != SP_HT_INVALID_INDEX) {              \
        (ht)->data[_ht_idx].state = SP_HT_ENTRY_DELETED; \
        if ((ht)->size) (ht)->size--;                    \
      }                                                  \
    }                                                    \
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

#define sp_ht_key_index(ht, key) \
  sp_ht_get_key_index_fn((void**)&(ht)->data, (void*)&(key), (ht)->capacity, (ht)->info)

////////////
// STR_HT //
////////////
#define sp_str_ht(t) sp_ht(sp_str_t, t)

#define sp_str_ht_ensure(ht) \
  if (!(ht)) sp_str_ht_init(ht)

#define sp_str_ht_init_a(__a, ht)                       \
  do {                                                  \
    sp_ht_init_a((__a), ht);                            \
    (ht)->info.fn.hash    = sp_ht_on_hash_str_key;      \
    (ht)->info.fn.compare = sp_ht_on_compare_str_key;   \
  } while (0)

#define sp_str_ht_insert(ht, key, value)  \
  sp_ht_insert_ex(ht, key, value);

#define sp_str_ht_get(ht, key)       sp_ht_getp(ht, key)
#define sp_str_ht_get_ex(ht, key, n) sp_ht_get_ex(ht, key, n)
#define sp_str_ht_erase(ht, key)     sp_ht_erase(ht, key)
#define sp_str_ht_size(ht)           sp_ht_size(ht)
#define sp_str_ht_capacity(ht)       sp_ht_capacity(ht)
#define sp_str_ht_empty(ht)          sp_ht_empty(ht)
#define sp_str_ht_clear(ht)          sp_ht_clear(ht)
#define sp_str_ht_free(ht)           sp_ht_free(ht)
#define sp_str_ht_front(ht)          sp_ht_front(ht)
#define sp_str_ht_for(ht, it)        sp_ht_for(ht, it)
#define sp_str_ht_for_kv(ht, it)     sp_ht_for_kv(ht, it)
#define sp_str_ht_it_init(ht)        sp_ht_it_init(ht)
#define sp_str_ht_it_valid(ht, it)   sp_ht_it_valid(ht, it)
#define sp_str_ht_it_advance(ht, it) sp_ht_it_advance(ht, it)
#define sp_str_ht_it_getp(ht, it)    sp_ht_it_getp(ht, it)
#define sp_str_ht_it_getkp(ht, it)   sp_ht_it_getkp(ht, it)

/////////////
// CSTR_HT //
/////////////
#define sp_cstr_ht(t) sp_ht(const c8*, t)

#define sp_cstr_ht_ensure(ht) \
  if (!(ht)) sp_cstr_ht_init(ht)

#define sp_cstr_ht_init_a(__a, ht)                      \
  do {                                                  \
    sp_ht_init_a((__a), ht);                            \
    (ht)->info.fn.hash    = sp_ht_on_hash_cstr_key;     \
    (ht)->info.fn.compare = sp_ht_on_compare_cstr_key;  \
  } while (0)

#define sp_cstr_ht_insert(ht, key, value)  \
  sp_ht_insert_ex(ht, key, value);

#define sp_cstr_ht_get(ht, key)       sp_ht_getp(ht, key)
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
SP_API sp_hash_t   sp_ht_on_hash_key(void* key, u64 size);
SP_API bool        sp_ht_on_compare_key(void* ka, void* kb, u64 size);
SP_API sp_hash_t   sp_ht_on_hash_str_key(void* key, u64 size);
SP_API bool        sp_ht_on_compare_str_key(void* ka, void* kb, u64 size);
SP_API sp_hash_t   sp_ht_on_hash_cstr_key(void* key, u64 size);
SP_API bool        sp_ht_on_compare_cstr_key(void* ka, void* kb, u64 size);


//   █████████  ███████████ ███████████   █████ ██████   █████   █████████
//  ███░░░░░███░█░░░███░░░█░░███░░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███
// ░███    ░░░ ░   ░███  ░  ░███    ░███  ░███  ░███░███ ░███  ███     ░░░
// ░░█████████     ░███     ░██████████   ░███  ░███░░███░███ ░███
//  ░░░░░░░░███    ░███     ░███░░░░░███  ░███  ░███ ░░██████ ░███    █████
//  ███    ░███    ░███     ░███    ░███  ░███  ░███  ░░█████ ░░███  ░░███
// ░░█████████     █████    █████   █████ █████ █████  ░░█████ ░░█████████
//  ░░░░░░░░░     ░░░░░    ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░
// @string
typedef struct {
  void* user_data;

  sp_io_writer_t* writer;
  struct {
    sp_str_t* data;
    u32 len;
  } elements;

  sp_str_t str;
  u32 index;
} sp_str_reduce_context_t;

typedef struct {
  sp_mem_t mem;
  sp_str_t str;
  void* user_data;
} sp_str_map_context_t;


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

#define sp_str(STR, LEN) (sp_rval(sp_str_t) { .data = (const c8*)(STR), .len = (u32)(LEN) })
#define sp_str_lit(STR)  (sp_rval(sp_str_t) { .data = (const c8*)(STR), .len = (u32)(sizeof(STR) - 1) })
#define sp_lit(STR)      SP_LIT(STR)

#define SP_CSTR(STR)     sp_str((STR), sp_cstr_len(STR))
#define sp_cstr(STR)     SP_CSTR(STR)
#define SP_STR(STR, LEN) sp_str(STR, LEN)
#define SP_LIT(STR)      sp_str_lit(STR)

#define sp_str_for(str, it) for (u32 it = 0; it < str.len; it++)
#define sp_str_for_it(str, it) for (sp_str_it_t it = sp_str_it(str); sp_str_it_valid(&it); sp_str_it_next(&it))
#define sp_str_for_utf8(str, it) for (sp_utf8_it_t it = sp_utf8_it(str); sp_utf8_it_valid(&it); sp_utf8_it_next(&it))
#define sp_str_rfor_utf8(str, it) for (sp_utf8_it_t it = sp_utf8_rit(str); sp_utf8_it_valid(&it); sp_utf8_it_prev(&it))

#define SP_STR_NO_MATCH -1

SP_API bool            sp_str_empty(sp_str_t);
SP_API bool            sp_str_equal(sp_str_t a, sp_str_t b);
SP_API bool            sp_str_equal_cstr(sp_str_t a, const c8* b);
SP_API bool            sp_str_starts_with(sp_str_t str, sp_str_t prefix);
SP_API bool            sp_str_ends_with(sp_str_t str, sp_str_t suffix);
SP_API bool            sp_str_contains(sp_str_t str, sp_str_t needle);
SP_API s32             sp_str_find(sp_str_t str, sp_str_t needle);
SP_API s32             sp_str_find_c8(sp_str_t str, c8 needle);
SP_API s32             sp_str_find_c8_reverse(sp_str_t str, c8 needle);
SP_API bool            sp_str_valid(sp_str_t str);
SP_API c8              sp_str_at(sp_str_t str, s32 index);
SP_API c8              sp_str_at_reverse(sp_str_t str, s32 index);
SP_API c8              sp_str_back(sp_str_t str);
SP_API s32             sp_str_compare_alphabetical(sp_str_t a, sp_str_t b);
SP_API sp_str_t        sp_str_prefix(sp_str_t str, s32 len);
SP_API sp_str_t        sp_str_suffix(sp_str_t str, s32 len);
SP_API sp_str_t        sp_str_sub(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t        sp_str_sub_reverse(sp_str_t str, s32 index, s32 len);
SP_API sp_str_t        sp_str_replace_c8(sp_str_t str, c8 from, c8 to);
SP_API sp_str_t        sp_str_pad(sp_str_t str, u32 n);
SP_API sp_str_t        sp_str_trim_left(sp_str_t str);
SP_API sp_str_t        sp_str_trim_right(sp_str_t str);
SP_API sp_str_t        sp_str_trim(sp_str_t str);
SP_API sp_str_t        sp_str_strip_left(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_strip_right(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_strip(sp_str_t str, sp_str_t strip);
SP_API sp_str_t        sp_str_truncate_a(sp_mem_t mem, sp_str_t str, u32 n, sp_str_t trailer);
SP_API sp_str_t        sp_str_join_cstr_n_a(sp_mem_t mem, const c8** strings, u32 num_strings, sp_str_t join);
SP_API sp_str_t        sp_str_to_upper(sp_str_t str);
SP_API sp_str_t        sp_str_to_lower(sp_str_t str);
SP_API sp_str_t        sp_str_to_pascal_case(sp_str_t str);
SP_API sp_str_pair_t   sp_str_cleave_c8(sp_str_t str, c8 delimiter);
SP_API sp_da(sp_str_t) sp_str_split_c8(sp_str_t, c8 c);
SP_API sp_da(sp_str_t) sp_str_pad_to_longest_a(sp_mem_t mem, sp_str_t* strs, u32 n);
SP_API c8*             sp_str_to_cstr(sp_str_t str);
SP_API sp_str_t        sp_str_copy(sp_str_t str);
SP_API void            sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity);
SP_API sp_str_t        sp_str_from_cstr(const c8* str);
SP_API sp_str_t        sp_str_from_cstr_n(const c8* str, u32 len);
SP_API sp_str_t        sp_str_alloc(u32 capacity);
SP_API sp_str_t        sp_str_view(const c8* cstr);
SP_API c8*             sp_cstr_copy(const c8* cstr);
SP_API c8*             sp_cstr_copy_n(const c8* str, u32 len);
SP_API void            sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_len);
SP_API void            sp_cstr_copy_to_n(const c8* str, u32 len, c8* buffer, u32 buffer_len);
SP_API bool            sp_cstr_equal(const c8* a, const c8* b);
SP_API u32             sp_cstr_len(const c8* str);
SP_API u32             sp_cstr_len_n(const c8* str, u32 n);
SP_API sp_str_t        sp_cstr_as_str(const c8* str);
SP_API sp_str_t        sp_cstr_as_str_n(const c8* str, u32 n);
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
SP_API bool            sp_wtf8_validate(sp_str_t str);
SP_API sp_wide_str_t   sp_wtf8_to_wtf16(sp_str_t wtf8);
SP_API sp_str_t        sp_wtf16_to_wtf8(sp_wide_str_t wtf16);
SP_API c8              sp_c8_to_upper(c8 c);
SP_API c8              sp_c8_to_lower(c8 c);
SP_API sp_da(sp_str_t) sp_str_map_a(sp_mem_t mem, sp_str_t* s, u32 n, void* ud, sp_str_map_fn_t fn);
SP_API sp_str_t        sp_str_map_kernel_prepend(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_append(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_prefix(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_trim(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_pad(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_to_upper(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_to_lower(sp_str_map_context_t* context);
SP_API sp_str_t        sp_str_map_kernel_pascal_case(sp_str_map_context_t* context);
SP_API s32             sp_str_sort_kernel_alphabetical(const void* a, const void* b);



//  █████          ███████      █████████
// ░░███         ███░░░░░███   ███░░░░░███
//  ░███        ███     ░░███ ███     ░░░
//  ░███       ░███      ░███░███
//  ░███       ░███      ░███░███    █████
//  ░███      █░░███     ███ ░░███  ░░███
//  ███████████ ░░░███████░   ░░█████████
// ░░░░░░░░░░░    ░░░░░░░      ░░░░░░░░░
// @log


//  ██████████ ██████   █████ █████   █████
// ░░███░░░░░█░░██████ ░░███ ░░███   ░░███
//  ░███  █ ░  ░███░███ ░███  ░███    ░███
//  ░██████    ░███░░███░███  ░███    ░███
//  ░███░░█    ░███ ░░██████  ░░███   ███
//  ░███ ░   █ ░███  ░░█████   ░░░█████░
//  ██████████ █████  ░░█████    ░░███
// ░░░░░░░░░░ ░░░░░    ░░░░░      ░░░
// @env
typedef struct {
  sp_str_t key;
  sp_str_t value;
} sp_env_var_t;

typedef sp_ht_a(sp_str_t, sp_str_t) sp_env_table_t;

typedef struct {
  sp_env_table_t vars;
} sp_env_t;

SP_API void     sp_env_init(sp_env_t* env, sp_mem_t mem);
SP_API sp_env_t sp_env_capture(sp_mem_t mem);
SP_API sp_env_t sp_env_copy(sp_env_t* env, sp_mem_t mem);
SP_API u32      sp_env_count(sp_env_t* env);
SP_API sp_str_t sp_env_get(sp_env_t* env, sp_str_t name);
SP_API bool     sp_env_contains(sp_env_t* env, sp_str_t name);
SP_API void     sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value);
SP_API void     sp_env_erase(sp_env_t* env, sp_str_t name);
SP_API void     sp_env_destroy(sp_env_t* env);

typedef struct {
  sp_str_t key;
  sp_str_t value;
  void* os;
} sp_os_env_it_t;


//  ███████████ █████ ██████   ██████ ██████████
// ░█░░░███░░░█░░███ ░░██████ ██████ ░░███░░░░░█
// ░   ░███  ░  ░███  ░███░█████░███  ░███  █ ░
//     ░███     ░███  ░███░░███ ░███  ░██████
//     ░███     ░███  ░███ ░░░  ░███  ░███░░█
//     ░███     ░███  ░███      ░███  ░███ ░   █
//     █████    █████ █████     █████ ██████████
//    ░░░░░    ░░░░░ ░░░░░     ░░░░░ ░░░░░░░░░░
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


SP_API sp_tm_epoch_t     sp_tm_now_epoch();
SP_API sp_tm_date_time_t sp_tm_epoch_to_date_time(sp_tm_epoch_t time);
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


// ███████████ █████ █████       ██████████  █████████  █████ █████  █████████  ███████████ ██████████ ██████   ██████
// ░░███░░░░░░█░░███ ░░███       ░░███░░░░░█ ███░░░░░███░░███ ░░███  ███░░░░░███░█░░░███░░░█░░███░░░░░█░░██████ ██████
//  ░███   █ ░  ░███  ░███        ░███  █ ░ ░███    ░░░  ░░███ ███  ░███    ░░░ ░   ░███  ░  ░███  █ ░  ░███░█████░███
//  ░███████    ░███  ░███        ░██████   ░░█████████   ░░█████   ░░█████████     ░███     ░██████    ░███░░███ ░███
//  ░███░░░█    ░███  ░███        ░███░░█    ░░░░░░░░███   ░░███     ░░░░░░░░███    ░███     ░███░░█    ░███ ░░░  ░███
//  ░███  ░     ░███  ░███      █ ░███ ░   █ ███    ░███    ░███     ███    ░███    ░███     ░███ ░   █ ░███      ░███
//  █████       █████ ███████████ ██████████░░█████████     █████   ░░█████████     █████    ██████████ █████     █████
// ░░░░░       ░░░░░ ░░░░░░░░░░░ ░░░░░░░░░░  ░░░░░░░░░     ░░░░░     ░░░░░░░░░     ░░░░░    ░░░░░░░░░░ ░░░░░     ░░░░░
// @fs @filesystem
#define SP_PATH_MAX 4096
#define SP_FS_IT_BUF_SIZE 2048

typedef enum {
  SP_FS_LINK_HARD,
  SP_FS_LINK_SYMBOLIC,
  SP_FS_LINK_COPY,
} sp_fs_link_kind_t;

typedef enum {
  SP_OS_NO_FOLLOW_SYMLINK,
  SP_OS_FOLLOW_SYMLINK,
} sp_os_follow_symlink_t;

typedef struct {
  sp_str_t path;
  sp_str_t name;
  sp_fs_kind_t kind;
} sp_fs_entry_t;

typedef struct {
#if defined(SP_WIN32)
  sp_win32_handle_t handle;
  sp_win32_find_data_t find_data;
  bool first;
#elif defined(SP_MACOS) || defined(SP_COSMO)
  DIR* dir;
#else
  s32 fd;
  sp_aligned() u8 buf [SP_FS_IT_BUF_SIZE];
  s32 buf_pos;
  s32 buf_end;
#endif
  sp_str_t path;
} sp_fs_it_frame_t;

typedef struct {
  sp_fs_entry_t entry;
  sp_da(sp_fs_it_frame_t) stack;
  bool recursive;
} sp_fs_it_t;

// SP_API sp_fs_it_t    sp_fs_it_new_recursive(sp_str_t path);
// SP_API sp_fs_it_t    sp_fs_it_new(sp_str_t path);
// SP_API void          sp_fs_it_begin(sp_fs_it_t* it, sp_str_t path);
// SP_API void          sp_fs_it_next(sp_fs_it_t* it);
// SP_API bool          sp_fs_it_valid(sp_fs_it_t* it);
// SP_API void          sp_fs_it_deinit(sp_fs_it_t* it);
// SP_API sp_da(sp_fs_entry_t) sp_fs_collect(sp_str_t path);
// SP_API sp_da(sp_fs_entry_t) sp_fs_collect_recursive(sp_str_t path);

bool sp_sys_diriter_open(sp_fs_it_frame_t* frame, sp_str_t path);
void sp_sys_diriter_close(sp_fs_it_frame_t* frame);
bool sp_sys_diriter_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry);


//  ███████████ █████   █████ ███████████   ██████████   █████████   ██████████   █████ ██████   █████   █████████
// ░█░░░███░░░█░░███   ░░███ ░░███░░░░░███ ░░███░░░░░█  ███░░░░░███ ░░███░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███
// ░   ░███  ░  ░███    ░███  ░███    ░███  ░███  █ ░  ░███    ░███  ░███   ░░███ ░███  ░███░███ ░███  ███     ░░░
//     ░███     ░███████████  ░██████████   ░██████    ░███████████  ░███    ░███ ░███  ░███░░███░███ ░███
//     ░███     ░███░░░░░███  ░███░░░░░███  ░███░░█    ░███░░░░░███  ░███    ░███ ░███  ░███ ░░██████ ░███    █████
//     ░███     ░███    ░███  ░███    ░███  ░███ ░   █ ░███    ░███  ░███    ███  ░███  ░███  ░░█████ ░░███  ░░███
//     █████    █████   █████ █████   █████ ██████████ █████   █████ ██████████   █████ █████  ░░█████ ░░█████████
//    ░░░░░    ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░░░░░░ ░░░░░   ░░░░░ ░░░░░░░░░░   ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░
// @threading @concurrency
// @atomic
typedef s32 sp_atomic_s32_t;
typedef void* sp_atomic_ptr_t;

SP_API bool  sp_atomic_s32_cas(sp_atomic_s32_t* value, s32 current, s32 desired);
SP_API s32   sp_atomic_s32_set(sp_atomic_s32_t* value, s32 desired);
SP_API s32   sp_atomic_s32_add(sp_atomic_s32_t* value, s32 add);
SP_API s32   sp_atomic_s32_get(sp_atomic_s32_t* value);
SP_API bool  sp_atomic_ptr_cas(sp_atomic_ptr_t* value, void* current, void* desired);
SP_API void* sp_atomic_ptr_set(sp_atomic_ptr_t* value, void* desired);
SP_API void* sp_atomic_ptr_get(sp_atomic_ptr_t* value);

// @mutex
typedef enum {
  SP_MUTEX_NONE = 0,
  SP_MUTEX_PLAIN = 1,
  SP_MUTEX_TIMED = 2,
  SP_MUTEX_RECURSIVE = 4
} sp_mutex_kind_t;

#if defined(SP_WIN32)
  typedef CRITICAL_SECTION sp_mutex_t;
#elif defined(SP_FREESTANDING)
  typedef s32 sp_mutex_t;
#elif defined(SP_POSIX)
  typedef pthread_mutex_t sp_mutex_t;
#else
  typedef s32 sp_mutex_t;
#endif

SP_API void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind);
SP_API void sp_mutex_lock(sp_mutex_t* mutex);
SP_API void sp_mutex_unlock(sp_mutex_t* mutex);
SP_API void sp_mutex_destroy(sp_mutex_t* mutex);
SP_API s32  sp_mutex_kind_to_c11(sp_mutex_kind_t kind);

// @cv @condition_variable @condvar
#if defined(SP_WIN32)
  typedef CONDITION_VARIABLE sp_cv_t;
#elif defined(SP_FREESTANDING)
  typedef s32 sp_cv_t;
#elif defined(SP_POSIX)
  typedef pthread_cond_t sp_cv_t;
#else
  typedef s32 sp_cv_t;
#endif

SP_API void sp_cv_init(sp_cv_t* cv);
SP_API void sp_cv_destroy(sp_cv_t* cv);
SP_API void sp_cv_wait(sp_cv_t* cv, sp_mutex_t* mutex);
SP_API bool sp_cv_wait_for(sp_cv_t* cv, sp_mutex_t* mutex, u32 ms);
SP_API void sp_cv_notify_one(sp_cv_t* cv);
SP_API void sp_cv_notify_all(sp_cv_t* cv);

// @semaphore
#if defined(SP_WIN32)
  typedef HANDLE sp_semaphore_t;
#elif defined(SP_FREESTANDING)
  typedef s32 sp_semaphore_t;
#elif defined(SP_MACOS)
  typedef dispatch_semaphore_t sp_semaphore_t;
#elif defined(SP_POSIX)
  typedef sem_t sp_semaphore_t;
#else
  typedef s32 sp_semaphore_t;
#endif

SP_API void sp_semaphore_init(sp_semaphore_t* semaphore);
SP_API void sp_semaphore_destroy(sp_semaphore_t* semaphore);
SP_API void sp_semaphore_wait(sp_semaphore_t* semaphore);
SP_API bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms);
SP_API void sp_semaphore_signal(sp_semaphore_t* semaphore);

// @spin @spin_lock
typedef s32 sp_spin_lock_t;

SP_API void sp_spin_pause();
SP_API bool sp_spin_try_lock(sp_spin_lock_t* lock);
SP_API void sp_spin_lock(sp_spin_lock_t* lock);
SP_API void sp_spin_unlock(sp_spin_lock_t* lock);

// @thread
#if defined(SP_WIN32)
  typedef sp_win32_handle_t sp_thread_t;
#elif defined(SP_FREESTANDING)
  typedef s32 sp_thread_t;
#elif defined(SP_POSIX)
  typedef pthread_t sp_thread_t;
#else
  typedef s32 sp_thread_t;
#endif

SP_TYPEDEF_FN(s32, sp_thread_fn_t, void*);

typedef struct {
  sp_thread_fn_t fn;
  void* userdata;
  sp_semaphore_t semaphore;
} sp_thread_launch_t;

SP_API void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata);
SP_API void sp_thread_join(sp_thread_t* thread);
SP_API s32  sp_thread_launch(void* userdata);


//     ███████     █████████
//   ███░░░░░███  ███░░░░░███
//  ███     ░░███░███    ░░░
// ░███      ░███░░█████████
// ░███      ░███ ░░░░░░░░███
// ░░███     ███  ███    ░███
//  ░░░███████░  ░░█████████
//    ░░░░░░░     ░░░░░░░░░
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
  SP_OS_SIGNAL_INTERRUPT,
  SP_OS_SIGNAL_TERMINATE,
  SP_OS_SIGNAL_ABORT,
} sp_os_signal_t;

sp_typedef_fn(void, sp_os_signal_handler_t, sp_os_signal_t signal, void* userdata);

SP_API void           sp_sleep_ns(u64 ns);
SP_API void           sp_sleep_ms(f64 ms);
SP_API sp_os_kind_t   sp_os_get_kind();
SP_API sp_str_t       sp_os_get_name();
SP_API sp_str_t       sp_os_get_executable_ext();
SP_API sp_str_t       sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind);
SP_API sp_str_t       sp_os_lib_to_file_name(sp_str_t lib, sp_os_lib_kind_t kind);
SP_API void           sp_os_sleep_ms(f64 ms);
SP_API void           sp_os_sleep_ns(u64 ns);
SP_API void           sp_os_print(sp_str_t message);
SP_API void           sp_os_print_err(sp_str_t message);
SP_API sp_str_t       sp_os_env_get(sp_str_t key);
SP_API sp_os_env_it_t sp_os_env_it_begin();
SP_API bool           sp_os_env_it_valid(sp_os_env_it_t* it);
SP_API void           sp_os_env_it_next(sp_os_env_it_t* it);
SP_API sp_err_t       sp_os_create_file(sp_str_t path);
SP_API sp_err_t       sp_os_create_dir(sp_str_t path);
SP_API sp_err_t       sp_os_create_hard_link(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t       sp_os_create_sym_link(sp_str_t target, sp_str_t link_path);
SP_API sp_str_t       sp_os_canonicalize_path(sp_str_t path);
SP_API sp_str_t       sp_os_get_cwd();
SP_API void           sp_os_register_signal_handler(sp_os_signal_t, sp_os_signal_handler_t, void* userdata);
SP_API bool           sp_os_is_tty(sp_sys_fd_t fd);
SP_API void           sp_os_tty_size(sp_sys_fd_t fd, u32* cols, u32* rows);
SP_API s32            sp_os_tty_enter_raw(sp_sys_fd_t fd, sp_tty_mode_t* saved);
SP_API s32            sp_os_tty_restore(sp_sys_fd_t fd, const sp_tty_mode_t* saved);
SP_API void           sp_os_qsort(void* arr, u64 len, u64 stride, sp_qsort_fn_t);


//  ███████████    ███████    ███████████   ██████   ██████   █████████   ███████████
// ░░███░░░░░░█  ███░░░░░███ ░░███░░░░░███ ░░██████ ██████   ███░░░░░███ ░█░░░███░░░█
//  ░███   █ ░  ███     ░░███ ░███    ░███  ░███░█████░███  ░███    ░███ ░   ░███  ░
//  ░███████   ░███      ░███ ░██████████   ░███░░███ ░███  ░███████████     ░███
//  ░███░░░█   ░███      ░███ ░███░░░░░███  ░███ ░░░  ░███  ░███░░░░░███     ░███
//  ░███  ░    ░░███     ███  ░███    ░███  ░███      ░███  ░███    ░███     ░███
//  █████       ░░░███████░   █████   █████ █████     █████ █████   █████    █████
// ░░░░░          ░░░░░░░    ░░░░░   ░░░░░ ░░░░░     ░░░░░ ░░░░░   ░░░░░    ░░░░░
// @format
sp_err_t sp_fmt_io(sp_io_writer_t* io, const c8* fmt, ...);

typedef enum {
  SP_FMT_ALIGN_NONE,
  SP_FMT_ALIGN_LEFT,
  SP_FMT_ALIGN_CENTER,
  SP_FMT_ALIGN_RIGHT,
} sp_fmt_align_t;

#define SP_FMT_MAX_DIRECTIVES 8
#define SP_FMT_WIDTH_MAX      4096

typedef enum {
  sp_fmt_id_u64    = 1 << 0,
  sp_fmt_id_s64    = 1 << 1,
  sp_fmt_id_f64    = 1 << 2,
  sp_fmt_id_str    = 1 << 3,
  sp_fmt_id_ptr    = 1 << 4,
  sp_fmt_id_custom = 1 << 5,
} sp_fmt_arg_kind_t;

typedef struct {
  u32 width;
  sp_fmt_align_t align;
  u8 fill;
  sp_opt(u8) precision;
  u8 fill_dynamic;
  u8 width_dynamic;
  u8 precision_dynamic;
  u8 directive_count;
  u8 directive_arg_dynamic;
  sp_str_t directive_names[SP_FMT_MAX_DIRECTIVES];
  sp_str_t directive_args [SP_FMT_MAX_DIRECTIVES];
} sp_fmt_spec_t;

typedef struct sp_fmt_arg sp_fmt_arg_t;
SP_TYPEDEF_FN(void, sp_fmt_fn_t, sp_io_writer_t*, sp_fmt_arg_t*, sp_fmt_arg_t*);
SP_TYPEDEF_FN(void, sp_fmt_transform_fn_t, sp_io_writer_t*, sp_str_t, sp_fmt_arg_t*, sp_fmt_arg_t*);

struct sp_fmt_arg {
  sp_fmt_arg_kind_t id;
  sp_fmt_spec_t spec;
  union {
    u64 u;
    s64 i;
    f64 f;
    sp_str_t s;
    void* p;
    struct { sp_fmt_fn_t fn; void* ptr; } custom;
  };
};

typedef enum {
  sp_fmt_directive_renderer,
  sp_fmt_directive_transformer,
  sp_fmt_directive_decorator,
} sp_fmt_directive_kind_t;

typedef struct {
  sp_fmt_directive_kind_t kind;
  sp_fmt_arg_kind_t       arg_kinds;
  sp_fmt_arg_kind_t       param_kinds;
  union {
    sp_fmt_fn_t           renderer;
    sp_fmt_transform_fn_t transformer;
    struct {
      sp_fmt_fn_t before;
      sp_fmt_fn_t after;
    } decorator;
  };
} sp_fmt_directive_t;

#define sp_fmt_uint(_value)  (sp_fmt_arg_t) { .id = sp_fmt_id_u64, .u = (_value) }
#define sp_fmt_int(_value)   (sp_fmt_arg_t) { .id = sp_fmt_id_s64, .i = (_value) }
#define sp_fmt_float(_value) (sp_fmt_arg_t) { .id = sp_fmt_id_f64, .f = (_value) }
#define sp_fmt_char(_value)  (sp_fmt_arg_t) { .id = sp_fmt_id_str, .s = { .data = &(_value), .len = 1 } }
#define sp_fmt_str(_value)   (sp_fmt_arg_t) { .id = sp_fmt_id_str, .s = (_value) }
#define sp_fmt_cstr(_value)  (sp_fmt_arg_t) { .id = sp_fmt_id_str, .s = sp_str_view(_value) }
#define sp_fmt_ptr(_value)   (sp_fmt_arg_t) { .id = sp_fmt_id_ptr, .p = (_value) }

// Use a tiny, portable trick to get some damn good type safety for custom format string arguments:
//
//   sizeof((T){0} = $value)
//
// In other words, zero initialize a T and assign it to the value that was passed to us. But instead
// of evaluating this in a way that emits instructions at runtime, we simply take the size of the
// result.
//
// This has the effect of checking whether the value passed can be assigned to a variable of type T.
// And since we're in C, that's functionally equivalent for structs to "is the value of type T". This
// works cleanly on every compiler I've tried against, produces good error messages, and does not
// require multi-level macro expansion.
//
// The downside is that it forces the call site to explicitly specify T; it'd be great if the caller
// could simply have the format function (which they want to pass anyway) take a T* instead of an
// sp_fmt_arg_t* and rely on that for the type checking machinery. But, alas, it's UB to store
// a function pointer as a differently-typed function pointer, and this is the only other piece
// of type information we have besides the type of T (what we are trying to validate)
//
// The upside is that this is how sp_fmt() works anyway! When you do:
//
//    sp_fmt("{}", sp_fmt_int(69));
//
// You're specifying the type at the call site anyway. It's not much different to do:
//
//    sp_fmt("{}", sp_fmt_foo(my_foo));
//
#define sp_fmt_custom(T, _fn, _value) (   \
  (void)sizeof((T)sp_zero() = (_value)),  \
  (sp_fmt_arg_t) {                     \
    .id = sp_fmt_id_custom,            \
    .custom = {                           \
      .fn = (_fn),        \
      .ptr = (void*)&(_value)             \
    }                                     \
  })
#define sp_fmt_custom_v(T, _fn, ...) (   \
  (void)sizeof((T)sp_zero() = (__VA_ARGS__)),  \
  (sp_fmt_arg_t) {                     \
    .id = sp_fmt_id_custom,            \
    .custom = {                           \
      .fn = (_fn),        \
      .ptr = (void*)&(__VA_ARGS__)        \
    }                                     \
  })

#define sp_fmt_register_renderer(_name, _fn, _arg_kinds) \
  sp_fmt_directive_register(_name, (sp_fmt_directive_t) { \
    .kind = sp_fmt_directive_renderer, \
    .arg_kinds = (_arg_kinds), \
    .renderer = (_fn), \
  })

#define sp_fmt_register_transformer(_name, _fn) \
  sp_fmt_directive_register(_name, (sp_fmt_directive_t) { \
    .kind = sp_fmt_directive_transformer, \
    .transformer = (_fn), \
  })

#define sp_fmt_register_decorator(_name, _before, _after) \
  sp_fmt_directive_register(_name, (sp_fmt_directive_t) { \
    .kind = sp_fmt_directive_decorator, \
    .decorator = { .before = (_before), .after = (_after) }, \
  })

#define sp_fmt_register_decorator_p(_name, _before, _after, _param_kinds) \
  sp_fmt_directive_register(_name, (sp_fmt_directive_t) { \
    .kind = sp_fmt_directive_decorator, \
    .param_kinds = (_param_kinds), \
    .decorator = { .before = (_before), .after = (_after) }, \
  })

void sp_fmt_directive_register(const c8* name, sp_fmt_directive_t directive);

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
SP_API u16       sp_parse_c16(sp_str_t str);
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
SP_API bool      sp_parse_c16_ex(sp_str_t str, u16* out);
SP_API bool      sp_parse_ptr_ex(sp_str_t str, void** out);
SP_API bool      sp_parse_bool_ex(sp_str_t str, bool* out);
SP_API bool      sp_parse_hash_ex(sp_str_t str, sp_hash_t* out);
SP_API bool      sp_parse_hex_ex(sp_str_t str, u64* out);
SP_API bool      sp_parse_is_digit(c8 c);


//    █████████     ███████    ██████   █████ ███████████ ██████████ █████ █████ ███████████
//   ███░░░░░███  ███░░░░░███ ░░██████ ░░███ ░█░░░███░░░█░░███░░░░░█░░███ ░░███ ░█░░░███░░░█
//  ███     ░░░  ███     ░░███ ░███░███ ░███ ░   ░███  ░  ░███  █ ░  ░░███ ███  ░   ░███  ░
// ░███         ░███      ░███ ░███░░███░███     ░███     ░██████     ░░█████       ░███
// ░███         ░███      ░███ ░███ ░░██████     ░███     ░███░░█      ███░███      ░███
// ░░███     ███░░███     ███  ░███  ░░█████     ░███     ░███ ░   █  ███ ░░███     ░███
//  ░░█████████  ░░░███████░   █████  ░░█████    █████    ██████████ █████ █████    █████
//   ░░░░░░░░░     ░░░░░░░    ░░░░░    ░░░░░    ░░░░░    ░░░░░░░░░░ ░░░░░ ░░░░░    ░░░░░
// @context
#ifndef SP_RT_MAX_CONTEXT
  #define SP_RT_MAX_CONTEXT 16
#endif

#ifndef SP_RT_NUM_SPIN_LOCKS
  #define SP_RT_NUM_SPIN_LOCKS 32
#endif

#if defined(SP_WIN32)
  typedef sp_win32_dword_t sp_tls_key_t;
  typedef INIT_ONCE sp_tls_once_t;
#elif defined(SP_FREESTANDING) || defined(SP_WASM)
  typedef u8 sp_tls_key_t;
  typedef u8 sp_tls_once_t;
#elif defined(SP_POSIX)
  typedef pthread_key_t  sp_tls_key_t;
  typedef pthread_once_t sp_tls_once_t;
#else
  typedef u8 sp_tls_key_t;
  typedef u8 sp_tls_once_t;
#endif

typedef struct sp_tls_block {
  struct sp_tls_block* self;
  void* data;
} sp_tls_block_t;

extern sp_tls_block_t sp_tls_block;

sp_typedef_fn(void, sp_tls_once_fn_t);
sp_typedef_fn(void, sp_tls_deinit_fn_t, void*);

SP_PRIVATE void  sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t fn);
SP_PRIVATE void* sp_tls_get(sp_tls_key_t key);
SP_PRIVATE void  sp_tls_set(sp_tls_key_t key, void* data);
SP_PRIVATE void  sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t);

typedef struct {
  sp_mem_t allocator;
} sp_context_t;

typedef struct {
  sp_context_t contexts [SP_RT_MAX_CONTEXT];
  u32 index;
  sp_mem_arena_t* scratch [2];
  sp_env_t env;
  struct {
    sp_ht_a(sp_str_t, sp_fmt_directive_t) directives;
  } format;
  struct {
    sp_io_writer_t* out;
    sp_io_writer_t* err;
  } std;
} sp_tls_rt_t;

#if defined(SP_WIN32)
typedef s32 sp_nt_status_t;

#define SP_NT_SUCCESS(s) ((sp_nt_status_t)(s) >= 0)
#define SP_NT_STATUS_SUCCESS             ((sp_nt_status_t)0x00000000)
#define SP_NT_STATUS_NAME_TOO_LONG       ((sp_nt_status_t)0xC0000106)
#define SP_NT_STATUS_OBJECT_NAME_INVALID ((sp_nt_status_t)0xC0000033)

typedef struct {
  u16 Length;
  u16 MaximumLength;
  u16* Buffer;
} sp_nt_unicode_string_t;

typedef struct {
  sp_nt_unicode_string_t name;
  u16* heap_buffer;
} sp_sys_nt_path_t;

typedef struct {
  u32 Length;
  void* RootDirectory;
  sp_nt_unicode_string_t* ObjectName;
  u32 Attributes;
  void* SecurityDescriptor;
  void* SecurityQualityOfService;
} sp_nt_object_attributes_t;

#define SP_NT_OBJ_INHERIT           0x00000002
#define SP_NT_OBJ_CASE_INSENSITIVE  0x00000040

typedef struct {
  union {
    sp_nt_status_t Status;
    void* Pointer;
  };
  uintptr_t Information;
} sp_nt_io_status_block_t;

#define SP_NT_FUNCS(X)                                                                                      \
  X(sp_nt_status_t, RtlDosPathNameToNtPathName_U_WithStatus, (const u16*, sp_nt_unicode_string_t*, u16**, void*)) \
  X(s32,            RtlFreeHeap,                             (void*, u32, void*)) \
  X(sp_nt_status_t, RtlSetCurrentDirectory_U,                (sp_nt_unicode_string_t*))  \
  X(sp_nt_status_t, NtClose,                                 (void*))                         \
  X(sp_nt_status_t, NtSetInformationFile,                    (void*, sp_nt_io_status_block_t*, void*, u32, u32)) \
  X(sp_nt_status_t, NtCreateFile,                            (void**, u32, sp_nt_object_attributes_t*, sp_nt_io_status_block_t*, s64*, u32, u32, u32, u32, void*, u32))


#define SP_NT_DECL(ret, name, args) ret (__stdcall *name) args;
typedef struct { SP_NT_FUNCS(SP_NT_DECL) } sp_nt_dispatch_t;
#undef SP_NT_DECL

SP_API sp_nt_status_t sp_sys_nt_path(sp_str_t utf8, sp_sys_nt_path_t* out);
SP_API void           sp_sys_nt_path_free(sp_sys_nt_path_t* path);
#endif

typedef struct {
  sp_os_signal_handler_t signal_handlers[3];
  void* signal_userdata[3];
  sp_mutex_t mutex;
  sp_spin_lock_t locks [SP_RT_NUM_SPIN_LOCKS];
  struct {
    sp_tls_key_t key;
    sp_tls_once_t once;
  } tls;
#if defined(SP_WIN32)
  sp_nt_dispatch_t nt;
#endif
} sp_rt_t;

sp_tls_rt_t*   sp_tls_rt_get();
sp_context_t*  sp_tls_rt_get_context(sp_tls_rt_t* tls);
sp_mem_t sp_tls_rt_get_allocator(sp_tls_rt_t* tls);
void           sp_sys_tls_init(sp_tls_rt_t* tls);
sp_context_t*  sp_context_get();
void           sp_context_set(sp_context_t context);
void           sp_context_push(sp_context_t context);
sp_mem_t sp_context_get_allocator();
void           sp_context_push_allocator(sp_mem_t allocator);
void           sp_context_push_arena(sp_mem_arena_t* arena);
void           sp_context_pop();

SP_API sp_rt_t sp_rt;


//  █████    ███████
// ░░███   ███░░░░░███
//  ░███  ███     ░░███
//  ░███ ░███      ░███
//  ░███ ░███      ░███
//  ░███ ░░███     ███
//  █████ ░░░███████░
// ░░░░░    ░░░░░░░
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
  SP_IO_CLOSE_MODE_NONE,
  SP_IO_CLOSE_MODE_AUTO,
} sp_io_close_mode_t;

typedef struct {
  sp_sys_fd_t fd;
  sp_io_close_mode_t close_mode;
} sp_io_file_data_t;

typedef enum {
  SP_IO_WRITE_MODE_OVERWRITE,
  SP_IO_WRITE_MODE_APPEND,
} sp_io_write_mode_t;

SP_TYPEDEF_FN(sp_err_t, sp_io_reader_read_cb, sp_io_reader_t* r, void* ptr, u64 size, u64* bytes_read);
SP_TYPEDEF_FN(sp_err_t, sp_io_reader_seek_cb, sp_io_reader_t* r, s64 offset, sp_io_whence_t whence, s64* position);
SP_TYPEDEF_FN(sp_err_t, sp_io_reader_size_cb, sp_io_reader_t* r, u64* size);
SP_TYPEDEF_FN(sp_err_t, sp_io_reader_close_cb, sp_io_reader_t* r);

SP_TYPEDEF_FN(sp_err_t, sp_io_writer_write_cb, sp_io_writer_t* w, const void* ptr, u64 size, u64* bytes_written);
SP_TYPEDEF_FN(sp_err_t, sp_io_writer_seek_cb, sp_io_writer_t* w, s64 offset, sp_io_whence_t whence, s64* position);
SP_TYPEDEF_FN(sp_err_t, sp_io_writer_size_cb, sp_io_writer_t* w, u64* size);
SP_TYPEDEF_FN(sp_err_t, sp_io_writer_close_cb, sp_io_writer_t* w);

typedef struct {
  const u8* ptr;
  u64 len;
  u64 pos;
} sp_io_reader_mem_t;

struct sp_io_reader {
  struct {
    sp_io_reader_read_cb  read;
    sp_io_reader_seek_cb  seek;
    sp_io_reader_size_cb  size;
    sp_io_reader_close_cb close;
  } vtable;

  union {
    sp_io_file_data_t file;
    sp_io_reader_mem_t mem;
  };

  sp_mem_buffer_t buffer;
  u64 seek;
};

typedef struct {
  u8* ptr;
  u64 len;
  u64 pos;
} sp_io_writer_mem_t;

typedef struct {
  sp_mem_t allocator;
  sp_mem_buffer_t buffer;
  u64 seek;
} sp_io_writer_dyn_mem_t;

struct sp_io_writer {
  struct {
    sp_io_writer_write_cb write;
    sp_io_writer_seek_cb  seek;
    sp_io_writer_size_cb  size;
    sp_io_writer_close_cb close;
  } vtable;

  union {
    sp_io_file_data_t file;
    sp_io_writer_mem_t mem;
    sp_io_writer_dyn_mem_t dyn_mem;
  };

  sp_mem_buffer_t buffer;
};

SP_API sp_err_t       sp_io_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read);
SP_API sp_err_t       sp_io_reader_seek(sp_io_reader_t* reader, s64 offset, sp_io_whence_t whence, s64* position);
SP_API sp_err_t       sp_io_reader_size(sp_io_reader_t* r, u64* size);
SP_API sp_err_t       sp_io_reader_close(sp_io_reader_t* r);
SP_API sp_err_t       sp_io_reader_from_file(sp_io_reader_t* reader, sp_str_t path);
SP_API void           sp_io_reader_from_fd(sp_io_reader_t* reader, sp_sys_fd_t fd, sp_io_close_mode_t mode);
SP_API void           sp_io_reader_from_mem(sp_io_reader_t* reader, const void* ptr, u64 size);
SP_API void           sp_io_reader_set_buffer(sp_io_reader_t* reader, u8* buf, u64 capacity);
SP_API sp_err_t       sp_io_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written);
SP_API sp_err_t       sp_io_write_str(sp_io_writer_t* writer, sp_str_t str, u64* bytes_written);
SP_API sp_err_t       sp_io_write_cstr(sp_io_writer_t* writer, const c8* cstr, u64* bytes_written);
SP_API sp_err_t       sp_io_write_c8(sp_io_writer_t* writer, c8 c);
SP_API sp_err_t       sp_io_pad(sp_io_writer_t* writer, u64 size, u64* bytes_written);
SP_API sp_err_t       sp_io_flush(sp_io_writer_t* w);
SP_API sp_err_t       sp_io_writer_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position);
SP_API sp_err_t       sp_io_writer_size(sp_io_writer_t* w, u64* size);
SP_API sp_err_t       sp_io_writer_close(sp_io_writer_t* w);
SP_API sp_err_t       sp_io_writer_from_file(sp_io_writer_t* writer, sp_str_t path, sp_io_write_mode_t mode);
SP_API void           sp_io_writer_from_fd(sp_io_writer_t* writer, sp_sys_fd_t fd, sp_io_close_mode_t close_mode);
SP_API void           sp_io_writer_from_mem(sp_io_writer_t* writer, void* ptr, u64 size);
SP_API void           sp_io_get_std_out(sp_io_writer_t* io);
SP_API void           sp_io_get_std_err(sp_io_writer_t* io);
SP_API sp_err_t       sp_io_writer_set_buffer(sp_io_writer_t* writer, u8* buf, u64 capacity);


//  ███████████  ███████████      ███████      █████████  ██████████  █████████   █████████
// ░░███░░░░░███░░███░░░░░███   ███░░░░░███   ███░░░░░███░░███░░░░░█ ███░░░░░███ ███░░░░░███
//  ░███    ░███ ░███    ░███  ███     ░░███ ███     ░░░  ░███  █ ░ ░███    ░░░ ░███    ░░░
//  ░██████████  ░██████████  ░███      ░███░███          ░██████   ░░█████████ ░░█████████
//  ░███░░░░░░   ░███░░░░░███ ░███      ░███░███          ░███░░█    ░░░░░░░░███ ░░░░░░░░███
//  ░███         ░███    ░███ ░░███     ███ ░░███     ███ ░███ ░   █ ███    ░███ ███    ░███
//  █████        █████   █████ ░░░███████░   ░░█████████  ██████████░░█████████ ░░█████████
// ░░░░░        ░░░░░   ░░░░░    ░░░░░░░      ░░░░░░░░░  ░░░░░░░░░░  ░░░░░░░░░   ░░░░░░░░░
// @ps @process
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
  .in  = { .mode = SP_PS_IO_MODE_NULL }, \
  .out = { .mode = SP_PS_IO_MODE_NULL }, \
  .err = { .mode = SP_PS_IO_MODE_NULL }, \
}

typedef struct {
  sp_sys_fd_t fd;
  sp_ps_io_mode_t mode;
  sp_ps_io_blocking_t block;
} sp_ps_io_in_config_t;

typedef struct {
  sp_sys_fd_t fd;
  sp_ps_io_mode_t mode;
  sp_ps_io_blocking_t block;
} sp_ps_io_out_config_t;

typedef struct {
  sp_ps_io_in_config_t in;
  sp_ps_io_out_config_t out;
  sp_ps_io_out_config_t err;
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

typedef struct sp_ps_os sp_ps_os_t;

typedef struct {
  sp_ps_io_t io;
  sp_ps_os_t* os;
  sp_mem_t allocator;
} sp_ps_t;

SP_API sp_ps_config_t  sp_ps_config_copy(const sp_ps_config_t* src);
SP_API void            sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg);
SP_API sp_ps_t         sp_ps_create(sp_ps_config_t config);
SP_API sp_ps_output_t  sp_ps_run(sp_ps_config_t config);
SP_API sp_io_writer_t* sp_ps_io_in(sp_ps_t* proc);
SP_API sp_io_reader_t* sp_ps_io_out(sp_ps_t* proc);
SP_API sp_io_reader_t* sp_ps_io_err(sp_ps_t* proc);
SP_API sp_ps_status_t  sp_ps_wait(sp_ps_t* proc);
SP_API sp_ps_status_t  sp_ps_poll(sp_ps_t* proc, u32 timeout_ms);
SP_API sp_ps_output_t  sp_ps_output(sp_ps_t* proc);
SP_API bool            sp_ps_kill(sp_ps_t* proc);


//    █████████   ███████████  ███████████
//   ███░░░░░███ ░░███░░░░░███░░███░░░░░███
//  ░███    ░███  ░███    ░███ ░███    ░███
//  ░███████████  ░██████████  ░██████████
//  ░███░░░░░███  ░███░░░░░░   ░███░░░░░░
//  ░███    ░███  ░███         ░███
//  █████   █████ █████        █████
// ░░░░░   ░░░░░ ░░░░░        ░░░░░
// @app
typedef enum {
  SP_APP_CONTINUE = 0,
  SP_APP_ERR = 1,
  SP_APP_QUIT = 2
} sp_app_result_t;

typedef enum {
  SP_APP_MODE_LOCKED = 0, // harness sleeps to fps; poll and update fire together each frame
  SP_APP_MODE_FREE,       // harness never sleeps; poll runs every tick, update gated by fps
} sp_app_mode_t;

typedef struct sp_app sp_app_t;

sp_typedef_fn(sp_app_result_t, sp_app_init_fn_t,   sp_app_t*);
sp_typedef_fn(sp_app_result_t, sp_app_poll_fn_t,   sp_app_t*);
sp_typedef_fn(sp_app_result_t, sp_app_update_fn_t, sp_app_t*);
sp_typedef_fn(void,            sp_app_deinit_fn_t, sp_app_t*);

typedef struct {
  void* user_data;
  sp_app_init_fn_t on_init;
  sp_app_poll_fn_t on_poll;
  sp_app_update_fn_t on_update;
  sp_app_deinit_fn_t on_deinit;
  u32 fps;
  sp_app_mode_t mode;
} sp_app_config_t;

struct sp_app {
  void* user_data;
  sp_app_init_fn_t on_init;
  sp_app_poll_fn_t on_poll;
  sp_app_update_fn_t on_update;
  sp_app_deinit_fn_t on_deinit;

  s32 rc;
  sp_app_result_t result;
  sp_atomic_s32_t shutdown;

  bool initted;
  bool deinitted;

  u32 fps;
  sp_app_mode_t mode;

  struct {
    sp_tm_timer_t timer;
    u64 accumulated;
    u64 num;
  } frame;
};


//  ███████████ █████ █████       ██████████    ██████   ██████    ███████    ██████   █████ █████ ███████████    ███████    ███████████
// ░░███░░░░░░█░░███ ░░███       ░░███░░░░░█   ░░██████ ██████   ███░░░░░███ ░░██████ ░░███ ░░███ ░█░░░███░░░█  ███░░░░░███ ░░███░░░░░███
//  ░███   █ ░  ░███  ░███        ░███  █ ░     ░███░█████░███  ███     ░░███ ░███░███ ░███  ░███ ░   ░███  ░  ███     ░░███ ░███    ░███
//  ░███████    ░███  ░███        ░██████       ░███░░███ ░███ ░███      ░███ ░███░░███░███  ░███     ░███    ░███      ░███ ░██████████
//  ░███░░░█    ░███  ░███        ░███░░█       ░███ ░░░  ░███ ░███      ░███ ░███ ░░██████  ░███     ░███    ░███      ░███ ░███░░░░░███
//  ░███  ░     ░███  ░███      █ ░███ ░   █    ░███      ░███ ░░███     ███  ░███  ░░█████  ░███     ░███    ░░███     ███  ░███    ░███
//  █████       █████ ███████████ ██████████    █████     █████ ░░░███████░   █████  ░░█████ █████    █████    ░░░███████░   █████   █████
// ░░░░░       ░░░░░ ░░░░░░░░░░░ ░░░░░░░░░░    ░░░░░     ░░░░░    ░░░░░░░    ░░░░░    ░░░░░ ░░░░░    ░░░░░       ░░░░░░░    ░░░░░   ░░░░░
// @fmon @file_monitor
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
typedef struct sp_fmon_os sp_fmon_os_t;

SP_TYPEDEF_FN(void, sp_fmon_fn_t, sp_fmon_t*, sp_fmon_event_t*, void*);

typedef struct {
	sp_hash_t hash;
	f64 last_event_time;
} sp_fmon_cache_t;


#define SP_FILE_MONITOR_BUFFER_SIZE 4092

struct sp_fmon {
	sp_fmon_fn_t callback;
	void* userdata;
	sp_da(sp_fmon_event_t) changes;
	sp_da(sp_fmon_cache_t) cache;
	sp_fmon_os_t* os;
	sp_fmon_event_kind_t events_to_watch;
	u32 debounce_time_ms;
  sp_mem_t allocator;
};


SP_END_EXTERN_C()

//    █████████
//   ███░░░░░███     ███         ███
//  ███     ░░░     ░███        ░███
// ░███          ███████████ ███████████
// ░███         ░░░░░███░░░ ░░░░░███░░░
// ░░███     ███    ░███        ░███
//  ░░█████████     ░░░         ░░░
//   ░░░░░░░░░
// @cpp @c++
#ifdef SP_CPP
SP_API sp_str_t operator/(const sp_str_t& a, const sp_str_t& b);
SP_API sp_str_t operator/(const sp_str_t& a, const c8* b);
#endif // SP_CPP

#endif // SP_SP_H



#ifndef SP_IMPL_H
#if defined(SP_PRIVATE_HEADER) || defined(SP_IMPLEMENTATION)
#define SP_IMPL_H
typedef struct {
  sp_str_t str;
  u32 i;
} sp_fmt_parser_t;

static sp_fmt_directive_t* sp_fmt_directive_lookup(sp_str_t name);
static void sp_fmt_directive_reset();
static void sp_fmt_register_builtins();
#endif // SP_PRIVATE_HEADER or SP_IMPLEMENTATION






// @header @top

void* sp_ht_alloc(u64 size);

// @private
SP_IMP c8*      sp_fmt_uint_to_buf_dec(u64 value, c8* buf_end);
SP_IMP c8*      sp_fmt_uint_to_buf_hex_ex(u64 value, c8* buf_end, const c8* digits);
SP_IMP c8*      sp_fmt_uint_to_buf_hex(u64 value, c8* buf_end);
SP_IMP void     sp_fmt_write_u64_a(sp_io_writer_t* io, u64 value);
SP_IMP void     sp_fmt_write_s64_a(sp_io_writer_t* io, s64 value);
SP_IMP void     sp_fmt_write_f64_a(sp_io_writer_t* io, f64 value, u32 precision);
SP_IMP void     sp_fmt_write_ptr_a(sp_io_writer_t* io, void* value);
SP_IMP sp_err_t sp_fmt_render_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params);
SP_IMP void     sp_fmt_apply_spec_a(sp_io_writer_t* io, sp_str_t pre, sp_str_t str, sp_str_t post, sp_fmt_spec_t spec);
SP_API void     sp_fmt_render_default_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param);

SP_IMP sp_mem_arena_t* sp_tls_rt_get_scratch_arena_a(sp_tls_rt_t* tls);
SP_IMP sp_mem_arena_t* sp_tls_rt_get_scratch_arena_for_a(sp_tls_rt_t* tls, sp_mem_t mem);

SP_IMP sp_err_t sp_io_writer_dyn_write(sp_io_writer_t* io, const void* ptr, u64 size, u64* bytes_written);
SP_IMP sp_err_t sp_io_writer_dyn_seek(sp_io_writer_t* io, s64 offset, sp_io_whence_t whence, s64* position);
SP_IMP sp_err_t sp_io_writer_dyn_size(sp_io_writer_t* io, u64* size);
SP_IMP sp_err_t sp_io_writer_dyn_close(sp_io_writer_t* io);
SP_IMP sp_err_t sp_io_writer_mem_write(sp_io_writer_t* io, const void* ptr, u64 size, u64* bytes_written);
SP_IMP sp_err_t sp_io_writer_mem_seek(sp_io_writer_t* io, s64 offset, sp_io_whence_t whence, s64* position);
SP_IMP sp_err_t sp_io_writer_mem_size(sp_io_writer_t* io, u64* size);
SP_IMP sp_err_t sp_io_writer_mem_close(sp_io_writer_t* io);

SP_IMP sp_fs_kind_t sp_fs_lstat_kind_a(sp_str_t path);
SP_IMP sp_fs_kind_t sp_fs_stat_kind_a(sp_str_t path);
SP_IMP sp_fs_it_t   sp_fs_it_new_a(sp_mem_t mem, sp_str_t path);
SP_IMP sp_fs_it_t   sp_fs_it_new_recursive_a(sp_mem_t mem, sp_str_t path);
SP_IMP void         sp_fs_it_begin_a(sp_fs_it_t* it, sp_str_t path);
SP_IMP void         sp_fs_it_next_a(sp_fs_it_t* it);
SP_IMP void         sp_fs_it_push_a(sp_fs_it_t* it, sp_str_t path);
SP_IMP bool         sp_fs_it_valid_a(sp_fs_it_t* it);
SP_IMP void         sp_fs_it_deinit_a(sp_fs_it_t* it);

// @public
SP_API sp_app_t*        sp_app_new(sp_mem_t mem, sp_app_config_t config);
SP_API sp_app_result_t  sp_app_tick(sp_app_t* app);
SP_API void             sp_app_destroy(sp_app_t* app);
SP_API s32              sp_app_run_locked(sp_app_t* app);
SP_API s32              sp_app_run_free(sp_app_t* app);
SP_API s32              sp_app_run(sp_app_config_t config);

SP_API sp_str_t      sp_tm_epoch_to_iso8601_a(sp_mem_t mem, sp_tm_epoch_t time);

SP_API sp_str_t      sp_str_copy_a(sp_mem_t mem, sp_str_t str);
SP_API sp_str_t      sp_str_concat_a(sp_mem_t mem, sp_str_t a, sp_str_t b);
SP_API sp_str_t      sp_str_join_a(sp_mem_t mem, sp_str_t a, sp_str_t b, sp_str_t join);
SP_API sp_str_t      sp_str_join_n_a(sp_mem_t mem, sp_str_t* strs, u32 n, sp_str_t joiner);
SP_API sp_str_t      sp_str_reduce_a(sp_mem_t mem, sp_str_t* strs, u32 n, void* ud, sp_str_reduce_fn_t fn);
SP_API void          sp_str_reduce_kernel_join_a(sp_str_reduce_context_t* context);
SP_API c8*           sp_cstr_from_str_a(sp_mem_t mem, sp_str_t str);

SP_API void          sp_io_writer_from_mem(sp_io_writer_t* writer, void* ptr, u64 size);
SP_API sp_err_t      sp_io_read_file_a(sp_mem_t mem, sp_str_t path, sp_str_t* content);
SP_API sp_err_t      sp_io_writer_from_file_a(sp_io_writer_t* writer, sp_str_t path, sp_io_write_mode_t mode);
SP_API sp_err_t      sp_io_reader_from_file_a(sp_io_reader_t* reader, sp_str_t path);
SP_API sp_str_t sp_io_writer_mem_as_str(sp_io_writer_mem_t* io);
SP_API sp_str_t sp_io_writer_dyn_mem_as_str(sp_io_writer_dyn_mem_t* io);
SP_API void sp_io_writer_from_dyn_mem_a(sp_mem_t mem, sp_io_writer_t* writer);


SP_API void sp_fmon_init_a(sp_mem_t mem, sp_fmon_t* m, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* user_data);
SP_API void sp_fmon_deinit(sp_fmon_t* monitor);
SP_API void sp_fmon_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_API void sp_fmon_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_API void sp_fmon_process_changes(sp_fmon_t* monitor);
SP_API void sp_fmon_emit_changes(sp_fmon_t* monitor);

SP_API sp_str_t       sp_fs_get_name(sp_str_t path);
SP_API sp_str_t       sp_fs_parent_path(sp_str_t path);
SP_API sp_str_t       sp_fs_trim_path(sp_str_t path);
SP_API sp_str_t       sp_fs_get_ext(sp_str_t path);
SP_API sp_str_t       sp_fs_get_stem(sp_str_t path);
SP_API bool           sp_fs_is_root(sp_str_t path);
SP_API bool           sp_fs_is_glob(sp_str_t path);
SP_API sp_str_t      sp_fs_normalize_path_a(sp_mem_t mem, sp_str_t path);
SP_API sp_str_t      sp_fs_join_path_a(sp_mem_t mem, sp_str_t a, sp_str_t b);
SP_API sp_str_t      sp_fs_replace_ext_a(sp_mem_t mem, sp_str_t path, sp_str_t ext);
SP_API sp_str_t      sp_fs_canonicalize_path_a(sp_mem_t mem, sp_str_t path);
SP_API sp_str_t      sp_fs_get_exe_path_a(sp_mem_t mem);
SP_API sp_str_t      sp_fs_get_cwd_a(sp_mem_t mem);
SP_API sp_str_t      sp_fs_get_storage_path_a(sp_mem_t mem);
SP_API sp_str_t      sp_fs_get_config_path_a(sp_mem_t mem);
SP_API sp_da(sp_fs_entry_t) sp_fs_collect_a(sp_mem_t mem, sp_str_t path);
SP_API sp_da(sp_fs_entry_t) sp_fs_collect_recursive_a(sp_mem_t mem, sp_str_t path);
SP_API bool          sp_fs_exists_a(sp_str_t path);
SP_API bool          sp_fs_is_file_a(sp_str_t path);
SP_API bool          sp_fs_is_dir_a(sp_str_t path);
SP_API bool          sp_fs_is_symlink_a(sp_str_t path);
SP_API bool          sp_fs_is_target_file_a(sp_str_t path);
SP_API bool          sp_fs_is_target_dir_a(sp_str_t path);
SP_API sp_fs_kind_t  sp_fs_get_kind_a(sp_str_t path);
SP_API sp_fs_kind_t  sp_fs_get_target_kind_a(sp_str_t path);
SP_API sp_tm_epoch_t sp_fs_get_mod_time_a(sp_str_t path);
SP_API sp_err_t      sp_fs_create_dir_a(sp_str_t path);
SP_API sp_err_t      sp_fs_create_file_a(sp_str_t path);
SP_API sp_err_t      sp_fs_create_file_str_a(sp_str_t path, sp_str_t str);
SP_API sp_err_t      sp_fs_create_file_slice_a(sp_str_t path, sp_mem_slice_t slice);
SP_API sp_err_t      sp_fs_create_file_cstr_a(sp_str_t path, const c8* str);
SP_API sp_err_t      sp_fs_remove_dir_a(sp_str_t path);
SP_API sp_err_t      sp_fs_remove_file_a(sp_str_t path);
SP_API sp_err_t      sp_fs_create_hard_link_a(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t      sp_fs_create_sym_link_a(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t      sp_fs_link_a(sp_str_t from, sp_str_t to, sp_fs_link_kind_t kind);
SP_API sp_err_t      sp_fs_copy_a(sp_str_t from, sp_str_t to);
SP_API void          sp_fs_copy_file_a(sp_str_t from, sp_str_t to);
SP_API void          sp_fs_copy_dir_a(sp_str_t from, sp_str_t to);
SP_API void          sp_fs_copy_glob_a(sp_str_t from, sp_str_t glob, sp_str_t to);

#define sp_fs_for(mem, dir, it) \
  for (sp_fs_it_t it = sp_fs_it_new_a(mem, dir); sp_fs_it_valid_a(&it); sp_fs_it_next_a(&it))
#define sp_fs_for_recursive(mem, dir, it) \
  for (sp_fs_it_t it = sp_fs_it_new_recursive_a(mem, dir); sp_fs_it_valid(&it); sp_fs_it_next(&it))


SP_API sp_mem_t              sp_mem_os_new();
SP_API sp_mem_arena_t*       sp_mem_get_scratch_arena_a(sp_mem_t mem);
SP_API sp_mem_arena_marker_t sp_mem_begin_scratch_a();
SP_API sp_mem_arena_marker_t sp_mem_begin_scratch_for_a(sp_mem_t mem);
SP_API void                  sp_mem_end_scratch_a(sp_mem_arena_marker_t s);
SP_API sp_mem_t              sp_mem_scratch_allocator_a();
SP_API sp_mem_t              sp_mem_arena_as_allocator(sp_mem_arena_t* arena);
SP_API void*                 sp_alloc_a(sp_mem_t mem, u64 size);
SP_API void*                 sp_realloc_a(sp_mem_t mem, void* memory, u64 size);
SP_API void                  sp_free_a(sp_mem_t mem, void* memory);

SP_API void sp_log_a(const c8* fmt, ...);
SP_API void sp_log_a_a(sp_mem_t mem, const c8* fmt, ...);
SP_API void sp_log_str_a(sp_str_t fmt, ...);
SP_API void sp_log_err_a(const c8* fmt, ...);
SP_API void sp_print_a(const c8* fmt, ...);
SP_API void sp_print_str_a(sp_str_t fmt, ...);
SP_API void sp_print_err_a(const c8* fmt, ...);

SP_API sp_str_r sp_fmt_a(sp_mem_t mem, const c8* fmt, ...);
SP_API sp_err_t sp_fmt_v_a(sp_io_writer_t* io, sp_str_t fmt, va_list args);



#define sp_alloc_n_a(a, T, n) (T*)sp_alloc_a(a, (n) * sizeof(T))
#define sp_alloc_type_a(a, T) sp_alloc_n_a(a, T, 1)
sp_mem_arena_t* sp_tls_rt_get_scratch_arena(sp_tls_rt_t* tls);
// `@header @bottom`

#endif



//  █████ ██████   ██████ ███████████  █████
// ░░███ ░░██████ ██████ ░░███░░░░░███░░███
//  ░███  ░███░█████░███  ░███    ░███ ░███
//  ░███  ░███░░███ ░███  ░██████████  ░███
//  ░███  ░███ ░░░  ░███  ░███░░░░░░   ░███
//  ░███  ░███      ░███  ░███         ░███      █
//  █████ █████     █████ █████        ███████████
// ░░░░░ ░░░░░     ░░░░░ ░░░░░        ░░░░░░░░░░░
// @implementation
#ifndef SP_SP_C
#ifdef SP_IMPLEMENTATION
#define SP_SP_C

SP_BEGIN_EXTERN_C()

sp_rt_t sp_rt;
sp_tls_block_t sp_tls_block;
#if defined(SP_FREESTANDING) || defined(SP_WASM_FREESTANDING)
c8** environ;
s32 errno;
#endif

//   █████████  █████ █████  █████████
//  ███░░░░░███░░███ ░░███  ███░░░░░███
// ░███    ░░░  ░░███ ███  ░███    ░░░
// ░░█████████   ░░█████   ░░█████████
//  ░░░░░░░░███   ░░███     ░░░░░░░░███
//  ███    ░███    ░███     ███    ░███
// ░░█████████     █████   ░░█████████
//  ░░░░░░░░░     ░░░░░     ░░░░░░░░░
// @sys
#define SP_WNOHANG    1
#define SP_WUNTRACED  2

#define SP_WIFEXITED(s)    (((s) & 0x7f) == 0)
#define SP_WEXITSTATUS(s)  (((s) >> 8) & 0xff)
#define SP_WIFSIGNALED(s)  (((s) & 0x7f) != 0 && ((s) & 0x7f) != 0x7f)
#define SP_WTERMSIG(s)     ((s) & 0x7f)
#define SP_WIFSTOPPED(s)   (((s) & 0xff) == 0x7f)
#define SP_WSTOPSIG(s)     SP_WEXITSTATUS(s)

#define SP_S_IFMT   0170000
#define SP_S_IFSOCK 0140000
#define SP_S_IFLNK  0120000
#define SP_S_IFREG  0100000
#define SP_S_IFBLK  0060000
#define SP_S_IFDIR  0040000
#define SP_S_IFCHR  0020000
#define SP_S_IFIFO  0010000

#define SP_S_ISDIR(m)  (((m) & SP_S_IFMT) == SP_S_IFDIR)
#define SP_S_ISREG(m)  (((m) & SP_S_IFMT) == SP_S_IFREG)
#define SP_S_ISLNK(m)  (((m) & SP_S_IFMT) == SP_S_IFLNK)
#define SP_S_ISCHR(m)  (((m) & SP_S_IFMT) == SP_S_IFCHR)
#define SP_S_ISFIFO(m) (((m) & SP_S_IFMT) == SP_S_IFIFO)
#define SP_S_ISBLK(m)  (((m) & SP_S_IFMT) == SP_S_IFBLK)
#define SP_S_ISSOCK(m) (((m) & SP_S_IFMT) == SP_S_IFSOCK)

#if defined(SP_WASM_WASI)
  #include <wasi/api.h>
#elif defined(SP_WASM_FREESTANDING)
  // No wasi-libc headers: vendor the same types and import declarations.
  // Layout/field names mirror wasi-libc's <wasi/api.h> exactly so call sites
  // are identical across both axes.

  typedef __SIZE_TYPE__ __wasi_size_t;
  typedef uint16_t      __wasi_errno_t;
  typedef int           __wasi_fd_t;
  typedef int64_t       __wasi_filedelta_t;
  typedef uint64_t      __wasi_filesize_t;
  typedef uint64_t      __wasi_timestamp_t;
  typedef uint32_t      __wasi_clockid_t;
  typedef uint64_t      __wasi_userdata_t;
  typedef uint8_t       __wasi_eventtype_t;
  typedef uint16_t      __wasi_eventrwflags_t;
  typedef uint16_t      __wasi_subclockflags_t;
  typedef uint8_t       __wasi_whence_t;
  typedef uint32_t      __wasi_exitcode_t;

  typedef struct __wasi_iovec_t {
    uint8_t*      buf;
    __wasi_size_t buf_len;
  } __wasi_iovec_t;

  typedef struct __wasi_ciovec_t {
    const uint8_t* buf;
    __wasi_size_t  buf_len;
  } __wasi_ciovec_t;

  typedef struct __wasi_event_fd_readwrite_t {
    __wasi_filesize_t     nbytes;
    __wasi_eventrwflags_t flags;
  } __wasi_event_fd_readwrite_t;

  typedef struct __wasi_event_t {
    __wasi_userdata_t           userdata;
    __wasi_errno_t              error;
    __wasi_eventtype_t          type;
    __wasi_event_fd_readwrite_t fd_readwrite;
  } __wasi_event_t;

  typedef struct __wasi_subscription_clock_t {
    __wasi_clockid_t       id;
    __wasi_timestamp_t     timeout;
    __wasi_timestamp_t     precision;
    __wasi_subclockflags_t flags;
  } __wasi_subscription_clock_t;

  typedef struct __wasi_subscription_fd_readwrite_t {
    __wasi_fd_t file_descriptor;
  } __wasi_subscription_fd_readwrite_t;

  typedef union __wasi_subscription_u_u_t {
    __wasi_subscription_clock_t        clock;
    __wasi_subscription_fd_readwrite_t fd_read;
    __wasi_subscription_fd_readwrite_t fd_write;
  } __wasi_subscription_u_u_t;

  typedef struct __wasi_subscription_u_t {
    uint8_t                   tag;
    __wasi_subscription_u_u_t u;
  } __wasi_subscription_u_t;

  typedef struct __wasi_subscription_t {
    __wasi_userdata_t       userdata;
    __wasi_subscription_u_t u;
  } __wasi_subscription_t;

  #define __WASI_EVENTTYPE_CLOCK    ((__wasi_eventtype_t)0)
  #define __WASI_EVENTTYPE_FD_READ  ((__wasi_eventtype_t)1)
  #define __WASI_EVENTTYPE_FD_WRITE ((__wasi_eventtype_t)2)
  #define __WASI_WHENCE_SET         ((__wasi_whence_t)0)
  #define __WASI_WHENCE_CUR         ((__wasi_whence_t)1)
  #define __WASI_WHENCE_END         ((__wasi_whence_t)2)

  #define sp_wasi_import(name) \
    __attribute__((import_module("wasi_snapshot_preview1"), import_name(name)))

  sp_wasi_import("fd_read")
  extern __wasi_errno_t __wasi_fd_read(__wasi_fd_t fd, const __wasi_iovec_t* iovs, __wasi_size_t niov, __wasi_size_t* nread);

  sp_wasi_import("fd_write")
  extern __wasi_errno_t __wasi_fd_write(__wasi_fd_t fd, const __wasi_ciovec_t* iovs, __wasi_size_t niov, __wasi_size_t* nwritten);

  sp_wasi_import("fd_close")
  extern __wasi_errno_t __wasi_fd_close(__wasi_fd_t fd);

  sp_wasi_import("fd_seek")
  extern __wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t* newoffset);

  sp_wasi_import("clock_time_get")
  extern __wasi_errno_t __wasi_clock_time_get(__wasi_clockid_t clock_id, __wasi_timestamp_t precision, __wasi_timestamp_t* time_out);

  sp_wasi_import("poll_oneoff")
  extern __wasi_errno_t __wasi_poll_oneoff(const __wasi_subscription_t* in, __wasi_event_t* out, __wasi_size_t nsub, __wasi_size_t* nevents);

  sp_wasi_import("args_sizes_get")
  extern __wasi_errno_t __wasi_args_sizes_get(__wasi_size_t* argc, __wasi_size_t* argv_buf_size);

  sp_wasi_import("args_get")
  extern __wasi_errno_t __wasi_args_get(uint8_t** argv, uint8_t* argv_buf);

  sp_wasi_import("environ_sizes_get")
  extern __wasi_errno_t __wasi_environ_sizes_get(__wasi_size_t* count, __wasi_size_t* buf_size);

  sp_wasi_import("environ_get")
  extern __wasi_errno_t __wasi_environ_get(uint8_t** environ, uint8_t* environ_buf);

  sp_wasi_import("proc_exit")
  extern _Noreturn void __wasi_proc_exit(__wasi_exitcode_t code);
#endif


#if defined(SP_LINUX)

// We vendor any constant we use on Linux to make freestanding compilation as simple as
// possible. We also vendor types, for the same reason. Since our Linux backend never calls
// libc, there is no possibility of a mismatch between a vendored struct and a libc struct.
//
// In cases where a piece of code only uses libc functions that are trivial wrappers over
// syscalls (e.g. ioctl), we'll do something like this:
//
// #if defined(SP_LINUX)
//   typedef sp_sys_termios_t sp_termios_t;
// #else
//   typedef struct termios sp_termios_t;
// #endif
//
// Which lets us (a) reuse the code that needs struct termios, identically (b) without
// needing to include termios.h on Linux, but also (c) not risking a struct mismatch
// when we're using libc.
//
// This kind of trick, and any similar sp_sys_* struct, gets defined in the implementation
// section to avoid polluting every TU that doesn't define the implementation. If
// something's in the header section but not part of the public API, it's probably a
// mistake

/////////////////////
// SYSCALL NUMBERS //
/////////////////////
#if defined(SP_AMD64)
  #define SP_SYSCALL_NUM_READ              0
  #define SP_SYSCALL_NUM_WRITE             1
  #define SP_SYSCALL_NUM_OPEN              2
  #define SP_SYSCALL_NUM_CLOSE             3
  #define SP_SYSCALL_NUM_STAT              4
  #define SP_SYSCALL_NUM_FSTAT             5
  #define SP_SYSCALL_NUM_LSTAT             6
  #define SP_SYSCALL_NUM_POLL              7
  #define SP_SYSCALL_NUM_LSEEK             8
  #define SP_SYSCALL_NUM_MMAP              9
  #define SP_SYSCALL_NUM_MUNMAP            11
  #define SP_SYSCALL_NUM_SIGACTION         13
  #define SP_SYSCALL_NUM_IOCTL             16
  #define SP_SYSCALL_NUM_MREMAP            25
  #define SP_SYSCALL_NUM_DUP2              33
  #define SP_SYSCALL_NUM_NANOSLEEP         35
  #define SP_SYSCALL_NUM_GETPID            39
  #define SP_SYSCALL_NUM_CLONE             56
  #define SP_SYSCALL_NUM_FORK              57
  #define SP_SYSCALL_NUM_EXECVE            59
  #define SP_SYSCALL_NUM_EXIT              60
  #define SP_SYSCALL_NUM_WAIT4             61
  #define SP_SYSCALL_NUM_KILL              62
  #define SP_SYSCALL_NUM_FCNTL             72
  #define SP_SYSCALL_NUM_GETCWD            79
  #define SP_SYSCALL_NUM_CHDIR             80
  #define SP_SYSCALL_NUM_RENAME            82
  #define SP_SYSCALL_NUM_MKDIR             83
  #define SP_SYSCALL_NUM_RMDIR             84
  #define SP_SYSCALL_NUM_LINK              86
  #define SP_SYSCALL_NUM_UNLINK            87
  #define SP_SYSCALL_NUM_SYMLINK           88
  #define SP_SYSCALL_NUM_READLINK          89
  #define SP_SYSCALL_NUM_CHMOD             90
  #define SP_SYSCALL_NUM_FCHMOD            91
  #define SP_SYSCALL_NUM_ARCH_PRCTL        158
  #define SP_SYSCALL_NUM_GETTID            186
  #define SP_SYSCALL_NUM_GETDENTS64        217
  #define SP_SYSCALL_NUM_CLOCK_GETTIME     228
  #define SP_SYSCALL_NUM_CLOCK_NANOSLEEP   230
  #define SP_SYSCALL_NUM_EXIT_GROUP        231
  #define SP_SYSCALL_NUM_INOTIFY_ADD_WATCH 254
  #define SP_SYSCALL_NUM_INOTIFY_RM_WATCH  255
  #define SP_SYSCALL_NUM_OPENAT            257
  #define SP_SYSCALL_NUM_MKDIRAT           258
  #define SP_SYSCALL_NUM_NEWFSTATAT        262
  #define SP_SYSCALL_NUM_UNLINKAT          263
  #define SP_SYSCALL_NUM_RENAMEAT          264
  #define SP_SYSCALL_NUM_READLINKAT        267
  #define SP_SYSCALL_NUM_FCHMODAT          268
  #define SP_SYSCALL_NUM_PPOLL             271
  #define SP_SYSCALL_NUM_DUP3              292
  #define SP_SYSCALL_NUM_PIPE2             293
  #define SP_SYSCALL_NUM_INOTIFY_INIT1     294

#elif defined(SP_ARM64)
  #define SP_SYSCALL_NUM_GETCWD            17
  #define SP_SYSCALL_NUM_DUP3              24
  #define SP_SYSCALL_NUM_FCNTL             25
  #define SP_SYSCALL_NUM_INOTIFY_INIT1     26
  #define SP_SYSCALL_NUM_INOTIFY_ADD_WATCH 27
  #define SP_SYSCALL_NUM_INOTIFY_RM_WATCH  28
  #define SP_SYSCALL_NUM_IOCTL             29
  #define SP_SYSCALL_NUM_MKDIRAT           34
  #define SP_SYSCALL_NUM_UNLINKAT          35
  #define SP_SYSCALL_NUM_SYMLINKAT         36
  #define SP_SYSCALL_NUM_LINKAT            37
  #define SP_SYSCALL_NUM_RENAMEAT          38
  #define SP_SYSCALL_NUM_CHDIR             49
  #define SP_SYSCALL_NUM_FCHMOD            52
  #define SP_SYSCALL_NUM_FCHMODAT          53
  #define SP_SYSCALL_NUM_OPENAT            56
  #define SP_SYSCALL_NUM_CLOSE             57
  #define SP_SYSCALL_NUM_PIPE2             59
  #define SP_SYSCALL_NUM_GETDENTS64        61
  #define SP_SYSCALL_NUM_LSEEK             62
  #define SP_SYSCALL_NUM_READ              63
  #define SP_SYSCALL_NUM_WRITE             64
  #define SP_SYSCALL_NUM_PPOLL             73
  #define SP_SYSCALL_NUM_READLINKAT        78
  #define SP_SYSCALL_NUM_NEWFSTATAT        79
  #define SP_SYSCALL_NUM_FSTAT             80
  #define SP_SYSCALL_NUM_EXIT              93
  #define SP_SYSCALL_NUM_EXIT_GROUP        94
  #define SP_SYSCALL_NUM_NANOSLEEP         101
  #define SP_SYSCALL_NUM_CLOCK_GETTIME     113
  #define SP_SYSCALL_NUM_CLOCK_NANOSLEEP   115
  #define SP_SYSCALL_NUM_KILL              129
  #define SP_SYSCALL_NUM_SIGACTION         134
  #define SP_SYSCALL_NUM_GETPID            172
  #define SP_SYSCALL_NUM_GETTID            178
  #define SP_SYSCALL_NUM_MUNMAP            215
  #define SP_SYSCALL_NUM_MREMAP            216
  #define SP_SYSCALL_NUM_CLONE             220
  #define SP_SYSCALL_NUM_EXECVE            221
  #define SP_SYSCALL_NUM_MMAP              222
  #define SP_SYSCALL_NUM_WAIT4             260
  #define SP_SYSCALL_NUM_OPEN              SP_SYSCALL_NUM_OPENAT
  #define SP_SYSCALL_NUM_STAT              SP_SYSCALL_NUM_NEWFSTATAT
  #define SP_SYSCALL_NUM_LSTAT             SP_SYSCALL_NUM_NEWFSTATAT
  #define SP_SYSCALL_NUM_MKDIR             SP_SYSCALL_NUM_MKDIRAT
  #define SP_SYSCALL_NUM_RMDIR             SP_SYSCALL_NUM_UNLINKAT
  #define SP_SYSCALL_NUM_UNLINK            SP_SYSCALL_NUM_UNLINKAT
  #define SP_SYSCALL_NUM_RENAME            SP_SYSCALL_NUM_RENAMEAT
  #define SP_SYSCALL_NUM_READLINK          SP_SYSCALL_NUM_READLINKAT
  #define SP_SYSCALL_NUM_CHMOD             SP_SYSCALL_NUM_FCHMODAT
  #define SP_SYSCALL_NUM_FORK              SP_SYSCALL_NUM_CLONE
  #define SP_SYSCALL_NUM_DUP2              SP_SYSCALL_NUM_DUP3
  #define SP_SYSCALL_NUM_SYMLINK           SP_SYSCALL_NUM_SYMLINKAT
  #define SP_SYSCALL_NUM_LINK              SP_SYSCALL_NUM_LINKAT
#endif

///////////
// TYPES //
///////////
typedef struct {
  s32 wd;
  u32 mask;
  u32 cookie;
  u32 len;
  c8  name[];
} sp_sys_inotify_event_t;

typedef struct {
  u64 d_ino;
  s64 d_off;
  u16 d_reclen;
  u8  d_type;
  c8  d_name[];
} sp_sys_dirent64_t;

typedef struct {
  u16 ws_row;
  u16 ws_col;
  u16 ws_xpixel;
  u16 ws_ypixel;
} sp_sys_winsize_t;

#if defined(SP_AMD64)
  typedef struct {
    u64 st_dev;
    u64 st_ino;
    u64 st_nlink;
    u32 st_mode;
    u32 st_uid;
    u32 st_gid;
    u32 __pad0;
    u64 st_rdev;
    s64 st_size;
    s64 st_blksize;
    s64 st_blocks;
    u64 st_atime_sec;
    u64 st_atime_nsec;
    u64 st_mtime_sec;
    u64 st_mtime_nsec;
    u64 st_ctime_sec;
    u64 st_ctime_nsec;
    s64 __unused[3];
  } sp_sys_linux_stat_t;

#elif defined(SP_ARM64)
  typedef struct {
    u64 st_dev;
    u64 st_ino;
    u32 st_mode;
    u32 st_nlink;
    u32 st_uid;
    u32 st_gid;
    u64 st_rdev;
    u64 __pad;
    s64  st_size;
    s32  st_blksize;
    s32  __pad2;
    s64  st_blocks;
    u64 st_atime_sec;
    u64 st_atime_nsec;
    u64 st_mtime_sec;
    u64 st_mtime_nsec;
    u64 st_ctime_sec;
    u64 st_ctime_nsec;
    u32 padding [2];
  } sp_sys_linux_stat_t;
#endif

/////////////////////
// SYSCALL HELPERS //
/////////////////////
//
// This is ripped directly from musl. The only things of note here are:
// - We set errno the same way they do, even though we only use it for EINTR and I'm not totally sure whether
//   we ought to be using it for that. Point being, though, we set errno on every syscall error
// - The u64 cast in sp_syscall() is just for sign conversion warnings
#define __scc(X) ((s64) (X))

s64 __sp_syscall_ret(u64);
s64 __sp_syscall_cp(s64, s64, s64, s64, s64, s64, s64);
s64 sp_syscall0(s64 n);
s64 sp_syscall1(s64 n, s64 a1);
s64 sp_syscall2(s64 n, s64 a1, s64 a2);
s64 sp_syscall3(s64 n, s64 a1, s64 a2, s64 a3);
s64 sp_syscall4(s64 n, s64 a1, s64 a2, s64 a3, s64 a4);
s64 sp_syscall5(s64 n, s64 a1, s64 a2, s64 a3, s64 a4, s64 a5);
s64 sp_syscall6(s64 n, s64 a1, s64 a2, s64 a3, s64 a4, s64 a5, s64 a6);

#define __sp_syscall1(n,a) sp_syscall1(n,__scc(a))
#define __sp_syscall2(n,a,b) sp_syscall2(n,__scc(a),__scc(b))
#define __sp_syscall3(n,a,b,c) sp_syscall3(n,__scc(a),__scc(b),__scc(c))
#define __sp_syscall4(n,a,b,c,d) sp_syscall4(n,__scc(a),__scc(b),__scc(c),__scc(d))
#define __sp_syscall5(n,a,b,c,d,e) sp_syscall5(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __sp_syscall6(n,a,b,c,d,e,f) sp_syscall6(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __sp_syscall7(n,a,b,c,d,e,f,g) sp_syscall7(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __SP_SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SP_SYSCALL_NARGS(...) __SP_SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SP_SYSCALL_CONCAT_X(a,b) a##b
#define __SP_SYSCALL_CONCAT(a,b) __SP_SYSCALL_CONCAT_X(a,b)
#define __SP_SYSCALL_DISP(b,...) __SP_SYSCALL_CONCAT(b,__SP_SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __sp_syscall(...) __SP_SYSCALL_DISP(__sp_syscall,__VA_ARGS__)
#define sp_syscall(...) __sp_syscall_ret((u64)__sp_syscall(__VA_ARGS__))

s64 __sp_syscall_ret(u64 r) {
	if (r > -4096UL) {
		errno = (s32)-r;
		return -1;
	}
	return (s64)r;
}

s64 sp_syscall0(s64 n) {
  s64 ret;
#if defined(SP_AMD64)
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0");
  __asm__ __volatile__ ("svc 0" : "=r"(x0) : "r"(x8) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall1(s64 n, s64 a1) {
  s64 ret;
#if defined(SP_AMD64)
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall2(s64 n, s64 a1, s64 a2) {
  s64 ret;
#if defined(SP_AMD64)
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  register s64 x1 __asm__("x1") = a2;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall3(s64 n, s64 a1, s64 a2, s64 a3) {
  s64 ret;
#if defined(SP_AMD64)
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  register s64 x1 __asm__("x1") = a2;
  register s64 x2 __asm__("x2") = a3;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall4(s64 n, s64 a1, s64 a2, s64 a3, s64 a4) {
  s64 ret;
#if defined(SP_AMD64)
  register s64 r10 __asm__("r10") = a4;
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  register s64 x1 __asm__("x1") = a2;
  register s64 x2 __asm__("x2") = a3;
  register s64 x3 __asm__("x3") = a4;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall5(s64 n, s64 a1, s64 a2, s64 a3, s64 a4, s64 a5) {
  s64 ret;
#if defined(SP_AMD64)
  register s64 r10 __asm__("r10") = a4;
  register s64 r8  __asm__("r8")  = a5;
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  register s64 x1 __asm__("x1") = a2;
  register s64 x2 __asm__("x2") = a3;
  register s64 x3 __asm__("x3") = a4;
  register s64 x4 __asm__("x4") = a5;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4) : "memory");
  ret = x0;
#endif
  return ret;
}

s64 sp_syscall6(s64 n, s64 a1, s64 a2, s64 a3, s64 a4, s64 a5, s64 a6) {
  s64 ret;
#if defined(SP_AMD64)
  register s64 r10 __asm__("r10") = a4;
  register s64 r8  __asm__("r8")  = a5;
  register s64 r9  __asm__("r9")  = a6;
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
#elif defined(SP_ARM64)
  register s64 x8 __asm__("x8") = n;
  register s64 x0 __asm__("x0") = a1;
  register s64 x1 __asm__("x1") = a2;
  register s64 x2 __asm__("x2") = a3;
  register s64 x3 __asm__("x3") = a4;
  register s64 x4 __asm__("x4") = a5;
  register s64 x5 __asm__("x5") = a6;
  __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
  ret = x0;
#endif
  return ret;
}

//////////////////////
// SYSCALL WRAPPERS //
//////////////////////
s64   sp_sys_getdents64(s32 fd, void* buf, u64 count);
s32   sp_sys_inotify_init1(s32 flags);
s32   sp_sys_inotify_add_watch(s32 fd, const c8* pathname, u32 mask);
s32   sp_sys_inotify_rm_watch(s32 fd, s32 wd);
s32   sp_sys_wait4(s32 pid, s32* status, s32 options, void* rusage);
void  sp_sys_exit(s32 code);

s32 sp_sys_inotify_init1(s32 flags) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_INIT1, flags);
}

s32 sp_sys_inotify_add_watch(s32 fd, const c8* pathname, u32 mask) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_ADD_WATCH, fd, pathname, mask);
}

s32 sp_sys_inotify_rm_watch(s32 fd, s32 wd) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_RM_WATCH, fd, wd);
}

s32 sp_sys_wait4(s32 pid, s32* status, s32 options, void* rusage) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_WAIT4, pid, status, options, rusage);
}

//////////////
// PLATFORM //
//////////////
static void sp_sys_stat_from_linux(const sp_sys_linux_stat_t* raw, sp_sys_stat_t* out) {
  if      (SP_S_ISLNK(raw->st_mode)) out->kind = SP_FS_KIND_SYMLINK;
  else if (SP_S_ISDIR(raw->st_mode)) out->kind = SP_FS_KIND_DIR;
  else if (SP_S_ISREG(raw->st_mode)) out->kind = SP_FS_KIND_FILE;
  else                               out->kind = SP_FS_KIND_NONE;

  out->size             = raw->st_size;
  out->atime.tv_sec     = (s64)raw->st_atime_sec;
  out->atime.tv_nsec    = (s64)raw->st_atime_nsec;
  out->mtime.tv_sec     = (s64)raw->st_mtime_sec;
  out->mtime.tv_nsec    = (s64)raw->st_mtime_nsec;
  out->btime.tv_sec     = (s64)raw->st_ctime_sec;
  out->btime.tv_nsec    = (s64)raw->st_ctime_nsec;
  out->id               = raw->st_ino;
  out->device           = raw->st_dev;
  out->nlink            = raw->st_nlink;
  out->raw_attrs        = raw->st_mode;
}

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  sp_sys_linux_stat_t raw = SP_ZERO_INITIALIZE();
  s32 rc = (s32)sp_syscall(SP_SYSCALL_NUM_FSTAT, fd, &raw);
  if (rc == 0) sp_sys_stat_from_linux(&raw, st);
  return rc;
}

s64 sp_sys_getdents64(s32 fd, void* buf, u64 count) {
  return sp_syscall(SP_SYSCALL_NUM_GETDENTS64, fd, buf, count);
}

#endif

#if defined(SP_MACOS) || defined(SP_COSMO)
static void sp_sys_stat_from_libc(const struct stat* src, sp_sys_stat_t* out) {
  if      (S_ISLNK(src->st_mode)) out->kind = SP_FS_KIND_SYMLINK;
  else if (S_ISDIR(src->st_mode)) out->kind = SP_FS_KIND_DIR;
  else if (S_ISREG(src->st_mode)) out->kind = SP_FS_KIND_FILE;
  else                            out->kind = SP_FS_KIND_NONE;

  out->size      = (s64)src->st_size;
  out->id        = (u64)src->st_ino;
  out->device    = (u64)src->st_dev;
  out->nlink     = (u64)src->st_nlink;
  out->raw_attrs = (u32)src->st_mode;
#if defined(SP_MACOS)
  out->atime.tv_sec  = (s64)src->st_atimespec.tv_sec;
  out->atime.tv_nsec = (s64)src->st_atimespec.tv_nsec;
  out->mtime.tv_sec  = (s64)src->st_mtimespec.tv_sec;
  out->mtime.tv_nsec = (s64)src->st_mtimespec.tv_nsec;
  out->btime.tv_sec  = (s64)src->st_birthtimespec.tv_sec;
  out->btime.tv_nsec = (s64)src->st_birthtimespec.tv_nsec;
#else
  out->atime.tv_sec  = (s64)src->st_atim.tv_sec;
  out->atime.tv_nsec = (s64)src->st_atim.tv_nsec;
  out->mtime.tv_sec  = (s64)src->st_mtim.tv_sec;
  out->mtime.tv_nsec = (s64)src->st_mtim.tv_nsec;
  out->btime         = out->mtime;
#endif
}

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = fstat(fd, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}

#elif defined(SP_WIN32)

SP_PRIVATE void sp_rt_init(void);

#define SP_NT(fn) ((SP_UNLIKELY(!sp_rt.nt.fn) ? sp_tls_once(&sp_rt.tls.once, sp_rt_init) : (void)0), sp_rt.nt.fn)

SP_PRIVATE void sp_nt_load(void) {
  HMODULE h = GetModuleHandleW(L"ntdll.dll");
  if (!h) return;
  #define SP_NT_RESOLVE(ret, name, args) \
    sp_rt.nt.name = (ret (__stdcall*) args)(void(*)(void))GetProcAddress(h, #name);
  SP_NT_FUNCS(SP_NT_RESOLVE)
  #undef SP_NT_RESOLVE
}

SP_PRIVATE u8* sp_nt_peb_base(void) {
  u8* peb_base;
  #if defined(_MSC_VER)
    #if defined(_M_X64)
      peb_base = (u8*)__readgsqword(0x60);
    #elif defined(_M_ARM64)
      peb_base = *(u8**)(_ReadStatusReg(0x5E82) + 0x60);
    #else
      #error "sp.h requires 64-bit Windows"
    #endif
  #else
    #if defined(__x86_64__)
      __asm__ volatile ("movq %%gs:0x60, %0" : "=r"(peb_base));
    #elif defined(__aarch64__)
      __asm__ volatile ("ldr %0, [x18, #0x60]" : "=r"(peb_base));
    #else
      #error "sp.h requires 64-bit Windows"
    #endif
  #endif
  return peb_base;
}

SP_PRIVATE void* sp_nt_process_heap(void) {
  return *(void**)(sp_nt_peb_base() + 0x30);
}

SP_PRIVATE u8* sp_nt_process_params(void) {
  return *(u8**)(sp_nt_peb_base() + 0x20);
}

sp_nt_status_t sp_sys_nt_path(sp_str_t utf8, sp_sys_nt_path_t* out) {
  *out = SP_ZERO_STRUCT(sp_sys_nt_path_t);
  if (sp_str_empty(utf8)) return SP_NT_STATUS_OBJECT_NAME_INVALID;

  sp_wide_str_t wpath = sp_wtf8_to_wtf16(utf8);
  if (!wpath.data) return SP_NT_STATUS_OBJECT_NAME_INVALID;

  sp_nt_unicode_string_t nt = SP_ZERO_INITIALIZE();
  u16* file_part = SP_NULLPTR;
  sp_nt_status_t status = SP_NT(RtlDosPathNameToNtPathName_U_WithStatus)(wpath.data, &nt, &file_part, SP_NULLPTR);
  if (!SP_NT_SUCCESS(status)) return status;

  out->name = nt;
  out->heap_buffer = nt.Buffer;
  return SP_NT_STATUS_SUCCESS;
}

void sp_sys_nt_path_free(sp_sys_nt_path_t* path) {
  if (!path || !path->heap_buffer) return;
  SP_NT(RtlFreeHeap)(sp_nt_process_heap(), 0, path->heap_buffer);
  path->heap_buffer = SP_NULLPTR;
  path->name = SP_ZERO_STRUCT(sp_nt_unicode_string_t);
}

#define SP_NT_FILE_SUPERSEDE    0x00000000
#define SP_NT_FILE_OPEN         0x00000001
#define SP_NT_FILE_CREATE       0x00000002
#define SP_NT_FILE_OPEN_IF      0x00000003
#define SP_NT_FILE_OVERWRITE    0x00000004
#define SP_NT_FILE_OVERWRITE_IF 0x00000005

#define SP_NT_FILE_DIRECTORY_FILE              0x00000001
#define SP_NT_FILE_WRITE_THROUGH               0x00000002
#define SP_NT_FILE_SEQUENTIAL_ONLY             0x00000004
#define SP_NT_FILE_NO_INTERMEDIATE_BUFFERING   0x00000008
#define SP_NT_FILE_SYNCHRONOUS_IO_ALERT        0x00000010
#define SP_NT_FILE_SYNCHRONOUS_IO_NONALERT     0x00000020
#define SP_NT_FILE_NON_DIRECTORY_FILE          0x00000040
#define SP_NT_FILE_OPEN_FOR_BACKUP_INTENT      0x00004000
#define SP_NT_FILE_DELETE_ON_CLOSE             0x00001000
#define SP_NT_FILE_OPEN_REPARSE_POINT          0x00200000

#define SP_NT_STATUS_OBJECT_NAME_COLLISION ((sp_nt_status_t)0xC0000035)
#define SP_NT_STATUS_OBJECT_NAME_NOT_FOUND ((sp_nt_status_t)0xC0000034)
#define SP_NT_STATUS_OBJECT_PATH_NOT_FOUND ((sp_nt_status_t)0xC000003A)
#define SP_NT_STATUS_ACCESS_DENIED         ((sp_nt_status_t)0xC0000022)

SP_PRIVATE u32 sp_sys_nt_disposition_from_flags(s32 flags) {
  if ((flags & SP_O_CREAT) && (flags & SP_O_EXCL))  return SP_NT_FILE_CREATE;
  if ((flags & SP_O_CREAT) && (flags & SP_O_TRUNC)) return SP_NT_FILE_OVERWRITE_IF;
  if (flags & SP_O_CREAT)                            return SP_NT_FILE_OPEN_IF;
  if (flags & SP_O_TRUNC)                            return SP_NT_FILE_OVERWRITE;
  return SP_NT_FILE_OPEN;
}

SP_PRIVATE void* sp_sys_nt_open(sp_str_t utf8, u32 access, u32 share, u32 disposition, u32 options, u32 file_attr) {
  sp_sys_nt_path_t nt = SP_ZERO_INITIALIZE();
  if (!SP_NT_SUCCESS(sp_sys_nt_path(utf8, &nt))) return SP_NULLPTR;

  sp_nt_object_attributes_t attr = {
    .Length = sizeof(sp_nt_object_attributes_t),
    .ObjectName = &nt.name,
    .Attributes = SP_NT_OBJ_CASE_INSENSITIVE,
  };

  void* handle = SP_NULLPTR;
  sp_nt_io_status_block_t iosb = SP_ZERO_INITIALIZE();
  sp_nt_status_t status = SP_NT(NtCreateFile)(
    &handle, access, &attr, &iosb, SP_NULLPTR,
    file_attr, share, disposition, options, SP_NULLPTR, 0
  );

  sp_sys_nt_path_free(&nt);

  if (!SP_NT_SUCCESS(status)) return SP_NULLPTR;
  return handle;
}

#define SP_NT_FILE_BASIC_INFORMATION      4
#define SP_NT_FILE_RENAME_INFORMATION    10
#define SP_NT_FILE_DISPOSITION_INFORMATION 13
#define SP_NT_FILE_LINK_INFORMATION      11

typedef struct {
  s64 CreationTime;
  s64 LastAccessTime;
  s64 LastWriteTime;
  s64 ChangeTime;
  u32 FileAttributes;
  u32 Reserved;
} sp_nt_file_basic_information_t;

typedef struct {
  u8 ReplaceIfExists;
} sp_nt_file_disposition_information_t;

static void sp_sys_timespec_from_filetime(FILETIME ft, sp_sys_timespec_t* out) {
  u64 t = ((u64)ft.dwHighDateTime << 32) | (u64)ft.dwLowDateTime;
  if (t >= 116444736000000000ULL) {
    t -= 116444736000000000ULL;
    out->tv_sec  = (s64)(t / 10000000ULL);
    out->tv_nsec = (s64)((t % 10000000ULL) * 100);
  } else {
    out->tv_sec = 0;
    out->tv_nsec = 0;
  }
}

static s32 sp_sys_stat_from_nt_handle(HANDLE handle, sp_sys_stat_t* out) {
  BY_HANDLE_FILE_INFORMATION info;
  if (!GetFileInformationByHandle(handle, &info)) return -1;

  sp_sys_stat_t st = SP_ZERO_INITIALIZE();

  if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    st.kind = SP_FS_KIND_SYMLINK;
  } else if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    st.kind = SP_FS_KIND_DIR;
  } else {
    st.kind = SP_FS_KIND_FILE;
  }

  st.size      = (s64)(((u64)info.nFileSizeHigh << 32) | (u64)info.nFileSizeLow);
  st.id        = ((u64)info.nFileIndexHigh << 32) | (u64)info.nFileIndexLow;
  st.device    = info.dwVolumeSerialNumber;
  st.nlink     = info.nNumberOfLinks;
  st.raw_attrs = info.dwFileAttributes;

  sp_sys_timespec_from_filetime(info.ftLastAccessTime, &st.atime);
  sp_sys_timespec_from_filetime(info.ftLastWriteTime,  &st.mtime);
  sp_sys_timespec_from_filetime(info.ftCreationTime,   &st.btime);

  *out = st;
  return 0;
}

static s32 sp_sys_stat_from_nt_path(const c8* path, sp_sys_stat_t* out, bool follow_symlinks) {
  u32 options = SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_FOR_BACKUP_INTENT;
  if (!follow_symlinks) options |= SP_NT_FILE_OPEN_REPARSE_POINT;
  void* h = sp_sys_nt_open(
    sp_str_view(path),
    FILE_READ_ATTRIBUTES | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN, options, 0
  );
  if (!h) return -1;
  s32 rc = sp_sys_stat_from_nt_handle((HANDLE)h, out);
  SP_NT(NtClose)(h);
  return rc;
}

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  if (fd == SP_SYS_INVALID_FD) return -1;
  return sp_sys_stat_from_nt_handle((HANDLE)fd, st);
}
#endif

// SP_SYS_RENAME //
#if defined(SP_WIN32)
typedef struct {
  u8 ReplaceIfExists;
  u8 _pad[7];
  void* RootDirectory;
  u32 FileNameLength;
  u16 FileName[1];
} sp_nt_file_rename_information_t;

typedef sp_nt_file_rename_information_t sp_nt_file_link_information_t;

s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
  void* handle = sp_sys_nt_open(
    sp_str_view(oldpath),
    DELETE | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_REPARSE_POINT,
    0
  );
  if (!handle) return -1;

  sp_sys_nt_path_t nt = SP_ZERO_INITIALIZE();
  if (!SP_NT_SUCCESS(sp_sys_nt_path(sp_str_view(newpath), &nt))) {
    SP_NT(NtClose)(handle);
    return -1;
  }

  u32 name_bytes = nt.name.Length;
  u32 info_bytes = sizeof(sp_nt_file_rename_information_t) + name_bytes - sizeof(u16);
  sp_nt_file_rename_information_t* info = (sp_nt_file_rename_information_t*)sp_alloc(info_bytes);
  *info = SP_ZERO_STRUCT(sp_nt_file_rename_information_t);
  info->ReplaceIfExists = 1;
  info->FileNameLength = name_bytes;
  sp_mem_copy(nt.name.Buffer, info->FileName, name_bytes);

  sp_nt_io_status_block_t iosb = SP_ZERO_INITIALIZE();
  sp_nt_status_t status = SP_NT(NtSetInformationFile)(handle, &iosb, info, info_bytes, SP_NT_FILE_RENAME_INFORMATION);

  sp_sys_nt_path_free(&nt);
  SP_NT(NtClose)(handle);

  return SP_NT_SUCCESS(status) ? 0 : -1;
}

#elif defined(SP_LINUX)
s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_RENAME, oldpath, newpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_RENAMEAT, SP_AT_FDCWD, oldpath, SP_AT_FDCWD, newpath);
#endif
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
  return rename(oldpath, newpath);
}

#elif defined(SP_WASM)
s32 sp_sys_rename(const c8* from, const c8* to) {
  return -1;
}
#else
#error ""
#endif

//////////////////
// SP_SYS_CLOSE //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_close(sp_sys_fd_t fd) {
  if (fd == SP_SYS_INVALID_FD) return -1;
  return CloseHandle((HANDLE)fd) ? 0 : -1;
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_close(sp_sys_fd_t fd) {
  return close(fd);
}

#elif defined(SP_LINUX)
s32 sp_sys_close(sp_sys_fd_t fd) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CLOSE, fd);
}

#elif defined(SP_WASM)
s32 sp_sys_close(s32 fd) {
  return __wasi_fd_close(fd) ? -1 : 0;
}

#else
#error "sp_sys_close"
#endif

/////////////////
// SP_SYS_PIPE //
/////////////////
#if defined(SP_WIN32)
s32 sp_sys_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end) {
  HANDLE r = SP_NULLPTR;
  HANDLE w = SP_NULLPTR;
  SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), SP_NULLPTR, FALSE };
  if (!CreatePipe(&r, &w, &sa, 0)) return -1;
  DWORD mode = PIPE_NOWAIT;
  if (!SetNamedPipeHandleState(r, &mode, SP_NULLPTR, SP_NULLPTR)) {
    CloseHandle(r);
    CloseHandle(w);
    return -1;
  }
  *read_end = (sp_sys_fd_t)r;
  *write_end = (sp_sys_fd_t)w;
  return 0;
}

#elif defined(SP_LINUX)
s32 sp_sys_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end) {
  s32 fds[2];
  s32 r = (s32)sp_syscall(SP_SYSCALL_NUM_PIPE2, fds, SP_O_NONBLOCK | SP_O_CLOEXEC, 0, 0, 0);
  if (r < 0) return -1;
  *read_end = fds[0];
  *write_end = fds[1];
  return 0;
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end) {
  s32 fds[2];
  if (pipe(fds) < 0) return -1;
  fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK);
  fcntl(fds[0], F_SETFD, fcntl(fds[0], F_GETFD) | FD_CLOEXEC);
  fcntl(fds[1], F_SETFD, fcntl(fds[1], F_GETFD) | FD_CLOEXEC);
  *read_end = fds[0];
  *write_end = fds[1];
  return 0;
}

#elif defined(SP_WASM)
s32 sp_sys_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end) {
  *read_end = SP_SYS_INVALID_FD;
  *write_end = SP_SYS_INVALID_FD;
  return -1;
}

#else
#error "sp_sys_pipe"
#endif

//////////////////
// SP_SYS_READ //
//////////////////
#if defined(SP_WIN32)
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  DWORD n = 0;
  if (!ReadFile((HANDLE)fd, buf, (DWORD)count, &n, SP_NULLPTR)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) return 0;
    return -1;
  }
  return (s64)n;
}

#elif defined(SP_LINUX)
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  s64 rc;
  do {
    rc = sp_syscall(SP_SYSCALL_NUM_READ, fd, buf, count);
  } while (rc == -1 && errno == SP_EINTR);
  return rc;
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  s64 rc;
  do {
    rc = read(fd, buf, count);
  } while (rc == -1 && errno == SP_EINTR);
  return rc;
}

#elif defined(SP_WASM)
s64 sp_sys_read(s32 fd, void* buf, u64 count) {
  __wasi_iovec_t iov = { (uint8_t*)buf, (__wasi_size_t)count };
  __wasi_size_t n = 0;
  __wasi_errno_t err = __wasi_fd_read((__wasi_fd_t)fd, &iov, 1, &n);
  return err ? -1 : (s64)n;
}

#else
#error "sp_sys_read"
#endif

///////////////////
// SP_SYS_WRITE //
///////////////////
#if defined(SP_WIN32)
s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  DWORD n = 0;
  if (!WriteFile((HANDLE)fd, buf, (DWORD)count, &n, SP_NULLPTR)) return -1;
  return (s64)n;
}

#elif defined(SP_LINUX)
s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  s64 rc;
  do {
    rc = sp_syscall(SP_SYSCALL_NUM_WRITE, fd, buf, count);
  } while (rc == -1 && errno == SP_EINTR);
  return rc;
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  s64 rc;
  do {
    rc = write(fd, buf, count);
  } while (rc == -1 && errno == SP_EINTR);
  return rc;
}

#elif defined(SP_WASM)
s64 sp_sys_write(s32 fd, const void* buf, u64 count) {
  __wasi_ciovec_t iov = { (const uint8_t*)buf, (__wasi_size_t)count };
  __wasi_size_t n = 0;
  __wasi_errno_t err = __wasi_fd_write((__wasi_fd_t)fd, &iov, 1, &n);
  return err ? -1 : (s64)n;
}

#else
#error "sp_sys_write"
#endif

//////////////////
// SP_SYS_LSEEK //
//////////////////
#if defined(SP_WIN32)
s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  DWORD method;
  switch (whence) {
    case SP_IO_SEEK_SET: method = FILE_BEGIN;   break;
    case SP_IO_SEEK_CUR: method = FILE_CURRENT; break;
    case SP_IO_SEEK_END: method = FILE_END;     break;
    default: return -1;
  }
  LARGE_INTEGER dist = { .QuadPart = offset };
  LARGE_INTEGER new_pos;
  if (!SetFilePointerEx((HANDLE)fd, dist, &new_pos, method)) return -1;
  return (s64)new_pos.QuadPart;
}

#elif defined(SP_LINUX)
s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  s32 native;
  switch (whence) {
    case SP_IO_SEEK_SET: native = 0; break;
    case SP_IO_SEEK_CUR: native = 1; break;
    case SP_IO_SEEK_END: native = 2; break;
    default: return -1;
  }
  return sp_syscall(SP_SYSCALL_NUM_LSEEK, fd, offset, native);
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  s32 native;
  switch (whence) {
    case SP_IO_SEEK_SET: native = SEEK_SET; break;
    case SP_IO_SEEK_CUR: native = SEEK_CUR; break;
    case SP_IO_SEEK_END: native = SEEK_END; break;
    default: return -1;
  }
  return lseek(fd, offset, native);
}

#elif defined(SP_WASM)
s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  __wasi_whence_t native;
  switch (whence) {
    case SP_IO_SEEK_SET: native = __WASI_WHENCE_SET; break;
    case SP_IO_SEEK_CUR: native = __WASI_WHENCE_CUR; break;
    case SP_IO_SEEK_END: native = __WASI_WHENCE_END; break;
    default: return -1;
  }
  __wasi_filesize_t newoff = 0;
  __wasi_errno_t err = __wasi_fd_seek((__wasi_fd_t)fd, (__wasi_filedelta_t)offset, native, &newoff);
  return err ? -1 : (s64)newoff;
}
#else
#error "sp_sys_lseek"
#endif



///////////////////////////
// SP_SYS_CLOCK_GETTIME //
//////////////////////////
#if defined(SP_WIN32)
s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  if (clockid == SP_CLOCK_MONOTONIC) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    u64 seconds   = (u64)(counter.QuadPart / freq.QuadPart);
    u64 remainder = (u64)(counter.QuadPart % freq.QuadPart);
    ts->tv_sec  = (s64)seconds;
    ts->tv_nsec = (s64)((remainder * 1000000000ULL) / (u64)freq.QuadPart);
    return 0;
  }
  if (clockid == SP_CLOCK_REALTIME) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    u64 windows_100ns = ((u64)ft.dwHighDateTime << 32) | (u64)ft.dwLowDateTime;
    u64 unix_100ns    = windows_100ns - 116444736000000000ULL;
    ts->tv_sec  = (s64)(unix_100ns / 10000000ULL);
    ts->tv_nsec = (s64)((unix_100ns % 10000000ULL) * 100);
    return 0;
  }
  return -1;
}

#elif defined(SP_LINUX)
s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CLOCK_GETTIME, clockid, ts);
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  struct timespec native;
  s32 rc = clock_gettime((clockid_t)clockid, &native);
  if (rc == 0) {
    ts->tv_sec  = (s64)native.tv_sec;
    ts->tv_nsec = (s64)native.tv_nsec;
  }
  return rc;
}

#elif defined(SP_WASM)
#define SP_CLOCK_REALTIME 0
#define SP_CLOCK_MONOTONIC 1
s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  __wasi_timestamp_t ns = 0;
  __wasi_errno_t err = __wasi_clock_time_get((__wasi_clockid_t)clockid, 1000, &ns);
  if (err) return -1;
  ts->tv_sec  = (s64)(ns / 1000000000ULL);
  ts->tv_nsec = (s64)(ns % 1000000000ULL);
  return 0;
}
#else
#error "sp_sys_clock_gettime"
#endif

/////////////////
// SP_SYS_OPEN //
/////////////////
#if defined(SP_WIN32)
SP_PRIVATE u32 sp_sys_nt_access_from_flags(s32 flags) {
  u32 access = SYNCHRONIZE | FILE_READ_ATTRIBUTES;
  s32 rw = flags & (SP_O_RDONLY | SP_O_WRONLY | SP_O_RDWR);
  if (rw == SP_O_WRONLY) {
    access |= FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA;
  }
  else if (rw == SP_O_RDWR) {
    access |= FILE_READ_DATA | FILE_READ_EA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA;
  }
  else {
    access |= FILE_READ_DATA | FILE_READ_EA;
  }
  if (flags & SP_O_APPEND) access |= FILE_APPEND_DATA;
  return access;
}


sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  (void)mode;
  u32 access = sp_sys_nt_access_from_flags(flags);
  u32 share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  u32 disposition = sp_sys_nt_disposition_from_flags(flags);
  u32 options = SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_NON_DIRECTORY_FILE | SP_NT_FILE_OPEN_FOR_BACKUP_INTENT;

  void* handle = sp_sys_nt_open(sp_str_view(path), access, share, disposition, options, FILE_ATTRIBUTE_NORMAL);
  if (!handle) return SP_SYS_INVALID_FD;

  if (flags & SP_O_APPEND) {
    LARGE_INTEGER zero = { .QuadPart = 0 };
    SetFilePointerEx((HANDLE)handle, zero, SP_NULLPTR, FILE_END);
  }

  return (sp_sys_fd_t)handle;
}
#elif defined(SP_LINUX)
sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  return (sp_sys_fd_t)sp_syscall(SP_SYSCALL_NUM_OPENAT, SP_AT_FDCWD, path, flags, mode);
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  return (sp_sys_fd_t)open(path, flags, mode);
}

#elif defined(SP_WASM)
#define SP_O_CREAT 0
#define SP_O_WRONLY 0
#define SP_O_TRUNC 0
#define SP_O_BINARY 0
#define SP_O_DIRECTORY 0
#define SP_O_RDONLY 0
#define SP_O_APPEND 0
sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  return -1;
}
#else
#error "sp_sys_open"
#endif

//////////////////////
// SP_SYS_NANOSLEEP //
//////////////////////
#if defined(SP_WIN32)
s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  (void)rem;
  u64 ns = (u64)req->tv_sec * 1000000000ULL + (u64)req->tv_nsec;
  Sleep((DWORD)(ns / 1000000ULL));
  return 0;
}

#elif defined(SP_LINUX)
s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  s32 rc = (s32)sp_syscall(SP_SYSCALL_NUM_NANOSLEEP, req, rem);
  while (rc == -1 && errno == SP_EINTR) {
    rc = (s32)sp_syscall(SP_SYSCALL_NUM_NANOSLEEP, rem, rem);
  }
  return rc;
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  struct timespec r_native   = { .tv_sec = (time_t)req->tv_sec, .tv_nsec = (long)req->tv_nsec };
  struct timespec rem_native = SP_ZERO_INITIALIZE();
  s32 rc = nanosleep(&r_native, &rem_native);
  while (rc == -1 && errno == EINTR) {
    r_native = rem_native;
    rc = nanosleep(&r_native, &rem_native);
  }
  if (rem) {
    rem->tv_sec  = (s64)rem_native.tv_sec;
    rem->tv_nsec = (s64)rem_native.tv_nsec;
  }
  return rc;
}

#elif defined(SP_WASM)
s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  __wasi_timestamp_t ns = (__wasi_timestamp_t)(req->tv_sec * SP_TM_S_TO_NS + req->tv_nsec);
  __wasi_subscription_t sub = {
    .userdata = 0,
    .u = {
      .tag = __WASI_EVENTTYPE_CLOCK,
      .u = { .clock = {
        .id = 1,
        .timeout = ns,
        .precision = 10000000,
        .flags = 0,
      }},
    },
  };
  __wasi_event_t event;
  __wasi_size_t num_events = 0;
  (void)__wasi_poll_oneoff(&sub, &event, 1, &num_events);

  if (rem) {
    rem->tv_sec = 0;
    rem->tv_nsec = 0;
  }
  return 0;
}

#else
#error "sp_sys_nanosleep"
#endif

#if defined(SP_WIN32)
static s64 sp_sys_copy_env_var(const c8* name, c8* buf, u64 size) {
  DWORD len = GetEnvironmentVariableA(name, buf, (DWORD)size);
  if (len == 0 || len >= size) return -1;
  return (s64)len;
}

s64 sp_sys_get_storage_path(c8* buf, u64 size) {
  return sp_sys_copy_env_var("LOCALAPPDATA", buf, size);
}

s64 sp_sys_get_config_path(c8* buf, u64 size) {
  return sp_sys_copy_env_var("APPDATA", buf, size);
}
#else
static s64 sp_sys_xdg_or_home(sp_str_t xdg_var, sp_str_t home_suffix, c8* buf, u64 size) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  sp_str_t value = sp_os_env_get(xdg_var);
  if (sp_str_empty(value)) {
    sp_str_t home = sp_os_env_get(SP_LIT("HOME"));
    if (!sp_str_empty(home)) {
      value = sp_str_join_a(sp_mem_scratch_allocator_a(), home, home_suffix, SP_LIT("/"));
    }
  }

  s64 result = -1;
  if (!sp_str_empty(value) && value.len < size) {
    for (u32 i = 0; i < value.len; i++) buf[i] = value.data[i];
    buf[value.len] = '\0';
    result = (s64)value.len;
  }

  sp_mem_end_scratch(scratch);
  return result;
}

s64 sp_sys_get_storage_path(c8* buf, u64 size) {
  return sp_sys_xdg_or_home(SP_LIT("XDG_DATA_HOME"), SP_LIT(".local/share"), buf, size);
}

s64 sp_sys_get_config_path(c8* buf, u64 size) {
  return sp_sys_xdg_or_home(SP_LIT("XDG_CONFIG_HOME"), SP_LIT(".config"), buf, size);
}
#endif

//////////////////
// SP_SYS_READY //
//////////////////
#define SP_SYS_FDS_WAIT_CAP 64

#if defined(SP_WIN32)
static s32 sp_sys_fd_ready_handle(HANDLE h, u8* out_ready) {
  *out_ready = 0;
  if (h == SP_NULLPTR || h == INVALID_HANDLE_VALUE) return 0;

  DWORD type = GetFileType(h);
  if (type == FILE_TYPE_PIPE) {
    DWORD avail = 0;
    if (!PeekNamedPipe(h, SP_NULLPTR, 0, SP_NULLPTR, &avail, SP_NULLPTR)) {
      *out_ready = 1;
      return 0;
    }
    *out_ready = avail > 0 ? 1 : 0;
    return 0;
  }
  if (type == FILE_TYPE_CHAR) {
    DWORD console_mode = 0;
    if (GetConsoleMode(h, &console_mode)) {
      for (;;) {
        DWORD num_events = 0;
        if (!GetNumberOfConsoleInputEvents(h, &num_events) || num_events == 0) return 0;

        INPUT_RECORD rec;
        DWORD peeked = 0;
        if (!PeekConsoleInputW(h, &rec, 1, &peeked) || peeked == 0) return 0;
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
          *out_ready = 1;
          return 0;
        }

        DWORD consumed = 0;
        if (!ReadConsoleInputW(h, &rec, 1, &consumed) || consumed == 0) return 0;
      }
    }
    *out_ready = (WaitForSingleObject(h, 0) == WAIT_OBJECT_0) ? 1 : 0;
    return 0;
  }
  if (type == FILE_TYPE_DISK) {
    *out_ready = 1;
    return 0;
  }
  if (type == FILE_TYPE_UNKNOWN) {
    return 0;
  }
  *out_ready = (WaitForSingleObject(h, 0) == WAIT_OBJECT_0) ? 1 : 0;
  return 0;
}

s32 sp_sys_fd_ready(sp_sys_fd_t fd, u8* ready) {
  return sp_sys_fd_ready_handle((HANDLE)fd, ready);
}

s32 sp_sys_fd_wait(sp_sys_fd_t fd) {
  for (;;) {
    u8 ready = 0;
    if (sp_sys_fd_ready_handle((HANDLE)fd, &ready) != 0) return -1;
    if (ready) return 0;
    Sleep(1);
  }
}

s32 sp_sys_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds) {
  if (nfds == 0) return 0;
  if (nfds > SP_SYS_FDS_WAIT_CAP) return -1;
  for (;;) {
    s32 count = 0;
    for (u64 i = 0; i < nfds; i++) {
      ready[i] = 0;
      if (sp_sys_fd_ready_handle((HANDLE)fds[i], &ready[i]) != 0) return -1;
      if (ready[i]) count++;
    }
    if (count > 0) return count;
    Sleep(1);
  }
}

#elif defined(SP_LINUX)
typedef struct {
  s32 fd;
  s16 events;
  s16 revents;
} sp_sys_linux_pollfd_t;

#define SP_SYS_LINUX_POLLIN  0x0001
#define SP_SYS_LINUX_POLLERR 0x0008
#define SP_SYS_LINUX_POLLHUP 0x0010

s32 sp_sys_fd_ready(sp_sys_fd_t fd, u8* ready) {
  *ready = 0;
  sp_sys_linux_pollfd_t pfd = { .fd = fd, .events = SP_SYS_LINUX_POLLIN };
  sp_sys_timespec_t ts = { 0, 0 };
  s32 r = (s32)sp_syscall(SP_SYSCALL_NUM_PPOLL, &pfd, 1, &ts, 0, 0);
  if (r < 0) return -1;
  if (r > 0 && (pfd.revents & (SP_SYS_LINUX_POLLIN | SP_SYS_LINUX_POLLHUP | SP_SYS_LINUX_POLLERR))) *ready = 1;
  return 0;
}

s32 sp_sys_fd_wait(sp_sys_fd_t fd) {
  sp_sys_linux_pollfd_t pfd = { .fd = fd, .events = SP_SYS_LINUX_POLLIN };
  s32 r = (s32)sp_syscall(SP_SYSCALL_NUM_PPOLL, &pfd, 1, SP_NULLPTR, 0, 0);
  return r < 0 ? -1 : 0;
}

s32 sp_sys_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds) {
  if (nfds == 0) return 0;
  if (nfds > SP_SYS_FDS_WAIT_CAP) return -1;
  sp_sys_linux_pollfd_t pfds[SP_SYS_FDS_WAIT_CAP];
  for (u64 i = 0; i < nfds; i++) {
    pfds[i] = (sp_sys_linux_pollfd_t){ .fd = fds[i], .events = SP_SYS_LINUX_POLLIN };
    ready[i] = 0;
  }
  s32 r = (s32)sp_syscall(SP_SYSCALL_NUM_PPOLL, pfds, nfds, SP_NULLPTR, 0, 0);
  if (r < 0) return -1;
  s32 count = 0;
  for (u64 i = 0; i < nfds; i++) {
    if (pfds[i].revents & (SP_SYS_LINUX_POLLIN | SP_SYS_LINUX_POLLHUP | SP_SYS_LINUX_POLLERR)) {
      ready[i] = 1;
      count++;
    }
  }
  return count;
}

#elif defined(SP_WASM)
static s32 sp_sys_fd_wasi_ready(sp_sys_fd_t fd, u8* out_ready) {
  *out_ready = 0;
  __wasi_subscription_t subs[2] = {
    {
      .userdata = 0,
      .u = {
        .tag = __WASI_EVENTTYPE_FD_READ,
        .u = { .fd_read = { .file_descriptor = (__wasi_fd_t)fd } },
      },
    },
    {
      .userdata = 1,
      .u = {
        .tag = __WASI_EVENTTYPE_CLOCK,
        .u = { .clock = { .id = 1, .timeout = 0, .precision = 0, .flags = 0 } },
      },
    },
  };
  __wasi_event_t events[2];
  __wasi_size_t nev = 0;
  if (__wasi_poll_oneoff(subs, events, 2, &nev)) return -1;
  for (__wasi_size_t i = 0; i < nev; i++) {
    if (events[i].userdata == 0) {
      *out_ready = 1;
      return 0;
    }
  }
  return 0;
}

s32 sp_sys_fd_ready(sp_sys_fd_t fd, u8* ready) {
  return sp_sys_fd_wasi_ready(fd, ready);
}

s32 sp_sys_fd_wait(sp_sys_fd_t fd) {
  __wasi_subscription_t sub = {
    .userdata = 0,
    .u = {
      .tag = __WASI_EVENTTYPE_FD_READ,
      .u = { .fd_read = { .file_descriptor = (__wasi_fd_t)fd } },
    },
  };
  __wasi_event_t event;
  __wasi_size_t nev = 0;
  if (__wasi_poll_oneoff(&sub, &event, 1, &nev)) return -1;
  return 0;
}

s32 sp_sys_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds) {
  if (nfds == 0) return 0;
  if (nfds > SP_SYS_FDS_WAIT_CAP) return -1;
  __wasi_subscription_t subs[SP_SYS_FDS_WAIT_CAP];
  __wasi_event_t        events[SP_SYS_FDS_WAIT_CAP];
  for (u64 i = 0; i < nfds; i++) {
    subs[i] = (__wasi_subscription_t){
      .userdata = (__wasi_userdata_t)i,
      .u = {
        .tag = __WASI_EVENTTYPE_FD_READ,
        .u = { .fd_read = { .file_descriptor = (__wasi_fd_t)fds[i] } },
      },
    };
    ready[i] = 0;
  }
  __wasi_size_t nev = 0;
  if (__wasi_poll_oneoff(subs, events, (__wasi_size_t)nfds, &nev)) return -1;
  s32 count = 0;
  for (__wasi_size_t i = 0; i < nev; i++) {
    u64 idx = (u64)events[i].userdata;
    if (idx < nfds && !ready[idx]) {
      ready[idx] = 1;
      count++;
    }
  }
  return count;
}

#else
s32 sp_sys_fd_ready(sp_sys_fd_t fd, u8* ready) {
  *ready = 0;
  struct pollfd pfd = { .fd = fd, .events = POLLIN };
  s32 r = (s32)poll(&pfd, 1, 0);
  if (r < 0) return -1;
  if (r > 0 && (pfd.revents & (POLLIN | POLLHUP | POLLERR))) *ready = 1;
  return 0;
}

s32 sp_sys_fd_wait(sp_sys_fd_t fd) {
  struct pollfd pfd = { .fd = fd, .events = POLLIN };
  s32 r = (s32)poll(&pfd, 1, -1);
  return r < 0 ? -1 : 0;
}

s32 sp_sys_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds) {
  if (nfds == 0) return 0;
  if (nfds > SP_SYS_FDS_WAIT_CAP) return -1;
  struct pollfd pfds[SP_SYS_FDS_WAIT_CAP];
  for (u64 i = 0; i < nfds; i++) {
    pfds[i] = (struct pollfd){ .fd = fds[i], .events = POLLIN };
    ready[i] = 0;
  }
  s32 r = (s32)poll(pfds, (nfds_t)nfds, -1);
  if (r < 0) return -1;
  s32 count = 0;
  for (u64 i = 0; i < nfds; i++) {
    if (pfds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
      ready[i] = 1;
      count++;
    }
  }
  return count;
}

#endif

////////////////
// SP_SYS_MEM //
////////////////
#if defined(SP_FREESTANDING)
__attribute__((weak))
void* memcpy(void* dest, const void* src, u64 n) {
#if defined(SP_AMD64)
  void* ret = dest;
  if (n == 0) return ret;
  __asm__ __volatile__ (
    "cmp $8, %[n]\n\t"
    "jc 2f\n\t"
    "test $7, %%edi\n\t"
    "jz 1f\n"
    "3:\n\t"
    "movsb\n\t"
    "dec %[n]\n\t"
    "test $7, %%edi\n\t"
    "jnz 3b\n"
    "1:\n\t"
    "mov %[n], %%rcx\n\t"
    "shr $3, %%rcx\n\t"
    "rep movsq\n\t"
    "and $7, %[n]\n\t"
    "jz 4f\n"
    "2:\n\t"
    "movsb\n\t"
    "dec %[n]\n\t"
    "jnz 2b\n"
    "4:\n"
    : [n] "+d" (n), "+D" (dest), "+S" (src)
    :
    : "rcx", "memory"
  );
  return ret;
#elif defined(SP_ARM64)
  u8* d = (u8*)dest;
  const u8* s = (const u8*)src;
  if (n == 0) return dest;
  if (n < 16) {
    while (n--) *d++ = *s++;
    return dest;
  }
  u64* d64;
  const u64* s64;
  __asm__ __volatile__ (
    "ldp x4, x5, [%[src]]\n\t"
    "stp x4, x5, [%[dst]]"
    : : [src] "r" (s), [dst] "r" (d)
    : "x4", "x5", "memory"
  );
  if (n <= 32) {
    __asm__ __volatile__ (
      "ldp x4, x5, [%[src]]\n\t"
      "stp x4, x5, [%[dst]]"
      : : [src] "r" (s + n - 16), [dst] "r" (d + n - 16)
      : "x4", "x5", "memory"
    );
    return dest;
  }
  size_t align = 16 - ((uintptr_t)d & 15);
  if (align < 16) { d += align; s += align; n -= align; }
  d64 = (u64*)d;
  s64 = (const u64*)s;
  while (n >= 64) {
    __asm__ __volatile__ (
      "ldp x4, x5, [%[src]]\n\t"
      "ldp x6, x7, [%[src], #16]\n\t"
      "ldp x8, x9, [%[src], #32]\n\t"
      "ldp x10, x11, [%[src], #48]\n\t"
      "stp x4, x5, [%[dst]]\n\t"
      "stp x6, x7, [%[dst], #16]\n\t"
      "stp x8, x9, [%[dst], #32]\n\t"
      "stp x10, x11, [%[dst], #48]"
      : : [src] "r" (s64), [dst] "r" (d64)
      : "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "memory"
    );
    d64 += 8; s64 += 8; n -= 64;
  }
  d = (u8*)d64;
  s = (const u8*)s64;
  if (n >= 32) {
    __asm__ __volatile__ (
      "ldp x4, x5, [%[src]]\n\t"
      "ldp x6, x7, [%[src], #16]\n\t"
      "stp x4, x5, [%[dst]]\n\t"
      "stp x6, x7, [%[dst], #16]"
      : : [src] "r" (s), [dst] "r" (d)
      : "x4", "x5", "x6", "x7", "memory"
    );
    d += 32; s += 32; n -= 32;
  }
  if (n >= 16) {
    __asm__ __volatile__ (
      "ldp x4, x5, [%[src]]\n\t"
      "stp x4, x5, [%[dst]]"
      : : [src] "r" (s), [dst] "r" (d)
      : "x4", "x5", "memory"
    );
    d += 16; s += 16; n -= 16;
  }
  while (n--) *d++ = *s++;
  return dest;
#endif
}

__attribute__((weak))
void* memmove(void* dest, const void* src, u64 n) {
#if defined(SP_AMD64)
  void* ret = dest;
  if ((uintptr_t)dest - (uintptr_t)src >= n) {
    return sp_sys_memcpy(dest, src, n);
  }
  __asm__ __volatile__ (
    "lea -1(%[dest], %[n]), %%rdi\n\t"
    "lea -1(%[src], %[n]), %%rsi\n\t"
    "mov %[n], %%rcx\n\t"
    "std\n\t"
    "rep movsb\n\t"
    "cld"
    :
    : [dest] "r" (dest), [src] "r" (src), [n] "r" (n)
    : "rdi", "rsi", "rcx", "memory"
  );
  return ret;
#elif defined(SP_ARM64)
  u8* d = (u8*)dest;
  const u8* s = (const u8*)src;
  if (d == s || n == 0) return dest;
  if ((uintptr_t)s - (uintptr_t)d - n <= -2*n) {
    return sp_sys_memcpy(dest, src, n);
  }
  if (d < s) {
    if ((uintptr_t)s % 8 == (uintptr_t)d % 8) {
      while ((uintptr_t)d % 8) { if (!n--) return dest; *d++ = *s++; }
      while (n >= 8) { __builtin_memcpy(d, s, 8); d += 8; s += 8; n -= 8; }
    }
    while (n--) *d++ = *s++;
  } else {
    if ((uintptr_t)s % 8 == (uintptr_t)d % 8) {
      while ((uintptr_t)(d + n) % 8) { if (!n--) return dest; d[n] = s[n]; }
      while (n >= 8) { n -= 8; __builtin_memcpy(d + n, s + n, 8); }
    }
    while (n--) d[n] = s[n];
  }
  return dest;
#endif
}

__attribute__((weak))
s32 memcmp(const void* va, const void* vb, u64 n) {
  const u8* a = (const u8*)va;
  const u8* b = (const u8*)vb;
  while (n >= 8) {
    u64 lv, rv;
    __builtin_memcpy(&lv, a, 8);
    __builtin_memcpy(&rv, b, 8);
    if (lv != rv) break;
    a += 8; b += 8; n -= 8;
  }
  for (; n && *a == *b; n--, a++, b++);
  return n ? *a - *b : 0;
}

__attribute__((weak))
void* memset(void* dest, s32 c, u64 n) {
  u8* d = (u8*)dest;
  u8 v = (u8)c;
#if defined(SP_AMD64)
  if (n >= 8) {
    u64 v64 = v;
    v64 |= v64 << 8;
    v64 |= v64 << 16;
    v64 |= v64 << 32;
    while ((uintptr_t)d & 7) { *d++ = v; n--; }
    u64* d64 = (u64*)d;
    while (n >= 8) { *d64++ = v64; n -= 8; }
    d = (u8*)d64;
  }
#endif
  while (n--) *d++ = v;
  return dest;
}
#elif defined(SP_WASM)
__attribute__((weak))
void* memcpy(void* dest, const void* src, size_t n) {
  return __builtin_memcpy(dest, src, n);
}

__attribute__((weak))
void* memmove(void* dest, const void* src, size_t n) {
  return __builtin_memmove(dest, src, n);
}

__attribute__((weak))
void* memset(void* dest, s32 c, size_t n) {
  return __builtin_memset(dest, c, n);
}

__attribute__((weak))
s32 memcmp(const void* va, const void* vb, size_t n) {
  const u8* a = (const u8*)va;
  const u8* b = (const u8*)vb;
  while (n >= 8) {
    u64 lv, rv;
    __builtin_memcpy(&lv, a, 8);
    __builtin_memcpy(&rv, b, 8);
    if (lv != rv) break;
    a += 8; b += 8; n -= 8;
  }
  for (; n && *a == *b; n--, a++, b++);
  return n ? *a - *b : 0;
}

#endif

#if defined(SP_MACOS) || defined(SP_COSMO) || defined(SP_LINUX) || defined(SP_WIN32)
void* sp_sys_memcpy(void* dest, const void* src, u64 n) {
  return memcpy(dest, src, n);
}

void* sp_sys_memmove(void* dest, const void* src, u64 n) {
  return memmove(dest, src, n);
}

void* sp_sys_memset(void* dest, s32 c, u64 n) {
  return memset(dest, c, n);
}

s32 sp_sys_memcmp(const void* a, const void* b, u64 n) {
  return memcmp(a, b, n);
}
#elif defined(SP_WASM)
void* sp_sys_memcpy(void* dest, const void* src, u64 n) {
  return memcpy(dest, src, (u32)n);
}

void* sp_sys_memmove(void* dest, const void* src, u64 n) {
  return memmove(dest, src, (u32)n);
}

void* sp_sys_memset(void* dest, s32 c, u64 n) {
  return memset(dest, c, (u32)n);
}

s32 sp_sys_memcmp(const void* a, const void* b, u64 n) {
  return memcmp(a, b, (u32)n);
}

#else
void* sp_sys_memcpy(void* dest, const void* src, size_t n) {
  #error "sp_sys_memcpy"
}

void* sp_sys_memmove(void* dest, const void* src, size_t n) {
  #error "sp_sys_memmove"
}

void* sp_sys_memset(void* dest, s32 c, u64 n) {
  #error "sp_sys_memset"
}

s32 sp_sys_memcmp(const void* a, const void* b, u64 n) {
  #error "sp_sys_memcmp"
}

#endif

//////////////////
// SP_SYS_ALLOC //
//////////////////
#if defined(SP_WIN32)
void* sp_sys_alloc(u64 size) {
  return VirtualAlloc(SP_NULLPTR, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void sp_sys_free(void* ptr, u64 size) {
  (void)size;
  if (ptr) VirtualFree(ptr, 0, MEM_RELEASE);
}

#elif defined(SP_LINUX)
void* sp_sys_alloc(u64 size) {
  void* p = (void*)sp_syscall(SP_SYSCALL_NUM_MMAP, 0, size, SP_PROT_READ | SP_PROT_WRITE, SP_MAP_PRIVATE | SP_MAP_ANONYMOUS, -1, 0);
  return p == SP_MAP_FAILED ? 0 : p;
}

void sp_sys_free(void* ptr, u64 size) {
  if (!ptr) return;
  sp_syscall(SP_SYSCALL_NUM_MUNMAP, ptr, size);
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
void* sp_sys_alloc(u64 size) {
  void* p = mmap(SP_NULLPTR, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return p == MAP_FAILED ? SP_NULLPTR : p;
}

void sp_sys_free(void* ptr, u64 size) {
  if (ptr) munmap(ptr, size);
}

#elif defined(SP_WASM)
void* sp_sys_alloc(u64 size) {
  const u64 page_size = 65536;
  u64 pages = (size + page_size - 1) / page_size;
  s32 prev = __builtin_wasm_memory_grow(0, (s32)pages);
  if (prev == -1) return SP_NULLPTR;
  return (void*)((u64)prev * page_size);
}

void sp_sys_free(void* ptr, u64 size) {
  sp_unused(ptr); sp_unused(size);
}

#else
#error "sp_sys_alloc"
#error "sp_sys_free"
#endif

///////////////////
// SP_SYS_ASSERT //
///////////////////
void sp_sys_assert(const c8* file, const c8* line, const c8* func, const c8* expr, bool cond) {
  if (cond) return;

  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_fd(&io, sp_sys_stderr, SP_IO_CLOSE_MODE_NONE);
  sp_fmt_io(
    &io,
    "{.red} {}:{.gray}:{.yellow}{.yellow} {}",
    sp_fmt_cstr("assert"),
    sp_fmt_cstr(file),
    sp_fmt_cstr(line),
    sp_fmt_cstr(func),
    sp_fmt_cstr("()"),
    sp_fmt_cstr(expr)
  );
  sp_io_write_cstr(&io, "\n", SP_NULLPTR);

  #if defined(SP_CLANG)
    __builtin_debugtrap();
  #elif defined(SP_GNUC)
    __builtin_trap();
  #elif defined(SP_MSVC)
    __debugbreak();
  #else
    #if defined(SP_AMD64)
      __asm__ volatile ("int3");
    #elif defined(SP_ARM64)
      __asm__ volatile ("brk #0");
    #endif
  #endif
}

///////////////
// SP_SYS_TP //
///////////////
#if defined(SP_FREESTANDING)
void* sp_sys_get_tp(void) {
  void* tp =  SP_NULLPTR;
#if defined(SP_AMD64)
  __asm__ __volatile__ ("mov %%fs:0, %0" : "=r"(tp));
#elif defined(SP_ARM64)
  __asm__ __volatile__ ("mrs %0, tpidr_el0" : "=r"(tp));
#endif
  return tp;
}

s32 sp_sys_set_tp(void* tp) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_ARCH_PRCTL, SP_ARCH_SET_FS, tp);
#elif defined(SP_ARM64)
  __asm__ __volatile__ ("msr tpidr_el0, %0" : : "r"(tp) : "memory");
  return 0;
#endif
}

#elif defined(SP_WASM)
void* sp_sys_get_tp() {
  return SP_NULLPTR;
}

s32 sp_sys_set_tp(void* tp) {
  sp_unused(tp);
  return 0;
}
#endif

/////////////////
// SP_SYS_EXIT //
/////////////////
#if defined(SP_FREESTANDING)
void sp_sys_exit(s32 code) {
  sp_syscall(SP_SYSCALL_NUM_EXIT_GROUP, code);
  __builtin_unreachable();
}
#elif defined(SP_WASM)
void sp_sys_exit(s32 code) {
  __wasi_proc_exit((u32)code);
}
#endif

/////////////////////
// SP_SYS_TLS_INIT //
/////////////////////
#if defined(SP_FREESTANDING) || defined(SP_WASM_FREESTANDING)
void sp_sys_tls_init(sp_tls_rt_t* tls) {
  sp_env_init(&tls->env, tls->contexts[0].allocator);
  if (!environ) return;
  for (c8** p = environ; *p; p++) {
    sp_str_pair_t pair = sp_str_cleave_c8(sp_str_view(*p), '=');
    sp_str_ht_insert(tls->env.vars, pair.first, pair.second);
  }
}
#else
void sp_sys_tls_init(sp_tls_rt_t* tls) {
  sp_unused(tls);
}
#endif

/////////////////
// SP_SYS_INIT //
/////////////////
#if defined(SP_FREESTANDING) || defined(SP_WASM_FREESTANDING)
void sp_sys_init() {
  sp_tls_block.self = &sp_tls_block;
  sp_tls_block.data = SP_NULLPTR;
  sp_sys_set_tp(&sp_tls_block);
  sp_tls_rt_get();
}
#else
void sp_sys_init() {

}
#endif

/////////////////
// SP_SYS_STAT //
/////////////////
#if defined(SP_WIN32)
s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  return sp_sys_stat_from_nt_path(path, st, true);
}
#elif defined(SP_LINUX)
s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  sp_sys_linux_stat_t raw = SP_ZERO_INITIALIZE();
  s32 rc;
#if defined(SP_AMD64)
  rc = (s32)sp_syscall(SP_SYSCALL_NUM_STAT, path, &raw);
#else
  rc = (s32)sp_syscall(SP_SYSCALL_NUM_NEWFSTATAT, SP_AT_FDCWD, path, &raw, 0);
#endif
  if (rc == 0) sp_sys_stat_from_linux(&raw, st);
  return rc;
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = stat(path, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}
#elif defined(SP_WASM)
s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  (void)path; (void)st;
  return -1;
}
#else
#error "sp_sys_stat"
#endif

//////////////////
// SP_SYS_LSTAT //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  return sp_sys_stat_from_nt_path(path, st, false);
}
#elif defined(SP_LINUX)
s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  sp_sys_linux_stat_t raw = SP_ZERO_INITIALIZE();
  s32 rc;
#if defined(SP_AMD64)
  rc = (s32)sp_syscall(SP_SYSCALL_NUM_LSTAT, path, &raw);
#else
  rc = (s32)sp_syscall(SP_SYSCALL_NUM_NEWFSTATAT, SP_AT_FDCWD, path, &raw, SP_AT_SYMLINK_NOFOLLOW);
#endif
  if (rc == 0) sp_sys_stat_from_linux(&raw, st);
  return rc;
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = lstat(path, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}
#elif defined(SP_WASM)
s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  (void)path; (void)st;
  return -1;
}
#else
#error "sp_sys_lstat"
#endif

//////////////////
// SP_SYS_MKDIR //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_mkdir(const c8* path, s32 mode) {
  (void)mode;
  void* handle = sp_sys_nt_open(
    sp_str_view(path),
    FILE_LIST_DIRECTORY | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_CREATE,
    SP_NT_FILE_DIRECTORY_FILE | SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_FOR_BACKUP_INTENT,
    FILE_ATTRIBUTE_DIRECTORY
  );
  if (!handle) return -1;
  SP_NT(NtClose)(handle);
  return 0;
}
#elif defined(SP_LINUX)
s32 sp_sys_mkdir(const c8* path, s32 mode) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_MKDIR, path, mode);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_MKDIRAT, SP_AT_FDCWD, path, mode);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_mkdir(const c8* path, s32 mode) {
  return mkdir(path, (mode_t)mode);
}
#elif defined(SP_WASM)
s32 sp_sys_mkdir(const c8* path, s32 mode) {
  (void)path; (void)mode;
  return -1;
}
#else
#error "sp_sys_mkdir"
#endif

//////////////////
// SP_SYS_RMDIR //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_rmdir(const c8* path) {
  void* handle = sp_sys_nt_open(
    sp_str_view(path),
    DELETE | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_DIRECTORY_FILE | SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_DELETE_ON_CLOSE | SP_NT_FILE_OPEN_REPARSE_POINT,
    0
  );
  if (!handle) return -1;
  SP_NT(NtClose)(handle);
  return 0;
}
#elif defined(SP_LINUX)
s32 sp_sys_rmdir(const c8* path) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_RMDIR, path);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINKAT, SP_AT_FDCWD, path, SP_AT_REMOVEDIR);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_rmdir(const c8* path) {
  return rmdir(path);
}
#elif defined(SP_WASM)
s32 sp_sys_rmdir(const c8* path) {
  (void)path;
  return -1;
}
#else
#error "sp_sys_rmdir"
#endif

///////////////////
// SP_SYS_UNLINK //
///////////////////
#if defined(SP_WIN32)
s32 sp_sys_unlink(const c8* path) {
  void* handle = sp_sys_nt_open(
    sp_str_view(path),
    DELETE | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_DELETE_ON_CLOSE | SP_NT_FILE_NON_DIRECTORY_FILE | SP_NT_FILE_OPEN_REPARSE_POINT,
    0
  );
  if (!handle) return -1;
  SP_NT(NtClose)(handle);
  return 0;
}
#elif defined(SP_LINUX)
s32 sp_sys_unlink(const c8* path) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINK, path);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINKAT, SP_AT_FDCWD, path, 0);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_unlink(const c8* path) {
  return unlink(path);
}
#elif defined(SP_WASM)
s32 sp_sys_unlink(const c8* path) {
  (void)path;
  return -1;
}
#else
#error "sp_sys_unlink"
#endif

//////////////////
// SP_SYS_CHDIR //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_chdir(const c8* path) {
  sp_wide_str_t w = sp_wtf8_to_wtf16(sp_str_view(path));
  if (!w.data) return -1;
  sp_nt_unicode_string_t us = {
    .Length = (u16)(w.len * sizeof(u16)),
    .MaximumLength = (u16)((w.len + 1) * sizeof(u16)),
    .Buffer = w.data,
  };
  return SP_NT_SUCCESS(SP_NT(RtlSetCurrentDirectory_U)(&us)) ? 0 : -1;
}
#elif defined(SP_LINUX)
s32 sp_sys_chdir(const c8* path) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CHDIR, path);
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_chdir(const c8* path) {
  return chdir(path);
}
#elif defined(SP_WASM)
s32 sp_sys_chdir(const c8* path) {
  (void)path;
  return -1;
}
#else
#error "sp_sys_chdir"
#endif

///////////////////
// SP_SYS_GETCWD //
///////////////////
#if defined(SP_WIN32)
s64 sp_sys_getcwd(char* buf, u64 size) {
  sp_nt_unicode_string_t* cwd = (sp_nt_unicode_string_t*)(sp_nt_process_params() + 0x38);
  u32 wlen = cwd->Length / (u32)sizeof(u16);
  if (wlen && cwd->Buffer[wlen - 1] == '\\') wlen--;
  sp_str_t utf8 = sp_wtf16_to_wtf8((sp_wide_str_t) { .data = cwd->Buffer, .len = wlen });
  if (utf8.len >= size) return -1;
  sp_mem_copy(utf8.data, buf, utf8.len);
  buf[utf8.len] = 0;
  return (s64)utf8.len;
}
#elif defined(SP_LINUX)
s64 sp_sys_getcwd(char* buf, u64 size) {
  return sp_syscall(SP_SYSCALL_NUM_GETCWD, buf, size);
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_getcwd(char* buf, u64 size) {
  return getcwd(buf, size) ? 0 : -1;
}
#elif defined(SP_WASM)
s64 sp_sys_getcwd(char* buf, u64 size) {
  if (!buf || size < 2) return -1;
  buf[0] = '/';
  buf[1] = '\0';
  return 1;
}
#else
#error "sp_sys_getcwd"
#endif

/////////////////
// SP_SYS_LINK //
/////////////////
#if defined(SP_WIN32)
s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
  void* handle = sp_sys_nt_open(
    sp_str_view(oldpath),
    FILE_READ_ATTRIBUTES | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_REPARSE_POINT,
    0
  );
  if (!handle) return -1;

  sp_sys_nt_path_t nt = SP_ZERO_INITIALIZE();
  if (!SP_NT_SUCCESS(sp_sys_nt_path(sp_str_view(newpath), &nt))) {
    SP_NT(NtClose)(handle);
    return -1;
  }

  u32 name_bytes = nt.name.Length;
  u32 info_bytes = sizeof(sp_nt_file_link_information_t) + name_bytes - sizeof(u16);
  sp_nt_file_link_information_t* info = (sp_nt_file_link_information_t*)sp_alloc(info_bytes);
  *info = SP_ZERO_STRUCT(sp_nt_file_link_information_t);
  info->ReplaceIfExists = 0;
  info->FileNameLength = name_bytes;
  sp_mem_copy(nt.name.Buffer, info->FileName, name_bytes);

  sp_nt_io_status_block_t iosb = SP_ZERO_INITIALIZE();
  sp_nt_status_t status = SP_NT(NtSetInformationFile)(handle, &iosb, info, info_bytes, SP_NT_FILE_LINK_INFORMATION);

  sp_sys_nt_path_free(&nt);
  SP_NT(NtClose)(handle);

  return SP_NT_SUCCESS(status) ? 0 : -1;
}
#elif defined(SP_LINUX)
s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_LINK, oldpath, newpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_LINKAT, SP_AT_FDCWD, oldpath, SP_AT_FDCWD, newpath, 0);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
  return link(oldpath, newpath);
}
#elif defined(SP_WASM)
s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
  (void)oldpath; (void)newpath;
  return -1;
}
#else
#error "sp_sys_link"
#endif

////////////////////
// SP_SYS_SYMLINK //
////////////////////
#if defined(SP_WIN32)
s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
  sp_wide_str_t wtarget = sp_wtf8_to_wtf16(sp_str_view(target));
  sp_wide_str_t wlink = sp_wtf8_to_wtf16(sp_str_view(linkpath));
  if (!wtarget.data || !wlink.data) return -1;

  DWORD flags = 0;
  DWORD target_attrs = GetFileAttributesW((LPCWSTR)wtarget.data);
  if (target_attrs != INVALID_FILE_ATTRIBUTES && (target_attrs & FILE_ATTRIBUTE_DIRECTORY)) {
    flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
  }
  #ifdef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
    flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
  #endif
  return CreateSymbolicLinkW((LPCWSTR)wlink.data, (LPCWSTR)wtarget.data, flags) ? 0 : -1;
}
#elif defined(SP_LINUX)
s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_SYMLINK, target, linkpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_SYMLINKAT, target, SP_AT_FDCWD, linkpath);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
  return symlink(target, linkpath);
}
#elif defined(SP_WASM)
s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
  (void)target; (void)linkpath;
  return -1;
}
#else
#error "sp_sys_symlink"
#endif

//////////////////
// SP_SYS_CHMOD //
//////////////////
#if defined(SP_WIN32)
s32 sp_sys_chmod(const c8* path, const sp_sys_stat_t* st) {
  void* handle = sp_sys_nt_open(
    sp_str_view(path),
    FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_REPARSE_POINT,
    0
  );
  if (!handle) return -1;

  sp_nt_file_basic_information_t info = SP_ZERO_INITIALIZE();
  info.FileAttributes = st->raw_attrs ? st->raw_attrs : FILE_ATTRIBUTE_NORMAL;

  sp_nt_io_status_block_t iosb = SP_ZERO_INITIALIZE();
  sp_nt_status_t status = SP_NT(NtSetInformationFile)(handle, &iosb, &info, sizeof(info), SP_NT_FILE_BASIC_INFORMATION);

  SP_NT(NtClose)(handle);
  return SP_NT_SUCCESS(status) ? 0 : -1;
}
#elif defined(SP_LINUX)
s32 sp_sys_chmod(const c8* path, const sp_sys_stat_t* st) {
  s32 mode = (s32)(st->raw_attrs & 07777);
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_CHMOD, path, mode);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_FCHMODAT, SP_AT_FDCWD, path, mode, 0);
#endif
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s32 sp_sys_chmod(const c8* path, const sp_sys_stat_t* st) {
  return chmod(path, (mode_t)(st->raw_attrs & 07777));
}
#elif defined(SP_WASM)
s32 sp_sys_chmod(const c8* path, const sp_sys_stat_t* st) {
  (void)path; (void)st;
  return -1;
}
#else
#error "sp_sys_chmod"
#endif

///////////////////////////////
// SP_SYS_CANONICALIZE_PATH //
///////////////////////////////
#if defined(SP_WIN32)
s64 sp_sys_canonicalize_path(const c8* path, c8* buf, u64 size) {
  if (!buf) return -1;

  void* h = sp_sys_nt_open(
    sp_str_view(path),
    FILE_READ_ATTRIBUTES | SYNCHRONIZE,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_SYNCHRONOUS_IO_NONALERT | SP_NT_FILE_OPEN_FOR_BACKUP_INTENT,
    0
  );
  if (!h) return -1;

  u16 wbuf[SP_PATH_MAX];
  DWORD wlen = GetFinalPathNameByHandleW((HANDLE)h, (LPWSTR)wbuf, SP_PATH_MAX, 0);
  SP_NT(NtClose)(h);
  if (wlen == 0 || wlen >= SP_PATH_MAX) return -1;

  u32 skip = 0;
  if (wlen >= 4 && wbuf[0] == '\\' && wbuf[1] == '\\' && wbuf[2] == '?' && wbuf[3] == '\\') skip = 4;
  sp_str_t utf8 = sp_wtf16_to_wtf8((sp_wide_str_t) { .data = wbuf + skip, .len = wlen - skip });
  if (utf8.len >= size) return -1;
  sp_mem_copy(utf8.data, buf, utf8.len);
  buf[utf8.len] = 0;
  return (s64)utf8.len;
}
#elif defined(SP_LINUX)
s64 sp_sys_canonicalize_path(const c8* path, c8* buf, u64 size) {
  s64 fd = sp_syscall(SP_SYSCALL_NUM_OPENAT, SP_AT_FDCWD, path, SP_O_RDONLY | SP_O_CLOEXEC, 0);
  if (fd < 0) return fd;

  c8 proc [64] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, proc, 64);
  sp_fmt_io(&io, "/proc/self/fd/{}", sp_fmt_int(fd));

  s64 len = sp_syscall(SP_SYSCALL_NUM_READLINKAT, SP_AT_FDCWD, proc, buf, size);
  sp_sys_close(fd);
  return len;
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_canonicalize_path(const c8* path, c8* buf, u64 size) {
  if (!path || !buf || size == 0) return -1;
  c8 resolved[4096];
  if (!realpath(path, resolved)) return -1;
  u64 len = 0;
  while (resolved[len] && len < size - 1) { buf[len] = resolved[len]; len++; }
  buf[len] = '\0';
  return (s64)len;
}
#elif defined(SP_WASM)
s64 sp_sys_canonicalize_path(const c8* path, c8* buf, u64 size) {
  if (!path || !buf || size == 0) return -1;
  u64 len = 0;
  while (path[len] && len < size - 1) { buf[len] = path[len]; len++; }
  buf[len] = '\0';
  return (s64)len;
}
#else
#error "sp_sys_canonicalize_path"
#endif

/////////////////////////
// SP_SYS_GET_EXE_PATH //
/////////////////////////
#if defined(SP_WIN32)
s64 sp_sys_get_exe_path(c8* buf, u64 size) {
  if (!buf) return -1;

  sp_nt_unicode_string_t* image = (sp_nt_unicode_string_t*)(sp_nt_process_params() + 0x60);
  u32 wlen = image->Length / (u32)sizeof(u16);
  sp_str_t utf8 = sp_wtf16_to_wtf8((sp_wide_str_t) { .data = image->Buffer, .len = wlen });
  if (utf8.len >= size) return -1;
  sp_mem_copy(utf8.data, buf, utf8.len);
  buf[utf8.len] = 0;
  return (s64)utf8.len;
}
#elif defined(SP_LINUX)
s64 sp_sys_get_exe_path(c8* buf, u64 size) {
  return sp_syscall(SP_SYSCALL_NUM_READLINKAT, SP_AT_FDCWD, "/proc/self/exe", buf, size);
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_get_exe_path(c8* buf, u64 size) {
  if (!buf || size == 0) return -1;
#if defined(SP_MACOS)
  c8 raw[4096];
  u32 raw_size = sizeof(raw);
  if (_NSGetExecutablePath(raw, &raw_size)) return -1;
  return sp_sys_canonicalize_path(raw, buf, size);
#elif defined(SP_COSMO)
  extern char* program_invocation_name;
  return sp_sys_canonicalize_path(program_invocation_name, buf, size);
#else
  (void)buf; (void)size;
  return -1;
#endif
}
#elif defined(SP_WASM)
s64 sp_sys_get_exe_path(c8* buf, u64 size) {
  (void)buf; (void)size;
  return -1;
}
#else
#error "sp_sys_get_exe_path"
#endif

/////////////////////
// SP_OS_GET_NAME //
/////////////////////
#if defined(SP_WIN32)
sp_str_t sp_os_get_name() {
  return sp_str_lit("windows");
}
#elif defined(SP_LINUX)
sp_str_t sp_os_get_name() {
  return sp_str_lit("linux");
}
#elif defined(SP_MACOS)
sp_str_t sp_os_get_name() {
  return sp_str_lit("macos");
}
#elif defined(SP_COSMO)
sp_str_t sp_os_get_name() {
  switch (sp_os_get_kind()) {
    case SP_OS_LINUX: return sp_str_lit("linux");
    case SP_OS_WIN32: return sp_str_lit("windows");
    case SP_OS_MACOS: return sp_str_lit("macos");
  }
  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}
#elif defined(SP_WASM)
sp_str_t sp_os_get_name() {
  return sp_str_lit("wasm");
}
#else
#error "sp_os_get_name"
#endif

/////////////
// SP_MAIN //
/////////////
#if defined(SP_FREESTANDING) || defined(SP_WASM_FREESTANDING)
void sp_main(s32 argc, const c8** argv, sp_entry_fn_t fn) {
  environ = (c8**)(argv + argc + 1);
  sp_sys_init();
  sp_sys_exit(fn(argc, argv));
}
#endif


//  █████   █████   █████████    █████████  █████   █████
// ░░███   ░░███   ███░░░░░███  ███░░░░░███░░███   ░░███
//  ░███    ░███  ░███    ░███ ░███    ░░░  ░███    ░███
//  ░███████████  ░███████████ ░░█████████  ░███████████
//  ░███░░░░░███  ░███░░░░░███  ░░░░░░░░███ ░███░░░░░███
//  ░███    ░███  ░███    ░███  ███    ░███ ░███    ░███
//  █████   █████ █████   █████░░█████████  █████   █████
// ░░░░░   ░░░░░ ░░░░░   ░░░░░  ░░░░░░░░░  ░░░░░   ░░░░░
// @hash
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
  u8* d = (u8*) p;
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
    data = d[0] | ((size_t) d[1] << 8) | ((size_t) d[2] << 16) | ((size_t) d[3] << 24);
    data |= ((size_t) d[4] | ((size_t) d[5] << 8) | ((size_t) d[6] << 16) | ((size_t) d[7] << 24)) << 16 << 16;

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
    case 4: data |= ((size_t) d[3] << 24); SP_FALLTHROUGH();
    case 3: data |= ((size_t) d[2] << 16); SP_FALLTHROUGH();
    case 2: data |= ((size_t) d[1] << 8); SP_FALLTHROUGH();
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


//  █████   █████   █████████    █████████  █████   █████    ███████████   █████████   ███████████  █████       ██████████
// ░░███   ░░███   ███░░░░░███  ███░░░░░███░░███   ░░███    ░█░░░███░░░█  ███░░░░░███ ░░███░░░░░███░░███       ░░███░░░░░█
//  ░███    ░███  ░███    ░███ ░███    ░░░  ░███    ░███    ░   ░███  ░  ░███    ░███  ░███    ░███ ░███        ░███  █ ░
//  ░███████████  ░███████████ ░░█████████  ░███████████        ░███     ░███████████  ░██████████  ░███        ░██████
//  ░███░░░░░███  ░███░░░░░███  ░░░░░░░░███ ░███░░░░░███        ░███     ░███░░░░░███  ░███░░░░░███ ░███        ░███░░█
//  ░███    ░███  ░███    ░███  ███    ░███ ░███    ░███        ░███     ░███    ░███  ░███    ░███ ░███      █ ░███ ░   █
//  █████   █████ █████   █████░░█████████  █████   █████       █████    █████   █████ ███████████  ███████████ ██████████
// ░░░░░   ░░░░░ ░░░░░   ░░░░░  ░░░░░░░░░  ░░░░░   ░░░░░       ░░░░░    ░░░░░   ░░░░░ ░░░░░░░░░░░  ░░░░░░░░░░░ ░░░░░░░░░░
// @hash_table @ht
bool sp_ht_on_compare_key(void* ka, void* kb, u64 size) {
  return sp_mem_is_equal(ka, kb, size);
}

sp_hash_t sp_ht_on_hash_key(void *key, u64 size) {
  return sp_hash_bytes(key, size, SP_HT_HASH_SEED);
}

sp_hash_t sp_ht_on_hash_str_key(void* key, u64 size) {
  (void)size;
  sp_str_t* str = (sp_str_t*)key;
  return sp_hash_str(*str);
}

bool sp_ht_on_compare_str_key(void* ka, void* kb, u64 size) {
  (void)size;
  sp_str_t* sa = (sp_str_t*)ka;
  sp_str_t* sb = (sp_str_t*)kb;
  return sp_str_equal(*sa, *sb);
}

sp_hash_t sp_ht_on_hash_cstr_key(void* key, u64 size) {
  (void)size;
  const c8** str = (const c8**)key;
  return sp_hash_cstr(*str);
}

bool sp_ht_on_compare_cstr_key(void* ka, void* kb, u64 size) {
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

  void* old_data = *data;
  void* new_data = sp_alloc_a(info.allocator, new_cap * info.stride.entry);

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

  sp_free_a(info.allocator, old_data);
  *data = new_data;
}

void sp_ht_insert_impl(void* ht, void* key, void* val, sp_ht_info_t info) {
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

  sp_hash_t hash = info.fn.hash(key, info.size.key);
  u64 hash_idx = hash % cap;
  u64 first_free = SP_HT_INVALID_INDEX;

  for (u64 c = 0; c < cap; ++c) {
    u64 i = (hash_idx + c) % cap;
    u64 offset = i * info.stride.entry;
    sp_ht_entry_state state = *(sp_ht_entry_state*)((u8*)(*data) + offset + info.stride.kv);

    if (state == SP_HT_ENTRY_INACTIVE) {
      if (first_free == SP_HT_INVALID_INDEX) first_free = i;
      break;
    }
    if (state == SP_HT_ENTRY_DELETED) {
      if (first_free == SP_HT_INVALID_INDEX) first_free = i;
      continue;
    }
    void* k = (u8*)(*data) + offset;
    if (info.fn.compare(k, key, info.size.key)) {
      u8* entry = (u8*)(*data) + offset;
      sp_mem_copy(val, entry + info.stride.value, info.size.value);
      return;
    }
  }

  u64 idx = first_free != SP_HT_INVALID_INDEX ? first_free : hash_idx;
  u8* entry = (u8*)(*data) + idx * info.stride.entry;
  sp_mem_copy(key, entry, info.size.key);
  sp_mem_copy(val, entry + info.stride.value, info.size.value);
  *(sp_ht_entry_state*)(entry + info.stride.kv) = SP_HT_ENTRY_ACTIVE;
  (*size)++;
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


//  ██████████   █████ █████ ██████   █████      █████████   ███████████   ███████████     █████████   █████ █████
// ░░███░░░░███ ░░███ ░░███ ░░██████ ░░███      ███░░░░░███ ░░███░░░░░███ ░░███░░░░░███   ███░░░░░███ ░░███ ░░███
//  ░███   ░░███ ░░███ ███   ░███░███ ░███     ░███    ░███  ░███    ░███  ░███    ░███  ░███    ░███  ░░███ ███
//  ░███    ░███  ░░█████    ░███░░███░███     ░███████████  ░██████████   ░██████████   ░███████████   ░░█████
//  ░███    ░███   ░░███     ░███ ░░██████     ░███░░░░░███  ░███░░░░░███  ░███░░░░░███  ░███░░░░░███    ░░███
//  ░███    ███     ░███     ░███  ░░█████     ░███    ░███  ░███    ░███  ░███    ░███  ░███    ░███     ░███
//  ██████████      █████    █████  ░░█████    █████   █████ █████   █████ █████   █████ █████   █████    █████
// ░░░░░░░░░░      ░░░░░    ░░░░░    ░░░░░    ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░   ░░░░░    ░░░░░
// @array @dyn_array @da
void* sp_da_resize(void* arr, u32 stride, u64 len) {
  len = sp_max(len, 4);
  sp_da_header_t* header = arr ? sp_da_head(arr) : SP_NULLPTR;
  sp_mem_t a = header ? header->allocator : sp_context_get_allocator();
  header = sp_mem_allocator_realloc(a, header, len * stride + sizeof(sp_da_header_t));

  if (!header) return SP_NULLPTR;

  if (!arr) {
    header->size = 0;
    header->allocator = a;
  }
  header->capacity = len;
  return header + 1;
}
void* sp_da_grow_ex(void* arr, u32 stride, u64 addlen) {
  u64 required = sp_da_size(arr) + addlen;
  if (required <= sp_da_capacity(arr)) return arr;

  u64 cap = sp_da_capacity(arr) * 2;
  cap = sp_max(cap, required);
  return sp_da_resize(arr, stride, cap);
}

void sp_da_push_ex(void** arr, void* val, u32 stride) {
  *arr = sp_da_grow_ex(*arr, stride, 1);
  if (*arr) {
    sp_mem_copy(val, ((u8*)(*arr)) + sp_da_size(*arr) * stride, stride);
    sp_da_head(*arr)->size++;
  }
}

void sp_da_init_ex(sp_mem_t a, void** arr, u32 stride) {
  // Match sp_da_resize's lazy-init: copy the allocator into a heap slot so the
  // array does not depend on the caller's `a` outliving the function frame.
  u32 cap = 4;
  sp_da_header_t* head = sp_mem_allocator_alloc(a, cap * stride + sizeof(sp_da_header_t));
  *head = (sp_da_header_t) {
    .allocator = a,
    .capacity = cap,
    .size = 0,
  };

  *arr = head + 1;
}



//  ███████████   █████ ██████   █████   █████████     ███████████  █████  █████ ███████████ ███████████ ██████████ ███████████
// ░░███░░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███   ░░███░░░░░███░░███  ░░███ ░░███░░░░░░█░░███░░░░░░█░░███░░░░░█░░███░░░░░███
//  ░███    ░███  ░███  ░███░███ ░███  ███     ░░░     ░███    ░███ ░███   ░███  ░███   █ ░  ░███   █ ░  ░███  █ ░  ░███    ░███
//  ░██████████   ░███  ░███░░███░███ ░███             ░██████████  ░███   ░███  ░███████    ░███████    ░██████    ░██████████
//  ░███░░░░░███  ░███  ░███ ░░██████ ░███    █████    ░███░░░░░███ ░███   ░███  ░███░░░█    ░███░░░█    ░███░░█    ░███░░░░░███
//  ░███    ░███  ░███  ░███  ░░█████ ░░███  ░░███     ░███    ░███ ░███   ░███  ░███  ░     ░███  ░     ░███ ░   █ ░███    ░███
//  █████   █████ █████ █████  ░░█████ ░░█████████     ███████████  ░░████████   █████       █████       ██████████ █████   █████
// ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░     ░░░░░░░░░░░    ░░░░░░░░   ░░░░░       ░░░░░       ░░░░░░░░░░ ░░░░░   ░░░░░
// @ring_buffer @rb
void* sp_rb_grow_impl(void* arr, u32 stride, u32 capacity) {
  sp_ring_buffer_t* rb = (sp_ring_buffer_t*)sp_alloc(capacity * stride + sizeof(sp_ring_buffer_t));
  if (!rb) return SP_NULLPTR;

  rb->head = 0;
  rb->size = 0;
  rb->capacity = capacity;
  rb->mode = SP_RQ_MODE_GROW;

  if (arr) {
    sp_ring_buffer_t* old = sp_rb_head(arr);
    u32 old_size = old->size;
    u32 old_cap = old->capacity;
    u32 old_head = old->head;
    rb->size = old_size;
    rb->mode = old->mode;

    u8* new_arr = (u8*)rb + sizeof(sp_ring_buffer_t);
    u8* old_arr = (u8*)arr;

    u32 first_chunk = old_cap - old_head;
    if (first_chunk > old_size) first_chunk = old_size;
    sp_mem_copy(old_arr + old_head * stride, new_arr, first_chunk * stride);

    u32 second_chunk = old_size - first_chunk;
    if (second_chunk > 0) {
      sp_mem_copy(old_arr, new_arr + first_chunk * stride, second_chunk * stride);
    }

    sp_free(old);
  }

  return (u8*)(rb + 1);
}


//  ███████████    ███████    ███████████   ██████   ██████   █████████   ███████████
// ░░███░░░░░░█  ███░░░░░███ ░░███░░░░░███ ░░██████ ██████   ███░░░░░███ ░█░░░███░░░█
//  ░███   █ ░  ███     ░░███ ░███    ░███  ░███░█████░███  ░███    ░███ ░   ░███  ░
//  ░███████   ░███      ░███ ░██████████   ░███░░███ ░███  ░███████████     ░███
//  ░███░░░█   ░███      ░███ ░███░░░░░███  ░███ ░░░  ░███  ░███░░░░░███     ░███
//  ░███  ░    ░░███     ███  ░███    ░███  ░███      ░███  ░███    ░███     ░███
//  █████       ░░░███████░   █████   █████ █████     █████ █████   █████    █████
// ░░░░░          ░░░░░░░    ░░░░░   ░░░░░ ░░░░░     ░░░░░ ░░░░░   ░░░░░    ░░░░░
// @format
void sp_fmt_directive_register(const c8* name, sp_fmt_directive_t directive) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_str_t id = sp_str_from_cstr(name);
  sp_str_ht_insert(tls->format.directives, id, directive);
}

sp_fmt_directive_t* sp_fmt_directive_lookup(sp_str_t name) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  u64 index = 0;
  return sp_str_ht_get_ex(tls->format.directives, name, index);
}

void sp_fmt_directive_reset() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_str_ht_free(tls->format.directives);
  sp_str_ht_init_a(tls->contexts[0].allocator, tls->format.directives);
  sp_fmt_register_builtins();
}

static u8 sp_fmt_peek(sp_fmt_parser_t* p, u32 offset) {
  u32 idx = p->i + offset;
  if (idx >= p->str.len) return 0;
  return (u8)p->str.data[idx];
}

typedef struct {
  u8 first;
  u8 second;
} sp_fmt_peek_t;

static sp_fmt_peek_t sp_fmt_peek2(sp_fmt_parser_t* p) {
  return (sp_fmt_peek_t){
    .first  = sp_fmt_peek(p, 0),
    .second = sp_fmt_peek(p, 1),
  };
}

static u8 sp_fmt_advance(sp_fmt_parser_t* p) {
  if (p->i >= p->str.len) return 0;
  return (u8)p->str.data[p->i++];
}

static bool sp_fmt_match(sp_fmt_parser_t* p, c8 c) {
  if (sp_fmt_peek(p, 0) != (u8)c) return false;
  p->i++;
  return true;
}

static bool sp_fmt_is_whitespace(u8 c) {
  return c == ' ' || c == '\t';
}

static bool sp_fmt_is_digit(u8 c) {
  return c >= '0' && c <= '9';
}

static bool sp_fmt_is_alpha(u8 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool sp_fmt_is_directive_id(u8 c) {
  return sp_fmt_is_alpha(c) || (c >= '0' && c <= '9') || c == '-' || c == '_';
}

static bool sp_fmt_is_align(u8 c) {
  return c == '<' || c == '^' || c == '>';
}

static sp_str_t sp_fmt_sub(sp_fmt_parser_t* p) {
  return (sp_str_t) {
    .data = p->str.data + p->i,
  };
}

static void sp_fmt_eat_whitespace(sp_fmt_parser_t* p) {
  while (sp_fmt_is_whitespace(sp_fmt_peek(p, 0))) {
    sp_fmt_advance(p);
  }
}

static sp_str_t sp_fmt_directive_name(sp_fmt_parser_t* p) {
  sp_str_t word = sp_fmt_sub(p);
  if (!sp_fmt_is_alpha(sp_fmt_peek(p, 0))) return word;
  while (sp_fmt_is_directive_id(sp_fmt_peek(p, 0))) {
    sp_fmt_advance(p);
    word.len++;
  }
  return word;
}

static sp_str_t sp_fmt_directive_arg(sp_fmt_parser_t* p) {
  sp_str_t arg = sp_fmt_sub(p);
  while (true) {
    c8 c = sp_fmt_peek(p, 0);
    if (!c || sp_fmt_is_whitespace(c) || c == '}' || c == '.' || c == '$') break;
    sp_fmt_advance(p);
    arg.len++;
  }
  return arg;
}

static bool sp_fmt_at_directive_boundary(sp_fmt_parser_t* p) {
  c8 c = sp_fmt_peek(p, 0);
  return c == 0 || c == '}' || sp_fmt_is_whitespace(c);
}

static sp_fmt_align_t sp_fmt_align_from_char(u8 c) {
  if (c == '<') return SP_FMT_ALIGN_LEFT;
  if (c == '^') return SP_FMT_ALIGN_CENTER;
  if (c == '>') return SP_FMT_ALIGN_RIGHT;
  return SP_FMT_ALIGN_NONE;
}

static c8 sp_fmt_align_to_char(sp_fmt_align_t align) {
  switch (align) {
    case SP_FMT_ALIGN_LEFT: return '<';
    case SP_FMT_ALIGN_CENTER: return '^';
    case SP_FMT_ALIGN_RIGHT: return '>';
    case SP_FMT_ALIGN_NONE: return 0;
  }
  return 0;
}

static sp_err_t sp_fmt_parse_number(sp_fmt_parser_t* p, u32* out) {
  u32 acc = 0;
  sp_err_t err = SP_ERR;
  while (sp_fmt_is_digit(sp_fmt_peek(p, 0))) {
    acc = acc * 10 + (u32)(sp_fmt_advance(p) - '0');
    err = SP_OK;
  }
  if (!err) *out = acc;
  return err;
}

// Parses the spec body that follows the `:` introducer:
// [fill_align] [width] [.precision]
static sp_err_t sp_fmt_parse_spec_body(sp_fmt_parser_t* p, sp_fmt_spec_t* spec) {
  sp_fmt_peek_t peek = sp_fmt_peek2(p);
  if (sp_fmt_is_align(peek.second)) {
    if (peek.first == '$') spec->fill_dynamic = 1;
    else                   spec->fill = peek.first;
    spec->align = sp_fmt_align_from_char(peek.second);
    sp_fmt_advance(p);
    sp_fmt_advance(p);
  }
  else if (sp_fmt_is_align(peek.first)) {
    spec->align = sp_fmt_align_from_char(peek.first);
    sp_fmt_advance(p);
  }

  c8 c = sp_fmt_peek(p, 0);
  if (c == '$') {
    sp_fmt_advance(p);
    spec->width_dynamic = 1;
  }
  else if (sp_fmt_is_digit(c)) {
    sp_fmt_parse_number(p, &spec->width);
  }

  if (sp_fmt_peek(p, 0) == '.') {
    sp_fmt_advance(p);
    if (sp_fmt_peek(p, 0) == '$') {
      sp_fmt_advance(p);
      spec->precision_dynamic = 1;
    }
    else {
      u32 prec = 0;
      sp_try_as(sp_fmt_parse_number(p, &prec), SP_ERR_FMT_BAD_PRECISION);
      sp_opt_set(spec->precision, (u8)prec);
    }
  }
  return SP_OK;
}

// Parses a single directive that follows the `.` introducer:
// name [whitespace (dynamic | literal_arg)]
// On success, leaves the cursor on a directive boundary (whitespace, `}`, or
// EOF). The trailing whitespace — if any — is left for the top-level loop to
// consume: it separates this directive from the next one.
static sp_err_t sp_fmt_parse_directive(sp_fmt_parser_t* p, sp_fmt_spec_t* spec) {
  if (spec->directive_count >= SP_FMT_MAX_DIRECTIVES) {
    return SP_ERR_FMT_TOO_MANY_DIRECTIVES;
  }

  sp_str_t name = sp_fmt_directive_name(p);
  if (!name.len) return SP_ERR_FMT_BAD_DIRECTIVE;

  u32 index = spec->directive_count++;
  spec->directive_names[index] = name;

  // An arg must be preceded by whitespace. Peek past any whitespace to
  // decide whether one follows — if not, leave the cursor on the whitespace
  // (or wherever it is) for the boundary check and the top-level loop.
  if (sp_fmt_is_whitespace(sp_fmt_peek(p, 0))) {
    u32 off = 1;
    while (sp_fmt_is_whitespace(sp_fmt_peek(p, off))) off++;
    c8 c = sp_fmt_peek(p, off);
    if (c == '$') {
      sp_fmt_eat_whitespace(p);
      sp_fmt_advance(p);
      spec->directive_arg_dynamic |= (u8)(1u << index);
    }
    else if (c && c != '}' && c != '.') {
      sp_fmt_eat_whitespace(p);
      spec->directive_args[index] = sp_fmt_directive_arg(p);
    }
  }

  if (!sp_fmt_at_directive_boundary(p)) return SP_ERR_FMT_BAD_PLACEHOLDER;
  return SP_OK;
}

sp_err_t sp_fmt_parse_specifier(sp_fmt_parser_t* p, sp_fmt_spec_t* spec) {
  if (!sp_fmt_match(p, '{')) return SP_ERR_FMT_BAD_PLACEHOLDER;

  if (sp_fmt_match(p, ':')) {
    sp_try(sp_fmt_parse_spec_body(p, spec));
  }

  sp_fmt_eat_whitespace(p);
  while (sp_fmt_match(p, '.')) {
    sp_try(sp_fmt_parse_directive(p, spec));
    sp_fmt_eat_whitespace(p);
  }

  if (!sp_fmt_match(p, '}')) {
    if (sp_fmt_peek(p, 0) == 0) return SP_ERR_FMT_UNTERMINATED_PLACEHOLDER;
    return SP_ERR_FMT_BAD_PLACEHOLDER;
  }
  return SP_OK;
}


// Pulls one int-kinded dynamic arg out of `a` and returns it as a signed
// value. Used for dynamic fill/width/precision, all of which require an int.
static sp_err_t sp_fmt_pull_int_arg(sp_fmt_arg_t a, s64* out) {
  if (a.id != sp_fmt_id_u64 && a.id != sp_fmt_id_s64) {
    return SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND;
  }
  *out = (a.id == sp_fmt_id_s64) ? a.i : (s64)a.u;
  return SP_OK;
}

sp_err_t sp_fmt_io(sp_io_writer_t* io, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_err_t result = sp_fmt_v_a(io, sp_str_view(fmt), args);
  va_end(args);
  return result;
}

static void sp_fmt_directive_bold(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_io_write_cstr(io, "\033[1m", SP_NULLPTR);
}

static void sp_fmt_directive_italic(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_io_write_cstr(io, "\033[3m", SP_NULLPTR);
}

static void sp_fmt_directive_red(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_RED, SP_NULLPTR);
}

static void sp_fmt_directive_green(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_GREEN, SP_NULLPTR);
}

static void sp_fmt_directive_yellow(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_YELLOW, SP_NULLPTR);
}

static void sp_fmt_directive_blue(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BLUE, SP_NULLPTR);
}

static void sp_fmt_directive_cyan(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_CYAN, SP_NULLPTR);
}

static void sp_fmt_directive_magenta(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_MAGENTA, SP_NULLPTR);
}

static void sp_fmt_directive_white(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_WHITE, SP_NULLPTR);
}

static void sp_fmt_directive_black(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BLACK, SP_NULLPTR);
}

static void sp_fmt_directive_bright_red(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_RED, SP_NULLPTR);
}

static void sp_fmt_directive_bright_green(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_GREEN, SP_NULLPTR);
}

static void sp_fmt_directive_bright_yellow(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_YELLOW, SP_NULLPTR);
}

static void sp_fmt_directive_bright_blue(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_BLUE, SP_NULLPTR);
}

static void sp_fmt_directive_bright_cyan(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_CYAN, SP_NULLPTR);
}

static void sp_fmt_directive_bright_magenta(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_MAGENTA, SP_NULLPTR);
}

static void sp_fmt_directive_bright_white(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_WHITE, SP_NULLPTR);
}

static void sp_fmt_directive_gray(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_io_write_cstr(io, SP_ANSI_FG_BRIGHT_BLACK, SP_NULLPTR);
}

static void sp_fmt_directive_ansi_reset(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(arg); sp_unused(param);
  sp_io_write_cstr(io, SP_ANSI_RESET, SP_NULLPTR);
}

static void sp_fmt_directive_hyperlink(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(params);
  sp_io_write_cstr(io, "\033]8;;", SP_NULLPTR);
  if (arg->id == sp_fmt_id_str) sp_io_write_str(io, arg->s, SP_NULLPTR);
  sp_io_write_cstr(io, "\033\\", SP_NULLPTR);
}

static void sp_fmt_directive_hyperlink_after(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_io_write_cstr(io, "\033]8;;\033\\", SP_NULLPTR);
}

static void sp_fmt_directive_quote(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_io_write_c8(io, '"');
}

static void sp_fmt_directive_quote_after(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_io_write_c8(io, '"');
}

static void sp_fmt_directive_upper_transform(sp_io_writer_t* io, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_for(i, content.len) {
    c8 c = content.data[i];
    sp_io_write_c8(io, (c >= 'a' && c <= 'z') ? (c8)(c - 32) : c);
  }
}

static void sp_fmt_directive_redact_transform(sp_io_writer_t* io, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_for(i, content.len) sp_io_write_c8(io, '*');
}

static void sp_fmt_directive_bytes_render(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  u64 bytes = arg->u; sp_unused(params);
  static const c8* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
  u32 unit_idx = 0;
  u64 whole = bytes;
  u64 rem = 0;
  while (whole >= 1024 && unit_idx < 5) {
    rem = whole & 1023;
    whole >>= 10;
    unit_idx++;
  }
  sp_fmt_write_u64_a(io, whole);
  if (unit_idx > 0) {
    u32 tenths = (u32)((rem * 10) >> 10);
    if (tenths > 0) {
      sp_io_write_c8(io, '.');
      sp_io_write_c8(io, (c8)('0' + tenths));
    }
  }
  sp_io_write_c8(io, ' ');
  sp_io_write_cstr(io, units[unit_idx], SP_NULLPTR);
}

static void sp_fmt_directive_iso_render(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(params);
  sp_tm_epoch_t epoch = SP_RVAL(sp_tm_epoch_t) { .s = arg->u, .ns = 0 };
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_io_write_str(io, sp_tm_epoch_to_iso8601_a(s.mem, epoch), SP_NULLPTR);
  sp_mem_end_scratch_a(s);
}

static void sp_fmt_directive_hex_render(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(params);
  u64 value = (arg->id == sp_fmt_id_s64) ? (u64)arg->i : arg->u;
  c8 buf[16];
  c8* end = buf + sizeof(buf);
  c8* start = sp_fmt_uint_to_buf_hex_ex(value, end, "0123456789ABCDEF");
  sp_io_write_cstr(io, "0x", SP_NULLPTR);
  sp_io_write(io, start, (u64)(end - start), SP_NULLPTR);
}

static void sp_fmt_directive_ordinal_render(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  s64 value = (arg->id == sp_fmt_id_s64) ? arg->i : (s64)arg->u; sp_unused(params);
  sp_fmt_write_s64_a(io, value);
  s64 abs = value < 0 ? -value : value;
  u32 mod100 = (u32)(abs % 100);
  u32 mod10 = (u32)(abs % 10);
  const c8* suffix = "th";
  if (mod100 < 11 || mod100 > 13) {
    if (mod10 == 1) suffix = "st";
    else if (mod10 == 2) suffix = "nd";
    else if (mod10 == 3) suffix = "rd";
  }
  sp_io_write_cstr(io, suffix, SP_NULLPTR);
}

static void sp_fmt_directive_duration_render(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(params);
  u64 ns = arg->u;
  if (ns < 1000) {
    sp_fmt_write_u64_a(io, ns);
    sp_io_write_cstr(io, " ns", SP_NULLPTR);
    return;
  }
  static const c8* units[] = { "us", "ms", "s" };
  u32 unit_idx = 0;
  u64 whole = ns / 1000;
  u64 rem = ns % 1000;
  while (whole >= 1000 && unit_idx < 2) {
    rem = whole % 1000;
    whole /= 1000;
    unit_idx++;
  }
  sp_fmt_write_u64_a(io, whole);
  if (rem >= 100) {
    sp_io_write_c8(io, '.');
    sp_io_write_c8(io, (c8)('0' + rem / 100));
  }
  sp_io_write_c8(io, ' ');
  sp_io_write_cstr(io, units[unit_idx], SP_NULLPTR);
}

sp_str_t sp_fmt_color_to_ansi_fg(sp_str_t id) {
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

static void sp_fmt_directive_fg(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(arg);
  sp_str_t ansi = sp_fmt_color_to_ansi_fg(param->s);
  sp_io_write_str(io, ansi, SP_NULLPTR);
}

void sp_fmt_register_builtins() {
  sp_fmt_register_decorator_p("fg", sp_fmt_directive_fg, sp_fmt_directive_ansi_reset, sp_fmt_id_str);
  sp_fmt_register_decorator("red", sp_fmt_directive_red, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("green", sp_fmt_directive_green, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("yellow", sp_fmt_directive_yellow, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("blue", sp_fmt_directive_blue, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("cyan", sp_fmt_directive_cyan, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("magenta", sp_fmt_directive_magenta, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("white", sp_fmt_directive_white, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("black", sp_fmt_directive_black, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_red", sp_fmt_directive_bright_red, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_green", sp_fmt_directive_bright_green, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_yellow", sp_fmt_directive_bright_yellow, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_blue", sp_fmt_directive_bright_blue, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_cyan", sp_fmt_directive_bright_cyan, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_magenta", sp_fmt_directive_bright_magenta, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_white", sp_fmt_directive_bright_white, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("br_black", sp_fmt_directive_gray, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("gray", sp_fmt_directive_gray, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("bold", sp_fmt_directive_bold, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("italic", sp_fmt_directive_italic, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("hyperlink", sp_fmt_directive_hyperlink, sp_fmt_directive_ansi_reset);
  sp_fmt_register_decorator("quote", sp_fmt_directive_quote, sp_fmt_directive_quote_after);
  sp_fmt_register_transformer("upper", sp_fmt_directive_upper_transform);
  sp_fmt_register_transformer("redact", sp_fmt_directive_redact_transform);
  sp_fmt_register_renderer("bytes", sp_fmt_directive_bytes_render, 0);
  sp_fmt_register_renderer("hex", sp_fmt_directive_hex_render, 0);
  sp_fmt_register_renderer("iso", sp_fmt_directive_iso_render, 0);
  sp_fmt_register_renderer("ordinal", sp_fmt_directive_ordinal_render, 0);
  sp_fmt_register_renderer("duration", sp_fmt_directive_duration_render, 0);

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
    if (abs_value > (u64)INT64_MAX + 1) return false;
    *out = (s64)(~abs_value + 1);
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

bool sp_parse_c16_ex(sp_str_t str, u16* out) {
  // handle 'a' format
  if (str.len == 3 && str.data[0] == '\'' && str.data[2] == '\'') {
    *out = (u16)str.data[1];
    return true;
  }
  // handle 'U+XXXX' format
  if (str.len >= 8 && str.data[0] == '\'' && str.data[1] == 'U' &&
      str.data[2] == '+' && str.data[str.len-1] == '\'') {
    sp_str_t hex_str = sp_str(str.data + 3, str.len - 4);
    u64 val;
    if (sp_parse_hex_ex(hex_str, &val) && val <= UINT16_MAX) {
      *out = (u16)val;
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

u16 sp_parse_c16(sp_str_t str) {
  u16 value = SP_ZERO_INITIALIZE();
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


//  █████          ███████      █████████
// ░░███         ███░░░░░███   ███░░░░░███
//  ░███        ███     ░░███ ███     ░░░
//  ░███       ░███      ░███░███
//  ░███       ░███      ░███░███    █████
//  ░███      █░░███     ███ ░░███  ░░███
//  ███████████ ░░░███████░   ░░█████████
// ░░░░░░░░░░░    ░░░░░░░      ░░░░░░░░░
// @log


//  ██████╗ ██████╗ ███╗   ██╗████████╗███████╗██╗  ██╗████████╗
// ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝
// ██║     ██║   ██║██╔██╗ ██║   ██║   █████╗   ╚███╔╝    ██║
// ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══╝   ██╔██╗    ██║
// ╚██████╗╚██████╔╝██║ ╚████║   ██║   ███████╗██╔╝ ██╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ═╝   ╚═╝
//  @context
SP_PRIVATE void sp_tls_rt_deinit(void *data);
SP_PRIVATE void sp_rt_init();

void sp_tls_rt_pop_context(sp_tls_rt_t* tls) {
  SP_ASSERT(tls->index > 0);
  tls->contexts[tls->index] = SP_ZERO_STRUCT(sp_context_t);
  tls->index--;
}

sp_context_t* sp_tls_rt_get_context(sp_tls_rt_t* tls) {
  return &tls->contexts[tls->index];
}

sp_mem_t sp_tls_rt_get_allocator(sp_tls_rt_t* tls) {
    return sp_tls_rt_get_context(tls)->allocator;
}

sp_mem_arena_t* sp_tls_rt_get_scratch_arena(sp_tls_rt_t* tls) {
  sp_mem_t allocator = sp_tls_rt_get_context(tls)->allocator;

  sp_carr_for(tls->scratch, it) {
    sp_mem_t arena = sp_mem_arena_as_allocator(tls->scratch[it]);
    if (arena.on_alloc != allocator.on_alloc || arena.user_data != allocator.user_data) {
      return tls->scratch[it];
    }
  }

  sp_unreachable_return(SP_NULLPTR);
}


void sp_context_check_index() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  SP_ASSERT(tls->index > 0);
  SP_ASSERT(tls->index < SP_RT_MAX_CONTEXT);
}

void sp_context_set(sp_context_t context) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  tls->contexts[tls->index] = context;
}

void sp_context_push(sp_context_t context) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  SP_ASSERT(tls->index + 1 < SP_RT_MAX_CONTEXT);
  tls->index++;
  tls->contexts[tls->index] = context;
}

void sp_context_push_allocator(sp_mem_t allocator) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_context_t context = tls->contexts[tls->index];
  context.allocator = allocator;
  sp_context_push(context);
}

sp_mem_t sp_context_get_allocator() {
  return sp_context_get()->allocator;
}

void sp_context_push_arena(sp_mem_arena_t* arena) {
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));
}

void sp_context_pop() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_tls_rt_pop_context(tls);
}

sp_context_t* sp_context_get() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  return sp_tls_rt_get_context(tls);
}

void sp_rt_init() {
  sp_mutex_init(&sp_rt.mutex, SP_MUTEX_PLAIN);
  sp_tls_new(&sp_rt.tls.key, sp_tls_rt_deinit);
#if defined(SP_WIN32)
  sp_nt_load();
#endif
}

void sp_tls_rt_deinit(void* ptr) {
  if (!ptr) return;
  sp_tls_rt_t* tls = (sp_tls_rt_t*)ptr;
  sp_mem_allocator_free(tls->contexts[0].allocator, tls->std.out);
  sp_mem_allocator_free(tls->contexts[0].allocator, tls->std.err);
  sp_str_ht_free(tls->format.directives);
  sp_carr_for(tls->scratch, it) {
    sp_mem_arena_destroy(tls->scratch[it]);
  }
  sp_sys_free(tls, sizeof(sp_tls_rt_t));
}

sp_tls_rt_t* sp_tls_rt_get() {
  sp_tls_once(&sp_rt.tls.once, sp_rt_init);

  sp_tls_rt_t* tls = (sp_tls_rt_t*)sp_tls_get(sp_rt.tls.key);
  if (!tls) {
    // It's important that you bootstrap the allocator and set the TLS key
    // before doing anything else so you can call functions that allocate
    // while initializing the other TLS stuff.
    tls = sp_sys_alloc_type(sp_tls_rt_t);
    tls->contexts[0].allocator = sp_mem_os_new();
    sp_tls_set(sp_rt.tls.key, tls);

    sp_carr_for(tls->scratch, it) {
      tls->scratch[it] = sp_mem_arena_new();
    }
    tls->std.out = sp_alloc_type(sp_io_writer_t);
    tls->std.err = sp_alloc_type(sp_io_writer_t);
    sp_io_writer_from_fd(tls->std.out, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
    sp_io_writer_from_fd(tls->std.err, sp_sys_stderr, SP_IO_CLOSE_MODE_NONE);
    sp_str_ht_init_a(tls->contexts[0].allocator, tls->format.directives);
    sp_fmt_register_builtins();
    sp_sys_tls_init(tls);
  }
  return tls;
}

#if defined(SP_FREESTANDING)
void sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t on_deinit) {

}

void* sp_tls_get(sp_tls_key_t key) {
  sp_tls_block_t* tb = (sp_tls_block_t*)sp_sys_get_tp();
  return tb ? tb->data : 0;
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  sp_tls_block_t* tb = (sp_tls_block_t*)sp_sys_get_tp();
  if (tb) tb->data = data;
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {

}

#elif defined(SP_WASM)
void sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t on_deinit) {
  (void)key; (void)on_deinit;
}

void* sp_tls_get(sp_tls_key_t key) {
  sp_unused(key);
  return sp_tls_block.data;
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  sp_unused(key);
  sp_tls_block.data = data;
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {
  if (once) return;
  fn();
  *once = 1;
}

#elif defined(SP_WIN32)
void sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t on_deinit) {
  SP_UNUSED(on_deinit);
  *key = TlsAlloc();
}

void* sp_tls_get(sp_tls_key_t key) {
  return TlsGetValue(key);
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  TlsSetValue(key, data);
}

SP_PRIVATE BOOL CALLBACK sp_tls_once_trampoline(PINIT_ONCE once, PVOID param, PVOID* ctx) {
  SP_UNUSED(once);
  SP_UNUSED(ctx);
  sp_tls_once_fn_t fn = *(sp_tls_once_fn_t*)param;
  fn();
  return TRUE;
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {
  sp_tls_once_fn_t callback = fn;
  InitOnceExecuteOnce(once, sp_tls_once_trampoline, &callback, SP_NULLPTR);
}

#elif defined(SP_POSIX)
void sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t on_deinit) {
  pthread_key_create(key, on_deinit);
}

void* sp_tls_get(sp_tls_key_t key) {
  return pthread_getspecific(key);
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  pthread_setspecific(key, data);
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {
  pthread_once(once, fn);
}

#else
void sp_tls_new(sp_tls_key_t* key, sp_tls_deinit_fn_t on_deinit) {
  SP_UNIMPLEMENTED();
}

void* sp_tls_get(sp_tls_key_t key) {
  SP_UNIMPLEMENTED();
  return SP_NULLPTR;
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  SP_UNIMPLEMENTED();
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {
  SP_UNIMPLEMENTED();
}
#endif


//  ██████   ██████ ██████████ ██████   ██████    ███████    ███████████   █████ █████
// ░░██████ ██████ ░░███░░░░░█░░██████ ██████   ███░░░░░███ ░░███░░░░░███ ░░███ ░░███
//  ░███░█████░███  ░███  █ ░  ░███░█████░███  ███     ░░███ ░███    ░███  ░░███ ███
//  ░███░░███ ░███  ░██████    ░███░░███ ░███ ░███      ░███ ░██████████    ░░█████
//  ░███ ░░░  ░███  ░███░░█    ░███ ░░░  ░███ ░███      ░███ ░███░░░░░███    ░░███
//  ░███      ░███  ░███ ░   █ ░███      ░███ ░░███     ███  ░███    ░███     ░███
//  █████     █████ ██████████ █████     █████ ░░░███████░   █████   █████    █████
// ░░░░░     ░░░░░ ░░░░░░░░░░ ░░░░░     ░░░░░    ░░░░░░░    ░░░░░   ░░░░░    ░░░░░
// @memory

void* sp_alloc(u64 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_alloc(ctx->allocator, size);
}

void* sp_realloc(void* memory, u64 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_realloc(ctx->allocator, memory, size);
}

void sp_free(void* memory) {
  sp_context_t* ctx = sp_context_get();
  sp_mem_allocator_free(ctx->allocator, memory);
}

void* sp_mem_allocator_alloc(sp_mem_t allocator, u64 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
}

void* sp_mem_allocator_realloc(sp_mem_t allocator, void* memory, u64 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_RESIZE, size, memory);
}

void sp_mem_allocator_free(sp_mem_t allocator, void* buffer) {
  allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_FREE, 0, buffer);
}

typedef struct {
  u64 size;
  u8 padding [8];
} sp_mem_arena_header_t;

sp_mem_arena_block_t* sp_mem_arena_block_new(sp_mem_arena_t* arena, u64 capacity) {
  sp_mem_arena_block_t* block = sp_mem_allocator_alloc_type(arena->allocator, sp_mem_arena_block_t);
  block->next = SP_NULLPTR;
  block->buffer = sp_mem_allocator_alloc_n(arena->allocator, u8, capacity);
  block->capacity = capacity;
  block->bytes_used = 0;
  return block;
}

sp_mem_arena_t* sp_mem_arena_new() {
  return sp_mem_arena_new_ex(SP_MEM_ARENA_BLOCK_SIZE, SP_MEM_ARENA_MODE_DEFAULT, SP_MEM_ALIGNMENT);
}

sp_mem_arena_t* sp_mem_arena_new_ex(u64 block_size, sp_mem_arena_mode_t mode, u8 alignment) {
  return sp_mem_arena_new_with_allocator(sp_context_get()->allocator, block_size, mode, alignment);
}

sp_mem_arena_t* sp_mem_arena_new_with_allocator(sp_mem_t allocator, u64 block_size, sp_mem_arena_mode_t mode, u8 alignment) {
  sp_mem_arena_t* arena = (sp_mem_arena_t*)sp_mem_allocator_alloc(allocator, sizeof(sp_mem_arena_t));
  arena->allocator = allocator;
  arena->mode = mode;
  arena->alignment = alignment == 0 ? SP_MEM_ALIGNMENT : alignment;

  sp_mem_arena_block_t* block = sp_mem_arena_block_new(arena, block_size);
  arena->head = block;
  arena->current = block;
  arena->block_size = block_size;
  return arena;
}

sp_mem_t sp_mem_arena_as_allocator(sp_mem_arena_t* a) {
  return (sp_mem_t) {
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

u64 sp_mem_arena_capacity(sp_mem_arena_t* arena) {
  u64 total = 0;
  for (sp_mem_arena_block_t* block = arena->head; block; block = block->next) {
    total += block->capacity;
  }
  return total;
}

u64 sp_mem_arena_bytes_used(sp_mem_arena_t* arena) {
  u64 total = 0;
  for (sp_mem_arena_block_t* block = arena->head; ; block = block->next) {
    total += block->bytes_used;
    if (block == arena->current) break;
  }
  return total;
}

sp_mem_arena_header_t* sp_mem_arena_get_header(void* ptr) {
  return (sp_mem_arena_header_t*)((u8*)ptr - sizeof(sp_mem_arena_header_t));
}

void* sp_mem_arena_get_ptr(sp_mem_arena_header_t* header) {
  return (void*)(header + 1);
}

void* sp_mem_arena_align_block(sp_mem_arena_block_t* block, u8 alignment) {
  block->bytes_used = sp_align_offset(block->bytes_used, alignment);
  return block->buffer + block->bytes_used;
}

sp_mem_arena_block_t* sp_mem_arena_get_block(sp_mem_arena_t* arena, u64 alloc_size) {
  sp_mem_arena_block_t* block = arena->current;
  u64 offset = sp_align_offset(block->bytes_used, arena->alignment);

  if (offset + alloc_size <= block->capacity) {
    block->bytes_used = offset;
    return block;
  }

  u64 new_capacity = sp_max(arena->block_size, alloc_size);

  if (block->next && block->next->capacity >= alloc_size) {
    block->next->bytes_used = 0;
    block = block->next;
  }
  else {
    sp_mem_arena_block_t* new_block = sp_mem_arena_block_new(arena, new_capacity);
    new_block->next = block->next;
    block->next = new_block;
    block = new_block;
  }

  arena->current = block;
  return block;
}

void* sp_mem_arena_alloc_with_header(sp_mem_arena_t* arena, u64 size) {
  u64 total = sizeof(sp_mem_arena_header_t) + size;

  sp_mem_arena_block_t* block = sp_mem_arena_get_block(arena, total);
  sp_mem_arena_header_t* header = (sp_mem_arena_header_t*)sp_mem_arena_align_block(block, arena->alignment);
  header->size = size;

  void* ptr = sp_mem_arena_get_ptr(header);
  sp_mem_zero(ptr, size);
  block->bytes_used += total;

  return ptr;
}

void* sp_mem_arena_alloc_no_header(sp_mem_arena_t* arena, u64 size) {
  sp_mem_arena_block_t* block = sp_mem_arena_get_block(arena, size);
  void* ptr = sp_mem_arena_align_block(block, arena->alignment);
  sp_mem_zero(ptr, size);
  block->bytes_used += size;

  return ptr;
}

void* sp_mem_arena_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* old_memory) {
  sp_mem_arena_t* arena = (sp_mem_arena_t*)user_data;

  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      switch (arena->mode) {
        case SP_MEM_ARENA_MODE_DEFAULT: {
          return sp_mem_arena_alloc_with_header(arena, size);
        }
        case SP_MEM_ARENA_MODE_NO_REALLOC: {
          return sp_mem_arena_alloc_no_header(arena, size);
        }
      }
      SP_UNREACHABLE_RETURN(SP_NULLPTR);
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      sp_require_as_null(arena->mode == SP_MEM_ARENA_MODE_DEFAULT);

      void* ptr = sp_mem_arena_alloc_with_header(arena, size);
      if (old_memory) {
        sp_mem_arena_header_t* header = sp_mem_arena_get_header(old_memory);
        sp_mem_move(old_memory, ptr, sp_min(header->size, size));
      }
      return ptr;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      return SP_NULLPTR;
    }
  }

  SP_UNREACHABLE_RETURN(SP_NULLPTR);
}

void* sp_mem_arena_alloc(sp_mem_arena_t* arena, u64 size) {
  return sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_ALLOC, size, SP_NULLPTR);
}

void* sp_mem_arena_realloc(sp_mem_arena_t* arena, void* ptr, u64 size) {
  return sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_RESIZE, size, ptr);
}

void sp_mem_arena_free(sp_mem_arena_t* arena, void* ptr) {
  sp_mem_arena_on_alloc(arena, SP_ALLOCATOR_MODE_FREE, 0, ptr);
}

sp_mem_arena_marker_t sp_mem_arena_mark(sp_mem_arena_t* arena) {
  return (sp_mem_arena_marker_t) {
    .arena = arena,
    .block = arena->current,
    .mark = arena->current->bytes_used,
    .mem = sp_mem_arena_as_allocator(arena),
  };
}

void sp_mem_arena_pop(sp_mem_arena_marker_t marker) {
  marker.block->bytes_used = marker.mark;
  marker.arena->current = marker.block;
}

sp_mem_arena_t* sp_mem_get_scratch_arena() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  return sp_tls_rt_get_scratch_arena(tls);
}

sp_mem_scratch_t sp_mem_begin_scratch() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_mem_arena_t* arena = sp_tls_rt_get_scratch_arena(tls);

  sp_mem_scratch_t s = {
    .marker = sp_mem_arena_mark(arena),
  };
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));
  return s;
}

void sp_mem_end_scratch(sp_mem_scratch_t scratch) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_tls_rt_pop_context(tls);
  sp_mem_arena_pop(scratch.marker);
}

bool sp_mem_is_equal(const void* a, const void* b, u64 len) {
  if (!len) return true;
  if (!a || !b) return false;
  return !sp_sys_memcmp(a, b, len);
}

void sp_mem_copy(const void* source, void* dest, u64 num_bytes) {
  if (!source) return;
  if (!dest) return;
  if (!num_bytes) return;
  sp_sys_memcpy(dest, source, num_bytes);
}

void sp_mem_move(const void* source, void* dest, u64 num_bytes) {
  if (!source) return;
  if (!dest) return;
  sp_sys_memmove(dest, source, num_bytes);
}

void sp_mem_fill(void* buffer, u64 buffer_size, void* fill, u64 fill_size) {
  u8* current_byte = (u8*)buffer;

  u64 i = 0;
  while (true) {
    if (i + fill_size > buffer_size) return;
    sp_mem_copy((u8*)fill, current_byte + i, fill_size);
    i += fill_size;
  }
}

void sp_mem_fill_u8(void* buffer, u64 buffer_size, u8 fill) {
  sp_mem_fill(buffer, buffer_size, &fill, sizeof(u8));
}

void sp_mem_zero(void* buffer, u64 buffer_size) {
  sp_mem_fill_u8(buffer, buffer_size, 0);
}

void* sp_mem_os_alloc(u64 size) {
  sp_mem_os_header_t* h = (sp_mem_os_header_t*)sp_sys_alloc(size + sizeof(sp_mem_os_header_t));
  if (!h) return SP_NULLPTR;
  h->size = size;
  return h + 1;
}

void* sp_mem_os_alloc_zero(u64 size) {
  return sp_mem_os_alloc(size);
}

void sp_mem_os_free(void* ptr) {
  if (!ptr) return;
  sp_mem_os_header_t* h = sp_mem_os_get_header(ptr);
  sp_sys_free(h, h->size + sizeof(sp_mem_os_header_t));
}

void* sp_mem_os_realloc(void* ptr, u64 size) {
  if (!ptr) return sp_mem_os_alloc(size);
  if (!size) {
    sp_mem_os_free(ptr);
    return SP_NULLPTR;
  }

  sp_mem_os_header_t* header = sp_mem_os_get_header(ptr);
  if (header->size >= size) {
    return ptr;
  }

  void* mem = sp_mem_os_alloc(size);
  if (!mem) return SP_NULLPTR;
  sp_mem_copy(ptr, mem, header->size);
  sp_mem_os_free(ptr);
  return mem;
}

sp_mem_slice_t sp_mem_slice(u8* ptr, u64 len) {
  return (sp_mem_slice_t) { .data = ptr, .len = len };
}

sp_mem_slice_t sp_mem_slice_copy(sp_mem_slice_t slice) {
  u8* buffer = sp_alloc_n(u8, slice.len);
  sp_mem_copy(slice.data, buffer, (u32)slice.len);
  return sp_mem_slice(buffer, slice.len);
}

sp_mem_slice_t sp_mem_slice_sub(sp_mem_slice_t slice, u64 start, u64 len) {
  if (start >= slice.len) {
    return sp_mem_slice(SP_NULLPTR, 0);
  }
  u64 actual_len = sp_min(len, slice.len - start);
  return sp_mem_slice(slice.data + start, actual_len);
}

sp_mem_slice_t sp_mem_slice_prefix(sp_mem_slice_t slice, u64 len) {
  return sp_mem_slice_sub(slice, 0, len);
}

sp_mem_slice_t sp_mem_slice_suffix(sp_mem_slice_t slice, u64 len) {
  if (len >= slice.len) return slice;
  return sp_mem_slice(slice.data + slice.len - len, len);
}

bool sp_mem_slice_empty(sp_mem_slice_t slice) {
  return slice.len == 0 || slice.data == SP_NULLPTR;
}

u8 sp_mem_slice_at(sp_mem_slice_t slice, u64 index) {
  SP_ASSERT(index < slice.len);
  return slice.data[index];
}

sp_mem_slice_it_t sp_mem_slice_it(sp_mem_slice_t slice) {
  return (sp_mem_slice_it_t) {
    .slice = slice,
    .index = 0,
    .byte = (u8)(slice.len ? slice.data[0] : 0)
  };
}

bool sp_mem_slice_it_valid(sp_mem_slice_it_t* it) {
  return it->index < it->slice.len;
}

void sp_mem_slice_it_next(sp_mem_slice_it_t* it) {
  it->index++;
  it->byte = sp_mem_slice_it_valid(it) ? it->slice.data[it->index] : 0;
}

sp_str_t sp_mem_buffer_as_str(sp_mem_buffer_t* buffer) {
  return (sp_str_t) {
    .data = (c8*)buffer->data,
    .len = (u32)buffer->len
  };
}

c8* sp_mem_buffer_as_cstr(sp_mem_buffer_t* buffer) {
  return (c8*)buffer->data;
}


//   █████████  ███████████ ███████████   █████ ██████   █████   █████████
//  ███░░░░░███░█░░░███░░░█░░███░░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███
// ░███    ░░░ ░   ░███  ░  ░███    ░███  ░███  ░███░███ ░███  ███     ░░░
// ░░█████████     ░███     ░██████████   ░███  ░███░░███░███ ░███
//  ░░░░░░░░███    ░███     ░███░░░░░███  ░███  ░███ ░░██████ ░███    █████
//  ███    ░███    ░███     ░███    ░███  ░███  ░███  ░░█████ ░░███  ░░███
// ░░█████████     █████    █████   █████ █████ █████  ░░█████ ░░█████████
//  ░░░░░░░░░     ░░░░░    ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░
// @string
c8* sp_cstr_copy_n(const c8* str, u32 length) {
  u32 buffer_length = length + 1;
  c8* copy = (c8*)sp_alloc(buffer_length);
  sp_cstr_copy_to_n(str, length, copy, buffer_length);
  return copy;
}

c8* sp_cstr_copy(const c8* str) {
  return sp_cstr_copy_n(str, sp_cstr_len(str));
}

void sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length) {
  sp_cstr_copy_to_n(str, sp_cstr_len(str), buffer, buffer_length);
}

void sp_cstr_copy_to_n(const c8* str, u32 length, c8* buffer, u32 buffer_length) {
  if (!str) return;
  if (!buffer) return;
  if (!buffer_length) return;

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

u32 sp_cstr_len_n(const c8* str, u32 n) {
  sp_require_as(str, 0);

  u32 len = 0;
  while (len < n && str[len]) len++;
  return len;
}

sp_str_t sp_cstr_as_str(const c8* str) {
  return (sp_str_t) {
    .data = str,
    .len = sp_cstr_len(str)
  };
}

sp_str_t sp_cstr_as_str_n(const c8* str, u32 n) {
  return (sp_str_t) {
    .data = str,
    .len = n
  };
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
  if (!out) return 0;

  u8 len = sp_utf8_num_bytes_from_codepoint(cp);
  switch (len) {
    case 1: { out[0] = (u8)cp; break; }
    case 2: { out[0] = (u8)(0xC0 | (cp >> 6)); out[1] = (u8)(0x80 | (cp & 0x3F)); break; }
    case 3: { out[0] = (u8)(0xE0 | (cp >> 12)); out[1] = (u8)(0x80 | ((cp >> 6) & 0x3F)); out[2] = (u8)(0x80 | (cp & 0x3F)); break; }
    case 4: { out[0] = (u8)(0xF0 | (cp >> 18)); out[1] = (u8)(0x80 | ((cp >> 12) & 0x3F)); out[2] = (u8)(0x80 | ((cp >> 6) & 0x3F)); out[3] = (u8)(0x80 | (cp & 0x3F)); break; }
  }
  return len;
}

sp_utf8_it_t sp_utf8_it(sp_str_t str) {
  if (sp_str_empty(str)) return sp_zero_struct(sp_utf8_it_t);

  return (sp_utf8_it_t) {
    .str = { .data = (const u8*)str.data, .len = str.len },
    .index = 0,
    .codepoint = sp_utf8_decode((const u8*)str.data),
    .codepoint_len = sp_utf8_num_bytes_from_byte((u8)str.data[0]),
  };
}

sp_utf8_it_t sp_utf8_rit(sp_str_t str) {
  if (sp_str_empty(str)) return sp_zero_struct(sp_utf8_it_t);

  // @spader
  // the cast is objectively wrong here. i just use s32 in the iterator to give me a sentinel when reverse
  // iterating. which in and of itself is stupid. double down.
  sp_utf8_it_t it = {
    .str = { .data = (const u8*)str.data, .len = str.len },
    .index = (s32)str.len
  };
  sp_utf8_it_prev(&it);
  return it;
}

bool sp_utf8_it_valid(sp_utf8_it_t* it) {
  if (!it) return false;
  return (it->index < (s32)it->str.len) && (it->index >= 0) && (it->codepoint_len);
}

void sp_utf8_it_next(sp_utf8_it_t* it) {
  if (!it) return;

  it->index += it->codepoint_len;
  if (it->index >= (s32)it->str.len) {
    return;
  }

  it->codepoint = sp_utf8_decode(it->str.data + it->index);
  it->codepoint_len = sp_utf8_num_bytes_from_byte(it->str.data[it->index]);
}

void sp_utf8_it_prev(sp_utf8_it_t* it) {
  if (!it) return;

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

bool sp_wtf8_validate(sp_str_t str) {
  const u8* ptr = (const u8*)str.data;
  u32 it = 0;

  while (it < str.len) {
    u8 len = sp_utf8_num_bytes_from_byte(ptr[it]);
    if (!len) return false;
    if (it + len > str.len) return false;

    sp_for_range(j, 1, len) {
      if (!sp_utf8_is_cont(ptr[it + j])) return false;
    }

    u32 cp = sp_utf8_decode(&ptr[it]);
    if (!sp_utf8_is_bounds_ok(cp, len)) return false;

    it += len;
  }
  return true;
}

sp_wide_str_t sp_wtf8_to_wtf16(sp_str_t wtf8) {
  sp_wide_str_t result = SP_ZERO_STRUCT(sp_wide_str_t);
  if (sp_str_empty(wtf8)) return result;

  u16* buf = sp_alloc_n(u16, wtf8.len + 1);
  const u8* ptr = (const u8*)wtf8.data;
  u32 i = 0;
  u32 n = 0;

  while (i < wtf8.len) {
    u8 len = sp_utf8_num_bytes_from_byte(ptr[i]);
    if (!len) goto error;
    if (i + len > wtf8.len) goto error;
    sp_for_range(j, 1, len) {
      if (!sp_utf8_is_cont(ptr[i + j])) goto error;
    }

    u32 cp = sp_utf8_decode(&ptr[i]);
    if (!sp_utf8_is_bounds_ok(cp, len)) goto error;

    if (cp < 0x10000) {
      buf[n++] = (u16)cp;
    }
    else {
      u32 v = cp - 0x10000;
      buf[n++] = (u16)(0xD800 | (v >> 10));
      buf[n++] = (u16)(0xDC00 | (v & 0x3FF));
    }

    i += len;
  }

  buf[n] = 0;
  return (sp_wide_str_t) { .data = buf, .len = n };

error:
  sp_free(buf);
  return result;
}

sp_str_t sp_wtf16_to_wtf8(sp_wide_str_t wtf16) {
  c8* buf = sp_alloc_n(c8, wtf16.len * 3 + 1);
  u32 offset = 0;
  sp_for(i, wtf16.len) {
    u16 u = wtf16.data[i];
    u32 cp;
    if (u >= 0xD800 && u <= 0xDBFF && i + 1 < wtf16.len && wtf16.data[i + 1] >= 0xDC00 && wtf16.data[i + 1] <= 0xDFFF) {
      cp = 0x10000 + (((u32)(u - 0xD800) << 10) | (u32)(wtf16.data[i + 1] - 0xDC00));
      i++;
    }
    else {
      cp = u;
    }
    offset += sp_utf8_encode(cp, (u8*)buf + offset);
  }
  buf[offset] = 0;

  return (sp_str_t) { .data = buf, .len = offset };
}

c8* sp_str_to_cstr(sp_str_t str) {
  return sp_cstr_copy_n(str.data, str.len);
}

sp_str_t sp_str_alloc(u32 capacity) {
  if (!capacity) return SP_ZERO_STRUCT(sp_str_t);
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
  return sp_str_find(str, needle) != SP_STR_NO_MATCH;
}

s32 sp_str_find(sp_str_t str, sp_str_t needle) {
  if (needle.len == 0) return 0;
  if (str.len < needle.len) return SP_STR_NO_MATCH;

  for (u32 i = 0; i <= str.len - needle.len; i++) {
    if (sp_mem_is_equal(str.data + i, needle.data, needle.len)) {
      return (s32)i;
    }
  }
  return SP_STR_NO_MATCH;
}

s32 sp_str_find_c8(sp_str_t str, c8 needle) {
  sp_str_for(str, it) {
    if (str.data[it] == needle) return it;
  }

  return SP_STR_NO_MATCH;
}

s32 sp_str_find_c8_reverse(sp_str_t str, c8 needle) {
  for (s32 i = str.len - 1; i >= 0; i--) {
    if (str.data[i] == needle) return i;
  }

  return SP_STR_NO_MATCH;
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
  return str.len ? str.data[str.len - 1] : 0;
}


sp_str_t sp_str_prefix(sp_str_t str, s32 len) {
  return sp_str_sub(str, 0, len);
}

sp_str_t sp_str_suffix(sp_str_t str, s32 len) {
  return sp_str_sub(str, str.len - len, len);
}

sp_str_t sp_str_sub(sp_str_t str, s32 index, s32 len) {
  SP_ASSERT(index + len <= (s32)str.len);
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  return sp_str(str.data + index, (u32)len);
}

sp_str_t sp_str_sub_reverse(sp_str_t str, s32 index, s32 len) {
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  return (sp_str_t) {
    .data = str.data + str.len + index - len,
    .len = (u32)len,
  };
}

sp_str_t sp_str_from_cstr_n(const c8* str, u32 length) {
  if (!str || !length) return SP_ZERO_STRUCT(sp_str_t);
  u32 len = SP_MIN(sp_cstr_len(str), length);
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = (c8*)sp_alloc(length);
  sp_mem_copy(str, buffer, len);

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr(const c8* str) {
  if (!str) return SP_ZERO_STRUCT(sp_str_t);
  u32 len = sp_cstr_len(str);
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = sp_alloc_n(c8, len + 1);
  sp_mem_copy(str, buffer, len);

  return sp_str(buffer, len);
}

sp_str_t sp_str_copy(sp_str_t str) {
  if (!str.data || !str.len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = sp_alloc_n(c8, str.len);
  sp_mem_copy(str.data, buffer, str.len);

  return SP_STR(buffer, str.len);
}

void sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity) {
  if (!str.data) return;
  sp_mem_copy(str.data, buffer, SP_MIN(str.len, capacity));
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

sp_str_t sp_str_pad(sp_str_t str, u32 n) {
  s32 delta = (s32)n - (s32)str.len;
  if (delta <= 0) return sp_str_copy(str);

  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, n),
    .capacity = n,
    .len = n,
  };
  sp_sys_memcpy(buffer.data, str.data, str.len);
  sp_sys_memset(buffer.data + str.len, ' ', (u32)delta);

  return sp_mem_buffer_as_str(&buffer);
}

sp_str_t sp_str_replace_c8(sp_str_t str, c8 from, c8 to) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, str.len),
    .capacity = str.len,
    .len = str.len,
  };

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    buffer.data[i] = (u8)(c == from ? to : c);
  }

  return sp_mem_buffer_as_str(&buffer);
}

sp_da(sp_str_t) sp_str_split_c8(sp_str_t str, c8 delimiter) {
  if (sp_str_empty(str)) return SP_NULLPTR;

  sp_da(sp_str_t) result = SP_NULLPTR;

  u32 i = 0, j = 0;
  for (; j < str.len; j++) {
    if (sp_str_at(str, j) == delimiter) {
      sp_da_push(result, sp_str_sub(str, i, j - i));
      i = j + 1;
    }
  }

  sp_da_push(result, sp_str_sub(str, i, j - i));

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

  result.first = sp_str_empty(str) ? SP_ZERO_STRUCT(sp_str_t) : str;
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

  return SP_ZERO_STRUCT(sp_str_t);
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

  return SP_ZERO_STRUCT(sp_str_t);
}

sp_str_t sp_str_trim(sp_str_t str) {
  sp_str_t trimmed = str;
  trimmed = sp_str_trim_left(trimmed);
  trimmed = sp_str_trim_right(trimmed);
  return trimmed;
}

sp_str_t sp_str_strip_left(sp_str_t str, sp_str_t strip) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  if (!sp_str_starts_with(str, strip)) return str;
  return sp_str_suffix(str, str.len - strip.len);
}

sp_str_t sp_str_strip_right(sp_str_t str, sp_str_t strip) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  if (!sp_str_ends_with(str, strip)) return str;
  return sp_str_prefix(str, str.len - strip.len);
}

sp_str_t sp_str_strip(sp_str_t str, sp_str_t strip) {
  sp_str_t result = sp_str_strip_left(str, strip);
  return sp_str_strip_right(result, strip);
}

sp_str_t sp_str_to_upper(sp_str_t str) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, str.len),
    .capacity = str.len,
  };

  sp_str_for_utf8(str, it) {
    buffer.len += sp_utf8_encode(sp_utf8_to_upper(it.codepoint), buffer.data + buffer.len);
  }

  return sp_mem_buffer_as_str(&buffer);
}

sp_str_t sp_str_to_lower(sp_str_t str) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, str.len),
    .capacity = str.len,
  };

  sp_str_for_utf8(str, it) {
    buffer.len += sp_utf8_encode(sp_utf8_to_lower(it.codepoint), buffer.data + buffer.len);
  }

  return sp_mem_buffer_as_str(&buffer);
}

sp_str_t sp_str_to_pascal_case(sp_str_t str) {
  if (sp_str_empty(str)) return SP_ZERO_STRUCT(sp_str_t);
  sp_mem_buffer_t buffer = {
    .data = sp_alloc_n(u8, str.len),
    .capacity = str.len,
  };
  bool word = true;

  sp_str_for_utf8(str, it) {
    u32 cp;
    switch (it.codepoint) {
      case ' ':
      case '\t':
      case '\n':
      case '\r': {
        cp = it.codepoint;
        word = true;
        break;
      }
      default: {
        cp = word ? sp_utf8_to_upper(it.codepoint) : sp_utf8_to_lower(it.codepoint);
        word = false;
        break;
      }
    }
    buffer.len += sp_utf8_encode(cp, buffer.data + buffer.len);
  }

  return sp_mem_buffer_as_str(&buffer);
}

sp_str_t sp_str_truncate_a(sp_mem_t mem, sp_str_t str, u32 max_len, sp_str_t trailer) {
  sp_require_as(max_len, sp_str_copy_a(mem, str));
  sp_require_as(str.len > max_len, sp_str_copy_a(mem, str));
  sp_require_as(trailer.len <= max_len, sp_str_copy_a(mem, str));

  return sp_str_concat_a(mem, sp_str_prefix(str, max_len - trailer.len), trailer);
}

sp_da(sp_str_t) sp_str_map_a(sp_mem_t mem, sp_str_t* strs, u32 num_strs, void* user_data, sp_str_map_fn_t fn) {
  sp_da(sp_str_t) results = SP_NULLPTR;

  sp_for(it, num_strs) {
    sp_str_map_context_t context = {
      .mem = mem,
      .str = strs[it],
      .user_data = user_data
    };
    sp_str_t result = fn(&context);
    sp_da_push(results, result);
  }

  return results;
}

sp_str_t sp_str_map_kernel_prepend(sp_str_map_context_t* context) {
  sp_str_t prefix = *(sp_str_t*)context->user_data;
  return sp_str_concat_a(context->mem, prefix, context->str);
}

sp_str_t sp_str_map_kernel_append(sp_str_map_context_t* context) {
  sp_str_t suffix = *(sp_str_t*)context->user_data;
  return sp_str_concat_a(context->mem, context->str, suffix);
}

sp_str_t sp_str_map_kernel_prefix(sp_str_map_context_t* context) {
  u32 len;
  sp_mem_copy(context->user_data, &len, sizeof(len));
  return sp_str_sub(context->str, 0, len);
}

sp_str_t sp_str_map_kernel_pad(sp_str_map_context_t* context) {
  u32 len;
  sp_mem_copy(context->user_data, &len, sizeof(len));
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

sp_da(sp_str_t) sp_str_pad_to_longest_a(sp_mem_t mem, sp_str_t* strs, u32 n) {
  u32 max_len = 0;
  sp_for(i, n) {
    if (strs[i].len > max_len) max_len = strs[i].len;
  }
  return sp_str_map_a(mem, strs, n, &max_len, sp_str_map_kernel_pad);
}


// ███████████ █████ █████       ██████████  █████████  █████ █████  █████████  ███████████ ██████████ ██████   ██████
// ░░███░░░░░░█░░███ ░░███       ░░███░░░░░█ ███░░░░░███░░███ ░░███  ███░░░░░███░█░░░███░░░█░░███░░░░░█░░██████ ██████
//  ░███   █ ░  ░███  ░███        ░███  █ ░ ░███    ░░░  ░░███ ███  ░███    ░░░ ░   ░███  ░  ░███  █ ░  ░███░█████░███
//  ░███████    ░███  ░███        ░██████   ░░█████████   ░░█████   ░░█████████     ░███     ░██████    ░███░░███ ░███
//  ░███░░░█    ░███  ░███        ░███░░█    ░░░░░░░░███   ░░███     ░░░░░░░░███    ░███     ░███░░█    ░███ ░░░  ░███
//  ░███  ░     ░███  ░███      █ ░███ ░   █ ███    ░███    ░███     ███    ░███    ░███     ░███ ░   █ ░███      ░███
//  █████       █████ ███████████ ██████████░░█████████     █████   ░░█████████     █████    ██████████ █████     █████
// ░░░░░       ░░░░░ ░░░░░░░░░░░ ░░░░░░░░░░  ░░░░░░░░░     ░░░░░     ░░░░░░░░░     ░░░░░    ░░░░░░░░░░ ░░░░░     ░░░░░
// @fs @filesystem

//////////////
// ITERATOR //
//////////////
SP_PRIVATE bool sp_fs_it_is_dot(const c8* name) {
  return name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0));
}

#if defined(SP_WIN32)
SP_PRIVATE sp_fs_kind_t sp_fs_it_win32_attrs(sp_win32_dword_t attrs) {
  if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) return SP_FS_KIND_SYMLINK;
  if (attrs & FILE_ATTRIBUTE_DIRECTORY) return SP_FS_KIND_DIR;
  return SP_FS_KIND_FILE;
}

SP_PRIVATE bool sp_win32_find_is_dot(const u16* name) {
  if (name[0] == '.' && name[1] == 0) return true;
  if (name[0] == '.' && name[1] == '.' && name[2] == 0) return true;
  return false;
}

SP_PRIVATE u32 sp_win32_find_name_len(const u16* name) {
  u32 n = 0;
  while (name[n]) n++;
  return n;
}

bool sp_sys_diriter_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  sp_str_t pattern = sp_fs_join_path(path, sp_str_lit("*"));
  sp_sys_nt_path_t nt = SP_ZERO_INITIALIZE();
  if (!SP_NT_SUCCESS(sp_sys_nt_path(pattern, &nt))) {
    return false;
  }

  u32 w_off = 0;
  if (nt.name.Length >= 8 && nt.name.Buffer[0] == '\\' && nt.name.Buffer[1] == '?' && nt.name.Buffer[2] == '?' && nt.name.Buffer[3] == '\\') {
    w_off = 4;
  }
  u32 prefixed_len = 4 + (nt.name.Length / sizeof(u16)) - w_off;
  u16* wpat = sp_alloc_n(u16, prefixed_len + 1);
  wpat[0] = '\\'; wpat[1] = '\\'; wpat[2] = '?'; wpat[3] = '\\';
  sp_mem_copy(nt.name.Buffer + w_off, wpat + 4, nt.name.Length - w_off * sizeof(u16));
  wpat[prefixed_len] = 0;
  sp_sys_nt_path_free(&nt);

  frame->handle = FindFirstFileW((LPCWSTR)wpat, &frame->find_data);
  if (frame->handle == INVALID_HANDLE_VALUE) return false;
  frame->first = true;
  return true;
}

void sp_sys_diriter_close(sp_fs_it_frame_t* frame) {
  FindClose(frame->handle);
}

bool sp_sys_diriter_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  while (true) {
    if (frame->first) {
      frame->first = false;
    } else {
      if (!FindNextFileW(frame->handle, &frame->find_data)) return false;
    }
    u16* name = (u16*)frame->find_data.cFileName;
    if (sp_win32_find_is_dot(name)) continue;
    entry->name = sp_wtf16_to_wtf8((sp_wide_str_t) { .data = name, .len = sp_win32_find_name_len(name) });
    entry->kind = sp_fs_it_win32_attrs(frame->find_data.dwFileAttributes);
    return true;
  }
}

#elif defined(SP_LINUX)
SP_PRIVATE sp_fs_kind_t sp_fs_it_dtype_to_attr(u8 d_type) {
  switch (d_type) {
    case SP_DT_REG: { return SP_FS_KIND_FILE; }
    case SP_DT_DIR: { return SP_FS_KIND_DIR; }
    case SP_DT_LNK: { return SP_FS_KIND_SYMLINK; }
  }
  return SP_FS_KIND_NONE;
}

bool sp_sys_diriter_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  frame->fd = sp_sys_open(sp_str_to_cstr(path), SP_O_RDONLY | SP_O_DIRECTORY, 0);
  return frame->fd >= 0;
}

void sp_sys_diriter_close(sp_fs_it_frame_t* frame) {
  sp_sys_close(frame->fd);
}

bool sp_sys_diriter_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  while (true) {
    if (frame->buf_pos < frame->buf_end) {
      sp_sys_dirent64_t* d = (sp_sys_dirent64_t*)(frame->buf + frame->buf_pos);
      frame->buf_pos += d->d_reclen;
      if (sp_fs_it_is_dot(d->d_name)) continue;
      entry->name = sp_str_from_cstr(d->d_name);
      entry->kind = sp_fs_it_dtype_to_attr(d->d_type);
      return true;
    }
    s64 n = sp_sys_getdents64(frame->fd, frame->buf, SP_FS_IT_BUF_SIZE);
    if (n <= 0) return false;
    frame->buf_pos = 0;
    frame->buf_end = (s32)n;
  }
}

#elif defined(SP_MACOS) || defined(SP_COSMO)
SP_PRIVATE sp_fs_kind_t sp_fs_it_dtype_to_attr(u8 d_type) {
  switch (d_type) {
    case SP_DT_REG: { return SP_FS_KIND_FILE; }
    case SP_DT_DIR: { return SP_FS_KIND_DIR; }
    case SP_DT_LNK: { return SP_FS_KIND_SYMLINK; }
  }
  return SP_FS_KIND_NONE;
}

bool sp_sys_diriter_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  frame->dir = opendir(sp_str_to_cstr(path));
  return frame->dir != SP_NULLPTR;
}

void sp_sys_diriter_close(sp_fs_it_frame_t* frame) {
  closedir(frame->dir);
}

bool sp_sys_diriter_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  while (true) {
    struct dirent* d = readdir(frame->dir);
    if (!d) return false;
    if (sp_fs_it_is_dot(d->d_name)) continue;
    entry->name = sp_str_from_cstr(d->d_name);
    entry->kind = sp_fs_it_dtype_to_attr(d->d_type);
    return true;
  }
}

#elif defined(SP_WASM)
bool sp_sys_diriter_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  sp_unreachable_return(false);
}

void sp_sys_diriter_close(sp_fs_it_frame_t* frame) {
  sp_unreachable();
}

bool sp_sys_diriter_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  sp_unreachable_return(false);
}

#else
#error "sp_sys_diriter_open"
#error "sp_sys_diriter_close"
#error "sp_sys_diriter_read"
#endif


//     ███████     █████████
//   ███░░░░░███  ███░░░░░███
//  ███     ░░███░███    ░░░
// ░███      ░███░░█████████
// ░███      ░███ ░░░░░░░░███
// ░░███     ███  ███    ░███
//  ░░░███████░  ░░█████████
//    ░░░░░░░     ░░░░░░░░░
// @os
///////////
// SLEEP //
///////////
void sp_os_sleep_ns(u64 ns) {
  sp_sys_timespec_t req = {
    .tv_sec  = (s64)(ns / 1000000000ULL),
    .tv_nsec = (s64)(ns % 1000000000ULL),
  };
  sp_sys_timespec_t rem = SP_ZERO_INITIALIZE();
  sp_sys_nanosleep(&req, &rem);
  // while (sp_sys_nanosleep(&req, &rem) == -1 && errno == SP_EINTR) {
  //   req = rem;
  // }
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


//////////////////
// SP_OS_PRINT //
/////////////////
#if defined(SP_WIN32)
SP_PRIVATE void sp_os_win32_write(DWORD handle_id, sp_str_t message) {
  HANDLE handle = GetStdHandle(handle_id);
  DWORD written;
  DWORD mode;
  if (GetConsoleMode(handle, &mode)) {
    WriteConsoleA(handle, message.data, message.len, &written, NULL);
  } else {
    WriteFile(handle, message.data, message.len, &written, NULL);
  }
}

void sp_os_print(sp_str_t message) {
  sp_os_win32_write(STD_OUTPUT_HANDLE, message);
}

void sp_os_print_err(sp_str_t message) {
  sp_os_win32_write(STD_ERROR_HANDLE, message);
}

#else
void sp_os_print(sp_str_t message) {
  sp_sys_write(sp_sys_stdout, message.data, message.len);
}

void sp_os_print_err(sp_str_t message) {
  sp_sys_write(sp_sys_stderr, message.data, message.len);
}
#endif


/////////
// TTY //
/////////
#if defined(SP_WIN32)
bool sp_os_is_tty(sp_sys_fd_t fd) {
  HANDLE handle = (HANDLE)fd;
  if (handle == INVALID_HANDLE_VALUE || handle == SP_NULLPTR) return false;
  DWORD mode;
  return GetConsoleMode(handle, &mode) != 0;
}

void sp_os_tty_size(sp_sys_fd_t fd, u32* cols, u32* rows) {
  if (cols) *cols = 0;
  if (rows) *rows = 0;
  HANDLE handle = (HANDLE)fd;
  if (handle == INVALID_HANDLE_VALUE || handle == SP_NULLPTR) return;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(handle, &csbi)) return;
  if (cols) *cols = (u32)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
  if (rows) *rows = (u32)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

s32 sp_os_tty_enter_raw(sp_sys_fd_t fd, sp_tty_mode_t* saved) {
  HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
  (void)fd;
  if (!GetConsoleMode(hin, &saved->input_mode)) return -1;
  if (!GetConsoleMode(hout, &saved->output_mode)) return -1;
  SetConsoleOutputCP(CP_UTF8);
  DWORD raw_in = ENABLE_VIRTUAL_TERMINAL_INPUT;
  DWORD raw_out = saved->output_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hin, raw_in)) return -1;
  if (!SetConsoleMode(hout, raw_out)) return -1;
  return 0;
}

s32 sp_os_tty_restore(sp_sys_fd_t fd, const sp_tty_mode_t* saved) {
  (void)fd;
  if (!SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), saved->input_mode)) return -1;
  if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), saved->output_mode)) return -1;
  return 0;
}

#elif defined(SP_MACOS) || defined(SP_COSMO) || defined(SP_LINUX)
#if defined(SP_LINUX)
  typedef sp_sys_termios_t sp_termios_t;
  typedef sp_sys_winsize_t sp_winsize_t;
  #define sp_tcgetattr(f, t) sp_sys_tcgetattr(f, t)
  #define sp_tcsetattr(f, o, t) sp_sys_tcsetattr(f, o, t)
  #define sp_ioctl(f, r, a) sp_sys_ioctl(f, r, a)

  s32 sp_sys_ioctl(s32 fd, u64 request, void* argp) {
    return (s32)sp_syscall(SP_SYSCALL_NUM_IOCTL, fd, request, argp);
  }

  s32 sp_sys_tcgetattr(s32 fd, sp_sys_termios_t* termios) {
    s32 result = sp_sys_ioctl(fd, SP_TCGETS, termios);
    return result < 0 ? -1 : result;
  }

  s32 sp_sys_tcsetattr(s32 fd, s32 opt, const sp_sys_termios_t* termios) {
    s32 result = sp_sys_ioctl(fd, SP_TCSETS + (u64)opt, (void*)termios);
    return result < 0 ? -1 : result;
  }
#else
  typedef struct termios sp_termios_t;
  typedef struct winsize sp_winsize_t;
  #define sp_tcgetattr(f, t) tcgetattr(f, t)
  #define sp_tcsetattr(f, o, t) tcsetattr(f, o, t)
  #define sp_ioctl(f, r, a) ioctl(f, r, a)
#endif

bool sp_os_is_tty(sp_sys_fd_t fd) {
  sp_termios_t t = sp_zero();
  return sp_tcgetattr(fd, &t) == 0;
}

void sp_os_tty_size(sp_sys_fd_t fd, u32* cols, u32* rows) {
  if (cols) *cols = 0;
  if (rows) *rows = 0;
  sp_winsize_t ws = sp_zero();
  if (sp_ioctl(fd, SP_TIOCGWINSZ, &ws) < 0) return;
  if (cols) *cols = (u32)ws.ws_col;
  if (rows) *rows = (u32)ws.ws_row;
}

s32 sp_os_tty_enter_raw(sp_sys_fd_t fd, sp_tty_mode_t* saved) {
  if (sp_tcgetattr(fd, saved) == -1) return -1;
  sp_termios_t raw = *saved;
  raw.c_iflag &= (u32)~(SP_BRKINT | SP_ICRNL | SP_INPCK | SP_ISTRIP | SP_IXON);
  raw.c_oflag &= (u32)~(SP_OPOST);
  raw.c_cflag |= (u32)SP_CS8;
  raw.c_lflag &= (u32)~(SP_ECHO | SP_ICANON | SP_IEXTEN | SP_ISIG);
  raw.c_cc[SP_VMIN] = 1;
  raw.c_cc[SP_VTIME] = 0;
  return sp_tcsetattr(fd, SP_TCSAFLUSH, &raw);
}

s32 sp_os_tty_restore(sp_sys_fd_t fd, const sp_tty_mode_t* saved) {
  return sp_tcsetattr(fd, SP_TCSAFLUSH, saved);
}

#elif defined(SP_WASM)
bool sp_os_is_tty(sp_sys_fd_t fd)                                  { return false; }
void sp_os_tty_size(sp_sys_fd_t fd, u32* cols, u32* rows)          { *cols = 0; *rows = 0; }
s32  sp_os_tty_enter_raw(sp_sys_fd_t fd, sp_tty_mode_t* saved)     { return 0; }
s32  sp_os_tty_restore(sp_sys_fd_t fd, const sp_tty_mode_t* saved) { return 0; }

#else
#error "sp_os_is_tty"
#error "sp_os_tty_size"
#error "sp_os_tty_enter_raw"
#error "sp_os_tty_restore"
#endif


///////////
// ENUMS //
///////////
#if defined(SP_WIN32)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_WIN32;
}

sp_str_t sp_os_get_executable_ext() {
  return sp_str_lit("exe");
}

sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) {
  switch (kind) {
    case SP_OS_LIB_SHARED: return SP_LIT("dll");
    case SP_OS_LIB_STATIC: return SP_LIT("lib");
  }

  SP_UNREACHABLE_RETURN(SP_LIT(""));
}

sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
  return sp_fmt_a(sp_context_get_allocator(), "{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind))).value;
}
#endif

#if defined(SP_MACOS)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_MACOS;
}

sp_str_t sp_os_get_executable_ext() {
  return sp_str_lit("");
}

sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) {
  switch (kind) {
    case SP_OS_LIB_SHARED: return SP_LIT("dylib");
    case SP_OS_LIB_STATIC: return SP_LIT("a");
  }

  SP_UNREACHABLE();
}

sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
  return sp_fmt_a(sp_context_get_allocator(), "lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind))).value;
}
#endif

#if defined(SP_LINUX)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_LINUX;
}

sp_str_t sp_os_get_executable_ext() {
  return sp_str_lit("");
}

sp_str_t sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind) {
  switch (kind) {
    case SP_OS_LIB_SHARED: return SP_LIT("so");
    case SP_OS_LIB_STATIC: return SP_LIT("a");
  }

  SP_UNREACHABLE_RETURN(sp_str_lit(""));
}

sp_str_t sp_os_lib_to_file_name(sp_str_t lib_name, sp_os_lib_kind_t kind) {
  return sp_fmt_a(sp_context_get_allocator(), "lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind))).value;
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

sp_str_t sp_os_get_executable_ext() {
  switch (sp_os_get_kind()) {
  case SP_OS_LINUX: return sp_str_lit("");
  case SP_OS_WIN32: return sp_str_lit("exe");
  case SP_OS_MACOS: return sp_str_lit("");
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
  return sp_fmt_a(sp_context_get_allocator(), "lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind))).value;
}
#endif


sp_err_t sp_os_create_dir(sp_str_t path) {
  return sp_sys_mkdir(sp_str_to_cstr(path), 0755) == 0 ? SP_OK : SP_ERR_OS;
}

sp_str_t sp_os_get_cwd() {
  c8 path[SP_PATH_MAX] = SP_ZERO_INITIALIZE();
  if (sp_sys_getcwd(path, SP_PATH_MAX - 1) < 0) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  return sp_str_from_cstr(path);
}

sp_err_t sp_os_create_file(sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  sp_sys_fd_t fd = sp_sys_open(sp_str_to_cstr(path), SP_O_CREAT | SP_O_WRONLY | SP_O_TRUNC, 0644);
  if (fd != SP_SYS_INVALID_FD) {
    sp_sys_close(fd);
  }

  sp_mem_end_scratch(scratch);
  return fd != SP_SYS_INVALID_FD ? SP_OK : SP_ERR_OS;
}

sp_err_t sp_os_create_hard_link(sp_str_t target, sp_str_t link_path) {
  if (sp_sys_link(sp_str_to_cstr(target), sp_str_to_cstr(link_path))) {
    return SP_ERR_OS;
  }
  return SP_OK;
}

sp_err_t sp_os_create_sym_link(sp_str_t target, sp_str_t link_path) {
  if (sp_sys_symlink(sp_str_to_cstr(target), sp_str_to_cstr(link_path)) != 0) {
    return SP_ERR_OS;
  }
  return SP_OK;
}

////////////
// SIGNAL //
////////////
#if defined(SP_WIN32)
  #define SP_SYS_SIGNAL_INTERRUPT 2
  #define SP_SYS_SIGNAL_TERMINATE 15
  #define SP_SYS_SIGNAL_ABORT 22
#else
  #define SP_SYS_SIGNAL_INTERRUPT 2
  #define SP_SYS_SIGNAL_TERMINATE 15
  #define SP_SYS_SIGNAL_ABORT 6
#endif

SP_PRIVATE void sp_os_signal_from_native(s32 sig) {
  sp_os_signal_t s;
  switch (sig) {
    case SP_SYS_SIGNAL_INTERRUPT: s = SP_OS_SIGNAL_INTERRUPT; break;
    case SP_SYS_SIGNAL_TERMINATE: s = SP_OS_SIGNAL_TERMINATE; break;
    case SP_SYS_SIGNAL_ABORT:     s = SP_OS_SIGNAL_ABORT;     break;
    default: return;
  }
  if (sp_rt.signal_handlers[s]) {
    sp_rt.signal_handlers[s](s, sp_rt.signal_userdata[s]);
  }
}

SP_PRIVATE s32 sp_os_signal_to_native(sp_os_signal_t signal) {
  switch (signal) {
    case SP_OS_SIGNAL_INTERRUPT: return SP_SYS_SIGNAL_INTERRUPT;
    case SP_OS_SIGNAL_TERMINATE: return SP_SYS_SIGNAL_TERMINATE;
    case SP_OS_SIGNAL_ABORT:     return SP_SYS_SIGNAL_ABORT;
  }
  return -1;
}

#if defined(SP_WIN32)
SP_PRIVATE BOOL WINAPI sp_os_signal_console_ctrl(DWORD type) {
  switch (type) {
    case CTRL_C_EVENT:     sp_os_signal_from_native(SP_SYS_SIGNAL_INTERRUPT); return TRUE;
    case CTRL_CLOSE_EVENT: sp_os_signal_from_native(SP_SYS_SIGNAL_TERMINATE); return TRUE;
    default: return FALSE;
  }
}

void sp_os_register_signal_handler(sp_os_signal_t sig, sp_os_signal_handler_t handler, void* userdata) {
  sp_rt.signal_handlers[sig] = handler;
  sp_rt.signal_userdata[sig] = userdata;
  if (sig == SP_OS_SIGNAL_ABORT) {
    signal(sp_os_signal_to_native(sig), sp_os_signal_from_native);
  } else {
    SetConsoleCtrlHandler(sp_os_signal_console_ctrl, TRUE);
  }
}

#elif defined(SP_FREESTANDING)
__attribute__((naked)) SP_PRIVATE void sp_os_signal_restorer(void) {
#if defined(SP_AMD64)
  __asm__ __volatile__ (
    "mov $15, %%rax\n"  /* __NR_rt_sigreturn */
    "syscall"
    ::: "rcx", "r11", "memory"
  );
#elif defined(SP_ARM64)
  __asm__ __volatile__ (
    "mov x8, #139\n"  /* __NR_rt_sigreturn */
    "svc #0"
    ::: "memory"
  );
#endif
}

void sp_os_register_signal_handler(sp_os_signal_t signal, sp_os_signal_handler_t handler, void* userdata) {
  sp_rt.signal_handlers[signal] = handler;
  sp_rt.signal_userdata[signal] = userdata;
  s32 sig = sp_os_signal_to_native(signal);
  /* kernel_sigaction for x86_64 */
  struct {
    void (*handler)(int);
    u64   flags;
    void (*restorer)(void);
    u64   mask;
  } sa;
  sp_mem_zero(&sa, sizeof(sa));
  sa.handler  = sp_os_signal_from_native;
  sa.flags    = 0x04000000; /* SA_RESTORER */
  sa.restorer = sp_os_signal_restorer;
  sp_syscall(SP_SYSCALL_NUM_SIGACTION, sig, &sa, 0, 8);
}

#elif defined(SP_POSIX)
void sp_os_register_signal_handler(sp_os_signal_t signal, sp_os_signal_handler_t handler, void* userdata) {
  sp_rt.signal_handlers[signal] = handler;
  sp_rt.signal_userdata[signal] = userdata;
  struct sigaction sa;
  sa.sa_handler = sp_os_signal_from_native;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(sp_os_signal_to_native(signal), &sa, SP_NULLPTR);
}

#else
void sp_os_register_signal_handler(sp_os_signal_t signal, sp_os_signal_handler_t handler, void* userdata) {
  (void)signal;
  (void)handler;
  (void)userdata;
}

#endif

#if defined(SP_LINUX) || defined(SP_WASM)
void sp_os_qsort(void *arr, u64 len, u64 stride, sp_qsort_fn_t cmp) {
  u8 *a = arr;
  u64 gap, i, j;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  u8* tmp = sp_alloc_n(u8, stride);

  for (gap = len / 3; gap > 0; gap /= 3 + 1) {
    for (i = gap; i < len; i++) {
      sp_sys_memcpy(tmp, a + i * stride, stride);
      for (j = i; j >= gap && cmp(a + (j - gap) * stride, tmp) > 0; j -= gap) {
        sp_sys_memcpy(a + j * stride, a + (j - gap) * stride, stride);
      }
      sp_sys_memcpy(a + j * stride, tmp, stride);
    }
  }

  sp_mem_end_scratch(scratch);
}
#else
void sp_os_qsort(void *arr, u64 len, u64 stride, sp_qsort_fn_t cmp) {
  qsort(arr, len, stride, cmp);
}

#endif


//  ██████████ ██████   █████ █████   █████
// ░░███░░░░░█░░██████ ░░███ ░░███   ░░███
//  ░███  █ ░  ░███░███ ░███  ░███    ░███
//  ░██████    ░███░░███░███  ░███    ░███
//  ░███░░█    ░███ ░░██████  ░░███   ███
//  ░███ ░   █ ░███  ░░█████   ░░░█████░
//  ██████████ █████  ░░█████    ░░███
// ░░░░░░░░░░ ░░░░░    ░░░░░      ░░░
// @env
#if defined(SP_WIN32)
typedef struct {
  u8* base;
  struct {
    u8* base;
    u16* env;
  } params;
} sp_win32_peb_t;

SP_PRIVATE sp_win32_peb_t sp_win32_get_peb() {
  sp_win32_peb_t peb = SP_ZERO_INITIALIZE();

  #if defined(_MSC_VER)
    static const s32 tpidr_el0 = 0x5E82;
    static const u32 peb_offset = 0x60;

    #if defined(_M_X64)
      peb.base = (u8*)__readgsqword(peb_offset);
    #elif defined(_M_ARM64)
      peb.base = *(u8**)(_ReadStatusReg(tpidr_el0) + peb_offset);
    #else
      #error "unsupported architecture"
    #endif
  #else
    #if defined(__x86_64__)
      __asm__ volatile ("movq %%gs:0x60, %0" : "=r"(peb.base));
    #elif defined(__i386__)
      __asm__ volatile ("movl %%fs:0x30, %0" : "=r"(peb.base));
    #elif defined(__aarch64__)
      __asm__ volatile ("ldr %0, [x18, #0x60]" : "=r"(peb.base));
    #else
      #error "unsupported architecture"
    #endif
  #endif

  // PEB->ProcessParameters
  peb.params.base = *(u8**)(peb.base + 0x20);

  // RTL_USER_PROCESS_PARAMETERS->Environment
  peb.params.env = *(u16**)(peb.params.base + 0x80);

  return peb;
}

SP_PRIVATE sp_str_t sp_win32_utf16_to_utf8(const u16* utf16, s32 len) {
  s32 num_bytes = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)utf16, len, NULL, 0, NULL, NULL);
  c8* utf8 = sp_alloc_n(c8, num_bytes + 1);
  WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)utf16, len, utf8, num_bytes, NULL, NULL);
  utf8[num_bytes] = '\0';
  return sp_rval(sp_str_t) {
    .data = utf8,
    .len = num_bytes
  };
}

SP_PRIVATE u32 sp_win32_utf16_len(const u16* str) {
  u32 len = 0;
  while (str[len]) len++;
  return len;
}

SP_PRIVATE bool sp_win32_utf16_equals_cstr(const u16* a, u32 a_len, const c8* b, u32 b_len) {
  if (a_len != b_len) return false;
  for (u32 i = 0; i < a_len; i++) {
    u16 wa = a[i];
    u16 wb = (u16)(u8)b[i];
    if (wa >= 'A' && wa <= 'Z') wa += 32;
    if (wb >= 'A' && wb <= 'Z') wb += 32;
    if (wa != wb) return false;
  }
  return true;
}

sp_str_t sp_os_env_get(sp_str_t key) {
  sp_win32_peb_t peb = sp_win32_get_peb();

  u16* p = peb.params.env;
  while (*p) {
    u32 eq = (*p == L'=') ? 1 : 0;
    while (p[eq] && p[eq] != L'=') eq++;

    if (sp_win32_utf16_equals_cstr(p, eq, key.data, key.len)) {
      u16* value = p + eq + 1;
      u32 len = sp_win32_utf16_len(value);
      return sp_win32_utf16_to_utf8(value, len);
    }

    p += sp_win32_utf16_len(p) + 1;
  }

  return SP_ZERO_STRUCT(sp_str_t);
}

SP_PRIVATE sp_env_var_t sp_os_env_parse_var(sp_str_t entry) {
  if (sp_str_empty(entry)) return sp_zero_struct(sp_env_var_t);

  sp_env_var_t result = { .key = entry };
  u32 start = entry.data[0] == '=' ? 1 : 0;

  sp_for_range(i, start, entry.len) {
    if (entry.data[i] == '=') {
      result.key = sp_str_sub(entry, 0, i);
      result.value = sp_str_sub(entry, i + 1, entry.len - i - 1);
      break;
    }
  }

  return result;
}

typedef struct {
  u16* block;
  u16* cursor;
  sp_str_t entry;
} sp_win32_env_it_t;

SP_PRIVATE void sp_win32_env_it_set_current(sp_os_env_it_t* it) {
  sp_win32_env_it_t* state = (sp_win32_env_it_t*)it->os;
  state->entry = sp_win32_utf16_to_utf8(state->cursor, sp_win32_utf16_len(state->cursor));

  sp_env_var_t var = sp_os_env_parse_var(state->entry);
  it->key = var.key;
  it->value = var.value;
}

sp_os_env_it_t sp_os_env_it_begin() {
  sp_os_env_it_t it = SP_ZERO_INITIALIZE();
  sp_win32_env_it_t* state = sp_alloc_type(sp_win32_env_it_t);
  state->block = sp_win32_get_peb().params.env;
  state->cursor = state->block;
  it.os = state;

  if (state->block && state->cursor[0] != L'\0') {
    sp_win32_env_it_set_current(&it);
  }
  return it;
}

bool sp_os_env_it_valid(sp_os_env_it_t* it) {
  sp_win32_env_it_t* state = (sp_win32_env_it_t*)it->os;
  return state && state->cursor && state->cursor[0] != L'\0';
}

void sp_os_env_it_next(sp_os_env_it_t* it) {
  sp_win32_env_it_t* state = (sp_win32_env_it_t*)it->os;
  state->cursor += wcslen(state->cursor) + 1;
  if (state->cursor[0] != L'\0') {
    sp_win32_env_it_set_current(it);
  } else {
    sp_free(state);
    *it = SP_ZERO_STRUCT(sp_os_env_it_t);
  }
}

#elif defined(SP_FREESTANDING)
typedef struct {
  sp_env_t* env;
  sp_ht_it_t it;
} sp_linux_env_it_t;

SP_PRIVATE void sp_linux_env_it_set_current(sp_os_env_it_t* it) {
  sp_linux_env_it_t* state = (sp_linux_env_it_t*)it->os;
  it->key = *sp_str_ht_it_getkp(state->env->vars, state->it);
  it->value = *sp_str_ht_it_getp(state->env->vars, state->it);
}

sp_str_t sp_os_env_get(sp_str_t key) {
  sp_tls_rt_t* state = sp_tls_rt_get();
  sp_str_t* value = sp_str_ht_get(state->env.vars, key);
  return value ? *value : SP_ZERO_STRUCT(sp_str_t);
}

sp_os_env_it_t sp_os_env_it_begin() {
  sp_os_env_it_t it = SP_ZERO_INITIALIZE();
  sp_tls_rt_t* runtime = sp_tls_rt_get();
  if (!runtime->env.vars) {
    return it;
  }

  sp_linux_env_it_t* state = (sp_linux_env_it_t*)sp_alloc(sizeof(sp_linux_env_it_t));
  *state = SP_ZERO_STRUCT(sp_linux_env_it_t);
  state->env = &runtime->env;
  state->it = sp_str_ht_it_init(runtime->env.vars);

  if (!sp_str_ht_it_valid(runtime->env.vars, state->it)) {
    sp_free(state);
    return it;
  }

  it.os = state;
  sp_linux_env_it_set_current(&it);
  return it;
}

bool sp_os_env_it_valid(sp_os_env_it_t* it) {
  sp_linux_env_it_t* state = (sp_linux_env_it_t*)it->os;
  return state && sp_str_ht_it_valid(state->env->vars, state->it);
}

void sp_os_env_it_next(sp_os_env_it_t* it) {
  sp_linux_env_it_t* state = (sp_linux_env_it_t*)it->os;
  sp_str_ht_it_advance(state->env->vars, state->it);

  if (sp_str_ht_it_valid(state->env->vars, state->it)) {
    sp_linux_env_it_set_current(it);
  } else {
    it->key = SP_ZERO_STRUCT(sp_str_t);
    it->value = SP_ZERO_STRUCT(sp_str_t);
    sp_free(state);
    it->os = SP_NULLPTR;
  }
}

#elif defined(SP_MACOS) || defined(SP_COSMO) || defined(SP_LINUX) || defined(SP_WASM)
extern c8** environ;

SP_PRIVATE bool sp_os_env_key_equal(sp_str_t a, sp_str_t b) {
#if defined(SP_COSMO)
  if (IsWindows()) {
    if (a.len != b.len) return false;
    for (u32 i = 0; i < a.len; i++) {
      c8 ca = a.data[i];
      c8 cb = b.data[i];
      if (ca >= 'A' && ca <= 'Z') ca += 32;
      if (cb >= 'A' && cb <= 'Z') cb += 32;
      if (ca != cb) return false;
    }
    return true;
  }
#endif
  return sp_str_equal(a, b);
}

sp_str_t sp_os_env_get(sp_str_t key) {
  for (c8** envp = environ; envp && *envp; envp++) {
    sp_str_pair_t pair = sp_str_cleave_c8(sp_str_view(*envp), '=');
    if (sp_os_env_key_equal(pair.first, key)) {
      return pair.second;
    }
  }
  return SP_ZERO_STRUCT(sp_str_t);
}

SP_PRIVATE void sp_os_env_it_set(sp_os_env_it_t* it) {
  c8** envp = (c8**)it->os;
  sp_str_t entry = sp_str_view(*envp);
  u32 offset = (entry.len > 0 && entry.data[0] == '=') ? 1 : 0;
  sp_str_t scan = { .data = entry.data + offset, .len = entry.len - offset };
  sp_str_pair_t pair = sp_str_cleave_c8(scan, '=');
  it->key = (sp_str_t) { .data = entry.data, .len = pair.first.len + offset };
  it->value = pair.second;
}

sp_os_env_it_t sp_os_env_it_begin() {
  sp_os_env_it_t it = SP_ZERO_INITIALIZE();
  it.os = environ;

  if (sp_os_env_it_valid(&it)) {
    sp_os_env_it_set(&it);
  }

  return it;
}

bool sp_os_env_it_valid(sp_os_env_it_t* it) {
  c8** envp = (c8**)it->os;
  return envp && *envp;
}

void sp_os_env_it_next(sp_os_env_it_t* it) {
  c8** envp = (c8**)it->os;
  envp++;

  if (envp && *envp) {
    it->os = envp;
    sp_os_env_it_set(it);
  } else {
    it->key = SP_ZERO_STRUCT(sp_str_t);
    it->value = SP_ZERO_STRUCT(sp_str_t);
    it->os = SP_NULLPTR;
  }
}

#else
#error "sp_os_env_it_begin"
#error "sp_os_env_it_valid"
#error "sp_os_env_it_next"
#endif

void sp_env_init(sp_env_t* env, sp_mem_t mem) {
  *env = SP_ZERO_STRUCT(sp_env_t);
  sp_str_ht_init_a(mem, env->vars);
}

sp_env_t sp_env_capture(sp_mem_t mem) {
  sp_env_t env = SP_ZERO_INITIALIZE();
  sp_str_ht_init_a(mem, env.vars);
  for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
    sp_str_ht_insert(env.vars, sp_str_copy(it.key), sp_str_copy(it.value));
  }

  return env;
}

sp_env_t sp_env_copy(sp_env_t* env, sp_mem_t mem) {
  sp_env_t copy = SP_ZERO_INITIALIZE();
  sp_str_ht_init_a(mem, copy.vars);

  sp_str_ht_for(env->vars, it) {
    sp_str_t key = *sp_str_ht_it_getkp(env->vars, it);
    sp_str_t val = *sp_str_ht_it_getp(env->vars, it);
    sp_env_insert(&copy, key, val);
  }

  return copy;
}

u32 sp_env_count(sp_env_t* env) {
  return (u32)sp_str_ht_size(env->vars);
}

sp_str_t sp_env_get(sp_env_t* env, sp_str_t name) {
  sp_str_t* val = sp_str_ht_get(env->vars, name);
  return val ? *val : SP_ZERO_STRUCT(sp_str_t);
}

bool sp_env_contains(sp_env_t* env, sp_str_t name) {
  return sp_str_ht_get(env->vars, name);
}

void sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value) {
  sp_str_ht_insert(env->vars, name, value);
}

void sp_env_erase(sp_env_t* env, sp_str_t name) {
  sp_str_ht_erase(env->vars, name);
}

void sp_env_destroy(sp_env_t* env) {
  sp_str_ht_free(env->vars);
  env->vars = SP_NULLPTR;
}




// ████████╗██╗███╗   ███╗███████╗
// ╚══██╔══╝██║████╗ ████║██╔════╝
//    ██║   ██║██╔████╔██║█████╗
//    ██║   ██║██║╚██╔╝██║██╔══╝
//    ██║   ██║██║ ╚═╝ ██║███████╗
//    ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝
// @time
sp_tm_point_t sp_tm_now_point() {
  sp_sys_timespec_t ts;
  sp_sys_clock_gettime(SP_CLOCK_MONOTONIC, &ts);
  return (sp_tm_point_t)((u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec);
}

sp_tm_epoch_t sp_tm_now_epoch() {
  sp_sys_timespec_t ts;
  sp_sys_clock_gettime(SP_CLOCK_REALTIME, &ts);
  return SP_RVAL(sp_tm_epoch_t) {
    .s  = (u64)ts.tv_sec,
    .ns = (u32)ts.tv_nsec,
  };
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

sp_tm_date_time_t sp_tm_epoch_to_date_time(sp_tm_epoch_t time) {
  u64 epoch = time.s;
  u32 sec = (u32)(epoch % 60); epoch /= 60;
  u32 min = (u32)(epoch % 60); epoch /= 60;
  u32 hour = (u32)(epoch % 24); epoch /= 24;

  s64 days = (s64)epoch + 719468;
  s64 era = (days >= 0 ? days : days - 146096) / 146097;
  u32 doe = (u32)(days - era * 146097);
  u32 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  s64 y = (s64)yoe + era * 400;
  u32 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  u32 mp = (5 * doy + 2) / 153;
  u32 d = doy - (153 * mp + 2) / 5 + 1;
  u32 m = mp < 10 ? mp + 3 : mp - 9;
  if (m <= 2) y += 1;

  return SP_RVAL(sp_tm_date_time_t) {
    .year = (s32)y,
    .month = (s32)m,
    .day = (s32)d,
    .hour = (s32)hour,
    .minute = (s32)min,
    .second = (s32)sec,
    .millisecond = (s32)(time.ns / 1000000),
  };
}

#if defined(SP_FREESTANDING)
sp_tm_date_time_t sp_tm_get_date_time() {
  return sp_zero_struct(sp_tm_date_time_t);
}
#else
sp_tm_date_time_t sp_tm_get_date_time() {
  return sp_tm_epoch_to_date_time(sp_tm_now_epoch());
}
#endif

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


//  ███████████ █████   █████ ███████████   ██████████   █████████   ██████████   █████ ██████   █████   █████████
// ░█░░░███░░░█░░███   ░░███ ░░███░░░░░███ ░░███░░░░░█  ███░░░░░███ ░░███░░░░███ ░░███ ░░██████ ░░███   ███░░░░░███
// ░   ░███  ░  ░███    ░███  ░███    ░███  ░███  █ ░  ░███    ░███  ░███   ░░███ ░███  ░███░███ ░███  ███     ░░░
//     ░███     ░███████████  ░██████████   ░██████    ░███████████  ░███    ░███ ░███  ░███░░███░███ ░███
//     ░███     ░███░░░░░███  ░███░░░░░███  ░███░░█    ░███░░░░░███  ░███    ░███ ░███  ░███ ░░██████ ░███    █████
//     ░███     ░███    ░███  ░███    ░███  ░███ ░   █ ░███    ░███  ░███    ███  ░███  ░███  ░░█████ ░░███  ░░███
//     █████    █████   █████ █████   █████ ██████████ █████   █████ ██████████   █████ █████  ░░█████ ░░█████████
//    ░░░░░    ░░░░░   ░░░░░ ░░░░░   ░░░░░ ░░░░░░░░░░ ░░░░░   ░░░░░ ░░░░░░░░░░   ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░
// @threading @concurrency
// @spin
void sp_spin_pause() {
  #if defined(SP_AMD64)
    #if defined(SP_MSVC)
      _mm_pause();
    #elif defined(SP_TCC)
      volatile int x = 0; (void)x;
    #elif defined(SP_GNUC)
      __asm__ __volatile__("pause");
    #endif

  #elif defined(SP_ARM64)
    #if defined(SP_MSVC)
      __yield();
    #elif defined(SP_TCC)
      volatile int x = 0; (void)x;
    #elif defined(SP_GNUC)
      __asm__ __volatile__("yield");
    #endif
  #endif
}

bool sp_spin_try_lock(sp_spin_lock_t* lock) {
  #if defined(SP_GNUC)
    return __sync_lock_test_and_set(lock, 1) == 0;
  #elif defined(SP_MSVC)
    return _InterlockedExchange((LONG*)lock, 1) == 0;
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
  #if defined(SP_GNUC)
    __sync_lock_release(lock);
  #elif defined(SP_MSVC)
    _InterlockedExchange((LONG*)lock, 0);
  #else
    sp_mutex_lock(&sp_rt.mutex);
    *lock = 0;
    sp_mutex_unlock(&sp_rt.mutex);
  #endif
}

// @atomic
bool sp_atomic_s32_cas(sp_atomic_s32_t* value, s32 current, s32 desired) {
  #if defined(SP_MSVC)
    return _InterlockedCompareExchange((long*)value, desired, current) == current;
  #elif defined(SP_GNUC)
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

s32 sp_atomic_s32_set(sp_atomic_s32_t* value, s32 desired) {
  #if defined(SP_MSVC)
    return _InterlockedExchange((long*)value, desired);
  #elif defined(SP_GNUC)
    return __sync_lock_test_and_set(value, desired);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cas(value, old, desired));
    return old;
  #endif
}

s32 sp_atomic_s32_add(sp_atomic_s32_t* value, s32 add) {
  #if defined(SP_MSVC)
    return _InterlockedExchangeAdd((long*)value, add);
  #elif defined(SP_GNUC)
    return __sync_fetch_and_add(value, add);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cas(value, old, old + add));
    return old;
  #endif
}

s32 sp_atomic_s32_get(sp_atomic_s32_t* value) {
  #if defined(SP_MSVC)
    return _InterlockedOr((long*)value, 0);
  #elif defined(SP_GNUC)
    return __sync_or_and_fetch(value, 0);
  #else
    s32 old;
    do {
      old = *value;
    } while (!sp_atomic_s32_cas(value, old, old));
    return old;
  #endif
}

bool sp_atomic_ptr_cas(sp_atomic_ptr_t* value, void* current, void* desired) {
  #if defined(SP_MSVC)
    return _InterlockedCompareExchangePointer(value, desired, current) == current;
  #elif defined(SP_GNUC)
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

void* sp_atomic_ptr_set(sp_atomic_ptr_t* value, void* desired) {
  #if defined(SP_MSVC)
    return _InterlockedExchangePointer(value, desired);
  #elif defined(SP_GNUC)
    void* old;
    do {
      old = *value;
    } while (!__sync_bool_compare_and_swap(value, old, desired));
    return old;
  #else
    void* old;
    do {
      old = *value;
    } while (!sp_atomic_ptr_cas(value, old, desired));
    return old;
  #endif
}

void* sp_atomic_ptr_get(sp_atomic_ptr_t* value) {
  #if defined(SP_MSVC)
    return _InterlockedCompareExchangePointer(value, SP_NULLPTR, SP_NULLPTR);
  #elif defined(SP_GNUC)
    return __sync_val_compare_and_swap(value, SP_NULLPTR, SP_NULLPTR);
  #else
    void* old;
    do {
      old = *value;
    } while (!sp_atomic_ptr_cas(value, old, old));
    return old;
  #endif
}

// @semaphore
#if defined(SP_WIN32)
void sp_semaphore_init(sp_semaphore_t* semaphore) {
  *semaphore = CreateSemaphoreW(NULL, 0, 0x7FFFFFF, NULL);
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

#else
void sp_semaphore_init(sp_semaphore_t* semaphore) {
  SP_UNIMPLEMENTED();
}

void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
  SP_UNIMPLEMENTED();
}

void sp_semaphore_wait(sp_semaphore_t* semaphore) {
  SP_UNIMPLEMENTED();
}

bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
  SP_UNIMPLEMENTED();
  return true;
}

void sp_semaphore_signal(sp_semaphore_t* semaphore) {
  SP_UNIMPLEMENTED();
}
#endif

// @mutex
#if defined(SP_WIN32)
s32 sp_mutex_kind_to_c11(sp_mutex_kind_t kind) {
  return kind;
}

void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
  SP_UNUSED(kind);
  InitializeCriticalSection(mutex);
}

void sp_mutex_lock(sp_mutex_t* mutex) {
  EnterCriticalSection(mutex);
}

void sp_mutex_unlock(sp_mutex_t* mutex) {
  LeaveCriticalSection(mutex);
}

void sp_mutex_destroy(sp_mutex_t* mutex) {
  DeleteCriticalSection(mutex);
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

#else
void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
  SP_UNIMPLEMENTED();
}

void sp_mutex_lock(sp_mutex_t* mutex) {
  SP_UNIMPLEMENTED();
}

void sp_mutex_unlock(sp_mutex_t* mutex) {
  SP_UNIMPLEMENTED();
}

void sp_mutex_destroy(sp_mutex_t* mutex) {
  SP_UNIMPLEMENTED();
}

#endif

// @cv
#if defined(SP_WIN32)
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

#elif defined(SP_POSIX)
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

#else
void sp_cv_init(sp_cv_t* cond) {
  SP_UNIMPLEMENTED();
}

void sp_cv_destroy(sp_cv_t* cond) {
  SP_UNIMPLEMENTED();
}

void sp_cv_wait(sp_cv_t* cond, sp_mutex_t* mutex) {
  SP_UNIMPLEMENTED();
}

bool sp_cv_wait_for(sp_cv_t* cond, sp_mutex_t* mutex, u32 ms) {
  SP_UNIMPLEMENTED();
  return true;
}

void sp_cv_notify_one(sp_cv_t* cond) {
  SP_UNIMPLEMENTED();
}

void sp_cv_notify_all(sp_cv_t* cond) {
  SP_UNIMPLEMENTED();
}

#endif

// @thread
#if defined(SP_WIN32)
DWORD WINAPI sp_win32_thread_launch(LPVOID args) {
  s32 result = sp_thread_launch(args);
  sp_free(args);
  return (DWORD)result;
}

void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
  sp_thread_launch_t* launch = sp_alloc_type(sp_thread_launch_t);
  *launch = SP_RVAL(sp_thread_launch_t) {
    .fn = fn,
    .userdata = userdata,
    .semaphore = SP_ZERO_STRUCT(sp_semaphore_t)
  };
  sp_semaphore_init(&launch->semaphore);

  *thread = CreateThread(SP_NULLPTR, 0, sp_win32_thread_launch, launch, 0, SP_NULLPTR);
  if (!*thread) {
    sp_semaphore_destroy(&launch->semaphore);
    sp_free(launch);
    return;
  }

  sp_semaphore_wait(&launch->semaphore);
  sp_semaphore_destroy(&launch->semaphore);
}

s32 sp_thread_launch(void* args) {
  sp_thread_launch_t* launch = (sp_thread_launch_t*)args;
  void* userdata = launch->userdata;
  sp_thread_fn_t fn = launch->fn;
  sp_semaphore_signal(&launch->semaphore);
  return fn(userdata);
}

void sp_thread_join(sp_thread_t* thread) {
  if (!*thread) return;
  WaitForSingleObject(*thread, INFINITE);
  CloseHandle(*thread);
  *thread = SP_NULLPTR;
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

#elif defined(SP_WASM) || defined(SP_FREESTANDING)
s32 sp_thread_launch(void* args) {
  sp_thread_launch_t* launch = (sp_thread_launch_t*)args;
  void* userdata = launch->userdata;
  sp_thread_fn_t fn = launch->fn;

  sp_semaphore_signal(&launch->semaphore);
  s32 result = fn(userdata);

  return result;
}

void sp_thread_join(sp_thread_t* thread) {
  SP_UNIMPLEMENTED();
}

void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
  SP_UNIMPLEMENTED();
}

#else
#error "sp_thread_launch"
#error "sp_thread_join"
#error "sp_thread_init"
#endif


//  ███████████  ███████████      ███████      █████████  ██████████  █████████   █████████
// ░░███░░░░░███░░███░░░░░███   ███░░░░░███   ███░░░░░███░░███░░░░░█ ███░░░░░███ ███░░░░░███
//  ░███    ░███ ░███    ░███  ███     ░░███ ███     ░░░  ░███  █ ░ ░███    ░░░ ░███    ░░░
//  ░██████████  ░██████████  ░███      ░███░███          ░██████   ░░█████████ ░░█████████
//  ░███░░░░░░   ░███░░░░░███ ░███      ░███░███          ░███░░█    ░░░░░░░░███ ░░░░░░░░███
//  ░███         ░███    ░███ ░░███     ███ ░░███     ███ ░███ ░   █ ███    ░███ ███    ░███
//  █████        █████   █████ ░░░███████░   ░░█████████  ██████████░░█████████ ░░█████████
// ░░░░░        ░░░░░   ░░░░░    ░░░░░░░      ░░░░░░░░░  ░░░░░░░░░░  ░░░░░░░░░   ░░░░░░░░░
// @ps @process
#if defined(SP_POSIX)
typedef struct {
  posix_spawn_file_actions_t* fa;
  sp_ps_io_file_number_t file_number;
  s32 flag;
  s32 mode;
  struct {
    s32 read;
    s32 write;
  } pipes;
} sp_ps_stdio_config_entry_t;

typedef struct {
  sp_ps_stdio_config_entry_t in;
  sp_ps_stdio_config_entry_t out;
  sp_ps_stdio_config_entry_t err;
} sp_ps_stdio_config_t;

struct sp_ps_os {
  s32 pid;
};

SP_PRIVATE void sp_ps_set_cwd(posix_spawn_file_actions_t* fa, sp_str_t cwd);
SP_PRIVATE bool sp_ps_create_pipes(s32 pipes [2]);
SP_PRIVATE sp_da(c8*) sp_ps_build_posix_args(sp_ps_config_t* config);
SP_PRIVATE void sp_ps_free_posix_args(c8** argv);
SP_PRIVATE void sp_ps_set_nonblocking(s32 fd);
SP_PRIVATE void sp_ps_set_blocking(s32 fd);

SP_PRIVATE sp_env_t sp_ps_build_env(sp_ps_env_config_t* config, sp_mem_t mem) {
  sp_env_t env = SP_ZERO_INITIALIZE();

  switch (config->mode) {
    case SP_PS_ENV_INHERIT: {
      env = sp_env_capture(mem);
      break;
    }
    case SP_PS_ENV_EXISTING: {
      env = sp_env_copy(&config->env, mem);
      break;
    }
    case SP_PS_ENV_CLEAN: {
      sp_env_init(&env, mem);
    }
  }

  sp_for(i, SP_PS_MAX_ENV) {
    if (sp_str_empty(config->extra[i].key)) break;
    sp_env_insert(&env, config->extra[i].key, config->extra[i].value);
  }

  return env;
}

bool sp_ps_is_fd_valid(sp_sys_fd_t fd) {
  return fd > 0;
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

sp_da(c8*) sp_ps_build_posix_args(sp_ps_config_t* config) {
  sp_da(c8*) args = SP_NULLPTR;

  sp_da_push(args, sp_str_to_cstr(config->command));

  sp_carr_for(config->args, it) {
    sp_str_t arg = config->args[it];
    if (sp_str_empty(arg)) break;

    sp_da_push(args, sp_str_to_cstr(arg));
  }

  sp_da_for(config->dyn_args, it) {
    sp_da_push(args, sp_str_to_cstr(config->dyn_args[it]));
  }

  sp_da_push(args, SP_NULLPTR);
  return args;
}

void sp_ps_free_posix_args(c8** args) {
  sp_da_free(args);
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
  sp_da_for(src->dyn_args, i) {
    sp_da_push(dst.dyn_args, sp_str_copy(src->dyn_args[i]));
  }

  dst.env.mode = src->env.mode;
  sp_env_init(&dst.env.env, sp_context_get()->allocator);

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
    sp_da_push(config->dyn_args, arg);
  }
}

void sp_ps_configure_io_in(sp_ps_io_in_config_t* io, sp_ps_stdio_config_entry_t* p) {
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
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, p->pipes.read, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_EXISTING: {
      SP_ASSERT(io->fd);
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, io->fd, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_REDIRECT: {
      break;
    }
    case SP_PS_IO_MODE_INHERIT: {
      break;
    }
  }
}

void sp_ps_configure_io_out(sp_ps_io_out_config_t* io, sp_ps_stdio_config_entry_t* p) {
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
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, p->pipes.write, p->file_number) == 0);
      break;
    }
    case SP_PS_IO_MODE_EXISTING: {
      SP_ASSERT(io->fd);
      SP_ASSERT(posix_spawn_file_actions_adddup2(p->fa, io->fd, p->file_number) == 0);
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

c8** sp_env_to_posix_envp(sp_env_t* env) {
  sp_da(c8*) envp = SP_NULLPTR;

  sp_str_ht_for(env->vars, it) {
    sp_str_t key = *sp_str_ht_it_getkp(env->vars, it);
    sp_str_t val = *sp_str_ht_it_getp(env->vars, it);

    sp_str_t entry = sp_fmt_a(sp_context_get_allocator(), "{}={}", sp_fmt_str(key), sp_fmt_str(val)).value;
    sp_da_push(envp, sp_str_to_cstr(entry));
  }

  sp_da_push(envp, SP_NULLPTR);
  return envp;
}

void sp_env_free_posix_envp(c8** envp) {
  if (!envp) return;

  for (u32 i = 0; envp[i] != SP_NULLPTR; i++) {
    sp_free(envp[i]);
  }
  sp_da_free(envp);
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  sp_ps_t proc = SP_ZERO_STRUCT(sp_ps_t);
  proc.allocator = sp_context_get()->allocator;
  proc.io = config.io;

  SP_ASSERT(!sp_str_empty(config.command));

  c8** argv = sp_ps_build_posix_args(&config);
  sp_env_t env = sp_ps_build_env(&config.env, proc.allocator);
  c8** envp = sp_env_to_posix_envp(&env);

  posix_spawnattr_t attr;
  posix_spawn_file_actions_t fa;

  SP_ASSERT(posix_spawnattr_init(&attr) == 0);
  SP_ASSERT(posix_spawn_file_actions_init(&fa) == 0);

  if (!sp_str_empty(config.cwd)) {
    sp_ps_set_cwd(&fa, config.cwd);
  }

  sp_ps_stdio_config_t io = {
    .in = (sp_ps_stdio_config_entry_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDIN,
      .flag = O_RDONLY,
      .mode = 0x000,
      .pipes = { .read = -1, .write = -1 }
    },
    .out = (sp_ps_stdio_config_entry_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDOUT,
      .flag = O_WRONLY,
      .mode = 0x644,
      .pipes = { .read = -1, .write = -1 },
    },
    .err = (sp_ps_stdio_config_entry_t) {
      .fa = &fa,
      .file_number = SP_PS_IO_FILENO_STDERR,
      .flag = O_WRONLY,
      .mode = 0x644,
      .pipes = { .read = -1, .write = -1 },
    },
  };

  sp_ps_configure_io_in(&proc.io.in, &io.in);

  if (proc.io.out.mode == SP_PS_IO_MODE_REDIRECT) {
    sp_ps_configure_io_out(&proc.io.err, &io.err);
    sp_ps_configure_io_out(&proc.io.out, &io.out);
  }
  else {
    sp_ps_configure_io_out(&proc.io.out, &io.out);
    sp_ps_configure_io_out(&proc.io.err, &io.err);
  }

  pid_t pid;
  if (posix_spawnp(&pid, argv[0], &fa, &attr, argv, envp) != 0) {
    posix_spawn_file_actions_destroy(&fa);
    posix_spawnattr_destroy(&attr);
    sp_ps_free_posix_args(argv);
    sp_env_free_posix_envp(envp);
    sp_env_destroy(&env);
    if (io.in.pipes.read >= 0)  { sp_sys_close(io.in.pipes.read); sp_sys_close(io.in.pipes.write); }
    if (io.out.pipes.read >= 0) { sp_sys_close(io.out.pipes.read); sp_sys_close(io.out.pipes.write); }
    if (io.err.pipes.read >= 0) { sp_sys_close(io.err.pipes.read); sp_sys_close(io.err.pipes.write); }

    return SP_ZERO_STRUCT(sp_ps_t);
  }

  proc.os = sp_alloc_type(sp_ps_os_t);
  proc.os->pid = pid;

  if (io.in.pipes.read >= 0) {
    sp_sys_close(io.in.pipes.read);

    switch (config.io.in.block) {
      case SP_PS_IO_NONBLOCKING: { sp_ps_set_nonblocking(io.in.pipes.write); break; }
      case SP_PS_IO_BLOCKING: { sp_ps_set_blocking(io.in.pipes.write); break; }
    }
    proc.io.in.fd = io.in.pipes.write;
  }

  if (io.out.pipes.read >= 0) {
    sp_sys_close(io.out.pipes.write);

    switch (config.io.out.block) {
      case SP_PS_IO_NONBLOCKING: { sp_ps_set_nonblocking(io.out.pipes.read); break; }
      case SP_PS_IO_BLOCKING: { sp_ps_set_blocking(io.out.pipes.read); break; }
    }
    proc.io.out.fd = io.out.pipes.read;
  }

  if (io.err.pipes.read >= 0) {
    sp_sys_close(io.err.pipes.write);

    switch (config.io.err.block) {
      case SP_PS_IO_NONBLOCKING: { sp_ps_set_nonblocking(io.err.pipes.read); break; }
      case SP_PS_IO_BLOCKING:    { sp_ps_set_blocking(io.err.pipes.read);    break; }
    }
    proc.io.err.fd = io.err.pipes.read;
  }

  posix_spawn_file_actions_destroy(&fa);
  posix_spawnattr_destroy(&attr);
  sp_ps_free_posix_args(argv);
  sp_env_free_posix_envp(envp);
  sp_env_destroy(&env);

  return proc;
}

sp_ps_output_t sp_ps_run(sp_ps_config_t config) {
  if (config.io.out.mode == SP_PS_IO_MODE_EXISTING || config.io.out.mode == SP_PS_IO_MODE_REDIRECT) {
    return sp_zero_struct(sp_ps_output_t);
  }

  config.io.out = (sp_ps_io_out_config_t) {
    .mode = SP_PS_IO_MODE_CREATE
  };
  sp_ps_t ps = sp_ps_create(config);
  if (ps.os) return sp_ps_output(&ps);
  return (sp_ps_output_t) { .status = { .state = SP_PS_STATE_DONE, .exit_code = -1 } };
}

int posix_spawn_file_actions_addchdir_np(posix_spawn_file_actions_t*, const char*);

void sp_ps_set_cwd(posix_spawn_file_actions_t* fa, sp_str_t cwd) {
  const c8* cwd_cstr = sp_str_to_cstr(cwd);
  SP_ASSERT(posix_spawn_file_actions_addchdir_np(fa, cwd_cstr) == 0);
}

sp_io_close_mode_t sp_ps_io_close_mode(sp_ps_io_mode_t mode) {
  switch (mode) {
    case SP_PS_IO_MODE_CREATE: {
      return SP_IO_CLOSE_MODE_AUTO;
    }
    case SP_PS_IO_MODE_EXISTING: {
      return SP_IO_CLOSE_MODE_NONE;
    }
    case SP_PS_IO_MODE_NULL:
    case SP_PS_IO_MODE_INHERIT:
    case SP_PS_IO_MODE_REDIRECT: {
      return SP_IO_CLOSE_MODE_NONE;
    }
  }
  SP_UNREACHABLE_RETURN(SP_IO_CLOSE_MODE_NONE);
}

sp_io_writer_t* sp_ps_io_in(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.in.fd)) return SP_NULLPTR;

  sp_io_writer_t* writer = sp_alloc_type(sp_io_writer_t);
  sp_io_writer_from_fd(writer, ps->io.in.fd, sp_ps_io_close_mode(ps->io.in.mode));
  return writer;
}

sp_io_reader_t* sp_ps_io_out(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.out.fd)) return SP_NULLPTR;

  sp_io_reader_t* reader = sp_alloc_type(sp_io_reader_t);
  sp_io_reader_from_fd(reader, ps->io.out.fd, sp_ps_io_close_mode(ps->io.out.mode));
  return reader;
}

sp_io_reader_t* sp_ps_io_err(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.err.fd)) return SP_NULLPTR;

  sp_io_reader_t* reader = sp_alloc_type(sp_io_reader_t);
  sp_io_reader_from_fd(reader, ps->io.err.fd, sp_ps_io_close_mode(ps->io.err.mode));
  return reader;
}

#define SP_POSIX_WAITPID_NO_BLOCK SP_WNOHANG
#define SP_POSIX_WAITPID_BLOCK 0

#if defined(SP_LINUX)
  #define sp_wait4(p, s, o, r)              sp_sys_wait4(p, s, o, r)
#else
  #define sp_wait4(p, s, o, r)              wait4(p, s, o, r)
#endif

sp_ps_status_t sp_ps_poll(sp_ps_t* ps, u32 timeout_ms) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  if (!ps || !ps->os) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  s32 wait_status = 0;
  s32 wait_result = 0;
  s32 time_remaining = timeout_ms;

  do {
    wait_result = sp_wait4(ps->os->pid, &wait_status, SP_POSIX_WAITPID_NO_BLOCK, SP_NULLPTR);
    if (wait_result == 0) {
      result.state = SP_PS_STATE_RUNNING;
    }
    else if (wait_result > 0) {
      result.state = SP_PS_STATE_DONE;

      if (SP_WIFEXITED(wait_status)) {
        result.exit_code = SP_WEXITSTATUS(wait_status);
      }
      else if (SP_WIFSIGNALED(wait_status)) {
        result.exit_code = -1 * SP_WTERMSIG(wait_status);
      }
      else {
        result.exit_code = -255;
      }

      return result;
    }
    else if (wait_result < 0 && errno == SP_EINTR) {
      continue;
    }
    else if (wait_result < 0) {
      // @spader A real error; also SP_PS_STATE feels useless to me
      result.exit_code = -1;
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

  if (!ps || !ps->os) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  s32 wait_status = 0;
  s32 wait_result = 0;

  do {
    wait_result = sp_wait4(ps->os->pid, &wait_status, SP_POSIX_WAITPID_BLOCK, SP_NULLPTR);
  } while (wait_result == -1 && errno == SP_EINTR);

  if (wait_result < 0) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  result.state = SP_PS_STATE_DONE;

  if (SP_WIFEXITED(wait_status)) {
    result.exit_code = SP_WEXITSTATUS(wait_status);
  }
  else if (SP_WIFSIGNALED(wait_status)) {
    result.exit_code = -1 * SP_WTERMSIG(wait_status);
  }
  else {
    result.exit_code = -255;
  }

  return result;
}

sp_ps_output_t sp_ps_output(sp_ps_t* ps) {
  sp_ps_output_t result = SP_ZERO_INITIALIZE();
  u8 buffer[4096];

  struct {
    sp_io_reader_t* out;
    sp_io_reader_t* err;
  } read = {
    .out = sp_ps_io_out(ps),
    .err = sp_ps_io_err(ps),
  };

  struct {
    sp_io_writer_t out;
    sp_io_writer_t err;
  } write = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &write.out);
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &write.err);

  sp_sys_fd_t fds[2];
  u8 ready[2];
  sp_io_reader_t* readers[2];
  sp_io_writer_t* writers[2];
  s32 nfds = 0;

  if (read.out) {
    fds[nfds] = read.out->file.fd;
    readers[nfds] = read.out;
    writers[nfds] = &write.out;
    nfds++;
  }
  if (read.err) {
    fds[nfds] = read.err->file.fd;
    readers[nfds] = read.err;
    writers[nfds] = &write.err;
    nfds++;
  }

  while (nfds > 0) {
    s32 ret = sp_sys_fds_wait(fds, ready, (u64)nfds);
    if (ret < 0) break;

    sp_for(i, (u32)nfds) {
      if (!ready[i]) {
        continue;
      }

      u64 n = 0;
      sp_io_read(readers[i], buffer, sizeof(buffer), &n);
      if (n > 0) {
        sp_io_write_str(writers[i], sp_str((c8*)buffer, n), SP_NULLPTR);
      } else {
        fds[i] = fds[nfds - 1];
        ready[i] = ready[nfds - 1];
        readers[i] = readers[nfds - 1];
        writers[i] = writers[nfds - 1];
        nfds--;
        i--;
      }
    }
  }

  result.out = sp_io_writer_dyn_mem_as_str(&write.out.dyn_mem);
  result.err = sp_io_writer_dyn_mem_as_str(&write.err.dyn_mem);
  result.status = sp_ps_wait(ps);
  return result;
}

bool sp_ps_kill(sp_ps_t* ps) {
  if (!ps || !ps->os) return false;
  if (kill(ps->os->pid, SIGKILL) != 0) return false;
  sp_ps_wait(ps);
  return true;
}

#elif defined(SP_WIN32)
struct sp_ps_os {
  sp_win32_handle_t pid;
};

bool sp_ps_is_fd_valid(sp_sys_fd_t fd) {
  return fd != SP_SYS_INVALID_FD && fd != 0;
}

sp_io_close_mode_t sp_ps_io_close_mode(sp_ps_io_mode_t mode) {
  switch (mode) {
    case SP_PS_IO_MODE_CREATE: {
      return SP_IO_CLOSE_MODE_AUTO;
    }
    case SP_PS_IO_MODE_EXISTING: {
      return SP_IO_CLOSE_MODE_NONE;
    }
    case SP_PS_IO_MODE_NULL:
    case SP_PS_IO_MODE_INHERIT:
    case SP_PS_IO_MODE_REDIRECT: {
      return SP_IO_CLOSE_MODE_NONE;
    }
  }
  SP_UNREACHABLE_RETURN(SP_IO_CLOSE_MODE_NONE);
}

void sp_ps_win32_append_quoted_arg(sp_io_writer_t* builder, sp_str_t arg) {
  bool needs_quotes = sp_str_empty(arg);
  sp_for(i, arg.len) {
    c8 c = arg.data[i];
    if (c == ' ' || c == '\t' || c == '"') {
      needs_quotes = true;
      break;
    }
  }

  if (!needs_quotes) {
    sp_io_write_str(builder, arg, SP_NULLPTR);
    return;
  }

  sp_io_write_c8(builder, '"');

  u32 backslashes = 0;
  sp_for(i, arg.len) {
    c8 c = arg.data[i];
    if (c == '\\') {
      backslashes++;
      continue;
    }

    if (c == '"') {
      sp_for(j, backslashes * 2 + 1) {
        sp_io_write_c8(builder, '\\');
      }
      sp_io_write_c8(builder, '"');
      backslashes = 0;
      continue;
    }

    sp_for(j, backslashes) {
      sp_io_write_c8(builder, '\\');
    }
    backslashes = 0;
    sp_io_write_c8(builder, c);
  }

  sp_for(i, backslashes * 2) {
    sp_io_write_c8(builder, '\\');
  }

  sp_io_write_c8(builder, '"');
}

c8* sp_ps_build_windows_cmdline(sp_ps_config_t* config) {
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &builder);

  sp_ps_win32_append_quoted_arg(&builder, config->command);

  sp_carr_for(config->args, it) {
    sp_str_t arg = config->args[it];
    if (sp_str_empty(arg)) {
      break;
    }

    sp_io_write_c8(&builder, ' ');
    sp_ps_win32_append_quoted_arg(&builder, arg);
  }

  sp_da_for(config->dyn_args, it) {
    sp_io_write_c8(&builder, ' ');
    sp_ps_win32_append_quoted_arg(&builder, config->dyn_args[it]);
  }

  sp_io_write_c8(&builder, '\0');
  return (c8*)builder.dyn_mem.buffer.data;
}

sp_win32_handle_t sp_ps_win32_open_null(sp_win32_dword_t access) {
  SECURITY_ATTRIBUTES attrs = SP_ZERO_INITIALIZE();
  attrs.nLength = sizeof(attrs);
  attrs.bInheritHandle = true;

  return CreateFileA("NUL", access, FILE_SHARE_READ | FILE_SHARE_WRITE, &attrs, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, SP_NULLPTR);
}

sp_win32_handle_t sp_ps_win32_fd_to_handle(sp_sys_fd_t fd) {
  if (!sp_ps_is_fd_valid(fd)) {
    return SP_NULLPTR;
  }
  return (sp_win32_handle_t)fd;
}

typedef struct {
  sp_win32_handle_t child;
  sp_sys_fd_t parent_fd;
} sp_ps_win32_stdio_entry_t;

typedef struct {
  sp_ps_win32_stdio_entry_t in;
  sp_ps_win32_stdio_entry_t out;
  sp_ps_win32_stdio_entry_t err;
} sp_ps_win32_stdio_t;

#define SP_WIN32_INHERITABLE true

sp_err_t sp_ps_win32_configure_io_in(sp_ps_io_in_config_t* io, sp_ps_win32_stdio_entry_t* entry) {
  entry->child = SP_NULLPTR;
  entry->parent_fd = SP_SYS_INVALID_FD;

  switch (io->mode) {
    case SP_PS_IO_MODE_NULL: {
      entry->child = sp_ps_win32_open_null(GENERIC_READ);
      return ((entry->child != SP_NULLPTR) && (entry->child != INVALID_HANDLE_VALUE)) ? SP_OK : SP_ERR;
    }
    case SP_PS_IO_MODE_CREATE: {
      SECURITY_ATTRIBUTES attrs = SP_ZERO_INITIALIZE();
      attrs.nLength = sizeof(attrs);
      attrs.bInheritHandle = true;

      sp_win32_handle_t child_read = SP_NULLPTR;
      sp_win32_handle_t parent_write = SP_NULLPTR;
      if (!CreatePipe(&child_read, &parent_write, &attrs, 0)) {
        return SP_ERR;
      }

      // When we CreateProcess, the child shouldn't inherit the handle the parent uses to write to its input
      SetHandleInformation(parent_write, HANDLE_FLAG_INHERIT, 0);

      entry->child = child_read;
      entry->parent_fd = (sp_sys_fd_t)parent_write;
      return SP_OK;
    }
    case SP_PS_IO_MODE_EXISTING: {
      sp_win32_handle_t process = GetCurrentProcess();
      if (!DuplicateHandle(process, sp_ps_win32_fd_to_handle(io->fd), process, &entry->child, SP_NULL, SP_WIN32_INHERITABLE, DUPLICATE_SAME_ACCESS)) {
        return SP_ERR;
      }
      return entry->child == SP_NULLPTR ? SP_ERR : SP_OK;
    }
    case SP_PS_IO_MODE_INHERIT: {
      sp_win32_handle_t process = GetCurrentProcess();
      if (!DuplicateHandle(process, GetStdHandle(STD_INPUT_HANDLE), process, &entry->child, SP_NULL, SP_WIN32_INHERITABLE, DUPLICATE_SAME_ACCESS)) {
        return SP_ERR;
      }
      return entry->child == SP_NULLPTR ? SP_ERR : SP_OK;
    }
    case SP_PS_IO_MODE_REDIRECT: {
      return SP_OK;
    }
  }

  SP_UNREACHABLE_RETURN(SP_ERR);
}

sp_err_t sp_ps_win32_configure_io_out(sp_ps_io_out_config_t* io, sp_win32_dword_t std_handle, sp_win32_dword_t null_access, sp_ps_win32_stdio_entry_t* entry) {
  entry->child = SP_NULLPTR;
  entry->parent_fd = SP_SYS_INVALID_FD;

  switch (io->mode) {
    case SP_PS_IO_MODE_NULL: {
      entry->child = sp_ps_win32_open_null(null_access);
      return ((entry->child != SP_NULLPTR) && (entry->child != INVALID_HANDLE_VALUE)) ? SP_OK : SP_ERR;
    }
    case SP_PS_IO_MODE_CREATE: {
      SECURITY_ATTRIBUTES attrs = SP_ZERO_INITIALIZE();
      attrs.nLength = sizeof(attrs);
      attrs.bInheritHandle = true;

      sp_win32_handle_t parent_read = SP_NULLPTR;
      sp_win32_handle_t child_write = SP_NULLPTR;
      if (!CreatePipe(&parent_read, &child_write, &attrs, 0)) {
        return SP_ERR;
      }

      SetHandleInformation(parent_read, HANDLE_FLAG_INHERIT, 0);

      entry->child = child_write;
      entry->parent_fd = (sp_sys_fd_t)parent_read;
      return SP_OK;
    }
    case SP_PS_IO_MODE_EXISTING: {
      sp_win32_handle_t process = GetCurrentProcess();
      if (!DuplicateHandle(process, sp_ps_win32_fd_to_handle(io->fd), process, &entry->child, SP_NULL, SP_WIN32_INHERITABLE, DUPLICATE_SAME_ACCESS)) {
        return SP_ERR;
      }

      return entry->child == SP_NULLPTR ? SP_ERR : SP_OK;
    }
    case SP_PS_IO_MODE_INHERIT: {
      sp_win32_handle_t process = GetCurrentProcess();
      if (!DuplicateHandle(process, GetStdHandle(std_handle), process, &entry->child, SP_NULL, SP_WIN32_INHERITABLE, DUPLICATE_SAME_ACCESS)) {
        return SP_ERR;
      }

      return entry->child == SP_NULLPTR ? SP_ERR : SP_OK;
    }
    case SP_PS_IO_MODE_REDIRECT: {
      return SP_OK;
    }
  }

  SP_UNREACHABLE_RETURN(false);
}

void sp_ps_win32_close_child_handles(sp_ps_win32_stdio_t* io) {
  sp_win32_handle_t handles[3] = { io->in.child, io->out.child, io->err.child };
  sp_for(i, 3) {
    if (!handles[i]) {
      continue;
    }

    bool seen = false;
    sp_for(j, i) {
      if (handles[j] == handles[i]) {
        seen = true;
        break;
      }
    }

    if (!seen) {
      CloseHandle(handles[i]);
    }
  }
}

void sp_ps_win32_close_parent_fds(sp_ps_win32_stdio_t* io) {
  sp_sys_fd_t fds[3] = { io->in.parent_fd, io->out.parent_fd, io->err.parent_fd };
  sp_for(i, 3) {
    if (fds[i] != SP_SYS_INVALID_FD) {
      CloseHandle((HANDLE)fds[i]);
    }
  }
}

sp_ps_config_t sp_ps_config_copy(const sp_ps_config_t* src) {
  sp_ps_config_t dst = SP_ZERO_INITIALIZE();

  dst.command = sp_str_copy(src->command);
  dst.cwd = sp_str_copy(src->cwd);

  sp_for(i, SP_PS_MAX_ARGS) {
    if (sp_str_empty(src->args[i])) {
      break;
    }
    dst.args[i] = sp_str_copy(src->args[i]);
  }

  sp_da_for(src->dyn_args, i) {
    sp_da_push(dst.dyn_args, sp_str_copy(src->dyn_args[i]));
  }

  dst.env.mode = src->env.mode;
  sp_env_init(&dst.env.env, sp_context_get()->allocator);
  sp_ht_for(src->env.env.vars, it) {
    sp_str_t key = *sp_ht_it_getkp(src->env.env.vars, it);
    sp_str_t value = *sp_ht_it_getp(src->env.env.vars, it);
    sp_env_insert(&dst.env.env, key, value);
  }

  sp_for(i, SP_PS_MAX_ENV) {
    if (sp_str_empty(src->env.extra[i].key)) {
      break;
    }
    dst.env.extra[i].key = sp_str_copy(src->env.extra[i].key);
    dst.env.extra[i].value = sp_str_copy(src->env.extra[i].value);
  }

  dst.io = src->io;
  return dst;
}

void sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg) {
  SP_ASSERT(config);

  if (!sp_str_empty(arg)) {
    sp_da_push(config->dyn_args, arg);
  }
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  sp_ps_t proc = SP_ZERO_STRUCT(sp_ps_t);
  proc.allocator = sp_context_get()->allocator;
  proc.io = config.io;

  if (proc.io.in.mode != SP_PS_IO_MODE_EXISTING) proc.io.in.fd = SP_SYS_INVALID_FD;
  if (proc.io.out.mode != SP_PS_IO_MODE_EXISTING) proc.io.out.fd = SP_SYS_INVALID_FD;
  if (proc.io.err.mode != SP_PS_IO_MODE_EXISTING) proc.io.err.fd = SP_SYS_INVALID_FD;

  SP_ASSERT(!sp_str_empty(config.command));

  c8* cmdline = sp_ps_build_windows_cmdline(&config);



  sp_io_writer_t b = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &b);

  switch (config.env.mode) {
    case SP_PS_ENV_INHERIT: {
      for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
        sp_io_write_str(&b, it.key, SP_NULLPTR);
        sp_io_write_c8(&b, '=');
        sp_io_write_str(&b, it.value, SP_NULLPTR);
        sp_io_write_c8(&b, '\0');
      }
      break;
    }
    case SP_PS_ENV_EXISTING: {
      sp_env_t* env = &config.env.env;
      sp_str_ht_for(env->vars, it) {
        sp_str_t key = *sp_str_ht_it_getkp(env->vars, it);
        sp_str_t val = *sp_str_ht_it_getp(env->vars, it);

        sp_fmt_io(&b, "{}={}", sp_fmt_str(key), sp_fmt_str(val));
        sp_io_write_c8(&b, '\0');
      }

      break;
    }
    case SP_PS_ENV_CLEAN: {
      break;
    }
  }

  for (s32 it = SP_PS_MAX_ENV - 1; it >= 0; it--) {
    sp_env_var_t e = config.env.extra[it];
    if (sp_str_empty(e.key)) continue;

    sp_fmt_io(&b, "{}={}", sp_fmt_str(e.key), sp_fmt_str(e.value));
    sp_io_write_c8(&b, '\0');
  }

  sp_io_write_c8(&b, '\0');
  sp_io_write_c8(&b, '\0');

  c8* env = (c8*)b.dyn_mem.buffer.data;

  c8* cwd = sp_str_empty(config.cwd) ? SP_NULLPTR : sp_str_to_cstr(config.cwd);

  sp_ps_win32_stdio_t io = {
    .in = { .parent_fd = SP_SYS_INVALID_FD },
    .out = { .parent_fd = SP_SYS_INVALID_FD },
    .err = { .parent_fd = SP_SYS_INVALID_FD },
  };

  sp_err_t error = SP_OK;
  sp_try_goto(sp_ps_win32_configure_io_in(&proc.io.in, &io.in), error, fail);

  if (proc.io.out.mode == SP_PS_IO_MODE_REDIRECT) {
    sp_try_goto(sp_ps_win32_configure_io_out(&proc.io.err, STD_ERROR_HANDLE, GENERIC_WRITE, &io.err), error, fail);
    io.out.child = io.err.child;
  } else if (proc.io.err.mode == SP_PS_IO_MODE_REDIRECT) {
    sp_try_goto(sp_ps_win32_configure_io_out(&proc.io.out, STD_OUTPUT_HANDLE, GENERIC_WRITE, &io.out), error, fail);
    io.err.child = io.out.child;
  } else {
    sp_try_goto(sp_ps_win32_configure_io_out(&proc.io.out, STD_OUTPUT_HANDLE, GENERIC_WRITE, &io.out), error, fail);
    sp_try_goto(sp_ps_win32_configure_io_out(&proc.io.err, STD_ERROR_HANDLE, GENERIC_WRITE, &io.err), error, fail);
  }

  STARTUPINFOA startup = {
    .cb = sizeof(STARTUPINFOA),
    .dwFlags = STARTF_USESTDHANDLES,
    .hStdInput = io.in.child,
    .hStdOutput = io.out.child,
    .hStdError = io.err.child,
  };

  PROCESS_INFORMATION process_info = SP_ZERO_INITIALIZE();
  if (!CreateProcessA(SP_NULLPTR, cmdline, SP_NULLPTR, SP_NULLPTR, true, 0, env, cwd, &startup, &process_info)) {
    sp_ps_win32_close_child_handles(&io);
    sp_ps_win32_close_parent_fds(&io);
    sp_free(cmdline);
    if (env) sp_free(env);
    if (cwd) sp_free(cwd);
    return SP_ZERO_STRUCT(sp_ps_t);
  }

  sp_ps_win32_close_child_handles(&io);
  CloseHandle(process_info.hThread);
  proc.os = sp_alloc_type(sp_ps_os_t);
  proc.os->pid = process_info.hProcess;

  if (io.in.parent_fd != SP_SYS_INVALID_FD) {
    proc.io.in.fd = io.in.parent_fd;
  }
  if (io.out.parent_fd != SP_SYS_INVALID_FD) {
    proc.io.out.fd = io.out.parent_fd;
  }
  if (io.err.parent_fd != SP_SYS_INVALID_FD) {
    proc.io.err.fd = io.err.parent_fd;
  }

  sp_free(cmdline);
  if (env) sp_free(env);
  if (cwd) sp_free(cwd);

  return proc;

fail:
  sp_ps_win32_close_child_handles(&io);
  sp_ps_win32_close_parent_fds(&io);
  sp_free(cmdline);
  if (env) sp_free(env);
  if (cwd) sp_free(cwd);
  return SP_ZERO_STRUCT(sp_ps_t);
}

sp_ps_output_t sp_ps_run(sp_ps_config_t config) {
  config.io.out = (sp_ps_io_out_config_t) {
    .mode = SP_PS_IO_MODE_CREATE,
  };
  sp_ps_t ps = sp_ps_create(config);
  if (ps.os) return sp_ps_output(&ps);
  return (sp_ps_output_t) { .status = { .state = SP_PS_STATE_DONE, .exit_code = -1 } };
}

sp_io_writer_t* sp_ps_io_in(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.in.fd)) return SP_NULLPTR;

  sp_io_writer_t* writer = sp_alloc_type(sp_io_writer_t);
  sp_io_writer_from_fd(writer, ps->io.in.fd, sp_ps_io_close_mode(ps->io.in.mode));
  return writer;
}

sp_io_reader_t* sp_ps_io_out(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.out.fd)) return SP_NULLPTR;

  sp_io_reader_t* reader = sp_alloc_type(sp_io_reader_t);
  sp_io_reader_from_fd(reader, ps->io.out.fd, sp_ps_io_close_mode(ps->io.out.mode));
  return reader;
}

sp_io_reader_t* sp_ps_io_err(sp_ps_t* ps) {
  if (!ps) return SP_NULLPTR;
  if (!sp_ps_is_fd_valid(ps->io.err.fd)) return SP_NULLPTR;

  sp_io_reader_t* reader = sp_alloc_type(sp_io_reader_t);
  sp_io_reader_from_fd(reader, ps->io.err.fd, sp_ps_io_close_mode(ps->io.err.mode));
  return reader;
}

sp_ps_status_t sp_ps_win32_finish_process(sp_ps_t* ps) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  DWORD exit_code = 0;
  if (!GetExitCodeProcess(ps->os->pid, &exit_code)) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
  }
  else {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = (s32)exit_code;
  }

  CloseHandle(ps->os->pid);
  return result;

}

sp_ps_status_t sp_ps_poll(sp_ps_t* ps, u32 timeout_ms) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  DWORD wait = WaitForSingleObject(ps->os->pid, timeout_ms);
  switch (wait) {
    case WAIT_TIMEOUT: {
      result.state = SP_PS_STATE_RUNNING;
      return result;
    }
    case WAIT_OBJECT_0: {
      return sp_ps_win32_finish_process(ps);
    }
    case WAIT_FAILED: {
      result.state = SP_PS_STATE_DONE;
      result.exit_code = -1;
      return result;
    }
  }

  result.state = SP_PS_STATE_DONE;
  result.exit_code = -1;
  return result;
}

sp_ps_status_t sp_ps_wait(sp_ps_t* ps) {
  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  if (!ps || !ps->os) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  DWORD wait = WaitForSingleObject(ps->os->pid, INFINITE);
  if (wait != WAIT_OBJECT_0) {
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  return sp_ps_win32_finish_process(ps);
}

u64 sp_ps_win32_read_available(sp_sys_fd_t fd, sp_io_writer_t* builder, bool* open) {
  sp_win32_handle_t handle = sp_ps_win32_fd_to_handle(fd);
  if (!handle) {
    *open = false;
    return 0;
  }

  u8 buffer[4096];
  u64 total = 0;

  while (true) {
    DWORD available = 0;
    if (!PeekNamedPipe(handle, SP_NULLPTR, 0, SP_NULLPTR, &available, SP_NULLPTR)) {
      if (GetLastError() == ERROR_BROKEN_PIPE) {
        *open = false;
      }
      return total;
    }

    if (available == 0) {
      return total;
    }

    u32 chunk = SP_MIN((u32)sizeof(buffer), (u32)available);
    s64 num_read = sp_sys_read(fd, buffer, chunk);
    if (num_read <= 0) {
      *open = false;
      return total;
    }

    sp_io_write_str(builder, sp_str((c8*)buffer, (u32)num_read), SP_NULLPTR);
    total += (u64)num_read;
  }
}

sp_ps_output_t sp_ps_output(sp_ps_t* ps) {
  sp_ps_output_t result = SP_ZERO_INITIALIZE();

  bool out_open = sp_ps_is_fd_valid(ps->io.out.fd);
  bool err_open = sp_ps_is_fd_valid(ps->io.err.fd);

  sp_io_writer_t out = sp_zero();
  sp_io_writer_t err = sp_zero();
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &out);
  sp_io_writer_from_dyn_mem_a(sp_context_get_allocator(), &err);

  DWORD exit_code = 0;
  bool process_done = !ps->os;

  while (!process_done || out_open || err_open) {
    bool read_any = false;

    if (out_open) {
      read_any |= sp_ps_win32_read_available(ps->io.out.fd, &out, &out_open) > 0;
    }
    if (err_open) {
      read_any |= sp_ps_win32_read_available(ps->io.err.fd, &err, &err_open) > 0;
    }

    if (!process_done && ps->os->pid) {
      DWORD wait = WaitForSingleObject(ps->os->pid, read_any ? 0 : 10);
      if (wait == WAIT_OBJECT_0) {
        process_done = true;
        if (!GetExitCodeProcess(ps->os->pid, &exit_code)) {
          exit_code = (DWORD)-1;
        }
      } else if (wait == WAIT_FAILED) {
        process_done = true;
        exit_code = (DWORD)-1;
      }
    }

    if (!read_any && process_done && !out_open && !err_open) {
      break;
    }
  }

  if (ps->os->pid) {
    CloseHandle(ps->os->pid);
    ps->os->pid = SP_NULLPTR;
  }

  result.out = sp_io_writer_dyn_mem_as_str(&out.dyn_mem);
  result.err = sp_io_writer_dyn_mem_as_str(&err.dyn_mem);
  result.status = (sp_ps_status_t) {
    .state = SP_PS_STATE_DONE,
    .exit_code = process_done ? (s32)exit_code : -1,
  };
  return result;
}

bool sp_ps_kill(sp_ps_t* ps) {
  if (!ps || !ps->os) {
    return false;
  }

  if (!TerminateProcess(ps->os->pid, 1)) {
    return false;
  }

  WaitForSingleObject(ps->os->pid, INFINITE);
  CloseHandle(ps->os->pid);
  ps->os->pid = SP_NULLPTR;
  return true;
}

#else
struct sp_ps_os {
  s32 dummy;
};

sp_ps_config_t sp_ps_config_copy(const sp_ps_config_t* src) {
  SP_UNIMPLEMENTED();
  return *src;
}

void sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg) {
  SP_UNIMPLEMENTED();
  if (!sp_str_empty(arg)) {
    sp_da_push(config->dyn_args, arg);
  }
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(config);
  return SP_ZERO_STRUCT(sp_ps_t);
}

sp_ps_output_t sp_ps_run(sp_ps_config_t config) {
  SP_UNIMPLEMENTED();
  sp_ps_t ps = sp_ps_create(config);
  return sp_ps_output(&ps);
}

sp_io_writer_t* sp_ps_io_in(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_io_reader_t* sp_ps_io_out(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_io_reader_t* sp_ps_io_err(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_ps_status_t sp_ps_poll(sp_ps_t* ps, u32 timeout_ms) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  SP_UNUSED(timeout_ms);
  return SP_RVAL(sp_ps_status_t) {
    .state = SP_PS_STATE_DONE,
    .exit_code = -1,
  };
}

sp_ps_status_t sp_ps_wait(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  return SP_RVAL(sp_ps_status_t) {
    .state = SP_PS_STATE_DONE,
    .exit_code = -1,
  };
}

sp_ps_output_t sp_ps_output(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  return SP_RVAL(sp_ps_output_t) {
    .status = sp_ps_wait(ps),
  };
}

bool sp_ps_kill(sp_ps_t* ps) {
  SP_UNIMPLEMENTED();
  SP_UNUSED(ps);
  return false;
}
#endif


//  ███████████ █████ █████       ██████████    ██████   ██████    ███████    ██████   █████ █████ ███████████    ███████    ███████████
// ░░███░░░░░░█░░███ ░░███       ░░███░░░░░█   ░░██████ ██████   ███░░░░░███ ░░██████ ░░███ ░░███ ░█░░░███░░░█  ███░░░░░███ ░░███░░░░░███
//  ░███   █ ░  ░███  ░███        ░███  █ ░     ░███░█████░███  ███     ░░███ ░███░███ ░███  ░███ ░   ░███  ░  ███     ░░███ ░███    ░███
//  ░███████    ░███  ░███        ░██████       ░███░░███ ░███ ░███      ░███ ░███░░███░███  ░███     ░███    ░███      ░███ ░██████████
//  ░███░░░█    ░███  ░███        ░███░░█       ░███ ░░░  ░███ ░███      ░███ ░███ ░░██████  ░███     ░███    ░███      ░███ ░███░░░░░███
//  ░███  ░     ░███  ░███      █ ░███ ░   █    ░███      ░███ ░░███     ███  ░███  ░░█████  ░███     ░███    ░░███     ███  ░███    ░███
//  █████       █████ ███████████ ██████████    █████     █████ ░░░███████░   █████  ░░█████ █████    █████    ░░░███████░   █████   █████
// ░░░░░       ░░░░░ ░░░░░░░░░░░ ░░░░░░░░░░    ░░░░░     ░░░░░    ░░░░░░░    ░░░░░    ░░░░░ ░░░░░    ░░░░░       ░░░░░░░    ░░░░░   ░░░░░
// @fmon @file_monitor
SP_PRIVATE void sp_fmon_os_init(sp_fmon_t* monitor);
SP_PRIVATE void sp_fmon_os_deinit(sp_fmon_t* monitor);
SP_PRIVATE void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_PRIVATE void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_PRIVATE void sp_fmon_os_process_changes(sp_fmon_t* monitor);

void sp_fmon_init_a(sp_mem_t mem, sp_fmon_t* monitor, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* userdata) {
  monitor->allocator = mem;
  monitor->callback = fn;
  monitor->events_to_watch = events;
  monitor->userdata = userdata;
  monitor->debounce_time_ms = 0;
  sp_fmon_os_init(monitor);
}

void sp_fmon_deinit(sp_fmon_t* monitor) {
  sp_fmon_os_deinit(monitor);
}

void sp_fmon_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  sp_fmon_os_add_dir(monitor, path);
}

void sp_fmon_add_file(sp_fmon_t* monitor, sp_str_t path) {
  sp_fmon_os_add_file(monitor, path);
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

// PLATFORM
#if defined(SP_WIN32)
typedef struct {
  sp_str_t path;
  sp_win32_overlapped_t overlapped;
  sp_win32_handle_t handle;
  void* notify_information;
  s32 bytes_returned;
} sp_fmon_dir_t;

struct sp_fmon_os {
  sp_da(sp_fmon_dir_t) dirs;
  sp_da(sp_str_t) watch_files;
};

#elif defined(SP_LINUX)
struct sp_fmon_os {
  sp_da(s32) fds;
  sp_da(sp_str_t) paths;
  sp_ht_a(sp_str_t, u8) files;
  sp_aligned() u8 buffer[4096];
  s32 fd;
};

#elif defined(SP_MACOS)
#ifndef SP_FMON_ARENA_SIZE
#define SP_FMON_ARENA_SIZE (64 * 1024)
#endif

#if defined(SP_FMON_MACOS_USE_FSEVENTS)
struct sp_fmon_os {
  FSEventStreamRef stream;
  dispatch_queue_t queue;
  sp_da(sp_str_t) watch_paths;
  sp_da(sp_str_t) watch_files;
  sp_fmon_t* monitor;
  sp_mutex_t mutex;
  sp_mem_arena_t* watch_arena;
  sp_mem_arena_t* event_arena;
};
#else
struct sp_fmon_os {
  s32 kq;
  sp_da(s32) fds;
  sp_da(sp_str_t) watch_paths;
};
#endif

#elif defined(SP_COSMO)
struct sp_fmon_os {
  s32 dummy;
};
#endif

#ifdef SP_WIN32
SP_PRIVATE void sp_win32_fmon_add_change(sp_fmon_t* monitor, sp_str_t file_path, sp_str_t file_name, sp_fmon_event_kind_t events);
SP_PRIVATE void sp_win32_fmon_issue_read(sp_fmon_t* monitor, sp_fmon_dir_t* info);

void sp_fmon_os_init(sp_fmon_t* monitor) {
  sp_fmon_os_t* os = SP_ALLOC(sp_fmon_os_t);
  monitor->os = os;
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = monitor->os;
  sp_da_for(os->dirs, i) {
    sp_fmon_dir_t* info = &os->dirs[i];
    if (info->handle && info->handle != INVALID_HANDLE_VALUE) {
      CancelIo(info->handle);
      CloseHandle(info->handle);
    }
    if (info->overlapped.hEvent) {
      CloseHandle(info->overlapped.hEvent);
    }
    if (info->notify_information) {
      sp_free(info->notify_information);
    }
  }
  sp_da_free(os->dirs);
  sp_da_free(os->watch_files);
  sp_free(os);
  monitor->os = NULL;
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  sp_fmon_os_t* os = monitor->os;

  sp_win32_handle_t event = CreateEventW(NULL, false, false, NULL);
  if (!event) return;

  sp_win32_handle_t handle = sp_sys_nt_open(
    path,
    FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NT_FILE_OPEN,
    SP_NT_FILE_DIRECTORY_FILE | SP_NT_FILE_OPEN_FOR_BACKUP_INTENT,
    0
  );

  if (handle == INVALID_HANDLE_VALUE) {
    CloseHandle(event);
    return;
  }

  sp_fmon_dir_t dir = SP_ZERO_INITIALIZE();
  dir.overlapped.hEvent = event;
  dir.handle = handle;
  dir.path = sp_fs_canonicalize_path(path);
  dir.notify_information = sp_alloc(SP_FILE_MONITOR_BUFFER_SIZE);
  sp_mem_zero(dir.notify_information, SP_FILE_MONITOR_BUFFER_SIZE);

  sp_da_push(os->dirs, dir);
  sp_fmon_dir_t* info = &os->dirs[sp_da_size(os->dirs) - 1];
  sp_win32_fmon_issue_read(monitor, info);
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_fmon_os_t* os = monitor->os;
  sp_str_t canonical = sp_fs_canonicalize_path(file_path);
  sp_da_push(os->watch_files, canonical);

  sp_str_t dir_path = sp_fs_parent_path(canonical);
  if (dir_path.len > 0) {
    bool found = false;
    sp_da_for(os->dirs, i) {
      if (sp_str_equal(sp_fs_canonicalize_path(os->dirs[i].path), dir_path)) {
        found = true;
        break;
      }
    }
    if (!found) {
      sp_fmon_os_add_dir(monitor, dir_path);
    }
  }
}

SP_PRIVATE bool sp_win32_fmon_file_matches(sp_fmon_os_t* os, sp_str_t full_path) {
  if (sp_da_size(os->watch_files) == 0) return true;
  sp_da_for(os->watch_files, i) {
    if (sp_str_equal(os->watch_files[i], full_path)) return true;
  }
  return false;
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  sp_fmon_os_t* os = monitor->os;

  sp_da_for(os->dirs, i) {
    sp_fmon_dir_t* info = &os->dirs[i];
    SP_ASSERT(info->handle != INVALID_HANDLE_VALUE);

    if (!HasOverlappedIoCompleted(&info->overlapped)) continue;

    s32 bytes_written = 0;
    bool success = GetOverlappedResult(info->handle, &info->overlapped, (LPDWORD)&bytes_written, false);
    if (!success || bytes_written == 0) {
      sp_win32_fmon_issue_read(monitor, info);
      continue;
    }

    FILE_NOTIFY_INFORMATION* notify = (FILE_NOTIFY_INFORMATION*)info->notify_information;
    while (true) {
      sp_fmon_event_kind_t events = SP_FILE_CHANGE_EVENT_NONE;
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
        events = SP_FILE_CHANGE_EVENT_REMOVED;
      }
      else if (notify->Action == FILE_ACTION_RENAMED_NEW_NAME) {
        events = SP_FILE_CHANGE_EVENT_ADDED;
      }

      if (events != SP_FILE_CHANGE_EVENT_NONE) {
        sp_str_t partial_path_str = sp_win32_utf16_to_utf8(&notify->FileName[0], (s32)(notify->FileNameLength / sizeof(WCHAR)));

        sp_str_t full_path = sp_fs_join_path(info->path, partial_path_str);

        if (sp_win32_fmon_file_matches(os, full_path)) {
          sp_str_t file_name = sp_fs_get_name(full_path);
          sp_win32_fmon_add_change(monitor, full_path, file_name, events);
        }
      }

      if (notify->NextEntryOffset == 0) break;
      notify = (FILE_NOTIFY_INFORMATION*)((char*)notify + notify->NextEntryOffset);
    }

    sp_win32_fmon_issue_read(monitor, info);
  }

  sp_fmon_emit_changes(monitor);
}

void sp_win32_fmon_add_change(sp_fmon_t* monitor, sp_str_t file_path, sp_str_t file_name, sp_fmon_event_kind_t events) {
  if (sp_fs_is_dir(file_path)) return;

  if (file_name.data && file_name.len > 0) {
    if (file_name.data[0] == '.' && file_name.len > 1 && file_name.data[1] == '#') return;
    if (file_name.data[0] ==  '#') return;
  }

  // Coalesce within the current batch
  sp_da_for(monitor->changes, i) {
    sp_fmon_event_t* change = &monitor->changes[i];
    if (sp_str_equal(change->file_path, file_path)) {
      change->events = (sp_fmon_event_kind_t)(change->events | events);
      return;
    }
  }

  sp_fmon_event_t change = {
    .file_path = sp_str_copy(file_path),
    .file_name = sp_str_copy(file_name),
    .events = events,
  };
  sp_da_push(monitor->changes, change);
}

void sp_win32_fmon_issue_read(sp_fmon_t* monitor, sp_fmon_dir_t* info) {
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

  linux_monitor->fd = sp_sys_inotify_init1(SP_IN_NONBLOCK | SP_IN_CLOEXEC);
  if (linux_monitor->fd == -1) {
    // Handle error but don't crash
    linux_monitor->fd = 0;
  }

  sp_str_ht_init_a(monitor->allocator, linux_monitor->files);

  monitor->os = linux_monitor;
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;

  if (os->fd <= 0) return;

  c8* path_cstr = sp_str_to_cstr(path);

  u32 mask = 0;
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_MODIFIED) {
    mask |= SP_IN_MODIFY | SP_IN_ATTRIB | SP_IN_CLOSE_WRITE;
  }
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_ADDED) {
    mask |= SP_IN_CREATE | SP_IN_MOVED_TO;
  }
  if (monitor->events_to_watch & SP_FILE_CHANGE_EVENT_REMOVED) {
    mask |= SP_IN_DELETE | SP_IN_MOVED_FROM;
  }

  s32 wd = sp_sys_inotify_add_watch(os->fd, path_cstr, mask);

  if (wd != -1) {
    sp_da_push(os->fds, wd);
    sp_da_push(os->paths, sp_str_copy(path));
  }

}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  if (!monitor->os) return;
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  if (os->fd > 0) {
    sp_sys_close(os->fd);
  }
  sp_ht_free(os->files);
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  sp_str_t canonical = sp_fs_canonicalize_path_a(monitor->allocator, file_path);
  sp_str_ht_insert(os->files, canonical, 1);

  sp_str_t dir_path = sp_fs_parent_path(canonical);
  if (!sp_str_empty(dir_path)) {
    sp_fmon_os_add_dir(monitor, dir_path);
  }
}

SP_PRIVATE bool sp_linux_fmon_file_matches(sp_fmon_os_t* os, sp_str_t full_path) {
  if (sp_ht_empty(os->files)) return true;
  return sp_ht_getp(os->files, full_path);
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  if (!monitor->os) return;

  sp_fmon_os_t* os = (sp_fmon_os_t*)monitor->os;
  if (os->fd <= 0) return;

  s64 len = sp_sys_read(os->fd, os->buffer, sizeof(os->buffer));
  if (len <= 0) return;

  u8* buffer = (u8*)os->buffer;
  u8* ptr = buffer;
  while (ptr < buffer + len) {
    sp_sys_inotify_event_t* event = (sp_sys_inotify_event_t*)ptr;

    // Find which path this watch descriptor corresponds to
    sp_da_for(os->fds, it) {
      s32 wd = os->fds[it];
      if (wd == event->wd) {
        sp_str_t dir_path = os->paths[it];

        // Build full path if there's a filename
        sp_str_t file_name = SP_ZERO_STRUCT(sp_str_t);
        sp_str_t file_path = SP_ZERO_STRUCT(sp_str_t);

        if (event->len > 0 && event->name[0] != '\0') {
          file_name = sp_str_from_cstr(event->name);
          file_path = sp_fs_join_path_a(monitor->allocator, dir_path, file_name);
        } else {
          file_path = sp_str_copy(dir_path);
          file_name = sp_fs_get_name(file_path);
        }

        if (!sp_linux_fmon_file_matches(os, file_path)) break;

        sp_fmon_event_kind_t events = SP_FILE_CHANGE_EVENT_NONE;
        if (event->mask & (SP_IN_MODIFY | SP_IN_ATTRIB | SP_IN_CLOSE_WRITE)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_MODIFIED);
        }
        if (event->mask & (SP_IN_CREATE | SP_IN_MOVED_TO)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_ADDED);
        }
        if (event->mask & (SP_IN_DELETE | SP_IN_MOVED_FROM)) {
          events = (sp_fmon_event_kind_t)(events | SP_FILE_CHANGE_EVENT_REMOVED);
        }

        if (events != SP_FILE_CHANGE_EVENT_NONE) {
          sp_fmon_event_t change = {
            .file_path = file_path,
            .file_name = file_name,
            .events = events,
            .time = 0  // TODO: get actual time
          };
          sp_da_push(monitor->changes, change);
        }
        break;
      }
    }

    ptr += sizeof(sp_sys_inotify_event_t) + event->len;
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
  sp_context_push_allocator(monitor->allocator);
  sp_fmon_os_t* os = sp_alloc_type(sp_fmon_os_t);
  os->queue = dispatch_queue_create("sp.fmon", DISPATCH_QUEUE_SERIAL);
  os->monitor = monitor;
  sp_mutex_init(&os->mutex, SP_MUTEX_PLAIN);
  os->watch_arena = sp_mem_arena_new(SP_FMON_ARENA_SIZE);
  os->event_arena = sp_mem_arena_new(SP_FMON_ARENA_SIZE);
  monitor->os = os;
  sp_context_pop();
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  sp_require(monitor);
  sp_require(monitor->os);
  sp_context_push_allocator(monitor->allocator);

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
  sp_free(os);
  monitor->os = SP_NULLPTR;

  sp_context_pop();
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
    sp_mutex_lock(&os->mutex);
    sp_da_for(os->watch_paths, it) {
      if (sp_str_equal(os->watch_paths[it], dir)) {
        found = true;
        break;
      }
    }
    sp_mutex_unlock(&os->mutex);

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
  monitor->changes = SP_NULLPTR;
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
          sp_da_push(monitor->changes, change);
        }
        break;
      }
    }
  }

  sp_fmon_emit_changes(monitor);
}

#endif // SP_FMON_MACOS_USE_FSEVENTS

#elif defined(SP_COSMO) || defined(SP_WASM)
void sp_fmon_os_init(sp_fmon_t* monitor) {
  sp_unused(monitor);
}

void sp_fmon_os_deinit(sp_fmon_t* monitor) {
  sp_unused(monitor);
}

void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t directory_path) {
  sp_unused(monitor); sp_unused(directory_path);
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_unused(monitor); sp_unused(file_path);
}

void sp_fmon_os_process_changes(sp_fmon_t* monitor) {
  sp_unused(monitor);
}

#else
#error "sp_fmon_os_init"
#error "sp_fmon_os_deinit"
#error "sp_fmon_os_add_dir"
#error "sp_fmon_os_add_file"
#error "sp_fmon_os_process_changes"
#endif


//  █████    ███████
// ░░███   ███░░░░░███
//  ░███  ███     ░░███
//  ░███ ░███      ░███
//  ░███ ░███      ░███
//  ░███ ░░███     ███
//  █████ ░░░███████░
// ░░░░░    ░░░░░░░
// @io
sp_err_t sp_io_reader_file_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  s64 rc = sp_sys_read(reader->file.fd, ptr, size);
  u64 num_bytes = rc < 0 ? 0 : (u64)rc;

  sp_err_t result = SP_OK;
  if (rc < 0) {
    result = SP_ERR_IO_READ_FAILED;
  }
  else if (size && !num_bytes) {
    result = SP_ERR_IO_EOF;
  }

  if (bytes_read) *bytes_read = num_bytes;
  return result;
}

sp_err_t sp_io_reader_file_seek(sp_io_reader_t* r, s64 offset, sp_io_whence_t whence, s64* position) {
  s64 pos = sp_sys_lseek(r->file.fd, offset, (s32)whence);
  if (pos < 0) {
    if (position) *position = -1;
    return SP_ERR_IO_SEEK_FAILED;
  }
  if (position) *position = pos;
  return SP_OK;
}

sp_err_t sp_io_reader_file_size(sp_io_reader_t* r, u64* size) {
  s64 current = sp_sys_lseek(r->file.fd, 0, SP_IO_SEEK_CUR);
  if (current < 0) {
    if (size) *size = 0;
    return SP_ERR_IO;
  }

  s64 end = sp_sys_lseek(r->file.fd, 0, SP_IO_SEEK_END);
  if (end < 0) {
    if (size) *size = 0;
    return SP_ERR_IO_SEEK_FAILED;
  }

  sp_sys_lseek(r->file.fd, current, SP_IO_SEEK_SET);
  if (size) *size = (u64)end;
  return SP_OK;
}

sp_err_t sp_io_reader_file_close(sp_io_reader_t* r) {
  if (r->file.close_mode == SP_IO_CLOSE_MODE_AUTO) {
    if (sp_sys_close(r->file.fd) < 0) {
      return SP_ERR_IO_CLOSE_FAILED;
    }
  }
  return SP_OK;
}

sp_err_t sp_io_reader_mem_read(sp_io_reader_t* r, void* ptr, u64 size, u64* bytes_read) {
  u64 available = r->mem.len - r->mem.pos;
  u64 n = SP_MIN(size, available);
  sp_mem_copy(r->mem.ptr + r->mem.pos, ptr, n);
  r->mem.pos += n;
  if (bytes_read) *bytes_read = n;
  if (size && !n) return SP_ERR_IO_EOF;
  return SP_OK;
}

sp_err_t sp_io_reader_mem_seek(sp_io_reader_t* r, s64 offset, sp_io_whence_t whence, s64* position) {
  s64 pos = 0;

  switch (whence) {
    case SP_IO_SEEK_SET: {
      pos = offset;
      break;
    }
    case SP_IO_SEEK_CUR: {
      pos = (s64)r->mem.pos + offset;
      break;
    }
    case SP_IO_SEEK_END: {
      pos = (s64)r->mem.len + offset;
      break;
    }
  }

  if (pos < 0 || (u64)pos > r->mem.len) {
    if (position) *position = -1;
    return SP_ERR_IO_SEEK_INVALID;
  }
  r->mem.pos = (u64)pos;
  if (position) *position = pos;
  return SP_OK;
}

sp_err_t sp_io_reader_mem_size(sp_io_reader_t* r, u64* size) {
  if (size) *size = r->mem.len;
  return SP_OK;
}

sp_err_t sp_io_reader_mem_close(sp_io_reader_t* r) {
  (void)r;
  return SP_OK;
}

sp_err_t sp_io_reader_from_file(sp_io_reader_t* reader, sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_sys_fd_t fd = sp_sys_open(sp_str_to_cstr(path), SP_O_RDONLY | SP_O_BINARY, 0);
  sp_mem_end_scratch(scratch);

  if (fd == SP_SYS_INVALID_FD) {
    *reader = SP_ZERO_STRUCT(sp_io_reader_t);
    return SP_ERR_IO_OPEN_FAILED;
  }

  *reader = (sp_io_reader_t) {
    .vtable = {
      .read = sp_io_reader_file_read,
      .seek = sp_io_reader_file_seek,
      .size = sp_io_reader_file_size,
      .close = sp_io_reader_file_close,
    },
    .file = {
      .fd = fd,
      .close_mode = SP_IO_CLOSE_MODE_AUTO
    },
  };
  return SP_OK;
}

void sp_io_reader_from_fd(sp_io_reader_t* reader, sp_sys_fd_t fd, sp_io_close_mode_t mode) {
  *reader = (sp_io_reader_t) {
    .vtable = {
      .read = sp_io_reader_file_read,
      .seek = sp_io_reader_file_seek,
      .size = sp_io_reader_file_size,
      .close = sp_io_reader_file_close,
    },
    .file = {
      .fd = fd,
      .close_mode = mode,
    },
  };
}

void sp_io_reader_from_mem(sp_io_reader_t* reader, const void* ptr, u64 size) {
  *reader = (sp_io_reader_t) {
    .vtable = {
      .read = sp_io_reader_mem_read,
      .seek = sp_io_reader_mem_seek,
      .size = sp_io_reader_mem_size,
      .close = sp_io_reader_mem_close,
    },
    .mem = {
      .ptr = (const u8*)ptr,
      .len = size,
      .pos = 0,
    },
  };
}

void sp_io_reader_set_buffer(sp_io_reader_t* reader, u8* buf, u64 capacity) {
  sp_assert(reader);

  reader->buffer = (sp_mem_buffer_t) {
    .data = buf,
    .len = 0,
    .capacity = capacity,
  };
  reader->seek = 0;
}

sp_err_t sp_io_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  sp_assert(reader);

  if (!reader->buffer.data) {
    return reader->vtable.read(reader, ptr, size, bytes_read);
  }

  sp_err_t err = SP_OK;
  u8* buffer = (u8*)ptr;
  u64 num_drained = 0;
  u64 num_read = 0;

  // Drain what we can from what's already buffered
  num_drained = SP_MIN(size, reader->buffer.len - reader->seek);
  sp_mem_copy(reader->buffer.data + reader->seek, buffer, num_drained);
  reader->seek += num_drained;

  // Issue a call to the backend for the rest
  u64 remaining = size - num_drained;

  if (remaining >= reader->buffer.capacity) {
    // If the request is too large to buffer, just read it directly
    err = reader->vtable.read(reader, buffer + num_drained, remaining, &num_read);
  }
  else if (remaining) {
    // If the request is bufferable, do so by completely filling the buffer and then draining
    // just what the user asked for
    err = reader->vtable.read(reader, reader->buffer.data, reader->buffer.capacity, &reader->buffer.len);

    num_read = SP_MIN(remaining, reader->buffer.len);
    reader->seek = num_read;
    sp_mem_copy(reader->buffer.data, buffer + num_drained, num_read);
  }

  u64 num_total = num_drained + num_read;
  if (bytes_read) *bytes_read = num_total;
  if (err == SP_ERR_IO_EOF && num_total) return SP_OK;
  return err;
}

sp_err_t sp_io_reader_seek(sp_io_reader_t* reader, s64 offset, sp_io_whence_t whence, s64* position) {
  sp_assert(reader);

  if (reader->buffer.data && reader->buffer.len > reader->seek) {
    s64 buffered = (s64)(reader->buffer.len - reader->seek);
    reader->vtable.seek(reader, -buffered, SP_IO_SEEK_CUR, SP_NULLPTR);
    reader->seek = 0;
    reader->buffer.len = 0;
  }

  return reader->vtable.seek(reader, offset, whence, position);
}

sp_err_t sp_io_reader_size(sp_io_reader_t* reader, u64* size) {
  sp_assert(reader);
  return reader->vtable.size(reader, size);
}

sp_err_t sp_io_reader_close(sp_io_reader_t* reader) {
  if (!reader->vtable.close) return SP_OK;
  return reader->vtable.close(reader);
}

sp_err_t sp_io_writer_file_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  s64 rc = sp_sys_write(writer->file.fd, ptr, size);
  u64 num_bytes = rc < 0 ? 0 : (u64)rc;

  sp_err_t result = SP_OK;
  if (rc < 0) {
    result = SP_ERR_IO_WRITE_FAILED;
  }
  else if (size && !num_bytes) {
    result = SP_ERR_IO_EOF;
  }

  if (bytes_written) *bytes_written = num_bytes;
  return result;
}

sp_err_t sp_io_writer_file_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position) {
  s64 p = 0;
  position = position ? position : &p;

  *position = sp_sys_lseek(writer->file.fd, offset, (s32)whence);
  if (*position < 0) {
    return SP_ERR_IO_SEEK_FAILED;
  }
  return SP_OK;
}

sp_err_t sp_io_writer_file_size(sp_io_writer_t* writer, u64* size) {
  u64 s = 0;
  size = size ? size : &s;

  s64 current = sp_sys_lseek(writer->file.fd, 0, SP_IO_SEEK_CUR);
  if (current < 0) {
    return SP_ERR_IO;
  }

  *size = sp_sys_lseek(writer->file.fd, 0, SP_IO_SEEK_END);
  if (*size < 0) {
    return SP_ERR_IO_SEEK_FAILED;
  }

  sp_sys_lseek(writer->file.fd, current, SP_IO_SEEK_SET);
  return SP_OK;
}

sp_err_t sp_io_writer_file_close(sp_io_writer_t* writer) {
  if (writer->file.close_mode != SP_IO_CLOSE_MODE_AUTO) return SP_OK;

  if (sp_sys_close(writer->file.fd) < 0) {
    return SP_ERR_IO_CLOSE_FAILED;
  }
  return SP_OK;
}

sp_err_t sp_io_writer_from_file(sp_io_writer_t* writer, sp_str_t path, sp_io_write_mode_t mode) {
  s32 flags = SP_O_WRONLY | SP_O_CREAT | SP_O_BINARY;
  switch (mode) {
    case SP_IO_WRITE_MODE_OVERWRITE: {
      flags |= SP_O_TRUNC;
      break;
    }
    case SP_IO_WRITE_MODE_APPEND: {
      flags |= SP_O_APPEND;
      break;
    }
  }


  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_fd_t fd = sp_sys_open(sp_cstr_from_str_a(s.mem, path), flags, 0644);
  sp_mem_end_scratch_a(s);

  if (fd == SP_SYS_INVALID_FD) {
    *writer = SP_ZERO_STRUCT(sp_io_writer_t);
    return SP_ERR_IO_OPEN_FAILED;
  }

  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_file_write,
      .seek = sp_io_writer_file_seek,
      .size = sp_io_writer_file_size,
      .close = sp_io_writer_file_close,
    },
    .file = {
      .fd = fd,
      .close_mode = SP_IO_CLOSE_MODE_AUTO,
    },
  };
  return SP_OK;
}

void sp_io_writer_from_fd(sp_io_writer_t* writer, sp_sys_fd_t fd, sp_io_close_mode_t close_mode) {
  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_file_write,
      .seek = sp_io_writer_file_seek,
      .size = sp_io_writer_file_size,
      .close = sp_io_writer_file_close,
    },
    .file = {
      .fd = fd,
      .close_mode = close_mode,
    },
  };
}

sp_err_t sp_io_writer_set_buffer(sp_io_writer_t* writer, u8* ptr, u64 size) {
  sp_assert(writer);
  sp_try(sp_io_flush(writer));

  writer->buffer = (sp_mem_buffer_t) {
    .data = ptr,
    .len = 0,
    .capacity = size
  };
  return SP_OK;
}

static sp_err_t sp_io_write_all(sp_io_writer_t* writer, const void* data, u64 size, u64* bytes_written) {
  sp_err_t result = SP_OK;
  u64 total = 0;
  while (total < size) {
    const u8* ptr = ((const u8*)data) + total;
    u64 remaining = size - total;

    // If write() returns 0 bytes written, but also does not report an error, we just
    // keep looping. If this keeps happening, though, you're stuck. Defensively, it
    // makes sense to just bail rather than risk *any* deadlock, but I think that doing
    // that would just hide the real breaking of an invariant.
    //
    // In other words: You asked to write some number of bytes. The backend failed to
    // write anything, but somehow did not encounter an error. That doesn't make sense,
    // and indicates a backend bug.
    u64 written = 0;
    sp_try_goto(writer->vtable.write(writer, ptr, remaining, &written), result, done);
    total += written;
  }

done:
  if (bytes_written) *bytes_written = total;
  return result;
}

sp_err_t sp_io_flush(sp_io_writer_t* writer) {
  sp_assert(writer);
  if (!writer->buffer.len) return SP_OK;

  // Treat a flush failure as not recoverable. This is probably naive, but I haven't hit
  // a case where I want to retry a failed flush. There's no big reason that I don't e.g.
  // move what remains unwritten to the front of the buffer for later
  sp_err_t err = sp_io_write_all(writer, writer->buffer.data, writer->buffer.len, SP_NULLPTR);
  writer->buffer.len = 0;
  return err;
}

sp_err_t sp_io_write(sp_io_writer_t* writer, const void* data, u64 size, u64* bytes_written) {
  sp_assert(writer);

  sp_err_t err = SP_OK;
  u64 total = 0;

  if (!writer->buffer.data) {
    err = sp_io_write_all(writer, data, size, &total);
    goto done;
  }

  const u8* ptr = (const u8*)data;

  if (size > writer->buffer.capacity) {
    // If the write is too big for the buffer, just flush whatever's currently
    // buffered and loop direct writes.
    sp_try_goto(sp_io_flush(writer), err, done);
    sp_try_goto(sp_io_write_all(writer, ptr, size, &total), err, done);
  }
  else {
    // If the write is bufferable in general, but the buffer is too full, flush it first.
    // The correct way to do this (on POSIX) is with writev(), which lets you write whatever's
    // buffered AND the new data in one syscall. But this requires a bit of platform code,
    // and there is a larger task to stop using libc to implement sp_io on Windows.
    //
    // We could use _get_osfhandle() to do this without rewriting everything, but that's junk
    // code that'll die as soon as we switch to using NT.
    if (writer->buffer.capacity - writer->buffer.len < size) {
      sp_try_goto(sp_io_flush(writer), err, done);
    }
    sp_mem_copy(ptr, writer->buffer.data + writer->buffer.len, size);
    writer->buffer.len += size;
    total = size;
  }

done:
  if (bytes_written) *bytes_written = total;
  return err;
}

sp_err_t sp_io_write_str(sp_io_writer_t* writer, sp_str_t str, u64* bytes_written) {
  return sp_io_write(writer, str.data, str.len, bytes_written);
}

sp_err_t sp_io_write_cstr(sp_io_writer_t* writer, const c8* cstr, u64* bytes_written) {
  return sp_io_write(writer, cstr, sp_cstr_len(cstr), bytes_written);
}

sp_err_t sp_io_write_c8(sp_io_writer_t* writer, c8 c) {
  return sp_io_write(writer, &c, 1, SP_NULLPTR);
}

sp_err_t sp_io_pad(sp_io_writer_t* writer, u64 size, u64* bytes_written) {
  static const u8 zeros [64] = SP_ZERO_INITIALIZE();

  sp_assert(writer);

  sp_err_t result = SP_OK;
  u64 total = 0;

  sp_try_goto(sp_io_flush(writer), result, done);

  while (total < size) {
    u64 chunk = SP_MIN(size - total, 64);
    u64 chunk_written = 0;
    sp_try_goto(sp_io_write(writer, zeros, chunk, &chunk_written), result, done);
    total += chunk_written;
  }

done:
  if (bytes_written) *bytes_written = total;
  return result;
}

sp_err_t sp_io_writer_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position) {
  sp_assert(writer);
  sp_try(sp_io_flush(writer));
  return writer->vtable.seek(writer, offset, whence, position);
}

sp_err_t sp_io_writer_size(sp_io_writer_t* writer, u64* size) {
  sp_assert(writer);
  sp_try(sp_io_flush(writer));
  return writer->vtable.size(writer, size);
}

sp_err_t sp_io_writer_close(sp_io_writer_t* writer) {
  sp_assert(writer);

  // We're trying to close it, so just eat the error if flush fails
  sp_io_flush(writer);

  if (!writer->vtable.close) return SP_OK;
  return writer->vtable.close(writer);
}

void sp_io_get_std_out(sp_io_writer_t* io) {
  sp_io_writer_from_fd(io, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
}

void sp_io_get_std_err(sp_io_writer_t* io) {
  sp_io_writer_from_fd(io, sp_sys_stderr, SP_IO_CLOSE_MODE_NONE);
}

/////////
// APP //
/////////
SP_API sp_app_t* sp_app_new(sp_mem_t mem, sp_app_config_t config) {
  sp_app_t* app = sp_alloc_type_a(mem, sp_app_t);
  *app = (sp_app_t) {
    .user_data = config.user_data,
    .on_init = config.on_init,
    .on_poll = config.on_poll,
    .on_update = config.on_update,
    .on_deinit = config.on_deinit,
    .fps = config.fps ? config.fps : 15,
    .mode = config.mode,
  };
  return app;
}

SP_API sp_app_result_t sp_app_tick(sp_app_t* app) {
  if (!app->initted) {
    if (app->on_init) {
      app->result = app->on_init(app);
      if (app->result != SP_APP_CONTINUE) {
        goto deinit;
      }
    }
    app->frame.timer = sp_tm_start_timer();
    app->initted = true;
    //return SP_APP_CONTINUE;
  }

  if (app->on_poll) {
    app->result = app->on_poll(app);
    if (app->result != SP_APP_CONTINUE) {
      goto deinit;
    }
  }

  u64 target = sp_tm_fps_to_ns(app->fps);
  app->frame.accumulated += sp_tm_lap_timer(&app->frame.timer);
  if (app->frame.accumulated >= target) {
    app->frame.accumulated -= target;
    app->frame.num++;
    if (app->on_update) {
      app->result = app->on_update(app);
      if (app->result != SP_APP_CONTINUE) {
        goto deinit;
      }
    }
  }

  return SP_APP_CONTINUE;

deinit:
  if (!app->deinitted) {
    if (app->on_deinit) {
      app->on_deinit(app);
    }
    app->deinitted = true;
  }
  return app->result;
}

SP_API void sp_app_destroy(sp_app_t* app) {
  if (!app) return;
  if (app->initted && !app->deinitted) {
    if (app->on_deinit) {
      app->on_deinit(app);
    }
    app->deinitted = true;
  }
}

SP_PRIVATE s32 sp_app_finalize_rc(sp_app_t* app) {
  switch (app->result) {
    case SP_APP_ERR: return app->rc ? app->rc : 1;
    case SP_APP_QUIT: return app->rc;
    case SP_APP_CONTINUE: { sp_unreachable_case(); }
  }

  sp_unreachable_return(1);
}

SP_API s32 sp_app_run_locked(sp_app_t* app) {
  while (sp_app_tick(app) == SP_APP_CONTINUE) {
    if (app->initted) {
      u64 target = sp_tm_fps_to_ns(app->fps);
      if (app->frame.accumulated < target) {
        sp_sleep_ns(target - app->frame.accumulated);
      }
    }
  }

  return sp_app_finalize_rc(app);
}

SP_API s32 sp_app_run_free(sp_app_t* app) {
  while (sp_app_tick(app) == SP_APP_CONTINUE) {}

  return sp_app_finalize_rc(app);
}

SP_API s32 sp_app_run(sp_app_config_t config) {
  sp_mem_t mem = sp_mem_os_new();
  sp_app_t* app = sp_app_new(mem, config);
  s32 rc = (app->mode == SP_APP_MODE_FREE)
    ? sp_app_run_free(app)
    : sp_app_run_locked(app);
  sp_mem_os_free(app);
  return rc;
}

#define SP_APP_MAIN(fn) \
  s32 sp_app_main(s32 num_args, const c8** args) { \
    sp_app_config_t config = fn(num_args, args); \
    return sp_app_run(config); \
  } \
  SP_MAIN(sp_app_main)








sp_mem_arena_t* sp_tls_rt_get_scratch_arena_a(sp_tls_rt_t* tls) {
  return tls->scratch[0];
}

sp_mem_arena_t* sp_tls_rt_get_scratch_arena_for_a(sp_tls_rt_t* tls, sp_mem_t mem) {
  sp_carr_for(tls->scratch, it) {
    sp_mem_t arena = sp_mem_arena_as_allocator(tls->scratch[it]);
    if (arena.on_alloc != mem.on_alloc || arena.user_data != mem.user_data) {
      return tls->scratch[it];
    }
  }

  sp_unreachable_return(SP_NULLPTR);
}

sp_mem_os_header_t* sp_mem_os_get_header(void* ptr) {
  return ((sp_mem_os_header_t*)ptr) - 1;
}

void* sp_mem_os_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr) {
  (void)user_data;
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC:  return sp_mem_os_alloc_zero(size);
    case SP_ALLOCATOR_MODE_RESIZE: return sp_mem_os_realloc(ptr, size);
    case SP_ALLOCATOR_MODE_FREE:   sp_mem_os_free(ptr); return SP_NULLPTR;
    default:                       return SP_NULLPTR;
  }
}

sp_mem_t sp_mem_os_new() {
  sp_mem_t allocator;
  allocator.on_alloc = sp_mem_os_on_alloc;
  allocator.user_data = NULL;
  return allocator;
}

void* sp_alloc_a(sp_mem_t allocator, u64 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_alloc(ctx->allocator, size);
}

void* sp_realloc_a(sp_mem_t allocator, void* memory, u64 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_realloc(ctx->allocator, memory, size);
}

void sp_free_a(sp_mem_t allocator, void* memory) {
  sp_context_t* ctx = sp_context_get();
  sp_mem_allocator_free(ctx->allocator, memory);
}

sp_mem_arena_t* sp_mem_get_scratch_arena_a(sp_mem_t mem) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  return sp_tls_rt_get_scratch_arena_for_a(tls, mem);
}

sp_mem_arena_marker_t sp_mem_begin_scratch_a() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_mem_arena_t* arena = sp_tls_rt_get_scratch_arena_a(tls);
  return sp_mem_arena_mark(arena);
}

sp_mem_arena_marker_t sp_mem_begin_scratch_for_a(sp_mem_t mem) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_mem_arena_t* arena = sp_tls_rt_get_scratch_arena_for_a(tls, mem);
  return sp_mem_arena_mark(arena);
}

void sp_mem_end_scratch_a(sp_mem_arena_marker_t s) {
  sp_mem_arena_pop(s);
}

sp_mem_t sp_mem_scratch_allocator_a() {
  return sp_mem_arena_as_allocator(sp_tls_rt_get_scratch_arena_a(sp_tls_rt_get()));
}

sp_err_t sp_io_writer_mem_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  sp_err_t result = SP_OK;
  u64 written = 0;

  // If you try a write that would overflow, write nothing. We could write what we're able to
  // and return an error, but the general principle is to stop as soon as you know you're in
  // an error state. And "I want to write 16 bytes into an 8 byte buffer" is an error state. I
  // would rather end up in the same state every time (nothing written, get an error).
  u64 available = writer->mem.len - writer->mem.pos;
  if (size > available) {
    result = SP_ERR_IO_NO_SPACE;
    goto done;
  }

  sp_mem_copy(ptr, writer->mem.ptr + writer->mem.pos, size);
  writer->mem.pos += size;
  written = size;

done:
  if (bytes_written) *bytes_written = written;
  return result;
}

sp_err_t sp_io_writer_mem_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position) {
  s64 pos = 0;

  switch (whence) {
    case SP_IO_SEEK_SET: {
      pos = offset;
      break;
    }
    case SP_IO_SEEK_CUR: {
      pos = (s64)writer->mem.pos + offset;
      break;
    }
    case SP_IO_SEEK_END: {
      pos = (s64)writer->mem.len + offset;
      break;
    }
  }

  if (pos < 0 || (u64)pos > writer->mem.len) {
    if (position) *position = -1;
    return SP_ERR_IO_SEEK_INVALID;
  }
  writer->mem.pos = (u64)pos;
  if (position) *position = pos;
  return SP_OK;
}

sp_err_t sp_io_writer_mem_size(sp_io_writer_t* writer, u64* size) {
  if (size) *size = writer->mem.len;
  return SP_OK;
}

sp_err_t sp_io_writer_mem_close(sp_io_writer_t* writer) {
  (void)writer;
  return SP_OK;
}

void sp_io_writer_from_mem(sp_io_writer_t* writer, void* ptr, u64 size) {
  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_mem_write,
      .seek = sp_io_writer_mem_seek,
      .size = sp_io_writer_mem_size,
      .close = sp_io_writer_mem_close,
    },
    .mem = {
      .ptr = (u8*)ptr,
      .len = size,
      .pos = 0,
    },
  };
}

sp_err_t sp_io_writer_dyn_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  sp_io_writer_dyn_mem_t* io = &writer->dyn_mem;

  // Keep doubling the underlying buffer if there's not enough space
  u64 required = io->seek + size;
  if (required > io->buffer.capacity) {
    u64 new_capacity = io->buffer.capacity ? io->buffer.capacity : 64;
    while (new_capacity < required) {
      new_capacity *= 2;
    }
    io->buffer.data = (u8*)sp_mem_allocator_realloc(io->allocator, io->buffer.data, new_capacity);
    io->buffer.capacity = new_capacity;
  }

  sp_mem_copy(ptr, io->buffer.data + io->seek, size);
  io->seek += size;
  if (io->seek > io->buffer.len) {
    io->buffer.len = io->seek;
  }
  if (bytes_written) *bytes_written = size;
  return SP_OK;
}

sp_err_t sp_io_writer_dyn_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position) {
  sp_io_writer_dyn_mem_t* io = &writer->dyn_mem;

  s64 pos = 0;
  position = position ? position : &pos;

  switch (whence) {
    case SP_IO_SEEK_SET: {
      *position = offset;
      break;
    }
    case SP_IO_SEEK_CUR: {
      *position = (s64)io->seek + offset;
      break;
    }
    case SP_IO_SEEK_END: {
      *position = (s64)io->buffer.len + offset;
      break;
    }
  }

  if (*position < 0) return SP_ERR_IO_SEEK_INVALID;
  if (*position > (s64)io->buffer.len) return SP_ERR_IO_SEEK_INVALID;

  io->seek = (u64)(*position);
  return SP_OK;
}

sp_err_t sp_io_writer_dyn_size(sp_io_writer_t* writer, u64* size) {
  sp_assert(size);
  *size = writer->dyn_mem.buffer.len;
  return SP_OK;
}

sp_err_t sp_io_writer_dyn_close(sp_io_writer_t* writer) {
  if (writer->dyn_mem.buffer.data) {
    sp_mem_allocator_free(writer->dyn_mem.allocator, writer->dyn_mem.buffer.data);
    writer->dyn_mem.buffer = SP_ZERO_STRUCT(sp_mem_buffer_t);
  }
  return SP_OK;
}

void sp_io_writer_from_dyn_mem_a(sp_mem_t mem, sp_io_writer_t* writer) {
  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_dyn_write,
      .seek = sp_io_writer_dyn_seek,
      .size = sp_io_writer_dyn_size,
      .close = sp_io_writer_dyn_close,
    },
    .dyn_mem = {
      .allocator = mem,
    }
  };
}

sp_str_t sp_io_writer_mem_as_str(sp_io_writer_mem_t* io) {
  return (sp_str_t){ .data = (c8*)io->ptr, .len = (u32)io->pos };
}

sp_str_t sp_io_writer_dyn_mem_as_str(sp_io_writer_dyn_mem_t* io) {
  return sp_mem_buffer_as_str(&io->buffer);
}

sp_str_r sp_fmt_a(sp_mem_t mem, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_dyn_mem_a(mem, &io);
  sp_err_t err = sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_str_r result = sp_zero();
  result.err = err;
  if (!err) result.value = sp_io_writer_dyn_mem_as_str(&io.dyn_mem);
  return result;
}

sp_err_t sp_fmt_v_a(sp_io_writer_t* io, sp_str_t fmt, va_list args) {
  sp_fmt_parser_t p = { .str = fmt };

  while (true) {
    c8 c = sp_fmt_peek(&p, 0);
    if (!c) break;

    if (c == '{') {
      if (sp_fmt_peek(&p, 1) == '{') {
        sp_fmt_advance(&p);
        sp_fmt_advance(&p);
        sp_io_write_c8(io, '{');
        continue;
      }

      sp_fmt_spec_t spec = sp_zero();
      sp_try(sp_fmt_parse_specifier(&p, &spec));

      if (spec.fill_dynamic) {
        s64 v;
        sp_try(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v));
        spec.fill = (u8)v;
      }
      if (spec.width_dynamic) {
        s64 v;
        sp_try(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v));
        if (v < 0) v = 0;
        if (v > SP_FMT_WIDTH_MAX) v = SP_FMT_WIDTH_MAX;
        spec.width = (u32)v;
      }
      if (spec.precision_dynamic) {
        s64 v;
        sp_try(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v));
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        sp_opt_set(spec.precision, (u8)v);
      }

      sp_fmt_arg_t params[SP_FMT_MAX_DIRECTIVES] = SP_ZERO_INITIALIZE();
      for (u8 di = 0; di < spec.directive_count; di++) {
        if (spec.directive_arg_dynamic & (1u << di)) {
          params[di] = va_arg(args, sp_fmt_arg_t);
        }
      }

      sp_fmt_arg_t arg = va_arg(args, sp_fmt_arg_t);
      arg.spec = spec;
      sp_try(sp_fmt_render_a(io, &arg, params));
      continue;
    }

    if (c == '}') {
      if (sp_fmt_peek(&p, 1) == '}') {
        sp_fmt_advance(&p);
        sp_fmt_advance(&p);
        sp_io_write_c8(io, '}');
        continue;
      }
      return SP_ERR_FMT_BAD_PLACEHOLDER;
    }

    sp_io_write_c8(io, c);
    sp_fmt_advance(&p);
  }

  return SP_OK;
}

void sp_log_a(const c8* fmt, ...) {
  u8 buffer [4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_str_t str = sp_io_writer_mem_as_str(&io.mem);
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, str, SP_NULLPTR);
  sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
}

void sp_log_a_a(sp_mem_t mem, const c8* fmt, ...) {
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_dyn_mem_a(mem, &io);

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_str_t str = sp_io_writer_dyn_mem_as_str(&io.dyn_mem);
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, str, SP_NULLPTR);
  sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
  sp_mem_end_scratch_a(s);
}

void sp_log_str_a(sp_str_t fmt, ...) {
  u8 buffer[4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, fmt, args);
  va_end(args);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, sp_io_writer_mem_as_str(&io.mem), SP_NULLPTR);
  sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
}

void sp_log_err_a(const c8* fmt, ...) {
  u8 buffer[4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, sp_io_writer_mem_as_str(&io.mem), SP_NULLPTR);
  sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
}

void sp_print_a(const c8* fmt, ...) {
  u8 buffer[4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, sp_io_writer_mem_as_str(&io.mem), SP_NULLPTR);
}

void sp_print_str_a(sp_str_t fmt, ...) {
  u8 buffer[4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, fmt, args);
  va_end(args);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.out, sp_io_writer_mem_as_str(&io.mem), SP_NULLPTR);
}

void sp_print_err_a(const c8* fmt, ...) {
  u8 buffer[4096] = sp_zero();
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_mem(&io, buffer, sizeof(buffer));

  va_list args;
  va_start(args, fmt);
  sp_fmt_v_a(&io, sp_str_view(fmt), args);
  va_end(args);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_io_write_str(tls->std.err, sp_io_writer_mem_as_str(&io.mem), SP_NULLPTR);
}

sp_err_t sp_fmt_render_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* directive_params) {
  sp_fmt_directive_t* directives[SP_FMT_MAX_DIRECTIVES];

  u8 num_dirs = arg->spec.directive_count;
  sp_for(i, num_dirs) {
    sp_str_t name = arg->spec.directive_names[i];
    sp_fmt_directive_t* directive = sp_fmt_directive_lookup(name);
    directives[i] = directive;
    if (!directives[i]) {
      return SP_ERR_FMT_UNKNOWN_DIRECTIVE;
    }
  }

  sp_for(it, num_dirs) {
    sp_fmt_arg_kind_t accepted = directives[it]->arg_kinds;
    if (accepted && !(accepted & arg->id)) {
      return SP_ERR_FMT_WRONG_PARAM_KIND;
    }
  }

  bool params [SP_FMT_MAX_DIRECTIVES] = sp_zero();
  sp_for(it, num_dirs) {
    bool is_dynamic = (arg->spec.directive_arg_dynamic & (1u << it)) != 0;
    bool is_literal = !sp_str_empty(arg->spec.directive_args[it]);
    if (is_dynamic) {
      params[it] = true;
    }
    else if (is_literal) {
      directive_params[it] = (sp_fmt_arg_t){
        .id = sp_fmt_id_str,
        .s  = arg->spec.directive_args[it],
      };
      params[it] = true;
    }
    else {
      params[it] = false;
    }

    sp_fmt_arg_kind_t kind = directives[it]->param_kinds;
    if (!kind) {
      if (params[it]) return SP_ERR_FMT_DIRECTIVE_ARG_UNEXPECTED;
    }
    else {
      if (!params[it]) return SP_ERR_FMT_DIRECTIVE_ARG_MISSING;
      if (!(kind & directive_params[it].id)) return SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND;
    }
  }

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_a();

  sp_io_writer_t before_io = sp_zero();
  sp_io_writer_t content_io = sp_zero();
  sp_io_writer_t after_io = sp_zero();
  sp_io_writer_from_dyn_mem_a(scratch.mem, &before_io);
  sp_io_writer_from_dyn_mem_a(scratch.mem, &content_io);
  sp_io_writer_from_dyn_mem_a(scratch.mem, &after_io);

  sp_for(it, num_dirs) {
    if (directives[it]->kind != sp_fmt_directive_decorator) continue;
    sp_fmt_fn_t before_fn = directives[it]->decorator.before;
    if (!before_fn) continue;
    sp_fmt_arg_t* p = params[it] ? &directive_params[it] : SP_NULLPTR;
    before_fn(&before_io, arg, p);
  }

  struct {
    sp_fmt_fn_t fn;
    sp_fmt_arg_t* param;
  } render = sp_zero();
  if (arg->id == sp_fmt_id_custom) {
    if (!arg->custom.fn) {
      sp_mem_end_scratch_a(scratch);
      return SP_ERR_FMT_CUSTOM_WITHOUT_FN;
    }
    render.fn = arg->custom.fn;
  }
  else {
    sp_for(it, num_dirs) {
      if (directives[it]->kind != sp_fmt_directive_renderer) continue;
      if (render.fn) {
        sp_mem_end_scratch_a(scratch);
        return SP_ERR_FMT_TOO_MANY_RENDERERS;
      }
      render.fn = directives[it]->renderer;
      render.param = params[it] ? &directive_params[it] : SP_NULLPTR;
    }
  }
  render.fn = render.fn ? render.fn : sp_fmt_render_default_a;
  render.fn(&content_io, arg, render.param);

  sp_str_t content = sp_io_writer_dyn_mem_as_str(&content_io.dyn_mem);

  u8 j = num_dirs;
  while (j--) {
    if (directives[j]->kind != sp_fmt_directive_transformer) continue;
    sp_fmt_arg_t* p = params[j] ? &directive_params[j] : SP_NULLPTR;
    sp_io_writer_t next = sp_zero();
    sp_io_writer_from_dyn_mem_a(scratch.mem, &next);
    directives[j]->transformer(&next, content, arg, p);
    content = sp_io_writer_dyn_mem_as_str(&next.dyn_mem);
  }

  j = num_dirs;
  while (j--) {
    if (directives[j]->kind != sp_fmt_directive_decorator) continue;
    sp_fmt_fn_t after_fn = directives[j]->decorator.after;
    if (!after_fn) continue;
    sp_fmt_arg_t* p = params[j] ? &directive_params[j] : SP_NULLPTR;
    after_fn(&after_io, arg, p);
  }

  sp_str_t before = sp_io_writer_dyn_mem_as_str(&before_io.dyn_mem);
  sp_str_t after  = sp_io_writer_dyn_mem_as_str(&after_io.dyn_mem);

  sp_fmt_apply_spec_a(io, before, content, after, arg->spec);
  sp_mem_end_scratch_a(scratch);
  return SP_OK;
}

void sp_fmt_apply_spec_a(sp_io_writer_t* io, sp_str_t pre, sp_str_t str, sp_str_t post, sp_fmt_spec_t spec) {
  u32 content_len = (u32)str.len;
  u32 width = spec.width > SP_FMT_WIDTH_MAX ? SP_FMT_WIDTH_MAX : spec.width;
  u32 pad = (width > content_len) ? (width - content_len) : 0;
  u8 fill = spec.fill ? spec.fill : ' ';
  sp_fmt_align_t align = spec.align;
  if (align == SP_FMT_ALIGN_NONE) align = SP_FMT_ALIGN_RIGHT;

  u32 left = 0;
  u32 right = 0;
  switch (align) {
    case SP_FMT_ALIGN_LEFT:
      right = pad;
      break;
    case SP_FMT_ALIGN_CENTER:
      left = pad / 2;
      right = pad - left;
      break;
    case SP_FMT_ALIGN_RIGHT:
      left = pad;
      break;
    case SP_FMT_ALIGN_NONE:
      left = pad;
      break;
  }

  sp_for(it, left) sp_io_write_c8(io, fill);
  sp_io_write_str(io, pre, SP_NULLPTR);
  sp_io_write_str(io, str, SP_NULLPTR);
  sp_io_write_str(io, post, SP_NULLPTR);
  sp_for(it, right) sp_io_write_c8(io, fill);
}

static const c8 sp_fmt_digit_pairs[201] =
  "00010203040506070809"
  "10111213141516171819"
  "20212223242526272829"
  "30313233343536373839"
  "40414243444546474849"
  "50515253545556575859"
  "60616263646566676869"
  "70717273747576777879"
  "80818283848586878889"
  "90919293949596979899";

c8* sp_fmt_uint_to_buf_dec(u64 value, c8* buf_end) {
  c8* p = buf_end;
  while (value >= 100) {
    u32 rem = (u32)(value % 100);
    value /= 100;
    p -= 2;
    p[0] = sp_fmt_digit_pairs[rem * 2 + 0];
    p[1] = sp_fmt_digit_pairs[rem * 2 + 1];
  }
  if (value < 10) {
    *--p = (c8)('0' + value);
  } else {
    p -= 2;
    p[0] = sp_fmt_digit_pairs[value * 2 + 0];
    p[1] = sp_fmt_digit_pairs[value * 2 + 1];
  }
  return p;
}

c8* sp_fmt_uint_to_buf_hex_ex(u64 value, c8* buf_end, const c8* digits) {
  c8* p = buf_end;
  if (value == 0) {
    *--p = '0';
    return p;
  }
  while (value) {
    *--p = digits[value & 0xf];
    value >>= 4;
  }
  return p;
}

c8* sp_fmt_uint_to_buf_hex(u64 value, c8* buf_end) {
  return sp_fmt_uint_to_buf_hex_ex(value, buf_end, "0123456789abcdef");
}

void sp_fmt_write_u64_a(sp_io_writer_t* io, u64 value) {
  c8 buf[20];
  c8* end = buf + sizeof(buf);
  c8* start = sp_fmt_uint_to_buf_dec(value, end);
  sp_io_write(io, start, (u64)(end - start), SP_NULLPTR);
}

void sp_fmt_write_s64_a(sp_io_writer_t* io, s64 value) {
  c8 buf[21];
  c8* end = buf + sizeof(buf);
  u64 abs = (value < 0) ? ((u64)(-(value + 1)) + 1) : (u64)value;
  c8* start = sp_fmt_uint_to_buf_dec(abs, end);
  if (value < 0) *--start = '-';
  sp_io_write(io, start, (u64)(end - start), SP_NULLPTR);
}

void sp_fmt_write_ptr_a(sp_io_writer_t* io, void* value) {
  c8 buf[18];
  c8* end = buf + sizeof(buf);
  c8* start = sp_fmt_uint_to_buf_hex((u64)(uintptr_t)value, end);
  *--start = 'x';
  *--start = '0';
  sp_io_write(io, start, (u64)(end - start), SP_NULLPTR);
}

void sp_fmt_write_f64_a(sp_io_writer_t* io, f64 value, u32 precision) {
  union { f64 f; u64 u; } bits;
  bits.f = value;
  u64  exponent = (bits.u >> 52) & 0x7ffULL;
  u64  mantissa = bits.u & 0x000fffffffffffffULL;
  bool is_neg   = (bits.u >> 63) != 0;

  if (exponent == 0x7ff) {
    if (mantissa == 0) {
      sp_io_write_cstr(io, is_neg ? "-inf" : "inf", SP_NULLPTR);
    } else {
      sp_io_write_cstr(io, "nan", SP_NULLPTR);
    }
    return;
  }

  if (is_neg) {
    sp_io_write_c8(io, '-');
    value = -value;
  }

  if (precision > 18) precision = 18;

  static const u64 pow10[] = {
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL,
    10000000ULL, 100000000ULL, 1000000000ULL, 10000000000ULL,
    100000000000ULL, 1000000000000ULL, 10000000000000ULL,
    100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
    100000000000000000ULL, 1000000000000000000ULL,
  };
  u64 scale = pow10[precision];

  if (value >= 1.8446744073709552e19) {
    sp_io_write_cstr(io, "inf", SP_NULLPTR);
    return;
  }

  u64 int_part = (u64)value;
  f64 frac     = value - (f64)int_part;
  u64 frac_scaled = (u64)(frac * (f64)scale + 0.5);
  if (frac_scaled >= scale) {
    int_part += 1;
    frac_scaled -= scale;
  }

  sp_fmt_write_u64_a(io, int_part);

  if (precision > 0) {
    sp_io_write_c8(io, '.');
    c8 frac_buf[18];
    u32 i = precision;
    while (i--) {
      frac_buf[i] = (c8)('0' + (frac_scaled % 10));
      frac_scaled /= 10;
    }
    sp_io_write(io, frac_buf, precision, SP_NULLPTR);
  }
}

void sp_fmt_render_default_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(param);
  switch (arg->id) {
    case sp_fmt_id_u64:
      sp_fmt_write_u64_a(io, arg->u);
      break;
    case sp_fmt_id_s64:
      sp_fmt_write_s64_a(io, arg->i);
      break;
    case sp_fmt_id_f64: {
      u32 p = sp_opt_is_null(arg->spec.precision) ? 6 : sp_opt_get(arg->spec.precision);
      sp_fmt_write_f64_a(io, arg->f, p);
      break;
    }
    case sp_fmt_id_str: {
      sp_str_t s = arg->s;
      if (!sp_opt_is_null(arg->spec.precision)) {
        u32 max = sp_opt_get(arg->spec.precision);
        if (max < s.len) s.len = max;
      }
      sp_io_write_str(io, s, SP_NULLPTR);
      break;
    }
    case sp_fmt_id_ptr:
      sp_fmt_write_ptr_a(io, arg->p);
      break;
    case sp_fmt_id_custom:
      if (arg->custom.fn) {
        arg->custom.fn(io, arg, SP_NULLPTR);
      }
      break;
  }
}
sp_str_t sp_fs_get_name(sp_str_t path) {
  // The only valid separator is '/'. If you're calling this function on a path
  // that might contain a '\', normalize it first. Garbage in, garbage out.
  s32 it = sp_str_find_c8_reverse(path, '/');
  return sp_str_suffix(path, ((s32)path.len) - it - 1);
}

sp_str_t sp_fs_trim_path(sp_str_t path) {
  if (sp_str_empty(path)) return path;

  while (!sp_str_empty(path) && !sp_fs_is_root(path)) {
    switch (sp_str_back(path)) {
      case '/':
      case '\\': {
        path.len--;
        break;
      }
      default: {
        return path;
      }
    }
  }

  return path;
}

sp_str_t sp_fs_parent_path(sp_str_t path) {
  if (sp_str_empty(path)) return path;
  if (sp_fs_is_root(path)) return path;

  path = sp_fs_trim_path(path);

  s32 index = sp_str_find_c8_reverse(path, '/');
  return index == SP_STR_NO_MATCH ? sp_str_lit("") : sp_str_prefix(path, index);
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

  if (sp_str_back(stem) == '.') stem.len--;

  return stem;
}

bool sp_fs_is_root(sp_str_t path) {
  if (path.len == 0) return true;
  if (path.len == 1 && path.data[0] == '/') return true;
  if (path.len == 2 && path.data[1] == ':') return true;
  if (path.len == 3 && path.data[1] == ':' && (path.data[2] == '/' || path.data[2] == '\\')) return true;
  return false;
}

bool sp_fs_is_glob(sp_str_t path) {
  return sp_str_find_c8(path, '*') != SP_STR_NO_MATCH;
}

c8* sp_cstr_from_str_a(sp_mem_t mem, sp_str_t str) {
  c8* buffer = (c8*)sp_mem_allocator_alloc(mem, str.len + 1);
  if (str.len) sp_mem_copy(str.data, buffer, str.len);
  buffer[str.len] = '\0';
  return buffer;
}

sp_str_t sp_str_copy_a(sp_mem_t mem, sp_str_t str) {
  if (!str.data || !str.len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = sp_alloc_n_a(mem, c8, str.len);
  sp_mem_copy(str.data, buffer, str.len);
  return sp_str(buffer, str.len);
}

sp_str_t sp_str_concat_a(sp_mem_t mem, sp_str_t a, sp_str_t b) {
  u32 len = a.len + b.len;
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = (c8*)sp_mem_allocator_alloc(mem, len);
  if (a.len) sp_mem_copy(a.data, buffer,         a.len);
  if (b.len) sp_mem_copy(b.data, buffer + a.len, b.len);
  return SP_STR(buffer, len);
}

sp_str_t sp_str_join_a(sp_mem_t mem, sp_str_t a, sp_str_t b, sp_str_t join) {
  u32 len = a.len + join.len + b.len;
  if (!len) return SP_ZERO_STRUCT(sp_str_t);
  c8* buffer = (c8*)sp_mem_allocator_alloc(mem, len);
  c8* p = buffer;
  if (a.len)    { sp_mem_copy(a.data,    p, a.len);    p += a.len; }
  if (join.len) { sp_mem_copy(join.data, p, join.len); p += join.len; }
  if (b.len)    { sp_mem_copy(b.data,    p, b.len); }
  return SP_STR(buffer, len);
}

sp_str_t sp_str_join_cstr_n_a(sp_mem_t mem, const c8** strings, u32 num_strings, sp_str_t join) {
  if (!strings) num_strings = 0;
  sp_io_writer_t builder = sp_zero();
  sp_io_writer_from_dyn_mem_a(mem, &builder);
  for (u32 index = 0; index < num_strings; index++) {
    sp_io_write_cstr(&builder, strings[index], SP_NULLPTR);

    if (index != (num_strings - 1)) {
      sp_io_write_str(&builder, join, SP_NULLPTR);
    }
  }

  return sp_io_writer_dyn_mem_as_str(&builder.dyn_mem);
}

sp_str_t sp_fs_normalize_path_a(sp_mem_t mem, sp_str_t path) {
  if (sp_str_back(path) == '/' || sp_str_back(path) == '\\') {
    path.len--;
  }

  c8* buffer = (c8*)sp_mem_allocator_alloc(mem, path.len);
  sp_for(i, path.len) {
    c8 c = path.data[i];
    buffer[i] = (c == '\\' ? '/' : c);
  }
  return (sp_str_t) { .data = buffer, .len = path.len };
}

sp_str_t sp_fs_join_path_a(sp_mem_t mem, sp_str_t a, sp_str_t b) {
  a = sp_fs_trim_path(a);
  b = sp_fs_trim_path(b);
  if (sp_str_empty(a)) return sp_str_copy_a(mem, b);
  if (sp_str_empty(b)) return sp_str_copy_a(mem, a);
  return sp_str_join_a(mem, a, b, SP_LIT("/"));
}

sp_str_t sp_fs_replace_ext_a(sp_mem_t mem, sp_str_t path, sp_str_t ext) {
  sp_str_t stripped = sp_str_strip_right(path, sp_fs_get_ext(path));
  return sp_str_empty(ext) ?
    sp_str_copy_a(mem, stripped) :
    sp_str_join_a(mem, path, ext, sp_str_lit("."));
}

sp_fs_kind_t sp_fs_lstat_kind_a(sp_str_t path) {
  if (sp_str_empty(path)) return SP_FS_KIND_NONE;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  s32 rc = sp_sys_lstat(sp_cstr_from_str_a(s.mem, path), &st);
  sp_mem_end_scratch_a(s);
  return rc == 0 ? st.kind : SP_FS_KIND_NONE;
}

sp_fs_kind_t sp_fs_stat_kind_a(sp_str_t path) {
  if (sp_str_empty(path)) return SP_FS_KIND_NONE;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  s32 rc = sp_sys_stat(sp_cstr_from_str_a(s.mem, path), &st);
  sp_mem_end_scratch_a(s);
  return rc == 0 ? st.kind : SP_FS_KIND_NONE;
}

bool sp_fs_exists_a(sp_str_t path)         {
  return sp_fs_stat_kind_a(path)  != SP_FS_KIND_NONE;
}

bool sp_fs_is_file_a(sp_str_t path)        {
  return sp_fs_lstat_kind_a(path) == SP_FS_KIND_FILE;
}

bool sp_fs_is_symlink_a(sp_str_t path)     {
  return sp_fs_lstat_kind_a(path) == SP_FS_KIND_SYMLINK;
}

bool sp_fs_is_dir_a(sp_str_t path)         {
  return sp_fs_lstat_kind_a(path) == SP_FS_KIND_DIR;
}

bool sp_fs_is_target_file_a(sp_str_t path) {
  return sp_fs_stat_kind_a(path)  == SP_FS_KIND_FILE;
}

bool sp_fs_is_target_dir_a(sp_str_t path)  {
  return sp_fs_stat_kind_a(path)  == SP_FS_KIND_DIR;
}

sp_fs_kind_t sp_fs_get_kind_a(sp_str_t path)        {
  return sp_fs_lstat_kind_a(path);
}

sp_fs_kind_t sp_fs_get_target_kind_a(sp_str_t path) {
  return sp_fs_stat_kind_a(path);
}

sp_tm_epoch_t sp_fs_get_mod_time_a(sp_str_t path) {
  sp_tm_epoch_t result = SP_ZERO_STRUCT(sp_tm_epoch_t);
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_stat_t st;
  if (sp_sys_stat(sp_cstr_from_str_a(s.mem, path), &st) == 0) {
    result.s = (u64)st.mtime.tv_sec;
    result.ns = (u32)st.mtime.tv_nsec;
  }
  sp_mem_end_scratch_a(s);
  return result;
}

sp_str_t sp_fs_canonicalize_path_a(sp_mem_t mem, sp_str_t path) {
  if (sp_str_empty(path)) return SP_ZERO_STRUCT(sp_str_t);

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  c8 buf[SP_PATH_MAX];
  s64 len = sp_sys_canonicalize_path(sp_cstr_from_str_a(s.mem, path), buf, SP_PATH_MAX);
  sp_mem_end_scratch_a(s);

  if (len <= 0) return SP_ZERO_STRUCT(sp_str_t);
  sp_str_t canonical = { .data = buf, .len = (u32)len };
  return sp_fs_normalize_path_a(mem, canonical);
}

sp_str_t sp_fs_get_exe_path_a(sp_mem_t mem) {
  c8 buf[SP_PATH_MAX];
  s64 len = sp_sys_get_exe_path(buf, SP_PATH_MAX);
  if (len <= 0) return sp_str_lit("");
  return sp_fs_normalize_path_a(mem, (sp_str_t){ .data = buf, .len = (u32)len });
}

sp_str_t sp_fs_get_cwd_a(sp_mem_t mem) {
  sp_str_t cwd = sp_os_get_cwd();
  return sp_fs_normalize_path_a(mem, cwd);
}

sp_str_t sp_fs_get_storage_path_a(sp_mem_t mem) {
  c8 buf[SP_PATH_MAX];
  s64 len = sp_sys_get_storage_path(buf, SP_PATH_MAX);
  if (len <= 0) return SP_ZERO_STRUCT(sp_str_t);
  return sp_fs_normalize_path_a(mem, (sp_str_t){ .data = buf, .len = (u32)len });
}

sp_str_t sp_fs_get_config_path_a(sp_mem_t mem) {
  c8 buf[SP_PATH_MAX];
  s64 len = sp_sys_get_config_path(buf, SP_PATH_MAX);
  if (len <= 0) return SP_ZERO_STRUCT(sp_str_t);
  return sp_fs_normalize_path_a(mem, (sp_str_t){ .data = buf, .len = (u32)len });
}

//
// fs: io wrappers
//
sp_err_t sp_io_reader_from_file_a(sp_io_reader_t* reader, sp_str_t path) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_fd_t fd = sp_sys_open(sp_cstr_from_str_a(s.mem, path), SP_O_RDONLY | SP_O_BINARY, 0);
  sp_mem_end_scratch_a(s);

  if (fd == SP_SYS_INVALID_FD) {
    *reader = SP_ZERO_STRUCT(sp_io_reader_t);
    return SP_ERR_IO_OPEN_FAILED;
  }

  *reader = (sp_io_reader_t) {
    .vtable = {
      .read = sp_io_reader_file_read,
      .seek = sp_io_reader_file_seek,
      .size = sp_io_reader_file_size,
      .close = sp_io_reader_file_close,
    },
    .file = { .fd = fd, .close_mode = SP_IO_CLOSE_MODE_AUTO },
  };
  return SP_OK;
}

sp_err_t sp_io_writer_from_file_a(sp_io_writer_t* writer, sp_str_t path, sp_io_write_mode_t mode) {
  s32 flags = SP_O_WRONLY | SP_O_CREAT | SP_O_BINARY;
  switch (mode) {
    case SP_IO_WRITE_MODE_OVERWRITE: flags |= SP_O_TRUNC;  break;
    case SP_IO_WRITE_MODE_APPEND:    flags |= SP_O_APPEND; break;
  }

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_sys_fd_t fd = sp_sys_open(sp_cstr_from_str_a(s.mem, path), flags, 0644);
  sp_mem_end_scratch_a(s);

  if (fd == SP_SYS_INVALID_FD) {
    *writer = SP_ZERO_STRUCT(sp_io_writer_t);
    return SP_ERR_IO_OPEN_FAILED;
  }

  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_file_write,
      .seek = sp_io_writer_file_seek,
      .size = sp_io_writer_file_size,
      .close = sp_io_writer_file_close,
    },
    .file = { .fd = fd, .close_mode = SP_IO_CLOSE_MODE_AUTO },
  };
  return SP_OK;
}

sp_err_t sp_io_read_file_a(sp_mem_t mem, sp_str_t path, sp_str_t* content) {
  sp_assert(content);
  sp_err_t err = SP_OK;
  c8* buffer = SP_NULLPTR;
  u64 size = 0;

  sp_io_reader_t reader = sp_zero();
  sp_try(sp_io_reader_from_file_a(&reader, path));

  sp_try_goto(sp_io_reader_size(&reader, &size), err, cleanup);
  if (!size) goto cleanup;

  buffer = sp_alloc_n_a(mem, c8, size);
  u64 bytes_read = 0;
  sp_try_goto(sp_io_read(&reader, buffer, size, &bytes_read), err, cleanup);
  content->data = buffer;
  content->len = (u32)bytes_read;
  buffer = SP_NULLPTR;

cleanup:
  if (buffer) sp_mem_allocator_free(mem, buffer);
  sp_io_reader_close(&reader);
  return err;
}

sp_err_t sp_fs_create_dir_a(sp_str_t path) {
  if (sp_str_empty(path)) return SP_ERR_LAZY;
  if (sp_fs_exists_a(path)) {
    return sp_fs_is_dir_a(path) ? SP_OK : SP_ERR_LAZY;
  }

  sp_err_t result = SP_OK;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_da(sp_str_t) missing = SP_NULLPTR;
  sp_da_init(s.mem, missing);

  // Walk up, collecting intermediate paths that don't exist
  path = sp_fs_trim_path(path);
  while (!sp_fs_is_root(path) && !sp_fs_exists_a(path)) {
    sp_da_push(missing, path);
    path = sp_fs_parent_path(path);
  }

  // Walk back down and create each one
  sp_da_rfor(missing, it) {
    result = sp_os_create_dir(missing[it]);
    if (result && !sp_fs_exists_a(missing[it])) goto cleanup;
  }

cleanup:
  sp_mem_end_scratch_a(s);
  return result;
}

sp_err_t sp_fs_create_file_a(sp_str_t path) {
  return sp_os_create_file(path);
}

sp_err_t sp_fs_create_file_slice_a(sp_str_t path, sp_mem_slice_t slice) {
  sp_try(sp_os_create_file(path));
  sp_io_writer_t io = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file_a(&io, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_try(sp_io_write(&io, slice.data, slice.len, SP_NULLPTR));
  sp_try(sp_io_writer_close(&io));
  return SP_OK;
}

sp_err_t sp_fs_create_file_str_a(sp_str_t path, sp_str_t str) {
  sp_try(sp_os_create_file(path));
  sp_io_writer_t io = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file_a(&io, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_try(sp_io_write_str(&io, str, SP_NULLPTR));
  sp_try(sp_io_writer_close(&io));
  return SP_OK;
}

sp_err_t sp_fs_create_file_cstr_a(sp_str_t path, const c8* str) {
  return sp_fs_create_file_str_a(path, sp_str_view(str));
}

sp_err_t sp_fs_create_hard_link_a(sp_str_t target, sp_str_t link_path) {
  return sp_os_create_hard_link(target, link_path);
}

sp_err_t sp_fs_create_sym_link_a(sp_str_t target, sp_str_t link_path) {
  return sp_os_create_sym_link(target, link_path);
}

sp_err_t sp_fs_link_a(sp_str_t from, sp_str_t to, sp_fs_link_kind_t kind) {
  switch (kind) {
    case SP_FS_LINK_HARD:     return sp_fs_create_hard_link_a(from, to);
    case SP_FS_LINK_SYMBOLIC: return sp_fs_create_sym_link_a(from, to);
    case SP_FS_LINK_COPY:     return sp_fs_copy_a(from, to);
  }
  SP_UNREACHABLE_RETURN(SP_OK);
}

sp_err_t sp_fs_remove_file_a(sp_str_t path) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_err_t err = sp_sys_unlink(sp_cstr_from_str_a(s.mem, path));
  sp_mem_end_scratch_a(s);
  return err;
}

//
// fs: iterator (uses scratch da internally)
//
void sp_fs_it_push_a(sp_fs_it_t* it, sp_str_t path) {
  sp_fs_it_frame_t frame = SP_ZERO_INITIALIZE();
  if (!sp_sys_diriter_open(&frame, path)) return;

  sp_mem_t a = sp_da_allocator(it->stack);
  frame.path = sp_str_copy_a(a, path);
  sp_da_push(it->stack, frame);
}

void sp_fs_it_begin_a(sp_fs_it_t* it, sp_str_t path) {
  if (sp_str_empty(path) || !sp_fs_is_dir_a(path)) return;
  sp_fs_it_push_a(it, path);
  sp_fs_it_next_a(it);
}

void sp_fs_it_next_a(sp_fs_it_t* it) {
  while (!sp_da_empty(it->stack)) {
    sp_fs_it_frame_t* top = sp_da_back(it->stack);

    if (sp_sys_diriter_read(top, &it->entry)) {
      sp_mem_t a = sp_da_allocator(it->stack);
      it->entry.path = sp_fs_join_path_a(a, top->path, it->entry.name);

      if (it->recursive && it->entry.kind == SP_FS_KIND_DIR) {
        sp_fs_it_push_a(it, it->entry.path);
      }
      return;
    }

    sp_sys_diriter_close(top);
    sp_da_pop(it->stack);
  }
}

bool sp_fs_it_valid_a(sp_fs_it_t* it) {
  return !sp_da_empty(it->stack);
}

void sp_fs_it_deinit_a(sp_fs_it_t* it) {
  sp_da_for(it->stack, i) {
    sp_sys_diriter_close(&it->stack[i]);
  }
  sp_da_free(it->stack);
}

sp_da(sp_fs_entry_t) sp_fs_collect_a(sp_mem_t mem, sp_str_t path) {
  sp_da(sp_fs_entry_t) entries = SP_NULLPTR;
  sp_da_init(mem, entries);

  sp_fs_it_t it = SP_ZERO_INITIALIZE();
  sp_da_init(mem, it.stack);
  sp_fs_it_begin_a(&it, path);
  while (sp_fs_it_valid_a(&it)) {
    sp_da_push(entries, it.entry);
    sp_fs_it_next_a(&it);
  }
  sp_fs_it_deinit_a(&it);
  return entries;
}

sp_da(sp_fs_entry_t) sp_fs_collect_recursive_a(sp_mem_t mem, sp_str_t path) {
  sp_da(sp_fs_entry_t) entries = SP_NULLPTR;
  sp_da_init(mem, entries);

  sp_fs_it_t it = { .recursive = true };
  sp_da_init(mem, it.stack);
  sp_fs_it_begin_a(&it, path);
  while (sp_fs_it_valid_a(&it)) {
    sp_da_push(entries, it.entry);
    sp_fs_it_next_a(&it);
  }
  sp_fs_it_deinit_a(&it);
  return entries;
}

sp_err_t sp_fs_remove_dir_a(sp_str_t path) {
  sp_err_t err = SP_OK;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();

  sp_da(sp_fs_entry_t) entries = sp_fs_collect_a(s.mem, path);
  sp_da_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];
    switch (entry->kind) {
      case SP_FS_KIND_FILE:
      case SP_FS_KIND_SYMLINK: err = sp_fs_remove_file_a(entry->path); break;
      case SP_FS_KIND_DIR:     err = sp_fs_remove_dir_a(entry->path);  break;
      case SP_FS_KIND_NONE:    err = SP_ERR_OS;                        break;
    }
    if (err) goto done;
  }

  {
    err = sp_sys_rmdir(sp_cstr_from_str_a(s.mem, path));
  }

done:
  sp_mem_end_scratch_a(s);
  return err;
}

void sp_fs_copy_file_a(sp_str_t from, sp_str_t to) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();

  if (sp_fs_is_dir_a(to)) {
    to = sp_fs_join_path_a(s.mem, to, sp_fs_get_name(from));
  }

  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  if (sp_sys_stat(sp_cstr_from_str_a(s.mem, from), &st)) goto done;

  sp_io_reader_t reader = SP_ZERO_INITIALIZE();
  if (sp_io_reader_from_file_a(&reader, from)) goto done;

  sp_io_writer_t writer = SP_ZERO_INITIALIZE();
  if (sp_io_writer_from_file_a(&writer, to, SP_IO_WRITE_MODE_OVERWRITE)) {
    sp_io_reader_close(&reader);
    goto done;
  }

  u8 buffer[4096];
  while (true) {
    u64 bytes_read = 0;
    sp_err_t err = sp_io_read(&reader, buffer, sizeof(buffer), &bytes_read);
    if (bytes_read) sp_io_write(&writer, buffer, bytes_read, SP_NULLPTR);
    if (err) break;
  }
  sp_io_reader_close(&reader);
  sp_io_writer_close(&writer);
  sp_sys_chmod(sp_cstr_from_str_a(s.mem, to), &st);

done:
  sp_mem_end_scratch_a(s);
}

void sp_fs_copy_glob_a(sp_str_t from, sp_str_t glob, sp_str_t to) {
  sp_fs_create_dir_a(to);

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_da(sp_fs_entry_t) entries = sp_fs_collect_a(s.mem, from);

  sp_da_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];
    sp_str_t entry_name = entry->name;

    bool matches = sp_str_equal(glob, sp_str_lit("*"));
    if (!matches) matches = sp_str_equal(entry_name, glob);

    if (matches) {
      sp_str_t entry_path = sp_fs_join_path_a(s.mem, from, entry_name);
      sp_fs_copy_a(entry_path, to);
    }
  }
  sp_mem_end_scratch_a(s);
}

void sp_fs_copy_dir_a(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir_a(to)) {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
    to = sp_fs_join_path_a(s.mem, to, sp_fs_get_name(from));
    sp_fs_copy_glob_a(from, sp_str_lit("*"), to);
    sp_mem_end_scratch_a(s);
    return;
  }
  sp_fs_copy_glob_a(from, sp_str_lit("*"), to);
}

sp_err_t sp_fs_copy_a(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_glob(from)) {
    sp_fs_copy_glob_a(sp_fs_parent_path(from), sp_fs_get_name(from), to);
  }
  else if (sp_fs_is_target_dir_a(from)) {
    SP_ASSERT(sp_fs_is_target_dir_a(to));
    sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
    sp_fs_copy_glob_a(from, sp_str_lit("*"), sp_fs_join_path_a(s.mem, to, sp_fs_get_name(from)));
    sp_mem_end_scratch_a(s);
  }
  else if (sp_fs_is_target_file_a(from)) {
    sp_fs_copy_file_a(from, to);
  }
  return SP_OK;
}

sp_fs_it_t sp_fs_it_new_a(sp_mem_t mem, sp_str_t path) {
  sp_fs_it_t it = SP_ZERO_INITIALIZE();
  sp_da_init(mem, it.stack);
  sp_fs_it_begin_a(&it, path);
  return it;
}

sp_fs_it_t sp_fs_it_new_recursive_a(sp_mem_t mem, sp_str_t path) {
  sp_fs_it_t it = { .recursive = true };
  sp_da_init(mem, it.stack);
  sp_fs_it_begin_a(&it, path);
  return it;
}

//
// time
//
sp_str_t sp_tm_epoch_to_iso8601_a(sp_mem_t mem, sp_tm_epoch_t time) {
  sp_tm_date_time_t dt = sp_tm_epoch_to_date_time(time);
  c8* buf = (c8*)sp_mem_allocator_alloc(mem, 24);
  // YYYY-MM-DDTHH:MM:SS.mmmZ
  buf[0]  = (c8)('0' + ((u32)dt.year / 1000) % 10);
  buf[1]  = (c8)('0' + ((u32)dt.year / 100) % 10);
  buf[2]  = (c8)('0' + ((u32)dt.year / 10) % 10);
  buf[3]  = (c8)('0' + (u32)dt.year % 10);
  buf[4]  = '-';
  buf[5]  = (c8)('0' + ((u32)dt.month / 10) % 10);
  buf[6]  = (c8)('0' + (u32)dt.month % 10);
  buf[7]  = '-';
  buf[8]  = (c8)('0' + ((u32)dt.day / 10) % 10);
  buf[9]  = (c8)('0' + (u32)dt.day % 10);
  buf[10] = 'T';
  buf[11] = (c8)('0' + ((u32)dt.hour / 10) % 10);
  buf[12] = (c8)('0' + (u32)dt.hour % 10);
  buf[13] = ':';
  buf[14] = (c8)('0' + ((u32)dt.minute / 10) % 10);
  buf[15] = (c8)('0' + (u32)dt.minute % 10);
  buf[16] = ':';
  buf[17] = (c8)('0' + ((u32)dt.second / 10) % 10);
  buf[18] = (c8)('0' + (u32)dt.second % 10);
  buf[19] = '.';
  buf[20] = (c8)('0' + ((u32)dt.millisecond / 100) % 10);
  buf[21] = (c8)('0' + ((u32)dt.millisecond / 10) % 10);
  buf[22] = (c8)('0' + (u32)dt.millisecond % 10);
  buf[23] = 'Z';
  return SP_STR(buf, 24);
}
// @refactor

sp_str_t sp_str_reduce_a(sp_mem_t mem, sp_str_t* strings, u32 num_strings, void* user_data, sp_str_reduce_fn_t fn) {
  sp_io_writer_t io = sp_zero();
  sp_io_writer_from_dyn_mem_a(mem, &io);

  sp_str_reduce_context_t context = {
    .user_data = user_data,
    .writer = &io,
    .elements = {
      .data = strings,
      .len = num_strings,
    },
  };

  sp_for(index, num_strings) {
    context.str = strings[index];
    context.index = index;
    fn(&context);
  }

  return sp_io_writer_dyn_mem_as_str(&io.dyn_mem);
}

void sp_str_reduce_kernel_join_a(sp_str_reduce_context_t* context) {
  if (sp_str_empty(context->str)) return;

  sp_io_write_str(context->writer, context->str, SP_NULLPTR);

  if (context->index != (context->elements.len - 1)) {
    sp_str_t joiner = *(sp_str_t*)context->user_data;
    sp_io_write_str(context->writer, joiner, SP_NULLPTR);
  }
}

sp_str_t sp_str_join_n_a(sp_mem_t mem, sp_str_t* strings, u32 num_strings, sp_str_t joiner) {
  return sp_str_reduce_a(mem, strings, num_strings, &joiner, sp_str_reduce_kernel_join_a);
}

void* sp_ht_alloc(u64 size) {
  sp_context_t* ctx = sp_context_get();
  return sp_mem_allocator_alloc(ctx->allocator, size);
}

SP_END_EXTERN_C()

#endif // SP_SP_C
#endif // SP_IMPLEMENTATION
