#define SP_IMPLEMENTATION
#include "sp.h"

static void print_entry(sp_mem_t mem, sp_fs_it_t* it) {
  sp_sys_file_meta_t meta = sp_zero;
  sp_sys_get_link_metadata_s(it->at.dir, it->at.sub, &meta);

  sp_tm_epoch_t modified = { .s = (u64)meta.mtime.tv_sec, .ns = (u32)meta.mtime.tv_nsec };
  sp_print("{:>10} {.gray} ", sp_fmt_int(meta.size), sp_fmt_str(sp_tm_epoch_to_iso8601(mem, modified)));

  switch (it->entry.kind) {
    case SP_FS_KIND_DIR: {
      sp_log("{.blue}", sp_fmt_str(it->entry.name));
      break;
    }
    case SP_FS_KIND_FILE: {
      sp_log("{}", sp_fmt_str(it->entry.name));
      break;
    }
    case SP_FS_KIND_SYMLINK: {
      c8 buf [SP_PATH_MAX];
      sp_str_t target = sp_zero;
      sp_sys_readlink_s(it->at.dir, it->at.sub, buf, sizeof(buf), &target);
      sp_log("{.cyan} -> {}", sp_fmt_str(it->entry.name), sp_fmt_str(target));
      break;
    }
    case SP_FS_KIND_NONE: {
      break;
    }
  }
}

s32 run(s32 num_args, const c8** args) {
  sp_str_t dir = num_args > 1 ? sp_cstr_as_str(args[1]) : sp_str_lit(".");

  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_fs_it_t it = sp_fs_it_new_at(s.mem, sp_path_from_str(dir), 0);
  while (sp_fs_it_next(&it)) {
    print_entry(s.mem, &it);
  }

  sp_err_t err = it.err;
  if (err) {
    sp_log("{.red}: {}", sp_fmt_str(dir), sp_fmt_str(sp_err_str(err)));
  }

  sp_fs_it_deinit(&it);
  sp_mem_end_scratch(s);
  return err ? 1 : 0;
}
SP_MAIN(run)
