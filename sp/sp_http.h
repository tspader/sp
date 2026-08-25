#ifndef SP_HTTP_H
#define SP_HTTP_H

#include "sp.h"
#include "sp_io.h"

#if defined(SP_WIN32) || defined(SP_POSIX)
  #define SP_HTTP_SOCKETS
#endif

typedef enum {
  SP_HTTP_OK = 0,
  SP_HTTP_ERR_NO_STORE,
  SP_HTTP_ERR_PARSE,
  SP_HTTP_ERR_OS,
  SP_HTTP_ERR_UNTRUSTED,
  SP_HTTP_ERR_BAD_CONFIG,
  SP_HTTP_ERR_UNSUPPORTED,
  SP_HTTP_ERR_URL,
  SP_HTTP_ERR_CONNECT,
  SP_HTTP_ERR_HANDSHAKE,
  SP_HTTP_ERR_PROTOCOL,
  SP_HTTP_ERR_STATUS,
  SP_HTTP_ERR_REDIRECTS,
  SP_HTTP_ERR_TIMEOUT,
  SP_HTTP_ERR_PROXY,
} sp_http_error_t;

#define SP_HTTP_DEFAULT_REDIRECTS          16
#define SP_HTTP_DEFAULT_CONNECT_TIMEOUT_MS 30000
#define SP_HTTP_DEFAULT_IO_TIMEOUT_MS      60000
#define SP_HTTP_TIMEOUT_INFINITE           0xffffffffu
#define SP_HTTP_MAX_ADDRS                  16

typedef struct {
  sp_str_t scheme;
  sp_str_t host;
  sp_str_t port;
  sp_str_t path;
  bool     tls;
} sp_http_url_t;

typedef enum {
  SP_HTTP_GET = 0,
  SP_HTTP_POST,
  SP_HTTP_PUT,
  SP_HTTP_PATCH,
  SP_HTTP_DELETE,
  SP_HTTP_HEAD,
} sp_http_method_t;

typedef struct {
  sp_str_t name;
  sp_str_t value;
} sp_http_header_t;

typedef enum {
  SP_HTTP_BODY_NONE,
  SP_HTTP_BODY_LENGTH,
  SP_HTTP_BODY_CHUNKED,
  SP_HTTP_BODY_EOF,
} sp_http_body_kind_t;

typedef struct {
  sp_http_body_kind_t kind;
  u64                 length;
} sp_http_body_t;

typedef enum {
  SP_HTTP_VERSION_1_1,
  SP_HTTP_VERSION_1_0,
} sp_http_version_t;

typedef struct {
  sp_str_t          method;
  sp_str_t          target;
  sp_str_t          headers;
  sp_http_version_t version;
} sp_http_request_head_t;

typedef struct {
  s32      status;
  sp_str_t headers;
} sp_http_response_head_t;

typedef struct {
  sp_str_t rest;
} sp_http_headers_it_t;

typedef struct {
  sp_io_reader_t      base;
  sp_io_reader_t*     inner;
  sp_http_body_kind_t kind;
  u64                 remaining;
  bool                line_pending;
  bool                done;
  sp_http_error_t     err;
} sp_http_body_reader_t;

typedef enum {
  SP_HTTP_ADDR_V4,
  SP_HTTP_ADDR_V6,
} sp_http_addr_kind_t;

typedef struct {
  sp_http_addr_kind_t kind;
  u8                  data [16];
} sp_http_addr_t;

typedef sp_http_error_t (*sp_http_resolve_fn)(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count);

typedef struct {
  sp_http_resolve_fn resolve;
  void*              user_data;
} sp_http_resolver_t;

/////////
// TLS //
/////////
typedef struct sp_tls sp_tls_t;

struct sp_tls {
  sp_http_error_t (*open)(sp_tls_t* tls, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms);
  void            (*close)(sp_tls_t* tls);
  sp_io_reader_t* reader;
  sp_io_writer_t* writer;
};

#if defined(SP_TLS_WITH_MBEDTLS)
#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

// mbedtls has a process-global PSA key store, which is used for TLS 1.3
// handshakes. By default, it isn't thread safe. The only way to fix this from
// our code would be:
// - A lock that covers every `mbedtls_ssl_read` / `mbedtls_ssl_write`, but
//   then nothing is actually concurrent
// - Downgrade to TLS 1.2
//
// The only solution is to force the consumer to choose multithreaded or single
// threaded, and make everyone agree
#if !defined(MBEDTLS_THREADING_C) && !defined(SP_TLS_MBEDTLS_SINGLE_THREAD)
  #error "sp_http is multithreaded, but the mbedtls configuration is single threaded; compile with SP_TLS_MBEDTLS_SINGLE_THREAD, or enable multithreading in mbedtls with MBEDTLS_THREADING_C"
#endif

typedef enum {
  SP_TLS_BACKEND_NONE,
  SP_TLS_BACKEND_ANCHORS,
  SP_TLS_BACKEND_OS_VERIFY,
} sp_tls_backend_t;

typedef struct {
  sp_tls_backend_t  backend;
  mbedtls_x509_crt* anchors;
  u32               loaded;
  u32               skipped;
  sp_mem_t          mem;
} sp_tls_trust_t;

typedef struct {
  sp_str_t hostname;
} sp_tls_verify_t;

typedef struct sp_tls_mbedtls sp_tls_mbedtls_t;

typedef struct {
  sp_io_reader_t    base;
  sp_tls_mbedtls_t* tls;
  bool              eof;
} sp_tls_mbedtls_reader_t;

typedef struct {
  sp_io_writer_t    base;
  sp_tls_mbedtls_t* tls;
} sp_tls_mbedtls_writer_t;

struct sp_tls_mbedtls {
  sp_tls_t                 base;
  const sp_tls_trust_t*    trust;
  sp_sys_socket_t          socket;
  u32                      io_timeout_ms;
  mbedtls_net_context      net;
  mbedtls_ssl_context      ssl;
  mbedtls_ssl_config       conf;
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context drbg;
  sp_tls_verify_t          verify;
  sp_tls_mbedtls_reader_t  reader;
  sp_tls_mbedtls_writer_t  writer;
};

SP_API sp_http_error_t  sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem);
SP_API void             sp_tls_trust_free(sp_tls_trust_t* trust);
SP_API sp_http_error_t  sp_tls_trust_load(sp_tls_trust_t* trust);
SP_API sp_http_error_t  sp_tls_load_pem(mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped);
SP_API void             sp_tls_mbedtls_init(sp_tls_mbedtls_t* tls, const sp_tls_trust_t* trust);
#endif

typedef struct {
  sp_str_t                url;
  sp_tls_t*               tls;
  sp_http_resolver_t      resolver;
  sp_io_writer_t*         sink;
  sp_http_method_t        method;
  sp_str_t                payload;
  sp_str_t                content_type;
  const sp_http_header_t* headers;
  u32                     num_headers;
  u32                     max_redirects;
  sp_str_t                proxy;
  bool                    no_proxy;
  u32                     connect_timeout_ms;
  u32                     io_timeout_ms;
} sp_http_request_t;

typedef struct {
  s32      status;
  u64      body_len;
  sp_str_t url;
  sp_str_t headers;
} sp_http_response_t;

SP_API bool                 sp_http_url_parse(sp_str_t url, sp_http_url_t* out);
SP_API sp_str_t             sp_http_method_name(sp_http_method_t method);
SP_API bool                 sp_http_method_parse(sp_str_t name, sp_http_method_t* out);
SP_API sp_str_t             sp_http_status_reason(s32 status);
SP_API sp_http_headers_it_t sp_http_headers_it(sp_str_t headers);
SP_API bool                 sp_http_headers_it_next(sp_http_headers_it_t* it, sp_http_header_t* out);
SP_API sp_str_t             sp_http_headers_find(sp_str_t headers, sp_str_t name);
SP_API bool                 sp_http_headers_has(sp_str_t headers, sp_str_t name);
SP_API sp_http_error_t      sp_http_headers_check(const sp_http_header_t* headers, u32 count);
SP_API sp_http_error_t      sp_http_request_head_parse(sp_str_t head, sp_http_request_head_t* out);
SP_API sp_http_error_t      sp_http_response_head_parse(sp_str_t head, sp_http_response_head_t* out);
SP_API sp_http_error_t      sp_http_body_parse(sp_str_t headers, sp_http_body_t* out);
SP_API sp_http_error_t      sp_http_response_body_parse(s32 status, bool head_request, sp_str_t headers, sp_http_body_t* out);
SP_API sp_http_error_t      sp_http_head_read(sp_io_reader_t* reader, sp_str_t* head);
SP_API void                 sp_http_body_reader_init(sp_http_body_reader_t* r, sp_io_reader_t* inner, sp_http_body_t body);
SP_API sp_http_error_t      sp_http_body_read(sp_io_reader_t* reader, sp_http_body_t body, sp_io_writer_t* sink, u64* len);
SP_API sp_http_error_t      sp_http_request_head_write(sp_io_writer_t* writer, sp_str_t method, sp_str_t target, const sp_http_header_t* headers, u32 count);
SP_API sp_http_error_t      sp_http_response_head_write(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count);
SP_API sp_http_error_t      sp_http_response_write(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count, sp_str_t body);
SP_API sp_http_resolver_t   sp_http_resolver_default(void);
SP_API sp_http_error_t      sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response);

typedef struct {
  sp_str_t path;
  sp_str_t query;
} sp_http_target_t;

SP_API sp_http_target_t     sp_http_target_split(sp_str_t target);
SP_API sp_str_t             sp_http_query_find(sp_str_t query, sp_str_t key);
SP_API sp_str_t             sp_http_percent_decode(sp_mem_t mem, sp_str_t str);
SP_API sp_str_t             sp_http_mime_type(sp_str_t path);

////////////
// SERVER //
////////////

#define SP_HTTP_REPLY_MAX_HEADERS 8

#define SP_HTTP_WANT_RECV    (1u << 0)
#define SP_HTTP_WANT_SEND    (1u << 1)
#define SP_HTTP_WANT_REQUEST (1u << 2)
#define SP_HTTP_WANT_CLOSE   (1u << 3)

typedef struct sp_http_ctx sp_http_ctx_t;
typedef struct sp_http_stream sp_http_stream_t;
typedef struct sp_http_conn sp_http_conn_t;

typedef enum {
  SP_HTTP_REPLY_ONESHOT,
  SP_HTTP_REPLY_STREAM,
} sp_http_reply_kind_t;

typedef struct {
  sp_http_reply_kind_t kind;
  s32                  status;
  sp_str_t             content_type;
  sp_str_t             body;
  sp_http_header_t     headers [SP_HTTP_REPLY_MAX_HEADERS];
  u32                  num_headers;
  sp_http_stream_t*    stream;
} sp_http_reply_t;

SP_TYPEDEF_FN(sp_http_reply_t, sp_http_handler_t, sp_http_ctx_t* c);

typedef struct {
  sp_http_method_t  method;
  const c8*         path;
  sp_http_handler_t handler;
} sp_http_route_t;

typedef struct {
  const sp_http_route_t* routes;
  u32                    count;
  void*                  user_data;
} sp_http_router_t;

struct sp_http_ctx {
  sp_mem_t          mem;
  sp_http_method_t  method;
  sp_str_t          path;
  sp_str_t          query;
  sp_str_t          headers;
  sp_str_t          body;
  sp_http_stream_t* stream;
  void*             user_data;
};

struct sp_http_stream {
  sp_io_writer_t base;
  u8*            ring;
  u32            cap;
  u32            head;
  u32            len;
  bool           held;
  bool           dead;
};

typedef struct {
  sp_mem_t mem;
  u32      head_max;
  u32      body_max;
  u32      stream_max;
} sp_http_conn_desc_t;

typedef enum {
  SP_HTTP_CONN_HEAD,
  SP_HTTP_CONN_BODY,
  SP_HTTP_CONN_REQUEST,
  SP_HTTP_CONN_REPLY,
  SP_HTTP_CONN_STREAM,
  SP_HTTP_CONN_CLOSED,
} sp_http_conn_state_t;

struct sp_http_conn {
  sp_http_conn_desc_t  desc;
  sp_http_conn_state_t state;
  sp_mem_arena_t*      arena;
  sp_http_ctx_t        ctx;
  sp_http_stream_t     stream;
  bool                 keep_alive;
  bool                 method_known;
  struct {
    u8* data;
    u32 cap;
    u32 len;
    u32 scan;
    u32 body_at;
    u32 need;
  } in;
  struct {
    sp_io_mem_writer_t head;
    u8*                head_data;
    sp_mem_slice_t     slices [3];
    u32                count;
    u32                at;
    u64                cursor;
  } out;
};

typedef struct {
  sp_io_t             io;
  sp_sys_ipv4_t       addr;
  sp_http_router_t    router;
  sp_http_conn_desc_t conn;
  u32                 max_conns;
  u32                 idle_ms;
} sp_http_server_desc_t;

typedef struct {
  sp_http_conn_t  conn;
  sp_sys_socket_t socket;
  sp_io_op_t      recv;
  sp_io_op_t      send;
  sp_io_time_t    active;
  bool            recv_armed;
  bool            send_armed;
  bool            closing;
  bool            live;
} sp_http_slot_t;

typedef struct {
  sp_http_server_desc_t desc;
  sp_sys_socket_t       listener;
  u16                   port;
  sp_io_op_t            accept;
  bool                  accept_armed;
  sp_http_slot_t*       slots;
  u64                   idle_ns;
  bool                  stopping;
  sp_atomic_u32_t       quit;
} sp_http_server_t;

SP_API void             sp_http_conn_init(sp_http_conn_t* conn, sp_http_conn_desc_t desc);
SP_API void             sp_http_conn_deinit(sp_http_conn_t* conn);
SP_API void             sp_http_conn_reset(sp_http_conn_t* conn);
SP_API u32              sp_http_conn_step(sp_http_conn_t* conn);
SP_API sp_mem_slice_t   sp_http_conn_recv_slot(sp_http_conn_t* conn);
SP_API void             sp_http_conn_received(sp_http_conn_t* conn, u64 n);
SP_API sp_mem_slice_t   sp_http_conn_send_slot(sp_http_conn_t* conn);
SP_API void             sp_http_conn_sent(sp_http_conn_t* conn, u64 n);
SP_API void             sp_http_conn_reply(sp_http_conn_t* conn, sp_http_reply_t reply);
SP_API void             sp_http_conn_serve(sp_http_conn_t* conn, sp_sys_socket_t socket, const sp_http_router_t* router);

SP_API sp_http_reply_t  sp_http_route(const sp_http_router_t* router, sp_http_ctx_t* c);
SP_API sp_str_t         sp_http_ctx_query(sp_http_ctx_t* c, const c8* key);
SP_API sp_str_t         sp_http_ctx_header(sp_http_ctx_t* c, const c8* name);
SP_API sp_http_stream_t* sp_http_ctx_stream(sp_http_ctx_t* c);
SP_API sp_http_reply_t  sp_http_reply_status(s32 status);
SP_API sp_http_reply_t  sp_http_reply_text(s32 status, sp_str_t body);
SP_API sp_http_reply_t  sp_http_reply_json(s32 status, sp_str_t body);
SP_API sp_http_reply_t  sp_http_reply_file(sp_http_ctx_t* c, sp_str_t root, sp_str_t rel);
SP_API sp_http_reply_t  sp_http_reply_stream(sp_http_stream_t* stream, sp_str_t content_type);
SP_API void             sp_http_reply_header(sp_http_reply_t* reply, sp_str_t name, sp_str_t value);
SP_API bool             sp_http_stream_closed(const sp_http_stream_t* stream);
SP_API void             sp_http_stream_close(sp_http_stream_t* stream);

SP_API sp_http_error_t  sp_http_server_init(sp_http_server_t* server, sp_http_server_desc_t desc);
SP_API void             sp_http_server_pump(sp_http_server_t* server, sp_io_timeout_t timeout);
SP_API void             sp_http_server_run(sp_http_server_t* server);
SP_API void             sp_http_server_stop(sp_http_server_t* server);
SP_API void             sp_http_server_deinit(sp_http_server_t* server);

