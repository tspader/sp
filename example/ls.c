#define SP_IMPLEMENTATION
#include "sp.h"

s32 compare_entries(const void* pa, const void* pb) {
  const sp_fs_dir_entry_t* a = (const sp_fs_dir_entry_t*)pa;
  const sp_fs_dir_entry_t* b = (const sp_fs_dir_entry_t*)pb;
  return sp_str_compare_alphabetical(a->name, b->name);
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_str_t path = num_args == 2 ? sp_cstr_as_str(args[1]) : sp_str_lit(".");

  sp_sys_dir_t dir = sp_zero;
  if (sp_fs_open_dir_at(sp_path_cwd(path), &dir)) {
    sp_log("could not read {}", sp_fmt_str(path));
    return 1;
  }

  sp_da(sp_fs_dir_entry_t) entries = sp_da_new(mem, sp_fs_dir_entry_t);
  sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) {
    sp_fs_dir_entry_t copy = { .name = sp_str_copy(mem, entry.name), .kind = entry.kind };
    sp_da_push(entries, copy);
  }
  sp_fs_dir_it_close(&iter);
  sp_fs_close_dir(dir);
  sp_da_sort(entries, compare_entries);

  sp_da_for(entries, it) {
    sp_fs_dir_entry_t* entry = &entries[it];
    switch (entry->kind) {
      case SP_FS_KIND_DIR:     sp_log("{.blue}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_FILE:    sp_log("{}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_SYMLINK: sp_log("{.cyan}", sp_fmt_str(entry->name)); break;
      case SP_FS_KIND_NONE:    break;
    }
  }
  return 0;
}
SP_MAIN(run)
