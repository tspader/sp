#ifndef SYS_REPARSE_H
#define SYS_REPARSE_H

#define SP_PRIVATE_HEADER
#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_WIN32)

#define REPARSE_STUB_TAG 0x00001234

typedef struct {
  u32 ReparseTag;
  u16 ReparseDataLength;
  u16 Reserved;
  u8 ReparseGuid [16];
  u8 Data [4];
} reparse_stub_t;

static HANDLE reparse_open(sp_mem_t mem, sp_path_t path, u32 access) {
  sp_str_t parent = sp_zero;
  if (sp_fs_canonicalize_path_at(mem, sp_path_parent(mem, path), &parent)) return INVALID_HANDLE_VALUE;

  sp_sys_nt_path_t* nt = sp_alloc_type(mem, sp_sys_nt_path_t);
  sp_str_t full = sp_fs_join_path(mem, parent, sp_fs_get_name(path.sub));
  if (!SP_NT_SUCCESS(sp_sys_nt_path(full, nt))) return INVALID_HANDLE_VALUE;

  u16* wide = nt->name.Buffer;
  wide[nt->name.Length / sizeof(u16)] = 0;
  wide[1] = '\\';
  return CreateFileW(
    (const WCHAR*)wide,
    access,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    SP_NULLPTR,
    OPEN_EXISTING,
    FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
    SP_NULLPTR
  );
}

static sp_err_t reparse_set(sp_mem_t mem, sp_path_t path, void* data, u32 len) {
  HANDLE handle = reparse_open(mem, path, GENERIC_WRITE);
  if (handle == INVALID_HANDLE_VALUE) return SP_ERR_SYS;

  DWORD returned = 0;
  sp_err_t err = SP_OK;
  if (!DeviceIoControl(handle, SP_NT_FSCTL_SET_REPARSE_POINT, data, len, SP_NULLPTR, 0, &returned, SP_NULLPTR)) {
    err = GetLastError() == ERROR_INVALID_FUNCTION ? SP_ERR_SYS_UNSUPPORTED : SP_ERR_SYS;
  }
  CloseHandle(handle);
  return err;
}

static sp_err_t reparse_get_tag(sp_mem_t mem, sp_path_t path, u32* tag) {
  HANDLE handle = reparse_open(mem, path, FILE_READ_ATTRIBUTES);
  if (handle == INVALID_HANDLE_VALUE) return SP_ERR_SYS;

  u8* data = sp_alloc_n(mem, u8, SP_NT_MAX_REPARSE_DATA);
  DWORD returned = 0;
  BOOL ok = DeviceIoControl(handle, SP_NT_FSCTL_GET_REPARSE_POINT, SP_NULLPTR, 0, data, SP_NT_MAX_REPARSE_DATA, &returned, SP_NULLPTR);
  CloseHandle(handle);
  if (!ok) return SP_ERR_SYS;

  sp_mem_copy(tag, data, sizeof(u32));
  return SP_OK;
}

static sp_err_t reparse_create_junction(sp_mem_t mem, sp_path_t path, sp_str_t target) {
  sp_sys_nt_path_t* nt = sp_alloc_type(mem, sp_sys_nt_path_t);
  if (!SP_NT_SUCCESS(sp_sys_nt_path(target, nt))) return SP_ERR_SYS_INVALID;
  sp_try(sp_sys_mkdir_s(path.dir, path.sub, sp_sys_default_dir_perms));

  u32 name_bytes = nt->name.Length;
  u32 len = (u32)sizeof(sp_nt_mount_point_reparse_buffer_t) + name_bytes + 2 * (u32)sizeof(u16);
  u8* data = sp_alloc_n(mem, u8, len);
  sp_nt_mount_point_reparse_buffer_t rdb = {
    .ReparseTag = SP_NT_IO_REPARSE_TAG_MOUNT_POINT,
    .ReparseDataLength = (u16)(len - 8),
    .SubstituteNameLength = (u16)name_bytes,
    .PrintNameOffset = (u16)(name_bytes + sizeof(u16)),
  };
  sp_mem_copy(data, &rdb, sizeof(rdb));
  sp_mem_copy(data + sizeof(rdb), nt->name.Buffer, name_bytes);
  return reparse_set(mem, path, data, len);
}

static sp_err_t reparse_create_stub(sp_mem_t mem, sp_path_t path, sp_fs_kind_t kind) {
  switch (kind) {
    case SP_FS_KIND_FILE: {
      sp_try(sp_fs_write_at(path, sp_str_lit(""), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
      break;
    }
    case SP_FS_KIND_DIR: {
      sp_try(sp_sys_mkdir_s(path.dir, path.sub, sp_sys_default_dir_perms));
      break;
    }
    case SP_FS_KIND_SYMLINK:
    case SP_FS_KIND_UNKNOWN:
    case SP_FS_KIND_NONE: {
      return SP_ERR_SYS_INVALID;
    }
  }

  reparse_stub_t stub = {
    .ReparseTag = REPARSE_STUB_TAG,
    .ReparseDataLength = sizeof(stub.Data),
    .ReparseGuid = { 0x53, 0x50 },
  };
  return reparse_set(mem, path, &stub, sizeof(stub));
}

static bool reparse_supported(sp_test_t* t) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t probe = sp_path_join(mem, sp_test_dir(t), sp_str_lit("R"));
  sp_err_t err = reparse_create_stub(mem, probe, SP_FS_KIND_FILE);
  sp_fs_remove_file_at(probe);
  return err != SP_ERR_SYS_UNSUPPORTED;
}

static bool reparse_on_network_path(sp_test_t* t) {
  static const u16 unc [] = { '\\', '?', '?', '\\', 'U', 'N', 'C', '\\' };
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t root = sp_zero;
  if (sp_fs_canonicalize_path_at(mem, sp_test_dir(t), &root)) return false;

  sp_sys_nt_path_t* nt = sp_alloc_type(mem, sp_sys_nt_path_t);
  if (!SP_NT_SUCCESS(sp_sys_nt_path(root, nt))) return false;
  return nt->name.Length >= sizeof(unc) && sp_mem_is_equal(nt->name.Buffer, unc, sizeof(unc));
}

#define reparse_skip_unless_supported() if (!reparse_supported(t)) return sp_test_skip(t, "reparse points not available");
#define reparse_skip_on_network_path() if (reparse_on_network_path(t)) return sp_test_skip(t, "a junction cannot target a network path");

#else

#define reparse_skip_unless_supported() ((void)0)
#define reparse_skip_on_network_path() ((void)0)

#endif

#endif
