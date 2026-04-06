#ifndef SP_MATH_H
#define SP_MATH_H

#include "sp.h"

// ███╗   ███╗ █████╗ ████████╗██╗  ██╗
// ████╗ ████║██╔══██╗╚══██╔══╝██║  ██║
// ██╔████╔██║███████║   ██║   ███████║
// ██║╚██╔╝██║██╔══██║   ██║   ██╔══██║
// ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║
// ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝
// @math
// Totally ripped from Handmade Math and tweaked to follow sp.h's style conventions. It's public domain, but here's
// the credits section from the version I ripped. Any usefulness of any of the vector code in this or derived
// libraries is solely due to the excellent work done by the following folks:
//
// https://github.com/HandmadeMath/HandmadeMath
//
//   CREDITS
//
//   Originally written by Zakary Strange.
//
//   Functionality:
//    Zakary Strange (strangezak@protonmail.com && @strangezak)
//    Matt Mascarenhas (@miblo_)
//    Aleph
//    FieryDrake (@fierydrake)
//    Gingerbill (@TheGingerBill)
//    Ben Visness (@bvisness)
//    Trinton Bullard (@Peliex_Dev)
//    @AntonDan
//    Logan Forman (@dev_dwarf)
//
//   Fixes:
//    Jeroen van Rijn (@J_vanRijn)
//    Kiljacken (@Kiljacken)
//    Insofaras (@insofaras)
//    Daniel Gibson (@DanielGibson)
#define SP_ABS(a) ((a) > 0 ? (a) : -(a))
#define sp_abs(a) SP_ABS(a)
#define SP_MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define sp_mod(a, m) SP_MOD(a, m)
#define SP_SQUARE(x) ((x) * (x))
#define sp_square(x) SP_SQUARE(x)

#if defined (SP_USE_LIBM)
  #define sp_sqrtf sqrtf
  #define sp_expf expf
  #define sp_sinf sinf
  #define sp_cosf cosf
  #define sp_tanf tanf
  #define sp_acosf acosf
#else
  #define sp_sqrtf sp_sys_sqrtf
  #define sp_expf sp_sys_expf
  #define sp_sinf sp_sys_sinf
  #define sp_cosf sp_sys_cosf
  #define sp_tanf sp_sys_tanf
  #define sp_acosf sp_sys_acosf
#endif

typedef union sp_vec2 {
  struct {
    f32 x, y;
  };

  struct {
    f32 u, v;
  };

  struct {
    f32 left, right;
  };

  struct {
    f32 width, height;
  };

  f32 elements[2];
} sp_vec2_t;

typedef union sp_vec3 {
  struct {
    f32 x, y, z;
  };

  struct {
    f32 u, v, w;
  };

  struct {
    f32 R, G, B;
  };

  struct {
    sp_vec2_t xy;
    f32 unused0;
  };

  struct {
    f32 unused1;
    sp_vec2_t yz;
  };

  struct {
    sp_vec2_t uv;
    f32 unused2;
  };

  struct {
    f32 unused3;
    sp_vec2_t vw;
  };

  f32 elements[3];
} sp_vec3_t;

typedef union sp_vec4 {
  struct {
    union {
      sp_vec3_t xyz;
      struct { f32 x, y, z; };
    };

    f32 w;
  };

  struct {
    union {
      sp_vec3_t rgb;
      struct { f32 r, g, b; };

      sp_vec3_t hsv;
      struct { f32 h, s, v; };
    };

    f32 a;
  };

  struct {
    sp_vec2_t xy;
    f32 unused0;
    f32 unused1;
  };

  struct {
    f32 unused2;
    sp_vec2_t yz;
    f32 unused3;
  };

  struct {
    f32 unused4;
    f32 unused5;
    sp_vec2_t zw;
  };

  f32 elements[4];
} sp_vec4_t;

typedef union sp_mat2 {
  f32 elements[2][2];
  sp_vec2_t columns[2];
} sp_mat2_t;

