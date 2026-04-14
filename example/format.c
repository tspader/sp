#define SP_IMPLEMENTATION
#include "sp.h"


static void bold_before(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  (void)arg;
  sp_str_builder_append_cstr(b, "\033[1m");
}

static void bold_after(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  (void)content; (void)arg;
  sp_str_builder_append_cstr(b, "\033[22m");
}

static void gray_before(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  sp_unused(arg);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_BLACK);
}

static void ansi_reset(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  sp_unused(arg); sp_unused(content);
  sp_str_builder_append_cstr(b, SP_ANSI_RESET);
}

static void cyan_before(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  sp_unused(arg);
  sp_str_builder_append_cstr(b, SP_ANSI_FG_BRIGHT_CYAN);
}

static void hyperlink_before(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  sp_str_builder_append_cstr(b, "\033]8;;");
  if (arg->id == _sp_fmt_id_str) sp_str_builder_append(b, arg->s);
  sp_str_builder_append_cstr(b, "\033\\");
}

static void hyperlink_after(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  (void)content; (void)arg;
  sp_str_builder_append_cstr(b, "\033]8;;\033\\");
}

static void quote_before(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  (void)arg;
  sp_str_builder_append_c8(b, '"');
}

static void quote_after(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  (void)content; (void)arg;
  sp_str_builder_append_c8(b, '"');
}

static void upper_after(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  (void)b; (void)arg;
  sp_mem_slice_for(content, i) {
    u8 c = content.data[i];
    if (c >= 'a' && c <= 'z') content.data[i] = c - 32;
  }
}

static void redact_after(sp_str_builder_t* b, sp_mem_slice_t content, _sp_fmt_arg_t* arg) {
  (void)b; (void)arg;
  sp_mem_slice_for(content, i) content.data[i] = '*';
}

static void bytes_render(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  u64 bytes = arg->u;
  static const c8* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
  u32 unit_idx = 0;
  u64 whole = bytes;
  u64 rem = 0;
  while (whole >= 1024 && unit_idx < 5) {
    rem = whole & 1023;
    whole >>= 10;
    unit_idx++;
  }
  _sp_fmt_write_u64(b, whole);
  if (unit_idx > 0) {
    u32 tenths = (u32)((rem * 10) >> 10);
    if (tenths > 0) {
      sp_str_builder_append_c8(b, '.');
      sp_str_builder_append_c8(b, (c8)('0' + tenths));
    }
  }
  sp_str_builder_append_c8(b, ' ');
  sp_str_builder_append_cstr(b, units[unit_idx]);
}

static void _write_zpad2(sp_str_builder_t* b, u32 value) {
  sp_str_builder_append_c8(b, (c8)('0' + (value / 10) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + value % 10));
}

static void _write_zpad4(sp_str_builder_t* b, u32 value) {
  sp_str_builder_append_c8(b, (c8)('0' + (value / 1000) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + (value / 100) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + (value / 10) % 10));
  sp_str_builder_append_c8(b, (c8)('0' + value % 10));
}

static void iso_render(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  u64 epoch = arg->u;
  u32 sec = (u32)(epoch % 60); epoch /= 60;
  u32 min = (u32)(epoch % 60); epoch /= 60;
  u32 hour = (u32)(epoch % 24); epoch /= 24;

  s64 days = (s64)epoch + 719468;
  s64 era = (days >= 0 ? days : days - 146096) / 146097;
  u32 doe = (u32)(days - era * 146097);
  u32 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  s64 y = (s64)yoe + era * 400;
  u32 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  u32 mp = (5 * doy + 2) / 153;
  u32 d = doy - (153 * mp + 2) / 5 + 1;
  u32 m = mp < 10 ? mp + 3 : mp - 9;
  if (m <= 2) y += 1;

  _write_zpad4(b, (u32)y);
  sp_str_builder_append_c8(b, '-');
  _write_zpad2(b, m);
  sp_str_builder_append_c8(b, '-');
  _write_zpad2(b, d);
  sp_str_builder_append_c8(b, 'T');
  _write_zpad2(b, hour);
  sp_str_builder_append_c8(b, ':');
  _write_zpad2(b, min);
  sp_str_builder_append_c8(b, ':');
  _write_zpad2(b, sec);
  sp_str_builder_append_c8(b, 'Z');
}