#endif

#if defined SP_IMPLEMENTATION && !defined(SP_HTTP_IMPLEMENTATION)
  #define SP_HTTP_IMPLEMENTATION
#endif

#if !defined(SP_HTTP_IMPL_H)
#if defined(SP_HTTP_EVERYTHING_PUBLIC) || defined(SP_HTTP_IMPLEMENTATION)
#define SP_HTTP_IMPL_H

typedef struct {
  sp_http_url_t           url;
  bool                    absolute_form;
  sp_http_method_t        method;
  sp_str_t                payload;
  sp_str_t                content_type;
  const sp_http_header_t* headers;
  u32                     num_headers;
  bool                    strip_auth;
} sp_http_wire_t;

SP_PRIVATE bool            sp_http_ci_equal(sp_str_t a, sp_str_t b);
SP_PRIVATE bool            sp_http_ci_contains(sp_str_t haystack, sp_str_t needle);
SP_PRIVATE sp_str_t        sp_http_str_tail(sp_str_t str, s32 from);
SP_PRIVATE sp_str_t        sp_http_host_bare(sp_str_t host);
SP_PRIVATE bool            sp_http_v6_group(sp_str_t part, u16* value);
SP_PRIVATE bool            sp_http_addr_parse_v4(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_addr_parse_v6(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_addr_parse(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_url_host_ok(sp_str_t host);
SP_PRIVATE bool            sp_http_url_port_ok(sp_str_t port);
SP_PRIVATE bool            sp_http_url_path_ok(sp_str_t path);
SP_PRIVATE bool            sp_http_token_ok(sp_str_t token);
SP_PRIVATE bool            sp_http_header_value_ok(sp_str_t value);
SP_PRIVATE sp_http_error_t sp_http_status_line_parse(sp_str_t line, s32* status);
SP_PRIVATE sp_http_error_t sp_http_header_lines_check(sp_str_t lines);
SP_PRIVATE bool            sp_http_location_is_absolute(sp_str_t location);
SP_PRIVATE sp_str_t        sp_http_resolve_url(sp_mem_t mem, sp_http_url_t base, sp_str_t location);
SP_PRIVATE bool            sp_http_no_proxy_match(sp_str_t host, sp_str_t no_proxy);
SP_PRIVATE sp_str_t        sp_http_proxy_pick(sp_http_url_t url, sp_str_t http_proxy, sp_str_t https_proxy, sp_str_t all_proxy, sp_str_t no_proxy);
SP_PRIVATE sp_str_t        sp_http_env_either(const c8* lower, const c8* upper);
SP_PRIVATE sp_str_t        sp_http_proxy_from_env(sp_http_url_t url);
SP_PRIVATE sp_http_error_t sp_http_map_io(sp_err_t err, sp_http_error_t fallback);
SP_PRIVATE u32             sp_http_timeout_ms(u32 requested, u32 fallback);
SP_PRIVATE sp_http_error_t sp_http_read_line(sp_io_reader_t* reader, sp_str_t* line);
SP_PRIVATE sp_http_error_t sp_http_chunk_begin(sp_http_body_reader_t* r);
SP_PRIVATE sp_err_t        sp_http_body_reader_advance(sp_http_body_reader_t* r, void* dst, u64 size, u64* moved);
SP_PRIVATE sp_err_t        sp_http_body_reader_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read);
SP_PRIVATE sp_err_t        sp_http_body_reader_discard(sp_io_reader_t* reader, u64 n, u64* discarded);
SP_PRIVATE bool            sp_http_header_list_have(const sp_http_header_t* headers, u32 count, sp_str_t name);
SP_PRIVATE bool            sp_http_headers_reserved(const sp_http_header_t* headers, u32 count);
SP_PRIVATE sp_http_error_t sp_http_header_write(sp_io_writer_t* writer, sp_http_header_t header);
SP_PRIVATE sp_http_error_t sp_http_headers_write(sp_io_writer_t* writer, const sp_http_header_t* headers, u32 count);
SP_PRIVATE sp_http_error_t sp_http_response_head_io(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count);
SP_PRIVATE sp_str_t        sp_http_build_request(sp_mem_t mem, sp_http_wire_t wire);
SP_PRIVATE bool            sp_http_proxy_url_parse(sp_mem_t mem, sp_str_t proxy, sp_http_url_t* out);

#if defined(SP_HTTP_SOCKETS)
typedef struct sp_http_transport sp_http_transport_t;

SP_PRIVATE sp_http_error_t sp_http_resolve_system(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count);
SP_PRIVATE sp_http_error_t sp_http_connect_addr(sp_sys_socket_t* out, sp_http_addr_t addr, u16 port, u32 timeout_ms);
SP_PRIVATE sp_http_error_t sp_http_net_connect(sp_sys_socket_t* out, sp_http_resolver_t resolver, sp_str_t host, u16 port, u32 timeout_ms);
SP_PRIVATE sp_http_error_t sp_http_transport_write(sp_http_transport_t* conn, sp_str_t data);
SP_PRIVATE sp_http_error_t sp_http_connect_reply(sp_http_transport_t* conn);
SP_PRIVATE sp_http_error_t sp_http_transport_open(sp_http_transport_t* conn, sp_tls_t* tls, sp_http_resolver_t resolver, sp_http_url_t url, const sp_http_url_t* proxy, u32 connect_timeout_ms, u32 io_timeout_ms);
SP_PRIVATE void            sp_http_transport_close(sp_http_transport_t* conn);
#endif

#if defined(SP_TLS_WITH_MBEDTLS)
SP_PRIVATE sp_tls_backend_t sp_tls_native_backend(void);
SP_PRIVATE u32             sp_tls_chain_count(const mbedtls_x509_crt* chain);
SP_PRIVATE sp_http_error_t sp_tls_load_unix(mbedtls_x509_crt* chain, u32* loaded, u32* skipped);
SP_PRIVATE sp_http_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, mbedtls_ssl_config* conf);
SP_PRIVATE sp_http_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname);
SP_PRIVATE s32             sp_tls_verify_cb(void* user_data, mbedtls_x509_crt* crt, s32 depth, u32* flags);
SP_PRIVATE sp_http_error_t sp_tls_macos_eval(const mbedtls_x509_crt* chain, sp_str_t hostname);
SP_PRIVATE sp_http_error_t sp_tls_windows_eval(const mbedtls_x509_crt* chain, sp_str_t hostname);
SP_PRIVATE sp_http_error_t sp_tls_mbedtls_open(sp_tls_t* base, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms);
SP_PRIVATE void            sp_tls_mbedtls_close(sp_tls_t* base);
SP_PRIVATE sp_http_error_t sp_tls_mbedtls_handshake(sp_tls_mbedtls_t* tls, sp_str_t hostname);
SP_PRIVATE void            sp_tls_mbedtls_drop(sp_tls_mbedtls_t* tls);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_wait(sp_tls_mbedtls_t* tls, s32 rc);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written);
#endif

#endif // SP_HTTP_EVERYTHING_PUBLIC or SP_HTTP_IMPLEMENTATION
#endif // SP_HTTP_IMPL_H

#ifndef SP_HTTP_C
#if defined(SP_HTTP_IMPLEMENTATION)
#define SP_HTTP_C

SP_PRIVATE bool sp_http_ci_equal(sp_str_t a, sp_str_t b) {
  if (a.len != b.len) return false;
  sp_for(it, a.len) {
    c8 ca = a.data[it];
    c8 cb = b.data[it];
    if (ca >= 'A' && ca <= 'Z') ca = (c8)(ca + 32);
    if (cb >= 'A' && cb <= 'Z') cb = (c8)(cb + 32);
    if (ca != cb) return false;
  }
  return true;
}

SP_PRIVATE bool sp_http_ci_contains(sp_str_t haystack, sp_str_t needle) {
  if (needle.len > haystack.len) return false;
  sp_for_range(it, 0, haystack.len - needle.len + 1) {
    if (sp_http_ci_equal(sp_str_sub(haystack, it, (s32)needle.len), needle)) return true;
  }
  return false;
}

SP_PRIVATE sp_str_t sp_http_str_tail(sp_str_t str, s32 from) {
  return sp_str_sub(str, from, (s32)str.len - from);
}

SP_PRIVATE sp_str_t sp_http_host_bare(sp_str_t host) {
  if (host.len >= 2 && host.data[0] == '[' && host.data[host.len - 1] == ']') {
    return sp_str_sub(host, 1, (s32)host.len - 2);
  }
  return host;
}

SP_PRIVATE bool sp_http_url_host_ok(sp_str_t host) {
  if (sp_str_empty(host)) return false;
  if (host.data[0] == '[') {
    if (host.len < 4 || host.data[host.len - 1] != ']') return false;
    sp_for_range(it, 1, host.len - 1) {
      c8 c = host.data[it];
      bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
      if (!hex && c != ':' && c != '.') return false;
    }
    return true;
  }
  sp_for(it, host.len) {
    c8 c = host.data[it];
    bool alnum = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    if (!alnum && c != '-' && c != '.' && c != '_') return false;
  }
  return true;
}

SP_PRIVATE bool sp_http_v6_group(sp_str_t part, u16* value) {
  if (sp_str_empty(part) || part.len > 4) return false;
  u32 group = 0;
  sp_for(it, part.len) {
    c8 c = part.data[it];
    u32 digit;
    if (c >= '0' && c <= '9') digit = (u32)(c - '0');
    else if (c >= 'a' && c <= 'f') digit = (u32)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') digit = (u32)(c - 'A' + 10);
    else return false;
    group = (group << 4) | digit;
  }
  *value = (u16)group;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse_v4(sp_str_t host, sp_http_addr_t* out) {
  *out = sp_zero_s(sp_http_addr_t);
  sp_str_t rest = host;
  sp_for(it, 4) {
    s32 dot = sp_str_find_c8(rest, '.');
    if ((it < 3) != (dot != SP_STR_NO_MATCH)) return false;
    sp_str_t part = dot == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, dot);
    if (sp_str_empty(part) || part.len > 3) return false;
    if (part.len > 1 && part.data[0] == '0') return false;
    u32 value = 0;
    if (!sp_parse_u32_ex(part, &value)) return false;
    if (value > 255) return false;
    out->data[it] = (u8)value;
    rest = dot == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, dot + 1);
  }
  out->kind = SP_HTTP_ADDR_V4;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse_v6(sp_str_t host, sp_http_addr_t* out) {
  *out = sp_zero_s(sp_http_addr_t);
  u16 groups [8] = sp_zero;
  u32 count = 0;
  s32 run = -1;

  sp_str_t rest = host;
  if (sp_str_starts_with(rest, sp_str_lit("::"))) {
    run = 0;
    rest = sp_http_str_tail(rest, 2);
  }
  else if (!sp_str_empty(rest) && rest.data[0] == ':') {
    return false;
  }

  while (!sp_str_empty(rest)) {
    if (count == 8) return false;
    s32 colon = sp_str_find_c8(rest, ':');
    sp_str_t part = colon == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, colon);

    if (sp_str_find_c8(part, '.') != SP_STR_NO_MATCH) {
      if (colon != SP_STR_NO_MATCH || count > 6) return false;
      sp_http_addr_t v4 = sp_zero;
      if (!sp_http_addr_parse_v4(part, &v4)) return false;
      groups[count++] = (u16)((v4.data[0] << 8) | v4.data[1]);
      groups[count++] = (u16)((v4.data[2] << 8) | v4.data[3]);
      break;
    }

    if (!sp_http_v6_group(part, &groups[count])) return false;
    count++;

    if (colon == SP_STR_NO_MATCH) break;
    rest = sp_http_str_tail(rest, colon + 1);
    if (sp_str_empty(rest)) return false;
    if (rest.data[0] == ':') {
      if (run >= 0) return false;
      run = (s32)count;
      rest = sp_http_str_tail(rest, 1);
    }
  }

  if (run < 0 && count != 8) return false;
  if (run >= 0 && count >= 8) return false;

  u32 front = run < 0 ? count : (u32)run;
  u32 back = count - front;
  sp_for(it, front) {
    out->data[it * 2 + 0] = (u8)(groups[it] >> 8);
    out->data[it * 2 + 1] = (u8)(groups[it] & 0xff);
  }
  sp_for(it, back) {
    u32 at = 8 - back + it;
    out->data[at * 2 + 0] = (u8)(groups[front + it] >> 8);
    out->data[at * 2 + 1] = (u8)(groups[front + it] & 0xff);
  }
  out->kind = SP_HTTP_ADDR_V6;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse(sp_str_t host, sp_http_addr_t* out) {
  if (sp_str_find_c8(host, ':') != SP_STR_NO_MATCH) return sp_http_addr_parse_v6(host, out);
  return sp_http_addr_parse_v4(host, out);
}

SP_PRIVATE bool sp_http_url_port_ok(sp_str_t port) {
  if (sp_str_empty(port) || port.len > 5) return false;
  sp_for(it, port.len) {
    if (port.data[it] < '0' || port.data[it] > '9') return false;
  }
  u32 value = 0;
  if (!sp_parse_u32_ex(port, &value)) return false;
  return value >= 1 && value <= 65535;
}

SP_PRIVATE bool sp_http_url_path_ok(sp_str_t path) {
  sp_for(it, path.len) {
    u8 c = (u8)path.data[it];
    if (c <= 0x20 || c == 0x7f) return false;
  }
  return true;
}

bool sp_http_url_parse(sp_str_t url, sp_http_url_t* out) {
  *out = sp_zero_s(sp_http_url_t);

  s32 hash = sp_str_find_c8(url, '#');
  if (hash != SP_STR_NO_MATCH) url = sp_str_sub(url, 0, hash);

  sp_str_t rest = url;
  s32 sep = sp_str_find(url, sp_str_lit("://"));
  if (sep != SP_STR_NO_MATCH) {
    out->scheme = sp_str_sub(url, 0, sep);
    rest = sp_http_str_tail(url, sep + 3);
  }
  else {
    out->scheme = sp_str_lit("https");
  }

  bool https = sp_http_ci_equal(out->scheme, sp_str_lit("https"));
  bool http  = sp_http_ci_equal(out->scheme, sp_str_lit("http"));
  if (!https && !http) return false;
  out->tls = https;

  s32 slash = sp_str_find_c8(rest, '/');
  s32 qmark = sp_str_find_c8(rest, '?');
  s32 cut = slash;
  if (qmark != SP_STR_NO_MATCH && (slash == SP_STR_NO_MATCH || qmark < slash)) cut = qmark;
  sp_str_t authority;
  if (cut == SP_STR_NO_MATCH) {
    authority = rest;
    out->path = sp_str_lit("/");
  }
  else {
    authority = sp_str_sub(rest, 0, cut);
    out->path = sp_http_str_tail(rest, cut);
  }

  if (sp_str_find_c8(authority, '@') != SP_STR_NO_MATCH) return false;

  if (!sp_str_empty(authority) && authority.data[0] == '[') {
    s32 close = sp_str_find_c8(authority, ']');
    if (close == SP_STR_NO_MATCH) return false;
    out->host = sp_str_sub(authority, 0, close + 1);
    sp_str_t after = sp_http_str_tail(authority, close + 1);
    if (sp_str_empty(after)) {
      out->port = out->tls ? sp_str_lit("443") : sp_str_lit("80");
    }
    else {
      if (after.data[0] != ':') return false;
      out->port = sp_http_str_tail(after, 1);
    }
  }
  else {
    s32 colon = sp_str_find_c8(authority, ':');
    if (colon == SP_STR_NO_MATCH) {
      out->host = authority;
      out->port = out->tls ? sp_str_lit("443") : sp_str_lit("80");
    }
    else {
      out->host = sp_str_sub(authority, 0, colon);
      out->port = sp_http_str_tail(authority, colon + 1);
    }
  }

  return sp_http_url_host_ok(out->host) && sp_http_url_port_ok(out->port) && sp_http_url_path_ok(out->path);
}

