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
    + sp_dyn_array     stb-style resizable array (intrusive T* + macros)
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
#if defined(_WIN32)
  #define SP_WIN32

#elif defined(__linux__)
  #if defined(SP_ENABLE_FREESTANDING)
    #define SP_FREESTANDING
  #endif

  #define SP_LINUX
  #define SP_UNIX

#elif defined(__APPLE__)
  #define SP_MACOS
  #define SP_UNIX

#elif __COSMOPOLITAN__
  #define SP_COSMO
  #define SP_UNIX
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



// ███████╗███████╗ █████╗ ████████╗██╗   ██╗██████╗ ███████╗███████╗
// ██╔════╝██╔════╝██╔══██╗╚══██╔══╝██║   ██║██╔══██╗██╔════╝██╔════╝
// █████╗  █████╗  ███████║   ██║   ██║   ██║██████╔╝█████╗  ███████╗
// ██╔══╝  ██╔══╝  ██╔══██║   ██║   ██║   ██║██╔══██╗██╔══╝  ╚════██║
// ██║     ███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║███████╗███████║
// ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝
// @features
//
// There are a few modules that we, unfortunately, haven't implemented without
// libc or POSIX.

#if defined(SP_FREESTANDING)
  #define SP_PS_DISABLE
#endif

#ifndef SP_MUTEX_DISABLE
  #define SP_MUTEX
#endif
#if defined(SP_MUTEX_ENABLE)
  #define SP_MUTEX
#endif

#ifndef SP_CV_DISABLE
  #define SP_CV
#endif
#if defined(SP_CV_ENABLE)
  #define SP_CV
#endif

#ifndef SP_SEMAPHORE_DISABLE
  #define SP_SEMAPHORE
#endif
#if defined(SP_SEMAPHORE_ENABLE)
  #define SP_SEMAPHORE
#endif

#ifndef SP_THREAD_DISABLE
  #define SP_THREAD
#endif
#if defined(SP_THREAD_ENABLE)
  #define SP_THREAD
#endif

#ifndef SP_PS_DISABLE
  #define SP_PS
#endif
#if defined(SP_PS_ENABLE)
  #define SP_PS
#endif

#if defined(SP_LINUX)
  #ifndef SP_SYS_DISABLE
    #define SP_SYS
  #endif
  #if defined(SP_SYS_ENABLE)
    #define SP_SYS
  #endif
#endif

#if defined(SP_MACOS)
  #if !defined(SP_MACHO_DISABLE)
    #define SP_MACHO
  #endif
#endif


// ███╗   ███╗ █████╗  ██████╗██████╗  ██████╗ ███████╗
// ████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔═══██╝██╔════╝
// ██╔████╔██║███████║██║     ██████╔╝██║   ██║███████╗
// ██║╚██╔╝██║██╔══██║██║     ██╔══██╗██║   ██║╚════██║
// ██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║╚██████╔╝███████║
// ╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝
// @macro
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

#if !defined(SP_MSVC)
  #define SP_HAS_ATTRIBUTE(attr) __has_attribute(attr)
#else
  #define SP_HAS_ATTRIBUTE(attr) 0
#endif

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
#define SP_ASSERT(condition) sp_assert((condition))
#define SP_FATAL(FMT, ...) \
  do { \
    sp_str_t message = sp_fmt((FMT), ##__VA_ARGS__); \
    sp_log("{.fg red}: {}", sp_fmt_cstr("SP_FATAL()"), sp_fmt_str(message)); \
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
#define SP_BROKEN() SP_ASSERT(false)
#define sp_broken() SP_BROKEN()
#define SP_ASSERT_FMT(COND, FMT, ...) \
  do { \
    if (!(COND)) { \
      const c8* condition = SP_MACRO_STR(COND); \
      sp_str_t message = sp_fmt((FMT), ##__VA_ARGS__); \
      sp_log("SP_ASSERT({.fg red}): {}", sp_fmt_cstr(condition), sp_fmt_str(message)); \
      SP_EXIT_FAILURE(); \
    } \
  } while (0)
#define sp_assert_fmt(COND, FMT, ...) SP_ASSERT_FMT(COND, FMT, ##__VA_ARGS__)
#define SP_UNTESTED()
#define sp_untested() SP_UNTESTED()
#define SP_INCOMPLETE()
#define sp_incomplete() SP_INCOMPLETE()

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
  #define sp_try_goto(expr, err, label) sp_assert(!expr)
#else
  #define sp_try(expr) do { \
    s32 _sp_result = (expr); \
    if (_sp_result) return _sp_result; \
  } while (0)
  #define sp_try_as(expr, err) do { \
    if (expr) return err; \
  } while (0)
  #define sp_try_as_void(expr) do { \
    if (expr) return; \
  } while (0)
  #define sp_try_goto(expr, err, label) do { \
    err = (expr); \
    if (err) goto label; \
  } while (0)
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

// ██╗███╗   ██╗ ██████╗██╗     ██╗   ██╗██████╗ ███████╗
// ██║████╗  ██║██╔════╝██║     ██║   ██║██╔══██╗██╔════╝
// ██║██╔██╗ ██║██║     ██║     ██║   ██║██║  ██║█████╗
// ██║██║╚██╗██║██║     ██║     ██║   ██║██║  ██║██╔══╝
// ██║██║ ╚████║╚██████╗███████╗╚██████╔╝██████╔╝███████╗
// ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝
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
    #warning "sp.h included after system headers under -std=c99 without _POSIX_C_SOURCE. Threading, semaphores, and process spawning will be unavailable."
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

// #if defined(__STRICT_ANSI__) && defined(_FEATURES_H) && !defined(__USE_POSIX199309)
//     #error "sp.h must be included before system headers under -std=c99, or use -std=gnu99"
//   #endif
// #if defined(SP_POSIX)
//   #ifndef _POSIX_C_SOURCE
//     #define _POSIX_C_SOURCE 200809L
//   #endif
// #endif

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

  extern char** environ;

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

  extern char** environ;

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

#if defined(SP_WIN32)
typedef HANDLE           sp_win32_handle_t;
typedef DWORD            sp_win32_dword_t;
typedef OVERLAPPED       sp_win32_overlapped_t;
typedef WIN32_FIND_DATAA sp_win32_find_data_t;

#endif

typedef struct sp_io_reader sp_io_reader_t;
typedef struct sp_io_writer sp_io_writer_t;

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

#define SP_POLLIN     0x0001
#define SP_POLLPRI    0x0002
#define SP_POLLOUT    0x0004
#define SP_POLLERR    0x0008
#define SP_POLLHUP    0x0010
#define SP_POLLNVAL   0x0020

#define SP_WNOHANG    1
#define SP_WUNTRACED  2

#define SP_WIFEXITED(s)    (((s) & 0x7f) == 0)
#define SP_WEXITSTATUS(s)  (((s) >> 8) & 0xff)
#define SP_WIFSIGNALED(s)  (((s) & 0x7f) != 0 && ((s) & 0x7f) != 0x7f)
#define SP_WTERMSIG(s)     ((s) & 0x7f)
#define SP_WIFSTOPPED(s)   (((s) & 0xff) == 0x7f)
#define SP_WSTOPSIG(s)     SP_WEXITSTATUS(s)

// ███████╗██╗   ██╗███████╗
// ██╔════╝╚██╗ ██╔╝██╔════╝
// ███████╗ ╚████╔╝ ███████╗
// ╚════██║  ╚██╔╝  ╚════██║
// ███████║   ██║   ███████║
// ╚══════╝   ╚═╝   ╚══════╝
// @sys
typedef long sp_word_t;
typedef unsigned long sp_uword_t;
#define __scc(X) ((sp_word_t) (X))

sp_word_t __sp_syscall_ret(sp_uword_t);
sp_word_t __sp_syscall_cp(sp_word_t, sp_word_t, sp_word_t, sp_word_t, sp_word_t, sp_word_t, sp_word_t);
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
#define sp_syscall(...) __sp_syscall_ret(__sp_syscall(__VA_ARGS__))

// SYSCALL NUMBERS
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
  #define SP_SYSCALL_NUM_IOCTL             16
  #define SP_SYSCALL_NUM_FCNTL             72
  #define SP_SYSCALL_NUM_GETCWD            79
  #define SP_SYSCALL_NUM_CHDIR             80
  #define SP_SYSCALL_NUM_RENAME            82
  #define SP_SYSCALL_NUM_MKDIR             83
  #define SP_SYSCALL_NUM_RMDIR             84
  #define SP_SYSCALL_NUM_UNLINK            87
  #define SP_SYSCALL_NUM_READLINK          89
  #define SP_SYSCALL_NUM_CHMOD             90
  #define SP_SYSCALL_NUM_FCHMOD            91
  #define SP_SYSCALL_NUM_SYMLINK           88
  #define SP_SYSCALL_NUM_LINK              86
  #define SP_SYSCALL_NUM_GETTID            186
  #define SP_SYSCALL_NUM_GETDENTS64        217
  #define SP_SYSCALL_NUM_CLOCK_GETTIME     228
  #define SP_SYSCALL_NUM_CLOCK_NANOSLEEP   230
  #define SP_SYSCALL_NUM_EXIT_GROUP        231
  #define SP_SYSCALL_NUM_OPENAT            257
  #define SP_SYSCALL_NUM_MKDIRAT           258
  #define SP_SYSCALL_NUM_NEWFSTATAT        262
  #define SP_SYSCALL_NUM_UNLINKAT          263
  #define SP_SYSCALL_NUM_RENAMEAT          264
  #define SP_SYSCALL_NUM_READLINKAT        267
  #define SP_SYSCALL_NUM_FCHMODAT          268
  #define SP_SYSCALL_NUM_DUP3              292
  #define SP_SYSCALL_NUM_PIPE2             293
  #define SP_SYSCALL_NUM_ARCH_PRCTL        158
  #define SP_SYSCALL_NUM_INOTIFY_INIT1     294
  #define SP_SYSCALL_NUM_INOTIFY_ADD_WATCH 254
  #define SP_SYSCALL_NUM_INOTIFY_RM_WATCH  255

#elif defined(SP_ARM64)
  #define SP_SYSCALL_NUM_GETCWD            17
  #define SP_SYSCALL_NUM_DUP3              24
  #define SP_SYSCALL_NUM_FCNTL             25
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
  #define SP_SYSCALL_NUM_IOCTL             29
  #define SP_SYSCALL_NUM_GETPID            172
  #define SP_SYSCALL_NUM_GETTID            178
  #define SP_SYSCALL_NUM_MUNMAP            215
  #define SP_SYSCALL_NUM_MREMAP            216
  #define SP_SYSCALL_NUM_CLONE             220
  #define SP_SYSCALL_NUM_EXECVE            221
  #define SP_SYSCALL_NUM_MMAP              222
  #define SP_SYSCALL_NUM_WAIT4             260
  #define SP_SYSCALL_NUM_INOTIFY_INIT1     26
  #define SP_SYSCALL_NUM_INOTIFY_ADD_WATCH 27
  #define SP_SYSCALL_NUM_INOTIFY_RM_WATCH  28
  #define SP_SYSCALL_NUM_PPOLL             73
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

////////////////////
// PLATFORM I/O   //
////////////////////
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

typedef struct {
  s64 tv_sec;
  s64 tv_nsec;
} sp_sys_timespec_t;

typedef struct {
  u64 st_dev;
  u64 st_ino;
  u64 st_nlink;
  u32 st_mode;
  u32 st_uid;
  u32 st_gid;
  u32 st_flags;
  u64 st_rdev;
  s64 st_size;
  s64 st_blksize;
  s64 st_blocks;
  sp_sys_timespec_t st_atim;
  sp_sys_timespec_t st_mtim;
  sp_sys_timespec_t st_ctim;
  sp_sys_timespec_t st_birthtim;
  u64 st_gen;
} sp_sys_stat_t;

s64         sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count);
s64         sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count);
sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode);
s32         sp_sys_close(sp_sys_fd_t fd);
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
s32         sp_sys_chmod(const c8* path, s32 mode);

///////////
// TYPES //
///////////
#if defined(SP_LINUX)
typedef struct {
  u64 d_ino;
  s64 d_off;
  u16 d_reclen;
  u8  d_type;
  c8  d_name[];
} sp_sys_dirent64_t;

typedef struct {
  s32 wd;
  u32 mask;
  u32 cookie;
  u32 len;
  c8  name[];
} sp_sys_inotify_event_t;

typedef struct {
  s32 fd;
  s16 events;
  s16 revents;
} sp_sys_pollfd_t;

typedef struct {
  u32 c_iflag;
  u32 c_oflag;
  u32 c_cflag;
  u32 c_lflag;
  u8  c_cc[20];
  u32 _c_ispeed;
  u32 _c_ospeed;
} sp_sys_termios_t;

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

//////////////////////
// SYSCALL WRAPPERS //
//////////////////////
s32   sp_sys_openat(s32 dirfd, const c8* path, s32 flags, s32 mode);
s64   sp_sys_getdents64(s32 fd, void* buf, u64 count);
s32   sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts);
s32   sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem);
s32   sp_sys_pipe2(s32 pipefd[2], s32 flags);
s32   sp_sys_dup2(s32 oldfd, s32 newfd);
s32   sp_sys_ioctl(s32 fd, u64 request, void* argp);
s32   sp_sys_fcntl(s32 fd, s32 cmd, s64 arg);
s32   sp_sys_tcgetattr(s32 fd, sp_sys_termios_t* termios);
s32   sp_sys_tcsetattr(s32 fd, s32 opt, const sp_sys_termios_t* termios);
s32   sp_sys_getpid();
s32   sp_sys_inotify_init1(s32 flags);
s32   sp_sys_inotify_add_watch(s32 fd, const c8* pathname, u32 mask);
s32   sp_sys_inotify_rm_watch(s32 fd, s32 wd);
s32   sp_sys_poll(sp_sys_pollfd_t* fds, u64 nfds, s32 timeout);
s32   sp_sys_wait4(s32 pid, s32* status, s32 options, void* rusage);
void* sp_sys_get_tp();
int   sp_sys_set_tp(void* tp);
void* sp_sys_mmap(void* addr, u64 len, s32 prot, s32 flags, s32 fd, s64 offset);
s32   sp_sys_munmap(void* addr, u64 len);
void* sp_sys_mremap(void* old_addr, u64 old_size, u64 new_size, s32 flags);
void  sp_sys_exit(s32 code);
void  sp_sys_assert(const c8* file, const c8* line, const c8* func, const c8* expr, bool cond);
#endif // SP_LINUX

//////////////
// BUILTINS //
//////////////
#if defined(SP_DEFINE_BUILTINS)
  void* memcpy(void* dest, const void* src, u64 n);
  void* memmove(void* dest, const void* src, u64 n);
  void* memset(void* dest, int c, u64 n);
  s32 memcmp(const void* a, const void* b, u64 n);
  u64 strlen(const char* s);
#endif // SP_DEFINE_BUILTINS

s32   sp_sys_memcmp(const void* vl, const void* vr, u64 n);
void* sp_sys_memset(void* dest, s32 c, u64 n);
void* sp_sys_alloc(u64 n);
void* sp_sys_alloc_zero(u64 n);
void  sp_sys_free(void* ptr);
void* sp_sys_realloc(void* ptr, u64 new_size);
sp_typedef_fn(int, sp_qsort_fn_t, const void *, const void *);
void sp_sys_qsort(void* arr, u64 len, u64 stride, sp_qsort_fn_t);

///////////////
// CONSTANTS //
///////////////
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

#else
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
#endif

//////////////
// SYSCALLS //
//////////////
// Prefer libc for highly tuned stuff where our naive implementation is actively harmful
#if defined(SP_FREESTANDING)
  #define sp_memcpy(d, s, n)                sp_sys_memcpy(d, s, n)
  #define sp_memmove(d, s, n)               sp_sys_memmove(d, s, n)
  #define sp_memset(d, c, n)                sp_sys_memset(d, c, n)
  #define sp_memcmp(a, b, n)                sp_sys_memcmp(a, b, n)
  #define sp_qsort(a, n, s, f)              sp_sys_qsort(a, n, s, f)
#else
  #define sp_memcpy(d, s, n)                memcpy(d, s, n)
  #define sp_memmove(d, s, n)               memmove(d, s, n)
  #define sp_memset(d, c, n)                memset(d, c, n)
  #define sp_memcmp(a, b, n)                memcmp(a, b, n)
  #define sp_qsort(a, n, s, f)              qsort(a, n, s, f)
#endif

#if defined(SP_LINUX)
  typedef sp_sys_inotify_event_t sp_inotify_event_t;
  typedef sp_sys_pollfd_t sp_pollfd_t;
  typedef sp_sys_termios_t sp_termios_t;

  #define sp_assert(x)                      sp_sys_assert(__FILE__, SP_MACRO_STR(__LINE__), __func__, #x, (bool)(x))
  #define sp_openat(d, p, f, m)             sp_sys_openat(d, p, f, m);
  #define sp_poll(fds, n, t)                sp_sys_poll(fds, n, t)
  #define sp_wait4(p, s, o, r)              sp_sys_wait4(p, s, o, r)
  #define sp_inotify_init1(f)               sp_sys_inotify_init1(f)
  #define sp_inotify_rm_watch(f, w)         sp_sys_inotify_rm_watch(f, w)
  #define sp_inotify_add_watch(f, p, m)     sp_sys_inotify_add_watch(f, p, m)
  #define sp_tcgetattr(fd, tio)             sp_sys_tcgetattr(fd, tio)
  #define sp_tcsetattr(fd, opt, tio)        sp_sys_tcsetattr(fd, opt, tio)
#elif defined(SP_WIN32)
  typedef struct { DWORD input_mode; DWORD output_mode; } sp_termios_t;
  #define sp_assert(condition)              assert((condition))
#else
  typedef struct pollfd sp_pollfd_t;
  typedef struct inotify_event sp_inotify_event_t;
  typedef struct termios sp_termios_t;

  #define sp_assert(condition)              assert((condition))
  #define sp_poll(p, f, m)                  poll(p, f, m)
  #define sp_openat(d, p, f, m)             openat(d, p, f, m);
  #define sp_wait4(p, s, o, r)              wait4(p, s, o, r)
  #define sp_tcgetattr(fd, tio)             tcgetattr(fd, tio)
  #define sp_tcsetattr(fd, opt, tio)        tcsetattr(fd, opt, tio)
#endif

///////////////////
// FREESTANDING //
//////////////////
typedef s32 (*sp_entry_fn_t)(s32, const c8**);

void  sp_sys_init();
void  sp_entry_init(s32 argc, const c8** argv, sp_entry_fn_t fn);

#if defined(SP_FREESTANDING)
  extern const c8** sp_envp;
  extern s32 errno;

  #if defined(SP_AMD64)
    #define SP_ENTRY(fn) \
      __attribute__((naked)) void _start(void) { \
        __asm__ volatile ( \
          "xor %%rbp, %%rbp\n" \
          "mov (%%rsp), %%rdi\n" \
          "lea 8(%%rsp), %%rsi\n" \
          "lea " #fn "(%%rip), %%rdx\n" \
          "call sp_entry_init\n" \
          ::: "memory" \
        ); \
      }
  #elif defined(SP_ARM64)
    #define SP_ENTRY(fn) \
      __attribute__((naked)) void _start(void) { \
        __asm__ volatile ( \
          "ldr x0, [sp]\n" \
          "add x1, sp, #8\n" \
          "adrp x2, " #fn "\n" \
          "add x2, x2, :lo12:" #fn "\n" \
          "bl sp_entry_init\n" \
          ::: "memory" \
        ); \
      }
  #endif
#else
  #define SP_ENTRY(fn) s32 main(s32 num_args, const c8** args) { return fn(num_args, args); }
#endif


// ███████╗██████╗ ██████╗  ██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗██╔═══██╗██╔══██╗
// █████╗  ██████╔╝██████╔╝██║   █║██████╔╝
// ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔══██╗
// ███████╗██║  ██║██║  ██║╚██████╔╝██║  ██
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝
// @error
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

typedef s32 sp_os_err_t;

typedef struct {
  sp_err_t sp;
  sp_os_err_t os;
} sp_err_ext_t;

SP_API void sp_err_clear();
SP_API void sp_err_set(sp_err_t err);
SP_API sp_err_t sp_err_get();
SP_API sp_os_err_t sp_err_get_os();
SP_API sp_err_ext_t sp_err_get_ext();


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
  void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr
);

typedef struct sp_allocator_t {
  sp_allocator_fn_t on_alloc;
  void* user_data;
} sp_allocator_t;

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
  sp_allocator_t allocator;
  sp_mem_arena_block_t* head;
  sp_mem_arena_block_t* current;
  u64 block_size;
  sp_mem_arena_mode_t mode;
  u8 alignment;
} sp_mem_arena_t;

typedef struct {
  u64 size;
  u8 padding [8];
} sp_mem_os_header_t;

typedef struct {
  sp_mem_arena_t* arena;
  sp_mem_arena_block_t* block;
  u64 mark;
} sp_mem_arena_marker_t;

typedef struct {
  sp_mem_arena_marker_t marker;
  sp_allocator_t old_allocator;
} sp_mem_scratch_t;

