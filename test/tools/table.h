#ifndef SP_TABLE_H
#define SP_TABLE_H

#include "sp.h"

typedef struct {
  sp_str_t header;
  const c8* fmt;
  sp_fmt_align_t align;
} sp_table_col_t;

typedef struct {
  sp_str_t value;
  const c8* color;
} sp_table_cell_t;

typedef struct {
  sp_mem_t mem;
  sp_da(sp_table_col_t) cols;
  sp_da(sp_table_cell_t) cells;
  const c8* color;
} sp_table_writer_t;

SP_API void     sp_table_init(sp_table_writer_t* table, sp_mem_t mem);
SP_API void     sp_table_add_col(sp_table_writer_t* table, sp_table_col_t col);
SP_API void     sp_table_begin(sp_table_writer_t* table);
SP_API void     sp_table_color(sp_table_writer_t* table, const c8* ansi);
SP_API void     sp_table_write_str(sp_table_writer_t* table, sp_str_t value);
SP_API void     sp_table_write_cstr(sp_table_writer_t* table, const c8* value);
SP_API void     sp_table_write_u64(sp_table_writer_t* table, u64 value);
SP_API void     sp_table_write_s64(sp_table_writer_t* table, s64 value);
SP_API void     sp_table_write_u32(sp_table_writer_t* table, u32 value);
SP_API void     sp_table_write_s32(sp_table_writer_t* table, s32 value);
SP_API void     sp_table_write_f64(sp_table_writer_t* table, f64 value);
SP_API void     sp_table_write_f32(sp_table_writer_t* table, f32 value);
SP_API sp_str_t sp_table_render(sp_table_writer_t* table, sp_mem_t mem);
SP_API void     sp_table_log(sp_table_writer_t* table);

#endif

#if defined(SP_TABLE_IMPLEMENTATION) && !defined(SP_TABLE_C)
#define SP_TABLE_C

void sp_table_init(sp_table_writer_t* table, sp_mem_t mem) {
  sp_mem_zero(table, sizeof(*table));
  table->mem = mem;
  sp_da_init(mem, table->cols);
  sp_da_init(mem, table->cells);
}

void sp_table_add_col(sp_table_writer_t* table, sp_table_col_t col) {
  SP_ASSERT(sp_da_empty(table->cells));
  if (!col.fmt) col.fmt = "{}";
  sp_da_push(table->cols, col);
}

void sp_table_begin(sp_table_writer_t* table) {
  SP_ASSERT(!sp_da_empty(table->cols));
  SP_ASSERT(sp_da_size(table->cells) % sp_da_size(table->cols) == 0);
}

static const sp_table_col_t* sp_table_current_col(sp_table_writer_t* table) {
  return &table->cols[sp_da_size(table->cells) % sp_da_size(table->cols)];
}

void sp_table_color(sp_table_writer_t* table, const c8* ansi) {
  table->color = ansi;
}

static void sp_table_push_cell(sp_table_writer_t* table, sp_str_t value) {
  sp_table_cell_t cell = {
    .value = value,
    .color = table->color,
  };
  sp_da_push(table->cells, cell);
  table->color = SP_NULLPTR;
}

void sp_table_write_str(sp_table_writer_t* table, sp_str_t value) {
  sp_table_push_cell(table, sp_str_copy(table->mem, value));
}

void sp_table_write_cstr(sp_table_writer_t* table, const c8* value) {
  sp_table_write_str(table, sp_str_view(value));
}

void sp_table_write_u64(sp_table_writer_t* table, u64 value) {
  sp_table_push_cell(table, sp_fmt(table->mem, sp_table_current_col(table)->fmt, sp_fmt_uint(value)).value);
}

void sp_table_write_s64(sp_table_writer_t* table, s64 value) {
  sp_table_push_cell(table, sp_fmt(table->mem, sp_table_current_col(table)->fmt, sp_fmt_int(value)).value);
}

void sp_table_write_u32(sp_table_writer_t* table, u32 value) {
  sp_table_write_u64(table, (u64)value);
}