SP_PRIVATE bool sp_http_token_ok(sp_str_t token) {
  if (sp_str_empty(token)) return false;
  sp_for(it, token.len) {
    u8 c = (u8)token.data[it];
    bool tchar =
      (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
      c == '!' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\'' ||
      c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
      c == '`' || c == '|' || c == '~';
    if (!tchar) return false;
  }
  return true;
}

SP_PRIVATE bool sp_http_header_value_ok(sp_str_t value) {
  sp_for(it, value.len) {
    u8 c = (u8)value.data[it];
    if ((c < 0x20 && c != '\t') || c == 0x7f) return false;
  }
  return true;
}

SP_PRIVATE sp_http_error_t sp_http_status_line_parse(sp_str_t line, s32* status) {
  if (!sp_str_starts_with(line, sp_str_lit("HTTP/"))) return SP_HTTP_ERR_PROTOCOL;
  s32 sp1 = sp_str_find_c8(line, ' ');
  if (sp1 == SP_STR_NO_MATCH) return SP_HTTP_ERR_PROTOCOL;
  sp_str_t after = sp_str_trim_left(sp_http_str_tail(line, sp1 + 1));
  s32 sp2 = sp_str_find_c8(after, ' ');
  sp_str_t code = sp2 == SP_STR_NO_MATCH ? after : sp_str_sub(after, 0, sp2);
  u32 value = 0;
  if (!sp_parse_u32_ex(code, &value)) return SP_HTTP_ERR_PROTOCOL;
  if (value < 100 || value > 599) return SP_HTTP_ERR_PROTOCOL;
  *status = (s32)value;
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_header_lines_check(sp_str_t lines) {
  sp_str_t rest = lines;
  while (!sp_str_empty(rest)) {
    s32 nl = sp_str_find(rest, sp_str_lit("\r\n"));
    sp_str_t line = nl == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, nl);
    rest = nl == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, nl + 2);

    s32 colon = sp_str_find_c8(line, ':');
    if (colon == SP_STR_NO_MATCH) return SP_HTTP_ERR_PROTOCOL;
    if (!sp_http_token_ok(sp_str_sub(line, 0, colon))) return SP_HTTP_ERR_PROTOCOL;
    if (!sp_http_header_value_ok(sp_http_str_tail(line, colon + 1))) return SP_HTTP_ERR_PROTOCOL;
  }
  return SP_HTTP_OK;
}

sp_http_headers_it_t sp_http_headers_it(sp_str_t headers) {
  return (sp_http_headers_it_t) { .rest = headers };
}

bool sp_http_headers_it_next(sp_http_headers_it_t* it, sp_http_header_t* out) {
  if (sp_str_empty(it->rest)) return false;
  s32 nl = sp_str_find(it->rest, sp_str_lit("\r\n"));
  sp_str_t line = nl == SP_STR_NO_MATCH ? it->rest : sp_str_sub(it->rest, 0, nl);
  it->rest = nl == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(it->rest, nl + 2);

  s32 colon = sp_str_find_c8(line, ':');
  if (colon == SP_STR_NO_MATCH || colon == 0) return false;
  out->name = sp_str_sub(line, 0, colon);
  out->value = sp_str_trim(sp_http_str_tail(line, colon + 1));
  return true;
}

sp_str_t sp_http_headers_find(sp_str_t headers, sp_str_t name) {
  sp_http_headers_it_t it = sp_http_headers_it(headers);
  sp_http_header_t header = sp_zero;
  while (sp_http_headers_it_next(&it, &header)) {
    if (sp_http_ci_equal(header.name, name)) return header.value;
  }
  return sp_zero_s(sp_str_t);
}

bool sp_http_headers_has(sp_str_t headers, sp_str_t name) {
  sp_http_headers_it_t it = sp_http_headers_it(headers);
  sp_http_header_t header = sp_zero;
  while (sp_http_headers_it_next(&it, &header)) {
    if (sp_http_ci_equal(header.name, name)) return true;
  }
  return false;
}

sp_http_error_t sp_http_request_head_parse(sp_str_t head, sp_http_request_head_t* out) {
  *out = sp_zero_s(sp_http_request_head_t);
  s32 nl = sp_str_find(head, sp_str_lit("\r\n"));
  sp_str_t line = nl == SP_STR_NO_MATCH ? head : sp_str_sub(head, 0, nl);

  s32 sp1 = sp_str_find_c8(line, ' ');
  if (sp1 == SP_STR_NO_MATCH) return SP_HTTP_ERR_PROTOCOL;
  sp_str_t method = sp_str_sub(line, 0, sp1);
  sp_str_t after = sp_http_str_tail(line, sp1 + 1);
  s32 sp2 = sp_str_find_c8(after, ' ');
  if (sp2 == SP_STR_NO_MATCH) return SP_HTTP_ERR_PROTOCOL;
  sp_str_t target = sp_str_sub(after, 0, sp2);
  sp_str_t version = sp_http_str_tail(after, sp2 + 1);

  if (!sp_http_token_ok(method)) return SP_HTTP_ERR_PROTOCOL;
  if (sp_str_empty(target) || !sp_http_url_path_ok(target)) return SP_HTTP_ERR_PROTOCOL;
  bool v11 = sp_str_equal_cstr(version, "HTTP/1.1");
  if (!v11 && !sp_str_equal_cstr(version, "HTTP/1.0")) return SP_HTTP_ERR_PROTOCOL;

  out->method = method;
  out->target = target;
  out->version = v11 ? SP_HTTP_VERSION_1_1 : SP_HTTP_VERSION_1_0;
  if (nl == SP_STR_NO_MATCH) return SP_HTTP_OK;
  sp_str_t lines = sp_http_str_tail(head, nl + 2);
  sp_http_error_t err = sp_http_header_lines_check(lines);
  if (err != SP_HTTP_OK) return err;
  out->headers = lines;
  return SP_HTTP_OK;
}

sp_http_error_t sp_http_response_head_parse(sp_str_t head, sp_http_response_head_t* out) {
  *out = sp_zero_s(sp_http_response_head_t);
  s32 nl = sp_str_find(head, sp_str_lit("\r\n"));
  sp_str_t line = nl == SP_STR_NO_MATCH ? head : sp_str_sub(head, 0, nl);
  sp_http_error_t err = sp_http_status_line_parse(line, &out->status);
  if (err != SP_HTTP_OK) return err;
  if (nl == SP_STR_NO_MATCH) return SP_HTTP_OK;
  sp_str_t lines = sp_http_str_tail(head, nl + 2);
  err = sp_http_header_lines_check(lines);
  if (err != SP_HTTP_OK) return err;
  out->headers = lines;
  return SP_HTTP_OK;
}

sp_http_error_t sp_http_body_parse(sp_str_t headers, sp_http_body_t* out) {
  *out = sp_zero_s(sp_http_body_t);
  bool chunked = false;
  bool has_length = false;
  u64 length = 0;
  sp_http_headers_it_t it = sp_http_headers_it(headers);
  sp_http_header_t header = sp_zero;
  while (sp_http_headers_it_next(&it, &header)) {
    if (sp_http_ci_equal(header.name, sp_str_lit("content-length"))) {
      u64 value = 0;
      if (!sp_parse_u64_ex(header.value, &value)) return SP_HTTP_ERR_PROTOCOL;
      if (has_length && length != value) return SP_HTTP_ERR_PROTOCOL;
      has_length = true;
      length = value;
    }
    else if (sp_http_ci_equal(header.name, sp_str_lit("transfer-encoding"))) {
      if (!sp_http_ci_contains(header.value, sp_str_lit("chunked"))) return SP_HTTP_ERR_PROTOCOL;
      chunked = true;
    }
  }
  if (chunked && has_length) return SP_HTTP_ERR_PROTOCOL;
  if (chunked) {
    out->kind = SP_HTTP_BODY_CHUNKED;
  }
  else if (has_length) {
    out->kind = SP_HTTP_BODY_LENGTH;
    out->length = length;
  }
  return SP_HTTP_OK;
}

sp_http_error_t sp_http_response_body_parse(s32 status, bool head_request, sp_str_t headers, sp_http_body_t* out) {
  *out = sp_zero_s(sp_http_body_t);
  bool interim = status >= 100 && status <= 199;
  if (head_request || interim || status == 204 || status == 304) return SP_HTTP_OK;
  sp_http_error_t err = sp_http_body_parse(headers, out);
  if (err != SP_HTTP_OK) return err;
  // a response with no framing headers is delimited by connection close
  if (out->kind == SP_HTTP_BODY_NONE) out->kind = SP_HTTP_BODY_EOF;
  return SP_HTTP_OK;
}

sp_str_t sp_http_method_name(sp_http_method_t method) {
  switch (method) {
    case SP_HTTP_GET:    return sp_str_lit("GET");
    case SP_HTTP_POST:   return sp_str_lit("POST");
    case SP_HTTP_PUT:    return sp_str_lit("PUT");
    case SP_HTTP_PATCH:  return sp_str_lit("PATCH");
    case SP_HTTP_DELETE: return sp_str_lit("DELETE");
    case SP_HTTP_HEAD:   return sp_str_lit("HEAD");
  }
  return sp_zero_s(sp_str_t);
}

bool sp_http_method_parse(sp_str_t name, sp_http_method_t* out) {
  const sp_http_method_t methods [] = {
    SP_HTTP_GET, SP_HTTP_POST, SP_HTTP_PUT, SP_HTTP_PATCH, SP_HTTP_DELETE, SP_HTTP_HEAD,
  };
  sp_carr_for(methods, it) {
    if (sp_str_equal(name, sp_http_method_name(methods[it]))) {
      *out = methods[it];
      return true;
    }
  }
  return false;
}

sp_str_t sp_http_status_reason(s32 status) {
  switch (status) {
    case 100: return sp_str_lit("Continue");
    case 101: return sp_str_lit("Switching Protocols");
    case 200: return sp_str_lit("OK");
    case 201: return sp_str_lit("Created");
    case 202: return sp_str_lit("Accepted");
    case 204: return sp_str_lit("No Content");
    case 206: return sp_str_lit("Partial Content");
    case 301: return sp_str_lit("Moved Permanently");
    case 302: return sp_str_lit("Found");
    case 303: return sp_str_lit("See Other");
    case 304: return sp_str_lit("Not Modified");
    case 307: return sp_str_lit("Temporary Redirect");
    case 308: return sp_str_lit("Permanent Redirect");
    case 400: return sp_str_lit("Bad Request");
    case 401: return sp_str_lit("Unauthorized");
    case 403: return sp_str_lit("Forbidden");
    case 404: return sp_str_lit("Not Found");
    case 405: return sp_str_lit("Method Not Allowed");
    case 408: return sp_str_lit("Request Timeout");
    case 409: return sp_str_lit("Conflict");
    case 411: return sp_str_lit("Length Required");
    case 413: return sp_str_lit("Content Too Large");
    case 414: return sp_str_lit("URI Too Long");
    case 415: return sp_str_lit("Unsupported Media Type");
    case 429: return sp_str_lit("Too Many Requests");
    case 431: return sp_str_lit("Request Header Fields Too Large");
    case 500: return sp_str_lit("Internal Server Error");
    case 501: return sp_str_lit("Not Implemented");
    case 502: return sp_str_lit("Bad Gateway");
    case 503: return sp_str_lit("Service Unavailable");
    case 504: return sp_str_lit("Gateway Timeout");
  }
  return sp_zero_s(sp_str_t);
}

sp_http_error_t sp_http_headers_check(const sp_http_header_t* headers, u32 count) {
  sp_for(it, count) {
    if (!sp_http_token_ok(headers[it].name)) return SP_HTTP_ERR_BAD_CONFIG;
    if (!sp_http_header_value_ok(headers[it].value)) return SP_HTTP_ERR_BAD_CONFIG;
  }
  return SP_HTTP_OK;
}

SP_PRIVATE bool sp_http_location_is_absolute(sp_str_t location) {
  s32 sep = sp_str_find(location, sp_str_lit("://"));
  if (sep == SP_STR_NO_MATCH || sep == 0) return false;
  sp_for(it, (u32)sep) {
    c8 c = location.data[it];
    bool alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    bool digit = (c >= '0' && c <= '9');
    if (it == 0 && !alpha) return false;
    if (!alpha && !digit && c != '+' && c != '-' && c != '.') return false;
  }
  return true;
}

SP_PRIVATE sp_str_t sp_http_resolve_url(sp_mem_t mem, sp_http_url_t base, sp_str_t location) {
  if (sp_http_location_is_absolute(location)) {
    return sp_str_copy(mem, location);
  }
  sp_str_t scheme = base.tls ? sp_str_lit("https") : sp_str_lit("http");
  if (location.len >= 2 && location.data[0] == '/' && location.data[1] == '/') {
    return sp_fmt(mem, "{}:{}", sp_fmt_str(scheme), sp_fmt_str(location)).value;
  }
  if (!sp_str_empty(location) && location.data[0] == '/') {
    return sp_fmt(mem, "{}://{}:{}{}",
      sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(location)).value;
  }
  s32 slash = sp_str_find_c8_reverse(base.path, '/');
  sp_str_t dir = slash == SP_STR_NO_MATCH ? sp_str_lit("/") : sp_str_sub(base.path, 0, slash + 1);
  return sp_fmt(mem, "{}://{}:{}{}{}",
    sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(dir), sp_fmt_str(location)).value;
}

SP_PRIVATE bool sp_http_no_proxy_match(sp_str_t host, sp_str_t no_proxy) {
  sp_str_t rest = no_proxy;
  while (!sp_str_empty(rest)) {
    s32 comma = sp_str_find_c8(rest, ',');
    sp_str_t entry = comma == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, comma);
    rest = comma == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, comma + 1);

    entry = sp_str_trim(entry);
    if (sp_str_empty(entry)) continue;
    if (sp_str_equal_cstr(entry, "*")) return true;
    if (entry.data[0] == '.') entry = sp_http_str_tail(entry, 1);
    if (sp_str_empty(entry)) continue;
    if (sp_http_ci_equal(host, entry)) return true;
    if (host.len > entry.len &&
        host.data[host.len - entry.len - 1] == '.' &&
        sp_http_ci_equal(sp_http_str_tail(host, (s32)(host.len - entry.len)), entry)) {
      return true;
    }
  }
  return false;
}

SP_PRIVATE sp_str_t sp_http_proxy_pick(sp_http_url_t url, sp_str_t http_proxy, sp_str_t https_proxy, sp_str_t all_proxy, sp_str_t no_proxy) {
  if (sp_http_no_proxy_match(sp_http_host_bare(url.host), no_proxy)) return sp_zero_s(sp_str_t);
  sp_str_t proxy = url.tls ? https_proxy : http_proxy;
  if (sp_str_empty(proxy)) proxy = all_proxy;
  return proxy;
}

SP_PRIVATE sp_str_t sp_http_env_either(const c8* lower, const c8* upper) {
  sp_str_t value = sp_os_env_get(sp_cstr_as_str(lower));
  if (sp_str_empty(value) && upper) value = sp_os_env_get(sp_cstr_as_str(upper));
  return value;
}

SP_PRIVATE sp_str_t sp_http_proxy_from_env(sp_http_url_t url) {
  return sp_http_proxy_pick(url,
    // uppercase HTTP_PROXY is deliberately ignored (CGI can set it from a request header)
    sp_http_env_either("http_proxy", SP_NULLPTR),
    sp_http_env_either("https_proxy", "HTTPS_PROXY"),
    sp_http_env_either("all_proxy", "ALL_PROXY"),
    sp_http_env_either("no_proxy", "NO_PROXY"));
}

