#ifndef SP_MSVC_H
#define SP_MSVC_H

#include "sp.h"

#ifndef SP_MSVC_MAX_SDKS
  #define SP_MSVC_MAX_SDKS 8
#endif

#ifndef SP_MSVC_MAX_VS
  #define SP_MSVC_MAX_VS 8
#endif

#ifndef SP_MSVC_PATH_MAX
  #define SP_MSVC_PATH_MAX 512
#endif

#define SP_MSVC_VERSION_MAX 24

typedef enum {
  SP_MSVC_ARCH_X64,
  SP_MSVC_ARCH_ARM64,
} sp_msvc_arch_t;

typedef enum {
  SP_MSVC_OK,
  SP_MSVC_ERR_SDK_NOT_FOUND,
  SP_MSVC_ERR_VS_NOT_FOUND,
  SP_MSVC_ERR_REGISTRY,
  SP_MSVC_ERR_UNSUPPORTED,
} sp_msvc_err_t;

typedef enum {
  SP_MSVC_SDK_PATH_LIB_UM,
  SP_MSVC_SDK_PATH_LIB_UCRT,
  SP_MSVC_SDK_PATH_INCLUDE_UCRT,
  SP_MSVC_SDK_PATH_INCLUDE_UM,
  SP_MSVC_SDK_PATH_INCLUDE_SHARED,
} sp_msvc_sdk_path_id_t;

typedef enum {
  SP_MSVC_VS_PATH_LIB,
  SP_MSVC_VS_PATH_INCLUDE,
  SP_MSVC_VS_PATH_BIN,
} sp_msvc_vs_path_id_t;

typedef struct {
  c8 data [SP_MSVC_PATH_MAX];
  u32 len;
} sp_msvc_path_t;

typedef struct {
  c8 str [SP_MSVC_VERSION_MAX];
  u32 str_len;
  u32 major;
  u32 minor;
  u32 build;
  u32 revision;
} sp_msvc_version_t;

typedef struct {
  sp_msvc_version_t version;
  sp_msvc_path_t root;
  sp_msvc_arch_t arch;
} sp_msvc_sdk_t;

typedef struct {
  struct {
    sp_msvc_version_t product;
    sp_msvc_version_t build;
    sp_msvc_version_t tools;
  } version;
  sp_msvc_path_t install_path;
  sp_msvc_arch_t host;
  sp_msvc_arch_t target;
} sp_msvc_vs_t;

typedef struct {
  sp_str_t lib_um;
  sp_str_t lib_ucrt;
  sp_str_t include_ucrt;
  sp_str_t include_um;
  sp_str_t include_shared;
} sp_msvc_sdk_paths_t;

typedef struct {
  sp_str_t lib;
  sp_str_t include;
  sp_str_t bin;
} sp_msvc_vs_paths_t;

typedef struct {
  sp_msvc_sdk_t sdks [SP_MSVC_MAX_SDKS];
  u32 num_sdks;
  sp_msvc_vs_t installations [SP_MSVC_MAX_VS];
  u32 num_installations;
} sp_msvc_t;

SP_API sp_str_t sp_msvc_path_str(const sp_msvc_path_t* path);
SP_API sp_str_t sp_msvc_version_str(const sp_msvc_version_t* version);

SP_API sp_str_t sp_msvc_sdk_path(sp_mem_t mem, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id);
SP_API sp_str_r sp_msvc_sdk_path_buf(c8* buffer, u64 len, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id);
SP_API sp_err_t sp_msvc_sdk_path_io(sp_io_writer_t* io, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id);
SP_API sp_str_t sp_msvc_vs_path(sp_mem_t mem, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id);
SP_API sp_str_r sp_msvc_vs_path_buf(c8* buffer, u64 len, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id);
SP_API sp_err_t sp_msvc_vs_path_io(sp_io_writer_t* io, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id);

SP_API sp_msvc_sdk_paths_t sp_msvc_sdk_render(sp_mem_t mem, const sp_msvc_sdk_t* sdk);
SP_API sp_msvc_vs_paths_t  sp_msvc_vs_render(sp_mem_t mem, const sp_msvc_vs_t* vs);

SP_API sp_msvc_err_t sp_msvc_find(sp_msvc_t* out);
SP_API sp_msvc_err_t sp_msvc_find_ex(sp_msvc_arch_t target, sp_msvc_t* out);
#endif // SP_MSVC_H

