#define SP_MATH_IMPLEMENTATION
#include "sp/math.h"

#include "test.h"
#include "utest.h"
#include <math.h>

SP_TEST_MAIN()

UTEST(math, color_conversion) {
  typedef struct {
    sp_color_t rgb;
    sp_color_t hsv;
  } color_conversion_t;

  f32 eps = 1e-3f;

  color_conversion_t colors [] = {
    { .rgb = SP_COLOR_RGB(255, 0, 0),     .hsv = SP_COLOR_HSV(0, 100, 100) },     // red
    { .rgb = SP_COLOR_RGB(255, 187, 0),   .hsv = SP_COLOR_HSV(44, 100, 100) },    // orange
    { .rgb = SP_COLOR_RGB(0, 255, 0),     .hsv = SP_COLOR_HSV(120, 100, 100) },   // green
    { .rgb = SP_COLOR_RGB(0, 0, 255),     .hsv = SP_COLOR_HSV(240, 100, 100) },   // blue
    { .rgb = SP_COLOR_RGB(0, 0, 0),       .hsv = SP_COLOR_HSV(0, 0, 0) },         // black
    { .rgb = SP_COLOR_RGB(255, 255, 255), .hsv = SP_COLOR_HSV(0, 0, 100) },       // white
    { .rgb = SP_COLOR_RGB(128, 128, 128), .hsv = SP_COLOR_HSV(0, 0, 50.196f) },   // gray
    // saturation/hue adjustment test series (same V=83.1373%)
    { .rgb = SP_COLOR_RGB(29, 19, 212),   .hsv = SP_COLOR_HSV(243.1088f, 91.0377f, 83.1373f) },  // saturated blue
    { .rgb = SP_COLOR_RGB(190, 190, 212), .hsv = SP_COLOR_HSV(240.0f, 10.3774f, 83.1373f) },     // desaturated blue
    { .rgb = SP_COLOR_RGB(141, 212, 106), .hsv = SP_COLOR_HSV(100.1887f, 50.0f, 83.1373f) },     // hue shift to green
  };

  // rgb -> hsv
  sp_carr_for(colors, it) {
    sp_color_t hsv = sp_color_rgb_to_hsv(colors[it].rgb);
    // hue wrap-aware comparison
    f32 h_diff = fabsf(hsv.h - colors[it].hsv.h);
    h_diff = SP_MIN(h_diff, 360.0f - h_diff);
    EXPECT_LT(h_diff, eps);
    EXPECT_LT(fabsf(hsv.s - colors[it].hsv.s), eps);
    EXPECT_LT(fabsf(hsv.v - colors[it].hsv.v), eps);
  }

  // hsv -> rgb
  sp_carr_for(colors, it) {
    sp_color_t rgb = sp_color_hsv_to_rgb(colors[it].hsv);
    EXPECT_LT(fabsf(rgb.r - colors[it].rgb.r), eps);
    EXPECT_LT(fabsf(rgb.g - colors[it].rgb.g), eps);
    EXPECT_LT(fabsf(rgb.b - colors[it].rgb.b), eps);
  }

  // roundtrip: rgb -> hsv -> rgb
  sp_carr_for(colors, it) {
    sp_color_t hsv = sp_color_rgb_to_hsv(colors[it].rgb);
    sp_color_t rgb = sp_color_hsv_to_rgb(hsv);
    EXPECT_LT(fabsf(rgb.r - colors[it].rgb.r), eps);
    EXPECT_LT(fabsf(rgb.g - colors[it].rgb.g), eps);
    EXPECT_LT(fabsf(rgb.b - colors[it].rgb.b), eps);
  }

  // sector boundary tests (H = 0, 60, 120, 180, 240, 300)
  color_conversion_t boundaries[] = {
    { .rgb = { .r = 1.0f, .g = 0.5f, .b = 0.0f }, .hsv = SP_COLOR_HSV(30, 100, 100) },   // sector 0/1
    { .rgb = { .r = 0.5f, .g = 1.0f, .b = 0.0f }, .hsv = SP_COLOR_HSV(90, 100, 100) },   // sector 1/2
    { .rgb = { .r = 0.0f, .g = 1.0f, .b = 0.5f }, .hsv = SP_COLOR_HSV(150, 100, 100) },  // sector 2/3
    { .rgb = { .r = 0.0f, .g = 0.5f, .b = 1.0f }, .hsv = SP_COLOR_HSV(210, 100, 100) },  // sector 3/4
    { .rgb = { .r = 0.5f, .g = 0.0f, .b = 1.0f }, .hsv = SP_COLOR_HSV(270, 100, 100) },  // sector 4/5
    { .rgb = { .r = 1.0f, .g = 0.0f, .b = 0.5f }, .hsv = SP_COLOR_HSV(330, 100, 100) },  // sector 5/0
  };

  sp_carr_for(boundaries, it) {
    sp_color_t hsv = sp_color_rgb_to_hsv(boundaries[it].rgb);
    f32 h_diff = fabsf(hsv.h - boundaries[it].hsv.h);
    h_diff = SP_MIN(h_diff, 360.0f - h_diff);
    EXPECT_LT(h_diff, eps);
    EXPECT_LT(fabsf(hsv.s - boundaries[it].hsv.s), eps);
    EXPECT_LT(fabsf(hsv.v - boundaries[it].hsv.v), eps);

    sp_color_t rgb = sp_color_hsv_to_rgb(boundaries[it].hsv);
    EXPECT_LT(fabsf(rgb.r - boundaries[it].rgb.r), eps);
    EXPECT_LT(fabsf(rgb.g - boundaries[it].rgb.g), eps);
    EXPECT_LT(fabsf(rgb.b - boundaries[it].rgb.b), eps);
  }

  // hsv with H=360 should equal H=0
  sp_color_t rgb360 = sp_color_hsv_to_rgb((sp_color_t)SP_COLOR_HSV(360, 100, 100));
  sp_color_t rgb0 = sp_color_hsv_to_rgb((sp_color_t)SP_COLOR_HSV(0, 100, 100));
  EXPECT_LT(fabsf(rgb360.r - rgb0.r), eps);
  EXPECT_LT(fabsf(rgb360.g - rgb0.g), eps);
  EXPECT_LT(fabsf(rgb360.b - rgb0.b), eps);
}
