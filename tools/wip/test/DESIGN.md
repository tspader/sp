# sp_test — design

A replacement for utest.h, designed around how sp/spn tests are actually written:
declarative test cases run through executors. Informed by a survey of ~1700 test
cases across both repos and by Zig's std.testing/test_runner design.

## Goals

1. **No globals.** Every test receives an explicit context (`sp_test_t* t`), the same
   way sp.h passes `sp_mem_t` down the call stack. Helpers take `t` and nothing else —
   no `utest_result`, no `__FILE__/__LINE__` threading, no `&ur` vs `utest_result` split.
2. **Concurrency from the ground up.** All per-test state (failures, kv context, output,
   allocators) lives on the context. The runner can execute any subset of tests on any
   thread with no shared mutable state outside an atomic queue cursor and a report mutex.
3. **Tests are data.** The descriptor+executor pattern is the framework's native shape,
   not a convention layered on top. A case table registers each row as a first-class
   test: its own name, its own report line, its own filter target, its own thread.
4. **Correct control flow.** `assert` stops the *test* (not the enclosing helper), `skip`
   skips the *test* from any depth, and cleanup always runs. These are the three standing
   semantic bugs in utest.h.
5. **No registration magic.** No constructors, no linker sections, no macro-wrapped
   struct stuffing. You write arrays and hand them to `sp_test_main`. Explicit beats
   clever; sp.h is the benchmark for acceptable macro usage.

Non-goals (v1): crash isolation / out-of-process supervisor (Zig's best trick; the
registry shape is deliberately compatible with adding one later), fuzzing, golden-file
management (stays in user code), benchmark timing.

## Registry

```c
typedef struct sp_test_t sp_test_t;

typedef void (*sp_test_fn_t)(sp_test_t* t);
typedef void (*sp_test_each_fn_t)(sp_test_t* t, const void* it);

typedef struct {
  const c8* name;
  sp_test_fn_t fn;            // plain test: exactly one of fn/each is set
  sp_test_each_fn_t each;     // executor, called once per case
  const void* cases;
  u32 stride;
  u32 count;
  bool serial;                // never run concurrently with anything else
} sp_test_decl_t;

typedef struct {
  const c8* name;
  const sp_test_decl_t* tests;   // terminated by { .name = SP_NULLPTR }
} sp_test_suite_t;

SP_API s32 sp_test_main(s32 argc, const c8** argv, const sp_test_suite_t* suites);
                                 // suites terminated by { .name = SP_NULLPTR }
```

Two thin sugar macros, nothing else:

```c
#define sp_test(FN)             { .name = #FN, .fn = (FN) }
#define sp_test_each(FN, ARR)   { .name = #FN, .each = (FN), .cases = (ARR), \
                                  .stride = sizeof((ARR)[0]), .count = sp_carr_len(ARR) }
```

**Case naming convention:** the first field of a case struct is `const c8* name`.
The runner reads it at `(const c8**)case_ptr` to name the instance `suite.decl/name`.
If null, the instance is `suite.decl/3` (index). This matches existing descriptors
(`copy_test_t.label`, `sys_test_t.label`, `exec_test_t.name` are all first-field names).

Executors take `const void*` and cast on the first line. Handwritten, explicit:

```c
static void run_copy(sp_test_t* t, const void* arg) {
  const copy_test_t* c = arg;
  ...
}
```

A full test binary:

```c
static const copy_test_t copy_cases[] = { ... };

static const sp_test_decl_t fs_tests[] = {
  sp_test_each(run_copy, copy_cases),
  sp_test(fs_manual_weird_thing),
  {0},
};

static const sp_test_suite_t suites[] = {
  { "fs", fs_tests },
  {0},
};

s32 entry(s32 argc, const c8** argv) { return sp_test_main(argc, argv, suites); }
SP_MAIN(entry)
```

No fixtures. The survey showed 56 empty fixtures used purely as namespaces (suites do
that now) while real setup lives in executors already. Setup stays in the executor;
teardown uses `sp_test_defer` (below). Shared setup recipes (tracking allocator, arena,
sandbox) become context services instead of copy-pasted setup bodies.

## Context and assertions

```c
// checks: expect records and continues; assert records and stops the test
#define sp_expect(t, cond)
#define sp_expect_eq(t, a, b)          // also: ne, lt, le, gt, ge
#define sp_expect_near(t, a, b, eps)
#define sp_expect_str_eq(t, a, b)      // sp_str_t — the most reimplemented primitive (5 copies today)
#define sp_expect_str_eq_c(t, a, cstr)
#define sp_expect_ok(t, err)           // sp_err_t == SP_OK
#define sp_assert(t, cond)             // assert variants of all of the above
...

SP_API void sp_test_skip(sp_test_t* t, const c8* fmt, ...);   // stops the test, from any depth
SP_API void sp_test_fail(sp_test_t* t, sp_test_failure_t f);  // record a custom failure, continue
SP_API bool sp_test_alive(sp_test_t* t);                      // false once failed-fatally/skipped

// sticky kv context: attached to every subsequent failure, overwrite by key, not
// consumed on success (fixes the re-stage-before-every-assert problem in dag/exec.c)
SP_API void sp_test_kv(sp_test_t* t, const c8* key, sp_str_t value);
SP_API void sp_test_kv_c(sp_test_t* t, const c8* key, const c8* value);
SP_API void sp_test_kv_clear(sp_test_t* t, const c8* key);    // key == NULL clears all

// cleanup: LIFO, runs when the test ends for any reason (pass, fail, assert, skip)
SP_API void sp_test_defer(sp_test_t* t, void (*fn)(void*), void* userdata);

// attached log: buffered per test, printed only when the test fails
SP_API void sp_test_log(sp_test_t* t, const c8* fmt, ...);

// services, created lazily
SP_API sp_mem_t sp_test_mem(sp_test_t* t);    // leak-tracked allocator; leaks fail the test
SP_API sp_mem_t sp_test_arena(sp_test_t* t);  // per-test arena, freed by runner, unchecked
SP_API sp_str_t sp_test_dir(sp_test_t* t);    // sandbox dir, unique per instance, removed by runner
```

