#ifndef HTTP_CONN_TEST_H
#define HTTP_CONN_TEST_H

#include "../http.h"

#define CONN_TEST_HEAD_MAX   256
#define CONN_TEST_BODY_MAX   64
#define CONN_TEST_STREAM_MAX 64
#define CONN_TEST_WIRE_MAX   4096
#define CONN_TEST_MAX_STEPS  4096

typedef struct {
  sp_http_conn_t conn;
  const c8*      input;
  u32            input_len;
  u32            input_at;
  u32            chunk;
  u32            send_chunk;
  bool           eof;
  bool           eof_sent;
  c8             wire [CONN_TEST_WIRE_MAX];
  u32            wire_len;
} conn_harness_t;

static void harness_init(conn_harness_t* h, sp_http_conn_desc_t desc, const c8* input) {
  *h = sp_zero_s(conn_harness_t);
  sp_http_conn_init(&h->conn, desc);
  h->input = input ? input : "";
  h->input_len = sp_cstr_len(h->input);
}

static sp_http_conn_desc_t harness_desc(sp_mem_t mem, u32 head_max, u32 body_max, u32 stream_max) {
  return (sp_http_conn_desc_t) {
    .mem = mem,
    .head_max = head_max ? head_max : CONN_TEST_HEAD_MAX,
    .body_max = body_max ? body_max : CONN_TEST_BODY_MAX,
    .stream_max = stream_max ? stream_max : CONN_TEST_STREAM_MAX,
  };
}

static bool harness_feed(conn_harness_t* h) {
  u32 left = h->input_len - h->input_at;
  if (left == 0) {
    if (!h->eof || h->eof_sent) return false;
    h->eof_sent = true;
    sp_http_conn_received(&h->conn, 0);
    return true;
  }
  sp_mem_slice_t slot = sp_http_conn_recv_slot(&h->conn);
  if (sp_mem_slice_empty(slot)) return false;
  u32 n = left;
  if (h->chunk && h->chunk < n) n = h->chunk;
  if (slot.len < n) n = (u32)slot.len;
  sp_mem_copy(slot.data, h->input + h->input_at, n);
  h->input_at += n;
  sp_http_conn_received(&h->conn, n);
  return true;
}

static bool harness_drain(conn_harness_t* h) {
  sp_mem_slice_t slot = sp_http_conn_send_slot(&h->conn);
  if (sp_mem_slice_empty(slot)) return false;
  u32 n = (u32)slot.len;
  if (h->send_chunk && h->send_chunk < n) n = h->send_chunk;
  if (h->wire_len + n > CONN_TEST_WIRE_MAX) n = CONN_TEST_WIRE_MAX - h->wire_len;
  if (n == 0) return false;
  sp_mem_copy(h->wire + h->wire_len, slot.data, n);
  h->wire_len += n;
  sp_http_conn_sent(&h->conn, n);
  return true;
}

static u32 harness_drive(conn_harness_t* h) {
  sp_for(it, CONN_TEST_MAX_STEPS) {
    u32 want = sp_http_conn_step(&h->conn);
    if (want & (SP_HTTP_WANT_REQUEST | SP_HTTP_WANT_CLOSE)) return want;
    if ((want & SP_HTTP_WANT_SEND) && harness_drain(h)) continue;
    if ((want & SP_HTTP_WANT_RECV) && harness_feed(h)) continue;
    return want;
  }
  return 0;
}

static sp_str_t harness_wire(conn_harness_t* h) {
  return sp_str(h->wire, h->wire_len);
}

static s32 harness_first_status(conn_harness_t* h) {
  sp_str_t wire = harness_wire(h);
  s32 end = sp_str_find(wire, sp_str_lit("\r\n\r\n"));
  if (end == SP_STR_NO_MATCH) return 0;
  sp_http_response_head_t head = sp_zero;
  if (sp_http_response_head_parse(sp_str_sub(wire, 0, end), &head) != SP_HTTP_OK) return 0;
  return head.status;
}

#endif
