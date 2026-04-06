#ifndef SP_MSVC_H
#define SP_MSVC_H

#include "sp.h"

#if !defined(SP_WIN32)
#error "msvc.h is Windows only"
#endif

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
  sp_da(sp_msvc_sdk_t) sdks;
  sp_da(sp_msvc_vs_t) installations;
} sp_msvc_t;

SP_API sp_msvc_err_t sp_msvc_find(sp_msvc_arch_t arch, sp_msvc_t* out);
#endif // SP_MSVC_H

#if defined(SP_MSVC_IMPLEMENTATION)
#include <windows.h>

SP_PRIVATE sp_msvc_err_t      sp_msvc_find_sdks(sp_msvc_arch_t arch, sp_da(sp_msvc_sdk_t)* out);
SP_PRIVATE sp_msvc_err_t      sp_msvc_find_installations(sp_msvc_arch_t arch, sp_da(sp_msvc_vs_t)* out);
SP_PRIVATE sp_str_t           sp_msvc_arch_str(sp_msvc_arch_t arch);
SP_PRIVATE sp_str_t           sp_msvc_bin_subdir(sp_msvc_arch_t arch);
SP_PRIVATE sp_msvc_version_t  sp_msvc_parse_version(sp_str_t str);
SP_PRIVATE bool               sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b);
SP_PRIVATE sp_str_t           sp_msvc_json_get_str(sp_str_t json, sp_str_t key);

sp_str_t sp_msvc_arch_str(sp_msvc_arch_t arch) {
  switch (arch) {
    case SP_MSVC_ARCH_X64:   { return SP_LIT("x64"); }
    case SP_MSVC_ARCH_ARM64: { return SP_LIT("arm64"); }
  }
  SP_UNREACHABLE_RETURN(SP_LIT("x64"));
}

sp_str_t sp_msvc_bin_subdir(sp_msvc_arch_t arch) {
  switch (arch) {
    case SP_MSVC_ARCH_X64:   { return SP_LIT("Hostx64/x64"); }
    case SP_MSVC_ARCH_ARM64: { return SP_LIT("Hostx64/arm64"); }
  }
  SP_UNREACHABLE_RETURN(SP_LIT("Hostx64/x64"));
}

sp_msvc_version_t sp_msvc_parse_version(sp_str_t str) {
  sp_msvc_version_t v = SP_ZERO_INITIALIZE();
  v.str = sp_str_copy(str);

  u32 parts[4] = SP_ZERO_INITIALIZE();
  u32 part = 0;
  u32 accum = 0;

  sp_for(i, str.len) {
    c8 c = str.data[i];
    if (c == '.') {
      if (part < 4) parts[part] = accum;
      part++;
      accum = 0;
    } else if (c >= '0' && c <= '9') {
      accum = accum * 10 + (u32)(c - '0');
    }
  }
  if (part < 4) parts[part] = accum;

  v.major    = parts[0];
  v.minor    = parts[1];
  v.build    = parts[2];
  v.revision = parts[3];
  return v;
}

bool sp_msvc_version_gt(sp_msvc_version_t a, sp_msvc_version_t b) {
  if (a.major != b.major) return a.major > b.major;
  if (a.minor != b.minor) return a.minor > b.minor;
  if (a.build != b.build) return a.build > b.build;
  return a.revision > b.revision;
}

sp_str_t sp_msvc_json_get_str(sp_str_t json, sp_str_t key) {
  sp_mem_scratch_t scratch = sp_mem_begin_scratch();
  sp_str_t needle = sp_format("\"{}\":\"", SP_FMT_STR(key));

  s32 pos = sp_str_find(json, needle);
  if (pos == SP_STR_NO_MATCH) {
    sp_mem_end_scratch(scratch);
    return SP_ZERO_STRUCT(sp_str_t);
  }

  u32 value_start = (u32)pos + needle.len;
  u32 value_end = value_start;
  while (value_end < json.len && json.data[value_end] != '"') {
    if (json.data[value_end] == '\\' && value_end + 1 < json.len) {
      value_end++;
    }
    value_end++;
  }

  sp_str_t raw = sp_str_sub(json, (s32)value_start, (s32)(value_end - value_start));

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_for(i, raw.len) {
    if (raw.data[i] == '\\' && i + 1 < raw.len && raw.data[i + 1] == '\\') {
      sp_str_builder_append_c8(&builder, '\\');
      i++;
    } else {
      sp_str_builder_append_c8(&builder, raw.data[i]);
    }
  }

  sp_context_push_allocator(scratch.old_allocator);
  sp_str_t result = sp_str_builder_to_str(&builder);
  sp_context_pop();

  sp_mem_end_scratch(scratch);
  return result;
}

