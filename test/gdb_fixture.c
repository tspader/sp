#include "sp.h"
#include "sp_om.h"

typedef struct { s32 x; s32 y; } point_t;

typedef sp_ht(s32, s32) ht_ints_t;
typedef sp_ht(s32, const c8*) ht_cstrval_t;
typedef sp_str_ht(s32) ht_strkey_t;
typedef sp_ht(point_t, point_t) ht_struct_t;
typedef sp_str_om(s32) om_strkey_t;
typedef sp_str_om(point_t) om_structval_t;

SP_NOINLINE void brk_da_null(sp_da(s32) subject)        { (void)subject; }
SP_NOINLINE void brk_da_empty(sp_da(s32) subject)       { (void)subject; }
SP_NOINLINE void brk_da_ints(sp_da(s32) subject)        { (void)subject; }
SP_NOINLINE void brk_da_cstrs(sp_da(const c8*) subject) { (void)subject; }

SP_NOINLINE void brk_ht_null(ht_ints_t subject)       { (void)subject; }
SP_NOINLINE void brk_ht_empty(ht_ints_t subject)      { (void)subject; }
SP_NOINLINE void brk_ht_ints(ht_ints_t subject)       { (void)subject; }
SP_NOINLINE void brk_ht_cstrval(ht_cstrval_t subject) { (void)subject; }
SP_NOINLINE void brk_ht_strkey(ht_strkey_t subject)   { (void)subject; }
SP_NOINLINE void brk_ht_tombstone(ht_ints_t subject)  { (void)subject; }
SP_NOINLINE void brk_ht_struct(ht_struct_t subject)   { (void)subject; }

SP_NOINLINE void brk_om_null(om_strkey_t subject)       { (void)subject; }
SP_NOINLINE void brk_om_strkey(om_strkey_t subject)     { (void)subject; }
SP_NOINLINE void brk_om_structval(om_structval_t subject) { (void)subject; }

SP_NOINLINE void brk_str(sp_str_t subject)            { (void)subject; }
SP_NOINLINE void brk_str_empty(sp_str_t subject)      { (void)subject; }

SP_NOINLINE void brk_tm(sp_tm_epoch_t subject)        { (void)subject; }

SP_NOINLINE void brk_arena(sp_mem_arena_t* subject)   { (void)subject; }

SP_NOINLINE void brk_ps_config(sp_ps_config_t subject) { (void)subject; }
SP_NOINLINE void brk_env_var(sp_env_var_t subject)     { (void)subject; }

static void run_da(sp_mem_t mem) {
  brk_da_null(SP_NULLPTR);

  {
    sp_da(s32) subject = SP_NULLPTR;
    sp_da_init(mem, subject);
    brk_da_empty(subject);
  }
  {
    sp_da(s32) subject = SP_NULLPTR;
    sp_da_init(mem, subject);
    sp_da_reserve(subject, 16);
    sp_for(it, 5) {
      sp_da_push(subject, (s32)(it * 10));
    }
    brk_da_ints(subject);
  }
  {
    sp_da(const c8*) subject = SP_NULLPTR;
    sp_da_init(mem, subject);
    sp_da_push(subject, "hello");
    sp_da_push(subject, "world");
    brk_da_cstrs(subject);
  }
}

static void run_ht(sp_mem_t mem) {
  brk_ht_null(SP_NULLPTR);

  {
    ht_ints_t subject = SP_NULLPTR;
    sp_ht_init(mem, subject);
    brk_ht_empty(subject);
  }
  {
    ht_ints_t subject = SP_NULLPTR;
    sp_ht_init(mem, subject);
    sp_ht_insert(subject, 69, 420);
    sp_ht_insert(subject, 1, 2);
    sp_ht_insert(subject, 7, 8);
    brk_ht_ints(subject);
  }
  {
    ht_cstrval_t subject = SP_NULLPTR;
    sp_ht_init(mem, subject);
    sp_ht_insert(subject, 1, "one");
    sp_ht_insert(subject, 2, "two");
    brk_ht_cstrval(subject);
  }
  {
    ht_strkey_t subject = SP_NULLPTR;
    sp_str_ht_init(mem, subject);
    sp_str_ht_insert(subject, sp_str_lit("hello"), 42);
    sp_str_ht_insert(subject, sp_str_lit("world"), 99);
    brk_ht_strkey(subject);
  }
  {
    ht_ints_t subject = SP_NULLPTR;
    sp_ht_init(mem, subject);
    sp_ht_insert(subject, 1, 10);
    sp_ht_insert(subject, 2, 20);
    sp_ht_insert(subject, 3, 30);
    sp_ht_erase(subject, 2);
    brk_ht_tombstone(subject);
  }
  {
    ht_struct_t subject = SP_NULLPTR;
    sp_ht_init(mem, subject);
    point_t k1 = { .x = 1, .y = 2 };
    point_t v1 = { .x = 3, .y = 4 };
    sp_ht_insert(subject, k1, v1);
    brk_ht_struct(subject);
  }
}

static void run_om(void) {
  brk_om_null(SP_NULLPTR);

  {
    om_strkey_t subject = SP_NULLPTR;
    sp_str_om_insert(subject, sp_str_lit("alpha"), 1);
    sp_str_om_insert(subject, sp_str_lit("beta"), 2);
    sp_str_om_insert(subject, sp_str_lit("gamma"), 3);
    brk_om_strkey(subject);
  }
  {
    om_structval_t subject = SP_NULLPTR;
    point_t a = { .x = 10, .y = 20 };
    point_t b = { .x = 30, .y = 40 };
    sp_str_om_insert(subject, sp_str_lit("origin"), a);
    sp_str_om_insert(subject, sp_str_lit("corner"), b);
    brk_om_structval(subject);
  }
}

static void run_str(void) {
  brk_str(sp_str_lit("hello world"));
  brk_str_empty(sp_str_lit(""));
}

static void run_tm(void) {
  brk_tm((sp_tm_epoch_t) { .s = 1700000000, .ns = 123456789 });
}

static void run_arena(void) {
  sp_mem_arena_t* arena = sp_mem_arena_new_ex(sp_mem_os_new(), 256, 0);
  sp_for(it, 4) {
    sp_mem_arena_alloc(arena, 100);
  }
  brk_arena(arena);
  sp_mem_arena_destroy(arena);
}

static void run_ps(void) {
  sp_ps_config_t config = sp_zero;
  config.command = sp_str_lit("ls");
  config.args[0] = sp_str_lit("-la");
  config.args[1] = sp_str_lit("/tmp");
  config.cwd = sp_str_lit("/home");
  config.io.out.mode = SP_PS_IO_MODE_NULL;
  config.io.err.mode = SP_PS_IO_MODE_INHERIT;
  brk_ps_config(config);

  brk_env_var((sp_env_var_t) {
    .key = sp_str_lit("PATH"),
    .value = sp_str_lit("/usr/bin"),
  });
}

static s32 gdb_fixture_main(s32 num_args, const c8** args) {
  (void)num_args;
  (void)args;

  sp_mem_t mem = sp_mem_arena_as_allocator(sp_mem_arena_new(sp_mem_os_new()));

  run_da(mem);
  run_ht(mem);
  run_om();
  run_str();
  run_tm();
  run_arena();
  run_ps();

  return 0;
}

SP_MAIN(gdb_fixture_main)
