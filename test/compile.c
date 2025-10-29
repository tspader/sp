#define SP_IMPLEMENTATION
#include "sp.h"

#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"
#define STB_INCLUDE_IMPLEMENTATION
#include "stb/stb_include.h"

s32 main(s32 num_args, const c8** args) {
  SP_LOG("hello, {:fg brightcyan}", SP_FMT_CSTR("world"));
  return 0;
}