SP_PRIVATE sp_http_error_t sp_http_map_io(sp_err_t err, sp_http_error_t fallback) {
  if (err == SP_ERR_IO_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_OS;
  if (err >= SP_ERR_SYS && err < SP_ERR_SYS + 100) return SP_HTTP_ERR_OS;
  return fallback;
}

SP_PRIVATE u32 sp_http_timeout_ms(u32 requested, u32 fallback) {
  if (requested == SP_HTTP_TIMEOUT_INFINITE) return 0;
  return requested ? requested : fallback;
}

sp_http_error_t sp_http_head_read(sp_io_reader_t* reader, sp_str_t* head) {
  sp_str_t acc = sp_zero;
  sp_err_t err = sp_io_peek_until(reader, sp_str_lit("\r\n\r\n"), &acc);
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_PROTOCOL;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  sp_io_consume(reader, acc.len);
  *head = sp_str_sub(acc, 0, (s32)acc.len - 4);
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_read_line(sp_io_reader_t* reader, sp_str_t* line) {
  sp_str_t acc = sp_zero;
  sp_err_t err = sp_io_peek_until(reader, sp_str_lit("\r\n"), &acc);
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_PROTOCOL;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  sp_io_consume(reader, acc.len);
  *line = sp_str_sub(acc, 0, (s32)acc.len - 2);
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_chunk_begin(sp_http_body_reader_t* r) {
  for (;;) {
    sp_str_t line = sp_zero;
    sp_http_error_t err = sp_http_read_line(r->inner, &line);
    if (err != SP_HTTP_OK) return err;
    if (r->line_pending) {
      if (!sp_str_empty(line)) return SP_HTTP_ERR_PROTOCOL;
      r->line_pending = false;
      continue;
    }
    s32 semi = sp_str_find_c8(line, ';');
    sp_str_t size_str = semi == SP_STR_NO_MATCH ? line : sp_str_sub(line, 0, semi);
    size_str = sp_str_trim(size_str);
    u64 size = 0;
    if (!sp_parse_hex_ex(size_str, &size)) return SP_HTTP_ERR_PROTOCOL;
    if (size) {
      r->remaining = size;
      return SP_HTTP_OK;
    }
    for (;;) {
      sp_str_t trailer = sp_zero;
      err = sp_http_read_line(r->inner, &trailer);
      if (err != SP_HTTP_OK) return err;
      if (sp_str_empty(trailer)) {
        r->done = true;
        return SP_HTTP_OK;
      }
    }
  }
}

SP_PRIVATE sp_err_t sp_http_body_reader_advance(sp_http_body_reader_t* r, void* dst, u64 size, u64* moved) {
  *moved = 0;
  if (r->err != SP_HTTP_OK) return SP_ERR_IO;
  if (r->done) return SP_ERR_IO_EOF;

  switch (r->kind) {
    case SP_HTTP_BODY_NONE: {
      r->done = true;
      return SP_ERR_IO_EOF;
    }
    case SP_HTTP_BODY_EOF: {
      sp_err_t err = dst
        ? sp_io_read(r->inner, dst, size, moved)
        : sp_io_discard(r->inner, size, moved);
      if (err == SP_ERR_IO_EOF) r->done = true;
      return err;
    }
    case SP_HTTP_BODY_LENGTH: {
      if (!r->remaining) {
        r->done = true;
        return SP_ERR_IO_EOF;
      }
      u64 n = sp_min(size, r->remaining);
      sp_err_t err = dst
        ? sp_io_read(r->inner, dst, n, moved)
        : sp_io_discard(r->inner, n, moved);
      r->remaining -= *moved;
      if (err == SP_ERR_IO_EOF && r->remaining) {
        r->err = SP_HTTP_ERR_PROTOCOL;
        return SP_ERR_IO;
      }
      return err;
    }
    case SP_HTTP_BODY_CHUNKED: {
      if (!r->remaining) {
        sp_http_error_t herr = sp_http_chunk_begin(r);
        if (herr != SP_HTTP_OK) {
          r->err = herr;
          return SP_ERR_IO;
        }
        if (r->done) return SP_ERR_IO_EOF;
      }
      u64 n = sp_min(size, r->remaining);
      sp_err_t err = dst
        ? sp_io_read(r->inner, dst, n, moved)
        : sp_io_discard(r->inner, n, moved);
      r->remaining -= *moved;
      if (!r->remaining) r->line_pending = true;
      if (err == SP_ERR_IO_EOF) {
        r->err = SP_HTTP_ERR_PROTOCOL;
        return SP_ERR_IO;
      }
      return err;
    }
  }
  return SP_ERR_IO;
}

SP_PRIVATE sp_err_t sp_http_body_reader_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  sp_http_body_reader_t* r = (sp_http_body_reader_t*)reader;
  u64 moved = 0;
  sp_err_t err = sp_http_body_reader_advance(r, ptr, size, &moved);
  if (bytes_read) *bytes_read = moved;
  return err;
}

SP_PRIVATE sp_err_t sp_http_body_reader_discard(sp_io_reader_t* reader, u64 n, u64* discarded) {
  sp_http_body_reader_t* r = (sp_http_body_reader_t*)reader;
  return sp_http_body_reader_advance(r, SP_NULLPTR, n, discarded);
}

void sp_http_body_reader_init(sp_http_body_reader_t* r, sp_io_reader_t* inner, sp_http_body_t body) {
  *r = sp_zero_s(sp_http_body_reader_t);
  r->base.read = sp_http_body_reader_read;
  r->base.discard = sp_http_body_reader_discard;
  r->inner = inner;
  r->kind = body.kind;
  if (body.kind == SP_HTTP_BODY_LENGTH) r->remaining = body.length;
}

sp_http_error_t sp_http_body_read(sp_io_reader_t* reader, sp_http_body_t body, sp_io_writer_t* sink, u64* len) {
  sp_http_body_reader_t r = sp_zero;
  sp_http_body_reader_init(&r, reader, body);
  u64 moved = 0;
  sp_err_t err = sink
    ? sp_io_copy(sink, &r.base, &moved)
    : sp_io_discard(&r.base, SP_LIMIT_U64_MAX, &moved);
  if (len) *len = moved;
  if (r.err != SP_HTTP_OK) return r.err;
  if (!sink && err == SP_ERR_IO_EOF) err = SP_OK;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  return SP_HTTP_OK;
}

SP_PRIVATE bool sp_http_header_list_have(const sp_http_header_t* headers, u32 count, sp_str_t name) {
  sp_for(it, count) {
    if (sp_http_ci_equal(headers[it].name, name)) return true;
  }
  return false;
}

SP_PRIVATE bool sp_http_headers_reserved(const sp_http_header_t* headers, u32 count) {
  return sp_http_header_list_have(headers, count, sp_str_lit("content-length")) ||
         sp_http_header_list_have(headers, count, sp_str_lit("transfer-encoding")) ||
         sp_http_header_list_have(headers, count, sp_str_lit("connection"));
}

SP_PRIVATE sp_http_error_t sp_http_header_write(sp_io_writer_t* writer, sp_http_header_t header) {
  sp_err_t err = sp_fmt_io(writer, "{}: {}\r\n", sp_fmt_str(header.name), sp_fmt_str(header.value));
  return err == SP_OK ? SP_HTTP_OK : sp_http_map_io(err, SP_HTTP_ERR_OS);
}

SP_PRIVATE sp_http_error_t sp_http_headers_write(sp_io_writer_t* writer, const sp_http_header_t* headers, u32 count) {
  sp_http_error_t err = sp_http_headers_check(headers, count);
  if (err != SP_HTTP_OK) return err;
  sp_for(it, count) {
    err = sp_http_header_write(writer, headers[it]);
    if (err != SP_HTTP_OK) return err;
  }
  return SP_HTTP_OK;
}

sp_http_error_t sp_http_request_head_write(sp_io_writer_t* writer, sp_str_t method, sp_str_t target, const sp_http_header_t* headers, u32 count) {
  if (!sp_http_token_ok(method)) return SP_HTTP_ERR_BAD_CONFIG;
  if (sp_str_empty(target) || !sp_http_url_path_ok(target)) return SP_HTTP_ERR_BAD_CONFIG;
  sp_err_t err = sp_fmt_io(writer, "{} {} HTTP/1.1\r\n", sp_fmt_str(method), sp_fmt_str(target));
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_OS);
  sp_http_error_t herr = sp_http_headers_write(writer, headers, count);
  if (herr != SP_HTTP_OK) return herr;
  err = sp_io_write_str(writer, sp_str_lit("\r\n"), SP_NULLPTR);
  return err == SP_OK ? SP_HTTP_OK : sp_http_map_io(err, SP_HTTP_ERR_OS);
}

SP_PRIVATE sp_http_error_t sp_http_response_head_io(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count) {
  if (status < 100 || status > 599) return SP_HTTP_ERR_BAD_CONFIG;
  sp_err_t err = sp_fmt_io(writer, "HTTP/1.1 {} {}\r\n", sp_fmt_int(status), sp_fmt_str(sp_http_status_reason(status)));
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_OS);
  return sp_http_headers_write(writer, headers, count);
}

sp_http_error_t sp_http_response_head_write(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count) {
  sp_http_error_t err = sp_http_response_head_io(writer, status, headers, count);
  if (err != SP_HTTP_OK) return err;
  sp_err_t werr = sp_io_write_str(writer, sp_str_lit("\r\n"), SP_NULLPTR);
  return werr == SP_OK ? SP_HTTP_OK : sp_http_map_io(werr, SP_HTTP_ERR_OS);
}

sp_http_error_t sp_http_response_write(sp_io_writer_t* writer, s32 status, const sp_http_header_t* headers, u32 count, sp_str_t body) {
  if (sp_http_header_list_have(headers, count, sp_str_lit("content-length")) ||
      sp_http_header_list_have(headers, count, sp_str_lit("transfer-encoding"))) {
    return SP_HTTP_ERR_BAD_CONFIG;
  }
  sp_http_error_t err = sp_http_response_head_io(writer, status, headers, count);
  if (err != SP_HTTP_OK) return err;
  sp_err_t werr = sp_fmt_io(writer, "Content-Length: {}\r\n\r\n", sp_fmt_uint(body.len));
  if (werr != SP_OK) return sp_http_map_io(werr, SP_HTTP_ERR_OS);
  werr = sp_io_write_str(writer, body, SP_NULLPTR);
  return werr == SP_OK ? SP_HTTP_OK : sp_http_map_io(werr, SP_HTTP_ERR_OS);
}

SP_PRIVATE sp_str_t sp_http_build_request(sp_mem_t mem, sp_http_wire_t wire) {
  sp_http_url_t url = wire.url;
  bool default_port =
    (url.tls && sp_str_equal_cstr(url.port, "443")) ||
    (!url.tls && sp_str_equal_cstr(url.port, "80"));
  sp_str_t host_header = default_port
    ? url.host
    : sp_fmt(mem, "{}:{}", sp_fmt_str(url.host), sp_fmt_str(url.port)).value;
  sp_str_t path = url.path;
  if (!sp_str_empty(path) && path.data[0] == '?') {
    path = sp_fmt(mem, "/{}", sp_fmt_str(path)).value;
  }

  sp_io_dyn_mem_writer_t head = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &head);
  if (wire.absolute_form) {
    sp_fmt_io(&head.base, "{} http://{}{} HTTP/1.1\r\n", sp_fmt_str(sp_http_method_name(wire.method)), sp_fmt_str(host_header), sp_fmt_str(path));
  }
  else {
    sp_fmt_io(&head.base, "{} {} HTTP/1.1\r\n", sp_fmt_str(sp_http_method_name(wire.method)), sp_fmt_str(path));
  }
  if (!sp_http_header_list_have(wire.headers, wire.num_headers, sp_str_lit("host"))) {
    sp_fmt_io(&head.base, "Host: {}\r\n", sp_fmt_str(host_header));
  }
  if (!sp_http_header_list_have(wire.headers, wire.num_headers, sp_str_lit("user-agent"))) {
    sp_io_write_str(&head.base, sp_str_lit("User-Agent: sp-http/1.0\r\n"), SP_NULLPTR);
  }
  if (!sp_http_header_list_have(wire.headers, wire.num_headers, sp_str_lit("accept"))) {
    sp_io_write_str(&head.base, sp_str_lit("Accept: */*\r\n"), SP_NULLPTR);
  }
  sp_io_write_str(&head.base, sp_str_lit("Connection: close\r\n"), SP_NULLPTR);
  bool body_expected =
    wire.method == SP_HTTP_POST ||
    wire.method == SP_HTTP_PUT ||
    wire.method == SP_HTTP_PATCH;
  if (body_expected || !sp_str_empty(wire.payload)) {
    sp_fmt_io(&head.base, "Content-Length: {}\r\n", sp_fmt_uint(wire.payload.len));
  }
  if (!sp_str_empty(wire.payload) && !sp_str_empty(wire.content_type) && !sp_http_header_list_have(wire.headers, wire.num_headers, sp_str_lit("content-type"))) {
    sp_fmt_io(&head.base, "Content-Type: {}\r\n", sp_fmt_str(wire.content_type));
  }
  sp_for(it, wire.num_headers) {
    sp_http_header_t header = wire.headers[it];
    if (wire.strip_auth &&
        (sp_http_ci_equal(header.name, sp_str_lit("authorization")) ||
         sp_http_ci_equal(header.name, sp_str_lit("cookie")))) {
      continue;
    }
    sp_http_header_write(&head.base, header);
  }
  sp_io_write_str(&head.base, sp_str_lit("\r\n"), SP_NULLPTR);
  return sp_io_dyn_mem_writer_as_str(&head);
}

// a proxy url; scheme defaults to http, and only plain-http proxies are supported
SP_PRIVATE bool sp_http_proxy_url_parse(sp_mem_t mem, sp_str_t proxy, sp_http_url_t* out) {
  if (sp_str_find(proxy, sp_str_lit("://")) == SP_STR_NO_MATCH) {
    proxy = sp_fmt(mem, "http://{}", sp_fmt_str(proxy)).value;
  }
  if (!sp_http_url_parse(proxy, out)) return false;
  return !out->tls;
}

#if defined(SP_HTTP_SOCKETS)

#if defined(SP_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#define SP_HTTP_BUFFER_SIZE 16384
#define SP_HTTP_HEAD_MAX    SP_HTTP_BUFFER_SIZE
#define SP_HTTP_MAX_INTERIM 8

struct sp_http_transport {
  sp_sys_socket_t       socket;
  sp_tls_t*             tls;
  u32                   io_timeout_ms;
  sp_io_socket_reader_t sock_reader;
  sp_io_socket_writer_t sock_writer;
  sp_io_reader_t*       reader;
  sp_io_writer_t*       writer;
  u8                    buffer[SP_HTTP_BUFFER_SIZE];
};

SP_PRIVATE sp_http_error_t sp_http_resolve_system(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count) {
  (void)user_data;
  (void)timeout_ms;
  *count = 0;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  c8* name = sp_str_to_cstr(scratch.mem, host);

  struct addrinfo hints = sp_zero;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  struct addrinfo* list = SP_NULLPTR;
  s32 rc = getaddrinfo(name, SP_NULLPTR, &hints, &list);
  sp_mem_end_scratch(scratch);
  if (rc != 0) return SP_HTTP_ERR_CONNECT;

  u32 found = 0;
  for (struct addrinfo* it = list; it && found < capacity; it = it->ai_next) {
    if (it->ai_family == AF_INET) {
      struct sockaddr_in* sa = (struct sockaddr_in*)it->ai_addr;
      addrs[found] = sp_zero_s(sp_http_addr_t);
      addrs[found].kind = SP_HTTP_ADDR_V4;
      sp_mem_copy(addrs[found].data, &sa->sin_addr, 4);
      found++;
    }
    else if (it->ai_family == AF_INET6) {
      struct sockaddr_in6* sa = (struct sockaddr_in6*)it->ai_addr;
      addrs[found] = sp_zero_s(sp_http_addr_t);
      addrs[found].kind = SP_HTTP_ADDR_V6;
      sp_mem_copy(addrs[found].data, &sa->sin6_addr, 16);
      found++;
    }
  }
  freeaddrinfo(list);

  *count = found;
  return found ? SP_HTTP_OK : SP_HTTP_ERR_CONNECT;
}

