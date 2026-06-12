#define SP_IMPLEMENTATION
#include "sp.h"

// Custom formatters are straightforward. Instead of pulling the value to be formatted from
// the value-typed members of the union in sp_fmt_arg_t, you cast the pointer member.
//
// From there, you can append arbitrary bytes to the string builder being used for this
// call to sp_fmt(mem, ...).value. You can (and are encouraged to) use sp.h's internal helpers for
// printing primitives, and you have access to the :specifier's width and precision.
//
// Then, to provide a convenient and type-safe call site, define the sp_fmt_*() macros that
// callers will use as wrappers over sp_fmt_custom()
typedef struct { f32 x; f32 y; } point_t;

void format_point(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  point_t* point = (point_t*)arg->value.custom.ptr;
  u32 precision = sp_opt_is_null(arg->spec.precision) ? 2 : sp_opt_get(arg->spec.precision);
  sp_io_write_c8(io, '(');
  sp_fmt_write_f64(io, point->x, precision);
  sp_io_write_cstr(io, ", ", SP_NULLPTR);
  sp_fmt_write_f64(io, point->y, precision);
  sp_io_write_c8(io, ')');
}
#define sp_fmt_point(p) sp_fmt_custom(point_t, format_point, (p))
#define sp_fmt_point_v(...) sp_fmt_custom_v(point_t, format_point, (point_t){__VA_ARGS__})

static void section(const c8* title) {
  sp_log("");
  sp_log("{.gray .italic}", sp_fmt_cstr(title));
}

void specifier(s64 n, c8 fill, sp_fmt_align_t align, u8 width) {
  const c8* fmt = SP_NULLPTR;
  switch (align) {
    case SP_FMT_ALIGN_LEFT: fmt = ":{.cyan}{.magenta}{.yellow} -> {:$<$}"; break;
    case SP_FMT_ALIGN_CENTER: fmt = ":{.cyan}{.magenta}{.yellow} -> {:$^$}"; break;
    case SP_FMT_ALIGN_RIGHT: fmt = ":{.cyan}{.magenta}{.yellow} -> {:$>$}"; break;
    case SP_FMT_ALIGN_NONE: break;
  }

  c8 aligner = 0;
  switch (align) {
    case SP_FMT_ALIGN_LEFT: aligner = '<';
    case SP_FMT_ALIGN_CENTER: aligner = '^';
    case SP_FMT_ALIGN_RIGHT: aligner = '>';
    case SP_FMT_ALIGN_NONE: aligner = 0;
  }
  sp_log(
    fmt,
    sp_fmt_char(fill),
    sp_fmt_char(aligner), sp_fmt_uint(width),
    sp_fmt_int(fill), sp_fmt_uint(width),
    sp_fmt_int(n)
  );
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_log("{.green} has Zig/Rust style format specifiers (fill, align, width), plus named directives which may:", sp_fmt_cstr("sp.h"));
  sp_log("- {} text from a format argument", sp_fmt_cstr("render"));
  sp_log("- {.bold .cyan} the rendered text", sp_fmt_cstr("decorate"));

  section("decorators");
  sp_log("{:<14 .italic} -> hello, {.bold}", sp_fmt_cstr("hello, {.bold}"), sp_fmt_cstr("world"));
  sp_log("{:<14 .italic} -> {.hyperlink}", sp_fmt_cstr("{.hyperlink}"), sp_fmt_cstr("https://spader.zone"));
  sp_log("{:<14 .italic} -> {.quote}", sp_fmt_cstr("{.quote}"), sp_fmt_cstr("supposedly"));

  section(".bytes");
  u64 byte_samples[] = { 0ULL, 512ULL, 1536ULL, 10485760ULL, 5368709120ULL };
  sp_carr_for(byte_samples, i) {
    sp_log("{:<10} -> {.bytes}", sp_fmt_uint(byte_samples[i]), sp_fmt_uint(byte_samples[i]));
  }

  section(".iso");
  u64 epoch_samples[] = { 0ULL, 1705330245ULL, 1735689599ULL };
  sp_carr_for(epoch_samples, i) {
    sp_log("{:<10} -> {.iso}", sp_fmt_uint(epoch_samples[i]), sp_fmt_uint(epoch_samples[i]));
  }

  section(".ordinal");
  s64 ord_samples[] = { 1, 2, 3, 4, 11, 12, 13, 21, 22, 23, 101, 102, 113 };
  sp_carr_for(ord_samples, i) {
    sp_log("{:<3} -> {.ordinal}", sp_fmt_int(ord_samples[i]), sp_fmt_int(ord_samples[i]));
  }

  section(".duration");
  u64 dur_samples[] = { 500ULL, 1500ULL, 2500000ULL, 3500000000ULL, 90000000000ULL };
  sp_carr_for(dur_samples, i) {
    sp_log("{:<11} -> {.duration}", sp_fmt_uint(dur_samples[i]), sp_fmt_uint(dur_samples[i]));
  }

  section(":specifier");
  specifier(69, '*', SP_FMT_ALIGN_LEFT, 9);
  specifier(69, '*', SP_FMT_ALIGN_CENTER, 9);
  specifier(69, '*', SP_FMT_ALIGN_RIGHT, 9);
  specifier(69, '0', SP_FMT_ALIGN_RIGHT, 5);
  specifier(694, '0', SP_FMT_ALIGN_RIGHT, 5);
  specifier(6942, '0', SP_FMT_ALIGN_RIGHT, 5);
  specifier(69420, '0', SP_FMT_ALIGN_RIGHT, 5);

  section("composition");
  struct { const c8* name; sp_str_t str; } examples [] = {
    { ".italic + .bold", sp_fmt(mem, "i never {.italic .bold} the kenosha kid", sp_fmt_cstr("did")).value },
    { ".bold + .cyan", sp_fmt(mem, "i never {.bold .cyan} the kenosha kid", sp_fmt_cstr("did")).value },
    { "kitchen sink", sp_fmt(mem, "i never {.quote .bold .italic .green} the kenosha kid", sp_fmt_cstr("did")).value },
    {
      ".bytes + :specifier",
      sp_fmt(mem, "{} bytes is [{:^$ .bytes}]", sp_fmt_uint(1536), sp_fmt_uint(12), sp_fmt_uint(1536)).value
    },
    {
      ".duration + .bold",
      sp_fmt(mem, "{.bold .duration}", sp_fmt_uint(2500000)).value
    }

  };

  u32 width = 0;
  sp_carr_for(examples, it) {
    width = sp_max(width, sp_cstr_len(examples[it].name));
  }
  sp_carr_for(examples, it) {
    sp_log("{:<$} -> {}", sp_fmt_uint(width), sp_fmt_cstr(examples[it].name), sp_fmt_str(examples[it].str));
  }

  section("custom");
  point_t point = { 69, 420 };

  // In C, you can use the &(foo_t) { 69 } syntax to take the address of a temporary and omit
  // the variable, which is pretty nice. But it's not possible in C++.
  #if !defined(SP_CPP)
  sp_log("{}", sp_fmt_point(((point_t) { 69, 420 })));
  sp_log("{}", sp_fmt_point_v(69, 420));
  #endif
  sp_log("{.cyan .bold}", sp_fmt_point(point));

  return 0;
}
SP_MAIN(run)
