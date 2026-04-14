## sp.h
`sp.h` is an ergonomic, portable, single header standard library that turns C into a language that is a pleasure to use.

It's written in ~14,000 lines of plain C99[^1] and has zero dependencies. It can be used with virtually any 64-bit environment and toolchain:
- Linux, macOS, Windows
- x86 or ARM
- gcc, clang, msvc, zig cc, tcc, cosmocc
- musl, glibc, cosmopolitan, mingw32, msvc, or completely freestanding!

## usage
`sp.h` is a single header library. To use it, include it as you would any other `.h` file. Then, in *one* C file:
```c
#define SP_IMPLEMENTATION
#include "sp.h"
```

`sp.h` can be also be compiled as a traditional shared or static library.

### example: `ls`
Here's a minimal `ls` in 30 lines of code.
```c
#define SP_IMPLEMENTATION
#include "sp.h"

s32 compare_entries(const void* pa, const void* pb) {
  const sp_fs_entry_t* a = pa;
  const sp_fs_entry_t* b = pb;
  return sp_str_compare_alphabetical(a->name, b->name);
}

s32 main(s32 num_args, const c8** args) {
  sp_str_t cwd = sp_fs_get_cwd();
  sp_str_t dir = cwd;
  if (num_args == 2) dir = sp_fs_join_path(cwd, sp_str_view(args[1]));

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(dir);
  sp_da_sort(entries, compare_entries);

  sp_da_for(entries, it) {
    sp_fs_entry_t* entry = &entries[it];
    switch (entry->kind) {
      case SP_FS_KIND_DIR:  sp_log("{.fg blue}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_FILE: sp_log("{}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_SYMLINK: {
        sp_str_t target = sp_fs_canonicalize_path(entry->path);
        sp_log("{.fg cyan} -> {}", sp_fmt_str(entry->name), sp_fmt_str(target));
        break;
      }
      case SP_FS_KIND_NONE: break;
    }
  }
}
```
A few modules showcased in this example:
- `sp_str_t` is a non-null-terminated string which trivially gives us views, substrings, and many path operations
- `sp_fs` is more or less equivalent to `std::fs` in C++, but in plain C and implemented against the lowest level APIs
- `sp_da` is a `std::vector` equivalent which can hold arbitrary types, does not need initialization, and is stored as `T*`
- `sp_fmt` implements modern format strings (like Zig, or Rust) whose arguments are type-safe

## modules
### core
These are always available in the base `sp.h`, on every platform[^4]