sp_http_resolver_t sp_http_resolver_default(void) {
  return (sp_http_resolver_t) { .resolve = sp_http_resolve_system };
}

SP_PRIVATE sp_http_error_t sp_http_connect_addr(sp_sys_socket_t* out, sp_http_addr_t addr, u16 port, u32 timeout_ms) {
  struct sockaddr_storage storage = sp_zero;
  s32 family;
  socklen_t addr_len;
  if (addr.kind == SP_HTTP_ADDR_V4) {
    struct sockaddr_in* sa = (struct sockaddr_in*)&storage;
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sp_mem_copy(&sa->sin_addr, addr.data, 4);
    family = AF_INET;
    addr_len = (socklen_t)sizeof(struct sockaddr_in);
  }
  else {
    struct sockaddr_in6* sa = (struct sockaddr_in6*)&storage;
    sa->sin6_family = AF_INET6;
    sa->sin6_port = htons(port);
    sp_mem_copy(&sa->sin6_addr, addr.data, 16);
    family = AF_INET6;
    addr_len = (socklen_t)sizeof(struct sockaddr_in6);
  }

#if defined(SP_WIN32)
  SOCKET fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == INVALID_SOCKET) return SP_HTTP_ERR_CONNECT;
  sp_sys_socket_set_nonblocking((sp_sys_socket_t)fd);
  bool connected = connect(fd, (struct sockaddr*)&storage, (int)addr_len) == 0;
  bool pending = !connected && WSAGetLastError() == WSAEWOULDBLOCK;
#else
  int fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) return SP_HTTP_ERR_CONNECT;
  sp_sys_socket_set_nonblocking((sp_sys_socket_t)fd);
  bool connected = connect(fd, (struct sockaddr*)&storage, addr_len) == 0;
  bool pending = !connected && errno == EINPROGRESS;
#endif

  sp_http_error_t result = SP_HTTP_ERR_CONNECT;
  if (pending) {
    sp_err_t wait = sp_sys_socket_wait((sp_sys_socket_t)fd, false, timeout_ms);
    if (wait == SP_ERR_SYS_TIMED_OUT) result = SP_HTTP_ERR_TIMEOUT;
    if (wait == SP_OK) connected = sp_sys_socket_error((sp_sys_socket_t)fd) == SP_OK;
  }

  if (connected) {
    *out = (sp_sys_socket_t)fd;
    return SP_HTTP_OK;
  }

  sp_sys_socket_close((sp_sys_socket_t)fd);
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_net_connect(sp_sys_socket_t* out, sp_http_resolver_t resolver, sp_str_t host, u16 port, u32 timeout_ms) {
#if defined(SP_WIN32)
  static bool wsa_init = false;
  if (!wsa_init) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return SP_HTTP_ERR_OS;
    wsa_init = true;
  }
#endif

  sp_http_addr_t addrs [SP_HTTP_MAX_ADDRS];
  u32 count = 0;
  if (sp_http_addr_parse(host, &addrs[0])) {
    count = 1;
  }
  else {
    sp_http_error_t err = resolver.resolve(resolver.user_data, host, timeout_ms, addrs, SP_HTTP_MAX_ADDRS, &count);
    if (err != SP_HTTP_OK) return err;
    if (!count) return SP_HTTP_ERR_CONNECT;
  }

  sp_http_error_t result = SP_HTTP_ERR_CONNECT;
  sp_for(it, count) {
    result = sp_http_connect_addr(out, addrs[it], port, timeout_ms);
    if (result == SP_HTTP_OK) break;
  }
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_transport_write(sp_http_transport_t* conn, sp_str_t data) {
  sp_err_t err = sp_io_write_all(conn->writer, data.data, data.len, SP_NULLPTR);
  return err == SP_OK ? SP_HTTP_OK : sp_http_map_io(err, SP_HTTP_ERR_OS);
}

SP_PRIVATE sp_http_error_t sp_http_connect_reply(sp_http_transport_t* conn) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  c8* head = sp_alloc_n(scratch.mem, c8, SP_HTTP_HEAD_MAX);
  u64 len = 0;

  sp_http_error_t result = SP_HTTP_ERR_PROXY;
  while (len < SP_HTTP_HEAD_MAX) {
    u64 n = 0;
    sp_err_t err = sp_io_read(conn->reader, head + len, 1, &n);
    if (err != SP_OK) {
      result = sp_http_map_io(err, SP_HTTP_ERR_PROXY);
      break;
    }
    len += n;

    if (sp_str_ends_with(sp_str(head, (u32)len), sp_str_lit("\r\n\r\n"))) {
      sp_http_response_head_t parsed = sp_zero;
      if (sp_http_response_head_parse(sp_str(head, (u32)len - 4), &parsed) == SP_HTTP_OK &&
          parsed.status >= 200 && parsed.status <= 299) {
        result = SP_HTTP_OK;
      }
      break;
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_transport_open(sp_http_transport_t* conn, sp_tls_t* tls, sp_http_resolver_t resolver, sp_http_url_t url, const sp_http_url_t* proxy, u32 connect_timeout_ms, u32 io_timeout_ms) {
  conn->socket = SP_SYS_INVALID_SOCKET;
  conn->tls = SP_NULLPTR;
  conn->io_timeout_ms = io_timeout_ms;

  sp_http_url_t target = proxy ? *proxy : url;
  u32 port = 0;
  sp_parse_u32_ex(target.port, &port);

  sp_http_error_t err = sp_http_net_connect(&conn->socket, resolver, sp_http_host_bare(target.host), (u16)port, connect_timeout_ms);
  if (err != SP_HTTP_OK) return err;

  sp_io_socket_reader_init(&conn->sock_reader, conn->socket, io_timeout_ms);
  sp_io_socket_writer_init(&conn->sock_writer, conn->socket, io_timeout_ms);
  conn->reader = &conn->sock_reader.base;
  conn->writer = &conn->sock_writer.base;

  if (proxy && url.tls) {
    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
    sp_str_t connect_req = sp_fmt(scratch.mem,
      "CONNECT {}:{} HTTP/1.1\r\n"
      "Host: {}:{}\r\n"
      "\r\n",
      sp_fmt_str(url.host), sp_fmt_str(url.port), sp_fmt_str(url.host), sp_fmt_str(url.port)).value;
    err = sp_http_transport_write(conn, connect_req);
    sp_mem_end_scratch(scratch);
    if (err != SP_HTTP_OK) return SP_HTTP_ERR_PROXY;
    err = sp_http_connect_reply(conn);
    if (err != SP_HTTP_OK) return err;
  }

  if (!url.tls) return SP_HTTP_OK;

  err = tls->open(tls, conn->socket, sp_http_host_bare(url.host), io_timeout_ms);
  if (err != SP_HTTP_OK) return err;
  conn->tls = tls;
  conn->reader = tls->reader;
  conn->writer = tls->writer;
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_http_transport_close(sp_http_transport_t* conn) {
  if (conn->tls) conn->tls->close(conn->tls);
  if (conn->socket != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(conn->socket);
  conn->tls = SP_NULLPTR;
  conn->socket = SP_SYS_INVALID_SOCKET;
}

sp_http_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  sp_http_response_t resp = sp_zero_s(sp_http_response_t);
  u32 max = request.max_redirects ? request.max_redirects : SP_HTTP_DEFAULT_REDIRECTS;
  u32 connect_timeout = sp_http_timeout_ms(request.connect_timeout_ms, SP_HTTP_DEFAULT_CONNECT_TIMEOUT_MS);
  u32 io_timeout = sp_http_timeout_ms(request.io_timeout_ms, SP_HTTP_DEFAULT_IO_TIMEOUT_MS);
  sp_http_resolver_t resolver = request.resolver.resolve ? request.resolver : sp_http_resolver_default();
  sp_str_t current = request.url;
  sp_http_error_t result = sp_http_headers_check(request.headers, request.num_headers);
  if (result == SP_HTTP_OK && sp_http_headers_reserved(request.headers, request.num_headers)) result = SP_HTTP_ERR_BAD_CONFIG;
  u32 redirects = 0;

  if (result != SP_HTTP_OK) {
    if (response) *response = resp;
    return result;
  }

  sp_http_method_t method = request.method;
  sp_str_t payload = request.payload;
  sp_str_t content_type = request.content_type;
  sp_str_t origin_host = sp_zero;
  bool origin_tls = false;
  bool origin_set = false;
  result = SP_HTTP_ERR_PROTOCOL;

  for (;;) {
    sp_http_url_t url = sp_zero_s(sp_http_url_t);
    if (!sp_http_url_parse(current, &url)) {
      result = SP_HTTP_ERR_URL;
      break;
    }
    if (url.tls && !request.tls) {
      result = SP_HTTP_ERR_BAD_CONFIG;
      break;
    }
    if (!origin_set) {
      origin_host = url.host;
      origin_tls = url.tls;
      origin_set = true;
    }

    sp_str_t proxy_str = request.proxy;
    if (sp_str_empty(proxy_str) && !request.no_proxy) proxy_str = sp_http_proxy_from_env(url);
    sp_http_url_t proxy_url = sp_zero_s(sp_http_url_t);
    bool use_proxy = !sp_str_empty(proxy_str);
    if (use_proxy && !sp_http_proxy_url_parse(mem, proxy_str, &proxy_url)) {
      result = SP_HTTP_ERR_PROXY;
      break;
    }

    sp_http_transport_t conn = sp_zero_s(sp_http_transport_t);
    result = sp_http_transport_open(&conn, request.tls, resolver, url, use_proxy ? &proxy_url : SP_NULLPTR, connect_timeout, io_timeout);
    if (result != SP_HTTP_OK) {
      sp_http_transport_close(&conn);
      break;
    }

    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(mem);
    result = sp_http_transport_write(&conn, sp_http_build_request(scratch.mem, (sp_http_wire_t) {
      .url           = url,
      .absolute_form = use_proxy && !url.tls,
      .method        = method,
      .payload       = payload,
      .content_type  = content_type,
      .headers       = request.headers,
      .num_headers   = request.num_headers,
      .strip_auth    = !sp_http_ci_equal(url.host, origin_host) || (origin_tls && !url.tls),
    }));
    sp_mem_end_scratch(scratch);
    if (result == SP_HTTP_OK && !sp_str_empty(payload)) {
      result = sp_http_transport_write(&conn, payload);
    }
    if (result != SP_HTTP_OK) {
      sp_http_transport_close(&conn);
      break;
    }

    sp_io_reader_set_buffer(conn.reader, conn.buffer, sizeof(conn.buffer));

    sp_http_response_head_t parsed = sp_zero;
    u32 interim = 0;
    for (;;) {
      sp_str_t head = sp_zero;
      result = sp_http_head_read(conn.reader, &head);
      if (result != SP_HTTP_OK) break;

      result = sp_http_response_head_parse(head, &parsed);
      if (result != SP_HTTP_OK) break;

      if (parsed.status >= 100 && parsed.status <= 199) {
        if (parsed.status == 101 || ++interim >= SP_HTTP_MAX_INTERIM) {
          result = SP_HTTP_ERR_PROTOCOL;
          break;
        }
        continue;
      }
      break;
    }
    if (result != SP_HTTP_OK) {
      sp_http_transport_close(&conn);
      break;
    }

    resp.status = parsed.status;
    resp.url = current;

    sp_str_t location = sp_http_headers_find(parsed.headers, sp_str_lit("location"));
    bool is_redirect =
      parsed.status == 301 || parsed.status == 302 || parsed.status == 303 ||
      parsed.status == 307 || parsed.status == 308;
    if (is_redirect && !sp_str_empty(location)) {
      if (redirects++ >= max) {
        result = SP_HTTP_ERR_REDIRECTS;
        sp_http_transport_close(&conn);
        break;
      }
      bool rewrite =
        (parsed.status == 301 || parsed.status == 302 || parsed.status == 303) &&
        method != SP_HTTP_GET && method != SP_HTTP_HEAD;
      if (rewrite) {
        method = SP_HTTP_GET;
        payload = sp_zero_s(sp_str_t);
        content_type = sp_zero_s(sp_str_t);
      }
      current = sp_http_resolve_url(mem, url, location);
      sp_http_transport_close(&conn);
      continue;
    }

    sp_http_body_t body = sp_zero;
    result = sp_http_response_body_parse(parsed.status, method == SP_HTTP_HEAD, parsed.headers, &body);
    if (result != SP_HTTP_OK) {
      sp_http_transport_close(&conn);
      break;
    }

    resp.headers = sp_str_copy(mem, parsed.headers);

    u64 len = 0;
    result = sp_http_body_read(conn.reader, body, request.sink, &len);
    resp.body_len = len;
    sp_http_transport_close(&conn);

    if (result != SP_HTTP_OK) break;
    result = parsed.status >= 200 && parsed.status < 300 ? SP_HTTP_OK : SP_HTTP_ERR_STATUS;
    break;
  }

  if (response) *response = resp;
  return result;
}

#else

sp_http_resolver_t sp_http_resolver_default(void) {
  return sp_zero_s(sp_http_resolver_t);
}

sp_http_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  (void)mem; (void)request;
  if (response) *response = sp_zero_s(sp_http_response_t);
  return SP_HTTP_ERR_UNSUPPORTED;
}

#endif // SP_HTTP_SOCKETS

///////////////
// PROTOCOL  //
///////////////

sp_http_target_t sp_http_target_split(sp_str_t target) {
  s32 at = sp_str_find_c8(target, '?');
  if (at == SP_STR_NO_MATCH) {
    return (sp_http_target_t) { .path = target };
  }
  return (sp_http_target_t) {
    .path = sp_str_sub(target, 0, at),
    .query = sp_http_str_tail(target, at + 1),
  };
}

sp_str_t sp_http_query_find(sp_str_t query, sp_str_t key) {
  sp_str_t rest = query;
  while (!sp_str_empty(rest)) {
    s32 amp = sp_str_find_c8(rest, '&');
    sp_str_t pair = amp == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, amp);
    rest = amp == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, amp + 1);
    s32 eq = sp_str_find_c8(pair, '=');
    sp_str_t name = eq == SP_STR_NO_MATCH ? pair : sp_str_sub(pair, 0, eq);
    if (sp_str_equal(name, key)) {
      return eq == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(pair, eq + 1);
    }
  }
  return sp_zero_s(sp_str_t);
}

sp_str_t sp_http_percent_decode(sp_mem_t mem, sp_str_t str) {
  c8* out = sp_alloc_n(mem, c8, str.len);
  u32 len = 0;
  u32 it = 0;
  while (it < str.len) {
    u64 value = 0;
    if (str.data[it] == '%' && it + 3 <= str.len && sp_parse_hex_ex(sp_str_sub(str, (s32)it + 1, 2), &value)) {
      out[len++] = (c8)value;
      it += 3;
      continue;
    }
    out[len++] = str.data[it++];
  }
  return sp_str(out, len);
}

sp_str_t sp_http_mime_type(sp_str_t path) {
  static const struct {
    const c8* extension;
    const c8* type;
  } types [] = {
    { "html",  "text/html" },
    { "htm",   "text/html" },
    { "js",    "text/javascript" },
    { "mjs",   "text/javascript" },
    { "css",   "text/css" },
    { "txt",   "text/plain" },
    { "json",  "application/json" },
    { "map",   "application/json" },
    { "xml",   "application/xml" },
    { "wasm",  "application/wasm" },
    { "pdf",   "application/pdf" },
    { "svg",   "image/svg+xml" },
    { "png",   "image/png" },
    { "jpg",   "image/jpeg" },
    { "jpeg",  "image/jpeg" },
    { "gif",   "image/gif" },
    { "webp",  "image/webp" },
    { "ico",   "image/x-icon" },
    { "woff",  "font/woff" },
    { "woff2", "font/woff2" },
    { "ttf",   "font/ttf" },
  };
  s32 dot = sp_str_find_c8_reverse(path, '.');
  s32 slash = sp_str_find_c8_reverse(path, '/');
  if (dot == SP_STR_NO_MATCH || slash > dot) {
    return sp_str_lit("application/octet-stream");
  }
  sp_str_t extension = sp_http_str_tail(path, dot + 1);
  sp_carr_for(types, it) {
    if (sp_http_ci_equal(extension, sp_cstr_as_str(types[it].extension))) {
      return sp_cstr_as_str(types[it].type);
    }
  }
  return sp_str_lit("application/octet-stream");
}

