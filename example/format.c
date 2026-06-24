#define SP_IMPLEMENTATION
#include "sp.h"

void render_table();
void push_row(sp_str_t fmt, sp_str_t args, sp_str_t result);
#define row(_fmt, ...) push_row(sp_str_lit(_fmt), sp_str_lit(#__VA_ARGS__), sp_fmt(mem, _fmt, ##__VA_ARGS__).value)

static sp_mem_t mem;

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  mem = sp_mem_heap_as_allocator(heap);

  {
    sp_str_t str = sp_fmt(mem, "hello, {}!", sp_fmt_cstr("world")).value;
  }

  {
    sp_str_r result = sp_fmt(mem, "hello, {}!", sp_fmt_cstr("world"));
    if (result.err) {
      // ...
    }
    sp_str_t str = result.value;
  }

  {
    c8 buffer [64] = sp_zero;
    sp_io_mem_writer_t io = sp_zero;
    sp_io_mem_writer_from_buffer(&io, buffer, sizeof(buffer));
    if (sp_fmt_io(&io.base, "hello, {}", sp_fmt_cstr("world"))) {
      // ...
    }
    sp_str_t str = sp_io_mem_writer_as_str(&io);
  }

  {
    sp_io_dyn_mem_writer_t io = sp_zero;
    sp_io_dyn_mem_writer_init(mem, &io);
    sp_fmt_io(&io.base, "hello, {}!", sp_fmt_cstr("world"));
    sp_str_t str = sp_io_dyn_mem_writer_as_str(&io);
    sp_str_t owned = sp_io_dyn_mem_writer_take_str(&io);
  }

  row("{}", sp_fmt_int(-42));
  row("{}", sp_fmt_uint(42));
  row("{}", sp_fmt_float(1.5));
  row("{}", sp_fmt_cstr("hi"));
  row("{}", sp_fmt_ptr((void*)0xabcd));
  row("{{}}");
  row("{:6}", sp_fmt_int(42));
  row("{:<6}", sp_fmt_int(42));
  row("{:^6}", sp_fmt_int(42));
  row("{:>6}", sp_fmt_int(42));
  row("{:*^9}", sp_fmt_int(42));
  row("{:0>5}", sp_fmt_int(42));
  row("{:.2}", sp_fmt_float(3.14159));
  row("{:.3}", sp_fmt_cstr("hello"));
  row("{:$}", sp_fmt_uint(6), sp_fmt_int(42));
  row("{:.$}", sp_fmt_uint(3), sp_fmt_float(3.14159));
  row("{:$^9}", sp_fmt_int('*'), sp_fmt_int(42));
  row("{:$<$.$}", sp_fmt_int('*'), sp_fmt_uint(8), sp_fmt_uint(2), sp_fmt_float(1.5));
  row("{:x}", sp_fmt_uint(255));
  row("{:X}", sp_fmt_uint(255));
  row("{:o}", sp_fmt_uint(255));
  row("{:b}", sp_fmt_uint(255));
  row("{:c}", sp_fmt_uint('A'));
  row("{B}", sp_fmt_uint(1536));
  row("{.red}", sp_fmt_cstr("error"));
  row("{.bold}", sp_fmt_cstr("strong"));
  row("{.italic}", sp_fmt_cstr("emphasis"));
  row("{.quote}", sp_fmt_cstr("supposedly"));
  row("{.bold .italic .green}", sp_fmt_cstr("composed"));
  render_table();


  return SP_OK;
}
SP_MAIN(run)

////////////
// TABLES //
////////////
// This is a tiny table renderer to visualize the examples. It has no bearing
// on the example itself and you should skip over it.
typedef struct {
  sp_str_t text;
  sp_fmt_style_t style;
} cell_t;

typedef struct {
  cell_t cols [3];
} row_t;

typedef struct {
  sp_da(row_t) rows;
  u32 width [3];
} ctx_t;

static ctx_t ctx;

void push_row(sp_str_t fmt, sp_str_t args, sp_str_t result) {
  if (!ctx.rows) sp_da_init(mem, ctx.rows);
  sp_da_push(ctx.rows, ((row_t){{
    { .text = fmt,    .style = sp_fmt_style_yellow },
    { .text = args,   .style = sp_fmt_style_none },
    { .text = result, .style = sp_fmt_style_none },
  }}));
}

void render_row(row_t row) {
  sp_str_t cells [3];
  sp_for(it, 3) {
    cells[it] = sp_fmt(mem, "{:<$ .$}",
      sp_fmt_uint(ctx.width[it]),
      sp_fmt_uint(row.cols[it].style),
      sp_fmt_str(row.cols[it].text)
    ).value;
  }
  sp_log("{}", sp_fmt_str(sp_str_join_n(mem, cells, 3, sp_str_lit(" "))));
}

void render_table() {
  sp_da_for(ctx.rows, r) {
    sp_for(c, 3) ctx.width[c] = sp_max(ctx.width[c], ctx.rows[r].cols[c].text.len);
  }

  render_row((row_t){{
    { sp_str_lit("fmt"),    sp_fmt_style_gray },
    { sp_str_lit("args"),   sp_fmt_style_gray },
    { sp_str_lit("result"), sp_fmt_style_gray },
  }});
  sp_da_for(ctx.rows, r) render_row(ctx.rows[r]);
}