SP_API void*                 sp_alloc(u64 size);
SP_API void*                 sp_realloc(void* memory, u64 size);
SP_API void                  sp_free(void* memory);
SP_API void*                 sp_mem_os_alloc(u64 size);
SP_API void*                 sp_mem_os_alloc_zero(u64 size);
SP_API void*                 sp_mem_os_realloc(void* ptr, u64 size);
SP_API void                  sp_mem_os_free(void* ptr);
SP_API void                  sp_mem_copy(const void* source, void* dest, u64 num_bytes);
SP_API void                  sp_mem_move(const void* source, void* dest, u64 num_bytes);
SP_API bool                  sp_mem_is_equal(const void* a, const void* b, u64 len);
SP_API void                  sp_mem_fill(void* buffer, u64 bsize, void* fill, u64 fsize);
SP_API void                  sp_mem_fill_u8(void* buffer, u64 buffer_size, u8 fill);
SP_API void                  sp_mem_zero(void* buffer, u64 buffer_size);
SP_API void*                 sp_mem_allocator_alloc(sp_allocator_t arena, u64 size);
SP_API void*                 sp_mem_allocator_realloc(sp_allocator_t arena, void* ptr, u64 size);
SP_API void                  sp_mem_allocator_free(sp_allocator_t arena, void* buffer);
SP_API sp_mem_arena_t*       sp_mem_arena_new();
SP_API sp_mem_arena_t*       sp_mem_arena_new_ex(u64 block_size, sp_mem_arena_mode_t mode, u8 alignment);
SP_API sp_allocator_t        sp_mem_arena_as_allocator(sp_mem_arena_t* arena);
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
SP_API sp_allocator_t        sp_mem_os_new();
SP_API void*                 sp_mem_os_on_alloc(void* ud, sp_mem_alloc_mode_t mode, u64 size, void* ptr);
SP_API sp_mem_os_header_t*   sp_mem_os_get_header(void* ptr);
SP_API sp_mem_arena_t*       sp_mem_get_scratch_arena();
SP_API sp_mem_scratch_t      sp_mem_begin_scratch();
SP_API void                  sp_mem_end_scratch(sp_mem_scratch_t scratch);

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
#define sp_da_sort(_ARR, fn)        sp_dyn_array_sort(_ARR, fn)

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

#define sp_dyn_array_sort(arr, fn) sp_qsort(arr, sp_dyn_array_size(arr), sizeof((arr)[0]), fn)
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
  sp_allocator_t allocator;
  u64 tmp_idx;
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

#define __sp_ht_entry(__K, __V)                \
    struct                                     \
    {                                          \
        __K key;                               \
        __V val;                               \
        sp_ht_entry_state state;               \
    }

#define sp_ht(__K, __V)                        \
    struct {                                   \
        __sp_ht_entry(__K, __V)* data;         \
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
  do { \
    sp_ht_ensure(ht); \
    sp_ht_insert_ex(ht, (k), v); \
  } while (0)

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

// ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗
// ██╔════╝╚══██╔══╝██╔═██╗██║████╗  ██║██╔════╝
// ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗
// ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║
// ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝
// @string
typedef struct {
  u8* data;
  u64 len;
  u64 capacity;
} sp_mem_buffer_t;

typedef struct {
  const c8* data;
  u32 len;
} sp_str_t;

typedef struct {
  sp_io_writer_t* writer;

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
SP_API sp_str_t        sp_str_join_n(sp_str_t* strs, u32 n, sp_str_t joiner);
SP_API sp_da(sp_str_t) sp_str_pad_to_longest(sp_str_t* strs, u32 n);
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
SP_API c8*             sp_cstr_copy(const c8* str);
SP_API void            sp_cstr_copy_to(const c8* str, c8* buffer, u32 buffer_length);
SP_API c8*             sp_cstr_copy_sized(const c8* str, u32 length);
SP_API void            sp_cstr_copy_to_sized(const c8* str, u32 n, c8* buffer, u32 bn);
SP_API bool            sp_cstr_equal(const c8* a, const c8* b);
SP_API u32             sp_cstr_len(const c8* str);
SP_API u32             sp_cstr_len_sized(const c8* str, u32 n);
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
SP_API c8              sp_c8_to_upper(c8 c);
SP_API c8              sp_c8_to_lower(c8 c);
SP_API sp_str_t        sp_str_reduce(sp_str_t* strs, u32 n, void* ud, sp_str_reduce_fn_t fn);
SP_API void            sp_str_reduce_kernel_join(sp_str_reduce_context_t* context);
SP_API sp_da(sp_str_t) sp_str_map(sp_str_t* s, u32 n, void* ud, sp_str_map_fn_t fn);
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
SP_API void            sp_str_builder_indent(sp_str_builder_t* builder);
SP_API void            sp_str_builder_dedent(sp_str_builder_t* builder);
SP_API void            sp_str_builder_append(sp_str_builder_t* builder, sp_str_t str);
SP_API void            sp_str_builder_append_cstr(sp_str_builder_t* builder, const c8* str);
SP_API void            sp_str_builder_append_c8(sp_str_builder_t* builder, c8 c);
SP_API void            sp_str_builder_append_fmt_str(sp_str_builder_t* builder, sp_str_t fmt, ...);
SP_API void            sp_str_builder_append_fmt(sp_str_builder_t* builder, const c8* fmt, ...);
SP_API void            sp_str_builder_new_line(sp_str_builder_t* builder);
SP_API u64             sp_str_builder_len(sp_str_builder_t* builder);
SP_API sp_str_t        sp_str_builder_as_str(sp_str_builder_t* builder);
SP_API sp_str_t        sp_str_builder_to_str(sp_str_builder_t* builder);
SP_API sp_mem_buffer_t sp_str_builder_into_buffer(sp_str_builder_t* builder);
SP_API void            sp_str_builder_free(sp_str_builder_t* builder);
SP_API sp_str_builder_t sp_str_builder_from_writer(sp_io_writer_t* writer);


// ███████╗██╗     ██╗ ██████╗███████╗
// ██╔════╝██║     ██║██╔════╝██╔════╝
// ███████╗██║     ██║██║     █████╗
// ╚════██║██║     ██║██║     ██╔══╝
// ███████║███████╗██║╚██████╗███████╗
// ╚══════╝╚══════╝╚═╝ ╚═════╝╚══════╝
// @slice
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

// ██╗      ██████╗  ██████╗
// ██║     ██╔═══██╗██╔════╝
// ██║     ██║   ██║██║  ███╗
// ██║     ██║   ██║██║   ██║
// ███████╗╚██████╔╝╚██████╔╝
// ╚══════╝ ╚═════╝  ╚═════╝
// @log
SP_API void sp_log(const c8* fmt, ...);
SP_API void sp_log_err(const c8* fmt, ...);
SP_API void sp_log_str(sp_str_t fmt, ...);
SP_API void sp_print(const c8* fmt, ...);
SP_API void sp_print_err(const c8* fmt, ...);
SP_API void sp_print_str(sp_str_t fmt, ...);


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

typedef sp_str_ht(sp_str_t) sp_env_table_t;

typedef struct {
  sp_env_table_t vars;
} sp_env_t;

SP_API void     sp_env_init(sp_env_t* env);
SP_API sp_env_t sp_env_capture();
SP_API sp_env_t sp_env_copy(sp_env_t* env);
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

typedef enum {
  SP_FS_KIND_NONE = 0,
  SP_FS_KIND_FILE,
  SP_FS_KIND_DIR,
  SP_FS_KIND_SYMLINK,
} sp_fs_kind_t;

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
  u8 buf[SP_FS_IT_BUF_SIZE] __attribute__((aligned(__alignof__(sp_sys_dirent64_t))));
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

SP_API bool          sp_fs_is_file(sp_str_t path);
SP_API bool          sp_fs_is_symlink(sp_str_t path);
SP_API bool          sp_fs_is_dir(sp_str_t path);
SP_API bool          sp_fs_is_target_file(sp_str_t path);
SP_API bool          sp_fs_is_target_dir(sp_str_t path);
SP_API bool          sp_fs_is_root(sp_str_t path);
SP_API bool          sp_fs_is_glob(sp_str_t path);
SP_API bool          sp_fs_exists(sp_str_t path);
SP_API sp_err_t      sp_fs_create_dir(sp_str_t path);
SP_API sp_err_t      sp_fs_create_file(sp_str_t path);
SP_API sp_err_t      sp_fs_create_file_slice(sp_str_t path, sp_mem_slice_t slice);
SP_API sp_err_t      sp_fs_create_file_str(sp_str_t path, sp_str_t str);
SP_API sp_err_t      sp_fs_create_file_cstr(sp_str_t path, const c8* str);
SP_API void          sp_fs_remove_dir(sp_str_t path);
SP_API void          sp_fs_remove_file(sp_str_t path);
SP_API sp_err_t      sp_fs_copy(sp_str_t from, sp_str_t to);
SP_API void          sp_fs_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to);
SP_API void          sp_fs_copy_file(sp_str_t from, sp_str_t to);
SP_API void          sp_fs_copy_dir(sp_str_t from, sp_str_t to);
SP_API sp_err_t      sp_fs_link(sp_str_t from, sp_str_t to, sp_fs_link_kind_t kind);
SP_API sp_err_t      sp_fs_create_hard_link(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t      sp_fs_create_sym_link(sp_str_t target, sp_str_t link_path);
SP_API sp_str_t      sp_fs_normalize_path(sp_str_t path);
SP_API sp_str_t      sp_fs_parent_path(sp_str_t path);
SP_API sp_str_t      sp_fs_join_path(sp_str_t a, sp_str_t b);
SP_API sp_str_t      sp_fs_trim_path(sp_str_t path);
SP_API sp_str_t      sp_fs_replace_ext(sp_str_t path, sp_str_t ext);
SP_API sp_str_t      sp_fs_get_ext(sp_str_t path);
SP_API sp_str_t      sp_fs_get_stem(sp_str_t path);
SP_API sp_str_t      sp_fs_get_name(sp_str_t path);
SP_API sp_str_t      sp_fs_get_cwd();
SP_API sp_str_t      sp_fs_get_exe_path();
SP_API sp_str_t      sp_fs_get_storage_path();
SP_API sp_str_t      sp_fs_get_config_path();
SP_API sp_tm_epoch_t sp_fs_get_mod_time(sp_str_t path);
SP_API sp_fs_kind_t  sp_fs_get_kind(sp_str_t path);
SP_API sp_fs_kind_t  sp_fs_get_target_kind(sp_str_t path);
SP_API sp_fs_it_t    sp_fs_it_new_recursive(sp_str_t path);
SP_API sp_fs_it_t    sp_fs_it_new(sp_str_t path);
SP_API void          sp_fs_it_begin(sp_fs_it_t* it, sp_str_t path);
SP_API void          sp_fs_it_next(sp_fs_it_t* it);
SP_API bool          sp_fs_it_valid(sp_fs_it_t* it);
SP_API void          sp_fs_it_deinit(sp_fs_it_t* it);
SP_API sp_da(sp_fs_entry_t) sp_fs_collect(sp_str_t path);
SP_API sp_da(sp_fs_entry_t) sp_fs_collect_recursive(sp_str_t path);

#define sp_fs_for(dir, it) for (sp_fs_it_t it = sp_fs_it_new(dir); sp_fs_it_valid(&it); sp_fs_it_next(&it))
#define sp_fs_for_recursive(dir, it) for (sp_fs_it_t it = sp_fs_it_new_recursive(dir); sp_fs_it_valid(&it); sp_fs_it_next(&it))

// Move to shared impl
SP_API sp_str_t sp_fs_canonicalize_path(sp_str_t path);


//  ██████╗ ██████╗ ███╗   ██╗ ██████╗██╗   ██╗██████╗ ██████╗ ███████╗███╗   ██╗ ██████╗██╗   ██╗
// ██╔════╝██╔═══██╗████╗  ██║██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝████╗  ██║██╔════╝╚██╗ ██╔╝
// ██║     ██║   ██║██╔██╗ ██║██║     ██║   ██║██████╔╝██████╔╝█████╗  ██╔██╗ ██║██║      ╚████╔╝
// ██║     ██║   ██║██║╚██╗██║██║     ██║   ██║██╔══██╗██╔══██╗██╔══╝  ██║╚██╗██║██║       ╚██╔╝
// ╚██████╗╚██████╔╝██║ ╚████║╚██████╗╚██████╔╝██║  ██║██║  ██║███████╗██║ ╚████║╚██████╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝ ╚═════╝   ╚═╝
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
  SP_OS_SIGNAL_INTERRUPT,
  SP_OS_SIGNAL_TERMINATE,
  SP_OS_SIGNAL_ABORT,
  SP_OS_SIGNAL_COUNT_,
} sp_os_signal_t;

sp_typedef_fn(void, sp_os_signal_handler_t, sp_os_signal_t signal, void* userdata);

typedef u32 sp_os_attr_t;
typedef sp_sys_fd_t sp_os_file_handle_t;

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
SP_API sp_err_t       sp_os_remove_file(sp_str_t path);
SP_API sp_err_t       sp_os_remove_dir(sp_str_t path);
SP_API sp_err_t       sp_os_create_hard_link(sp_str_t target, sp_str_t link_path);
SP_API sp_err_t       sp_os_create_sym_link(sp_str_t target, sp_str_t link_path);
SP_API sp_str_t       sp_os_canonicalize_path(sp_str_t path);
SP_API sp_str_t       sp_os_get_cwd();
SP_API sp_str_t       sp_os_get_exe_path();
SP_API sp_fs_kind_t   sp_os_get_file_attrs(sp_str_t path);
SP_API sp_fs_kind_t   sp_os_get_target_attrs(sp_str_t path);
SP_API sp_os_attr_t   sp_os_get_raw_file_attrs(sp_str_t path);
SP_API sp_os_attr_t   sp_os_get_raw_target_attrs(sp_str_t path);
SP_API sp_err_t       sp_os_set_raw_file_attrs(sp_str_t path, sp_os_attr_t attrs);
SP_API void           sp_os_register_signal_handler(sp_os_signal_t, sp_os_signal_handler_t, void* userdata);
SP_API bool           sp_os_is_tty(sp_sys_fd_t fd);
SP_API void           sp_os_tty_size(sp_sys_fd_t fd, s32* cols, s32* rows);

// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║ ██║   ██║
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝
// @format
sp_str_t sp_fmt(const c8* fmt, ...);
sp_err_t sp_fmt_e(sp_str_t* str, const c8* fmt, ...);
sp_err_t sp_fmt_v(sp_str_t* str, sp_str_t fmt, va_list args);

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
SP_TYPEDEF_FN(void, sp_fmt_fn_t, sp_str_builder_t*, sp_fmt_arg_t*, sp_fmt_arg_t*);
SP_TYPEDEF_FN(void, sp_fmt_transform_fn_t, sp_str_builder_t*, sp_str_t, sp_fmt_arg_t*, sp_fmt_arg_t*);

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
      .fn = (sp_fmt_fn_t)(_fn),        \
      .ptr = (void*)&(_value)             \
    }                                     \
  })
#define sp_fmt_custom_v(T, _fn, ...) (   \
  (void)sizeof((T)sp_zero() = (__VA_ARGS__)),  \
  (sp_fmt_arg_t) {                     \
    .id = sp_fmt_id_custom,            \
    .custom = {                           \
      .fn = (sp_fmt_fn_t)(_fn),        \
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
SP_API void sp_fmt_render_default(sp_str_builder_t* builder, sp_fmt_arg_t* arg, sp_fmt_arg_t* param);

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

#if defined(SP_WIN32)
  typedef sp_win32_dword_t sp_tls_key_t;
  typedef INIT_ONCE sp_tls_once_t;
#elif defined(SP_FREESTANDING)
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
  sp_allocator_t allocator;
  sp_err_ext_t err;
} sp_context_t;

typedef struct {
  sp_context_t contexts [SP_RT_MAX_CONTEXT];
  u32 index;
  sp_mem_arena_t* scratch;
  sp_env_t env;
  struct {
    sp_str_ht(sp_fmt_directive_t) directives;
  } format;
  struct {
    sp_io_writer_t* out;
    sp_io_writer_t* err;
  } std;
} sp_tls_rt_t;

typedef struct {
  sp_os_signal_handler_t signal_handlers[SP_OS_SIGNAL_COUNT_];
  void*                  signal_userdata[SP_OS_SIGNAL_COUNT_];
  sp_mutex_t mutex;
  sp_spin_lock_t locks [SP_RT_NUM_SPIN_LOCKS];
  struct {
    sp_tls_key_t key;
    sp_tls_once_t once;
  } tls;
} sp_rt_t;

sp_tls_rt_t*  sp_tls_rt_get();
sp_context_t* sp_context_get();
void          sp_context_set(sp_context_t context);
void          sp_context_push(sp_context_t context);
void          sp_context_push_allocator(sp_allocator_t allocator);
void          sp_context_push_arena(sp_mem_arena_t* arena);
void          sp_context_pop();

extern sp_rt_t sp_rt;


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
  SP_IO_CLOSE_MODE_NONE,
  SP_IO_CLOSE_MODE_AUTO,
} sp_io_close_mode_t;

typedef struct {
  sp_os_file_handle_t fd;
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
  sp_allocator_t allocator;
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
SP_API sp_err_t       sp_io_read_file(sp_str_t path, sp_str_t* content);
SP_API sp_err_t       sp_io_reader_seek(sp_io_reader_t* reader, s64 offset, sp_io_whence_t whence, s64* position);
SP_API sp_err_t       sp_io_reader_size(sp_io_reader_t* r, u64* size);
SP_API sp_err_t       sp_io_reader_close(sp_io_reader_t* r);
SP_API sp_err_t       sp_io_reader_from_file(sp_io_reader_t* reader, sp_str_t path);
SP_API void           sp_io_reader_from_fd(sp_io_reader_t* reader, sp_os_file_handle_t fd, sp_io_close_mode_t mode);
SP_API void           sp_io_reader_from_mem(sp_io_reader_t* reader, const void* ptr, u64 size);
SP_API void           sp_io_reader_set_buffer(sp_io_reader_t* reader, u8* buf, u64 capacity);
SP_API sp_err_t       sp_io_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written);
SP_API sp_err_t       sp_io_write_str(sp_io_writer_t* writer, sp_str_t str, u64* bytes_written);
SP_API sp_err_t       sp_io_write_cstr(sp_io_writer_t* writer, const c8* cstr, u64* bytes_written);
SP_API sp_err_t       sp_io_pad(sp_io_writer_t* writer, u64 size, u64* bytes_written);
SP_API sp_err_t       sp_io_flush(sp_io_writer_t* w);
SP_API sp_err_t       sp_io_writer_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position);
SP_API sp_err_t       sp_io_writer_size(sp_io_writer_t* w, u64* size);
SP_API sp_err_t       sp_io_writer_close(sp_io_writer_t* w);
SP_API sp_err_t       sp_io_writer_from_file(sp_io_writer_t* writer, sp_str_t path, sp_io_write_mode_t mode);
SP_API void           sp_io_writer_from_fd(sp_io_writer_t* writer, sp_os_file_handle_t fd, sp_io_close_mode_t close_mode);
SP_API void           sp_io_writer_from_mem(sp_io_writer_t* writer, void* ptr, u64 size);
SP_API void           sp_io_writer_from_dyn_mem(sp_io_writer_t* writer);
SP_API void           sp_io_writer_from_dyn_mem_ex(sp_io_writer_t* writer, u8* buffer, u64 size, sp_allocator_t allocator);
SP_API void           sp_io_get_std_out(sp_io_writer_t* io);
SP_API void           sp_io_get_std_err(sp_io_writer_t* io);
SP_API sp_err_t       sp_io_writer_set_buffer(sp_io_writer_t* writer, u8* buf, u64 capacity);


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
  .in  = { .mode = SP_PS_IO_MODE_NULL }, \
  .out = { .mode = SP_PS_IO_MODE_NULL }, \
  .err = { .mode = SP_PS_IO_MODE_NULL }, \
}

typedef struct {
  sp_os_file_handle_t fd;
  sp_ps_io_mode_t mode;
  sp_ps_io_blocking_t block;
} sp_ps_io_in_config_t;

typedef struct {
  sp_os_file_handle_t fd;
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
  sp_allocator_t allocator;
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


//  █████╗ ██████╗ ██████╗
// ██╔══██╗██╔══██╗██╔══██╗
// ███████║██████╔╝██████╔╝
// ██╔══██║██╔═══╝ ██╔═══╝
// ██║  ██║██║     ██║
// ╚═╝  ╚═╝╚═╝     ╚═╝
// @app
typedef enum {
  SP_APP_CONTINUE = 0,
  SP_APP_ERR = 1,
  SP_APP_QUIT = 2
} sp_app_result_t;

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


// ███╗   ███╗ ██████╗ ███╗   ██╗██╗████████╗ ██████╗ ██████╗
// ████╗ ████║██╔═══██╗████╗  ██║██║╚══██╔══╝██╔═══██╗██╔══██╗
// ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║   ██║   ██║██████╔╝
// ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║   ██║   ██║██╔══██╗
// ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
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
  sp_allocator_t allocator;
};

SP_API void sp_fmon_init(sp_fmon_t* m, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* user_data);
SP_API void sp_fmon_init_ex(sp_fmon_t* m, sp_fmon_fn_t fn, sp_fmon_event_kind_t events, void* user_data, u32 debounce, sp_allocator_t alloc);
SP_API void sp_fmon_deinit(sp_fmon_t* monitor);
SP_API void sp_fmon_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_API void sp_fmon_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_API void sp_fmon_process_changes(sp_fmon_t* monitor);
SP_API void sp_fmon_emit_changes(sp_fmon_t* monitor);

SP_END_EXTERN_C()

#ifdef SP_CPP
SP_API sp_str_t operator/(const sp_str_t& a, const sp_str_t& b);
SP_API sp_str_t operator/(const sp_str_t& a, const c8* b);
#endif // SP_CPP

#endif // SP_SP_H


// @implementation
#ifndef SP_SP_C
#ifdef SP_IMPLEMENTATION
#define SP_SP_C

SP_BEGIN_EXTERN_C()

sp_rt_t sp_rt;
#if defined(SP_FREESTANDING)
const c8** sp_envp;
s32 errno;
sp_tls_block_t sp_tls_block;
#endif

