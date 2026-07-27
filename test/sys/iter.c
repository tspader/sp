#include "harness.h"

#if !defined(SP_WASM)

#define SYS_ITER_MAX_RAW 8
#define SYS_ITER_BUF_SIZE 4096

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
} sys_iter_raw_t;

typedef struct {
  const c8* name;
  sys_iter_raw_t raw [SYS_ITER_MAX_RAW];
} sys_iter_case_t;

#if defined(SP_WIN32)
static u64 sys_iter_pack(u8* buf, const sys_iter_raw_t* raw) {
  u64 cursor = 0;
  u64 len = 0;
  sp_for(it, SYS_ITER_MAX_RAW) {
    if (!raw[it].name) break;
    u32 name_len = sp_cstr_len(raw[it].name);

    sp_nt_file_directory_information_t* d = sp_ptr_cast(sp_nt_file_directory_information_t*, buf + cursor);
    *d = sp_zero_s(sp_nt_file_directory_information_t);
    d->FileNameLength = name_len * sizeof(u16);
    sp_for(n, name_len) {
      d->FileName[n] = (u16)raw[it].name[n];
    }
    switch (raw[it].kind) {
      case SP_FS_KIND_FILE: { d->FileAttributes = FILE_ATTRIBUTE_NORMAL; break; }
      case SP_FS_KIND_DIR: { d->FileAttributes = FILE_ATTRIBUTE_DIRECTORY; break; }
      case SP_FS_KIND_SYMLINK: { d->FileAttributes = FILE_ATTRIBUTE_REPARSE_POINT; break; }
      case SP_FS_KIND_NONE: { break; }
    }

    len = cursor + sizeof(sp_nt_file_directory_information_t) + d->FileNameLength;
    bool last = it + 1 >= SYS_ITER_MAX_RAW || !raw[it + 1].name;
    d->NextEntryOffset = last ? 0 : (u32)sp_align_offset(sizeof(sp_nt_file_directory_information_t) + d->FileNameLength, 8);
    cursor += d->NextEntryOffset;
  }
  return len;
}
#elif defined(SP_LINUX)
static u64 sys_iter_pack(u8* buf, const sys_iter_raw_t* raw) {
  u64 cursor = 0;
  sp_for(it, SYS_ITER_MAX_RAW) {
    if (!raw[it].name) break;
    u32 name_len = sp_cstr_len(raw[it].name);

    sp_sys_dirent64_t* d = sp_ptr_cast(sp_sys_dirent64_t*, buf + cursor);
    d->d_ino = it + 1;
    d->d_off = 0;
    d->d_reclen = (u16)sp_align_offset(sizeof(sp_sys_dirent64_t) + name_len + 1, 8);
    switch (raw[it].kind) {
      case SP_FS_KIND_FILE: { d->d_type = SP_DT_REG; break; }
      case SP_FS_KIND_DIR: { d->d_type = SP_DT_DIR; break; }
      case SP_FS_KIND_SYMLINK: { d->d_type = SP_DT_LNK; break; }
      case SP_FS_KIND_NONE: { d->d_type = 0; break; }
    }
    sp_mem_copy(d->d_name, raw[it].name, name_len + 1);
    cursor += d->d_reclen;
  }
  return cursor;
}
#elif defined(SP_MACOS) || defined(SP_COSMO)
static u64 sys_iter_pack(u8* buf, const sys_iter_raw_t* raw) {
  u64 cursor = 0;
  sp_for(it, SYS_ITER_MAX_RAW) {
    if (!raw[it].name) break;
    u32 name_len = sp_cstr_len(raw[it].name);

    struct dirent d = sp_zero;
    d.d_reclen = sizeof(struct dirent);
    switch (raw[it].kind) {
      case SP_FS_KIND_FILE: { d.d_type = SP_DT_REG; break; }
      case SP_FS_KIND_DIR: { d.d_type = SP_DT_DIR; break; }
      case SP_FS_KIND_SYMLINK: { d.d_type = SP_DT_LNK; break; }
      case SP_FS_KIND_NONE: { d.d_type = 0; break; }
    }
    sp_mem_copy(d.d_name, raw[it].name, name_len + 1);
    sp_mem_copy(buf + cursor, &d, sizeof(struct dirent));
    cursor += sizeof(struct dirent);
  }
  return cursor;
}
#endif

static const sys_iter_case_t sys_iter_cases [] = {
  {
    .name = "decodes_kinds",
    .raw = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_DIR },
      { "L", SP_FS_KIND_SYMLINK },
    },
  },
  {
    .name = "returns_dot_entries",
    .raw = {
      { ".", SP_FS_KIND_DIR },
      { "..", SP_FS_KIND_DIR },
      { "A", SP_FS_KIND_FILE },
    },
  },
};

sp_test_each(sys, iter, sys_iter_case_t, sys_iter_cases) {
  SP_ALIGNED u8 raw [SYS_ITER_BUF_SIZE];
  sp_mem_buffer_t buf = {
    .data = raw,
    .capacity = sizeof(raw),
  };
  buf.len = sys_iter_pack(raw, it->raw);

  sp_sys_dir_t dir = sp_zero;
  u64 cursor = 0;

  sp_carr_for(it->raw, n) {
    if (!it->raw[n].name) break;

    if (cursor >= buf.len) {
      sp_test_fail(t, "parse consumed the buffer before {}", sp_fmt_cstr(it->raw[n].name));
      return SP_ERR;
    }

    sp_test_kv_c(t, "entry", it->raw[n].name);

    sp_sys_dir_entry_t entry = sp_zero;
    sp_must_ok(t, sp_sys_dir_parse(&dir, &buf, &cursor, &entry));

    sp_expect_str_eq(t, sp_str(entry.name, entry.len), sp_cstr_as_str(it->raw[n].name));
    sp_expect_eq(t, (u32)entry.kind, (u32)it->raw[n].kind);
  }

  sp_test_kv_clear(t, "entry");
  sp_expect_eq(t, cursor, buf.len);
  return SP_OK;
}

#endif
