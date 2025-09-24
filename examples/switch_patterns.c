#define SP_IMPLEMENTATION
#include "../sp.h"

typedef enum {
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_SPACE,
} token_type_t;

typedef struct {
  token_type_t type;
} token_t;

typedef enum {
  BUILD_PENDING,
  BUILD_RUNNING,
  BUILD_SUCCESS,
  BUILD_FAILED,
} build_status_t;

static void parse_number(token_t* token) { SP_UNUSED(token); }
static void parse_string(token_t* token) { SP_UNUSED(token); }
static void parse_operator(token_t* token) { SP_UNUSED(token); }

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

int main(void) {
  sp_example_init();

  token_t token = { .type = TOKEN_NUMBER };
  switch (token.type) {
    case TOKEN_NUMBER: {
      parse_number(&token);
      break;
    }

    case TOKEN_STRING: {
      parse_string(&token);
      break;
    }

    case TOKEN_PLUS:
    case TOKEN_MINUS: {
      parse_operator(&token);
      break;
    }

    case TOKEN_SPACE: {
      break;
    }

    default: {
      SP_UNREACHABLE_CASE();
    }
  }

  build_status_t build_status = BUILD_SUCCESS;
  sp_str_t target = SP_LIT("target");
  sp_str_t msg = SP_ZERO_STRUCT(sp_str_t);

  switch (build_status) {
    case BUILD_PENDING: {
      msg = sp_format("{:fg brightblack} {}", SP_FMT_CSTR("[ ]"), SP_FMT_STR(target));
      break;
    }
    case BUILD_RUNNING: {
      msg = sp_format("{:fg yellow} {}", SP_FMT_CSTR("[.]"), SP_FMT_STR(target));
      break;
    }
    case BUILD_SUCCESS: {
      msg = sp_format("{:fg green} {}", SP_FMT_CSTR("[+]"), SP_FMT_STR(target));
      break;
    }
    case BUILD_FAILED: {
      msg = sp_format("{:fg red} {}", SP_FMT_CSTR("[-]"), SP_FMT_STR(target));
      break;
    }
  }

  if (msg.data) {
    sp_free((void*)msg.data);
  }

  sp_example_shutdown();
  return 0;
}
