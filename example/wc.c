#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  if (num_args < 2) {
    sp_log("usage: wc {.fg cyan}", sp_fmt_cstr("$file"));
    return 1;
  }

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_str_t path = sp_cstr_as_str(args[1]);
  sp_str_t content = sp_zero;
  sp_fs_read(mem, path, &content);

  sp_ht(sp_str_t, u32) counts = sp_zero;
  sp_da(sp_str_t) lines = SP_NULLPTR;
  sp_try(sp_str_split_c8(mem, content, '\n', &lines));
  sp_da_for(lines, i) {
    sp_da(sp_str_t) words = SP_NULLPTR;
    sp_try(sp_str_split_c8(mem, lines[i], ' ', &words));

    sp_da_for(words, j) {
      u32* count = sp_str_ht_get(counts, words[j]);
      if (count) {
        *count = *count + 1;
      } else {
        sp_try(sp_str_ht_insert(mem, counts, words[j], 1));
      }
    }
  }

  sp_str_ht_for_kv(counts, it) {
    sp_log("{} {}", sp_fmt_uint(*it.val), sp_fmt_str(*it.key));
  }
  return 0;
}
SP_MAIN(run)
