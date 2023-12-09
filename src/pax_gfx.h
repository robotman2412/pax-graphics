
// SPDX-License-Identifier: MIT

#ifndef PAX_GFX_H
#define PAX_GFX_H

#include "pax_fonts.h"
#include "pax_orientation.h"
#include "pax_shaders.h"
#include "pax_shapes.h"
#include "pax_text.h"
#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ DEBUG ============ */

// The last error reported.
extern pax_err_t pax_last_error;

// Describe error.
char const *pax_desc_err(pax_err_t error);



/* ===== MULTI-CORE RENDERING ==== */

// If multi-core rendering is enabled, wait for the other core.
void pax_join();
// Enable multi-core rendering.
// You can specify the core number to use, though this may be irrelevant on some platforms.
void pax_enable_multicore(int core);
// Disable multi-core rendering.
void pax_disable_multicore();



/* ============ BUFFER =========== */

// Get the bits per pixel for the given buffer type.
#define PAX_GET_BPP(type)    ((type) & 0xff)
// Reflects whether the buffer type is greyscale.
#define PAX_IS_GREY(type)    (((type) & 0xf0000000) == 0x10000000)
// Reflects whether the buffer type is paletted.
#define PAX_IS_PALETTE(type) (((type) & 0xf0000000) == 0x20000000)
// Reflects whether the buffer type is color.
#define PAX_IS_COLOR(type)   (((type) & 0xf0000000) == 0x00000000)
// Whether the buffer type potentially has alpha.
#define PAX_IS_ALPHA(type)   (((type) & 0x00f00000) || PAX_IS_PALETTE(type))

// Determine how much capacity a certain buffer initialisation needs.
#define PAX_BUF_CALC_SIZE(width, height, type) ((PAX_GET_BPP(type) * (width) * (height) + 7) >> 3)
// Create a new buffer.
// If mem is NULL, a new area is allocated.
void pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type);
// Enable/disable the reversing of endianness for `buf`.
// Some displays might require a feature like this one.
void pax_buf_reversed(pax_buf_t *buf, bool reversed_endianness);
// Destroy the buffer, freeing its memory.
void pax_buf_destroy(pax_buf_t *buf);
// WARNING: This is a beta feature and it does not work!
//
// Convert the buffer to the given new format.
// If dest is NULL or equal to src, src will be converted.
void pax_buf_convert(pax_buf_t *dst, pax_buf_t *src, pax_buf_type_t type);

// Retrieve the width of the buffer.
int            pax_buf_get_width(pax_buf_t const *buf);
// Retrieve the height of the buffer.
int            pax_buf_get_height(pax_buf_t const *buf);
// Retrieve the width of the buffer.
float          pax_buf_get_widthf(pax_buf_t const *buf);
// Retrieve the height of the buffer.
float          pax_buf_get_heightf(pax_buf_t const *buf);
// Retrieve the type of the buffer.
pax_buf_type_t pax_buf_get_type(pax_buf_t const *buf);

// Get a const pointer to the image data.
// See <../docs/pixelformat.md> for the format.
void const *pax_buf_get_pixels(pax_buf_t const *buf);
// Get a non-const pointer to the image data.
// See <../docs/pixelformat.md> for the format.
void       *pax_buf_get_pixels_rw(pax_buf_t *buf);
// Get the byte size of the image data.
size_t      pax_buf_get_size(pax_buf_t const *buf);

// Set orientation of the buffer.
void              pax_buf_set_orientation(pax_buf_t *buf, pax_orientation_t x);
// Get orientation of the buffer.
pax_orientation_t pax_buf_get_orientation(pax_buf_t const *buf);
// Scroll the buffer, filling with a placeholder color.
void              pax_buf_scroll(pax_buf_t *buf, pax_col_t placeholder, int x, int y);

// Clip the buffer to the desired rectangle.
void      pax_clip(pax_buf_t *buf, int x, int y, int width, int height);
// Get the current clip rectangle.
pax_recti pax_get_clip(pax_buf_t const *buf);
// Clip the buffer to it's full size.
void      pax_noclip(pax_buf_t *buf);

