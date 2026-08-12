#ifndef TLS_TEST_H
#define TLS_TEST_H

// The suite links against private sp_tls functions across translation units,
// so they must have external linkage; the implementation TU (fetch.c) defines
// SP_PRIVATE empty for the same reason.
#define SP_PRIVATE
#define SP_IMP
#define SP_TLS_EVERYTHING_PUBLIC
#include "sp/sp_tls.h"
#include "sp/sp_test.h"

#endif
