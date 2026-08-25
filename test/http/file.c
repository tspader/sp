#include "http.h"

#define FILE_TEST_MAX_FILES 4

typedef struct {
  const c8* path;
  const c8* content;
  bool      dir;
} file_setup_t;

typedef struct {
  s32       status;
  const c8* body;
  const c8* content_type;
} expect_t;

typedef struct {
  const c8*    name;
  file_setup_t files [FILE_TEST_MAX_FILES];
  const c8*    rel;
  expect_t     expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "serves_file",
    .files = {
      { .path = "root", .dir = true },
      { .path = "root/a.txt", .content = "hello" },
    },
    .rel = "a.txt",
    .expect = { .status = 200, .body = "hello", .content_type = "text/plain" },
  },
  {
    .name = "mime_from_extension",
    .files = {
      { .path = "root", .dir = true },
      { .path = "root/app.js", .content = "x" },
    },
    .rel = "app.js",
    .expect = { .status = 200, .body = "x", .content_type = "text/javascript" },
  },
  {
    .name = "nested_path",
    .files = {
      { .path = "root", .dir = true },
      { .path = "root/web", .dir = true },
      { .path = "root/web/x.css", .content = "c" },
    },
    .rel = "web/x.css",
    .expect = { .status = 200, .body = "c", .content_type = "text/css" },
  },
  {
    .name = "missing_file",
    .files = {
      { .path = "root", .dir = true },
    },
    .rel = "nope.txt",
    .expect = { .status = 404 },
  },
  {
    .name = "traversal_rejected",
    .files = {
      { .path = "root", .dir = true },
      { .path = "secret", .content = "s" },
    },
    .rel = "../secret",
    .expect = { .status = 404 },
  },
  {
    .name = "absolute_rejected",
    .files = {
      { .path = "root", .dir = true },
    },
    .rel = "/etc/hostname",
    .expect = { .status = 404 },
  },
  {
    .name = "backslash_traversal_rejected",
    .files = {
      { .path = "root", .dir = true },
      { .path = "secret", .content = "s" },
    },
    .rel = "..\\secret",
    .expect = { .status = 404 },
  },
  {
    .name = "empty_rel_rejected",
    .files = {
      { .path = "root", .dir = true },
    },
    .rel = "",
    .expect = { .status = 404 },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_str_t sandbox = sp_test_dir(t);
  sp_carr_for(c->files, it) {
    if (!c->files[it].path) break;
    sp_str_t path = sp_fs_join_path(sp_test_arena(t), sandbox, sp_cstr_as_str(c->files[it].path));
    if (c->files[it].dir) {
      sp_must_ok(t, sp_fs_create_dir(path));
    }
    else {
      sp_must_ok(t, sp_fs_create_file_str(path, sp_cstr_as_str(c->files[it].content)));
    }
  }

  sp_http_ctx_t ctx = { .mem = sp_test_arena(t) };
  sp_str_t root = sp_fs_join_path(sp_test_arena(t), sandbox, sp_str_lit("root"));
  sp_http_reply_t reply = sp_http_reply_file(&ctx, root, sp_cstr_as_str(c->rel));

  sp_expect_eq(t, reply.status, c->expect.status);
  if (c->expect.body) sp_expect_str_eq_c(t, reply.body, c->expect.body);
  if (c->expect.content_type) sp_expect_str_eq_c(t, reply.content_type, c->expect.content_type);
  return SP_OK;
}

sp_test_each_fn(http, file, test_t, tests, run);