// ███████╗██╗   ██╗███████╗
// ██╔════╝╚██╗ ██╔╝██╔════╝
// ███████╗ ╚████╔╝ ███████╗
// ╚════██║  ╚██╔╝  ╚════██║
// ███████║   ██║   ███████║
// ╚══════╝   ╚═╝   ╚══════╝
// @sys
#if defined(SP_LINUX)
sp_word_t __sp_syscall_ret(sp_uword_t r) {
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
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

void* sp_sys_get_tp(void) {
  void* tp;
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

void* sp_sys_memcpy(void* dest, const void* src, size_t n) {
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

void* sp_sys_memmove(void* dest, const void* src, size_t n) {
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
      while (n >= 8) { *(u64*)d = *(const u64*)s; d += 8; s += 8; n -= 8; }
    }
    while (n--) *d++ = *s++;
  } else {
    if ((uintptr_t)s % 8 == (uintptr_t)d % 8) {
      while ((uintptr_t)(d + n) % 8) { if (!n--) return dest; d[n] = s[n]; }
      while (n >= 8) { n -= 8; *(u64*)(d + n) = *(const u64*)(s + n); }
    }
    while (n--) d[n] = s[n];
  }
  return dest;
#endif
}

//////////////
// BUILTINS //
//////////////
#if defined(SP_DEFINE_BUILTINS)
void* memcpy(void* dest, const void* src, u64 n) {
  return sp_sys_memcpy(dest, src, n);
}

void* memmove(void* dest, const void* src, u64 n) {
  return sp_sys_memmove(dest, src, n);
}

void* memset(void* dest, s32 c, u64 n) {
  return sp_sys_memset(dest, c, n);
}

s32 memcmp(const void* a, const void* b, u64 n) {
  return sp_sys_memcmp(a, b, n);
}

u64 strlen(const char* s) {
  u64 len = 0;
  while (s[len]) len++;
  return len;
}
#endif

#define SP_SYS_ALLOC_ALIGN    16
#define SP_SYS_ALLOC_HEADER   16

void* sp_sys_alloc(u64 n) {
  if (n == 0) n = 1;
  n = (n + SP_SYS_ALLOC_ALIGN - 1) & ~(SP_SYS_ALLOC_ALIGN - 1);
  u64 total = n + SP_SYS_ALLOC_HEADER;
  void* p = sp_sys_mmap(0, total, SP_PROT_READ | SP_PROT_WRITE, SP_MAP_PRIVATE | SP_MAP_ANONYMOUS, -1, 0);
  if (p == SP_MAP_FAILED) return 0;
  *(u64*)p = total;
  return (char*)p + SP_SYS_ALLOC_HEADER;
}

void* sp_sys_alloc_zero(u64 n) {
  return sp_sys_alloc(n);
}

void sp_sys_free(void* ptr) {
  if (!ptr) return;
  void* base = (char*)ptr - SP_SYS_ALLOC_HEADER;
  u64 total = *(u64*)base;
  sp_sys_munmap(base, total);
}

void* sp_sys_realloc(void* ptr, u64 new_size) {
  if (!ptr) return sp_sys_alloc(new_size);
  if (new_size == 0) { sp_sys_free(ptr); return 0; }
  void* base = (char*)ptr - SP_SYS_ALLOC_HEADER;
  u64 old_total = *(u64*)base;
  new_size = (new_size + SP_SYS_ALLOC_ALIGN - 1) & ~(SP_SYS_ALLOC_ALIGN - 1);
  u64 new_total = new_size + SP_SYS_ALLOC_HEADER;
  void* new_base = sp_sys_mremap(base, old_total, new_total, 1);
  if (new_base != SP_MAP_FAILED) {
    *(u64*)new_base = new_total;
    return (char*)new_base + SP_SYS_ALLOC_HEADER;
  }
  void* new_ptr = sp_sys_alloc(new_size);
  if (!new_ptr) return 0;
  u64 old_size = old_total - SP_SYS_ALLOC_HEADER;
  u64 copy_size = old_size < new_size ? old_size : new_size;
  u8* d = (u8*)new_ptr;
  u8* s = (u8*)ptr;
  for (u64 i = 0; i < copy_size; i++) d[i] = s[i];
  sp_sys_free(ptr);
  return new_ptr;
}

s32 sp_sys_memcmp(const void* vl, const void* vr, u64 n) {
  const u8* l = (const u8*)vl;
  const u8* r = (const u8*)vr;
  while (n >= 8) {
    u64 lv = *(const u64*)l;
    u64 rv = *(const u64*)r;
    if (lv != rv) break;
    l += 8; r += 8; n -= 8;
  }
  for (; n && *l == *r; n--, l++, r++);
  return n ? *l - *r : 0;
}

void* sp_sys_memset(void* dest, s32 c, u64 n) {
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

void sp_sys_qsort(void *arr, u64 len, u64 stride, sp_qsort_fn_t cmp) {
  u8 *a = arr;
  u64 gap, i, j;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  u8* tmp = sp_alloc_n(u8, stride);

  for (gap = len / 3; gap > 0; gap /= 3 + 1) {
    for (i = gap; i < len; i++) {
      sp_memcpy(tmp, a + i * stride, stride);
      for (j = i; j >= gap && cmp(a + (j - gap) * stride, tmp) > 0; j -= gap) {
        sp_memcpy(a + j * stride, a + (j - gap) * stride, stride);
      }
      sp_memcpy(a + j * stride, tmp, stride);
    }
  }

  sp_mem_end_scratch(scratch);
}

//////////////////////
// SYSCALL WRAPPERS //
//////////////////////
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  return sp_syscall(SP_SYSCALL_NUM_READ, fd, buf, count);
}

s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  return sp_syscall(SP_SYSCALL_NUM_WRITE, fd, buf, count);
}

sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
#if defined(SP_AMD64)
  return (sp_sys_fd_t)sp_syscall(SP_SYSCALL_NUM_OPEN, path, flags, mode);
#else
  return (sp_sys_fd_t)sp_syscall(SP_SYSCALL_NUM_OPENAT, SP_AT_FDCWD, path, flags, mode);
#endif
}

s32 sp_sys_openat(s32 dirfd, const c8* path, s32 flags, s32 mode) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_OPENAT, dirfd, path, flags, mode);
}

s32 sp_sys_close(sp_sys_fd_t fd) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CLOSE, fd);
}

s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  return sp_syscall(SP_SYSCALL_NUM_LSEEK, fd, offset, whence);
}

static void sp_sys_stat_from_linux(const sp_sys_linux_stat_t* raw, sp_sys_stat_t* out) {
  out->st_dev            = raw->st_dev;
  out->st_ino            = raw->st_ino;
  out->st_nlink          = raw->st_nlink;
  out->st_mode           = raw->st_mode;
  out->st_uid            = raw->st_uid;
  out->st_gid            = raw->st_gid;
  out->st_flags          = 0;
  out->st_rdev           = raw->st_rdev;
  out->st_size           = raw->st_size;
  out->st_blksize        = raw->st_blksize;
  out->st_blocks         = raw->st_blocks;
  out->st_atim.tv_sec    = raw->st_atime_sec;
  out->st_atim.tv_nsec   = raw->st_atime_nsec;
  out->st_mtim.tv_sec    = raw->st_mtime_sec;
  out->st_mtim.tv_nsec   = raw->st_mtime_nsec;
  out->st_ctim.tv_sec    = raw->st_ctime_sec;
  out->st_ctim.tv_nsec   = raw->st_ctime_nsec;
  out->st_birthtim       = out->st_ctim;
  if (out->st_atim.tv_sec < out->st_ctim.tv_sec) out->st_birthtim = out->st_atim;
  if (out->st_mtim.tv_sec < out->st_ctim.tv_sec) out->st_birthtim = out->st_mtim;
  out->st_gen            = 0;
}

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

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  sp_sys_linux_stat_t raw = SP_ZERO_INITIALIZE();
  s32 rc = (s32)sp_syscall(SP_SYSCALL_NUM_FSTAT, fd, &raw);
  if (rc == 0) sp_sys_stat_from_linux(&raw, st);
  return rc;
}

s32 sp_sys_mkdir(const c8* path, s32 mode) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_MKDIR, path, mode);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_MKDIRAT, SP_AT_FDCWD, path, mode);
#endif
}

s32 sp_sys_rmdir(const c8* path) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_RMDIR, path);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINKAT, SP_AT_FDCWD, path, SP_AT_REMOVEDIR);
#endif
}

s32 sp_sys_unlink(const c8* path) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINK, path);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_UNLINKAT, SP_AT_FDCWD, path, 0);
#endif
}

s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_RENAME, oldpath, newpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_RENAMEAT, SP_AT_FDCWD, oldpath, SP_AT_FDCWD, newpath);
#endif
}

s64 sp_sys_getcwd(char* buf, u64 size) {
  return sp_syscall(SP_SYSCALL_NUM_GETCWD, buf, size);
}

s32 sp_sys_chdir(const c8* path) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CHDIR, path);
}

s64 sp_sys_getdents64(s32 fd, void* buf, u64 count) {
  return sp_syscall(SP_SYSCALL_NUM_GETDENTS64, fd, buf, count);
}

s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_SYMLINK, target, linkpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_SYMLINKAT, target, SP_AT_FDCWD, linkpath);
#endif
}

s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_LINK, oldpath, newpath);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_LINKAT, SP_AT_FDCWD, oldpath, SP_AT_FDCWD, newpath, 0);
#endif
}

s32 sp_sys_chmod(const c8* path, s32 mode) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_CHMOD, path, mode);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_FCHMODAT, SP_AT_FDCWD, path, mode, 0);
#endif
}

s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_CLOCK_GETTIME, clockid, ts);
}

s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_NANOSLEEP, req, rem);
}

s32 sp_sys_pipe2(s32 pipefd[2], s32 flags) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_PIPE2, pipefd, flags);
}

s32 sp_sys_dup2(s32 oldfd, s32 newfd) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_DUP2, oldfd, newfd);
#else
  return (s32)sp_syscall(SP_SYSCALL_NUM_DUP3, oldfd, newfd, 0);
#endif
}

s32 sp_sys_ioctl(s32 fd, u64 request, void* argp) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_IOCTL, fd, request, argp);
}

s32 sp_sys_fcntl(s32 fd, s32 cmd, s64 arg) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_FCNTL, fd, cmd, arg);
}

s32 sp_sys_tcgetattr(s32 fd, sp_sys_termios_t* termios) {
  s32 result = sp_sys_ioctl(fd, SP_TCGETS, termios);
  return result < 0 ? -1 : result;
}

s32 sp_sys_tcsetattr(s32 fd, s32 opt, const sp_sys_termios_t* termios) {
  s32 result = sp_sys_ioctl(fd, SP_TCSETS + (u64)opt, (void*)termios);
  return result < 0 ? -1 : result;
}

s32 sp_sys_getpid(void) {
  return (s32)__sp_syscall_ret(sp_syscall0(SP_SYSCALL_NUM_GETPID));
}

s32 sp_sys_inotify_init1(s32 flags) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_INIT1, flags);
}

s32 sp_sys_inotify_add_watch(s32 fd, const c8* pathname, u32 mask) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_ADD_WATCH, fd, pathname, mask);
}

s32 sp_sys_inotify_rm_watch(s32 fd, s32 wd) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_INOTIFY_RM_WATCH, fd, wd);
}

s32 sp_sys_poll(sp_sys_pollfd_t* fds, u64 nfds, s32 timeout) {
#if defined(SP_AMD64)
  return (s32)sp_syscall(SP_SYSCALL_NUM_POLL, fds, nfds, timeout);
#elif defined(SP_ARM64)
  sp_sys_timespec_t ts = { .tv_sec = timeout / 1000, .tv_nsec = (timeout % 1000) * 1000000 };
  sp_sys_timespec_t* tsp = timeout < 0 ? SP_NULLPTR : &ts;
  return (s32)sp_syscall(SP_SYSCALL_NUM_PPOLL, fds, nfds, tsp, 0, 0);
#endif
}

s32 sp_sys_wait4(s32 pid, s32* status, s32 options, void* rusage) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_WAIT4, pid, status, options, rusage);
}

void* sp_sys_mmap(void* addr, u64 len, s32 prot, s32 flags, s32 fd, s64 offset) {
  return (void*)sp_syscall(SP_SYSCALL_NUM_MMAP, addr, len, prot, flags, fd, offset);
}

s32 sp_sys_munmap(void* addr, u64 len) {
  return (s32)sp_syscall(SP_SYSCALL_NUM_MUNMAP, addr, len);
}

void* sp_sys_mremap(void* old_addr, u64 old_size, u64 new_size, s32 flags) {
  return (void*)sp_syscall(SP_SYSCALL_NUM_MREMAP, old_addr, old_size, new_size, flags);
}


void sp_sys_exit(s32 code) {
  sp_syscall(SP_SYSCALL_NUM_EXIT_GROUP, code);
  __builtin_unreachable();
}

void sp_sys_assert(const c8* file, const c8* line, const c8* func, const c8* expr, bool cond) {
  if (cond) return;

  sp_io_writer_t io = sp_zero_initialize();
  sp_io_writer_from_fd(&io, sp_sys_stderr, SP_IO_CLOSE_MODE_NONE);

  // "file:line: func: assert: expr\n"
  //
  // We don't have a version of sp_fmt() that doesn't allocate, unfortunately, which means
  // it goes through the context and opens us up to asserting again.
  sp_io_write_cstr(&io, SP_ANSI_FG_RED, SP_NULLPTR);
  sp_io_write_cstr(&io, "assert ", SP_NULLPTR);
  sp_io_write_cstr(&io, SP_ANSI_RESET, SP_NULLPTR);
  sp_io_write_cstr(&io, SP_ANSI_FG_BRIGHT_CYAN, SP_NULLPTR);
  sp_io_write_cstr(&io, file, SP_NULLPTR);
  sp_io_write_cstr(&io, SP_ANSI_RESET, SP_NULLPTR);
  sp_io_write_cstr(&io, ":", SP_NULLPTR);
  sp_io_write_cstr(&io, line, SP_NULLPTR);
  sp_io_write_cstr(&io, ": ", SP_NULLPTR);
  sp_io_write_cstr(&io, SP_ANSI_FG_BRIGHT_CYAN, SP_NULLPTR);
  sp_io_write_cstr(&io, func, SP_NULLPTR);
  sp_io_write_cstr(&io, "() ", SP_NULLPTR);
  sp_io_write_cstr(&io, SP_ANSI_RESET, SP_NULLPTR);
  sp_io_write_cstr(&io, expr, SP_NULLPTR);
  sp_io_write_cstr(&io, "\n", SP_NULLPTR);

  #ifdef SP_GNUC
    __builtin_trap();
  #else
    #if defined(SP_AMD64)
      __asm__ volatile ("int3")
    #elif defined(SP_ARM64)
      __asm__ volatile ("brk #0")
    #endif
  #endif
}

static s64 sp_lx_readlink_at(const c8* path, c8* buf, u64 size) {
#if defined(SP_AMD64)
  return sp_syscall(SP_SYSCALL_NUM_READLINK, path, buf, size);
#else
  return sp_syscall(SP_SYSCALL_NUM_READLINKAT, SP_AT_FDCWD, path, buf, size);
#endif
}


#endif

#if defined(SP_MACOS) || defined(SP_COSMO)
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  return read(fd, buf, count);
}

s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  return write(fd, buf, count);
}

sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  return (sp_sys_fd_t)open(path, flags, mode);
}

s32 sp_sys_close(sp_sys_fd_t fd) {
  return close(fd);
}

s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  return lseek(fd, offset, whence);
}

s32 sp_sys_mkdir(const c8* path, s32 mode) {
  return mkdir(path, (mode_t)mode);
}

s32 sp_sys_rmdir(const c8* path) {
  return rmdir(path);
}

s32 sp_sys_unlink(const c8* path) {
  return unlink(path);
}

s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
  return rename(oldpath, newpath);
}

s32 sp_sys_chdir(const c8* path) {
  return chdir(path);
}

s64 sp_sys_getcwd(char* buf, u64 size) {
  return getcwd(buf, size) ? 0 : -1;
}

s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
  return link(oldpath, newpath);
}

s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
  return symlink(target, linkpath);
}

s32 sp_sys_chmod(const c8* path, s32 mode) {
  return chmod(path, (mode_t)mode);
}

s32 sp_sys_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  struct timespec native;
  s32 rc = clock_gettime((clockid_t)clockid, &native);
  if (rc == 0) {
    ts->tv_sec  = (s64)native.tv_sec;
    ts->tv_nsec = (s64)native.tv_nsec;
  }
  return rc;
}

s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  struct timespec req_native = {
    .tv_sec  = (time_t)req->tv_sec,
    .tv_nsec = (long)req->tv_nsec,
  };
  struct timespec rem_native = SP_ZERO_INITIALIZE();
  s32 rc = nanosleep(&req_native, &rem_native);
  if (rem) {
    rem->tv_sec  = (s64)rem_native.tv_sec;
    rem->tv_nsec = (s64)rem_native.tv_nsec;
  }
  return rc;
}

static void sp_sys_stat_from_libc(const struct stat* src, sp_sys_stat_t* out) {
  out->st_dev            = (u64)src->st_dev;
  out->st_ino            = (u64)src->st_ino;
  out->st_nlink          = (u64)src->st_nlink;
  out->st_mode           = (u32)src->st_mode;
  out->st_uid            = (u32)src->st_uid;
  out->st_gid            = (u32)src->st_gid;
  out->st_rdev           = (u64)src->st_rdev;
  out->st_size           = (s64)src->st_size;
  out->st_blksize        = (s64)src->st_blksize;
  out->st_blocks         = (s64)src->st_blocks;
#if defined(SP_MACOS)
  out->st_flags          = (u32)src->st_flags;
  out->st_gen            = (u64)src->st_gen;
  out->st_atim.tv_sec    = (s64)src->st_atimespec.tv_sec;
  out->st_atim.tv_nsec   = (s64)src->st_atimespec.tv_nsec;
  out->st_mtim.tv_sec    = (s64)src->st_mtimespec.tv_sec;
  out->st_mtim.tv_nsec   = (s64)src->st_mtimespec.tv_nsec;
  out->st_ctim.tv_sec    = (s64)src->st_ctimespec.tv_sec;
  out->st_ctim.tv_nsec   = (s64)src->st_ctimespec.tv_nsec;
  out->st_birthtim.tv_sec  = (s64)src->st_birthtimespec.tv_sec;
  out->st_birthtim.tv_nsec = (s64)src->st_birthtimespec.tv_nsec;
#else
  out->st_flags          = 0;
  out->st_gen            = 0;
  out->st_atim.tv_sec    = (s64)src->st_atim.tv_sec;
  out->st_atim.tv_nsec   = (s64)src->st_atim.tv_nsec;
  out->st_mtim.tv_sec    = (s64)src->st_mtim.tv_sec;
  out->st_mtim.tv_nsec   = (s64)src->st_mtim.tv_nsec;
  out->st_ctim.tv_sec    = (s64)src->st_ctim.tv_sec;
  out->st_ctim.tv_nsec   = (s64)src->st_ctim.tv_nsec;
  out->st_birthtim       = out->st_ctim;
#endif
}

s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = stat(path, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}

s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = lstat(path, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  struct stat native;
  s32 rc = fstat(fd, &native);
  if (rc == 0) sp_sys_stat_from_libc(&native, st);
  return rc;
}
#elif defined(SP_WIN32)
s64 sp_sys_read(sp_sys_fd_t fd, void* buf, u64 count) {
  DWORD n = 0;
  if (!ReadFile((HANDLE)fd, buf, (DWORD)count, &n, SP_NULLPTR)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) return 0;
    return -1;
  }
  return (s64)n;
}

s64 sp_sys_write(sp_sys_fd_t fd, const void* buf, u64 count) {
  DWORD n = 0;
  if (!WriteFile((HANDLE)fd, buf, (DWORD)count, &n, SP_NULLPTR)) return -1;
  return (s64)n;
}

sp_sys_fd_t sp_sys_open(const c8* path, s32 flags, s32 mode) {
  (void)mode;

  DWORD access = 0;
  s32 access_mode = flags & (SP_O_RDONLY | SP_O_WRONLY | SP_O_RDWR);
  if (access_mode == SP_O_RDONLY) access = GENERIC_READ;
  else if (access_mode == SP_O_WRONLY) access = GENERIC_WRITE;
  else if (access_mode == SP_O_RDWR) access = GENERIC_READ | GENERIC_WRITE;
  else access = GENERIC_READ;

  if (flags & SP_O_APPEND) access |= FILE_APPEND_DATA;

  DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

  DWORD disposition;
  if ((flags & SP_O_CREAT) && (flags & SP_O_EXCL)) disposition = CREATE_NEW;
  else if ((flags & SP_O_CREAT) && (flags & SP_O_TRUNC)) disposition = CREATE_ALWAYS;
  else if (flags & SP_O_CREAT) disposition = OPEN_ALWAYS;
  else if (flags & SP_O_TRUNC) disposition = TRUNCATE_EXISTING;
  else disposition = OPEN_EXISTING;

  HANDLE h = CreateFileA(path, access, share, SP_NULLPTR, disposition,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, SP_NULLPTR);
  if (h == INVALID_HANDLE_VALUE) return SP_SYS_INVALID_FD;
  if (flags & SP_O_APPEND) {
    LARGE_INTEGER zero = { .QuadPart = 0 };
    SetFilePointerEx(h, zero, SP_NULLPTR, FILE_END);
  }
  return (sp_sys_fd_t)h;
}

s32 sp_sys_close(sp_sys_fd_t fd) {
  if (fd == SP_SYS_INVALID_FD) return -1;
  return CloseHandle((HANDLE)fd) ? 0 : -1;
}

