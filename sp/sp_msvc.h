#ifndef SP_MSVC_H
#define SP_MSVC_H

#include "sp.h"

typedef enum {
  SP_MSVC_ARCH_X64,
  SP_MSVC_ARCH_ARM64,
} sp_msvc_arch_t;

typedef enum {
  SP_MSVC_OK,
  SP_MSVC_ERR_SDK_NOT_FOUND,
  SP_MSVC_ERR_VS_NOT_FOUND,
  SP_MSVC_ERR_REGISTRY,
} sp_msvc_err_t;

typedef struct {
  sp_str_t str;
  u32 major;
  u32 minor;
  u32 build;
  u32 revision;
} sp_msvc_version_t;

typedef struct {
  sp_msvc_version_t version;
  sp_str_t root;
  sp_str_t lib_um;
  sp_str_t lib_ucrt;
  sp_str_t include_ucrt;
  sp_str_t include_um;
  sp_str_t include_shared;
} sp_msvc_sdk_t;

typedef struct {
  struct {
    sp_str_t product;
    sp_msvc_version_t build;
    sp_msvc_version_t tools;
  } version;
  sp_str_t install_path;
  sp_str_t lib;
  sp_str_t include;
  sp_str_t bin;
} sp_msvc_vs_t;

typedef struct {
  sp_mem_arena_t* arena;
  sp_mem_t mem;
  sp_da(sp_msvc_sdk_t) sdks;
  sp_da(sp_msvc_vs_t) installations;
} sp_msvc_t;

#if defined(SP_WIN32)
SP_API sp_msvc_err_t sp_msvc_find(sp_mem_t mem, sp_msvc_arch_t arch, sp_msvc_t* out);
SP_API void          sp_msvc_free(sp_msvc_t* msvc);
#endif
#endif // SP_MSVC_H

#if defined(SP_IMPLEMENTATION) && !defined(SP_MSVC_IMPLEMENTATION)
  #define SP_MSVC_IMPLEMENTATION
#endif

#ifndef SP_MSVC_IMPL_H
#if defined(SP_PRIVATE_HEADER) || defined(SP_MSVC_IMPLEMENTATION)
#define SP_MSVC_IMPL_H

typedef struct {
  sp_str_t install_path;
  sp_str_t build_version;
  sp_str_t product_line;
} sp_msvc_state_t;

SP_PRIVATE sp_msvc_version_t sp_msvc_parse_version(sp_mem_t mem, sp_str_t str);
SP_PRIVATE bool              sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b);
SP_PRIVATE bool              sp_msvc_parse_state(sp_mem_t mem, sp_str_t json, sp_msvc_state_t* out);
SP_PRIVATE sp_msvc_sdk_t     sp_msvc_sdk_new(sp_mem_t mem, sp_msvc_arch_t arch, sp_str_t root, sp_str_t version);
SP_PRIVATE sp_msvc_vs_t      sp_msvc_vs_new(sp_mem_t mem, sp_msvc_arch_t arch, sp_msvc_state_t state, sp_str_t tools_version);
#endif
#endif // SP_MSVC_IMPL_H

#if defined(SP_MSVC_IMPLEMENTATION) && !defined(SP_MSVC_IMPLEMENTED)
#define SP_MSVC_IMPLEMENTED

static sp_str_t sp_msvc_arch_name(sp_msvc_arch_t arch) {
  switch (arch) {
    case SP_MSVC_ARCH_X64:   { return sp_str_lit("x64"); }
    case SP_MSVC_ARCH_ARM64: { return sp_str_lit("arm64"); }
  }
  SP_UNREACHABLE_RETURN(sp_str_lit("x64"));
}

static sp_str_t sp_msvc_json_get_str(sp_mem_t mem, sp_str_t json, sp_str_t key) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(mem);
  sp_str_t needle = sp_fmt(scratch.mem, "\"{}\":\"", sp_fmt_str(key)).value;
  s32 pos = sp_str_find(json, needle);
  u32 start = (u32)pos + needle.len;
  sp_mem_end_scratch(scratch);
  if (pos == SP_STR_NO_MATCH) return sp_zero_s(sp_str_t);

  sp_io_dyn_mem_writer_t value = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &value);
  sp_for_range(it, start, json.len) {
    c8 c = json.data[it];
    if (c == '"') break;
    if (c == '\\' && it + 1 < json.len) {
      it++;
      c = json.data[it];
    }
    sp_io_write_c8(&value.base, c);
  }
  return sp_io_dyn_mem_writer_as_str(&value);
}

