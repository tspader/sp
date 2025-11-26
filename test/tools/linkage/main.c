#define SP_IMPLEMENTATION
#include "sp.h"

s32 main() {
  if (sp_tm_now_epoch().s == 0) {
    SP_EXIT_FAILURE();
  }

  SP_EXIT_SUCCESS();
}