typedef union sp_mat3 {
  f32 elements[3][3];
  sp_vec3_t columns[3];
} sp_mat3_t;

typedef union sp_mat4 {
  f32 elements[4][4];
  sp_vec4_t columns[4];
} sp_mat4_t;

typedef union sp_quat {
  struct {
    union {
      sp_vec3_t xyz;
      struct { f32 x, y, z; };
    };

    f32 w;
  };

  f32 elements[4];
} sp_quat_t;

typedef sp_vec4_t sp_color_t;

typedef enum sp_interp_mode_t {
  SP_INTERP_MODE_LERP,
  SP_INTERP_MODE_EASE_IN,
  SP_INTERP_MODE_EASE_OUT,
  SP_INTERP_MODE_EASE_INOUT,
  SP_INTERP_MODE_EASE_INOUT_BOUNCE,
  SP_INTERP_MODE_EXPONENTIAL,
  SP_INTERP_MODE_PARABOLIC,
  SP_INTERP_MODE_COUNT
} sp_interp_mode_t;

typedef struct sp_interp_t {
  f32 start;
  f32 delta;
  f32 t;
  f32 time_scale;
} sp_interp_t;

SP_API f32          sp_sys_sqrtf(f32 x);
SP_API f32          sp_sys_expf(f32 x);
SP_API f32          sp_sys_sinf(f32 x);
SP_API f32          sp_sys_cosf(f32 x);
SP_API f32          sp_sys_tanf(f32 x);
SP_API f32          sp_sys_acosf(f32 x);
SP_API sp_color_t   sp_color_rgb_255(u8 r, u8 g, u8 b);
SP_API sp_color_t   sp_color_rgb_to_hsv(sp_color_t color);
SP_API sp_color_t   sp_color_hsv_to_rgb(sp_color_t color);
SP_API f32          sp_inv_sqrtf(f32 value);
SP_API f32          sp_lerp(f32 a, f32 t, f32 b);
SP_API f32          sp_clamp(f32 low, f32 value, f32 high);
SP_API sp_vec2_t    sp_vec2(f32 x, f32 y);
SP_API sp_vec3_t    sp_vec3(f32 x, f32 y, f32 z);
SP_API sp_vec4_t    sp_vec4(f32 x, f32 y, f32 z, f32 w);
SP_API sp_vec4_t    sp_vec4V(sp_vec3_t xyz, f32 w);
SP_API sp_vec2_t    sp_vec2_add(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec3_t    sp_vec3_add(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec4_t    sp_vec4_add(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec2_t    sp_vec2_sub(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec3_t    sp_vec3_sub(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec4_t    sp_vec4_sub(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec2_t    sp_vec2_mul(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec2_t    sp_vec2_scale(sp_vec2_t left, f32 right);
SP_API sp_vec3_t    sp_vec3_mul(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec3_t    sp_vec3_scale(sp_vec3_t left, f32 right);
SP_API sp_vec4_t    sp_vec4_mul(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec4_t    sp_vec4_scale(sp_vec4_t left, f32 right);
SP_API sp_vec2_t    sp_vec2_div(sp_vec2_t left, sp_vec2_t right);
SP_API sp_vec2_t    sp_vec2_divf(sp_vec2_t left, f32 right);
SP_API sp_vec3_t    sp_vec3_div(sp_vec3_t left, sp_vec3_t right);
SP_API sp_vec3_t    sp_vec3_divf(sp_vec3_t left, f32 right);
SP_API sp_vec4_t    sp_vec4_div(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec4_t    sp_vec4_divf(sp_vec4_t left, f32 right);
SP_API bool         sp_vec2_eq(sp_vec2_t left, sp_vec2_t right);
SP_API bool         sp_vec3_eq(sp_vec3_t left, sp_vec3_t right);
SP_API bool         sp_vec4_eq(sp_vec4_t left, sp_vec4_t right);
SP_API f32          sp_vec2_dot(sp_vec2_t left, sp_vec2_t right);
SP_API f32          sp_vec3_dot(sp_vec3_t left, sp_vec3_t right);
SP_API f32          sp_vec4_dot(sp_vec4_t left, sp_vec4_t right);
SP_API sp_vec3_t    sp_vec3_cross(sp_vec3_t left, sp_vec3_t right);
SP_API f32          sp_vec2_len_sqr(sp_vec2_t v);
SP_API f32          sp_vec3_len_sqr(sp_vec3_t v);
SP_API f32          sp_vec4_len_sqr(sp_vec4_t v);
SP_API f32          sp_vec2_len(sp_vec2_t v);
SP_API f32          sp_vec3_len(sp_vec3_t v);
SP_API f32          sp_vec4_len(sp_vec4_t v);
SP_API sp_vec2_t    sp_vec2_norm(sp_vec2_t v);
SP_API sp_vec3_t    sp_vec3_norm(sp_vec3_t v);
SP_API sp_vec4_t    sp_vec4_norm(sp_vec4_t v);
SP_API sp_vec2_t    sp_vec2_lerp(sp_vec2_t a, f32 t, sp_vec2_t b);
SP_API sp_vec3_t    sp_vec3_lerp(sp_vec3_t a, f32 t, sp_vec3_t b);
SP_API sp_vec4_t    sp_vec4_lerp(sp_vec4_t a, f32 t, sp_vec4_t b);
SP_API sp_interp_t  sp_interp_build(f32 start, f32 target, f32 time);
SP_API bool         sp_interp_update(sp_interp_t* interp, f32 dt);
SP_API f32          sp_interp_lerp(sp_interp_t* interp);
SP_API f32          sp_interp_ease_in(sp_interp_t* interp);
SP_API f32          sp_interp_ease_out(sp_interp_t* interp);
SP_API f32          sp_interp_ease_inout(sp_interp_t* interp);
SP_API f32          sp_interp_ease_inout_bounce(sp_interp_t* interp);
SP_API f32          sp_interp_exponential(sp_interp_t* interp);
SP_API f32          sp_interp_parabolic(sp_interp_t* interp);
#endif // SP_MATH_H

#ifdef SP_MATH_IMPLEMENTATION
f32 sp_sys_sqrtf(f32 x) {
  if (x < 0) return 0;
  if (x == 0) return 0;
  f32 guess = x / 2.0f;
  for (int i = 0; i < 10; i++) {
    guess = (guess + x / guess) / 2.0f;
  }
  return guess;
}

f32 sp_sys_expf(f32 x) {
  f32 result = 1.0f;
  f32 term = 1.0f;
  for (int i = 0; i < 20; i++) {
    term *= x / (f32)(i + 1);
    result += term;
  }
  return result;
}

f32 sp_sys_sinf(f32 x) {
  while (x > 3.14159265f) x -= 6.28318530f;
  while (x < -3.14159265f) x += 6.28318530f;
  f32 x2 = x * x;
  return x * (1.0f - x2/6.0f * (1.0f - x2/20.0f * (1.0f - x2/42.0f)));
}

f32 sp_sys_cosf(f32 x) {
  while (x > 3.14159265f) x -= 6.28318530f;
  while (x < -3.14159265f) x += 6.28318530f;
  f32 x2 = x * x;
  return 1.0f - x2/2.0f * (1.0f - x2/12.0f * (1.0f - x2/30.0f));
}

f32 sp_sys_tanf(f32 x) {
  f32 c = sp_sys_cosf(x);
  if (c == 0) return 0;
  return sp_sys_sinf(x) / c;
}

f32 sp_sys_acosf(f32 x) {
  if (x < -1.0f) x = -1.0f;
  if (x > 1.0f) x = 1.0f;
  return 1.5707963f - x - x*x*x/6.0f - 3.0f*x*x*x*x*x/40.0f;
}
sp_color_t sp_color_rgb_255(u8 r, u8 g, u8 b) {
  return (sp_color_t) SP_COLOR_RGB(r, g, b);
}

sp_color_t sp_color_rgb_to_hsv(sp_color_t color) {
  f32 r = color.r;
  f32 g = color.g;
  f32 b = color.b;

  f32 max = SP_MAX(r, SP_MAX(g, b));
  f32 min = SP_MIN(r, SP_MIN(g, b));
  f32 delta = max - min;

  f32 h = 0.0f;
  f32 s = 0.0f;
  f32 v = max;

  if (delta > 1e-6f && max > 1e-6f) {
    s = delta / max;

    if (max - r < 1e-6f) {
      h = (g - b) / delta;
    } else if (max - g < 1e-6f) {
      h = 2.0f + (b - r) / delta;
    } else {
      h = 4.0f + (r - g) / delta;
    }

    h = h / 6.0f;
    if (h < 0.0f) {
      h += 1.0f;
    }
  }

  return (sp_color_t){
    .h = h * 360.0f,
    .s = s * 100.0f,
    .v = v * 100.0f,
    .a = color.a
  };
}

sp_color_t sp_color_hsv_to_rgb(sp_color_t color) {
  f32 h = color.h / 360.0f;
  f32 s = color.s / 100.0f;
  f32 v = color.v / 100.0f;

  f32 r = v;
  f32 g = v;
  f32 b = v;

  if (s > 1e-6f) {
    f32 h6 = h * 6.0f;
    if (h6 >= 6.0f) {
      h6 = 0.0f;
    }
    s32 sector = (s32)h6;
    f32 f = h6 - (f32)sector;

    f32 p = v * (1.0f - s);
    f32 q = v * (1.0f - s * f);
    f32 t = v * (1.0f - s * (1.0f - f));

    switch (sector) {
      case 0: { r = v; g = t; b = p; break; }
      case 1: { r = q; g = v; b = p; break; }
      case 2: { r = p; g = v; b = t; break; }
      case 3: { r = p; g = q; b = v; break; }
      case 4: { r = t; g = p; b = v; break; }
      case 5: { r = v; g = p; b = q; break; }
    }
  }

  return (sp_color_t){
    .r = r,
    .g = g,
    .b = b,
    .a = color.a
  };
}

f32 sp_inv_sqrtf(f32 value) {
  return 1.0f / sp_sqrtf(value);
}

f32 sp_lerp(f32 a, f32 t, f32 b) {
  return (1.0f - t) * a + t * b;
}

f32 sp_clamp(f32 low, f32 value, f32 high) {
  f32 result = value;
  if (result < low) {
    result = low;
  }
  if (result > high) {
    result = high;
  }
  return result;
}

sp_vec2_t sp_vec2(f32 x, f32 y) {
  return (sp_vec2_t){ .x = x, .y = y };
}

sp_vec3_t sp_vec3(f32 x, f32 y, f32 z) {
  return (sp_vec3_t){ .x = x, .y = y, .z = z };
}

sp_vec4_t sp_vec4(f32 x, f32 y, f32 z, f32 w) {
  return (sp_vec4_t){ .x = x, .y = y, .z = z, .w = w };
}

sp_vec4_t sp_vec4V(sp_vec3_t xyz, f32 w) {
  return (sp_vec4_t){ .xyz = xyz, .w = w };
}

sp_vec2_t sp_vec2_add(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x + right.x, .y = left.y + right.y };
}

sp_vec3_t sp_vec3_add(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x + right.x, .y = left.y + right.y, .z = left.z + right.z };
}

sp_vec4_t sp_vec4_add(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x + right.x, .y = left.y + right.y, .z = left.z + right.z, .w = left.w + right.w };
}

sp_vec2_t sp_vec2_sub(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x - right.x, .y = left.y - right.y };
}

sp_vec3_t sp_vec3_sub(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x - right.x, .y = left.y - right.y, .z = left.z - right.z };
}

sp_vec4_t sp_vec4_sub(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x - right.x, .y = left.y - right.y, .z = left.z - right.z, .w = left.w - right.w };
}

sp_vec2_t sp_vec2_mul(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x * right.x, .y = left.y * right.y };
}

sp_vec2_t sp_vec2_scale(sp_vec2_t left, f32 right) {
  return (sp_vec2_t){ .x = left.x * right, .y = left.y * right };
}

sp_vec3_t sp_vec3_mul(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x * right.x, .y = left.y * right.y, .z = left.z * right.z };
}

sp_vec3_t sp_vec3_scale(sp_vec3_t left, f32 right) {
  return (sp_vec3_t){ .x = left.x * right, .y = left.y * right, .z = left.z * right };
}

sp_vec4_t sp_vec4_mul(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x * right.x, .y = left.y * right.y, .z = left.z * right.z, .w = left.w * right.w };
}

sp_vec4_t sp_vec4_scale(sp_vec4_t left, f32 right) {
  return (sp_vec4_t){ .x = left.x * right, .y = left.y * right, .z = left.z * right, .w = left.w * right };
}

sp_vec2_t sp_vec2_div(sp_vec2_t left, sp_vec2_t right) {
  return (sp_vec2_t){ .x = left.x / right.x, .y = left.y / right.y };
}

sp_vec2_t sp_vec2_divf(sp_vec2_t left, f32 right) {
  return (sp_vec2_t){ .x = left.x / right, .y = left.y / right };
}

sp_vec3_t sp_vec3_div(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){ .x = left.x / right.x, .y = left.y / right.y, .z = left.z / right.z };
}

