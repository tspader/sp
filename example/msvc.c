#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_MSVC_IMPLEMENTATION
#include "sp/sp_msvc.h"

s32 msvc_main(s32 argc, const c8** argv);
SP_MAIN(msvc_main)

s32 msvc_main(s32 argc, const c8** argv) {
  (void)argc;
  (void)argv;

  sp_msvc_t result = SP_ZERO_INITIALIZE();
  sp_msvc_err_t err = sp_msvc_find(SP_MSVC_ARCH_X64, &result);

  if (err) {
    sp_log_a("sp_msvc_find failed: {}", sp_fmt_int(err));
    return 1;
  }

  sp_log_a("{.fg cyan} Visual Studio installation(s)", sp_fmt_uint(sp_da_size(result.installations)));
  sp_da_for(result.installations, i) {
    sp_msvc_vs_t* vs = &result.installations[i];
    sp_log_a("");
    sp_log_a("  [{.fg yellow}] VS {.fg green}", sp_fmt_uint(i), sp_fmt_str(vs->version.product));
    sp_log_a("      build:   {}", sp_fmt_str(vs->version.build.str));
    sp_log_a("      tools:   {}", sp_fmt_str(vs->version.tools.str));
    sp_log_a("      path:    {}", sp_fmt_str(vs->install_path));
    sp_log_a("      lib:     {}", sp_fmt_str(vs->lib));
    sp_log_a("      include: {}", sp_fmt_str(vs->include));
    sp_log_a("      bin:     {}", sp_fmt_str(vs->bin));
  }

  sp_log_a("");
  sp_log_a("{.fg cyan} Windows SDK(s)", sp_fmt_uint(sp_da_size(result.sdks)));
  sp_da_for(result.sdks, i) {
    sp_msvc_sdk_t* sdk = &result.sdks[i];
    sp_log_a("");
    sp_log_a("  [{.fg yellow}] SDK {.fg green}", sp_fmt_uint(i), sp_fmt_str(sdk->version.str));
    sp_log_a("      root:       {}", sp_fmt_str(sdk->root));
    sp_log_a("      lib_um:     {}", sp_fmt_str(sdk->lib_um));
    sp_log_a("      lib_ucrt:   {}", sp_fmt_str(sdk->lib_ucrt));
    sp_log_a("      inc_ucrt:   {}", sp_fmt_str(sdk->include_ucrt));
    sp_log_a("      inc_um:     {}", sp_fmt_str(sdk->include_um));
    sp_log_a("      inc_shared: {}", sp_fmt_str(sdk->include_shared));
  }

  return 0;
}
