#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_MSVC_IMPLEMENTATION
#include "sp/msvc.h"

s32 msvc_main(s32 argc, const c8** argv);
SP_ENTRY(msvc_main)

s32 msvc_main(s32 argc, const c8** argv) {
  (void)argc;
  (void)argv;

  sp_msvc_t result = SP_ZERO_INITIALIZE();
  sp_msvc_err_t err = sp_msvc_find(SP_MSVC_ARCH_X64, &result);

  if (err) {
    SP_LOG("sp_msvc_find failed: {}", SP_FMT_S32(err));
    return 1;
  }

  SP_LOG("{:fg cyan} Visual Studio installation(s)", SP_FMT_U32(sp_da_size(result.installations)));
  sp_da_for(result.installations, i) {
    sp_msvc_vs_t* vs = &result.installations[i];
    SP_LOG("");
    SP_LOG("  [{:fg yellow}] VS {:fg green}", SP_FMT_U32(i), SP_FMT_STR(vs->version.product));
    SP_LOG("      build:   {}", SP_FMT_STR(vs->version.build.str));
    SP_LOG("      tools:   {}", SP_FMT_STR(vs->version.tools.str));
    SP_LOG("      path:    {}", SP_FMT_STR(vs->install_path));
    SP_LOG("      lib:     {}", SP_FMT_STR(vs->lib));
    SP_LOG("      include: {}", SP_FMT_STR(vs->include));
    SP_LOG("      bin:     {}", SP_FMT_STR(vs->bin));
  }

  SP_LOG("");
  SP_LOG("{:fg cyan} Windows SDK(s)", SP_FMT_U32(sp_da_size(result.sdks)));
  sp_da_for(result.sdks, i) {
    sp_msvc_sdk_t* sdk = &result.sdks[i];
    SP_LOG("");
    SP_LOG("  [{:fg yellow}] SDK {:fg green}", SP_FMT_U32(i), SP_FMT_STR(sdk->version.str));
    SP_LOG("      root:       {}", SP_FMT_STR(sdk->root));
    SP_LOG("      lib_um:     {}", SP_FMT_STR(sdk->lib_um));
    SP_LOG("      lib_ucrt:   {}", SP_FMT_STR(sdk->lib_ucrt));
    SP_LOG("      inc_ucrt:   {}", SP_FMT_STR(sdk->include_ucrt));
    SP_LOG("      inc_um:     {}", SP_FMT_STR(sdk->include_um));
    SP_LOG("      inc_shared: {}", SP_FMT_STR(sdk->include_shared));
  }

  return 0;
}
