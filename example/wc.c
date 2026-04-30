#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  if (num_args < 2) {
    sp_log_a("usage: wc {.fg cyan}", sp_fmt_cstr("$file"));
    return 1;
  }

  sp_mem_t mem = sp_mem_os_new();

  sp_str_t cwd = sp_fs_get_cwd_a(mem);
  sp_str_t path = sp_fs_join_path_a(mem, cwd, sp_str_view(args[1]));
  sp_str_t content = sp_zero_initialize();
  sp_io_read_file_a(mem, path, &content);

  sp_ht_a(sp_str_t, u32) counts = sp_zero_initialize();
  sp_str_ht_init_a(mem, counts);
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
    sp_log_a("{} {}", sp_fmt_uint(*it.val), sp_fmt_str(*it.key));
  }
  return 0;
}
SP_MAIN(run)