sp_msvc_err_t sp_msvc_find_sdks(sp_msvc_arch_t arch, sp_da(sp_msvc_sdk_t)* out) {
  HKEY roots_key;
  LSTATUS rc = RegOpenKeyExA(
    HKEY_LOCAL_MACHINE,
    "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
    0, KEY_QUERY_VALUE | KEY_WOW64_32KEY | KEY_ENUMERATE_SUB_KEYS,
    &roots_key
  );
  if (rc != ERROR_SUCCESS) return SP_MSVC_ERR_REGISTRY;

  c8 root_buf[SP_PATH_MAX] = SP_ZERO_INITIALIZE();
  DWORD root_len = SP_PATH_MAX;
  DWORD type;
  rc = RegQueryValueExA(roots_key, "KitsRoot10", SP_NULLPTR, &type, (LPBYTE)root_buf, &root_len);
  RegCloseKey(roots_key);

  if (rc != ERROR_SUCCESS || type != REG_SZ) return SP_MSVC_ERR_SDK_NOT_FOUND;

  sp_str_t sdk_root = sp_fs_normalize_path(sp_str_view(root_buf));
  sp_str_t lib_dir = sp_fs_join_path(sdk_root, SP_LIT("Lib"));
  sp_str_t arch_str = sp_msvc_arch_str(arch);

  for (sp_fs_it_t it = sp_fs_it_new(lib_dir); sp_fs_it_valid(&it); sp_fs_it_next(&it)) {
    if (!(it.entry.attributes == SP_FS_KIND_DIR)) continue;
    sp_str_t name = it.entry.file_name;
    if (!sp_str_starts_with(name, SP_LIT("10."))) continue;

    sp_str_t ucrt_lib = sp_fs_join_path(
      sp_fs_join_path(it.entry.file_path, SP_LIT("ucrt")), arch_str
    );
    if (!sp_fs_is_dir(ucrt_lib)) continue;

    sp_str_t ver_root = it.entry.file_path;
    sp_msvc_sdk_t sdk = {
      .version     = sp_msvc_parse_version(name),
      .root        = sp_str_copy(sdk_root),
      .lib_um      = sp_fs_join_path(sp_fs_join_path(ver_root, SP_LIT("um")), arch_str),
      .lib_ucrt    = sp_str_copy(ucrt_lib),
      .include_ucrt   = sp_fs_join_path(
        sp_fs_join_path(sp_fs_join_path(sdk_root, SP_LIT("Include")), name), SP_LIT("ucrt")
      ),
      .include_um     = sp_fs_join_path(
        sp_fs_join_path(sp_fs_join_path(sdk_root, SP_LIT("Include")), name), SP_LIT("um")
      ),
      .include_shared = sp_fs_join_path(
        sp_fs_join_path(sp_fs_join_path(sdk_root, SP_LIT("Include")), name), SP_LIT("shared")
      ),
    };
    sp_da_push(*out, sdk);
  }

  sp_da_for(*out, i) {
    sp_for_range(j, i + 1, sp_da_size(*out)) {
      if (sp_msvc_version_gt((*out)[j].version, (*out)[i].version)) {
        sp_msvc_sdk_t tmp = (*out)[i];
        (*out)[i] = (*out)[j];
        (*out)[j] = tmp;
      }
    }
  }

  if (sp_da_empty(*out)) return SP_MSVC_ERR_SDK_NOT_FOUND;
  return SP_MSVC_OK;
}

