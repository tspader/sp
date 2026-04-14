#define SP_IMPLEMENTATION
#include "sp.h"


static void section(const c8* title) {
  _sp_log("{:gray}", _sp_fmt_cstr(title));
}

typedef struct { f32 x; f32 y; } point_t;

void format_point(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  point_t* point = (point_t*)arg->custom.ptr;
  u32 p = sp_opt_is_null(arg->spec.precision) ? 2 : sp_opt_get(arg->spec.precision);
  sp_str_builder_append_c8(b, '(');
  _sp_fmt_write_f64(b, point->x, p);
  sp_str_builder_append_cstr(b, ", ");
  _sp_fmt_write_f64(b, point->y, p);
  sp_str_builder_append_c8(b, ')');
}
#define SP_FMT_POINT(p) _sp_fmt_custom(point_t, format_point, (p))
#define SP_FMT_POINT_V(...) _sp_fmt_custom_v(point_t, format_point, (point_t){__VA_ARGS__})

s32 run(s32 num_args, const c8** args) {
  section("wrapper directives");
  _sp_log("  :bold       -> {:bold}",      _sp_fmt_cstr("hello world"));
  _sp_log("  :hyperlink  -> {:hyperlink}", _sp_fmt_cstr("https://anthropic.com"));
  _sp_log("  :quote      -> {:quote}",     _sp_fmt_cstr("secret"));

  section("content-mutation directives");
  _sp_log("  :upper      -> {:upper}",  _sp_fmt_cstr("hello world"));
  _sp_log("  :redact     -> {:redact}", _sp_fmt_cstr("hunter2"));

  section(":bytes");
  u64 byte_samples[] = { 0ULL, 512ULL, 1536ULL, 10485760ULL, 5368709120ULL };
  sp_carr_for(byte_samples, i) {
    _sp_log("{:12} -> {:bytes :cyan}", _sp_fmt_u64(byte_samples[i]), _sp_fmt_u64(byte_samples[i]));
  }

  section(":iso");
  u64 epoch_samples[] = { 0ULL, 1705330245ULL, 1735689599ULL };
  sp_carr_for(epoch_samples, i) {
    _sp_log("{:12} -> {:iso :cyan}", _sp_fmt_u64(epoch_samples[i]), _sp_fmt_u64(epoch_samples[i]));
  }

  section(":ordinal");
  s64 ord_samples[] = { 1, 2, 3, 4, 11, 12, 13, 21, 22, 23, 101, 102, 113 };
  sp_carr_for(ord_samples, i) {
    _sp_log("  {:4} -> {:ordinal}", _sp_fmt_s64(ord_samples[i]), _sp_fmt_s64(ord_samples[i]));
  }

  section(":errno");
  s32 errno_samples[] = { 1, 2, 5, 11, 22, 99 };
  sp_carr_for(errno_samples, i) {
    _sp_log("  {:4} -> {:errno}", _sp_fmt_int(errno_samples[i]), _sp_fmt_int(errno_samples[i]));
  }

  section(":duration");
  u64 dur_samples[] = { 500ULL, 1500ULL, 2500000ULL, 3500000000ULL, 90000000000ULL };
  sp_carr_for(dur_samples, i) {
    _sp_log("  {:14} -> {:duration}", _sp_fmt_u64(dur_samples[i]), _sp_fmt_u64(dur_samples[i]));
  }

  section("composed directives");
  _sp_log("bold + upper:    {/^11 :bold :upper}", _sp_fmt_cstr("hello"));
  _sp_log("quote + upper:   {:quote :upper}", _sp_fmt_cstr("hello"));
  _sp_log("bytes + padding: {} bytes is [{:^$ :bytes}]", _sp_fmt_u64(1536), _sp_fmt_u64(12), _sp_fmt_u64(1536));
  _sp_log("ordinal center:  {:*^8 :ordinal}", _sp_fmt_s64(21));
  _sp_log("duration + bold: {:bold :duration}", _sp_fmt_u64(2500000));

  _sp_log("bytes with float:    {:bytes}", _sp_fmt_f64(69.69));

  sp_log_str(sp_fmt("{:cyan}", _sp_fmt_cstr("69")));
  point_t point = { 69, 420 };
  _sp_log("{:cyan :bold}", SP_FMT_POINT(point));
  _sp_log("{}", SP_FMT_POINT(((point_t) { 69, 420 })));
  _sp_log("{}", SP_FMT_POINT_V(69, 420));

  return 0;
}
SP_ENTRY(run)