static void ordinal_render(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  s64 value = (arg->id == _sp_fmt_id_s64) ? arg->i : (s64)arg->u;
  _sp_fmt_write_s64(b, value);
  s64 abs = value < 0 ? -value : value;
  u32 mod100 = (u32)(abs % 100);
  u32 mod10 = (u32)(abs % 10);
  const c8* suffix = "th";
  if (mod100 < 11 || mod100 > 13) {
    if (mod10 == 1) suffix = "st";
    else if (mod10 == 2) suffix = "nd";
    else if (mod10 == 3) suffix = "rd";
  }
  sp_str_builder_append_cstr(b, suffix);
}

static const struct { s32 code; const c8* name; } _errno_table[] = {
  {  1, "EPERM"  },
  {  2, "ENOENT" },
  {  5, "EIO"    },
  {  9, "EBADF"  },
  { 11, "EAGAIN" },
  { 12, "ENOMEM" },
  { 13, "EACCES" },
  { 17, "EEXIST" },
  { 22, "EINVAL" },
  { 28, "ENOSPC" },
  { 32, "EPIPE"  },
};

static void errno_render(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  s64 value = (arg->id == _sp_fmt_id_s64) ? arg->i : (s64)arg->u;
  s32 code = (s32)value;
  const c8* name = SP_NULLPTR;
  sp_carr_for(_errno_table, i) {
    if (_errno_table[i].code == code) { name = _errno_table[i].name; break; }
  }
  if (name != SP_NULLPTR) {
    sp_str_builder_append_cstr(b, name);
  } else {
    sp_str_builder_append_cstr(b, "errno(");
    _sp_fmt_write_s64(b, value);
    sp_str_builder_append_c8(b, ')');
  }
}

static void duration_render(sp_str_builder_t* b, _sp_fmt_arg_t* arg) {
  u64 ns = arg->u;
  if (ns < 1000) {
    _sp_fmt_write_u64(b, ns);
    sp_str_builder_append_cstr(b, " ns");
    return;
  }
  static const c8* units[] = { "us", "ms", "s" };
  u32 unit_idx = 0;
  u64 whole = ns / 1000;
  u64 rem = ns % 1000;
  while (whole >= 1000 && unit_idx < 2) {
    rem = whole % 1000;
    whole /= 1000;
    unit_idx++;
  }
  _sp_fmt_write_u64(b, whole);
  if (rem >= 100) {
    sp_str_builder_append_c8(b, '.');
    sp_str_builder_append_c8(b, (c8)('0' + rem / 100));
  }
  sp_str_builder_append_c8(b, ' ');
  sp_str_builder_append_cstr(b, units[unit_idx]);
}

static void register_directives(void) {
  sp_fmt_directive_register(sp_str_view("cyan"),
    (_sp_fmt_directive_t){ .before = cyan_before, .after = ansi_reset });
  sp_fmt_directive_register(sp_str_view("gray"),
    (_sp_fmt_directive_t){ .before = gray_before, .after = ansi_reset });
  sp_fmt_directive_register(sp_str_view("bold"),
    (_sp_fmt_directive_t){ .before = bold_before, .after = bold_after });
  sp_fmt_directive_register(sp_str_view("hyperlink"),
    (_sp_fmt_directive_t){ .before = hyperlink_before, .after = hyperlink_after });
  sp_fmt_directive_register(sp_str_view("quote"),
    (_sp_fmt_directive_t){ .before = quote_before, .after = quote_after });
  sp_fmt_directive_register(sp_str_view("upper"),
    (_sp_fmt_directive_t){ .after = upper_after });
  sp_fmt_directive_register(sp_str_view("redact"),
    (_sp_fmt_directive_t){ .after = redact_after });
  sp_fmt_directive_register(sp_str_view("bytes"),
    (_sp_fmt_directive_t){ .render = bytes_render, .kinds = _sp_fmt_id_u64 });
  sp_fmt_directive_register(sp_str_view("iso"),
    (_sp_fmt_directive_t){ .render = iso_render, .kinds = _sp_fmt_id_u64 });
  sp_fmt_directive_register(sp_str_view("ordinal"),
    (_sp_fmt_directive_t){ .render = ordinal_render, .kinds = _sp_fmt_id_s64 | _sp_fmt_id_u64 });
  sp_fmt_directive_register(sp_str_view("errno"),
    (_sp_fmt_directive_t){ .render = errno_render, .kinds = _sp_fmt_id_s64 | _sp_fmt_id_u64 });
  sp_fmt_directive_register(sp_str_view("duration"),
    (_sp_fmt_directive_t){ .render = duration_render, .kinds = _sp_fmt_id_u64 });
}

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

int main(void) {
  register_directives();

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

