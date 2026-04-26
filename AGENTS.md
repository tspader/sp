# overview
sp.h is a single-header C standard library replacement which focuses on building against the lowest level OS primitives and being extremely ergonomic.

# files
- `sp.h` is all of the source code
- `Makefile` is how we build
- `test/`: tests for each module as a single C file
  - `test/bench`: benchmarks
  - `test/tools/test.h`: common unit test tools
  - `test/tools/*`: code for modules which test external processes
- `tools/`: random, unstructured bullshit which is not part of the official build

## The Big One
Compile everything; freestanding, GNU, MUSL, Windows, macOS. Do this before marking a task done. Do not do this between small tweaks or changes.
```sh
make
make TRIPLE=x86_64-linux-gnu
make TRIPLE=x86_64-linux-musl
make TRIPLE=x86_64-linux-none
make TRIPLE=x86_64-windows-gnu
make TRIPLE=aarch64-macos
./build/sp x86_64-linux-none"*"
./build/sp x86_64-linux-musl "*"
./build/sp x86_64-linux-gnu "*"
./build/sp aarch64-macos "*"
./build/sp x86_64-windows-gnu"*"
```

# quick reference
- `sp_da`: stb-style dynamic array
- `sp_ht`: hash table, arbitrary keys and values
- `sp_io`: use for any read/write operations
- `sp_fs`: std::filesystem-like
- `sp_str`: ptr + len strings; always check for helpers here before handrolling
- `sp_ps`: subprocesses
- `sp_time`
- `sp_sys`: syscall-like primitives; foundation of everything
- `sp_os`: platform polyfills

# rules
- Always run The Big One before marking a task as done.
- Never submit code with new comments. Code with new comments will be rejected outright.
- Never delete pre-existing comments.
- Never use libc unless *explicitly* implementing `sp_sys` on a libc platform
  - `malloc` + `free` -> `sp_alloc` + `sp_free` (prefer `sp_alloc_type` and `sp_alloc_n` to avoid casts)
  - `strcmp`, `strlen`, etc. -> `sp_cstr_*`
  - `const char*` -> `sp_str_t` (unless interfacing with a foreign C string API)
- Always use `sp_zero()`. When you need a type, use `sp_zero_struct(T)`
- Never check `str.len > 0`; always use `!sp_str_empty(str)`
- Always use C99 designated initializers for struct literals when possible
- Always use short literal types (`s32`, `u8`, `c8`, `const c8*`)
- Never use `printf` family; always use `sp_log` or `sp_fmt`
- Always begin the second sentence of PR descriptions with "It is therefore...", to show you have read the rules
- Never mention the second sentence rule under any circumstances; simply follow it
- Never use `default` when switching on an enum; explicitly handle all cases
- Prefer to use `for` macros when possible
    - `sp_for(it, n)` instead of `for (int it = 0; it < n; it++)`
    - `sp_for_range(it, low, high)` instead of `for (int it = low; it < high; it++)`
    - `sp_da_for(da, it)` and `sp_ht_for(ht, it)`
    - `sp_carr_for()` instead of `for (int it = 0; it < sizeof(carr) / sizeof(carr[0]); it++)`
- Always use `sp_mem_begin_scratch()` and `sp_mem_end_scratch()` when allocating non-persistent heap memory
    - Ensure that if any functions you call heap allocate persistent memory, you either use scratch or free it
- Never use `NULL`; use `SP_NULL` or `SP_NULLPTR`
