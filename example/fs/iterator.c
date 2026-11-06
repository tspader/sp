#define SP_IMPLEMENTATION
#include "sp.h"

void flat(sp_sys_dir_t dir) {
  sp_fs_dir_it_t it = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  while (sp_fs_dir_it_next(&it, sp_mem_slice(buf, sizeof(buf)), &entry)) {
    sp_log("{} {}", sp_fmt_str(entry.name), sp_fmt_int(entry.kind));
  }
  sp_fs_dir_it_close(&it);
}

void recursive(sp_mem_t mem, sp_sys_dir_t dir) {
  sp_fs_it_t it = sp_fs_it(mem, dir);
  sp_fs_it_entry_t entry = sp_zero;
  while (sp_fs_it_next(&it, &entry)) {
    sp_log("{}", sp_fmt_str(entry.path));
  }
  sp_fs_it_deinit(&it);
}

void pruned(sp_mem_t mem, sp_sys_dir_t dir) {
  sp_fs_it_t it = sp_fs_it(mem, dir);
  sp_fs_it_entry_t entry = sp_zero;
  while (sp_fs_it_next(&it, &entry)) {
    if (sp_str_equal(entry.name, sp_str_lit(".git"))) sp_fs_it_skip(&it);
    sp_log("{}", sp_fmt_str(entry.path));
  }
  sp_fs_it_deinit(&it);
}

void ordered(sp_mem_t mem, sp_sys_dir_t dir) {
  u32 counts [SP_FS_IT_MAX_DEPTH] = sp_zero;

  sp_fs_it_t it = sp_fs_it(mem, dir);
  sp_fs_visit_t visit = sp_zero;
  while (sp_fs_it_visit(&it, &visit)) {
    sp_fs_it_entry_t entry = visit.entry;
    u32 depth = sp_fs_it_get_entry_depth(entry);
    switch (visit.order) {
      case SP_FS_PRE_ORDER: {
        if (entry.kind != SP_FS_KIND_DIR) counts[depth - 1]++;
        else if (sp_str_equal(entry.name, sp_str_lit(".git"))) sp_fs_it_skip(&it);
        else counts[depth] = 0;
        break;
      }
      case SP_FS_POST_ORDER: {
        sp_log("{}: {}", sp_fmt_str(entry.path), sp_fmt_int(counts[depth]));
        counts[depth - 1] += counts[depth];
        break;
      }
    }
  }
  sp_fs_it_deinit(&it);

  sp_log("total: {}", sp_fmt_int(counts[0]));
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_str_t path = num_args == 2 ? sp_cstr_as_str(args[1]) : sp_str_lit(".");

  sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
  if (sp_fs_open_dir_at(sp_path_cwd(path), &dir)) {
    sp_log("could not open {}", sp_fmt_str(path));
    return 1;
  }

  flat(dir);
  recursive(mem, dir);
  pruned(mem, dir);
  ordered(mem, dir);

  sp_fs_close_dir(dir);
  return 0;
}
SP_MAIN(run)