s64 sp_sys_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  LARGE_INTEGER dist, new_pos;
  dist.QuadPart = offset;
  DWORD method;
  if (whence == SP_SEEK_SET) method = FILE_BEGIN;
  else if (whence == SP_SEEK_CUR) method = FILE_CURRENT;
  else if (whence == SP_SEEK_END) method = FILE_END;
  else return -1;
  if (!SetFilePointerEx((HANDLE)fd, dist, &new_pos, method)) return -1;
  return (s64)new_pos.QuadPart;
}

s32 sp_sys_mkdir(const c8* path, s32 mode) {
  (void)mode;
  return CreateDirectoryA(path, SP_NULLPTR) ? 0 : -1;
}

s32 sp_sys_rmdir(const c8* path) {
  return RemoveDirectoryA(path) ? 0 : -1;
}

s32 sp_sys_unlink(const c8* path) {
  return DeleteFileA(path) ? 0 : -1;
}

s32 sp_sys_rename(const c8* oldpath, const c8* newpath) {
  return MoveFileExA(oldpath, newpath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) ? 0 : -1;
}

s32 sp_sys_chdir(const c8* path) {
  return SetCurrentDirectoryA(path) ? 0 : -1;
}

s64 sp_sys_getcwd(char* buf, u64 size) {
  DWORD len = GetCurrentDirectoryA((DWORD)size, buf);
  if (!len || len >= size) return -1;
  return 0;
}

s32 sp_sys_link(const c8* oldpath, const c8* newpath) {
  return CreateHardLinkA(newpath, oldpath, SP_NULLPTR) ? 0 : -1;
}

s32 sp_sys_symlink(const c8* target, const c8* linkpath) {
  DWORD flags = 0;
  DWORD target_attrs = GetFileAttributesA(target);
  if (target_attrs != INVALID_FILE_ATTRIBUTES && (target_attrs & FILE_ATTRIBUTE_DIRECTORY)) {
    flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
  }
  #ifdef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
    flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
  #endif
  return CreateSymbolicLinkA(linkpath, target, flags) ? 0 : -1;
}

s32 sp_sys_chmod(const c8* path, s32 mode) {
  (void)path;
  (void)mode;
  return -1;
}

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

  DWORD file_type = GetFileType(handle);

  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  st.st_blksize = 4096;
  st.st_flags   = info.dwFileAttributes;
  st.st_nlink   = info.nNumberOfLinks;
  st.st_dev     = info.dwVolumeSerialNumber;
  st.st_ino     = ((u64)info.nFileIndexHigh << 32) | (u64)info.nFileIndexLow;
  st.st_size    = (s64)(((u64)info.nFileSizeHigh << 32) | (u64)info.nFileSizeLow);

  u32 mode = 0444;
  if (!(info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) mode |= 0220;

  if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    mode |= SP_S_IFLNK;
  } else if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    mode |= SP_S_IFDIR | 0111;
  } else if (file_type == FILE_TYPE_DISK) {
    mode |= SP_S_IFREG;
  } else if (file_type == FILE_TYPE_CHAR) {
    mode |= SP_S_IFCHR;
  } else if (file_type == FILE_TYPE_PIPE) {
    mode |= SP_S_IFIFO;
  } else {
    mode |= SP_S_IFREG;
  }
  st.st_mode = mode;

  sp_sys_timespec_from_filetime(info.ftLastAccessTime,   &st.st_atim);
  sp_sys_timespec_from_filetime(info.ftLastWriteTime,    &st.st_mtim);
  sp_sys_timespec_from_filetime(info.ftCreationTime,     &st.st_birthtim);
  st.st_ctim = st.st_mtim;

  st.st_blocks = (st.st_size + 511) / 512;
  *out = st;
  return 0;
}

static s32 sp_sys_stat_from_nt_path(const c8* path, sp_sys_stat_t* out, bool follow_symlinks) {
  DWORD flags = FILE_FLAG_BACKUP_SEMANTICS;
  if (!follow_symlinks) flags |= FILE_FLAG_OPEN_REPARSE_POINT;
  HANDLE h = CreateFileA(
    path,
    FILE_READ_ATTRIBUTES,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NULLPTR,
    OPEN_EXISTING,
    flags,
    SP_NULLPTR
  );
  if (h == INVALID_HANDLE_VALUE) return -1;
  s32 rc = sp_sys_stat_from_nt_handle(h, out);
  CloseHandle(h);
  return rc;
}

s32 sp_sys_stat(const c8* path, sp_sys_stat_t* st) {
  return sp_sys_stat_from_nt_path(path, st, true);
}

s32 sp_sys_lstat(const c8* path, sp_sys_stat_t* st) {
  return sp_sys_stat_from_nt_path(path, st, false);
}

s32 sp_sys_fstat(sp_sys_fd_t fd, sp_sys_stat_t* st) {
  if (fd == SP_SYS_INVALID_FD) return -1;
  return sp_sys_stat_from_nt_handle((HANDLE)fd, st);
}

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

s32 sp_sys_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  (void)rem;
  u64 ns = (u64)req->tv_sec * 1000000000ULL + (u64)req->tv_nsec;
  Sleep((DWORD)(ns / 1000000ULL));
  return 0;
}
#endif

//////////////////
// FREESTANDING //
//////////////////
#if !defined(SP_FREESTANDING)
// We need to set up the runtime in freestanding mode, plus thunk to the
// user's main()
//
// Normally, a function like this that's part of the public API would at least
// be stubbed out on every platform, to make compilation easy for users. But
// this code's only invoked via SP_ENTRY() when compiling in freestanding
// mode.
//
// There's no analagous code when you're linking to libc, so if you're calling
// it explicitly then you probably know what you're doing
#else
void sp_sys_init() {
  sp_tls_block.self = &sp_tls_block;
  sp_tls_block.data = SP_NULLPTR;
  sp_sys_set_tp(&sp_tls_block);

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_env_init(&tls->env);

  if (!sp_envp) return;

  for (const c8** p = sp_envp; *p; p++) {
    sp_str_t entry = sp_str_view(*p);
    sp_str_pair_t pair = sp_str_cleave_c8(entry, '=');
    sp_str_ht_insert(tls->env.vars, pair.first, pair.second);
  }
}

void sp_entry_init(s32 argc, const c8** argv, sp_entry_fn_t fn) {
  sp_envp = argv + argc + 1;
  sp_sys_init();
  sp_sys_exit(fn(argc, argv));
}
#endif


//  ██╗  ██╗ █████╗ ███████╗██╗  ██╗
//  ██║  ██║██╔══██╗██╔════╝██║  ██║
//  ███████║███████║███████╗███████║
//  ██╔══██║██╔══██║╚════██║██╔══██║
//  ██║  ██║██║  ██║███████║██║  ██║
//  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝
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

// ██╗  ██╗ █████╗ ███████╗██╗  ██╗    ████████╗ █████╗ ██████╗ ██╗     ███████╗
// ██║  ██║██╔══██╗██╔════╝██║  ██║    ╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝
// ███████║███████║███████╗███████║       ██║   ███████║██████╔╝██║     █████╗
// ██╔══██║██╔══██║╚════██║██╔══██║       ██║   ██╔══██║██╔══██╗██║     ██╔══╝
// ██║  ██║██║  ██║███████║██║  ██║       ██║   ██║  ██║██████╔╝███████╗███████╗
// ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝       ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝
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
      sp_context_pop();
      return;
    }
  }

  u64 idx = first_free != SP_HT_INVALID_INDEX ? first_free : hash_idx;
  u8* entry = (u8*)(*data) + idx * info.stride.entry;
  sp_mem_copy(key, entry, info.size.key);
  sp_mem_copy(val, entry + info.stride.value, info.size.value);
  *(sp_ht_entry_state*)(entry + info.stride.kv) = SP_HT_ENTRY_ACTIVE;
  (*size)++;
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


// ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ████████╗
// ██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗╚══██╔══╝
// █████╗  ██║   ██║██████╔╝██╔████╔██║███████║   ██║
// ██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║   ██║
// ██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║   ██║
// ╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═     ╚═╝╚═╝  ╚═╝   ╚═╝
static sp_fmt_directive_t* sp_fmt_directive_lookup(sp_str_t name);
static void sp_fmt_directive_reset();
static void sp_fmt_register_builtins();

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

void sp_fmt_directive_reset(void) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_str_ht_free(tls->format.directives);
  tls->format.directives = SP_NULLPTR;
  sp_fmt_register_builtins();
}

typedef struct {
  sp_str_t str;
  u32 i;
} sp_fmt_parser_t;

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

static c8* sp_fmt_uint_to_buf_dec(u64 value, c8* buf_end) {
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

static c8* sp_fmt_uint_to_buf_hex(u64 value, c8* buf_end) {
  static const c8 digits[] = "0123456789abcdef";
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

static void sp_fmt_append_range(sp_str_builder_t* builder, const c8* start, const c8* end) {
  sp_str_t s = { .data = start, .len = (u32)(end - start) };
  sp_str_builder_append(builder, s);
}

static void sp_fmt_write_u64(sp_str_builder_t* builder, u64 value) {
  c8 buf[20];
  c8* end = buf + sizeof(buf);
  c8* start = sp_fmt_uint_to_buf_dec(value, end);
  sp_fmt_append_range(builder, start, end);
}

static void sp_fmt_write_s64(sp_str_builder_t* builder, s64 value) {
  c8 buf[21];
  c8* end = buf + sizeof(buf);
  u64 abs = (value < 0) ? ((u64)(-(value + 1)) + 1) : (u64)value;
  c8* start = sp_fmt_uint_to_buf_dec(abs, end);
  if (value < 0) *--start = '-';
  sp_fmt_append_range(builder, start, end);
}

static void sp_fmt_write_ptr(sp_str_builder_t* builder, void* value) {
  c8 buf[18];
  c8* end = buf + sizeof(buf);
  c8* start = sp_fmt_uint_to_buf_hex((u64)(uintptr_t)value, end);
  *--start = 'x';
  *--start = '0';
  sp_fmt_append_range(builder, start, end);
}

static void sp_fmt_write_f64(sp_str_builder_t* builder, f64 value, u32 precision) {
  union { f64 f; u64 u; } bits;
  bits.f = value;
  u64  exponent = (bits.u >> 52) & 0x7ffULL;
  u64  mantissa = bits.u & 0x000fffffffffffffULL;
  bool is_neg   = (bits.u >> 63) != 0;

  if (exponent == 0x7ff) {
    if (mantissa == 0) {
      sp_str_builder_append_cstr(builder, is_neg ? "-inf" : "inf");
    } else {
      sp_str_builder_append_cstr(builder, "nan");
    }
    return;
  }

  if (is_neg) {
    sp_str_builder_append_c8(builder, '-');
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
    sp_str_builder_append_cstr(builder, "inf");
    return;
  }

  u64 int_part = (u64)value;
  f64 frac     = value - (f64)int_part;
  u64 frac_scaled = (u64)(frac * (f64)scale + 0.5);
  if (frac_scaled >= scale) {
    int_part += 1;
    frac_scaled -= scale;
  }

  sp_fmt_write_u64(builder, int_part);

  if (precision > 0) {
    sp_str_builder_append_c8(builder, '.');
    c8 frac_buf[18];
    u32 i = precision;
    while (i--) {
      frac_buf[i] = (c8)('0' + (frac_scaled % 10));
      frac_scaled /= 10;
    }
    sp_str_t s = { .data = frac_buf, .len = precision };
    sp_str_builder_append(builder, s);
  }
}

void sp_fmt_render_default(sp_str_builder_t* builder, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(param);
  switch (arg->id) {
    case sp_fmt_id_u64:
      sp_fmt_write_u64(builder, arg->u);
      break;
    case sp_fmt_id_s64:
      sp_fmt_write_s64(builder, arg->i);
      break;
    case sp_fmt_id_f64: {
      u32 p = sp_opt_is_null(arg->spec.precision) ? 6 : sp_opt_get(arg->spec.precision);
      sp_fmt_write_f64(builder, arg->f, p);
      break;
    }
    case sp_fmt_id_str: {
      sp_str_t s = arg->s;
      if (!sp_opt_is_null(arg->spec.precision)) {
        u32 max = sp_opt_get(arg->spec.precision);
        if (max < s.len) s.len = max;
      }
      sp_str_builder_append(builder, s);
      break;
    }
    case sp_fmt_id_ptr:
      sp_fmt_write_ptr(builder, arg->p);
      break;
    case sp_fmt_id_custom:
      if (arg->custom.fn) arg->custom.fn(builder, arg, SP_NULLPTR);
      break;
  }
}

void sp_fmt_apply_spec_wrapped(
  sp_str_builder_t* out,
  sp_str_t before,
  sp_str_t content,
  sp_str_t after,
  sp_fmt_spec_t spec
) {
  u32 content_len = (u32)content.len;
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

  sp_for(k, left)  sp_str_builder_append_c8(out, fill);
  sp_str_builder_append(out, before);
  sp_str_builder_append(out, content);
  sp_str_builder_append(out, after);
  sp_for(k, right) sp_str_builder_append_c8(out, fill);
}

void sp_fmt_apply_spec(sp_str_builder_t* out, sp_str_t content, sp_fmt_spec_t spec) {
  sp_str_t empty = sp_zero();
  sp_fmt_apply_spec_wrapped(out, empty, content, empty, spec);
}

static const c8* sp_fmt_kind_name(sp_fmt_arg_kind_t k) {
  switch (k) {
    case sp_fmt_id_u64:    return "u64";
    case sp_fmt_id_s64:    return "s64";
    case sp_fmt_id_f64:    return "f64";
    case sp_fmt_id_str:    return "str";
    case sp_fmt_id_ptr:    return "ptr";
    case sp_fmt_id_custom: return "custom";
  }
  return "?";
}

static void sp_fmt_append_kind_mask(sp_str_builder_t* b, sp_fmt_arg_kind_t mask) {
  static const sp_fmt_arg_kind_t all[] = {
    sp_fmt_id_u64, sp_fmt_id_s64, sp_fmt_id_f64,
    sp_fmt_id_str, sp_fmt_id_ptr, sp_fmt_id_custom,
  };
  bool first = true;
  sp_carr_for(all, i) {
    if (mask & all[i]) {
      if (!first) sp_str_builder_append_c8(b, '|');
      sp_str_builder_append_cstr(b, sp_fmt_kind_name(all[i]));
      first = false;
    }
  }
}

static sp_err_t sp_fmt_render(sp_str_builder_t* out, sp_fmt_arg_t* arg, sp_fmt_arg_t* directive_params) {
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

  bool params[SP_FMT_MAX_DIRECTIVES] = sp_zero();
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

  struct {
    sp_str_builder_t before;
    sp_str_builder_t content;
    sp_str_builder_t after;
  } b = sp_zero();

  sp_for(it, num_dirs) {
    if (directives[it]->kind != sp_fmt_directive_decorator) continue;
    sp_fmt_fn_t before_fn = directives[it]->decorator.before;
    if (!before_fn) continue;
    sp_fmt_arg_t* p = params[it] ? &directive_params[it] : SP_NULLPTR;
    before_fn(&b.before, arg, p);
  }

  struct {
    sp_fmt_fn_t fn;
    sp_fmt_arg_t* param;
  } render = sp_zero();
  if (arg->id == sp_fmt_id_custom) {
    if (!arg->custom.fn) return SP_ERR_FMT_CUSTOM_WITHOUT_FN;
    render.fn = arg->custom.fn;
  }
  else {
    sp_for(it, num_dirs) {
      if (directives[it]->kind != sp_fmt_directive_renderer) continue;
      if (render.fn) return SP_ERR_FMT_TOO_MANY_RENDERERS;
      render.fn = directives[it]->renderer;
      render.param = params[it] ? &directive_params[it] : SP_NULLPTR;
    }
  }
  render.fn = render.fn ? render.fn : sp_fmt_render_default;
  render.fn(&b.content, arg, render.param);

  sp_str_t content = sp_str_builder_as_str(&b.content);

  u8 j = num_dirs;
  while (j--) {
    if (directives[j]->kind != sp_fmt_directive_transformer) continue;
    sp_fmt_arg_t* p = params[j] ? &directive_params[j] : SP_NULLPTR;
    sp_str_builder_t next = SP_ZERO_INITIALIZE();
    directives[j]->transformer(&next, content, arg, p);
    content = sp_str_builder_as_str(&next);
  }

  j = num_dirs;
  while (j--) {
    if (directives[j]->kind != sp_fmt_directive_decorator) continue;
    sp_fmt_fn_t after_fn = directives[j]->decorator.after;
    if (!after_fn) continue;
    sp_fmt_arg_t* p = params[j] ? &directive_params[j] : SP_NULLPTR;
    after_fn(&b.after, arg, p);
  }

  sp_str_t before = sp_str_builder_as_str(&b.before);
  sp_str_t after  = sp_str_builder_as_str(&b.after);

  sp_fmt_apply_spec_wrapped(out, before, content, after, arg->spec);
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

sp_err_t sp_fmt_v(sp_str_t* str, sp_str_t fmt, va_list args) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_err_t result = SP_OK;
  sp_fmt_parser_t p = { .str = fmt };

  while (true) {
    c8 c = sp_fmt_peek(&p, 0);
    if (!c) break;

    if (c == '{') {
      if (sp_fmt_peek(&p, 1) == '{') {
        sp_fmt_advance(&p);
        sp_fmt_advance(&p);
        sp_str_builder_append_c8(&builder, '{');
        continue;
      }

      sp_fmt_spec_t spec = sp_zero();
      sp_try_goto(sp_fmt_parse_specifier(&p, &spec), result, error);

      if (spec.fill_dynamic) {
        s64 v;
        sp_try_goto(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v), result, error);
        spec.fill = (u8)v;
      }
      if (spec.width_dynamic) {
        s64 v;
        sp_try_goto(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v), result, error);
        if (v < 0) v = 0;
        if (v > SP_FMT_WIDTH_MAX) v = SP_FMT_WIDTH_MAX;
        spec.width = (u32)v;
      }
      if (spec.precision_dynamic) {
        s64 v;
        sp_try_goto(sp_fmt_pull_int_arg(va_arg(args, sp_fmt_arg_t), &v), result, error);
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        sp_opt_set(spec.precision, (u8)v);
      }

      sp_fmt_arg_t directive_params[SP_FMT_MAX_DIRECTIVES] = SP_ZERO_INITIALIZE();
      for (u8 di = 0; di < spec.directive_count; di++) {
        if (spec.directive_arg_dynamic & (1u << di)) {
          directive_params[di] = va_arg(args, sp_fmt_arg_t);
        }
      }

      sp_fmt_arg_t arg = va_arg(args, sp_fmt_arg_t);
      arg.spec = spec;
      sp_try_goto(sp_fmt_render(&builder, &arg, directive_params), result, error);
      continue;
    }

    if (c == '}') {
      if (sp_fmt_peek(&p, 1) == '}') {
        sp_fmt_advance(&p);
        sp_fmt_advance(&p);
        sp_str_builder_append_c8(&builder, '}');
        continue;
      }
      // Lone `}` — unbalanced close brace. Mirrors `{` error policy.
      result = SP_ERR_FMT_BAD_PLACEHOLDER;
      goto error;
    }

    sp_str_builder_append_c8(&builder, c);
    sp_fmt_advance(&p);
  }

  *str = sp_str_builder_to_str(&builder);

error:
  return result;
}

sp_str_t sp_fmt(const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t str = sp_zero();
  sp_fmt_v(&str, sp_str_view(fmt), args);
  va_end(args);
  return str;
}

sp_err_t sp_fmt_e(sp_str_t* str, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_err_t error = sp_fmt_v(str, sp_str_view(fmt), args);
  va_end(args);
  return error;
}

static void sp_fmt_directive_bold(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_str_builder_append_cstr(b, "\033[1m");
}

static void sp_fmt_directive_italic(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_str_builder_append_cstr(b, "\033[3m");
}

static void sp_fmt_directive_red(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_RED);
}

static void sp_fmt_directive_green(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_GREEN);
}

static void sp_fmt_directive_yellow(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_YELLOW);
}

static void sp_fmt_directive_blue(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BLUE);
}

static void sp_fmt_directive_cyan(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_CYAN);
}

static void sp_fmt_directive_magenta(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_MAGENTA);
}

static void sp_fmt_directive_white(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_WHITE);
}

static void sp_fmt_directive_black(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BLACK);
}

static void sp_fmt_directive_bright_red(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_RED);
}

static void sp_fmt_directive_bright_green(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_GREEN);
}

static void sp_fmt_directive_bright_yellow(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_YELLOW);
}

static void sp_fmt_directive_bright_blue(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_BLUE);
}

static void sp_fmt_directive_bright_cyan(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_CYAN);
}

static void sp_fmt_directive_bright_magenta(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_MAGENTA);
}

static void sp_fmt_directive_bright_white(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_WHITE);
}

static void sp_fmt_directive_gray(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(arg); sp_unused(params);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_BLACK);
}

static void sp_fmt_directive_ansi_reset(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(arg); sp_unused(param);
  sp_str_builder_append_cstr(b, SP_ANSI_RESET);
}

static void sp_fmt_directive_hyperlink(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_str_builder_append_cstr(b, "\033]8;;"); sp_unused(params);
  if (arg->id == sp_fmt_id_str) sp_str_builder_append(b, arg->s);
  sp_str_builder_append_cstr(b, "\033\\");
}

static void sp_fmt_directive_hyperlink_after(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_str_builder_append_cstr(b, "\033]8;;\033\\");
}

static void sp_fmt_directive_quote(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_str_builder_append_c8(b, '"');
}

static void sp_fmt_directive_quote_after(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_str_builder_append_c8(b, '"');
}

static void sp_fmt_directive_upper_transform(sp_str_builder_t* out, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_for(i, content.len) {
    c8 c = content.data[i];
    sp_str_builder_append_c8(out, (c >= 'a' && c <= 'z') ? (c8)(c - 32) : c);
  }
}