#if defined(SP_IMPLEMENTATION) && !defined(SP_MSVC_IMPLEMENTATION)
  #define SP_MSVC_IMPLEMENTATION
#endif

#ifndef SP_MSVC_IMPL_H
#if defined(SP_PRIVATE_HEADER) || defined(SP_MSVC_IMPLEMENTATION)
#define SP_MSVC_IMPL_H

typedef struct {
  sp_msvc_path_t install_path;
  sp_msvc_path_t build_version;
  sp_msvc_path_t product_line;
} sp_msvc_state_t;

SP_PRIVATE sp_msvc_path_t    sp_msvc_path_new(sp_str_t str);
SP_PRIVATE sp_msvc_version_t sp_msvc_parse_version(sp_str_t str);
SP_PRIVATE bool              sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b);
SP_PRIVATE bool              sp_msvc_parse_state(sp_io_reader_t* reader, sp_msvc_state_t* out);
SP_PRIVATE sp_msvc_sdk_t     sp_msvc_sdk_new(sp_msvc_arch_t arch, sp_str_t root, sp_str_t version);
SP_PRIVATE sp_msvc_vs_t      sp_msvc_vs_new(sp_msvc_arch_t host, sp_msvc_arch_t target, const sp_msvc_state_t* state, sp_str_t tools_version);
#endif
#endif // SP_MSVC_IMPL_H

#if defined(SP_MSVC_IMPLEMENTATION) && !defined(SP_MSVC_IMPLEMENTED)
#define SP_MSVC_IMPLEMENTED