SP_PRIVATE sp_msvc_version_t sp_msvc_parse_version(sp_mem_t mem, sp_str_t str) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(mem);
  u32 parts [4] = sp_zero;
  sp_da(sp_str_t) split = sp_str_split_c8(scratch.mem, str, '.');
  sp_da_for(split, it) {
    if (it >= sp_carr_len(parts)) break;
    parts[it] = sp_parse_u32(split[it]);
  }
  sp_mem_end_scratch(scratch);

  return (sp_msvc_version_t) {
    .str      = sp_str_copy(mem, str),
    .major    = parts[0],
    .minor    = parts[1],
    .build    = parts[2],
    .revision = parts[3],
  };
}

SP_PRIVATE bool sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b) {
  if (a.major != b.major) return a.major > b.major;
  if (a.minor != b.minor) return a.minor > b.minor;
  if (a.build != b.build) return a.build > b.build;
  return a.revision > b.revision;
}

SP_PRIVATE bool sp_msvc_parse_state(sp_mem_t mem, sp_str_t json, sp_msvc_state_t* out) {
  *out = sp_zero_s(sp_msvc_state_t);

  sp_str_t install = sp_msvc_json_get_str(mem, json, sp_str_lit("installationPath"));
  sp_str_t build = sp_msvc_json_get_str(mem, json, sp_str_lit("buildVersion"));
  if (sp_str_empty(install) || sp_str_empty(build)) return false;

  out->install_path = sp_fs_normalize_path(mem, install);
  out->build_version = build;
  out->product_line = sp_msvc_json_get_str(mem, json, sp_str_lit("productLineVersion"));
  return true;
}

SP_PRIVATE sp_msvc_sdk_t sp_msvc_sdk_new(sp_mem_t mem, sp_msvc_arch_t arch, sp_str_t root, sp_str_t version) {
  sp_str_t arch_name = sp_msvc_arch_name(arch);

  return (sp_msvc_sdk_t) {
    .version        = sp_msvc_parse_version(mem, version),
    .root           = sp_str_copy(mem, root),
    .lib_um         = sp_fmt(mem, "{}/Lib/{}/um/{}", sp_fmt_str(root), sp_fmt_str(version), sp_fmt_str(arch_name)).value,
    .lib_ucrt       = sp_fmt(mem, "{}/Lib/{}/ucrt/{}", sp_fmt_str(root), sp_fmt_str(version), sp_fmt_str(arch_name)).value,
    .include_ucrt   = sp_fmt(mem, "{}/Include/{}/ucrt", sp_fmt_str(root), sp_fmt_str(version)).value,
    .include_um     = sp_fmt(mem, "{}/Include/{}/um", sp_fmt_str(root), sp_fmt_str(version)).value,
    .include_shared = sp_fmt(mem, "{}/Include/{}/shared", sp_fmt_str(root), sp_fmt_str(version)).value,
  };
}

SP_PRIVATE sp_msvc_vs_t sp_msvc_vs_new(sp_mem_t mem, sp_msvc_arch_t arch, sp_msvc_state_t state, sp_str_t tools_version) {
  sp_str_t arch_name = sp_msvc_arch_name(arch);
  sp_str_t tools = sp_fmt(mem, "{}/VC/Tools/MSVC/{}", sp_fmt_str(state.install_path), sp_fmt_str(tools_version)).value;

  return (sp_msvc_vs_t) {
    .version = {
      .product = sp_str_copy(mem, state.product_line),
      .build   = sp_msvc_parse_version(mem, state.build_version),
      .tools   = sp_msvc_parse_version(mem, tools_version),
    },
    .install_path = sp_str_copy(mem, state.install_path),
    .lib          = sp_fmt(mem, "{}/Lib/{}", sp_fmt_str(tools), sp_fmt_str(arch_name)).value,
    .include      = sp_fmt(mem, "{}/include", sp_fmt_str(tools)).value,
    .bin          = sp_fmt(mem, "{}/bin/Hostx64/{}", sp_fmt_str(tools), sp_fmt_str(arch_name)).value,
  };
}

#if defined(SP_WIN32)
#include <windows.h>

static s32 sp_msvc_sdk_order(const void* a, const void* b) {
  const sp_msvc_sdk_t* sa = (const sp_msvc_sdk_t*)a;
  const sp_msvc_sdk_t* sb = (const sp_msvc_sdk_t*)b;
  if (sp_msvc_version_gt(sa->version, sb->version)) return -1;
  if (sp_msvc_version_gt(sb->version, sa->version)) return 1;
  return 0;
}

static s32 sp_msvc_vs_order(const void* a, const void* b) {
  const sp_msvc_vs_t* va = (const sp_msvc_vs_t*)a;
  const sp_msvc_vs_t* vb = (const sp_msvc_vs_t*)b;
  if (sp_msvc_version_gt(va->version.build, vb->version.build)) return -1;
  if (sp_msvc_version_gt(vb->version.build, va->version.build)) return 1;
  return 0;
}