static void sp_fmt_directive_redact_transform(sp_str_builder_t* out, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  (void)arg; sp_unused(params);
  sp_for(i, content.len) sp_str_builder_append_c8(out, '*');
}

static void sp_fmt_directive_bytes_render(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
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
  sp_fmt_write_u64(b, whole);
  if (unit_idx > 0) {
    u32 tenths = (u32)((rem * 10) >> 10);
    if (tenths > 0) {
      sp_str_builder_append_c8(b, '.');
      sp_str_builder_append_c8(b, (c8)('0' + tenths));
    }
  }
  sp_str_builder_append_c8(b, ' ');
  sp_str_builder_append_cstr(b, units[unit_idx]);
}

static void sp_fmt_directive_write_zpad2(sp_str_builder_t* b, u32 value) {
  sp_str_builder_append_c8(b, (c8)('0' + (value / 10) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + value % 10));
}

static void sp_fmt_directive_write_zpad4(sp_str_builder_t* b, u32 value) {
  sp_str_builder_append_c8(b, (c8)('0' + (value / 1000) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + (value / 100) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + (value / 10) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + value % 10));
}

static void sp_fmt_directive_iso_render(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  u64 epoch = arg->u; sp_unused(params);
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

  sp_fmt_directive_write_zpad4(b, (u32)y);
  sp_str_builder_append_c8(b, '-');
  sp_fmt_directive_write_zpad2(b, m);
  sp_str_builder_append_c8(b, '-');
  sp_fmt_directive_write_zpad2(b, d);
  sp_str_builder_append_c8(b, 'T');
  sp_fmt_directive_write_zpad2(b, hour);
  sp_str_builder_append_c8(b, ':');
  sp_fmt_directive_write_zpad2(b, min);
  sp_str_builder_append_c8(b, ':');
  sp_fmt_directive_write_zpad2(b, sec);
  sp_str_builder_append_c8(b, 'Z');
}

static void sp_fmt_directive_ordinal_render(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  s64 value = (arg->id == sp_fmt_id_s64) ? arg->i : (s64)arg->u; sp_unused(params);
  sp_fmt_write_s64(b, value);
  s64 abs = value < 0 ? -value : value;
  u32 mod100 = (u32)(abs % 100);
  u32 mod10 = (u32)(abs % 10);
  const c8* suffix = "th";
  if (mod100 < 11 || mod100 > 13) {
    if (mod10 == 1) suffix = "st";
    else if (mod10 == 2) suffix = "nd";
    else if (mod10 == 3) suffix = "rd";
  }
  sp_str_builder_append_cstr(b, suffix);
}

static void sp_fmt_directive_duration_render(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* params) {
  sp_unused(params);
  u64 ns = arg->u;
  if (ns < 1000) {
    sp_fmt_write_u64(b, ns);
    sp_str_builder_append_cstr(b, " ns");
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
  sp_fmt_write_u64(b, whole);
  if (rem >= 100) {
    sp_str_builder_append_c8(b, '.');
    sp_str_builder_append_c8(b, (c8)('0' + rem / 100));
  }
  sp_str_builder_append_c8(b, ' ');
  sp_str_builder_append_cstr(b, units[unit_idx]);
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

static void sp_fmt_directive_fg(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  sp_unused(arg);
  sp_str_t ansi = sp_fmt_color_to_ansi_fg(param->s);
  sp_str_builder_append(b, ansi);
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
  sp_fmt_register_renderer("iso", sp_fmt_directive_iso_render, 0);
  sp_fmt_register_renderer("ordinal", sp_fmt_directive_ordinal_render, 0);
  sp_fmt_register_renderer("duration", sp_fmt_directive_duration_render, 0);

}


// @format @fmt
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


// @log

void sp_print(const c8* fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, sp_str_view(fmt), args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.out, formatted, SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }
}

void sp_print_str(sp_str_t fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, fmt, args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.out, formatted, SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }

}

void sp_print_err(const c8* fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, sp_str_view(fmt), args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.err, formatted, SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }
}

void sp_log(const c8* fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, sp_str_view(fmt), args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.out, formatted, SP_NULLPTR);
    sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }
}

void sp_log_str(sp_str_t fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, fmt, args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.out, formatted, SP_NULLPTR);
    sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }
}

void sp_log_err(const c8* fmt, ...) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch(); {
    va_list args;
    va_start(args, fmt);
    sp_str_t formatted = sp_zero();
    sp_fmt_v(&formatted, sp_str_view(fmt), args);
    va_end(args);

    sp_tls_rt_t* tls = sp_tls_rt_get();
    sp_io_write_str(tls->std.out, formatted, SP_NULLPTR);
    sp_io_write_cstr(tls->std.out, "\n", SP_NULLPTR);
    sp_mem_end_scratch(scratch);
  }
}


//  ██████╗ ██████╗ ███╗   ██╗████████╗███████╗██╗  ██╗████████╗
// ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝
// ██║     ██║   ██║██╔██╗ ██║   ██║   █████╗   ╚███╔╝    ██║
// ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══╝   ██╔██╗    ██║
// ╚██████╗╚██████╔╝██║ ╚████║   ██║   ███████╗██╔╝ ██╗   ██║
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ═╝   ╚═╝
//  @context
SP_PRIVATE void sp_tls_rt_deinit(void *data);
SP_PRIVATE void sp_rt_init();

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

void sp_context_push_allocator(sp_allocator_t allocator) {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_context_t context = tls->contexts[tls->index];
  context.allocator = allocator;
  sp_context_push(context);
}

void sp_context_push_arena(sp_mem_arena_t* arena) {
  sp_context_push_allocator(sp_mem_arena_as_allocator(arena));
}

void sp_context_pop() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  SP_ASSERT(tls->index > 0);
  tls->contexts[tls->index] = SP_ZERO_STRUCT(sp_context_t); // Not required, just for the debugger
  tls->index--;
}

sp_context_t* sp_context_get() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  return &tls->contexts[tls->index];
}

void sp_rt_init() {
  sp_mutex_init(&sp_rt.mutex, SP_MUTEX_PLAIN);
  sp_tls_new(&sp_rt.tls.key, sp_tls_rt_deinit);
}

void sp_tls_rt_deinit(void* ptr) {
  if (!ptr) return;
  sp_tls_rt_t* tls = (sp_tls_rt_t*)ptr;
  sp_mem_os_free(tls->std.out);
  sp_mem_os_free(tls->std.err);
  sp_str_ht_free(tls->format.directives);
  sp_mem_arena_destroy(tls->scratch);
  sp_mem_os_free(ptr);

}

sp_tls_rt_t* sp_tls_rt_get() {
  sp_tls_once(&sp_rt.tls.once, sp_rt_init);

  sp_tls_rt_t* tls = (sp_tls_rt_t*)sp_tls_get(sp_rt.tls.key);
  if (!tls) {
    // It's important that you bootstrap the allocator and set the TLS key
    // before doing anything else so you can call functions that allocate
    // while initializing the other TLS stuff.
    tls = sp_os_alloc_type(sp_tls_rt_t);
    tls->contexts[0].allocator = sp_mem_os_new();
    sp_tls_set(sp_rt.tls.key, tls);

    tls->scratch = sp_mem_arena_new_ex(SP_MEM_ARENA_BLOCK_SIZE, SP_MEM_ARENA_MODE_DEFAULT, SP_MEM_ALIGNMENT);
    tls->std.out = sp_alloc_type(sp_io_writer_t);
    tls->std.err = sp_alloc_type(sp_io_writer_t);
    sp_io_writer_from_fd(tls->std.out, sp_sys_stdout, SP_IO_CLOSE_MODE_NONE);
    sp_io_writer_from_fd(tls->std.err, sp_sys_stderr, SP_IO_CLOSE_MODE_NONE);
    sp_fmt_register_builtins();
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
  SP_BROKEN();
}

void* sp_tls_get(sp_tls_key_t key) {
  SP_BROKEN();
  return SP_NULLPTR;
}

void sp_tls_set(sp_tls_key_t key, void* data) {
  SP_BROKEN();
}

void sp_tls_once(sp_tls_once_t* once, sp_tls_once_fn_t fn) {
  SP_BROKEN();
}
#endif


// ███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗
// ████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗╚██╗ ██╔╝
// ██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝ ╚████╔╝
// ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗  ╚██╔╝
// ██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║   ██║
// ╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝
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

void* sp_mem_allocator_alloc(sp_allocator_t allocator, u64 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
}

void* sp_mem_allocator_realloc(sp_allocator_t allocator, void* memory, u64 size) {
  return allocator.on_alloc(allocator.user_data, SP_ALLOCATOR_MODE_RESIZE, size, memory);
}

void sp_mem_allocator_free(sp_allocator_t allocator, void* buffer) {
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
  sp_mem_arena_t* arena = sp_alloc_type(sp_mem_arena_t);
  arena->allocator = sp_context_get()->allocator;
  arena->mode = mode;
  arena->alignment = alignment == 0 ? SP_MEM_ALIGNMENT : alignment;

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

sp_mem_os_header_t* sp_mem_os_get_header(void* ptr) {
  return ((sp_mem_os_header_t*)ptr) - 1;
}

void* sp_mem_os_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr) {
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      u64 total_size = size + sizeof(sp_mem_os_header_t);
      sp_mem_os_header_t* metadata = (sp_mem_os_header_t*)sp_mem_os_alloc_zero(total_size);
      metadata->size = size;
      return metadata + 1;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      if (!ptr) {
        return sp_mem_os_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      }

      sp_mem_os_header_t* metadata = sp_mem_os_get_header(ptr);
      if (metadata->size >= size) {
        return ptr;
      }

      void* buffer = sp_mem_os_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, NULL);
      sp_mem_copy(ptr, buffer, metadata->size);
      sp_mem_os_on_alloc(user_data, SP_ALLOCATOR_MODE_FREE, 0, ptr);

      return buffer;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      sp_mem_os_header_t* metadata = sp_mem_os_get_header(ptr);
      sp_mem_os_free(metadata);
      return NULL;
    }
    default: {
      return NULL;
    }
  }
}

sp_allocator_t sp_mem_os_new() {
  sp_allocator_t allocator;
  allocator.on_alloc = sp_mem_os_on_alloc;
  allocator.user_data = NULL;
  return allocator;
}

bool sp_mem_is_equal(const void* a, const void* b, u64 len) {
  if (!len) return true;
  if (!a || !b) return false;
  return !sp_memcmp(a, b, len);
}

void sp_mem_copy(const void* source, void* dest, u64 num_bytes) {
  if (!source) return;
  if (!dest) return;
  if (!num_bytes) return;
  sp_memcpy(dest, source, num_bytes);
}