sp_msvc_err_t sp_msvc_find_installations(sp_msvc_arch_t arch, sp_da(sp_msvc_vs_t)* out) {
  sp_str_t program_data = sp_os_env_get(SP_LIT("ProgramData"));
  if (!sp_str_valid(program_data)) return SP_MSVC_ERR_VS_NOT_FOUND;

  sp_str_t instances_dir = sp_fs_join_path(
    program_data, SP_LIT("Microsoft/VisualStudio/Packages/_Instances")
  );
  if (!sp_fs_is_dir(instances_dir)) return SP_MSVC_ERR_VS_NOT_FOUND;

  sp_str_t arch_str = sp_msvc_arch_str(arch);

  for (sp_fs_it_t it = sp_fs_it_new(instances_dir); sp_fs_it_valid(&it); sp_fs_it_next(&it)) {
    if (!(it.entry.attributes == SP_FS_KIND_DIR)) continue;

    sp_str_t state_path = sp_fs_join_path(it.entry.file_path, SP_LIT("state.json"));
    if (!sp_fs_exists(state_path)) continue;

    sp_str_t json = sp_io_read_file(state_path);
    if (sp_str_empty(json)) continue;

    sp_str_t install_path = sp_msvc_json_get_str(json, SP_LIT("installationPath"));
    if (sp_str_empty(install_path)) continue;

    install_path = sp_fs_normalize_path(install_path);

    sp_str_t build_version_str = sp_msvc_json_get_str(json, SP_LIT("buildVersion"));
    if (sp_str_empty(build_version_str)) continue;

    sp_str_t product_line = sp_msvc_json_get_str(json, SP_LIT("productLineVersion"));

    sp_str_t tools_file = sp_fs_join_path(
      install_path, SP_LIT("VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")
    );
    sp_str_t tools_version_str = sp_io_read_file(tools_file);
    if (sp_str_empty(tools_version_str)) continue;

    tools_version_str = sp_str_trim(tools_version_str);

    sp_str_t tools_base = sp_fs_join_path(
      sp_fs_join_path(install_path, SP_LIT("VC/Tools/MSVC")), tools_version_str
    );

    sp_str_t lib_dir = sp_fs_join_path(
      sp_fs_join_path(tools_base, SP_LIT("Lib")), arch_str
    );

    sp_str_t vcruntime = sp_fs_join_path(lib_dir, SP_LIT("vcruntime.lib"));
    if (!sp_fs_exists(vcruntime)) continue;

    sp_msvc_vs_t vs = {
      .version = {
        .product = sp_str_copy(product_line),
        .build   = sp_msvc_parse_version(build_version_str),
        .tools   = sp_msvc_parse_version(tools_version_str),
      },
      .install_path = sp_str_copy(install_path),
      .lib          = sp_str_copy(lib_dir),
      .include      = sp_fs_join_path(tools_base, SP_LIT("include")),
      .bin          = sp_fs_join_path(
        sp_fs_join_path(tools_base, SP_LIT("bin")), sp_msvc_bin_subdir(arch)
      ),
    };
    sp_da_push(*out, vs);
  }

  sp_da_for(*out, i) {
    sp_for_range(j, i + 1, sp_da_size(*out)) {
      if (sp_msvc_version_gt((*out)[j].version.build, (*out)[i].version.build)) {
        sp_msvc_vs_t tmp = (*out)[i];
        (*out)[i] = (*out)[j];
        (*out)[j] = tmp;
      }
    }
  }

  if (sp_da_empty(*out)) return SP_MSVC_ERR_VS_NOT_FOUND;
  return SP_MSVC_OK;
}

sp_msvc_err_t sp_msvc_find(sp_msvc_arch_t arch, sp_msvc_t* out) {
  *out = SP_ZERO_STRUCT(sp_msvc_t);

  sp_msvc_err_t sdk_err = sp_msvc_find_sdks(arch, &out->sdks);
  sp_msvc_err_t vs_err = sp_msvc_find_installations(arch, &out->installations);

  if (vs_err) return vs_err;
  if (sdk_err) return sdk_err;
  return SP_MSVC_OK;
}

#endif // SP_MSVC_IMPLEMENTATION
