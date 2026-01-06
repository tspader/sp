#include "sp.h"

typedef struct {
  sp_str_t name;
  s32 stack_offset;
} jit_var_t;

typedef struct {
  sp_da(u8) code;
  sp_da(jit_var_t) vars;
  s32 stack_size;
} jit_ctx_t;

static void emit(jit_ctx_t* ctx, u8 byte) {
  sp_da_push(ctx->code, byte);
}

static void emit_u32(jit_ctx_t* ctx, u32 val) {
  emit(ctx, val & 0xFF);
  emit(ctx, (val >> 8) & 0xFF);
  emit(ctx, (val >> 16) & 0xFF);
  emit(ctx, (val >> 24) & 0xFF);
}

static void emit_prologue(jit_ctx_t* ctx) {
  emit(ctx, 0x55);
  emit(ctx, 0x48); emit(ctx, 0x89); emit(ctx, 0xe5);
}

static void emit_epilogue(jit_ctx_t* ctx) {
  emit(ctx, 0x48); emit(ctx, 0x89); emit(ctx, 0xec);
  emit(ctx, 0x5d);
  emit(ctx, 0xc3);
}

static void emit_sub_rsp(jit_ctx_t* ctx, s32 n) {
  emit(ctx, 0x48); emit(ctx, 0x81); emit(ctx, 0xec);
  emit_u32(ctx, n);
}

static void emit_mov_imm_to_rax(jit_ctx_t* ctx, s32 val) {
  emit(ctx, 0x48); emit(ctx, 0xc7); emit(ctx, 0xc0);
  emit_u32(ctx, val);
}

static void emit_mov_rax_to_stack(jit_ctx_t* ctx, s32 offset) {
  emit(ctx, 0x48); emit(ctx, 0x89); emit(ctx, 0x85);
  emit_u32(ctx, (u32)(s32)offset);
}

static void emit_mov_stack_to_rax(jit_ctx_t* ctx, s32 offset) {
  emit(ctx, 0x48); emit(ctx, 0x8b); emit(ctx, 0x85);
  emit_u32(ctx, (u32)(s32)offset);
}

static void emit_add_rcx_to_rax(jit_ctx_t* ctx) {
  emit(ctx, 0x48); emit(ctx, 0x01); emit(ctx, 0xc8);
}

static void emit_sub_rcx_from_rax(jit_ctx_t* ctx) {
  emit(ctx, 0x48); emit(ctx, 0x29); emit(ctx, 0xc8);
}

static void emit_imul_rcx(jit_ctx_t* ctx) {
  emit(ctx, 0x48); emit(ctx, 0x0f); emit(ctx, 0xaf); emit(ctx, 0xc1);
}

