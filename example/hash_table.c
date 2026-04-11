#define SP_IMPLEMENTATION
#include "sp.h"

typedef struct {
  s32 foo;
  u32 bar;
} ht_key_t;

void header(const c8* key, const c8* value) {
  sp_log("[{:fg brightcyan}] -> {:fg brightgreen}", SP_FMT_CSTR(key), SP_FMT_CSTR(value));

}

s32 run(s32 num_args, const c8** args) {
  struct {
    sp_ht(s32, u32) integer;
    sp_cstr_ht(u8) cstr;
    sp_ht(ht_key_t, const c8*) key;
  } hts = sp_zero_initialize();

  header("const c8*", "u8");
  sp_cstr_ht_insert(hts.cstr, "kram", 8);
  sp_cstr_ht_insert(hts.cstr, "qux", 69);
  sp_ht_for_kv(hts.cstr, it) {
    sp_log("[{}] -> {}", SP_FMT_CSTR(*it.key), SP_FMT_U8(*it.val));
  }
  sp_log("");

  header("ht_key_t", "const c8*");
  ht_key_t key = { -1, 256 };
  sp_ht_insert(hts.key, key, "first");
  sp_ht_insert(hts.key, ((ht_key_t) { -69, 256 }), "second");
  sp_ht_for_kv(hts.key, it) {
    sp_log("[{}, {}] -> {} {:fg brightblack}", SP_FMT_S32(it.key->foo), SP_FMT_U32(it.key->bar), SP_FMT_CSTR(*it.val), SP_FMT_PTR(it.val));
  }
  return 0;
}
SP_ENTRY(run)
