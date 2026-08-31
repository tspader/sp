#define SP_IMPLEMENTATION
#include "sp.h"

#include "sp/sp_msvc.h"

s32 msvc_main(s32 argc, const c8** argv);
SP_MAIN(msvc_main)

void print_entry(const c8* label, sp_str_t value) {
  sp_log("  {:<8}: {}", sp_fmt_cstr(label), sp_fmt_str(value));
}

void print_header(const c8* label, sp_str_t version, u32 index) {
  sp_log("[{.gray}] {} {.green}", sp_fmt_uint(index), sp_fmt_cstr(label), sp_fmt_str(version));
}

s32 msvc_main(s32 argc, const c8** argv) {
  (void)argc;
  (void)argv;

  sp_msvc_t result = sp_zero;
  sp_msvc_err_t err = sp_msvc_find(&result);

  if (err) {
    sp_log("sp_msvc_find failed: {}", sp_fmt_int(err));
    return 1;
  }

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_log("{} Visual Studio installation(s)", sp_fmt_uint(result.num_installations));
  sp_for(i, result.num_installations) {
    sp_msvc_vs_t* vs = &result.installations[i];
    sp_msvc_vs_paths_t paths = sp_msvc_vs_render(mem, vs);
    print_header("Visual Studio", sp_msvc_version_str(&vs->version.product), i);
    print_entry("build", sp_msvc_version_str(&vs->version.build));
    print_entry("tools", sp_msvc_version_str(&vs->version.tools));
    print_entry("path", sp_msvc_path_str(&vs->install_path));
    print_entry("lib", paths.lib);
    print_entry("include", paths.include);
    print_entry("bin", paths.bin);
    sp_log("");
  }

  sp_log("{} Windows SDK(s)", sp_fmt_uint(result.num_sdks));
  sp_for(i, result.num_sdks) {
    sp_msvc_sdk_t* sdk = &result.sdks[i];
    sp_msvc_sdk_paths_t paths = sp_msvc_sdk_render(mem, sdk);
    print_header("SDK", sp_msvc_version_str(&sdk->version), i);
    print_entry("root", sp_msvc_path_str(&sdk->root));
    print_entry("lib_um", paths.lib_um);
    print_entry("lib_ucrt", paths.lib_ucrt);
    print_entry("inc_ucrt", paths.include_ucrt);
    print_entry("inc_um", paths.include_um);
    print_entry("inc_shared:", paths.include_shared);
    if (i != result.num_sdks - 1) sp_log("");
  }

  return 0;
}
