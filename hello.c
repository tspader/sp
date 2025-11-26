#define SP_IMPLEMENTATION
#include "sp.h"

int main(void) {
    SP_LOG("hello {:fg brightcyan}", SP_FMT_CSTR("world"));
    return 0;
}