//////////
// CONN //
//////////

#define SP_HTTP_CONN_DEFAULT_HEAD_MAX   16384
#define SP_HTTP_CONN_DEFAULT_BODY_MAX   65536
#define SP_HTTP_CONN_DEFAULT_STREAM_MAX 65536
#define SP_HTTP_CONN_REPLY_HEAD_MIN     1024
#define SP_HTTP_CONN_CONTINUE           sp_str_lit("HTTP/1.1 100 Continue\r\n\r\n")

static sp_http_conn_t* stream_conn(sp_http_stream_t* stream) {
  return (sp_http_conn_t*)((u8*)stream - offsetof(sp_http_conn_t, stream));
}

static bool conn_streaming(const sp_http_conn_t* conn) {
  return conn->state == SP_HTTP_CONN_STREAM && !conn->stream.dead;
}

static u32 reply_head_cap(const sp_http_conn_t* conn) {
  u32 cap = conn->desc.head_max;
  return cap < SP_HTTP_CONN_REPLY_HEAD_MIN ? SP_HTTP_CONN_REPLY_HEAD_MIN : cap;
}

static sp_err_t stream_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  sp_http_stream_t* stream = (sp_http_stream_t*)writer;
  *bytes_written = 0;
  if (stream->dead) return SP_ERR_IO_EOF;
  if (size > stream->cap - stream->len) {
    stream->dead = true;
    stream_conn(stream)->state = SP_HTTP_CONN_CLOSED;
    return SP_ERR_IO_NO_SPACE;
  }
  u32 tail = (stream->head + stream->len) % stream->cap;
  u32 first = sp_min((u32)size, stream->cap - tail);
  sp_mem_copy(stream->ring + tail, ptr, first);
  sp_mem_copy(stream->ring, (const u8*)ptr + first, (u32)size - first);
  stream->len += (u32)size;
  *bytes_written = size;
  return SP_OK;
}

static sp_mem_slice_t stream_segment(sp_http_stream_t* stream) {
  return sp_mem_slice(stream->ring + stream->head, sp_min(stream->len, stream->cap - stream->head));
}

static void stream_consume(sp_http_stream_t* stream, u32 n) {
  stream->head = (stream->head + n) % stream->cap;
  stream->len -= n;
}

static bool out_pending(const sp_http_conn_t* conn) {
  return conn->out.at < conn->out.count;
}

static void out_push(sp_http_conn_t* conn, sp_str_t bytes) {
  conn->out.slices[conn->out.count++] = sp_mem_slice((u8*)bytes.data, bytes.len);
}

static void ctx_clear(sp_http_conn_t* conn) {
  sp_mem_t mem = conn->ctx.mem;
  void* user_data = conn->ctx.user_data;
  conn->ctx = (sp_http_ctx_t) { .mem = mem, .stream = &conn->stream, .user_data = user_data };
}

static void conn_fail(sp_http_conn_t* conn, s32 status) {
  conn->keep_alive = false;
  sp_http_conn_reply(conn, sp_http_reply_text(status, sp_http_status_reason(status)));
}

static void conn_next(sp_http_conn_t* conn) {
  u32 consumed = conn->in.body_at + conn->in.need;
  u32 leftover = conn->in.len - consumed;
  sp_mem_move(conn->in.data, conn->in.data + consumed, leftover);
  conn->in.len = leftover;
  conn->in.scan = 0;
  conn->in.body_at = 0;
  conn->in.need = 0;
  conn->out.at = 0;
  conn->out.count = 0;
  conn->out.cursor = 0;
  conn->keep_alive = false;
  conn->method_known = false;
  sp_mem_arena_clear(conn->arena);
  ctx_clear(conn);
  conn->state = SP_HTTP_CONN_HEAD;
}

static bool conn_head(sp_http_conn_t* conn) {
  sp_str_t buffered = sp_str((c8*)conn->in.data, conn->in.len);
  s32 end = sp_str_find(sp_http_str_tail(buffered, (s32)conn->in.scan), sp_str_lit("\r\n\r\n"));
  if (end == SP_STR_NO_MATCH) {
    if (conn->in.len >= conn->desc.head_max) {
      conn_fail(conn, 431);
      return true;
    }
    conn->in.scan = conn->in.len > 3 ? conn->in.len - 3 : 0;
    return false;
  }
  end += (s32)conn->in.scan;
  if ((u32)end + 4 > conn->desc.head_max) {
    conn_fail(conn, 431);
    return true;
  }

  sp_http_request_head_t head = sp_zero;
  sp_http_body_t body = sp_zero;
  if (sp_http_request_head_parse(sp_str_sub(buffered, 0, end), &head) != SP_HTTP_OK ||
      sp_http_body_parse(head.headers, &body) != SP_HTTP_OK) {
    conn_fail(conn, 400);
    return true;
  }

  u64 need = 0;
  switch (body.kind) {
    case SP_HTTP_BODY_NONE:
    case SP_HTTP_BODY_EOF: {
      need = 0;
      break;
    }
    case SP_HTTP_BODY_LENGTH: {
      need = body.length;
      break;
    }
    case SP_HTTP_BODY_CHUNKED: {
      conn_fail(conn, 411);
      return true;
    }
  }
  if (need > conn->desc.body_max) {
    conn_fail(conn, 413);
    return true;
  }

  sp_http_target_t target = sp_http_target_split(head.target);
  conn->in.body_at = (u32)end + 4;
  conn->in.need = (u32)need;
  conn->method_known = sp_http_method_parse(head.method, &conn->ctx.method);
  conn->ctx.path = sp_http_percent_decode(conn->ctx.mem, target.path);
  conn->ctx.query = target.query;
  conn->ctx.headers = head.headers;
  conn->keep_alive = head.version == SP_HTTP_VERSION_1_1 &&
    !sp_http_ci_contains(sp_http_headers_find(head.headers, sp_str_lit("connection")), sp_str_lit("close"));
  if (need && sp_http_ci_equal(sp_http_headers_find(head.headers, sp_str_lit("expect")), sp_str_lit("100-continue"))) {
    out_push(conn, SP_HTTP_CONN_CONTINUE);
  }
  conn->state = SP_HTTP_CONN_BODY;
  return true;
}

void sp_http_conn_init(sp_http_conn_t* conn, sp_http_conn_desc_t desc) {
  *conn = sp_zero_s(sp_http_conn_t);
  conn->desc = desc;
  if (!conn->desc.head_max) conn->desc.head_max = SP_HTTP_CONN_DEFAULT_HEAD_MAX;
  if (!conn->desc.body_max) conn->desc.body_max = SP_HTTP_CONN_DEFAULT_BODY_MAX;
  if (!conn->desc.stream_max) conn->desc.stream_max = SP_HTTP_CONN_DEFAULT_STREAM_MAX;
  conn->arena = sp_mem_arena_new(conn->desc.mem);
  conn->in.cap = conn->desc.head_max + conn->desc.body_max;
  conn->in.data = sp_alloc_n(conn->desc.mem, u8, conn->in.cap);
  conn->out.head_data = sp_alloc_n(conn->desc.mem, u8, reply_head_cap(conn));
  conn->stream.base.write = stream_write;
  conn->stream.ring = sp_alloc_n(conn->desc.mem, u8, conn->desc.stream_max);
  conn->stream.cap = conn->desc.stream_max;
  conn->ctx.mem = sp_mem_arena_as_allocator(conn->arena);
  sp_http_conn_reset(conn);
}

void sp_http_conn_deinit(sp_http_conn_t* conn) {
  sp_mem_arena_destroy(conn->arena);
  sp_free(conn->desc.mem, conn->in.data, conn->in.cap);
  sp_free(conn->desc.mem, conn->out.head_data, reply_head_cap(conn));
  sp_free(conn->desc.mem, conn->stream.ring, conn->desc.stream_max);
}

void sp_http_conn_reset(sp_http_conn_t* conn) {
  conn->in.len = 0;
  conn->in.body_at = 0;
  conn->in.need = 0;
  conn->stream.head = 0;
  conn->stream.len = 0;
  conn->stream.held = false;
  conn->stream.dead = false;
  conn_next(conn);
}

u32 sp_http_conn_step(sp_http_conn_t* conn) {
  for (;;) {
    switch (conn->state) {
      case SP_HTTP_CONN_HEAD: {
        if (!conn_head(conn)) return SP_HTTP_WANT_RECV;
        break;
      }
      case SP_HTTP_CONN_BODY: {
        bool complete = conn->in.len >= conn->in.body_at + conn->in.need;
        if (out_pending(conn)) {
          return SP_HTTP_WANT_SEND | (complete ? 0 : SP_HTTP_WANT_RECV);
        }
        if (!complete) return SP_HTTP_WANT_RECV;
        conn->ctx.body = sp_str((c8*)conn->in.data + conn->in.body_at, conn->in.need);
        if (!conn->method_known) {
          sp_http_conn_reply(conn, sp_http_reply_text(501, sp_http_status_reason(501)));
          break;
        }
        conn->state = SP_HTTP_CONN_REQUEST;
        break;
      }
      case SP_HTTP_CONN_REQUEST: {
        return SP_HTTP_WANT_REQUEST;
      }
      case SP_HTTP_CONN_REPLY: {
        if (out_pending(conn)) return SP_HTTP_WANT_SEND;
        if (!conn->keep_alive) {
          conn->state = SP_HTTP_CONN_CLOSED;
          break;
        }
        conn_next(conn);
        break;
      }
      case SP_HTTP_CONN_STREAM: {
        bool pending = out_pending(conn) || conn->stream.len;
        if (conn->stream.dead) {
          return pending ? SP_HTTP_WANT_SEND : SP_HTTP_WANT_CLOSE;
        }
        u32 want = pending ? SP_HTTP_WANT_SEND : 0u;
        if (conn->in.len < conn->in.cap) want |= SP_HTTP_WANT_RECV;
        return want;
      }
      case SP_HTTP_CONN_CLOSED: {
        return SP_HTTP_WANT_CLOSE;
      }
    }
  }
}

sp_mem_slice_t sp_http_conn_recv_slot(sp_http_conn_t* conn) {
  return sp_mem_slice(conn->in.data + conn->in.len, conn->in.cap - conn->in.len);
}

void sp_http_conn_received(sp_http_conn_t* conn, u64 n) {
  switch (conn->state) {
    case SP_HTTP_CONN_HEAD:
    case SP_HTTP_CONN_BODY:
    case SP_HTTP_CONN_CLOSED: {
      if (n == 0) conn->state = SP_HTTP_CONN_CLOSED;
      conn->in.len += (u32)n;
      break;
    }
    case SP_HTTP_CONN_REQUEST:
    case SP_HTTP_CONN_REPLY: {
      if (n == 0) conn->keep_alive = false;
      conn->in.len += (u32)n;
      break;
    }
    case SP_HTTP_CONN_STREAM: {
      if (n == 0) {
        conn->stream.dead = true;
        conn->state = SP_HTTP_CONN_CLOSED;
      }
      break;
    }
  }
}

sp_mem_slice_t sp_http_conn_send_slot(sp_http_conn_t* conn) {
  if (out_pending(conn)) {
    return sp_mem_slice_suffix(conn->out.slices[conn->out.at], conn->out.slices[conn->out.at].len - conn->out.cursor);
  }
  if (conn->state == SP_HTTP_CONN_STREAM) {
    return stream_segment(&conn->stream);
  }
  return sp_zero_s(sp_mem_slice_t);
}

void sp_http_conn_sent(sp_http_conn_t* conn, u64 n) {
  if (n == 0) {
    conn->stream.dead = true;
    conn->state = SP_HTTP_CONN_CLOSED;
    return;
  }
  if (out_pending(conn)) {
    conn->out.cursor += n;
    if (conn->out.cursor == conn->out.slices[conn->out.at].len) {
      conn->out.at++;
      conn->out.cursor = 0;
    }
    if (!out_pending(conn)) {
      conn->out.at = 0;
      conn->out.count = 0;
    }
    return;
  }
  stream_consume(&conn->stream, (u32)n);
}

static bool status_has_length(s32 status) {
  return status >= 200 && status != 204 && status != 304;
}

void sp_http_conn_reply(sp_http_conn_t* conn, sp_http_reply_t reply) {
  sp_http_header_t headers [SP_HTTP_REPLY_MAX_HEADERS + 3];
  u32 count = 0;
  if (!sp_str_empty(reply.content_type)) {
    headers[count++] = (sp_http_header_t) { .name = sp_str_lit("Content-Type"), .value = reply.content_type };
  }
  sp_for(it, reply.num_headers) {
    headers[count++] = reply.headers[it];
  }

  bool head_only = conn->method_known && conn->ctx.method == SP_HTTP_HEAD;
  bool has_length = reply.kind == SP_HTTP_REPLY_ONESHOT && status_has_length(reply.status);
  if (reply.kind == SP_HTTP_REPLY_ONESHOT) {
    conn->stream.held = false;
    conn->stream.dead = false;
    conn->stream.head = 0;
    conn->stream.len = 0;
  }
  if (has_length) {
    sp_str_t length = sp_fmt(conn->ctx.mem, "{}", sp_fmt_uint(reply.body.len)).value;
    headers[count++] = (sp_http_header_t) { .name = sp_str_lit("Content-Length"), .value = length };
  }
  if (reply.kind == SP_HTTP_REPLY_STREAM) {
    sp_assert(reply.stream == &conn->stream);
    conn->keep_alive = false;
  }
  if (!conn->keep_alive) {
    headers[count++] = (sp_http_header_t) { .name = sp_str_lit("Connection"), .value = sp_str_lit("close") };
  }

  sp_io_mem_writer_from_buffer(&conn->out.head, conn->out.head_data, reply_head_cap(conn));
  if (sp_http_response_head_write(&conn->out.head.base, reply.status, headers, count) != SP_HTTP_OK) {
    conn->state = SP_HTTP_CONN_CLOSED;
    return;
  }
  out_push(conn, sp_io_mem_writer_as_str(&conn->out.head));
  if (has_length && !head_only && !sp_str_empty(reply.body)) {
    out_push(conn, reply.body);
  }
  conn->state = reply.kind == SP_HTTP_REPLY_STREAM ? SP_HTTP_CONN_STREAM : SP_HTTP_CONN_REPLY;
  if (reply.kind == SP_HTTP_REPLY_STREAM && head_only) {
    conn->stream.dead = true;
    conn->stream.held = false;
    conn->stream.len = 0;
  }
}

static bool route_method_matches(sp_http_method_t route_method, sp_http_method_t method) {
  if (route_method == method) return true;
  return method == SP_HTTP_HEAD && route_method == SP_HTTP_GET;
}

static bool route_matches(const sp_http_route_t* route, sp_str_t path) {
  sp_str_t pattern = sp_cstr_as_str(route->path);
  if (sp_str_ends_with(pattern, sp_str_lit("*"))) {
    return sp_str_starts_with(path, sp_str_sub(pattern, 0, (s32)pattern.len - 1));
  }
  return sp_str_equal(path, pattern);
}