static sp_str_t sp_msvc_arch_to_str(sp_msvc_arch_t arch) {
  switch (arch) {
    case SP_MSVC_ARCH_X64:   { return sp_str_lit("x64"); }
    case SP_MSVC_ARCH_ARM64: { return sp_str_lit("arm64"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit("x64"));
}

sp_str_t sp_msvc_path_str(const sp_msvc_path_t* path) {
  return sp_str(path->data, path->len);
}

sp_str_t sp_msvc_version_str(const sp_msvc_version_t* version) {
  return sp_str(version->str, version->str_len);
}

SP_PRIVATE sp_msvc_path_t sp_msvc_path_new(sp_str_t str) {
  sp_msvc_path_t path = sp_zero;
  path.len = sp_min(str.len, SP_MSVC_PATH_MAX);
  sp_str_copy_to(str, path.data, SP_MSVC_PATH_MAX);
  return path;
}

static void sp_msvc_path_normalize(sp_msvc_path_t* path) {
  sp_for(it, path->len) {
    if (path->data[it] == '\\') path->data[it] = '/';
  }
  if (path->len && path->data[path->len - 1] == '/') {
    path->len--;
  }
}

SP_PRIVATE sp_msvc_version_t sp_msvc_parse_version(sp_str_t str) {
  u32 parts [4] = sp_zero;
  sp_str_t rest = str;
  sp_carr_for(parts, it) {
    if (sp_str_empty(rest)) break;
    sp_str_pair_t pair = sp_str_cleave_c8(rest, '.');
    parts[it] = sp_parse_u32(pair.first);
    rest = pair.second;
  }

  sp_msvc_version_t version = {
    .str_len  = sp_min(str.len, SP_MSVC_VERSION_MAX),
    .major    = parts[0],
    .minor    = parts[1],
    .build    = parts[2],
    .revision = parts[3],
  };
  sp_str_copy_to(str, version.str, SP_MSVC_VERSION_MAX);
  return version;
}

SP_PRIVATE bool sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b) {
  if (a.major != b.major) return a.major > b.major;
  if (a.minor != b.minor) return a.minor > b.minor;
  if (a.build != b.build) return a.build > b.build;
  return a.revision > b.revision;
}

typedef struct {
  sp_msvc_path_t str;
  bool overflow;
  bool closed;
} sp_msvc_json_str_t;

static bool sp_msvc_json_read_c8(sp_io_reader_t* reader, c8* c) {
  u64 num_read = 0;
  if (sp_io_read(reader, c, 1, &num_read)) return false;
  return num_read == 1;
}

static bool sp_msvc_json_seek_string(sp_io_reader_t* reader) {
  c8 c = 0;
  while (sp_msvc_json_read_c8(reader, &c)) {
    if (c == '"') return true;
  }
  return false;
}

static sp_msvc_json_str_t sp_msvc_json_read_string(sp_io_reader_t* reader) {
  sp_msvc_json_str_t result = sp_zero;
  c8 c = 0;
  while (sp_msvc_json_read_c8(reader, &c)) {
    if (c == '"') {
      result.closed = true;
      return result;
    }
    if (c == '\\' && !sp_msvc_json_read_c8(reader, &c)) {
      return result;
    }
    if (result.str.len < SP_MSVC_PATH_MAX) {
      result.str.data[result.str.len++] = c;
    }
    else {
      result.overflow = true;
    }
  }
  return result;
}

static c8 sp_msvc_json_next_token(sp_io_reader_t* reader) {
  c8 c = 0;
  while (sp_msvc_json_read_c8(reader, &c)) {
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') return c;
  }
  return 0;
}

SP_PRIVATE bool sp_msvc_parse_state(sp_io_reader_t* reader, sp_msvc_state_t* out) {
  *out = sp_zero_s(sp_msvc_state_t);

  struct {
    sp_str_t key;
    sp_msvc_path_t* value;
    bool seen;
  } fields [] = {
    { .key = sp_str_lit("installationPath"),   .value = &out->install_path },
    { .key = sp_str_lit("buildVersion"),       .value = &out->build_version },
    { .key = sp_str_lit("productLineVersion"), .value = &out->product_line },
  };

  u32 num_seen = 0;
  while (num_seen < sp_carr_len(fields)) {
    if (!sp_msvc_json_seek_string(reader)) break;
    sp_msvc_json_str_t key = sp_msvc_json_read_string(reader);
    if (!key.closed) break;
    if (sp_msvc_json_next_token(reader) != ':') continue;
    if (sp_msvc_json_next_token(reader) != '"') continue;
    sp_msvc_json_str_t value = sp_msvc_json_read_string(reader);

    sp_carr_for(fields, it) {
      if (fields[it].seen) continue;
      if (!sp_str_equal(sp_msvc_path_str(&key.str), fields[it].key)) continue;
      if (value.overflow) return false;
      *fields[it].value = value.str;
      fields[it].seen = true;
      num_seen++;
      break;
    }

    if (!value.closed) break;
  }

  sp_msvc_path_normalize(&out->install_path);
  return !sp_str_empty(sp_msvc_path_str(&out->install_path)) && !sp_str_empty(sp_msvc_path_str(&out->build_version));
}

SP_PRIVATE sp_msvc_sdk_t sp_msvc_sdk_new(sp_msvc_arch_t arch, sp_str_t root, sp_str_t version) {
  return (sp_msvc_sdk_t) {
    .version = sp_msvc_parse_version(version),
    .root    = sp_msvc_path_new(root),
    .arch    = arch,
  };
}

SP_PRIVATE sp_msvc_vs_t sp_msvc_vs_new(sp_msvc_arch_t host, sp_msvc_arch_t target, const sp_msvc_state_t* state, sp_str_t tools_version) {
  return (sp_msvc_vs_t) {
    .version = {
      .product = sp_msvc_parse_version(sp_msvc_path_str(&state->product_line)),
      .build   = sp_msvc_parse_version(sp_msvc_path_str(&state->build_version)),
      .tools   = sp_msvc_parse_version(tools_version),
    },
    .install_path = state->install_path,
    .host         = host,
    .target       = target,
  };
}

sp_err_t sp_msvc_sdk_path_io(sp_io_writer_t* io, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id) {
  sp_str_t root = sp_msvc_path_str(&sdk->root);
  sp_str_t version = sp_msvc_version_str(&sdk->version);
  sp_str_t arch = sp_msvc_arch_to_str(sdk->arch);

  switch (id) {
    case SP_MSVC_SDK_PATH_LIB_UM:         return sp_fmt_io(io, "{}/Lib/{}/um/{}",      sp_fmt_str(root), sp_fmt_str(version), sp_fmt_str(arch));
    case SP_MSVC_SDK_PATH_LIB_UCRT:       return sp_fmt_io(io, "{}/Lib/{}/ucrt/{}",    sp_fmt_str(root), sp_fmt_str(version), sp_fmt_str(arch));
    case SP_MSVC_SDK_PATH_INCLUDE_UCRT:   return sp_fmt_io(io, "{}/Include/{}/ucrt",   sp_fmt_str(root), sp_fmt_str(version));
    case SP_MSVC_SDK_PATH_INCLUDE_UM:     return sp_fmt_io(io, "{}/Include/{}/um",     sp_fmt_str(root), sp_fmt_str(version));
    case SP_MSVC_SDK_PATH_INCLUDE_SHARED: return sp_fmt_io(io, "{}/Include/{}/shared", sp_fmt_str(root), sp_fmt_str(version));
  }
  SP_UNREACHABLE_RETURN(SP_ERR);
}

sp_str_r sp_msvc_sdk_path_buf(c8* buffer, u64 len, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id) {
  sp_io_mem_writer_t io = sp_zero;
  sp_io_mem_writer_from_buffer(&io, buffer, len);

  sp_str_r result = sp_zero;
  result.err = sp_msvc_sdk_path_io(&io.base, sdk, id);
  if (!result.err) result.value = sp_io_mem_writer_as_str(&io);
  return result;
}

sp_str_t sp_msvc_sdk_path(sp_mem_t mem, const sp_msvc_sdk_t* sdk, sp_msvc_sdk_path_id_t id) {
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &io);
  sp_msvc_sdk_path_io(&io.base, sdk, id);
  return sp_io_dyn_mem_writer_as_str(&io);
}