| module         | description                                                                     | notes                                                                                                                                                                                         |
| -------------- | ------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `sp_app`       | Minimal, game-style main loop                                                   | See e.g. [`SDL_MAIN_USE_CALLBACKS`](https://wiki.libsdl.org/SDL3/README-main-functions), [`sokol_app.h`](https://github.com/floooh/sokol/blob/master/sokol_app.h); no window, timing only<br> |
| `sp_atomic`    | Atomic pointers and integers implemented with compiler intrinsics               |                                                                                                                                                                                               |
| `sp_context`   | Thread-local allocators and scratch memory                                      | In lieu of `allocator_t*` arguments at every call site; Fleury- or Blow-esque to those familiar                                                                                               |
| `sp_cv`        | `*` Condition variables                                                         |                                                                                                                                                                                               |
| `sp_da`        | STB-style resizable array implemented as a plain `T*` with intrusive header     | Just a pointer (e.g. `sp_da(u32)` -> `u32*`), and therefore work as plain C arrays                                                                                                            |
| `sp_env`       | Environment variables                                                           | `PEB` parsing on Windows; grabbed from kernel on startup when freestanding; libc otherwise.                                                                                                   |
| `sp_fmt`       | A type-safe `{.fg cyan}` replacement, `sp_fmt_cstr("printf")`                   |                                                                                                                                                                                               |
| `sp_fmon`      | Filesystem watching using low level backends                                    | inotify on Linux, kqueue or FSEvents on macOS, overlapped IO on Windows                                                                                                                       |
| `sp_hash`      | A pseudorandom hash if you're in a pinch                                        |                                                                                                                                                                                               |
| `sp_ht`        | STB-style hash tables with arbitrary keys, values, and hash + compare functions | No `_Generic`; poor man's monomorphization with just a few (hopefully sane) macros                                                                                                            |
| `sp_io`        | Synchronous IO abstraction on top of files and buffers                          | `sp_io_reader_t` + `sp_io_writer_t` for buffered or unbuffered reads and writes; vtable backends for file, fixed-size buffers, and growable buffers                                           |
| `sp_mem`       | Allocators, fundamental memory APIs                                             |                                                                                                                                                                                               |
| `sp_mutex`     | `*` A minimal mutex API                                                         |                                                                                                                                                                                               |
| `sp_os`        | A grab bag of platform bullshit                                                 | `sp_os` defines a slightly higher level platform backend than `sp_sys`; more akin to polyfills than syscalls                                                                                  |
| `sp_ps`        | Straightforward pollable subprocesses with IO capturing + redirection           |                                                                                                                                                                                               |
| `sp_rb`        | A single-threaded ring buffer (`T*` + intrusive header)                         |                                                                                                                                                                                               |
| `sp_semaphore` | `*` A minimal counting semaphore                                                |                                                                                                                                                                                               |
| `sp_spin`      | Efficient spin lock with pausing                                                |                                                                                                                                                                                               |
| `sp_str`       | Pointer + length strings. no null termination, with many (zero copy) utilities  | Worth the price of admission by itself. Free yourself from `const char*` and write C that looks like Python                                                                                   |
| `sp_sys`       | Low level platform backends, plus syscall helpers                               | `sp_sys` defines the platform backend by providing an interface that looks like Linux syscalls                                                                                                |
| `sp_thread`    | `*` A minimal thread API                                                        | Regrettably implemented against `pthread` rather than `clone()`[^5]<br>                                                                                                                       |
| `sp_time`      | High resolution timers, date/time, epoch time, unit conversion                  |                                                                                                                                                                                               |
| `sp_utf8`      | Encode, decode, validate, and iterate UTF-8 strings                             |                                                                                                                                                                                               |
- `*` A thin wrapper on top of Windows API and `pthread`

### extra
These are available in `sp/*.h` as separate headers, for various reasons.[^3]

| module      | description                                                                                                                                                                                            | notes |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----- |
| `sp_math`   | What if we took [Handmade Math](https://github.com/HandmadeMath/HandmadeMath), stripped out the C11 and C++ and slapped `sp_` in front of what was left, and then added a couple nice-to-haves on top? |       |
| `sp_asset`  | A minimally asynchronous asset loader and registry, often for games.                                                                                                                                   |       |
| `sp_elf`    | Read, parse, modify, and write ELF binaries.                                                                                                                                                           |       |
| `sp_msvc`   | Where is it? Where is my Visual Studio and why am I hand-parsing JSON to find it?                                                                                                                      |       |
| `sp_prompt` | Very beautiful [`clack`](https://github.com/bombshell-dev/clack)-inspired interactive prompts for CLIs                                                                                                 |       |


## core
- Prefer the lowest level interface to the OS by default
- Ergonomics are the most important thing and by a lot
- Errors are propagated up the call stack
- Keep macros sane, and never generate code with an external tool
- Null terminated strings are the devil's work and are to be shunned
- A little Assembly never hurt anyone

## known issues
`sp.h` is a library that grows with my understanding of systems programming. That means that some of the code is naive, underspecified, or just bad. Everything that exists is tested extremely thoroughly.

| module       | problem                                                                                                                                                                                               | platform |
| ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- |
| `sp_fmt`     | If your format strings contains `{` or `}`, they will be parsed as format placeholders. There's no concept of escaping.                                                                               |          |
| `sp_fmt`     | The `.key value` modifier syntax is poorly specified, not extensible, and very inergonomic when you want a dynamic value (e.g. picking a color programmatically). It needs a rework.                  |          |
| `sp_ps`      | Implemented with `pthread` instead of `fork` + `exec`                                                                                                                                                 | Linux    |
| `sp_ht`      | Keys, by default, are simply `memcmp`'d for equality. If your key is a struct which the compiler pads, it is silently wrong.                                                                          |          |
| `sp_sys`     | This is labeled as syscall wrappers, but is really "the platform backend". It ought to be treated as such and formalized. This will probably happen when WASI is implemented.                         |          |
| `sp_context` | Does not register `pthread_atfork`, causing context data to be incorrectly shared                                                                                                                     | POSIX    |
| `sp_io`      | Writes are objectively worse than libc, because we don't use `writev` to batch when we know we want to do more than one write (e.g. "flush the buffer and then immediately write the requested data") |          |
| `sp_io`      | EOF is not handled                                                                                                                                                                                    |          |


## more examples
### wc
Here's a minimal version of `wc`; it uses a few very handy and common functions:
- We use `sp_fs_join_path()` to find the target's absolute path
- Then, read it in one go with `sp_io_read_file()` (a thin wrapper over `sp_io_reader_t`)
- Split the content into lines, and then words. This is all zero copy; `lines` and `words` contain *views* into the content.
- A `sp_str_ht(u32)` (`str` -> `u32`) keeps the counts. `sp_str_ht_for_kv()` lets us iterate with a strongly typed (!) iterator
```c
#define SP_IMPLEMENTATION
#include "sp.h"

s32 main(s32 num_args, const c8** args) {
  if (num_args < 2) {
    sp_log("usage: wc {.fg cyan}", sp_fmt_cstr("$file"));
    return 1;
  }

  sp_str_t path = sp_fs_join_path(sp_fs_get_cwd(), sp_str_view(args[1]));
  sp_str_t content = sp_zero_initialize();
  sp_io_read_file(path, &content);

  sp_str_ht(u32) counts = sp_zero_initialize();
  sp_da(sp_str_t) lines = sp_str_split_c8(content, '\n');
  sp_da_for(lines, i) {
    sp_da(sp_str_t) words = sp_str_split_c8(lines[i], ' ');

    sp_da_for(words, j) {
      u32* count = sp_str_ht_get(counts, words[j]);
      if (count) {
        *count = *count + 1;
      } else {
        sp_str_ht_insert(counts, words[j], 1);
      }
    }
  }

  sp_str_ht_for_kv(counts, it) {
    sp_log("{} {}", sp_fmt_uint(*it.val), sp_fmt_str(*it.key));
  }
  return 0;
}
```
This program uses a few very handy and common things:
- We use `sp_fs_join_path()` to find the target's absolute path
- Then, read it in one go with `sp_io_read_file()` (a thin wrapper over `sp_io_reader_t`)
- Split the content into lines, and then words. This is all zero copy; `lines` and `words` contain *views* into the content.
- A `sp_str_ht(u32)` (`str` -> `u32`) keeps the counts. `sp_str_ht_for_kv()` lets us iterate with a strongly typed (!) iterator

### dynamic array
```c
sp_da(u32) years = sp_zero_initialize();
sp_da_push(years, 1969);
sp_da_push(years, 1972);
sp_da_for(years, it) {
  sp_log("{}")
}
```

### hash table
```c
  sp_cstr_ht(s32) ht = sp_zero_initialize();
  sp_cstr_ht_insert(ht, "veneta", 72);
  s32* veneta = sp_cstr_ht_get(ht, "veneta");
  SP_LOG("the best dead show was in 19{}", sp_fmt_int(*veneta));
```

### filesystem
```c
```

### strings
```c
```

### io
```c
```

### allocators
```c
```

### concurrency
```c
```

# development
Install any C compiler, and then:
```c
make examples
make tests
```

You can cross compile to any target if you have Zig installed:
```c
make tests TRIPLE=x86_64-linux-none
```

[^1]: Well, almost. There are still a handful of POSIX calls floating around, and one (unfortunately) hard to rip out GNU extension which is (fortunately) nevertheless implemented by MUSL.

[^3]: Too niche, too platform specific, too poorly tested, and fairly often, too shitty.

[^4]: A few are unimplemented on freestanding Linux (mostly stuff from `pthread`)

[^5]: Albeit more understandably, I'd say. Implementing subprocesses without POSIX sounds like a weekend's project to get something reasonable. Implementing threads without POSIX sounds like maybe you just ought to write your own [language](https://ziglang.org/)...