sp_vec3_t sp_vec3_divf(sp_vec3_t left, f32 right) {
  return (sp_vec3_t){ .x = left.x / right, .y = left.y / right, .z = left.z / right };
}

sp_vec4_t sp_vec4_div(sp_vec4_t left, sp_vec4_t right) {
  return (sp_vec4_t){ .x = left.x / right.x, .y = left.y / right.y, .z = left.z / right.z, .w = left.w / right.w };
}

sp_vec4_t sp_vec4_divf(sp_vec4_t left, f32 right) {
  return (sp_vec4_t){ .x = left.x / right, .y = left.y / right, .z = left.z / right, .w = left.w / right };
}

bool sp_vec2_eq(sp_vec2_t left, sp_vec2_t right) {
  return left.x == right.x && left.y == right.y;
}

bool sp_vec3_eq(sp_vec3_t left, sp_vec3_t right) {
  return left.x == right.x && left.y == right.y && left.z == right.z;
}

bool sp_vec4_eq(sp_vec4_t left, sp_vec4_t right) {
  return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

f32 sp_vec2_dot(sp_vec2_t left, sp_vec2_t right) {
  return (left.x * right.x) + (left.y * right.y);
}

f32 sp_vec3_dot(sp_vec3_t left, sp_vec3_t right) {
  return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
}

f32 sp_vec4_dot(sp_vec4_t left, sp_vec4_t right) {
  return ((left.x * right.x) + (left.z * right.z)) + ((left.y * right.y) + (left.w * right.w));
}

sp_vec3_t sp_vec3_cross(sp_vec3_t left, sp_vec3_t right) {
  return (sp_vec3_t){
    .x = (left.y * right.z) - (left.z * right.y),
    .y = (left.z * right.x) - (left.x * right.z),
    .z = (left.x * right.y) - (left.y * right.x)
  };
}

f32 sp_vec2_len_sqr(sp_vec2_t v) {
  return sp_vec2_dot(v, v);
}

f32 sp_vec3_len_sqr(sp_vec3_t v) {
  return sp_vec3_dot(v, v);
}

f32 sp_vec4_len_sqr(sp_vec4_t v) {
  return sp_vec4_dot(v, v);
}

f32 sp_vec2_len(sp_vec2_t v) {
  return sp_sqrtf(sp_vec2_len_sqr(v));
}

f32 sp_vec3_len(sp_vec3_t v) {
  return sp_sqrtf(sp_vec3_len_sqr(v));
}

f32 sp_vec4_len(sp_vec4_t v) {
  return sp_sqrtf(sp_vec4_len_sqr(v));
}

sp_vec2_t sp_vec2_norm(sp_vec2_t v) {
  return sp_vec2_scale(v, sp_inv_sqrtf(sp_vec2_dot(v, v)));
}

sp_vec3_t sp_vec3_norm(sp_vec3_t v) {
  return sp_vec3_scale(v, sp_inv_sqrtf(sp_vec3_dot(v, v)));
}

sp_vec4_t sp_vec4_norm(sp_vec4_t v) {
  return sp_vec4_scale(v, sp_inv_sqrtf(sp_vec4_dot(v, v)));
}

sp_vec2_t sp_vec2_lerp(sp_vec2_t a, f32 t, sp_vec2_t b) {
  return sp_vec2_add(sp_vec2_scale(a, 1.0f - t), sp_vec2_scale(b, t));
}

sp_vec3_t sp_vec3_lerp(sp_vec3_t a, f32 t, sp_vec3_t b) {
  return sp_vec3_add(sp_vec3_scale(a, 1.0f - t), sp_vec3_scale(b, t));
}

sp_vec4_t sp_vec4_lerp(sp_vec4_t a, f32 t, sp_vec4_t b) {
  return sp_vec4_add(sp_vec4_scale(a, 1.0f - t), sp_vec4_scale(b, t));
}

sp_interp_t sp_interp_build(f32 start, f32 target, f32 time) {
  return (sp_interp_t){ .start = start, .delta = target - start, .t = 0, .time_scale = 1.0f / time };
}

bool sp_interp_update(sp_interp_t* interp, f32 dt) {
  interp->t += dt * interp->time_scale;
  if (interp->t > 1.0f) { interp->t = 1.0f; }
  return interp->t >= 1.0f;
}

f32 sp_interp_lerp(sp_interp_t* interp) {
  return interp->start + interp->delta * interp->t;
}

f32 sp_interp_ease_in(sp_interp_t* interp) {
  f32 eased = interp->t * interp->t;
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_out(sp_interp_t* interp) {
  f32 eased = 1.0f - (1.0f - interp->t) * (1.0f - interp->t);
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_inout(sp_interp_t* interp) {
  f32 eased;
  if (interp->t < 0.5f) {
    eased = 2.0f * interp->t * interp->t;
  } else {
    eased = 1.0f - (-2.0f * interp->t + 2.0f) * (-2.0f * interp->t + 2.0f) / 2.0f;
  }
  return interp->start + interp->delta * eased;
}

f32 sp_interp_ease_inout_bounce(sp_interp_t* interp) {
  f32 c1 = 1.70158f;
  f32 c2 = c1 * 1.525f;
  f32 eased;
  if (interp->t < 0.5f) {
    f32 x = 2.0f * interp->t;
    eased = 0.5f * (x * x * ((c2 + 1.0f) * x - c2));
  } else {
    f32 x = 2.0f * interp->t - 2.0f;
    eased = 0.5f * (x * x * ((c2 + 1.0f) * x + c2) + 2.0f);
  }
  return interp->start + interp->delta * eased;
}

f32 sp_interp_exponential(sp_interp_t* interp) {
  f32 k = 5.0f;
  f32 e_k = 148.413159f;
  f32 eased = (sp_expf(k * interp->t) - 1.0f) / (e_k - 1.0f);
  return interp->start + interp->delta * eased;
}

f32 sp_interp_parabolic(sp_interp_t* interp) {
  f32 x = 2.0f * interp->t - 1.0f;
  f32 eased = 1.0f - x * x;
  return interp->start + interp->delta * eased;
}

#endif // SP_MATH_IMPLEMENTATION