sp_err_t sp_msvc_vs_path_io(sp_io_writer_t* io, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id) {
  sp_str_t install = sp_msvc_path_str(&vs->install_path);
  sp_str_t tools = sp_msvc_version_str(&vs->version.tools);
  sp_str_t host = sp_msvc_arch_to_str(vs->host);
  sp_str_t target = sp_msvc_arch_to_str(vs->target);

  switch (id) {
    case SP_MSVC_VS_PATH_LIB: return sp_fmt_io(io, "{}/VC/Tools/MSVC/{}/Lib/{}", sp_fmt_str(install), sp_fmt_str(tools), sp_fmt_str(target));
    case SP_MSVC_VS_PATH_INCLUDE: return sp_fmt_io(io, "{}/VC/Tools/MSVC/{}/include", sp_fmt_str(install), sp_fmt_str(tools));
    case SP_MSVC_VS_PATH_BIN: return sp_fmt_io(io, "{}/VC/Tools/MSVC/{}/bin/Host{}/{}", sp_fmt_str(install), sp_fmt_str(tools), sp_fmt_str(host), sp_fmt_str(target));
  }
  SP_UNREACHABLE_RETURN(SP_ERR);
}

sp_str_r sp_msvc_vs_path_buf(c8* buffer, u64 len, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id) {
  sp_io_mem_writer_t io = sp_zero;
  sp_io_mem_writer_from_buffer(&io, buffer, len);

  sp_str_r result = sp_zero;
  result.err = sp_msvc_vs_path_io(&io.base, vs, id);
  if (!result.err) result.value = sp_io_mem_writer_as_str(&io);
  return result;
}

sp_str_t sp_msvc_vs_path(sp_mem_t mem, const sp_msvc_vs_t* vs, sp_msvc_vs_path_id_t id) {
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &io);
  sp_msvc_vs_path_io(&io.base, vs, id);
  return sp_io_dyn_mem_writer_as_str(&io);
}

sp_msvc_sdk_paths_t sp_msvc_sdk_render(sp_mem_t mem, const sp_msvc_sdk_t* sdk) {
  return (sp_msvc_sdk_paths_t) {
    .lib_um         = sp_msvc_sdk_path(mem, sdk, SP_MSVC_SDK_PATH_LIB_UM),
    .lib_ucrt       = sp_msvc_sdk_path(mem, sdk, SP_MSVC_SDK_PATH_LIB_UCRT),
    .include_ucrt   = sp_msvc_sdk_path(mem, sdk, SP_MSVC_SDK_PATH_INCLUDE_UCRT),
    .include_um     = sp_msvc_sdk_path(mem, sdk, SP_MSVC_SDK_PATH_INCLUDE_UM),
    .include_shared = sp_msvc_sdk_path(mem, sdk, SP_MSVC_SDK_PATH_INCLUDE_SHARED),
  };
}

sp_msvc_vs_paths_t sp_msvc_vs_render(sp_mem_t mem, const sp_msvc_vs_t* vs) {
  return (sp_msvc_vs_paths_t) {
    .lib     = sp_msvc_vs_path(mem, vs, SP_MSVC_VS_PATH_LIB),
    .include = sp_msvc_vs_path(mem, vs, SP_MSVC_VS_PATH_INCLUDE),
    .bin     = sp_msvc_vs_path(mem, vs, SP_MSVC_VS_PATH_BIN),
  };
}

#if defined(SP_WIN32)
#include <windows.h>

