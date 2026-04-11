#define SP_IMPLEMENTATION
#include "sp.h"

void wc_split(sp_str_t text) {
}

s32 main(s32 num_args, const c8** args) {
  if (num_args < 2) {
    sp_log("usage: wc {:fg cyan}", SP_FMT_CSTR("$file"));
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
    sp_log("{} {}", SP_FMT_U32(*it.val), SP_FMT_STR(*it.key));
  }
  return 0;
}
