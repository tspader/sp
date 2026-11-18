// The self-test instrumentation in ecp.c increments plain global counters on
// every point multiplication, which is a data race the moment two threads do
// ECC at once.
#if defined(MBEDTLS_THREADING_C)
  #undef MBEDTLS_SELF_TEST
#endif
