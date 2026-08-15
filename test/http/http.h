#ifndef HTTP_TEST_H
#define HTTP_TEST_H

// The suite links against private sp_http functions across translation units,
// so they must have external linkage; the implementation TU (fetch.c) defines
// SP_PRIVATE empty for the same reason.
#define SP_PRIVATE
#define SP_IMP
#define SP_HTTP_EVERYTHING_PUBLIC
#include "sp/sp_http.h"
#include "sp/sp_test.h"

#endif