static sp_http_reply_t route_no_method(const sp_http_router_t* router, sp_http_ctx_t* c) {
  sp_io_dyn_mem_writer_t allow = sp_zero;
  sp_io_dyn_mem_writer_init(c->mem, &allow);
  u32 seen = 0;
  sp_for(it, router->count) {
    const sp_http_route_t* route = &router->routes[it];
    if (!route_matches(route, c->path)) continue;
    if (seen & (1u << route->method)) continue;
    if (seen) sp_io_write_str(&allow.base, sp_str_lit(", "), SP_NULLPTR);
    sp_io_write_str(&allow.base, sp_http_method_name(route->method), SP_NULLPTR);
    seen |= 1u << route->method;
  }
  sp_http_reply_t reply = sp_http_reply_status(405);
  sp_http_reply_header(&reply, sp_str_lit("Allow"), sp_io_dyn_mem_writer_as_str(&allow));
  return reply;
}

sp_http_reply_t sp_http_route(const sp_http_router_t* router, sp_http_ctx_t* c) {
  bool path_matched = false;
  sp_for(it, router->count) {
    const sp_http_route_t* route = &router->routes[it];
    if (!route_matches(route, c->path)) continue;
    path_matched = true;
    if (!route_method_matches(route->method, c->method)) continue;
    c->user_data = router->user_data;
    return route->handler(c);
  }
  if (path_matched) return route_no_method(router, c);
  return sp_http_reply_status(404);
}

sp_str_t sp_http_ctx_query(sp_http_ctx_t* c, const c8* key) {
  return sp_http_percent_decode(c->mem, sp_http_query_find(c->query, sp_cstr_as_str(key)));
}

sp_str_t sp_http_ctx_header(sp_http_ctx_t* c, const c8* name) {
  return sp_http_headers_find(c->headers, sp_cstr_as_str(name));
}

sp_http_stream_t* sp_http_ctx_stream(sp_http_ctx_t* c) {
  c->stream->held = true;
  return c->stream;
}

sp_http_reply_t sp_http_reply_status(s32 status) {
  return (sp_http_reply_t) { .status = status };
}

sp_http_reply_t sp_http_reply_text(s32 status, sp_str_t body) {
  return (sp_http_reply_t) { .status = status, .content_type = sp_str_lit("text/plain"), .body = body };
}

sp_http_reply_t sp_http_reply_json(s32 status, sp_str_t body) {
  return (sp_http_reply_t) { .status = status, .content_type = sp_str_lit("application/json"), .body = body };
}

static bool path_segment_ok(sp_str_t segment) {
  if (sp_str_equal_cstr(segment, "..")) return false;
  if (sp_str_find_c8(segment, '\\') != SP_STR_NO_MATCH) return false;
  if (sp_str_find_c8(segment, '\0') != SP_STR_NO_MATCH) return false;
  return true;
}

static bool path_is_contained(sp_str_t rel) {
  if (sp_str_empty(rel) || rel.data[0] == '/') return false;
  sp_str_t rest = rel;
  while (!sp_str_empty(rest)) {
    s32 slash = sp_str_find_c8(rest, '/');
    sp_str_t segment = slash == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, slash);
    rest = slash == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, slash + 1);
    if (!path_segment_ok(segment)) return false;
  }
  return true;
}

sp_http_reply_t sp_http_reply_file(sp_http_ctx_t* c, sp_str_t root, sp_str_t rel) {
  if (!path_is_contained(rel)) {
    return sp_http_reply_status(404);
  }
  sp_str_t content = sp_zero;
  if (sp_io_read_file(c->mem, sp_fs_join_path(c->mem, root, rel), &content) != SP_OK) {
    return sp_http_reply_status(404);
  }
  return (sp_http_reply_t) { .status = 200, .content_type = sp_http_mime_type(rel), .body = content };
}

sp_http_reply_t sp_http_reply_stream(sp_http_stream_t* stream, sp_str_t content_type) {
  return (sp_http_reply_t) { .kind = SP_HTTP_REPLY_STREAM, .status = 200, .content_type = content_type, .stream = stream };
}

void sp_http_reply_header(sp_http_reply_t* reply, sp_str_t name, sp_str_t value) {
  sp_assert(reply->num_headers < SP_HTTP_REPLY_MAX_HEADERS);
  reply->headers[reply->num_headers++] = (sp_http_header_t) { .name = name, .value = value };
}

bool sp_http_stream_closed(const sp_http_stream_t* stream) {
  return stream->dead;
}

void sp_http_stream_close(sp_http_stream_t* stream) {
  stream->dead = true;
  stream->held = false;
}

#if defined(SP_HTTP_SOCKETS)

void sp_http_conn_serve(sp_http_conn_t* conn, sp_sys_socket_t socket, const sp_http_router_t* router) {
  for (;;) {
    u32 want = sp_http_conn_step(conn);
    if (want & SP_HTTP_WANT_REQUEST) {
      sp_http_conn_reply(conn, sp_http_route(router, &conn->ctx));
      continue;
    }
    if (want & SP_HTTP_WANT_SEND) {
      sp_mem_slice_t out = sp_http_conn_send_slot(conn);
      u64 n = 0;
      sp_err_t err = sp_sys_socket_send(socket, out.data, out.len, &n);
      sp_http_conn_sent(conn, err == SP_OK ? n : 0);
      continue;
    }
    if (conn_streaming(conn)) {
      sp_http_stream_close(&conn->stream);
      continue;
    }
    if (want & SP_HTTP_WANT_RECV) {
      sp_mem_slice_t in = sp_http_conn_recv_slot(conn);
      u64 n = 0;
      sp_err_t err = sp_sys_socket_recv(socket, in.data, in.len, &n);
      sp_http_conn_received(conn, err == SP_OK ? n : 0);
      continue;
    }
    sp_sys_socket_close(socket);
    return;
  }
}

////////////
// SERVER //
////////////

#define SP_HTTP_SERVER_DEFAULT_CONNS   16
#define SP_HTTP_SERVER_DEFAULT_IDLE_MS 5000
#define SP_HTTP_SERVER_DONE_MAX        64

static void server_arm_accept(sp_http_server_t* server) {
  server->accept = (sp_io_op_t) {
    .kind = SP_IO_OP_ACCEPT,
    .accept = { .socket = server->listener },
    .user_data = server,
  };
  sp_assert(sp_io_submit(server->desc.io, &server->accept) == SP_OK);
  server->accept_armed = true;
}

static sp_http_slot_t* slot_take(sp_http_server_t* server) {
  sp_for(it, server->desc.max_conns) {
    if (!server->slots[it].live) return &server->slots[it];
  }
  return SP_NULLPTR;
}

static void slot_arm_recv(sp_http_server_t* server, sp_http_slot_t* slot) {
  slot->recv = (sp_io_op_t) {
    .kind = SP_IO_OP_RECV,
    .recv = { .socket = slot->socket, .buf = sp_http_conn_recv_slot(&slot->conn) },
    .user_data = slot,
  };
  sp_assert(sp_io_submit(server->desc.io, &slot->recv) == SP_OK);
  slot->recv_armed = true;
}

static void slot_arm_send(sp_http_server_t* server, sp_http_slot_t* slot) {
  slot->send = (sp_io_op_t) {
    .kind = SP_IO_OP_SEND,
    .send = { .socket = slot->socket, .buf = sp_http_conn_send_slot(&slot->conn) },
    .user_data = slot,
  };
  sp_assert(sp_io_submit(server->desc.io, &slot->send) == SP_OK);
  slot->send_armed = true;
}

static void slot_settle(sp_http_server_t* server, sp_http_slot_t* slot) {
  if (slot->recv_armed || slot->send_armed) return;
  if (slot->socket != SP_SYS_INVALID_SOCKET) {
    sp_sys_socket_close(slot->socket);
    slot->socket = SP_SYS_INVALID_SOCKET;
  }
  if (slot->conn.stream.held) return;
  slot->live = false;
  if (!server->accept_armed && !server->stopping) {
    server_arm_accept(server);
  }
}

static void slot_close(sp_http_server_t* server, sp_http_slot_t* slot) {
  slot->closing = true;
  if (slot->recv_armed) sp_io_cancel(server->desc.io, &slot->recv);
  if (slot->send_armed) sp_io_cancel(server->desc.io, &slot->send);
  slot_settle(server, slot);
}

static void slot_drive(sp_http_server_t* server, sp_http_slot_t* slot) {
  if (slot->closing) {
    slot_settle(server, slot);
    return;
  }
  u32 want = sp_http_conn_step(&slot->conn);
  while (want & SP_HTTP_WANT_REQUEST) {
    sp_http_conn_reply(&slot->conn, sp_http_route(&server->desc.router, &slot->conn.ctx));
    want = sp_http_conn_step(&slot->conn);
  }
  if (want & SP_HTTP_WANT_CLOSE) {
    slot_close(server, slot);
    return;
  }
  if ((want & SP_HTTP_WANT_SEND) && !slot->send_armed) slot_arm_send(server, slot);
  if ((want & SP_HTTP_WANT_RECV) && !slot->recv_armed) slot_arm_recv(server, slot);
}

static void slot_complete(sp_http_server_t* server, sp_http_slot_t* slot, sp_io_op_t* op) {
  u64 n = op->result.err == SP_OK ? op->result.len : 0;
  if (op == &slot->recv) {
    slot->recv_armed = false;
    if (!slot->closing) sp_http_conn_received(&slot->conn, n);
  }
  else {
    slot->send_armed = false;
    if (!slot->closing) sp_http_conn_sent(&slot->conn, n);
  }
  slot->active = sp_io_now(server->desc.io, SP_IO_CLOCK_AWAKE);
  slot_drive(server, slot);
}

static void server_accepted(sp_http_server_t* server, sp_io_op_t* op) {
  server->accept_armed = false;
  if (op->result.err != SP_OK) {
    if (!server->stopping) server_arm_accept(server);
    return;
  }
  if (server->stopping) {
    sp_sys_socket_close(op->result.socket);
    return;
  }
  sp_http_slot_t* slot = slot_take(server);
  sp_sys_socket_no_delay(op->result.socket);
  slot->socket = op->result.socket;
  slot->live = true;
  slot->closing = false;
  slot->active = sp_io_now(server->desc.io, SP_IO_CLOCK_AWAKE);
  sp_http_conn_reset(&slot->conn);
  slot_drive(server, slot);
  if (slot_take(server)) {
    server_arm_accept(server);
  }
}

static void server_dispatch(sp_http_server_t* server, sp_io_op_t* op) {
  if (op == &server->accept) {
    server_accepted(server, op);
    return;
  }
  sp_http_slot_t* slot = sp_cast(sp_http_slot_t*, op->user_data);
  sp_assert(slot >= server->slots && slot < server->slots + server->desc.max_conns);
  slot_complete(server, slot, op);
}

static bool server_armed(const sp_http_server_t* server) {
  if (server->accept_armed) return true;
  sp_for(it, server->desc.max_conns) {
    if (server->slots[it].recv_armed || server->slots[it].send_armed) return true;
  }
  return false;
}

sp_http_error_t sp_http_server_init(sp_http_server_t* server, sp_http_server_desc_t desc) {
  *server = sp_zero_s(sp_http_server_t);
  server->desc = desc;
  server->desc.max_conns = desc.max_conns ? desc.max_conns : SP_HTTP_SERVER_DEFAULT_CONNS;
  server->desc.idle_ms = desc.idle_ms ? desc.idle_ms : SP_HTTP_SERVER_DEFAULT_IDLE_MS;
  server->idle_ns = (u64)server->desc.idle_ms * 1000 * 1000;
  server->listener = SP_SYS_INVALID_SOCKET;

  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  if (sp_sys_socket_open(&listener, sp_zero_s(sp_sys_handle_desc_t)) != SP_OK) {
    return SP_HTTP_ERR_OS;
  }
  if (sp_sys_socket_reuse_addr(listener) != SP_OK ||
      sp_sys_socket_bind(listener, desc.addr) != SP_OK ||
      sp_sys_socket_listen(listener, (s32)server->desc.max_conns) != SP_OK ||
      sp_sys_socket_local_port(listener, &server->port) != SP_OK) {
    sp_sys_socket_close(listener);
    return SP_HTTP_ERR_OS;
  }
  server->listener = listener;

  server->slots = sp_alloc_n(server->desc.conn.mem, sp_http_slot_t, server->desc.max_conns);
  sp_for(it, server->desc.max_conns) {
    server->slots[it] = sp_zero_s(sp_http_slot_t);
    server->slots[it].socket = SP_SYS_INVALID_SOCKET;
    sp_http_conn_init(&server->slots[it].conn, server->desc.conn);
  }

  server_arm_accept(server);
  return SP_HTTP_OK;
}

void sp_http_server_pump(sp_http_server_t* server, sp_io_timeout_t timeout) {
  sp_io_time_t now = sp_io_now(server->desc.io, SP_IO_CLOCK_AWAKE);
  sp_for(it, server->desc.max_conns) {
    sp_http_slot_t* slot = &server->slots[it];
    if (!slot->live) continue;
    bool idle = now.ns - slot->active.ns > server->idle_ns && !conn_streaming(&slot->conn) && !slot->closing;
    if (idle) {
      slot_close(server, slot);
      continue;
    }
    slot_drive(server, slot);
  }

  sp_io_op_t* done [SP_HTTP_SERVER_DONE_MAX];
  sp_for(round, 2) {
    u32 count = 0;
    sp_io_wait(server->desc.io, done, sp_carr_len(done), round == 0 ? timeout : sp_io_timeout_after(0), &count);
    if (count == 0) break;
    sp_for(it, count) {
      server_dispatch(server, done[it]);
    }
  }
}

void sp_http_server_run(sp_http_server_t* server) {
  while (!sp_atomic_u32_load(&server->quit, SP_ATOMIC_ACQUIRE)) {
    sp_http_server_pump(server, sp_io_timeout_after(server->idle_ns));
  }
}

void sp_http_server_stop(sp_http_server_t* server) {
  sp_atomic_u32_store(&server->quit, 1, SP_ATOMIC_RELEASE);
  sp_io_wake(server->desc.io);
}

void sp_http_server_deinit(sp_http_server_t* server) {
  server->stopping = true;
  if (server->accept_armed) sp_io_cancel(server->desc.io, &server->accept);
  sp_for(it, server->desc.max_conns) {
    if (server->slots[it].live) slot_close(server, &server->slots[it]);
  }

  sp_io_op_t* done [SP_HTTP_SERVER_DONE_MAX];
  while (server_armed(server)) {
    u32 count = 0;
    sp_io_wait(server->desc.io, done, sp_carr_len(done), sp_io_timeout_none(), &count);
    sp_for(it, count) {
      server_dispatch(server, done[it]);
    }
  }

  sp_for(it, server->desc.max_conns) {
    sp_http_conn_deinit(&server->slots[it].conn);
  }
  sp_free(server->desc.conn.mem, server->slots, sizeof(sp_http_slot_t) * server->desc.max_conns);
  sp_sys_socket_close(server->listener);
}

#else

void sp_http_conn_serve(sp_http_conn_t* conn, sp_sys_socket_t socket, const sp_http_router_t* router) {
  sp_unused(conn); sp_unused(socket); sp_unused(router);
}

sp_http_error_t sp_http_server_init(sp_http_server_t* server, sp_http_server_desc_t desc) {
  *server = sp_zero_s(sp_http_server_t);
  server->desc = desc;
  server->listener = SP_SYS_INVALID_SOCKET;
  return SP_HTTP_ERR_UNSUPPORTED;
}

void sp_http_server_pump(sp_http_server_t* server, sp_io_timeout_t timeout) {
  sp_unused(server); sp_unused(timeout);
}

void sp_http_server_run(sp_http_server_t* server) {
  sp_unused(server);
}

void sp_http_server_stop(sp_http_server_t* server) {
  sp_unused(server);
}

void sp_http_server_deinit(sp_http_server_t* server) {
  sp_unused(server);
}

#endif

#if defined(SP_TLS_WITH_MBEDTLS)

#if defined(SP_MACOS) && !defined(SP_TLS_MACOS_SECTRUST) && defined(__has_include)
  #if __has_include(<Security/Security.h>)
    #error "sp_tls on macOS requires SecTrust: define SP_TLS_MACOS_SECTRUST and link -framework Security -framework CoreFoundation"
  #endif