void sp_table_write_s32(sp_table_writer_t* table, s32 value) {
  sp_table_write_s64(table, (s64)value);
}

void sp_table_write_f64(sp_table_writer_t* table, f64 value) {
  sp_table_push_cell(table, sp_fmt(table->mem, sp_table_current_col(table)->fmt, sp_fmt_float(value)).value);
}

void sp_table_write_f32(sp_table_writer_t* table, f32 value) {
  sp_table_write_f64(table, (f64)value);
}

static const c8* sp_table_pad_fmt(sp_fmt_align_t align) {
  switch (align) {
    case SP_FMT_ALIGN_NONE:   return "{:<$}";
    case SP_FMT_ALIGN_LEFT:   return "{:<$}";
    case SP_FMT_ALIGN_CENTER: return "{:^$}";
    case SP_FMT_ALIGN_RIGHT:  return "{:>$}";
  }
  return "{:<$}";
}

static void sp_table_render_cell(sp_io_writer_t* io, const sp_table_col_t* col, sp_table_cell_t cell, u32 width, bool last) {
  if (cell.color) sp_io_write_cstr(io, cell.color, SP_NULLPTR);
  if (last && (col->align == SP_FMT_ALIGN_NONE || col->align == SP_FMT_ALIGN_LEFT)) {
    sp_io_write_str(io, cell.value, SP_NULLPTR);
  }
  else {
    sp_fmt_io(io, sp_table_pad_fmt(col->align), sp_fmt_uint(width), sp_fmt_str(cell.value));
  }
  if (cell.color) sp_io_write_cstr(io, SP_ANSI_RESET, SP_NULLPTR);
}

sp_str_t sp_table_render(sp_table_writer_t* table, sp_mem_t mem) {
  u32 num_cols = (u32)sp_da_size(table->cols);
  SP_ASSERT(num_cols);
  SP_ASSERT(sp_da_size(table->cells) % num_cols == 0);
  u32 num_rows = (u32)(sp_da_size(table->cells) / num_cols);

  u32* widths = sp_alloc_n(mem, u32, num_cols);
  sp_for(col, num_cols) {
    widths[col] = table->cols[col].header.len;
  }
  sp_da_for(table->cells, it) {
    u32 col = (u32)(it % num_cols);
    widths[col] = sp_max(widths[col], table->cells[it].value.len);
  }

  sp_io_dyn_mem_writer_t builder = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &builder);

  sp_io_write_cstr(&builder.base, SP_ANSI_FG_BRIGHT_BLACK, SP_NULLPTR);
  sp_for(col, num_cols) {
    if (col) sp_io_write_cstr(&builder.base, "  ", SP_NULLPTR);
    sp_table_cell_t header = { .value = table->cols[col].header };
    sp_table_render_cell(&builder.base, &table->cols[col], header, widths[col], col == num_cols - 1);
  }
  sp_io_write_cstr(&builder.base, SP_ANSI_RESET, SP_NULLPTR);
  // sp_io_write_c8(&builder.base, '\n');
  // sp_for(col, num_cols) {
  //   if (col) sp_io_write_cstr(&builder.base, "  ", SP_NULLPTR);
  //   sp_for(it, widths[col]) {
  //     sp_io_write_c8(&builder.base, '-');
  //   }
  // }

  sp_for(row, num_rows) {
    sp_io_write_c8(&builder.base, '\n');
    sp_for(col, num_cols) {
      if (col) sp_io_write_cstr(&builder.base, "  ", SP_NULLPTR);
      sp_table_render_cell(&builder.base, &table->cols[col], table->cells[row * num_cols + col], widths[col], col == num_cols - 1);
    }
  }

  return sp_io_dyn_mem_writer_as_str(&builder);
}

void sp_table_log(sp_table_writer_t* table) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_log("{}", sp_fmt_str(sp_table_render(table, scratch.mem)));
  sp_mem_end_scratch(scratch);
}

#endif
