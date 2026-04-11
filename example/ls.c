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
      case SP_FS_KIND_DIR:  sp_log("{:fg blue}", SP_FMT_STR(entry->name)); break;
      case SP_FS_KIND_FILE: sp_log("{}", SP_FMT_STR(entry->name)); break;
      case SP_FS_KIND_SYMLINK: {
        sp_str_t target = sp_fs_canonicalize_path(entry->path);
        sp_log("{:fg cyan} -> {}", SP_FMT_STR(entry->name), SP_FMT_STR(target));
        break;
      }
      case SP_FS_KIND_NONE: break;
    }
  }
}