static void sp_msvc_add_sdk(sp_msvc_t* msvc, const sp_msvc_sdk_t* sdk) {
  u32 at = msvc->num_sdks;
  while (at && sp_msvc_version_gt(sdk->version, msvc->sdks[at - 1].version)) at--;
  if (at >= SP_MSVC_MAX_SDKS) return;

  u32 num = sp_min(msvc->num_sdks, SP_MSVC_MAX_SDKS - 1);
  sp_mem_move(&msvc->sdks[at + 1], &msvc->sdks[at], (num - at) * sizeof(sp_msvc_sdk_t));
  msvc->sdks[at] = *sdk;
  msvc->num_sdks = num + 1;
}

static void sp_msvc_add_vs(sp_msvc_t* msvc, const sp_msvc_vs_t* vs) {
  u32 at = msvc->num_installations;
  while (at && sp_msvc_version_gt(vs->version.build, msvc->installations[at - 1].version.build)) at--;
  if (at >= SP_MSVC_MAX_VS) return;

  u32 num = sp_min(msvc->num_installations, SP_MSVC_MAX_VS - 1);
  sp_mem_move(&msvc->installations[at + 1], &msvc->installations[at], (num - at) * sizeof(sp_msvc_vs_t));
  msvc->installations[at] = *vs;
  msvc->num_installations = num + 1;
}

static bool sp_msvc_read_state(sp_str_t path, sp_msvc_state_t* state) {
  sp_io_file_reader_t reader = sp_zero;
  if (sp_io_file_reader_from_path(&reader, path)) return false;

  u8 buf [4096];
  sp_io_reader_set_buffer(&reader.base, buf, sizeof(buf));
  bool ok = sp_msvc_parse_state(&reader.base, state);
  sp_io_file_reader_close(&reader);
  return ok;
}

static sp_str_t sp_msvc_read_tools_version(sp_str_t path, c8* buf, u64 len) {
  sp_io_file_reader_t reader = sp_zero;
  if (sp_io_file_reader_from_path(&reader, path)) return sp_zero_s(sp_str_t);

  u64 num_read = 0;
  sp_io_read_all(&reader.base, buf, len, &num_read);
  sp_io_file_reader_close(&reader);
  return sp_str_trim(sp_str(buf, sp_cast(u32, num_read)));
}

static sp_msvc_err_t sp_msvc_find_sdks(sp_msvc_t* msvc, sp_msvc_arch_t arch) {
  sp_msvc_path_t root = sp_zero;
  DWORD len = sizeof(root.data);
  LSTATUS rc = RegGetValueA(
    HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10",
    RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY, SP_NULLPTR, root.data, &len
  );
  if (rc == ERROR_FILE_NOT_FOUND) return SP_MSVC_ERR_SDK_NOT_FOUND;
  if (rc != ERROR_SUCCESS) return SP_MSVC_ERR_REGISTRY;

  root.len = sp_cstr_len_n(root.data, SP_MSVC_PATH_MAX);
  sp_msvc_path_normalize(&root);

  c8 buf [SP_PATH_MAX] = sp_zero;
  sp_str_t lib = sp_fmt_buf(buf, sizeof(buf), "{}/Lib", sp_fmt_str(sp_msvc_path_str(&root))).value;

  SP_ALIGNED u8 dir_buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_t dir = sp_zero;
  if (sp_fs_dir_open(&dir, sp_sys_get_root(0), lib, sp_mem_slice(dir_buf, sizeof(dir_buf)))) {
    return SP_MSVC_ERR_SDK_NOT_FOUND;
  }

  while (true) {
    sp_fs_dir_entry_t entry = sp_zero;
    if (sp_fs_dir_next(&dir, &entry)) break;
    if (!entry.name.data) break;
    if (entry.kind != SP_FS_KIND_DIR) continue;
    if (!sp_str_starts_with(entry.name, sp_str_lit("10."))) continue;

    sp_msvc_sdk_t sdk = sp_msvc_sdk_new(arch, sp_msvc_path_str(&root), entry.name);
    sp_str_r lib_ucrt = sp_msvc_sdk_path_buf(buf, sizeof(buf), &sdk, SP_MSVC_SDK_PATH_LIB_UCRT);
    if (lib_ucrt.err || !sp_fs_is_dir(lib_ucrt.value)) continue;

    sp_msvc_add_sdk(msvc, &sdk);
  }
  sp_fs_dir_close(&dir);

  if (!msvc->num_sdks) return SP_MSVC_ERR_SDK_NOT_FOUND;
  return SP_MSVC_OK;
}