#endif

#if defined(SP_WIN32)
#include <wincrypt.h>
#endif

#if defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

SP_PRIVATE sp_tls_backend_t sp_tls_native_backend(void) {
#if defined(SP_WIN32)
  return SP_TLS_BACKEND_OS_VERIFY;
#elif defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
  return SP_TLS_BACKEND_OS_VERIFY;
#elif defined(SP_MACOS)
  // If you didn't compile in SecTrust, fail here instead of at runtime
  return SP_TLS_BACKEND_NONE;
#elif defined(SP_LINUX)
  return SP_TLS_BACKEND_ANCHORS;
#else
  return SP_TLS_BACKEND_NONE;
#endif
}

SP_PRIVATE u32 sp_tls_chain_count(const mbedtls_x509_crt* chain) {
  u32 count = 0;
  const mbedtls_x509_crt* it = chain;
  while (it) {
    count++;
    it = it->next;
  }
  return count;
}

sp_http_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  trust->anchors = sp_alloc_type(mem, mbedtls_x509_crt);
  mbedtls_x509_crt_init(trust->anchors);
  return SP_HTTP_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  if (trust->anchors) {
    mbedtls_x509_crt_free(trust->anchors);
    sp_free(trust->mem, trust->anchors, sizeof(mbedtls_x509_crt));
  }
  *trust = sp_zero_s(sp_tls_trust_t);
}

sp_http_error_t sp_tls_trust_load(sp_tls_trust_t* trust) {
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
      return sp_tls_load_unix(trust->anchors, &trust->loaded, &trust->skipped);
    case SP_TLS_BACKEND_OS_VERIFY:
      return SP_HTTP_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_HTTP_ERR_UNSUPPORTED;
  }
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_load_pem(mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_str_t content = sp_zero;
  sp_http_error_t result = SP_HTTP_ERR_NO_STORE;
  if (sp_io_read_file(scratch.mem, path, &content) == SP_OK) {
    c8* pem = sp_str_to_cstr(scratch.mem, content);
    s32 rc = mbedtls_x509_crt_parse(chain, (const unsigned char*)pem, (size_t)content.len + 1);
    if (rc < 0) {
      result = SP_HTTP_ERR_PARSE;
    }
    else {
      if (loaded)  *loaded = sp_tls_chain_count(chain);
      if (skipped) *skipped = (u32)rc;
      result = SP_HTTP_OK;
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

SP_PRIVATE sp_http_error_t sp_tls_load_unix(mbedtls_x509_crt* chain, u32* loaded, u32* skipped) {
  sp_str_t env = sp_os_env_get(sp_str_lit("SSL_CERT_FILE"));
  if (!sp_str_empty(env)) {
    return sp_tls_load_pem(chain, env, loaded, skipped);
  }
  const c8* candidates[] = {
    "/etc/ssl/certs/ca-certificates.crt",
    "/etc/pki/tls/certs/ca-bundle.crt",
    "/etc/ssl/cert.pem",
    "/etc/ssl/ca-bundle.pem",
    "/etc/pki/tls/cacert.pem",
  };
  sp_carr_for(candidates, it) {
    sp_str_t path = sp_cstr_as_str(candidates[it]);
    if (sp_fs_exists(path)) {
      return sp_tls_load_pem(chain, path, loaded, skipped);
    }
  }
  return SP_HTTP_ERR_NO_STORE;
}

SP_PRIVATE sp_http_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, mbedtls_ssl_config* conf) {
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
    case SP_TLS_BACKEND_OS_VERIFY:
      mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_REQUIRED);
      mbedtls_ssl_conf_ca_chain(conf, trust->anchors, SP_NULLPTR);
      return SP_HTTP_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_HTTP_ERR_UNSUPPORTED;
  }
  return SP_HTTP_ERR_UNSUPPORTED;
}

SP_PRIVATE sp_http_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_URL;
  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  if (mbedtls_ssl_set_hostname(ssl, host) != 0) return SP_HTTP_ERR_OS;
  if (trust->backend == SP_TLS_BACKEND_OS_VERIFY) {
#if !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
    return SP_HTTP_ERR_BAD_CONFIG;
#else
    if (verify) verify->hostname = hostname;
    mbedtls_ssl_set_verify(ssl, sp_tls_verify_cb, verify);
#endif
  }
  return SP_HTTP_OK;
}

SP_PRIVATE s32 sp_tls_verify_cb(void* user_data, mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  *flags = 0;
  if (depth != 0) return 0;
  sp_tls_verify_t* verify = (sp_tls_verify_t*)user_data;
  sp_str_t hostname = verify ? verify->hostname : sp_zero_s(sp_str_t);
#if defined(SP_WIN32)
  sp_http_error_t err = sp_tls_windows_eval(crt, hostname);
#else
  sp_http_error_t err = sp_tls_macos_eval(crt, hostname);
#endif
  if (err != SP_HTTP_OK) {
    *flags = MBEDTLS_X509_BADCERT_NOT_TRUSTED;
  }
  return 0;
}

SP_PRIVATE sp_http_error_t sp_tls_macos_eval(const mbedtls_x509_crt* chain, sp_str_t hostname) {
#if defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_UNTRUSTED;

  CFMutableArrayRef certs = CFArrayCreateMutable(SP_NULLPTR, 0, &kCFTypeArrayCallBacks);
  if (!certs) return SP_HTTP_ERR_OS;

  bool converted = true;
  const mbedtls_x509_crt* it = chain;
  while (it) {
    CFDataRef der = CFDataCreate(SP_NULLPTR, it->raw.p, (CFIndex)it->raw.len);
    SecCertificateRef cert = der ? SecCertificateCreateWithData(SP_NULLPTR, der) : SP_NULLPTR;
    if (der) CFRelease(der);
    if (!cert) {
      converted = false;
      break;
    }
    CFArrayAppendValue(certs, cert);
    CFRelease(cert);
    it = it->next;
  }

  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  CFStringRef cfhost = CFStringCreateWithCString(SP_NULLPTR, host, kCFStringEncodingUTF8);
  SecPolicyRef policy = cfhost ? SecPolicyCreateSSL(true, cfhost) : SP_NULLPTR;

  SecTrustRef trust = SP_NULLPTR;
  sp_http_error_t result = SP_HTTP_ERR_UNTRUSTED;
  if (converted && CFArrayGetCount(certs) > 0 && policy) {
    if (SecTrustCreateWithCertificates(certs, policy, &trust) == errSecSuccess) {
      CFErrorRef error = SP_NULLPTR;
      if (SecTrustEvaluateWithError(trust, &error)) result = SP_HTTP_OK;
      if (error) CFRelease(error);
    }
  }

  if (trust) CFRelease(trust);
  if (policy) CFRelease(policy);
  if (cfhost) CFRelease(cfhost);
  CFRelease(certs);
  return result;
#else
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
#endif
}

SP_PRIVATE sp_http_error_t sp_tls_windows_eval(const mbedtls_x509_crt* chain, sp_str_t hostname) {
#if defined(SP_WIN32)
  const mbedtls_x509_crt* leaf = chain;
  if (!leaf) return SP_HTTP_ERR_UNTRUSTED;
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_UNTRUSTED;

  WCHAR wide[SP_PATH_MAX];
  s32 wide_len = MultiByteToWideChar(CP_UTF8, 0, hostname.data, (int)hostname.len, wide, SP_PATH_MAX - 1);
  if (wide_len <= 0) return SP_HTTP_ERR_UNTRUSTED;
  wide[wide_len] = 0;

  HCERTSTORE extra = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, SP_NULLPTR);
  if (!extra) return SP_HTTP_ERR_OS;

  sp_http_error_t result = SP_HTTP_ERR_OS;
  PCCERT_CHAIN_CONTEXT verdict = SP_NULLPTR;
  PCCERT_CONTEXT ctx = CertCreateCertificateContext(X509_ASN_ENCODING, leaf->raw.p, (DWORD)leaf->raw.len);

  bool converted = ctx != SP_NULLPTR;
  const mbedtls_x509_crt* it = leaf->next;
  while (converted && it) {
    converted = CertAddEncodedCertificateToStore(extra, X509_ASN_ENCODING, it->raw.p, (DWORD)it->raw.len, CERT_STORE_ADD_REPLACE_EXISTING, SP_NULLPTR) != 0;
    it = it->next;
  }

  LPSTR usages[] = { (LPSTR)szOID_PKIX_KP_SERVER_AUTH };
  CERT_CHAIN_PARA para = sp_zero;
  para.cbSize = sizeof(para);
  para.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
  para.RequestedUsage.Usage.cUsageIdentifier = 1;
  para.RequestedUsage.Usage.rgpszUsageIdentifier = usages;

  if (converted && CertGetCertificateChain(SP_NULLPTR, ctx, SP_NULLPTR, extra, &para, CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT, SP_NULLPTR, &verdict)) {
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA ssl = sp_zero;
    ssl.cbSize = sizeof(ssl);
    ssl.dwAuthType = AUTHTYPE_SERVER;
    ssl.pwszServerName = wide;
    CERT_CHAIN_POLICY_PARA policy = sp_zero;
    policy.cbSize = sizeof(policy);
    policy.dwFlags = CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;
    policy.pvExtraPolicyPara = &ssl;
    CERT_CHAIN_POLICY_STATUS status = sp_zero;
    status.cbSize = sizeof(status);
    if (CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, verdict, &policy, &status)) {
      result = status.dwError == 0 ? SP_HTTP_OK : SP_HTTP_ERR_UNTRUSTED;
    }
  }

  if (verdict) CertFreeCertificateChain(verdict);
  if (ctx) CertFreeCertificateContext(ctx);
  CertCloseStore(extra, 0);
  return result;
#else
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
#endif
}

SP_PRIVATE sp_err_t sp_tls_mbedtls_wait(sp_tls_mbedtls_t* tls, s32 rc) {
  bool readable = rc == MBEDTLS_ERR_SSL_WANT_READ;
  sp_err_t wait = sp_sys_socket_wait(tls->socket, readable, tls->io_timeout_ms);
  if (wait == SP_ERR_SYS_TIMED_OUT) return SP_ERR_IO_TIMEOUT;
  return wait;
}

SP_PRIVATE sp_err_t sp_tls_mbedtls_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  sp_tls_mbedtls_reader_t* wrapper = (sp_tls_mbedtls_reader_t*)reader;
  sp_tls_mbedtls_t* tls = wrapper->tls;
  if (bytes_read) *bytes_read = 0;
  if (wrapper->eof) return SP_ERR_IO_EOF;
  for (;;) {
    s32 n = mbedtls_ssl_read(&tls->ssl, (unsigned char*)ptr, (size_t)sp_min(size, (u64)INT32_MAX));
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
      sp_err_t err = sp_tls_mbedtls_wait(tls, n);
      if (err != SP_OK) return err;
      continue;
    }
    if (n == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
      wrapper->eof = true;
      return SP_ERR_IO_EOF;
    }
    if (n == MBEDTLS_ERR_SSL_TIMEOUT) return SP_ERR_IO_TIMEOUT;
    if (n == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) continue;
    // a raw EOF without close_notify could be a truncation attack, so it is
    // an error, not end-of-stream
    if (n <= 0) return SP_ERR_IO;
    if (bytes_read) *bytes_read = (u64)n;
    return SP_OK;
  }
}

SP_PRIVATE sp_err_t sp_tls_mbedtls_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  sp_tls_mbedtls_t* tls = ((sp_tls_mbedtls_writer_t*)writer)->tls;
  if (bytes_written) *bytes_written = 0;
  for (;;) {
    s32 n = mbedtls_ssl_write(&tls->ssl, (const unsigned char*)ptr, (size_t)sp_min(size, (u64)INT32_MAX));
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
      sp_err_t err = sp_tls_mbedtls_wait(tls, n);
      if (err != SP_OK) return err;
      continue;
    }
    if (n <= 0) return SP_ERR_IO;
    if (bytes_written) *bytes_written = (u64)n;
    return SP_OK;
  }
}

SP_PRIVATE sp_http_error_t sp_tls_mbedtls_handshake(sp_tls_mbedtls_t* tls, sp_str_t hostname) {
  if (mbedtls_ctr_drbg_seed(&tls->drbg, mbedtls_entropy_func, &tls->entropy, SP_NULLPTR, 0) != 0) return SP_HTTP_ERR_OS;
  if (mbedtls_ssl_config_defaults(&tls->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) return SP_HTTP_ERR_OS;
  mbedtls_ssl_conf_min_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_2);
  mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->drbg);
  mbedtls_ssl_conf_read_timeout(&tls->conf, tls->io_timeout_ms);
  if (sp_tls_conf_apply(tls->trust, &tls->conf) != SP_HTTP_OK) return SP_HTTP_ERR_BAD_CONFIG;
  if (mbedtls_ssl_setup(&tls->ssl, &tls->conf) != 0) return SP_HTTP_ERR_OS;
  if (sp_tls_ssl_attach(tls->trust, &tls->ssl, &tls->verify, hostname) != SP_HTTP_OK) return SP_HTTP_ERR_BAD_CONFIG;
  mbedtls_ssl_set_bio(&tls->ssl, &tls->net, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  s32 rc;
  while ((rc = mbedtls_ssl_handshake(&tls->ssl)) != 0) {
    if (rc == MBEDTLS_ERR_SSL_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
    if (rc == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) return SP_HTTP_ERR_UNTRUSTED;
    if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) return SP_HTTP_ERR_HANDSHAKE;
    sp_err_t err = sp_tls_mbedtls_wait(tls, rc);
    if (err == SP_ERR_IO_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
    if (err != SP_OK) return SP_HTTP_ERR_OS;
  }
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_tls_mbedtls_drop(sp_tls_mbedtls_t* tls) {
  tls->net.fd = -1;
  mbedtls_ssl_free(&tls->ssl);
  mbedtls_ssl_config_free(&tls->conf);
  mbedtls_ctr_drbg_free(&tls->drbg);
  mbedtls_entropy_free(&tls->entropy);
  mbedtls_net_free(&tls->net);
}

SP_PRIVATE sp_http_error_t sp_tls_mbedtls_open(sp_tls_t* base, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms) {
  sp_tls_mbedtls_t* tls = (sp_tls_mbedtls_t*)base;
  mbedtls_net_init(&tls->net);
  mbedtls_ssl_init(&tls->ssl);
  mbedtls_ssl_config_init(&tls->conf);
  mbedtls_entropy_init(&tls->entropy);
  mbedtls_ctr_drbg_init(&tls->drbg);
  tls->verify = sp_zero_s(sp_tls_verify_t);
  tls->socket = socket;
  tls->io_timeout_ms = io_timeout_ms;
  tls->net.fd = (int)socket;
  tls->reader.eof = false;

  sp_http_error_t err = sp_tls_mbedtls_handshake(tls, hostname);
  if (err != SP_HTTP_OK) {
    sp_tls_mbedtls_drop(tls);
    return err;
  }
  tls->base.reader = &tls->reader.base;
  tls->base.writer = &tls->writer.base;
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_tls_mbedtls_close(sp_tls_t* base) {
  sp_tls_mbedtls_t* tls = (sp_tls_mbedtls_t*)base;
  mbedtls_ssl_close_notify(&tls->ssl);
  sp_tls_mbedtls_drop(tls);
}

void sp_tls_mbedtls_init(sp_tls_mbedtls_t* tls, const sp_tls_trust_t* trust) {
  *tls = sp_zero_s(sp_tls_mbedtls_t);
  tls->base.open = sp_tls_mbedtls_open;
  tls->base.close = sp_tls_mbedtls_close;
  tls->trust = trust;
  tls->reader.base.read = sp_tls_mbedtls_read;
  tls->reader.tls = tls;
  tls->writer.base.write = sp_tls_mbedtls_write;
  tls->writer.tls = tls;
}

#endif // SP_TLS_WITH_MBEDTLS

#endif // SP_HTTP_IMPLEMENTATION
#endif // SP_HTTP_C
