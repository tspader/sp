# sp.h
`sp.h` is an ergonomic, portable, single header standard library that turns C into a modern language that is a pleasure to use. It provides:
- `sp_fmt`, a type-safe `{:^16 .cyan}` replacement, `sp_fmt_cstr("printf")`
- `sp_str_t`, a proper zero-copy string library and type
- `sp_fs`, a std::filesystem-like API
- `sp_da`, a dynamic array
- `sp_ht`, hash tables with arbitrary keys and values (including strings)
- `sp_ps`, subprocesses
- `sp_io`, synchronous, bufferable, zero-copy IO
- File monitors, [beautiful, interactive CLIs](https://spader.zone/prompt), ELF parsing, memory allocators, concurrency, UTF-8, screaming fast globbing, and a whole lot more!

It's written in ~15,000 lines of plain C99[^1] and has zero dependencies. It does not depend on libc. It can be used with virtually any environment and toolchain:
- Linux, macOS, Windows
- x86, ARM, or WASM
- gcc, clang, MSVC, mingw, zig cc, tcc, cosmocc
- musl, glibc, cosmopolitan, MSVCRT, UCRT, WASI, or completely freestanding!

# usage
`sp.h` is a single header library. Download the file and include it as you would any other `.h` file. Then, in *one* C file:
```c
#define SP_IMPLEMENTATION
#include "sp.h"
```

`sp.h` can be also be compiled as a traditional shared or static library. `sp.h` makes no assumptions about its place in your code. You can use any small piece of it as a standalone utility, or you can use it as the foundation for almost any program. Give it a try!

## example: `ls`
Here's a minimal `ls` in 30 lines of code.
```c
#define SP_IMPLEMENTATION
#include "sp.h"

s32 compare_entries(const void* pa, const void* pb) {
  const sp_fs_entry_t* a = (const sp_fs_entry_t*)pa;
  const sp_fs_entry_t* b = (const sp_fs_entry_t*)pb;
  return sp_str_compare_alphabetical(a->name, b->name);
}

s32 main(s32 num_args, const c8** args) {
  sp_mem_t mem = sp_mem_os_new();
  sp_str_t cwd = sp_fs_get_cwd(mem);
  sp_str_t dir = cwd;
  if (num_args == 2) dir = sp_fs_join_path(mem, cwd, sp_str_view(args[1]));

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(mem, dir);
  sp_da_sort(entries, compare_entries);

  sp_da_for(entries, it) {
    sp_fs_entry_t* entry = &entries[it];
    switch (entry->kind) {
      case SP_FS_KIND_DIR:  sp_log("{.blue}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_FILE: sp_log("{}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_SYMLINK: sp_log("{.cyan} -> {}", sp_fmt_str(entry->name), sp_fmt_str(entry->path)); break;
      case SP_FS_KIND_NONE: break;
    }
  }
  return 0;
}
```
A few modules showcased in this example:
- `sp_mem_t` is an allocator; everything that allocates takes one. In the example, we use the default heap allocator.
- `sp_str_t` is a non-null-terminated string which trivially gives us views, substrings, and many path operations
- `sp_fs` is more or less equivalent to `std::fs` in C++, but in plain C and implemented against the lowest level APIs
- `sp_da` is a `std::vector` equivalent which can hold arbitrary types and is stored as and usable as a plain `T*`
- `sp_fmt` implements modern format strings (like Zig, or Rust) whose arguments are type-safe

# modules
## core
These are always available in the base `sp.h`, on every platform[^4].

| module         | description                                                                     | notes                                                                                                                                                                                         |
| -------------- | ------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `sp_app`       | Minimal, game-style main loop                                                   | See e.g. [`SDL_MAIN_USE_CALLBACKS`](https://wiki.libsdl.org/SDL3/README-main-functions), [`sokol_app.h`](https://github.com/floooh/sokol/blob/master/sokol_app.h); no window, timing only |
| `sp_atomic`    | Atomic pointers and integers implemented with compiler intrinsics               |                                                                                                                                                                                               |
| `sp_context`   | Thread-local allocators and scratch memory                                      | In lieu of `allocator_t*` arguments at every call site; Fleury- or Blow-esque to those familiar                                                                                               |
| `sp_cv`        | `*` Condition variables                                                         |                                                                                                                                                                                               |
| `sp_da`        | STB-style resizable array implemented as a plain `T*` with intrusive header     | Just a pointer (e.g. `sp_da(u32)` -> `u32*`), and therefore work as plain C arrays                                                                                                            |
| `sp_env`       | Environment variables                                                           | `PEB` parsing on Windows; grabbed from kernel on startup when freestanding; libc otherwise.                                                                                                   |
| `sp_fmt`       | A type-safe `{.cyan}` replacement, `sp_fmt_cstr("printf")`                   |                                                                                                                                                                                               |
| `sp_fmon`      | Filesystem watching using low level backends                                    | inotify on Linux, kqueue or FSEvents on macOS, overlapped IO on Windows                                                                                                                       |
| `sp_fs`      | Synchronous file utilities; path manipulation                                    |                                                                                                                        |
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
| `sp_str`       | Pointer + length strings; not null terminated, many (zero copy) utilities  | Worth the price of admission by itself. Free yourself from `const char*` and write C that looks like Python                                                                                   |
| `sp_sys`       | Low level platform backends, plus syscall helpers                               | `sp_sys` defines the platform backend by providing an interface that looks like Linux syscalls                                                                                                |
| `sp_thread`    | `*` A minimal thread API                                                        | Regrettably implemented against `pthread` rather than `clone()`[^5]<br>                                                                                                                       |
| `sp_time`      | High resolution timers, date/time, epoch time, unit conversion                  |                                                                                                                                                                                               |
| `sp_utf8`      | Encode, decode, validate, and iterate UTF-8 strings                             |                                                                                                                                                                                               |
- `*` A thin wrapper on top of Windows API and `pthread`

## extra
These are available in `sp/*.h` as separate headers, for various reasons.[^3]

| module      | description                                                                                                                                                                                            | notes |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----- |
| `sp_math`   | What if we took [Handmade Math](https://github.com/HandmadeMath/HandmadeMath), stripped out the C11 and C++ and slapped `sp_` in front of what was left, and then added a couple nice-to-haves on top? |       |
| `sp_asset`  | A minimally asynchronous asset loader and registry, often for games.                                                                                                                                   |       |
| `sp_elf`    | Read, parse, modify, and write ELF binaries.                                                                                                                                                           |       |
| `sp_msvc`   | Where is it? Where is my Visual Studio and why am I hand-parsing JSON to find it?                                                                                                                      |       |
| `sp_prompt` | Very beautiful [`clack`](https://github.com/bombshell-dev/clack)-inspired interactive prompts for CLIs                                                                                                 |       |

# principles
I wrote about some of the core design [here](https://spader.zone/sp/). The short version:
- Prefer the lowest level interface to the OS by default
- Ergonomics are the most important thing and by a lot
- Errors are propagated up the call stack
- Keep macros sane, and never generate code with an external tool
- Null terminated strings are the devil's work and are to be shunned
- A little Assembly never hurt anyone

Please note that `sp.h` is in alpha. I [use](https://github.com/tspader/spn) [it](https://github.com/tspader/space), [a](https://github.com/tspader/mbench) [lot](https://github.com/tspader/tomlc17), and there are about a thousand tests, but...it's still in alpha. The core API shape is done, but there will likely be breakage around the edges. There are also several POSIX-isms which have clung around, and `sp_io` was recently finished.

Thankfully, since the library is a single file, it's very easy for you or an LLM to diff two copies of the library and make any changes needed. More, since the library is *not* build on decades of cruft, there are very few layers between your code and the syscalls it boils down to. Nevertheless, you should feel comfortable reading the library's source code if you plan to use it seriously.

# development
Install any C compiler, and then:
```bash
make
./build/test/amalg
```

You can cross compile to any target if you have Zig installed:
```bash
make x86_64-linux-none
```

[^1]: Well, almost. There are still a handful of POSIX calls floating around, and one (unfortunately) hard to rip out GNU extension which is (fortunately) nevertheless implemented by MUSL.

[^3]: Too niche, too platform specific, too poorly tested, and fairly often, too shitty.

[^4]: A few are unimplemented on freestanding Linux (mostly stuff from `pthread`)

[^5]: Albeit more understandably, I'd say. Implementing subprocesses without POSIX sounds like a weekend's project to get something reasonable. Implementing threads without POSIX sounds like maybe you just ought to write your own [language](https://ziglang.org/)...
