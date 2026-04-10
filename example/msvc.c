#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_MSVC_IMPLEMENTATION
#include "sp/sp_msvc.h"

s32 msvc_main(s32 argc, const c8** argv);
SP_ENTRY(msvc_main)

s32 msvc_main(s32 argc, const c8** argv) {
  (void)argc;
  (void)argv;

  sp_msvc_t result = SP_ZERO_INITIALIZE();
  sp_msvc_err_t err = sp_msvc_find(SP_MSVC_ARCH_X64, &result);

  if (err) {
    sp_log("sp_msvc_find failed: {}", SP_FMT_S32(err));
    return 1;
  }

  sp_log("{:fg cyan} Visual Studio installation(s)", SP_FMT_U32(sp_da_size(result.installations)));
  sp_da_for(result.installations, i) {
    sp_msvc_vs_t* vs = &result.installations[i];
    sp_log("");
    sp_log("  [{:fg yellow}] VS {:fg green}", SP_FMT_U32(i), SP_FMT_STR(vs->version.product));
    sp_log("      build:   {}", SP_FMT_STR(vs->version.build.str));
    sp_log("      tools:   {}", SP_FMT_STR(vs->version.tools.str));
    sp_log("      path:    {}", SP_FMT_STR(vs->install_path));
    sp_log("      lib:     {}", SP_FMT_STR(vs->lib));
    sp_log("      include: {}", SP_FMT_STR(vs->include));
    sp_log("      bin:     {}", SP_FMT_STR(vs->bin));
  }

  sp_log("");
  sp_log("{:fg cyan} Windows SDK(s)", SP_FMT_U32(sp_da_size(result.sdks)));
  sp_da_for(result.sdks, i) {
    sp_msvc_sdk_t* sdk = &result.sdks[i];
    sp_log("");
    sp_log("  [{:fg yellow}] SDK {:fg green}", SP_FMT_U32(i), SP_FMT_STR(sdk->version.str));
    sp_log("      root:       {}", SP_FMT_STR(sdk->root));
    sp_log("      lib_um:     {}", SP_FMT_STR(sdk->lib_um));
    sp_log("      lib_ucrt:   {}", SP_FMT_STR(sdk->lib_ucrt));
    sp_log("      inc_ucrt:   {}", SP_FMT_STR(sdk->include_ucrt));
    sp_log("      inc_um:     {}", SP_FMT_STR(sdk->include_um));
    sp_log("      inc_shared: {}", SP_FMT_STR(sdk->include_shared));
  }

  return 0;
}
