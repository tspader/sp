#include "../http.h"

typedef struct {
  const c8* type;
} expect_t;

typedef struct {
  const c8* name;
  const c8* path;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  { .name = "html", .path = "index.html", .expect = { .type = "text/html" } },
  { .name = "js",   .path = "web/main.js", .expect = { .type = "text/javascript" } },
  { .name = "css",  .path = "a.css", .expect = { .type = "text/css" } },
  { .name = "json", .path = "a.json", .expect = { .type = "application/json" } },
  { .name = "png",  .path = "a.png", .expect = { .type = "image/png" } },
  { .name = "svg",  .path = "a.svg", .expect = { .type = "image/svg+xml" } },
  { .name = "wasm", .path = "a.wasm", .expect = { .type = "application/wasm" } },
  { .name = "txt",  .path = "a.txt", .expect = { .type = "text/plain" } },
  { .name = "uppercase_extension", .path = "A.HTML", .expect = { .type = "text/html" } },
  { .name = "unknown_extension", .path = "a.xyz", .expect = { .type = "application/octet-stream" } },
  { .name = "no_extension", .path = "Makefile", .expect = { .type = "application/octet-stream" } },
  { .name = "dot_in_directory_only", .path = "a.b/c", .expect = { .type = "application/octet-stream" } },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_expect_str_eq_c(t, sp_http_mime_type(sp_cstr_as_str(c->path)), c->expect.type);
  return SP_OK;
}

sp_test_each_fn(http, mime, test_t, tests, run);