void sp_mem_move(const void* source, void* dest, u64 num_bytes) {
  if (!source) return;
  if (!dest) return;
  sp_memmove(dest, source, num_bytes);
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

#if defined(SP_WIN32)
void* sp_mem_os_alloc(u64 size) {
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void* sp_mem_os_alloc_zero(u64 size) {
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void* sp_mem_os_realloc(void* ptr, u64 size) {
  if (!ptr) return sp_mem_os_alloc(size);
  if (!size) { HeapFree(GetProcessHeap(), 0, ptr); return NULL; }
  return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

void sp_mem_os_free(void* ptr) {
  if (!ptr) return;
  HeapFree(GetProcessHeap(), 0, ptr);
}

#elif defined(SP_FREESTANDING)
void* sp_mem_os_alloc(u64 size) {
  return sp_sys_alloc(size);
}

void* sp_mem_os_alloc_zero(u64 size) {
  return sp_sys_alloc_zero(size);
}

void* sp_mem_os_realloc(void* ptr, u64 size) {
  return sp_sys_realloc(ptr, size);
}

void sp_mem_os_free(void* ptr) {
  sp_sys_free(ptr);
}

#else
void* sp_mem_os_alloc(u64 size) {
  return calloc(size, 1);
}

void* sp_mem_os_alloc_zero(u64 size) {
  return calloc(size, 1);
}

void* sp_mem_os_realloc(void* ptr, u64 size) {
  return realloc(ptr, size);
}

void sp_mem_os_free(void* ptr) {
  free(ptr);
}
#endif


// ███████╗██╗     ██╗ ██████╗███████╗
// ██╔════╝██║     ██║██╔════╝██╔════╝
// ███████╗██║     ██║██║     █████╗
// ╚════██║██║     ██║██║     ██╔══╝
// ███████║███████╗██║╚██████╗███████╗
// ╚══════╝╚══════╝╚═╝ ╚═════╝╚══════╝
// @slice
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

u32 sp_cstr_len_sized(const c8* str, u32 n) {
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

sp_str_t sp_str_concat(sp_str_t a, sp_str_t b) {
  return sp_fmt("{}{}", sp_fmt_str(a), sp_fmt_str(b));
}

sp_str_t sp_str_join(sp_str_t a, sp_str_t b, sp_str_t join) {
  return sp_fmt("{}{}{}", sp_fmt_str(a), sp_fmt_str(join), sp_fmt_str(b));
}

sp_str_t sp_str_join_cstr_n(const c8** strings, u32 num_strings, sp_str_t join) {
  if (!strings) return sp_str_lit("");
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  for (u32 index = 0; index < num_strings; index++) {
    sp_str_builder_append_cstr(&builder, strings[index]);

    if (index != (num_strings - 1)) {
      sp_str_builder_append(&builder, join);
    }
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
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
  if (!str) return sp_str_lit("");
  c8* buffer = (c8*)sp_alloc(length);
  u32 len = sp_cstr_len(str);
  len = SP_MIN(len, length);
  sp_mem_copy(str, buffer, len);

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr_null(const c8* str) {
  if (!str) return sp_str_lit("");
  u32 len = sp_cstr_len(str);
  c8* buffer = sp_alloc_n(c8, len + 1);
  sp_mem_copy(str, buffer, len);
  buffer[len] = 0;

  return SP_STR(buffer, len);
}

sp_str_t sp_str_from_cstr(const c8* str) {
  if (!str) return sp_str_lit("");
  u32 len = sp_cstr_len(str);
  c8* buffer = sp_alloc_n(c8, len + 1);
  sp_mem_copy(str, buffer, len);

  return sp_str(buffer, len);
}

sp_str_t sp_str_copy(sp_str_t str) {
  if (!str.data) return sp_str_lit("");
  c8* buffer = sp_alloc_n(c8, str.len);
  sp_mem_copy(str.data, buffer, str.len);

  return SP_STR(buffer, str.len);
}

void sp_str_copy_to(sp_str_t str, c8* buffer, u32 capacity) {
  if (!str.data) return;
  sp_mem_copy(str.data, buffer, SP_MIN(str.len, capacity));
}

sp_str_t sp_str_null_terminate(sp_str_t str) {
  if (!str.data) return sp_str_lit("");
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

sp_str_builder_t sp_str_builder_from_writer(sp_io_writer_t* writer) {
  return (sp_str_builder_t) { .writer = writer };
}

void sp_str_builder_ensure_writer(sp_str_builder_t* builder) {
  if (!builder->writer) {
    builder->writer = sp_alloc_type(sp_io_writer_t);
    sp_io_writer_from_dyn_mem(builder->writer);
  }
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
  sp_str_builder_ensure_writer(builder);
  sp_io_write_str(builder->writer, str, SP_NULLPTR);
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
  sp_str_t formatted = sp_zero();
  sp_fmt_v(&formatted, fmt, args);
  va_end(args);

  sp_str_builder_append(builder, formatted);
}

void sp_str_builder_append_fmt(sp_str_builder_t* builder, const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t formatted = sp_zero();
  sp_fmt_v(&formatted, sp_str_view(fmt), args);
  va_end(args);

  sp_str_builder_append(builder, formatted);
}

void sp_str_builder_new_line(sp_str_builder_t* builder) {
  sp_str_builder_append_c8(builder, '\n');

  if (!sp_str_valid(builder->indent.word)) {
    builder->indent.word = SP_LIT("  ");
  }

  sp_for(index, builder->indent.level) {
    sp_str_builder_append(builder, builder->indent.word);
  }
}

u64 sp_str_builder_len(sp_str_builder_t* builder) {
  if (!builder->writer) return 0;

  u64 size = 0;
  sp_io_writer_size(builder->writer, &size);
  return size;
}

sp_str_t sp_str_builder_as_str(sp_str_builder_t* builder) {
  sp_str_builder_ensure_writer(builder);
  return sp_mem_buffer_as_str(&builder->writer->dyn_mem.buffer);
}

sp_str_t sp_str_builder_to_str(sp_str_builder_t* builder) {
  return sp_str_copy(sp_str_builder_as_str(builder));
}

sp_mem_buffer_t sp_str_builder_into_buffer(sp_str_builder_t* builder) {
  sp_str_builder_ensure_writer(builder);
  sp_mem_buffer_t buffer = builder->writer->dyn_mem.buffer;
  sp_free(builder->writer);
  builder->writer = SP_NULLPTR;
  return buffer;
}

void sp_str_builder_free(sp_str_builder_t* builder) {
  if (builder->writer) {
    sp_io_writer_close(builder->writer);
    sp_free(builder->writer);
    builder->writer = SP_NULLPTR;
  }
}


sp_str_t sp_str_reduce(sp_str_t* strings, u32 num_strings, void* user_data, sp_str_reduce_fn_t fn) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_reduce_context_t context = {
    .user_data = user_data,
    .builder = SP_ZERO_INITIALIZE(),
    .elements = {
      .data = strings,
      .len = num_strings
    },
  };

  sp_for(index, num_strings) {
    context.str = strings[index];
    context.index = index;
    fn(&context);
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&context.builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
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

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, str);
  for (s32 index = 0; index < delta; index++) {
    sp_str_builder_append_c8(&builder, ' ');
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_str_t sp_str_replace_c8(sp_str_t str, c8 from, c8 to) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  for (u32 i = 0; i < str.len; i++) {
    c8 c = str.data[i];
    if (c == from) {
      sp_str_builder_append_c8(&builder, to);
    } else {
      sp_str_builder_append_c8(&builder, c);
    }
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
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
  sp_str_t result = sp_str_builder_to_str(&builder);
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
  sp_str_t result = sp_str_builder_to_str(&builder);
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
  sp_str_t result = sp_str_builder_to_str(&builder);
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

sp_dyn_array(sp_str_t) sp_str_map(sp_str_t* strs, u32 num_strs, void* user_data, sp_str_map_fn_t fn) {
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


sp_dyn_array(sp_str_t) sp_str_pad_to_longest(sp_str_t* strs, u32 n) {
  u32 max_len = 0;
  sp_for(i, n) {
    if (strs[i].len > max_len) max_len = strs[i].len;
  }
  return sp_str_map(strs, n, &max_len, sp_str_map_kernel_pad);
}


// ███████╗██╗██╗     ███████╗███████╗██╗   ██╗███████╗████████╗███████╗███╗   ███╗
// ██╔════╝██║██║     ██╔════╝██╔════╝╚██╗ ██╔╝██╔════╝╚══██╔══╝██╔════╝████╗ ████║
// █████╗  ██║██║     █████╗  ███████╗ ╚████╔╝ ███████╗   ██║   █████╗  ██╔████╔██║
// ██╔══╝  ██║██║     ██╔══╝  ╚════██║  ╚██╔╝  ╚════██║   ██║   ██╔══╝  ██║╚██╔╝██║
// ██║     ██║███████╗███████╗███████║   ██║   ███████║   ██║   ███████╗██║ ╚═╝ ██║
// ╚═╝     ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝   ╚══════╝   ╚═╝   ╚══════╝╚═╝     ╚═╝
// @filesystem @fs
sp_str_t sp_fs_normalize_path(sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_for(i, path.len) {
    c8 c = path.data[i];
    bool is_last = (i == path.len - 1);
    bool is_slash = (c == '/' || c == '\\');

    if (is_last && is_slash) {
      continue;
    }

    if (c == '\\') {
      sp_str_builder_append_c8(&builder, '/');
    }
    else {
      sp_str_builder_append_c8(&builder, c);
    }
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_str_t sp_fs_get_name(sp_str_t path) {
  // The only valid separator is '/'. If you're calling this function on a path
  // that might contain a '\', normalize it first. Garbage in, garbage out.
  s32 it = sp_str_find_c8_reverse(path, '/');
  return sp_str_suffix(path, ((s32)path.len) - it - 1);
}

sp_str_t sp_fs_parent_path(sp_str_t path) {
  if (sp_str_empty(path)) return path;
  if (sp_fs_is_root(path)) return path;

  path = sp_fs_trim_path(path);

  s32 index = sp_str_find_c8_reverse(path, '/');
  return index == SP_STR_NO_MATCH ? sp_str_lit("") : sp_str_prefix(path, index);
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

sp_str_t sp_fs_join_path(sp_str_t a, sp_str_t b) {
  a = sp_fs_trim_path(a);
  b = sp_fs_trim_path(b);
  if (sp_str_empty(a)) return sp_str_copy(b);
  if (sp_str_empty(b)) return sp_str_copy(a);
  return sp_str_join(a, b, SP_LIT("/"));
}

sp_str_t sp_fs_replace_ext(sp_str_t path, sp_str_t ext) {
  sp_str_t stripped = sp_str_strip_right(path, sp_fs_get_ext(path));
  return sp_str_empty(ext) ?
    stripped :
    sp_str_join(path, ext, sp_str_lit("."));
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

sp_err_t sp_fs_link(sp_str_t from, sp_str_t to, sp_fs_link_kind_t kind) {
  switch (kind) {
    case SP_FS_LINK_HARD:     return sp_fs_create_hard_link(from, to);
    case SP_FS_LINK_SYMBOLIC: return sp_fs_create_sym_link(from, to);
    case SP_FS_LINK_COPY:     return sp_fs_copy(from, to);
  }

  SP_UNREACHABLE_RETURN(SP_OK);
}

sp_da(sp_fs_entry_t) sp_fs_collect(sp_str_t path) {
  sp_da(sp_fs_entry_t) entries = SP_NULLPTR;

  for (sp_fs_it_t it = sp_fs_it_new(path); sp_fs_it_valid(&it); sp_fs_it_next(&it)) {
    sp_da_push(entries, it.entry);
  }
  return entries;
}

sp_da(sp_fs_entry_t) sp_fs_collect_recursive(sp_str_t path) {
  sp_da(sp_fs_entry_t) entries = SP_NULLPTR;

  for (sp_fs_it_t it = sp_fs_it_new_recursive(path); sp_fs_it_valid(&it); sp_fs_it_next(&it)) {
    sp_da_push(entries, it.entry);
  }
  return entries;
}


sp_err_t sp_fs_create_dir(sp_str_t path) {
  // The return code answers whether the path:
  // - Exists
  // - Is a plain directory
  if (sp_str_empty(path)) return SP_ERR_LAZY;
  if (sp_fs_exists(path)) {
    return sp_fs_is_dir(path) ? SP_OK : SP_ERR_LAZY;
  }

  sp_err_t result = SP_OK;
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_da(sp_str_t) missing = SP_NULLPTR;

  // Walk up, collecting intermediate paths that don't exist
  path = sp_fs_trim_path(path);

  while (!sp_fs_is_root(path) && !sp_fs_exists(path)) {
    sp_da_push(missing, path);
    path = sp_fs_parent_path(path);
  }

  // Walk back down and create each one
  sp_da_rfor(missing, it) {
    result = sp_os_create_dir(missing[it]);
    if (result && !sp_fs_exists(missing[it])) {
      goto cleanup;
    }
  }

cleanup:
  sp_mem_end_scratch(scratch);
  return result;
}

sp_err_t sp_fs_create_file(sp_str_t path) {
  return sp_os_create_file(path);
}

sp_err_t sp_fs_create_file_slice(sp_str_t path, sp_mem_slice_t slice) {
  sp_try(sp_os_create_file(path));
  sp_io_writer_t io = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file(&io, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_try(sp_io_write(&io, slice.data, slice.len, SP_NULLPTR));
  sp_try(sp_io_writer_close(&io));
  return SP_OK;
}

sp_err_t sp_fs_create_file_str(sp_str_t path, sp_str_t str) {
  sp_try(sp_os_create_file(path));
  sp_io_writer_t io = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file(&io, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_try(sp_io_write_str(&io, str, SP_NULLPTR));
  sp_try(sp_io_writer_close(&io));
  return SP_OK;
}

sp_err_t sp_fs_create_file_cstr(sp_str_t path, const c8* str) {
  return sp_fs_create_file_str(path, sp_str_view(str));
}

sp_err_t sp_fs_create_hard_link(sp_str_t target, sp_str_t link_path) {
  return sp_os_create_hard_link(target, link_path);
}

sp_err_t sp_fs_create_sym_link(sp_str_t target, sp_str_t link_path) {
  return sp_os_create_sym_link(target, link_path);
}

sp_err_t sp_fs_copy(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_glob(from)) {
    sp_fs_copy_glob(sp_fs_parent_path(from), sp_fs_get_name(from), to);
  }
  else if (sp_fs_is_target_dir(from)) {
    SP_ASSERT(sp_fs_is_target_dir(to));
    sp_fs_copy_glob(from, sp_str_lit("*"), sp_fs_join_path(to, sp_fs_get_name(from)));
  }
  else if (sp_fs_is_target_file(from)) {
    sp_fs_copy_file(from, to);
  }

  return SP_OK;
}

void sp_fs_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to) {
  sp_fs_create_dir(to);

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(from);

  sp_da_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];
    sp_str_t entry_name = entry->name;

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

void sp_fs_copy_dir(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  sp_fs_copy_glob(from, sp_str_lit("*"), to);
}

void sp_fs_copy_file(sp_str_t from, sp_str_t to) {
  if (sp_fs_is_dir(to)) {
    sp_fs_create_dir(to);
    to = sp_fs_join_path(to, sp_fs_get_name(from));
  }

  sp_os_attr_t attrs = sp_os_get_raw_target_attrs(from);
  if (!attrs) return;

  sp_io_reader_t reader = SP_ZERO_INITIALIZE();
  if (sp_io_reader_from_file(&reader, from)) return;

  sp_io_writer_t writer = SP_ZERO_INITIALIZE();
  if (sp_io_writer_from_file(&writer, to, SP_IO_WRITE_MODE_OVERWRITE)) {
    sp_io_reader_close(&reader);
    return;
  }

  u8 buffer[4096];
  u64 bytes_read = 0;
  while (sp_io_read(&reader, buffer, sizeof(buffer), &bytes_read) == SP_OK && bytes_read > 0) {
    sp_io_write(&writer, buffer, bytes_read, SP_NULLPTR);
  }

  sp_io_reader_close(&reader);
  sp_io_writer_close(&writer);

  sp_os_set_raw_file_attrs(to, attrs);
}

void sp_fs_remove_dir(sp_str_t path) {
  sp_da(sp_fs_entry_t) entries = sp_fs_collect(path);

  sp_da_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];

    if (sp_fs_is_symlink(entry->path)) {
      sp_fs_remove_file(entry->path);
    } else if (sp_fs_is_dir(entry->path)) {
      sp_fs_remove_dir(entry->path);
    } else if (sp_fs_is_file(entry->path)) {
      sp_fs_remove_file(entry->path);
    }
  }

  sp_os_remove_dir(path);
}

bool sp_fs_exists(sp_str_t path) {
  return sp_os_get_target_attrs(path) != SP_FS_KIND_NONE;
}

bool sp_fs_is_glob(sp_str_t path) {
  return sp_str_find_c8(path, '*') != SP_STR_NO_MATCH;
}

sp_fs_kind_t sp_fs_get_kind(sp_str_t path) {
  return sp_os_get_file_attrs(path);
}

sp_fs_kind_t sp_fs_get_target_kind(sp_str_t path) {
  return sp_os_get_target_attrs(path);
}

bool sp_fs_is_file(sp_str_t path) {
  return sp_os_get_file_attrs(path) == SP_FS_KIND_FILE;
}

bool sp_fs_is_symlink(sp_str_t path) {
  return sp_os_get_file_attrs(path) == SP_FS_KIND_SYMLINK;
}

bool sp_fs_is_dir(sp_str_t path) {
  return sp_os_get_file_attrs(path) == SP_FS_KIND_DIR;
}

bool sp_fs_is_target_file(sp_str_t path) {
  return sp_os_get_target_attrs(path) == SP_FS_KIND_FILE;
}

bool sp_fs_is_target_dir(sp_str_t path) {
  return sp_os_get_target_attrs(path) == SP_FS_KIND_DIR;
}

bool sp_fs_is_root(sp_str_t path) {
  if (path.len == 0) return true;
  if (path.len == 1 && path.data[0] == '/') return true;
  if (path.len == 2 && path.data[1] == ':') return true;
  if (path.len == 3 && path.data[1] == ':' && (path.data[2] == '/' || path.data[2] == '\\')) return true;
  return false;
}

sp_str_t sp_fs_get_cwd() {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_t cwd = sp_os_get_cwd();

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_fs_normalize_path(cwd);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

void sp_fs_remove_file(sp_str_t path) {
  sp_os_remove_file(path);
}

SP_PRIVATE bool sp_fs_it_os_open(sp_fs_it_frame_t* frame, sp_str_t path);
SP_PRIVATE void sp_fs_it_os_close(sp_fs_it_frame_t* frame);
SP_PRIVATE bool sp_fs_it_os_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry);
SP_PRIVATE void sp_fs_it_push(sp_fs_it_t* it, sp_str_t path);

sp_fs_it_t sp_fs_it_new(sp_str_t path) {
  sp_fs_it_t it = SP_ZERO_INITIALIZE();
  sp_fs_it_begin(&it, path);
  return it;
}

sp_fs_it_t sp_fs_it_new_recursive(sp_str_t path) {
  sp_fs_it_t it = { .recursive = true };
  sp_fs_it_begin(&it, path);
  return it;
}

void sp_fs_it_begin(sp_fs_it_t* it, sp_str_t path) {
  if (sp_str_empty(path) || !sp_fs_is_dir(path)) return;

  sp_fs_it_push(it, path);
  sp_fs_it_next(it);
}

void sp_fs_it_push(sp_fs_it_t* it, sp_str_t path) {
  sp_fs_it_frame_t frame = SP_ZERO_INITIALIZE();
  if (!sp_fs_it_os_open(&frame, path)) {
    return;
  }

  frame.path = sp_str_copy(path);
  sp_da_push(it->stack, frame);
}

void sp_fs_it_next(sp_fs_it_t* it) {
  while (!sp_da_empty(it->stack)) {
    sp_fs_it_frame_t* top = sp_da_back(it->stack);

    if (sp_fs_it_os_read(top, &it->entry)) {
      it->entry.path = sp_fs_join_path(top->path, it->entry.name);

      if (it->recursive && it->entry.kind == SP_FS_KIND_DIR) {
        sp_fs_it_push(it, it->entry.path);
      }
      return;
    }

    sp_fs_it_os_close(top);
    sp_da_pop(it->stack);
  }
}

bool sp_fs_it_valid(sp_fs_it_t* it) {
  return !sp_da_empty(it->stack);
}

void sp_fs_it_deinit(sp_fs_it_t* it) {
  sp_da_for(it->stack, i) {
    sp_fs_it_os_close(&it->stack[i]);
  }
  sp_da_free(it->stack);
}

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

bool sp_fs_it_os_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  sp_str_t pattern = sp_fs_join_path(path, sp_str_lit("*"));
  frame->handle = FindFirstFileA(sp_str_to_cstr(pattern), &frame->find_data);
  if (frame->handle == INVALID_HANDLE_VALUE) return false;
  frame->first = true;
  return true;
}

void sp_fs_it_os_close(sp_fs_it_frame_t* frame) {
  FindClose(frame->handle);
}

bool sp_fs_it_os_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  while (true) {
    if (frame->first) {
      frame->first = false;
    } else {
      if (!FindNextFileA(frame->handle, &frame->find_data)) return false;
    }
    if (sp_fs_it_is_dot(frame->find_data.cFileName)) continue;
    entry->name = sp_str_from_cstr(frame->find_data.cFileName);
    entry->kind = sp_fs_it_win32_attrs(frame->find_data.dwFileAttributes);
    return true;
  }
}

#elif defined(SP_UNIX)

SP_PRIVATE sp_fs_kind_t sp_fs_it_dtype_to_attr(u8 d_type) {
  switch (d_type) {
    case SP_DT_REG: { return SP_FS_KIND_FILE; }
    case SP_DT_DIR: { return SP_FS_KIND_DIR; }
    case SP_DT_LNK: { return SP_FS_KIND_SYMLINK; }
  }
  return SP_FS_KIND_NONE;
}

#if defined(SP_LINUX)

bool sp_fs_it_os_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  frame->fd = sp_sys_open(sp_str_to_cstr(path), SP_O_RDONLY | SP_O_DIRECTORY, 0);
  return frame->fd >= 0;
}

void sp_fs_it_os_close(sp_fs_it_frame_t* frame) {
  sp_sys_close(frame->fd);
}

bool sp_fs_it_os_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
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

#else
bool sp_fs_it_os_open(sp_fs_it_frame_t* frame, sp_str_t path) {
  frame->dir = opendir(sp_str_to_cstr(path));
  return frame->dir != SP_NULLPTR;
}

void sp_fs_it_os_close(sp_fs_it_frame_t* frame) {
  closedir(frame->dir);
}

bool sp_fs_it_os_read(sp_fs_it_frame_t* frame, sp_fs_entry_t* entry) {
  while (true) {
    struct dirent* d = readdir(frame->dir);
    if (!d) return false;
    if (sp_fs_it_is_dot(d->d_name)) continue;
    entry->name = sp_str_from_cstr(d->d_name);
    entry->kind = sp_fs_it_dtype_to_attr(d->d_type);
    return true;
  }
}

#endif
#endif


//////////////////
// CANONICALIZE //
//////////////////
#if defined(SP_WIN32)
sp_str_t sp_fs_canonicalize_path(sp_str_t path) {
  if (sp_str_empty(path)) return SP_ZERO_STRUCT(sp_str_t);

  c8* path_cstr = sp_str_to_cstr(path);

  HANDLE h = CreateFileA(
    path_cstr,
    0,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NULLPTR,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS,
    SP_NULLPTR
  );

  if (h == INVALID_HANDLE_VALUE) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  c8 canonical_path[SP_MAX_PATH_LEN];
  sp_win32_dword_t len = GetFinalPathNameByHandleA(h, canonical_path, SP_MAX_PATH_LEN, 0);
  CloseHandle(h);

  if (len == 0 || len >= SP_MAX_PATH_LEN) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  // GetFinalPathNameByHandleA returns \\?\ prefix — strip it
  sp_str_t result = (sp_str_t) { .data = canonical_path, .len = len };
  if (sp_str_starts_with(result, SP_LIT("\\\\?\\"))) {
    result = sp_str_suffix(result, result.len - 4);
  }

  result = sp_fs_normalize_path(result);

  return sp_str_copy(result);
}
#elif defined(SP_LINUX)
sp_str_t sp_fs_canonicalize_path(sp_str_t path) {
  sp_str_t result = sp_zero_struct(sp_str_t);
  if (sp_str_empty(path)) return result;

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  s64 fd = sp_openat(SP_AT_FDCWD, sp_str_to_cstr(path), SP_O_RDONLY | SP_O_CLOEXEC, 0);
  if (fd < 0) {
    goto cleanup;
  }

  // @spader I need an sprintf() equivalent, plus format-direct-to-cstr
  const c8* proc = sp_str_to_cstr(sp_fmt("/proc/self/fd/{}", sp_fmt_int(fd)));

  c8 buffer[SP_MAX_PATH_LEN];
  s64 len = sp_lx_readlink_at(proc, buffer, SP_MAX_PATH_LEN - 1);
  sp_sys_close(fd);

  if (len <= 0) {
    goto cleanup;
  }

  sp_str_t canonical_path = {
    .data = buffer,
    .len = len
  };
  result = sp_fs_normalize_path(canonical_path);

  sp_context_push_allocator(scratch.old_allocator);
  result = sp_str_copy(result);
  sp_context_pop();

cleanup:
  sp_mem_end_scratch(scratch);
  return result;
}
#else
sp_str_t sp_fs_canonicalize_path(sp_str_t path) {
  if (sp_str_empty(path)) return SP_ZERO_STRUCT(sp_str_t);

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  c8 canonical_path[SP_MAX_PATH_LEN] = SP_ZERO_INITIALIZE();
  c8* path_cstr = sp_str_to_cstr(path);

  if (!realpath(path_cstr, canonical_path)) {
    sp_mem_end_scratch(scratch);
    return SP_ZERO_STRUCT(sp_str_t);
  }

  sp_str_t result = sp_fs_normalize_path(SP_CSTR(canonical_path));

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t copy = sp_str_copy(result);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return copy;
}
#endif

//////////////
// EXE PATH //
//////////////
#if defined(SP_WIN32)
sp_str_t sp_fs_get_exe_path() {
  c8 exe_path[SP_MAX_PATH_LEN] = SP_ZERO_INITIALIZE();
  GetModuleFileNameA(NULL, exe_path, SP_MAX_PATH_LEN);

  sp_str_t path = sp_fs_normalize_path(sp_str_view(exe_path));

  return sp_str_copy(path);
}
#endif

#if defined(SP_COSMO)
extern char* program_invocation_name;

sp_str_t sp_fs_get_exe_path() {
  c8 exe_path [SP_PATH_MAX] = SP_ZERO_INITIALIZE();
  if (realpath(program_invocation_name, exe_path)) {
    return sp_str_copy(sp_fs_normalize_path(SP_CSTR(exe_path)));
  }
  return sp_str_lit("");
}
#endif

#if defined(SP_LINUX)
sp_str_t sp_fs_get_exe_path() {
  c8 exe_path[SP_PATH_MAX];
  s64 len = sp_lx_readlink_at("/proc/self/exe", exe_path, SP_PATH_MAX - 1);
  if (len < 0) {
    return sp_str_lit("");
  }

  exe_path[len] = '\0';
  return sp_str_copy(sp_fs_normalize_path(SP_CSTR(exe_path)));
}
#endif

#if defined(SP_MACOS)
sp_str_t sp_fs_get_exe_path() {
  c8 exe_path[SP_PATH_MAX];

  u32 len = SP_PATH_MAX;
  if (_NSGetExecutablePath(exe_path, &len)) {
    return sp_str_lit("");
  }

  c8 canonical[SP_PATH_MAX];
  if (realpath(exe_path, canonical)) {
    return sp_str_copy(sp_fs_normalize_path(SP_CSTR(canonical)));
  }

  return sp_str_copy(sp_fs_normalize_path(SP_CSTR(exe_path)));
}
#endif

//////////////////
// SYSTEM PATHS //
//////////////////
#if defined(SP_WIN32)
sp_str_t sp_os_try_xdg_or_home(sp_str_t xdg, sp_str_t home_suffix) {
  (void)xdg;
  (void)home_suffix;
  return SP_ZERO_STRUCT(sp_str_t);
}

sp_str_t sp_fs_get_storage_path() {
  return sp_fs_normalize_path(sp_os_env_get(SP_LIT("LOCALAPPDATA")));
}

sp_str_t sp_fs_get_config_path() {
  return sp_fs_normalize_path(sp_os_env_get(SP_LIT("APPDATA")));
}

#else
sp_str_t sp_os_try_xdg_or_home(sp_str_t xdg, sp_str_t home_suffix) {
  sp_str_t path =  sp_os_env_get(xdg);
  if (sp_str_valid(path)) return path;

  path = sp_os_env_get(SP_LIT("HOME"));
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

//////////////////
// GET MOD TIME //
//////////////////
sp_tm_epoch_t sp_fs_get_mod_time(sp_str_t file_path) {
  sp_tm_epoch_t result = sp_zero_struct(sp_tm_epoch_t);

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_sys_stat_t st;
  if (sp_sys_stat(sp_str_to_cstr(file_path), &st) == 0) {
    result.s = (u64)st.st_mtim.tv_sec;
    result.ns = (u32)st.st_mtim.tv_nsec;
  }
  sp_mem_end_scratch(scratch);
  return result;
}

/////////////////
// WIN32 UTILS //
/////////////////
#if defined(SP_WIN32)
SP_PRIVATE c8* sp_fs_win32_path_for_api(sp_str_t path) {
  sp_str_t normalized = sp_fs_normalize_path(path);
  c8* path_cstr = sp_str_to_cstr(normalized);
  if (normalized.len < SP_MAX_PATH_LEN - 12) {
    return path_cstr;
  }

  sp_win32_dword_t absolute_len = GetFullPathNameA(path_cstr, 0, SP_NULLPTR, SP_NULLPTR);
  if (!absolute_len) {
    return path_cstr;
  }

  c8* absolute = sp_alloc_n(c8, absolute_len);
  if (!GetFullPathNameA(path_cstr, absolute_len, absolute, SP_NULLPTR)) {
    sp_free(absolute);
    return path_cstr;
  }

  sp_free(path_cstr);

  u32 len = sp_cstr_len(absolute);
  sp_for(i, len) {
    if (absolute[i] == '/') {
      absolute[i] = '\\';
    }
  }

  bool is_unc = len >= 2 && absolute[0] == '\\' && absolute[1] == '\\';
  c8* prefixed = sp_alloc_n(c8, len + (is_unc ? 7 : 4) + 1);
  if (is_unc) {
    prefixed[0] = '\\';
    prefixed[1] = '\\';
    prefixed[2] = '?';
    prefixed[3] = '\\';
    prefixed[4] = 'U';
    prefixed[5] = 'N';
    prefixed[6] = 'C';
    prefixed[7] = '\\';
    sp_mem_copy(absolute + 2, prefixed + 8, len - 1);
  }
  else {
    prefixed[0] = '\\';
    prefixed[1] = '\\';
    prefixed[2] = '?';
    prefixed[3] = '\\';
    sp_mem_copy(absolute, prefixed + 4, len + 1);
  }

  sp_free(absolute);
  return prefixed;
}
#endif


//  ██████╗ ███████╗
// ██╔═══██╗██╔════╝
// ██║   ██║███████╗
// ██║   ██║╚════██║
// ╚██████╔╝███████║
//  ╚═════╝ ╚══════╝
//  @os

///////////
// SLEEP //
///////////
void sp_os_sleep_ns(u64 ns) {
  sp_sys_timespec_t req = {
    .tv_sec  = (s64)(ns / 1000000000ULL),
    .tv_nsec = (s64)(ns % 1000000000ULL),
  };
  sp_sys_timespec_t rem = SP_ZERO_INITIALIZE();
  while (sp_sys_nanosleep(&req, &rem) == -1 && errno == SP_EINTR) {
    req = rem;
  }
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



///////////
// WRITE //
///////////
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

void sp_os_tty_size(sp_sys_fd_t fd, s32* cols, s32* rows) {
  if (cols) *cols = 0;
  if (rows) *rows = 0;
  HANDLE handle = (HANDLE)fd;
  if (handle == INVALID_HANDLE_VALUE || handle == SP_NULLPTR) return;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(handle, &csbi)) return;
  if (cols) *cols = (s32)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
  if (rows) *rows = (s32)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

#elif defined(SP_SYS)
bool sp_os_is_tty(sp_os_file_handle_t fd) {
  sp_sys_termios_t t = SP_ZERO_STRUCT(sp_sys_termios_t);
  return sp_sys_tcgetattr(fd, &t) == 0;
}

void sp_os_tty_size(sp_os_file_handle_t fd, s32* cols, s32* rows) {
  if (cols) *cols = 0;
  if (rows) *rows = 0;
  sp_sys_winsize_t ws = SP_ZERO_STRUCT(sp_sys_winsize_t);
  if (sp_sys_ioctl(fd, SP_TIOCGWINSZ, &ws) < 0) return;
  if (cols) *cols = (s32)ws.ws_col;
  if (rows) *rows = (s32)ws.ws_row;
}

#else
bool sp_os_is_tty(sp_os_file_handle_t fd) {
  return isatty(fd) != 0;
}

void sp_os_tty_size(sp_os_file_handle_t fd, s32* cols, s32* rows) {
  if (cols) *cols = 0;
  if (rows) *rows = 0;
  struct winsize ws = SP_ZERO_STRUCT(struct winsize);
  if (ioctl(fd, TIOCGWINSZ, &ws) != 0) return;
  if (cols) *cols = (s32)ws.ws_col;
  if (rows) *rows = (s32)ws.ws_row;
}
#endif


///////////
// ENUMS //
///////////
#if defined(SP_WIN32)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_WIN32;
}

sp_str_t sp_os_get_name() {
  return sp_str_lit("windows");
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
  return sp_fmt("{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind)));
}
#endif

#if defined(SP_MACOS)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_MACOS;
}

sp_str_t sp_os_get_name() {
  return sp_str_lit("macos");
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
  return sp_fmt("lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind)));
}
#endif

#if defined(SP_LINUX)
sp_os_kind_t sp_os_get_kind() {
  return SP_OS_LINUX;
}

sp_str_t sp_os_get_name() {
  return sp_str_lit("linux");
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
  return sp_fmt("lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind)));
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
  return sp_fmt("lib{}.{}", sp_fmt_str(lib_name), sp_fmt_str(sp_os_lib_kind_to_extension(kind)));
}
#endif


///////////
// MKDIR //
///////////
sp_err_t sp_os_create_dir(sp_str_t path) {
  return sp_sys_mkdir(sp_str_to_cstr(path), 0755) == 0 ? SP_OK : SP_ERR_OS;
}


////////////////////
// GET FILE ATTRS //
////////////////////
#if defined(SP_WIN32)
sp_fs_kind_t sp_os_get_file_attrs(sp_str_t path) {
  if (sp_str_empty(path)) return SP_FS_KIND_NONE;
  sp_os_attr_t attrs = sp_os_get_raw_file_attrs(path);

  if (attrs == INVALID_FILE_ATTRIBUTES) return SP_FS_KIND_NONE;
  if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) return SP_FS_KIND_SYMLINK;
  if (attrs & FILE_ATTRIBUTE_DIRECTORY) return SP_FS_KIND_DIR;
  return SP_FS_KIND_FILE;
}

sp_fs_kind_t sp_os_get_target_attrs(sp_str_t path) {
  if (sp_str_empty(path)) return SP_FS_KIND_NONE;
  sp_os_attr_t attrs = sp_os_get_raw_target_attrs(path);

  if (attrs == INVALID_FILE_ATTRIBUTES) return SP_FS_KIND_NONE;
  if (attrs & FILE_ATTRIBUTE_DIRECTORY) return SP_FS_KIND_DIR;
  return SP_FS_KIND_FILE;
}

#else
sp_fs_kind_t sp_os_attr_to_fs_attr(sp_os_attr_t attrs) {
  if (SP_S_ISLNK(attrs)) return SP_FS_KIND_SYMLINK;
  if (SP_S_ISDIR(attrs)) return SP_FS_KIND_DIR;
  if (SP_S_ISREG(attrs)) return SP_FS_KIND_FILE;
  return SP_FS_KIND_NONE;
}

sp_fs_kind_t sp_os_get_file_attrs(sp_str_t path) {
  return sp_os_attr_to_fs_attr(sp_os_get_raw_file_attrs(path));
}

sp_fs_kind_t sp_os_get_target_attrs(sp_str_t path) {
  return sp_os_attr_to_fs_attr(sp_os_get_raw_target_attrs(path));
}
#endif


////////////////////////
// GET RAW FILE ATTRS //
////////////////////////
#if defined(SP_WIN32)
sp_os_attr_t sp_os_get_raw_file_attrs(sp_str_t path) {
  c8* path_cstr = sp_fs_win32_path_for_api(path);
  sp_win32_dword_t attrs = GetFileAttributesA(path_cstr);
  sp_free(path_cstr);

  return attrs;
}

sp_os_attr_t sp_os_get_raw_target_attrs(sp_str_t path) {
  return sp_os_get_raw_file_attrs(path);
}

#else
sp_os_attr_t sp_os_get_raw_file_attrs(sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  c8* cstr = sp_str_to_cstr(path);
  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  s32 result = 0;

  result = sp_sys_lstat(cstr, &st);

  sp_mem_end_scratch(scratch);
  return result ? 0 : st.st_mode;
}

sp_os_attr_t sp_os_get_raw_target_attrs(sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  c8* cstr = sp_str_to_cstr(path);
  sp_sys_stat_t st = SP_ZERO_INITIALIZE();
  s32 result = 0;

  result = sp_sys_stat(cstr, &st);

  sp_mem_end_scratch(scratch);
  return result ? 0 : st.st_mode;
}
#endif


////////////////////////
// SET RAW FILE ATTRS //
////////////////////////
#if defined(SP_WIN32)
sp_err_t sp_os_set_raw_file_attrs(sp_str_t path, sp_os_attr_t attrs) {
  c8* path_cstr = sp_fs_win32_path_for_api(path);
  bool ok = SetFileAttributesA(path_cstr, attrs);
  sp_free(path_cstr);
  return ok ? SP_OK : SP_ERR_OS;
}

#else
sp_err_t sp_os_set_raw_file_attrs(sp_str_t path, sp_os_attr_t attrs) {
  return sp_sys_chmod(sp_str_to_cstr(path), attrs) == 0 ? SP_OK : SP_ERR_OS;
}
#endif


/////////////
// GET CWD //
/////////////
sp_str_t sp_os_get_cwd() {
  c8 path[SP_PATH_MAX] = SP_ZERO_INITIALIZE();
  if (sp_sys_getcwd(path, SP_PATH_MAX - 1) < 0) {
    return SP_ZERO_STRUCT(sp_str_t);
  }

  return sp_str_from_cstr(path);
}


/////////////////
// CREATE FILE //
/////////////////
#if defined(SP_WIN32)
sp_err_t sp_os_create_file(sp_str_t path) {
  c8* path_cstr = sp_fs_win32_path_for_api(path);
  HANDLE handle = CreateFileA(path_cstr, GENERIC_WRITE, 0, SP_NULLPTR, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, SP_NULLPTR);
  sp_free(path_cstr);
  if (handle == INVALID_HANDLE_VALUE) {
    return SP_ERR_OS;
  }

  CloseHandle(handle);
  return SP_OK;
}

#else
sp_err_t sp_os_create_file(sp_str_t path) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();

  sp_sys_fd_t fd = sp_sys_open(sp_str_to_cstr(path), SP_O_CREAT | SP_O_WRONLY | SP_O_TRUNC, 0644);
  if (fd != SP_SYS_INVALID_FD) {
    sp_sys_close(fd);
  }

  sp_mem_end_scratch(scratch);
  return fd != SP_SYS_INVALID_FD ? SP_OK : SP_ERR_OS;
}
#endif


/////////////////
// REMOVE FILE //
/////////////////
#if defined(SP_WIN32)
sp_err_t sp_os_remove_file(sp_str_t path) {
  c8* path_cstr = sp_fs_win32_path_for_api(path);
  s32 rc = sp_sys_unlink(path_cstr);
  sp_free(path_cstr);
  return rc == 0 ? SP_OK : SP_ERR_OS;
}

#else
sp_err_t sp_os_remove_file(sp_str_t path) {
  return sp_sys_unlink(sp_str_to_cstr(path)) == 0 ? SP_OK : SP_ERR_OS;
}
#endif


////////////////
// REMOVE DIR //
////////////////
#if defined(SP_WIN32)
sp_err_t sp_os_remove_dir(sp_str_t path) {
  c8* path_cstr = sp_fs_win32_path_for_api(path);
  s32 rc = sp_sys_rmdir(path_cstr);
  sp_free(path_cstr);
  return rc == 0 ? SP_OK : SP_ERR_OS;
}

#else
sp_err_t sp_os_remove_dir(sp_str_t path) {
  return sp_sys_rmdir(sp_str_to_cstr(path)) == 0 ? SP_OK : SP_ERR_OS;
}
#endif

//////////////////////
// CREATE HARD LINK //
//////////////////////
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

/////////
// ENV //
/////////
#if defined(SP_WIN32)
typedef struct {
  u8* base;
  struct {
    u8* base;
    c16* env;
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
  peb.params.env = *(c16**)(peb.params.base + 0x80);

  return peb;
}

SP_PRIVATE sp_str_t sp_win32_utf16_to_utf8(const c16* utf16, s32 len) {
  s32 num_bytes = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)utf16, len, NULL, 0, NULL, NULL);
  c8* utf8 = sp_alloc_n(c8, num_bytes + 1);
  WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)utf16, len, utf8, num_bytes, NULL, NULL);
  utf8[num_bytes] = '\0';
  return sp_rval(sp_str_t) {
    .data = utf8,
    .len = num_bytes
  };
}

SP_PRIVATE u32 sp_win32_utf16_len(const c16* str) {
  u32 len = 0;
  while (str[len]) len++;
  return len;
}

SP_PRIVATE bool sp_win32_utf16_equals_cstr(const c16* a, u32 a_len, const c8* b, u32 b_len) {
  if (a_len != b_len) return false;
  for (u32 i = 0; i < a_len; i++) {
    c16 wa = a[i];
    c16 wb = (c16)(u8)b[i];
    if (wa >= 'A' && wa <= 'Z') wa += 32;
    if (wb >= 'A' && wb <= 'Z') wb += 32;
    if (wa != wb) return false;
  }
  return true;
}

sp_str_t sp_os_env_get(sp_str_t key) {
  sp_win32_peb_t peb = sp_win32_get_peb();

  c16* p = peb.params.env;
  while (*p) {
    u32 eq = (*p == L'=') ? 1 : 0;
    while (p[eq] && p[eq] != L'=') eq++;

    if (sp_win32_utf16_equals_cstr(p, eq, key.data, key.len)) {
      c16* value = p + eq + 1;
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
  c16* block;
  c16* cursor;
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

#else
sp_str_t sp_os_env_get(sp_str_t key) {
  const c8* value = getenv(sp_str_to_cstr(key));
  return sp_str_view(value);
}

SP_PRIVATE void sp_os_env_it_set(sp_os_env_it_t* it) {
  c8** envp = (c8**)it->os;
  sp_str_pair_t pair = sp_str_cleave_c8(sp_str_view(*envp), '=');
  it->key = pair.first;
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
#endif

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
    case SP_OS_SIGNAL_COUNT_:    return -1;
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
  SP_BROKEN();
}

#endif



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

#if defined(SP_WIN32)
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

sp_str_t sp_tm_epoch_to_iso8601(sp_tm_epoch_t time) {
  struct tm* time_info = SP_NULLPTR;
  time_t raw_time = (time_t)time.s;
  time_info = gmtime(&raw_time);

  c8 buffer[256];
  u32 len = sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d",
    time_info->tm_year + 1900,
    time_info->tm_mon + 1,
    time_info->tm_mday,
    time_info->tm_hour,
    time_info->tm_min,
    time_info->tm_sec);

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, sp_str(buffer, len));
  sp_str_builder_append_c8(&builder, '.');

  u32 ms = time.ns / 1000000;
  if (ms < 100) sp_str_builder_append_c8(&builder, '0');
  if (ms < 10) sp_str_builder_append_c8(&builder, '0');
  sp_str_builder_append_fmt(&builder, "{}", sp_fmt_uint(ms));
  sp_str_builder_append_c8(&builder, 'Z');

  return sp_str_builder_to_str(&builder);
}
#elif defined(SP_FREESTANDING)
sp_tm_date_time_t sp_tm_get_date_time() {
  return sp_zero_struct(sp_tm_date_time_t);
}

sp_str_t sp_tm_epoch_to_iso8601(sp_tm_epoch_t time) {
  return sp_str_lit("");
}

#else
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
  sp_str_builder_append_fmt(&builder, "{}", sp_fmt_uint(ms));
  sp_str_builder_append_c8(&builder, 'Z');

  return sp_str_builder_to_str(&builder);
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

//  █████╗ ████████╗ ██████╗ ███╗   ███╗██╗ ██████╗
// ██╔══██╗╚══██╔══╝██╔═══██╗████╗ ████║██║██╔════╝
// ███████║   ██║   ██║   ██║██╔████╔██║██║██║
// ██╔══██║   ██║   ██║   ██║██║╚██╔╝██║██║██║
// ██║  ██║   ██║   ╚██████╔╝██║ ╚═╝ ██║██║╚██████╗
// ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝     ═╝╚═╝ ╚═════╝
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

// ███████╗███████╗███╗   ███╗ █████╗ ██████╗ ██╗  ██╗ ██████╗ ██████╗ ███████╗
// ██╔════╝██╔════╝████╗ ████║██╔══██╗██╔══██╗██║  ██║██╔═══██╗██╔══██╗██╔════╝
// ███████╗█████╗  ██╔████╔██║███████║██████╔╝███████║██║   ██║██████╔╝█████╗
// ╚════██║██╔══╝  ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔══██║██║   ██║██╔══██╗██╔══╝
// ███████║███████╗██║ ╚═╝ ██║██║  ██║██║     ██║  ██║╚██████╔╝██║  ██║███████╗
// ╚══════╝╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
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
  SP_BROKEN();
}

void sp_semaphore_destroy(sp_semaphore_t* semaphore) {
  SP_BROKEN();
}

void sp_semaphore_wait(sp_semaphore_t* semaphore) {
  SP_BROKEN();
}

bool sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms) {
  SP_BROKEN();
  return true;
}

void sp_semaphore_signal(sp_semaphore_t* semaphore) {
  SP_BROKEN();
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

#elif defined(SP_FREESTANDING)
void sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind) {
}

void sp_mutex_lock(sp_mutex_t* mutex) {
  SP_ASSERT(false);
}

void sp_mutex_unlock(sp_mutex_t* mutex) {
  SP_ASSERT(false);
}

void sp_mutex_destroy(sp_mutex_t* mutex) {
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
  SP_BROKEN();
}

void sp_mutex_lock(sp_mutex_t* mutex) {
  SP_BROKEN();
}

void sp_mutex_unlock(sp_mutex_t* mutex) {
  SP_BROKEN();
}

void sp_mutex_destroy(sp_mutex_t* mutex) {
  SP_BROKEN();
}

#endif

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

#elif defined(SP_FREESTANDING)
void sp_cv_init(sp_cv_t* cond) {

}

void sp_cv_destroy(sp_cv_t* cond) {

}

void sp_cv_wait(sp_cv_t* cond, sp_mutex_t* mutex) {
  SP_ASSERT(false);
}

bool sp_cv_wait_for(sp_cv_t* cond, sp_mutex_t* mutex, u32 ms) {
  SP_ASSERT(false);
  return true;
}

void sp_cv_notify_one(sp_cv_t* cond) {
  SP_ASSERT(false);
}

void sp_cv_notify_all(sp_cv_t* cond) {
  SP_ASSERT(false);
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
  SP_BROKEN();
}

void sp_cv_destroy(sp_cv_t* cond) {
  SP_BROKEN();
}

void sp_cv_wait(sp_cv_t* cond, sp_mutex_t* mutex) {
  SP_BROKEN();
}

bool sp_cv_wait_for(sp_cv_t* cond, sp_mutex_t* mutex, u32 ms) {
  SP_BROKEN();
  return false;
}

void sp_cv_notify_one(sp_cv_t* cond) {
  SP_BROKEN();
}

void sp_cv_notify_all(sp_cv_t* cond) {
  SP_BROKEN();
}

#endif


// ████████╗██╗  ██╗██████╗ ███████╗ █████╗ ██████╗
// ╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗██╔══██╗
//    ██║   ███████║██████╔╝█████╗  ███████║██║  ██║
//    ██║   ██╔══██║██╔══██╗██╔══╝  ██╔══██║██║  ██║
//    ██║   ██║  ██║██║  ██║███████╗██║  ██║██████╔╝
//    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝
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
#elif defined(SP_FREESTANDING)
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
  SP_ASSERT(false);
}

void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
  SP_ASSERT(false);
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

#else
void* sp_posix_thread_launch(void* args) {
  SP_BROKEN();
}

s32 sp_thread_launch(void* args) {
  SP_BROKEN();
  return 0;
}

void sp_thread_join(sp_thread_t* thread) {
  SP_BROKEN();
}

void sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata) {
  SP_BROKEN();
}

#endif


// ███████╗███╗   ██╗██╗   ██╗
// ██╔════╝████╗  ██║██║   ██║
// █████╗  ██╔██╗ ██║██║   ██║
// ██╔══╝  ██║╚██╗██║╚██╗ ██╔╝
// ███████╗██║ ╚████║ ╚████╔╝
// ╚══════╝╚═╝  ╚═══╝  ╚═══╝
// @env
void sp_env_init(sp_env_t* env) {
  *env = SP_ZERO_STRUCT(sp_env_t);
}

sp_env_t sp_env_capture() {
  sp_env_t env = SP_ZERO_INITIALIZE();
  for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
    sp_str_ht_insert(env.vars, sp_str_copy(it.key), sp_str_copy(it.value));
  }

  return env;
}

sp_env_t sp_env_copy(sp_env_t* env) {
  sp_env_t copy = SP_ZERO_INITIALIZE();

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


// ██████╗ ██████╗  ██████╗  ██████╗███████╗███████╗███████╗
// ██╔══██╗██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██╔════╝
// ██████╔╝██████╔╝██║   ██║██║     █████╗  ███████╗███████╗
// ██╔═══╝ ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║╚════██║
// ██║     ██║  ██║╚██████╔╝╚██████╗███████╗███████║███████║
// ╚═╝     ╚═╝  ╚═╝ ╚═════╝  ╚═════╝╚══════╝╚══════╝╚══════╝
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

SP_PRIVATE sp_env_t sp_ps_build_env(sp_ps_env_config_t* config) {
  sp_env_t env = SP_ZERO_INITIALIZE();

  switch (config->mode) {
    case SP_PS_ENV_INHERIT: {
      env = sp_env_capture();
      break;
    }
    case SP_PS_ENV_EXISTING: {
      env = sp_env_copy(&config->env);
      break;
    }
    case SP_PS_ENV_CLEAN: {
    }
  }

  sp_for(i, SP_PS_MAX_ENV) {
    if (sp_str_empty(config->extra[i].key)) break;
    sp_env_insert(&env, config->extra[i].key, config->extra[i].value);
  }

  return env;
}

bool sp_ps_is_fd_valid(sp_os_file_handle_t fd) {
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
  sp_dyn_array(c8*) envp = SP_NULLPTR;

  sp_str_ht_for(env->vars, it) {
    sp_str_t key = *sp_str_ht_it_getkp(env->vars, it);
    sp_str_t val = *sp_str_ht_it_getp(env->vars, it);

    sp_str_builder_t builder = SP_ZERO_INITIALIZE();
    sp_str_builder_append_fmt(&builder, "{}={}", sp_fmt_str(key), sp_fmt_str(val));
    sp_dyn_array_push(envp, sp_str_to_cstr(sp_str_builder_to_str(&builder)));
  }

  sp_dyn_array_push(envp, SP_NULLPTR);
  return envp;
}

void sp_env_free_posix_envp(c8** envp) {
  if (!envp) return;

  for (u32 i = 0; envp[i] != SP_NULLPTR; i++) {
    sp_free(envp[i]);
  }
  sp_dyn_array_free(envp);
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  sp_ps_t proc = SP_ZERO_STRUCT(sp_ps_t);
  proc.allocator = sp_context_get()->allocator;
  proc.io = config.io;

  SP_ASSERT(!sp_str_empty(config.command));

  c8** argv = sp_ps_build_posix_args(&config);
  sp_env_t env = sp_ps_build_env(&config.env);
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
    SP_FATAL(
      "You called sp_ps_run() but your config redirected stdout. sp_ps_run() always creates a new "
      "file descriptor for stdout, since its purpose is to run a command and capture output without "
      "the command polluting the parent command's output. If you really wanted this, use sp_ps_create() "
      "and sp_ps_output(). The failing command was {.fg brightyellow}",
      sp_fmt_str(config.command)
    );
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
    sp_err_set(SP_ERR_LAZY);
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
    sp_str_builder_t out;
    sp_str_builder_t err;
  } write = SP_ZERO_INITIALIZE();

  sp_pollfd_t fds[2];
  sp_io_reader_t* readers[2];
  sp_str_builder_t* builders[2];
  s32 nfds = 0;

  if (read.out) {
    fds[nfds] = (sp_pollfd_t){ .fd = read.out->file.fd, .events = SP_POLLIN };
    readers[nfds] = read.out;
    builders[nfds] = &write.out;
    nfds++;
  }
  if (read.err) {
    fds[nfds] = (sp_pollfd_t){ .fd = read.err->file.fd, .events = SP_POLLIN };
    readers[nfds] = read.err;
    builders[nfds] = &write.err;
    nfds++;
  }

  while (nfds > 0) {
    s32 ret = sp_poll(fds, nfds, -1);
    if (ret < 0) {
      if (errno == SP_EINTR) continue;
      break;
    }

    sp_for(i, (u32)nfds) {
      if (!(fds[i].revents & (SP_POLLIN | SP_POLLHUP))) {
        continue;
      }

      u64 n = 0;
      sp_io_read(readers[i], buffer, sizeof(buffer), &n);
      if (n > 0) {
        sp_str_builder_append(builders[i], sp_str((c8*)buffer, n));
      } else {
        fds[i] = fds[nfds - 1];
        readers[i] = readers[nfds - 1];
        builders[i] = builders[nfds - 1];
        nfds--;
        i--;
      }
    }
  }

  result.out = sp_str_builder_as_str(&write.out);
  result.err = sp_str_builder_as_str(&write.err);
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

bool sp_ps_is_fd_valid(sp_os_file_handle_t fd) {
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

void sp_ps_win32_append_quoted_arg(sp_str_builder_t* builder, sp_str_t arg) {
  bool needs_quotes = sp_str_empty(arg);
  sp_for(i, arg.len) {
    c8 c = arg.data[i];
    if (c == ' ' || c == '\t' || c == '"') {
      needs_quotes = true;
      break;
    }
  }

  if (!needs_quotes) {
    sp_str_builder_append(builder, arg);
    return;
  }

  sp_str_builder_append_c8(builder, '"');

  u32 backslashes = 0;
  sp_for(i, arg.len) {
    c8 c = arg.data[i];
    if (c == '\\') {
      backslashes++;
      continue;
    }

    if (c == '"') {
      sp_for(j, backslashes * 2 + 1) {
        sp_str_builder_append_c8(builder, '\\');
      }
      sp_str_builder_append_c8(builder, '"');
      backslashes = 0;
      continue;
    }

    sp_for(j, backslashes) {
      sp_str_builder_append_c8(builder, '\\');
    }
    backslashes = 0;
    sp_str_builder_append_c8(builder, c);
  }

  sp_for(i, backslashes * 2) {
    sp_str_builder_append_c8(builder, '\\');
  }

  sp_str_builder_append_c8(builder, '"');
}

c8* sp_ps_build_windows_cmdline(sp_ps_config_t* config) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_ps_win32_append_quoted_arg(&builder, config->command);

  sp_carr_for(config->args, it) {
    sp_str_t arg = config->args[it];
    if (sp_str_empty(arg)) {
      break;
    }

    sp_str_builder_append_c8(&builder, ' ');
    sp_ps_win32_append_quoted_arg(&builder, arg);
  }

  sp_da_for(config->dyn_args, it) {
    sp_str_builder_append_c8(&builder, ' ');
    sp_ps_win32_append_quoted_arg(&builder, config->dyn_args[it]);
  }

  sp_str_builder_append_c8(&builder, '\0');
  sp_mem_buffer_t buffer = sp_str_builder_into_buffer(&builder);
  return (c8*)buffer.data;
}

sp_win32_handle_t sp_ps_win32_open_null(sp_win32_dword_t access) {
  SECURITY_ATTRIBUTES attrs = SP_ZERO_INITIALIZE();
  attrs.nLength = sizeof(attrs);
  attrs.bInheritHandle = true;

  return CreateFileA("NUL", access, FILE_SHARE_READ | FILE_SHARE_WRITE, &attrs, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, SP_NULLPTR);
}

sp_win32_handle_t sp_ps_win32_fd_to_handle(sp_os_file_handle_t fd) {
  if (!sp_ps_is_fd_valid(fd)) {
    return SP_NULLPTR;
  }
  return (sp_win32_handle_t)fd;
}

typedef struct {
  sp_win32_handle_t child;
  sp_os_file_handle_t parent_fd;
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
      entry->parent_fd = (sp_os_file_handle_t)parent_write;
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
      entry->parent_fd = (sp_os_file_handle_t)parent_read;
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
  sp_os_file_handle_t fds[3] = { io->in.parent_fd, io->out.parent_fd, io->err.parent_fd };
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
  sp_env_init(&dst.env.env);
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



  sp_str_builder_t b = SP_ZERO_INITIALIZE();

  switch (config.env.mode) {
    case SP_PS_ENV_INHERIT: {
      for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
        sp_str_builder_append(&b, it.key);
        sp_str_builder_append_c8(&b, '=');
        sp_str_builder_append(&b, it.value);
        sp_str_builder_append_c8(&b, '\0');
      }
      break;
    }
    case SP_PS_ENV_EXISTING: {
      sp_env_t* env = &config.env.env;
      sp_str_ht_for(env->vars, it) {
        sp_str_t key = *sp_str_ht_it_getkp(env->vars, it);
        sp_str_t val = *sp_str_ht_it_getp(env->vars, it);

        sp_str_builder_append_fmt(&b, "{}={}", sp_fmt_str(key), sp_fmt_str(val));
        sp_str_builder_append_c8(&b, '\0');
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

    sp_str_builder_append_fmt(&b, "{}={}", sp_fmt_str(e.key), sp_fmt_str(e.value));
    sp_str_builder_append_c8(&b, '\0');
  }

  sp_str_builder_append_c8(&b, '\0');
  sp_str_builder_append_c8(&b, '\0');

  sp_mem_buffer_t buffer = sp_str_builder_into_buffer(&b);
  c8* env = sp_mem_buffer_as_cstr(&buffer);

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
    sp_err_set(SP_ERR_OS);
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
  sp_err_set(error);
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
    sp_err_set(SP_ERR_OS);
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
      sp_err_set(SP_ERR_OS);
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
    sp_err_set(SP_ERR_OS);
    result.state = SP_PS_STATE_DONE;
    result.exit_code = -1;
    return result;
  }

  return sp_ps_win32_finish_process(ps);
}

u64 sp_ps_win32_read_available(sp_os_file_handle_t fd, sp_str_builder_t* builder, bool* open) {
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

    sp_str_builder_append(builder, sp_str((c8*)buffer, (u32)num_read));
    total += (u64)num_read;
  }
}

sp_ps_output_t sp_ps_output(sp_ps_t* ps) {
  sp_ps_output_t result = SP_ZERO_INITIALIZE();

  bool out_open = sp_ps_is_fd_valid(ps->io.out.fd);
  bool err_open = sp_ps_is_fd_valid(ps->io.err.fd);

  sp_str_builder_t out = SP_ZERO_INITIALIZE();
  sp_str_builder_t err = SP_ZERO_INITIALIZE();

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
          sp_err_set(SP_ERR_OS);
        }
      } else if (wait == WAIT_FAILED) {
        process_done = true;
        exit_code = (DWORD)-1;
        sp_err_set(SP_ERR_OS);
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

  result.out = sp_str_builder_as_str(&out);
  result.err = sp_str_builder_as_str(&err);
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
  SP_BROKEN();
  return *src;
}

void sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg) {
  SP_BROKEN();
  if (!sp_str_empty(arg)) {
    sp_da_push(config->dyn_args, arg);
  }
}

sp_ps_t sp_ps_create(sp_ps_config_t config) {
  SP_BROKEN();
  SP_UNUSED(config);
  return SP_ZERO_STRUCT(sp_ps_t);
}

sp_ps_output_t sp_ps_run(sp_ps_config_t config) {
  SP_BROKEN();
  sp_ps_t ps = sp_ps_create(config);
  return sp_ps_output(&ps);
}

sp_io_writer_t* sp_ps_io_in(sp_ps_t* ps) {
  SP_BROKEN();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_io_reader_t* sp_ps_io_out(sp_ps_t* ps) {
  SP_BROKEN();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_io_reader_t* sp_ps_io_err(sp_ps_t* ps) {
  SP_BROKEN();
  SP_UNUSED(ps);
  return SP_NULLPTR;
}

sp_ps_status_t sp_ps_poll(sp_ps_t* ps, u32 timeout_ms) {
  SP_BROKEN();
  SP_UNUSED(ps);
  SP_UNUSED(timeout_ms);
  return SP_RVAL(sp_ps_status_t) {
    .state = SP_PS_STATE_DONE,
    .exit_code = -1,
  };
}

sp_ps_status_t sp_ps_wait(sp_ps_t* ps) {
  SP_BROKEN();
  SP_UNUSED(ps);
  return SP_RVAL(sp_ps_status_t) {
    .state = SP_PS_STATE_DONE,
    .exit_code = -1,
  };
}

sp_ps_output_t sp_ps_output(sp_ps_t* ps) {
  SP_BROKEN();
  return SP_RVAL(sp_ps_output_t) {
    .status = sp_ps_wait(ps),
  };
}

bool sp_ps_kill(sp_ps_t* ps) {
  SP_BROKEN();
  SP_UNUSED(ps);
  return false;
}
#endif


// ███████╗███╗   ███╗ ██████╗ ███╗   ██╗
// ██╔════╝████╗ ████║██╔═══██╗████╗  ██║
// █████╗  ██╔████╔██║██║   ██║██╔██╗ ██║
// ██╔══╝  ██║╚██╔╝██║██║   ██║██║╚██╗██║
// ██║     ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║
// ╚═╝     ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
// @fmon
SP_PRIVATE void sp_fmon_os_init(sp_fmon_t* monitor);
SP_PRIVATE void sp_fmon_os_deinit(sp_fmon_t* monitor);
SP_PRIVATE void sp_fmon_os_add_dir(sp_fmon_t* monitor, sp_str_t path);
SP_PRIVATE void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path);
SP_PRIVATE void sp_fmon_os_process_changes(sp_fmon_t* monitor);

void sp_fmon_init(sp_fmon_t* monitor, sp_fmon_fn_t callback, sp_fmon_event_kind_t events, void* userdata) {
  sp_fmon_init_ex(monitor, callback, events, userdata, 0, sp_context_get()->allocator);
}

void sp_fmon_init_ex(sp_fmon_t* monitor, sp_fmon_fn_t callback, sp_fmon_event_kind_t events, void* userdata, u32 debounce, sp_allocator_t alloc) {
  monitor->allocator = alloc;
  monitor->callback = callback;
  monitor->events_to_watch = events;
  monitor->userdata = userdata;
  monitor->debounce_time_ms = debounce;
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
  sp_str_ht(u8) files;
  u8 buffer[4096] __attribute__((aligned(__alignof__(sp_inotify_event_t))));
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

  c8* directory_cstr = sp_str_to_cstr(path);
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

  sp_fmon_dir_t dir = SP_ZERO_INITIALIZE();
  dir.overlapped.hEvent = event;
  dir.handle = handle;
  dir.path = sp_fs_canonicalize_path(path);
  dir.notify_information = sp_alloc(SP_FILE_MONITOR_BUFFER_SIZE);
  sp_mem_zero(dir.notify_information, SP_FILE_MONITOR_BUFFER_SIZE);

  sp_dyn_array_push(os->dirs, dir);
  sp_fmon_dir_t* info = &os->dirs[sp_da_size(os->dirs) - 1];
  sp_win32_fmon_issue_read(monitor, info);
}

void sp_fmon_os_add_file(sp_fmon_t* monitor, sp_str_t file_path) {
  sp_fmon_os_t* os = monitor->os;
  sp_str_t canonical = sp_fs_canonicalize_path(file_path);
  sp_dyn_array_push(os->watch_files, canonical);

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
  sp_dyn_array_push(monitor->changes, change);
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

  linux_monitor->fd = sp_inotify_init1(SP_IN_NONBLOCK | SP_IN_CLOEXEC);
  if (linux_monitor->fd == -1) {
    // Handle error but don't crash
    linux_monitor->fd = 0;
  }

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

  s32 wd = sp_inotify_add_watch(os->fd, path_cstr, mask);

  if (wd != -1) {
    sp_dyn_array_push(os->fds, wd);
    sp_dyn_array_push(os->paths, sp_str_copy(path));
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
  sp_str_t canonical = sp_fs_canonicalize_path(file_path);
  sp_str_ht_insert(os->files, canonical, 1);

  sp_str_t dir_path = sp_fs_parent_path(canonical);
  if (!sp_str_empty(dir_path)) {
    sp_fmon_os_add_dir(monitor, dir_path);
  }
}

SP_PRIVATE bool sp_linux_fmon_file_matches(sp_fmon_os_t* os, sp_str_t full_path) {
  if (!os->files) return true;
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
    sp_inotify_event_t* event = (sp_inotify_event_t*)ptr;

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
          file_path = sp_fs_join_path(dir_path, file_name);
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

    ptr += sizeof(sp_inotify_event_t) + event->len;
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
  sp_context_get()->err.sp = err;
  sp_context_get()->err.os = errno;
}

void sp_err_clear() {
  sp_context_get()->err.sp = SP_OK;
  sp_context_get()->err.os = 0;
}

sp_err_t sp_err_get() {
  return sp_context_get()->err.sp;
}

sp_os_err_t sp_err_get_os() {
  return sp_context_get()->err.os;
}

sp_err_ext_t sp_err_get_ext() {
  return sp_context_get()->err;
}


// ██╗ ██████╗
// ██║██╔═══██╗
// ██║██║   ██║
// ██║██║   ██║
// ██║╚██████╔╝
// ╚═╝ ╚═════╝
// @io
sp_err_t sp_io_read_file(sp_str_t path, sp_str_t* content) {
  sp_assert(content);

  sp_err_t err = SP_OK;
  c8* buffer = SP_NULLPTR;
  u64 size = 0;

  sp_io_reader_t reader = SP_ZERO_INITIALIZE();
  sp_try(sp_io_reader_from_file(&reader, path));

  sp_try_goto(sp_io_reader_size(&reader, &size), err, cleanup);
  if (!size) {
    goto cleanup;
  }

  buffer = sp_alloc_n(c8, size);
  u64 bytes_read = 0;
  sp_try_goto(sp_io_read(&reader, buffer, size, &bytes_read), err, cleanup);
  content->data = buffer;
  content->len = (u32)bytes_read;
  buffer = SP_NULLPTR;

cleanup:
  if (buffer) sp_free(buffer);
  sp_io_reader_close(&reader);
  return err;
}

sp_err_t sp_io_reader_file_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  sp_err_t result = SP_OK;
  u64 num_bytes = 0;

  while (true) {
    s64 rc = sp_sys_read(reader->file.fd, ptr, size);
    if (rc >= 0) {
      num_bytes = (u64)rc;
      goto done;
    }

    if (errno == SP_EINTR) {
      continue;
    }

    result = SP_ERR_IO_READ_FAILED;
    goto done;
  }

done:
  if (bytes_read) *bytes_read = num_bytes;
  return result;
}

sp_err_t sp_io_reader_file_seek(sp_io_reader_t* r, s64 offset, sp_io_whence_t whence, s64* position) {
  s32 posix_whence = SP_SEEK_SET;
  switch (whence) {
    case SP_IO_SEEK_SET: {
      posix_whence = SP_SEEK_SET;
      break;
    }
    case SP_IO_SEEK_CUR: {
      posix_whence = SP_SEEK_CUR;
      break;
    }
    case SP_IO_SEEK_END: {
      posix_whence = SP_SEEK_END;
      break;
    }
  }

  s64 pos = sp_sys_lseek(r->file.fd, offset, posix_whence);
  if (pos < 0) {
    if (position) *position = -1;
    return SP_ERR_IO_SEEK_FAILED;
  }
  if (position) *position = pos;
  return SP_OK;
}

sp_err_t sp_io_reader_file_size(sp_io_reader_t* r, u64* size) {
  s64 current = sp_sys_lseek(r->file.fd, 0, SP_SEEK_CUR);
  if (current < 0) {
    if (size) *size = 0;
    return SP_ERR_IO;
  }

  s64 end = sp_sys_lseek(r->file.fd, 0, SP_SEEK_END);
  if (end < 0) {
    if (size) *size = 0;
    return SP_ERR_IO_SEEK_FAILED;
  }

  sp_sys_lseek(r->file.fd, current, SP_SEEK_SET);
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

void sp_io_reader_from_fd(sp_io_reader_t* reader, sp_os_file_handle_t fd, sp_io_close_mode_t mode) {
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

  u8* buffer = (u8*)ptr;
  u64 total = 0;

  u64 buffered = reader->buffer.len - reader->seek;
  u64 n = SP_MIN(size, buffered);
  sp_mem_copy(reader->buffer.data + reader->seek, buffer, n);

  reader->seek += n;
  buffer += n;
  total += n;

  if (total == size) {
    if (bytes_read) *bytes_read = size;
    return SP_OK;
  }

  u64 remaining = size - total;
  if (remaining >= reader->buffer.capacity) {
    u64 direct_read = 0;
    sp_err_t err = reader->vtable.read(reader, buffer, remaining, &direct_read);
    if (bytes_read) *bytes_read = total + direct_read;
    return err;
  }

  reader->seek = 0;
  u64 fill_read = 0;
  sp_err_t err = reader->vtable.read(reader, reader->buffer.data, reader->buffer.capacity, &fill_read);
  reader->buffer.len = fill_read;
  n = SP_MIN(remaining, reader->buffer.len);
  sp_mem_copy(reader->buffer.data, buffer, n);

  reader->seek = n;
  if (bytes_read) *bytes_read = total + n;
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
  sp_err_t result = SP_OK;
  u64 num_bytes = 0;

  while (true) {
    s64 rc = sp_sys_write(writer->file.fd, ptr, size);
    if (rc >= 0) {
      num_bytes = (u64)rc;
      goto done;
    }

    if (errno == SP_EINTR) {
      continue;
    }

    result = SP_ERR_IO_WRITE_FAILED;
    goto done;
  }

done:
  if (bytes_written) *bytes_written = num_bytes;
  return result;
}

sp_err_t sp_io_writer_file_seek(sp_io_writer_t* writer, s64 offset, sp_io_whence_t whence, s64* position) {
  int posix_whence = SP_SEEK_SET;
  switch (whence) {
    case SP_IO_SEEK_SET: posix_whence = SP_SEEK_SET; break;
    case SP_IO_SEEK_CUR: posix_whence = SP_SEEK_CUR; break;
    case SP_IO_SEEK_END: posix_whence = SP_SEEK_END; break;
  }

  s64 p = 0;
  position = position ? position : &p;

  *position = sp_sys_lseek(writer->file.fd, offset, posix_whence);
  if (*position < 0) {
    return SP_ERR_IO_SEEK_FAILED;
  }
  return SP_OK;
}

sp_err_t sp_io_writer_file_size(sp_io_writer_t* writer, u64* size) {
  u64 s = 0;
  size = size ? size : &s;

  s64 current = sp_sys_lseek(writer->file.fd, 0, SP_SEEK_CUR);
  if (current < 0) {
    return SP_ERR_IO;
  }

  *size = sp_sys_lseek(writer->file.fd, 0, SP_SEEK_END);
  if (*size < 0) {
    return SP_ERR_IO_SEEK_FAILED;
  }

  sp_sys_lseek(writer->file.fd, current, SP_SEEK_SET);
  return SP_OK;
}

sp_err_t sp_io_writer_file_close(sp_io_writer_t* writer) {
  if (writer->file.close_mode != SP_IO_CLOSE_MODE_AUTO) return SP_OK;

  if (sp_sys_close(writer->file.fd) < 0) {
    return SP_ERR_IO_CLOSE_FAILED;
  }
  return SP_OK;
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

  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_sys_fd_t fd = sp_sys_open(sp_str_to_cstr(path), flags, 0644);
  sp_mem_end_scratch(scratch);

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

void sp_io_writer_from_fd(sp_io_writer_t* writer, sp_os_file_handle_t fd, sp_io_close_mode_t close_mode) {
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

void sp_io_writer_from_dyn_mem(sp_io_writer_t* writer) {
  sp_io_writer_from_dyn_mem_ex(writer, SP_NULLPTR, 0, sp_context_get()->allocator);
}

void sp_io_writer_from_dyn_mem_ex(sp_io_writer_t* writer, u8* buffer, u64 size, sp_allocator_t allocator) {
  *writer = (sp_io_writer_t) {
    .vtable = {
      .write = sp_io_writer_dyn_write,
      .seek = sp_io_writer_dyn_seek,
      .size = sp_io_writer_dyn_size,
      .close = sp_io_writer_dyn_close,
    },
    .dyn_mem = {
      .allocator = allocator,
      .buffer = {
        .data = buffer,
        .len = 0,
        .capacity = size
      }
    }
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

SP_API sp_app_t* sp_app_new(sp_app_config_t config) {
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
    sp->result = sp->on_init(sp);
    if (sp->result != SP_APP_CONTINUE) {
      goto deinit;
    }
  }

  sp->frame.timer = sp_tm_start_timer();
  while (true) {
    if (sp->on_poll) {
      sp->result = sp->on_poll(sp);
      if (sp->result != SP_APP_CONTINUE) {
        goto deinit;
      }
    }

    sp->frame.accumulated += sp_tm_lap_timer(&sp->frame.timer);
    if (sp->frame.accumulated >= sp->frame.target) {
      sp->frame.accumulated -= sp->frame.target;
      sp->frame.num++;
      sp->result = sp->on_update(sp);
      if (sp->result != SP_APP_CONTINUE) {
        goto deinit;
      }
    }
    else {
      sp_sleep_ns(sp->frame.target - sp->frame.accumulated);
    }
  }

deinit:
  if (sp->on_deinit) {
    sp->on_deinit(sp);
  }

  switch (sp->result) {
    case SP_APP_ERR: return sp->rc ? sp->rc : 1;
    case SP_APP_QUIT: return sp->rc;
    case SP_APP_CONTINUE: { sp_unreachable_case(); }
  }

  sp_unreachable_return(1);
}

#if defined(SP_MAIN)

s32 _sp_main(s32 num_args, const c8** args) {
  return sp_app_run(sp_main(num_args, args));
}
SP_ENTRY(_sp_main)
#endif

SP_END_EXTERN_C()

#ifdef SP_CPP
sp_str_t operator/(const sp_str_t& a, const sp_str_t& b) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append(&builder, b);
  sp_str_t result = sp_str_builder_to_str(&builder);
  return sp_fs_normalize_path(result);
}

sp_str_t operator/(const sp_str_t& a, const c8* b) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, a);
  sp_str_builder_append_c8(&builder, '/');
  sp_str_builder_append_cstr(&builder, b);
  sp_str_t result = sp_str_builder_to_str(&builder);
  return sp_fs_normalize_path(result);
}
#endif

#endif // SP_SP_C
#endif // SP_IMPLEMENTATION