The assert macros evaluate operands once (`__typeof__`), compare inline, and on failure
call a plain function with the stringized operands and formatted values. Value formatting
uses clang `overloadable` functions with a `_Generic` fallback (gcc/tcc) and `"?"` as the
last resort — same triage the spn fork uses today.

Failure records carry: file:line of the assertion site, expected/actual strings, a
snapshot of the sticky kvs, and the case identity. Descriptor-driven tests today report
`harness.c:1114` for every failure; here the report leads with the instance name
(`fs.run_copy/copy_dir_nested`), so file:line becomes secondary detail, automatically.

## Control flow

`sp_assert_*` and `sp_test_skip` unwind to the runner via `__builtin_setjmp`/
`__builtin_longjmp` (gcc/clang, works freestanding, no libc). The runner then runs the
defer stack LIFO and reports. This gives, at any helper depth:

- assert failure → test over, cleanup runs
- skip → test skipped with reason, cleanup runs

On targets without setjmp (wasm), assert degrades: it records the failure, marks the
context dead, and returns from the current function; every subsequent check is a no-op
and `sp_test_alive` lets executors early-out. Same reporting, weaker unwind. This is
strictly better than utest.h's current behavior on every target.

Skip reasons are strings computed at runtime (`sp_test_skip(t, "symlinks unavailable: {}", ...)`),
matching the capability-probe pattern that is the best-used feature of the spn fork.
Platform skips become one-liners at the top of a test or executor.

## Memory

- The context's own bookkeeping (failure records, kv, log, fmt) comes from a private
  per-context arena. Nothing in the assert path touches a shared allocator — the spn
  fork's threaded mode races on exactly this.
- `sp_test_mem(t)` wraps the arena in a tracking allocator (the existing
  `sp_mem_tracking_t` design: live/freed lists, double-free and wild-free detection).
  After defers run, outstanding allocations are reported as a failure record
  (`leaked 3 allocations, 210 bytes`). This replaces the 10 copy-pasted
  tracking-fixture setups, and unlike today it cannot be silently skipped.
- `sp_test_arena(t)` is for don't-care allocations (most tests — today they use scratch).
  sp.h's thread-local scratch also works normally on worker threads.

## Runner

- Flatten registry → instances (one per plain test, one per case row).
- `--filter=<wildcard>` matches the full instance name (`fs.run_copy/copy_dir_nested`);
  same wildcard matcher as today. `--list` prints instance names. `--jobs=N` (default 1,
  0 = ncpu). Exit code = failed count.
- Threaded mode: atomic cursor over the instance list; each worker builds a fresh
  context per instance; output is buffered per instance and flushed under one mutex at
  completion (whole-line output, no interleaving). `serial` instances run after the
  parallel batch, on one thread.
- Report format keeps the spn fork's good parts: `suite.decl/case ok 12us` lines,
  red-bar failure blocks with padded keys, skip reasons in gray, summary with failed
  list. Adds: kv snapshot + attached log under the failure bar.

## Porting sketches

`fs/get_ext.c` (table-in-one-test today, loses row identity) becomes:

```c
typedef struct { const c8* name; const c8* input; const c8* expect; } ext_case_t;

static const ext_case_t ext_cases[] = {
  { "basic",     "foo/bar.txt", "txt" },
  { "dotfile",   ".foo",        "foo" },
  { "trailing",  "foo.",        ""    },
};

static void run_ext(sp_test_t* t, const void* arg) {
  const ext_case_t* c = arg;
  sp_expect_str_eq_c(t, sp_fs_get_ext(sp_str_view(c->input)), c->expect);
}
```

Three report lines, three filter targets, and a failing row names itself.

`sys.h`'s `run_sys_test` keeps its exact shape, minus the pain:

```c
static void run_sys(sp_test_t* t, const void* arg) {
  const sys_test_t* c = arg;
  sp_test_file_manager_t* fm = sys_sandbox(t, c->label);   // registers defer internally
  if (sys_wants_symlinks(c) && !sys_probe_symlinks(fm)) {
    sp_test_skip(t, "symlinks not available");             // actually skips, cleanup runs
  }
  sp_carr_for(c->steps, it) {
    if (c->steps[it].kind == SYS_STEP_NONE) break;
    sp_test_kv(t, "step", sp_fmt(sp_test_arena(t), "{} {}", ...).value);  // sticky
    sys_run_step(t, fm, &c->steps[it]);                    // helpers just take t
  }
}
```

No `goto done`, no `s32* utest_result` in six helper signatures, no skip-that-doesn't-skip.

## Open questions

1. Case-table matrices (dag/exec.c runs every case × {mem, fs} store): executor-internal
   loop with `sp_test_kv`, or first-class axes in the decl? Leaning executor-internal.
2. Leak failure vs Zig's orthogonal leak axis (test passes, run fails). Leaning: leak is
   a failure of the test.
3. `sp_test_each`'s `const void*` executor — acceptable, or is a per-suite 3-line typed
   thunk worth mandating in style?
4. Does `sp_test_log` earn its place vs just using kv?
5. Naming: `sp_expect_*`/`sp_assert_*` vs keeping the screaming `EXPECT_*` for grep
   compatibility with 1600 existing sites.