static bool is_whitespace(c8 c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_alpha(c8 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(c8 c) {
  return c >= '0' && c <= '9';
}

static bool is_alnum(c8 c) {
  return is_alpha(c) || is_digit(c);
}

static sp_str_t skip_ws(sp_str_t s) {
  while (s.len > 0 && is_whitespace(s.data[0])) {
    s.data++;
    s.len--;
  }
  return s;
}

static sp_str_t parse_ident(sp_str_t s, sp_str_t* out) {
  s = skip_ws(s);
  if (s.len == 0 || !is_alpha(s.data[0])) {
    *out = (sp_str_t){0};
    return s;
  }
  const c8* start = s.data;
  while (s.len > 0 && is_alnum(s.data[0])) {
    s.data++;
    s.len--;
  }
  out->data = start;
  out->len = s.data - start;
  return s;
}

static sp_str_t parse_number(sp_str_t s, s32* out) {
  s = skip_ws(s);
  s32 sign = 1;
  if (s.len > 0 && s.data[0] == '-') {
    sign = -1;
    s.data++;
    s.len--;
  }
  s32 val = 0;
  while (s.len > 0 && is_digit(s.data[0])) {
    val = val * 10 + (s.data[0] - '0');
    s.data++;
    s.len--;
  }
  *out = val * sign;
  return s;
}

static bool expect_char(sp_str_t* s, c8 c) {
  *s = skip_ws(*s);
  if (s->len == 0 || s->data[0] != c) return false;
  s->data++;
  s->len--;
  return true;
}

static jit_var_t* find_var(jit_ctx_t* ctx, sp_str_t name) {
  sp_da_for(ctx->vars, i) {
    if (sp_str_equal(ctx->vars[i].name, name)) {
      return &ctx->vars[i];
    }
  }
  return SP_NULLPTR;
}

static s32 alloc_var(jit_ctx_t* ctx, sp_str_t name) {
  ctx->stack_size += 8;
  jit_var_t var = {.name = name, .stack_offset = -ctx->stack_size};
  sp_da_push(ctx->vars, var);
  return var.stack_offset;
}

typedef enum {
  EXPR_NUM,
  EXPR_VAR,
  EXPR_ADD,
  EXPR_SUB,
  EXPR_MUL,
} expr_kind_t;

typedef struct expr expr_t;
struct expr {
  expr_kind_t kind;
  s32 num_val;
  sp_str_t var_name;
  expr_t* left;
  expr_t* right;
};

static expr_t* alloc_expr(void) {
  return sp_sys_alloc_zero(sizeof(expr_t));
}

static sp_str_t parse_expr(sp_str_t s, expr_t** out);

static sp_str_t parse_primary(sp_str_t s, expr_t** out) {
  s = skip_ws(s);
  if (s.len == 0) {
    *out = SP_NULLPTR;
    return s;
  }

  if (expect_char(&s, '(')) {
    s = parse_expr(s, out);
    expect_char(&s, ')');
    return s;
  }

  if (is_digit(s.data[0]) || (s.data[0] == '-' && s.len > 1 && is_digit(s.data[1]))) {
    expr_t* e = alloc_expr();
    e->kind = EXPR_NUM;
    s = parse_number(s, &e->num_val);
    *out = e;
    return s;
  }

  if (is_alpha(s.data[0])) {
    expr_t* e = alloc_expr();
    e->kind = EXPR_VAR;
    s = parse_ident(s, &e->var_name);
    *out = e;
    return s;
  }

  *out = SP_NULLPTR;
  return s;
}

static sp_str_t parse_mul(sp_str_t s, expr_t** out) {
  s = parse_primary(s, out);
  if (!*out) return s;

  while (1) {
    s = skip_ws(s);
    if (s.len == 0) break;
    c8 op = s.data[0];
    if (op != '*') break;
    s.data++; s.len--;

    expr_t* right;
    s = parse_primary(s, &right);
    if (!right) break;

    expr_t* node = alloc_expr();
    node->kind = EXPR_MUL;
    node->left = *out;
    node->right = right;
    *out = node;
  }
  return s;
}

static sp_str_t parse_add(sp_str_t s, expr_t** out) {
  s = parse_mul(s, out);
  if (!*out) return s;

  while (1) {
    s = skip_ws(s);
    if (s.len == 0) break;
    c8 op = s.data[0];
    if (op != '+' && op != '-') break;
    s.data++; s.len--;

    expr_t* right;
    s = parse_mul(s, &right);
    if (!right) break;

    expr_t* node = alloc_expr();
    node->kind = (op == '+') ? EXPR_ADD : EXPR_SUB;
    node->left = *out;
    node->right = right;
    *out = node;
  }
  return s;
}

static sp_str_t parse_expr(sp_str_t s, expr_t** out) {
  return parse_add(s, out);
}

static bool codegen_expr(jit_ctx_t* ctx, expr_t* e) {
  switch (e->kind) {
    case EXPR_NUM: {
      emit_mov_imm_to_rax(ctx, e->num_val);
      return true;
    }
    case EXPR_VAR: {
      jit_var_t* v = find_var(ctx, e->var_name);
      if (!v) {
        SP_LOG("error: undefined variable '{}'", SP_FMT_STR(e->var_name));
        return false;
      }
      emit_mov_stack_to_rax(ctx, v->stack_offset);
      return true;
    }
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL: {
      if (!codegen_expr(ctx, e->right)) return false;
      emit(ctx, 0x50);
      if (!codegen_expr(ctx, e->left)) return false;
      emit(ctx, 0x59);

      switch (e->kind) {
        case EXPR_ADD: { emit_add_rcx_to_rax(ctx); break; }
        case EXPR_SUB: { emit_sub_rcx_from_rax(ctx); break; }
        case EXPR_MUL: { emit_imul_rcx(ctx); break; }
        case EXPR_NUM: { break; }
        case EXPR_VAR: { break; }
      }
      return true;
    }
  }
  return false;
}

static bool parse_stmt(jit_ctx_t* ctx, sp_str_t* s) {
  *s = skip_ws(*s);
  if (s->len == 0) return true;

  sp_str_t first;
  sp_str_t rest = parse_ident(*s, &first);
  if (first.len == 0) return false;

  if (sp_str_equal_cstr(first, "int")) {
    sp_str_t name;
    rest = parse_ident(rest, &name);
    if (name.len == 0) {
      SP_LOG("error: expected variable name after 'int'");
      return false;
    }

    s32 offset = alloc_var(ctx, name);

    rest = skip_ws(rest);
    if (rest.len > 0 && rest.data[0] == '=') {
      rest.data++; rest.len--;
      expr_t* e;
      rest = parse_expr(rest, &e);
      if (!e) {
        SP_LOG("error: expected expression after '='");
        return false;
      }
      if (!codegen_expr(ctx, e)) return false;
      emit_mov_rax_to_stack(ctx, offset);
    } else {
      emit_mov_imm_to_rax(ctx, 0);
      emit_mov_rax_to_stack(ctx, offset);
    }

    if (!expect_char(&rest, ';')) {
      SP_LOG("error: expected ';'");
      return false;
    }
    *s = rest;
    return true;
  }

  rest = skip_ws(rest);
  if (rest.len > 0 && rest.data[0] == '=') {
    jit_var_t* v = find_var(ctx, first);
    if (!v) {
      SP_LOG("error: undefined variable '{}'", SP_FMT_STR(first));
      return false;
    }
    rest.data++; rest.len--;
    expr_t* e;
    rest = parse_expr(rest, &e);
    if (!e) {
      SP_LOG("error: expected expression");
      return false;
    }
    if (!codegen_expr(ctx, e)) return false;
    emit_mov_rax_to_stack(ctx, v->stack_offset);

    if (!expect_char(&rest, ';')) {
      SP_LOG("error: expected ';'");
      return false;
    }
    *s = rest;
    return true;
  }

  if (sp_str_equal_cstr(first, "return")) {
    expr_t* e;
    rest = parse_expr(rest, &e);
    if (!e) {
      SP_LOG("error: expected expression after 'return'");
      return false;
    }
    if (!codegen_expr(ctx, e)) return false;
    if (!expect_char(&rest, ';')) {
      SP_LOG("error: expected ';'");
      return false;
    }
    *s = rest;
    return true;
  }

  SP_LOG("error: unexpected identifier '{}'", SP_FMT_STR(first));
  return false;
}

typedef s64 (*jit_fn_t)(void);

static jit_fn_t compile(sp_str_t source) {
  jit_ctx_t ctx = SP_ZERO_INITIALIZE();
  ctx.stack_size = 0;

  emit_prologue(&ctx);

  u32 rsp_patch = sp_da_size(ctx.code);
  emit_sub_rsp(&ctx, 0);

  sp_str_t s = source;
  while (s.len > 0) {
    s = skip_ws(s);
    if (s.len == 0) break;
    if (!parse_stmt(&ctx, &s)) {
      return SP_NULLPTR;
    }
  }

  emit_epilogue(&ctx);

  s32 aligned_stack = (ctx.stack_size + 15) & ~15;
  ctx.code[rsp_patch + 3] = aligned_stack & 0xFF;
  ctx.code[rsp_patch + 4] = (aligned_stack >> 8) & 0xFF;
  ctx.code[rsp_patch + 5] = (aligned_stack >> 16) & 0xFF;
  ctx.code[rsp_patch + 6] = (aligned_stack >> 24) & 0xFF;

  u64 code_size = sp_da_size(ctx.code);
  u64 page_size = 4096;
  u64 alloc_size = (code_size + page_size - 1) & ~(page_size - 1);

  void* mem = sp_sys_mmap(
    SP_NULLPTR, alloc_size,
    SP_PROT_READ | SP_PROT_WRITE | SP_PROT_EXEC,
    SP_MAP_PRIVATE | SP_MAP_ANONYMOUS,
    -1, 0
  );

  if (mem == SP_MAP_FAILED) {
    SP_LOG("error: mmap failed");
    return SP_NULLPTR;
  }

  sp_sys_memcpy(mem, ctx.code, code_size);

  SP_LOG("compiled {} bytes of machine code", SP_FMT_U32((u32)code_size));

  return (jit_fn_t)mem;
}

static bool run_test(const c8* name, sp_str_t src, s64 expected) {
  SP_LOG("{}:", SP_FMT_CSTR(name));
  SP_LOG("  source: {}", SP_FMT_STR(src));
  
  jit_fn_t fn = compile(src);
  if (fn) {
    s64 result = fn();
    if (result == expected) {
      SP_LOG("  result: {} {:fg green}", SP_FMT_S64(result), SP_FMT_CSTR("[OK]"));
      return true;
    } else {
      SP_LOG("  result: {} (expected {}) {:fg red}", 
             SP_FMT_S64(result), SP_FMT_S64(expected), SP_FMT_CSTR("[FAIL]"));
    }
  }
  return false;
}

s32 jit_main(s32 argc, const c8** argv);
SP_ENTRY(jit_main)

s32 jit_main(s32 argc, const c8** argv) {
  (void)argc; (void)argv;
  sp_sys_init();
  SP_LOG("=== sp.h JIT compiler test ===\n");

  s32 failures = 0;
  if (!run_test("test 1: simple arithmetic",
           sp_str_lit("int x = 10; int y = 32; return x + y;"), 42)) failures++;

  if (!run_test("test 2: multiplication",
           sp_str_lit("int a = 6; int b = 7; return a * b;"), 42)) failures++;

  if (!run_test("test 3: complex expression",
           sp_str_lit("int x = 5; int y = 3; int z = x * y + x - y; return z;"), 17)) failures++;

  if (!run_test("test 4: reassignment",
           sp_str_lit("int x = 10; x = x + 5; x = x * 2; return x;"), 30)) failures++;

  SP_LOG("\n=== done ===");
  return failures;
}
