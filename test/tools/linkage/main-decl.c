#include "sp.h"

extern sp_rt_t* get_sp_rt_ptr();

s32 main() {
  if (get_sp_rt_ptr() != &sp_rt) {
    SP_EXIT_FAILURE();
  }

  if (sp_tm_now_epoch().s == 0) {
    SP_EXIT_FAILURE();
  }

  SP_EXIT_SUCCESS();
}