// Check whether the buffer is dirty.
bool      pax_is_dirty(pax_buf_t const *buf);
// Get a copy of the dirty rectangle.
pax_recti pax_get_dirty(pax_buf_t const *buf);
// Mark the entire buffer as clean.
void      pax_mark_clean(pax_buf_t *buf);
// Mark the entire buffer as dirty.
void      pax_mark_dirty0(pax_buf_t *buf);
// Mark a single point as dirty.
void      pax_mark_dirty1(pax_buf_t *buf, int x, int y);
// Mark a rectangle as dirty.
void      pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height);



/* ============ COLORS =========== */

// Multiplicatively decreases alpha based on a float.
static inline pax_col_t pax_col_reduce_alpha(pax_col_t in, float coeff) {
    return ((pax_col_t)(((in & 0xff000000) * coeff)) & 0xff000000) | (in & 0x00ffffff);
}
// Combines RGB.
static inline pax_col_t pax_col_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}
// Combines ARGB.
static inline pax_col_t pax_col_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Converts HSV to RGB, ranges are 0-255.
pax_col_t pax_col_hsv(uint8_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255.
pax_col_t pax_col_ahsv(uint8_t a, uint8_t h, uint8_t s, uint8_t v);
// Converts HSV to RGB, ranges are 0-359, 0-99, 0-99.
pax_col_t pax_col_hsv_alt(uint16_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255, 0-359, 0-99, 0-99.
pax_col_t pax_col_ahsv_alt(uint8_t a, uint16_t h, uint8_t s, uint8_t v);

// Converts ARGB into AHSV, ranges are 0-255.
void pax_undo_ahsv(pax_col_t in, uint8_t *a, uint8_t *h, uint8_t *s, uint8_t *v);
// Converts RGB into HSV, ranges are 0-255.
void pax_undo_hsv(pax_col_t in, uint8_t *h, uint8_t *s, uint8_t *v);
// Converts ARGB into AHSV, ranges are 0-255, 0-359, 0-99, 0-99.
void pax_undo_ahsv_alt(pax_col_t in, uint8_t *a, uint16_t *h, uint8_t *s, uint8_t *v);
// Converts RGB into HSV, ranges are 0-359, 0-99, 0-99.
void pax_undo_hsv_alt(pax_col_t in, uint16_t *h, uint8_t *s, uint8_t *v);

// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp(uint8_t part, pax_col_t from, pax_col_t to);
// Merges the two colors, based on alpha.
pax_col_t pax_col_merge(pax_col_t base, pax_col_t top);
// Tints the color, commonly used for textures.
pax_col_t pax_col_tint(pax_col_t col, pax_col_t tint);



/* ============ MATRIX =========== */

// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a);
// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf);
// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf);
// Reset the matrix stack.
// If full is true, the entire stack gets cleared instead of just the top.
void pax_reset_2d(pax_buf_t *buf, bool full);



/* ======== DRAWING: PIXEL ======= */

// Set a pixel, merging with alpha.
void      pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y);
// Set a pixel.
void      pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel (does palette lookup if applicable).
pax_col_t pax_get_pixel(pax_buf_t const *buf, int x, int y);
// Set a pixel without color conversion.
void      pax_set_pixel_raw(pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel without color conversion.
pax_col_t pax_get_pixel_raw(pax_buf_t const *buf, int x, int y);



/* ========= DRAWING: 2D ========= */

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_rect(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               width,
    float               height
);
// Draw a line with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0).
void pax_shade_line(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_linef const    *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1
);
// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void pax_shade_tri(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_trif const     *uvs,
    float               x0,
    float               y0,
    float               x1,
    float               y1,
    float               x2,
    float               y2
);
// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_arc(
    pax_buf_t          *buf,
    pax_col_t           color,
    pax_shader_t const *shader,
    pax_quadf const    *uvs,
    float               x,
    float               y,
    float               r,
    float               a0,
    float               a1
);
// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(
    pax_buf_t *buf, pax_col_t color, pax_shader_t const *shader, pax_quadf const *uvs, float x, float y, float r
);

// Draws an image at the image's normal size.
void pax_draw_image(pax_buf_t *buf, pax_buf_t const *image, float x, float y);
// Draw an image with a prespecified size.
void pax_draw_image_sized(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height);
// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y);
// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void pax_draw_image_sized_op(pax_buf_t *buf, pax_buf_t const *image, float x, float y, float width, float height);
// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);
// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);
// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw an arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);
// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r);



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color);

// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);

// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);

// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Draw na arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);
// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r);



#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GFX_H