static sp_msvc_err_t sp_msvc_find_sdks(sp_msvc_t* msvc, sp_msvc_arch_t arch) {
  c8 buf [SP_PATH_MAX] = sp_zero;
  DWORD len = sizeof(buf);
  LSTATUS rc = RegGetValueA(
    HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10",
    RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY, SP_NULLPTR, buf, &len
  );
  if (rc == ERROR_FILE_NOT_FOUND) return SP_MSVC_ERR_SDK_NOT_FOUND;
  if (rc != ERROR_SUCCESS) return SP_MSVC_ERR_REGISTRY;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(msvc->mem);
  sp_str_t root = sp_fs_normalize_path(scratch.mem, sp_cstr_as_str(buf));

  sp_da(sp_fs_entry_t) entries = sp_zero;
  sp_fs_collect(scratch.mem, sp_fs_join_path(scratch.mem, root, sp_str_lit("Lib")), &entries);

  sp_da_for(entries, it) {
    sp_fs_entry_t* entry = &entries[it];
    if (entry->kind != SP_FS_KIND_DIR) continue;
    if (!sp_str_starts_with(entry->name, sp_str_lit("10."))) continue;

    sp_msvc_sdk_t sdk = sp_msvc_sdk_new(msvc->mem, arch, root, entry->name);
    if (!sp_fs_is_dir(sdk.lib_ucrt)) continue;

    sp_da_push(msvc->sdks, sdk);
  }
  sp_mem_end_scratch(scratch);

  if (sp_da_empty(msvc->sdks)) return SP_MSVC_ERR_SDK_NOT_FOUND;
  sp_da_sort(msvc->sdks, sp_msvc_sdk_order);
  return SP_MSVC_OK;
}

static sp_msvc_err_t sp_msvc_find_installations(sp_msvc_t* msvc, sp_msvc_arch_t arch) {
  sp_str_t program_data = sp_os_env_get(sp_str_lit("ProgramData"));
  if (!sp_str_valid(program_data)) return SP_MSVC_ERR_VS_NOT_FOUND;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(msvc->mem);

  sp_da(sp_fs_entry_t) entries = sp_zero;
  sp_fs_collect(
    scratch.mem,
    sp_fs_join_path(scratch.mem, program_data, sp_str_lit("Microsoft/VisualStudio/Packages/_Instances")),
    &entries
  );

  sp_da_for(entries, it) {
    sp_fs_entry_t* entry = &entries[it];
    if (entry->kind != SP_FS_KIND_DIR) continue;

    sp_str_t json = sp_zero;
    sp_io_read_file(scratch.mem, sp_fs_join_path(scratch.mem, entry->path, sp_str_lit("state.json")), &json);

    sp_msvc_state_t state = sp_zero;
    if (!sp_msvc_parse_state(scratch.mem, json, &state)) continue;

    sp_str_t tools = sp_zero;
    sp_io_read_file(
      scratch.mem,
      sp_fs_join_path(scratch.mem, state.install_path, sp_str_lit("VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")),
      &tools
    );
    tools = sp_str_trim(tools);
    if (sp_str_empty(tools)) continue;

    sp_msvc_vs_t vs = sp_msvc_vs_new(msvc->mem, arch, state, tools);
    if (!sp_fs_exists(sp_fs_join_path(scratch.mem, vs.lib, sp_str_lit("vcruntime.lib")))) continue;

    sp_da_push(msvc->installations, vs);
  }
  sp_mem_end_scratch(scratch);

  if (sp_da_empty(msvc->installations)) return SP_MSVC_ERR_VS_NOT_FOUND;
  sp_da_sort(msvc->installations, sp_msvc_vs_order);
  return SP_MSVC_OK;
}

sp_msvc_err_t sp_msvc_find(sp_mem_t mem, sp_msvc_arch_t arch, sp_msvc_t* out) {
  *out = sp_zero_s(sp_msvc_t);
  out->arena = sp_mem_arena_new(mem);
  out->mem = sp_mem_arena_as_allocator(out->arena);
  sp_da_init(out->mem, out->sdks);
  sp_da_init(out->mem, out->installations);

  sp_msvc_err_t sdk_err = sp_msvc_find_sdks(out, arch);
  sp_msvc_err_t vs_err = sp_msvc_find_installations(out, arch);

  if (vs_err) return vs_err;
  return sdk_err;
}

void sp_msvc_free(sp_msvc_t* msvc) {
  sp_mem_arena_destroy(msvc->arena);
}

#endif // SP_WIN32
#endif // SP_MSVC_IMPLEMENTATION
