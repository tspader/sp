#ifndef HTTP_CLIENT_TEST_H
#define HTTP_CLIENT_TEST_H

#include "../http.h"

#define CLIENT_TEST_CAPTURE 8192
#define CLIENT_TEST_WAIT_MS 3000

typedef struct {
  u16             port;
  const c8*       send;
  c8              captured [CLIENT_TEST_CAPTURE];
  u32             captured_len;
  bool            eof;
  sp_atomic_s32_t done;
} client_t;

typedef struct {
  const c8* text;
  u32       n;
} counted_t;

static s32 client_main(void* user_data) {
  client_t* client = sp_cast(client_t*, user_data);
  sp_sys_socket_t socket = SP_SYS_INVALID_SOCKET;
  if (sp_sys_socket_open(&socket, sp_zero_s(sp_sys_handle_desc_t)) != SP_OK) goto done;
  if (sp_sys_socket_connect(socket, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 }, .port = client->port }) != SP_OK) goto close;

  sp_str_t send = sp_cstr_as_str(client->send);
  u32 sent = 0;
  while (sent < send.len) {
    u64 n = 0;
    if (sp_sys_socket_send(socket, send.data + sent, send.len - sent, &n) != SP_OK) goto close;
    sent += (u32)n;
  }

  while (client->captured_len < CLIENT_TEST_CAPTURE) {
    if (sp_sys_socket_wait(socket, true, CLIENT_TEST_WAIT_MS) != SP_OK) break;
    u64 n = 0;
    if (sp_sys_socket_recv(socket, client->captured + client->captured_len, CLIENT_TEST_CAPTURE - client->captured_len, &n) != SP_OK) break;
    if (n == 0) {
      client->eof = true;
      break;
    }
    client->captured_len += (u32)n;
  }

close:
  sp_sys_socket_close(socket);
done:
  sp_atomic_s32_store(&client->done, 1, SP_ATOMIC_RELEASE);
  return 0;
}

static u32 count_matches(sp_str_t haystack, sp_str_t needle) {
  if (sp_str_empty(needle) || needle.len > haystack.len) return 0;
  u32 count = 0;
  sp_for_range(it, 0, haystack.len - needle.len + 1) {
    if (sp_str_equal(sp_str_sub(haystack, it, (s32)needle.len), needle)) count++;
  }
  return count;
}

static void expect_contains(sp_test_t* t, sp_str_t captured, const c8* const* contains, u32 capacity) {
  sp_for(it, capacity) {
    if (!contains[it]) break;
    sp_test_kv_c(t, "contains", contains[it]);
    sp_expect_ne(t, sp_str_find(captured, sp_cstr_as_str(contains[it])), SP_STR_NO_MATCH);
  }
  sp_test_kv_clear(t, "contains");
}

static void expect_counted(sp_test_t* t, sp_str_t captured, const counted_t* counted, u32 capacity) {
  sp_for(it, capacity) {
    if (!counted[it].text) break;
    sp_test_kv_c(t, "counted", counted[it].text);
    sp_expect_eq(t, count_matches(captured, sp_cstr_as_str(counted[it].text)), counted[it].n);
  }
  sp_test_kv_clear(t, "counted");
}

#endif