static sp_msvc_arch_t sp_msvc_host_arch() {
  USHORT process = 0;
  USHORT native = 0;
  if (IsWow64Process2(GetCurrentProcess(), &process, &native) && native == IMAGE_FILE_MACHINE_ARM64) {
    return SP_MSVC_ARCH_ARM64;
  }
  return SP_MSVC_ARCH_X64;
}

static sp_msvc_err_t sp_msvc_find_installations(sp_msvc_t* msvc, sp_msvc_arch_t arch) {
  sp_str_t program_data = sp_os_env_get(sp_str_lit("ProgramData"));
  if (!sp_str_valid(program_data)) return SP_MSVC_ERR_VS_NOT_FOUND;

  sp_msvc_arch_t host = sp_msvc_host_arch();

  c8 instances_buf [SP_PATH_MAX] = sp_zero;
  sp_str_r instances = sp_fmt_buf(
    instances_buf, sizeof(instances_buf),
    "{}/Microsoft/VisualStudio/Packages/_Instances", sp_fmt_str(program_data)
  );
  if (instances.err) return SP_MSVC_ERR_VS_NOT_FOUND;

  SP_ALIGNED u8 dir_buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_t dir = sp_zero;
  if (sp_fs_dir_open(&dir, sp_sys_get_root(0), instances.value, sp_mem_slice(dir_buf, sizeof(dir_buf)))) {
    return SP_MSVC_ERR_VS_NOT_FOUND;
  }

  while (true) {
    sp_fs_dir_entry_t entry = sp_zero;
    if (sp_fs_dir_next(&dir, &entry)) break;
    if (!entry.name.data) break;
    if (entry.kind != SP_FS_KIND_DIR) continue;

    c8 buf [SP_PATH_MAX] = sp_zero;
    sp_str_r path = sp_fmt_buf(buf, sizeof(buf), "{}/{}/state.json", sp_fmt_str(instances.value), sp_fmt_str(entry.name));
    if (path.err) continue;

    sp_msvc_state_t state = sp_zero;
    if (!sp_msvc_read_state(path.value, &state)) continue;

    path = sp_fmt_buf(
      buf, sizeof(buf),
      "{}/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt",
      sp_fmt_str(sp_msvc_path_str(&state.install_path))
    );
    if (path.err) continue;

    c8 tools_buf [64] = sp_zero;
    sp_str_t tools = sp_msvc_read_tools_version(path.value, tools_buf, sizeof(tools_buf));
    if (sp_str_empty(tools)) continue;

    sp_msvc_vs_t vs = sp_msvc_vs_new(host, arch, &state, tools);

    sp_io_mem_writer_t io = sp_zero;
    sp_io_mem_writer_from_buffer(&io, buf, sizeof(buf));
    if (sp_msvc_vs_path_io(&io.base, &vs, SP_MSVC_VS_PATH_LIB)) continue;
    if (sp_io_write_str(&io.base, sp_str_lit("/vcruntime.lib"), SP_NULLPTR)) continue;
    if (!sp_fs_exists(sp_io_mem_writer_as_str(&io))) continue;

    sp_msvc_add_vs(msvc, &vs);
  }
  sp_fs_dir_close(&dir);

  if (!msvc->num_installations) return SP_MSVC_ERR_VS_NOT_FOUND;
  return SP_MSVC_OK;
}

sp_msvc_err_t sp_msvc_find_ex(sp_msvc_arch_t target, sp_msvc_t* out) {
  *out = sp_zero_s(sp_msvc_t);

  sp_msvc_err_t sdk_err = sp_msvc_find_sdks(out, target);
  sp_msvc_err_t vs_err = sp_msvc_find_installations(out, target);

  if (vs_err) return vs_err;
  return sdk_err;
}

sp_msvc_err_t sp_msvc_find(sp_msvc_t* out) {
  return sp_msvc_find_ex(sp_msvc_host_arch(), out);
}

#else

sp_msvc_err_t sp_msvc_find_ex(sp_msvc_arch_t target, sp_msvc_t* out) {
  (void)target;
  *out = sp_zero_s(sp_msvc_t);
  return SP_MSVC_ERR_UNSUPPORTED;
}

sp_msvc_err_t sp_msvc_find(sp_msvc_t* out) {
  return sp_msvc_find_ex(SP_MSVC_ARCH_X64, out);
}

#endif // SP_WIN32
#endif // SP_MSVC_IMPLEMENTATION
