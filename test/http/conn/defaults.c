#include "conn.h"

sp_test(conn, defaults) {
  sp_http_conn_t conn = sp_zero;
  sp_http_conn_init(&conn, (sp_http_conn_desc_t) { .mem = sp_test_arena(t) });
  sp_expect_eq(t, conn.desc.head_max, 16u * 1024);
  sp_expect_eq(t, conn.desc.body_max, 64u * 1024);
  sp_expect_eq(t, conn.desc.stream_max, 64u * 1024);
  sp_http_conn_deinit(&conn);
  return SP_OK;
}
