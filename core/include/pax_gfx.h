
// SPDX-License-Identifier: MIT

#ifndef PAX_GFX_H
#define PAX_GFX_H

#include "pax_fonts.h"
#include "pax_orientation.h"
#include "pax_shaders.h"
#include "pax_shapes.h"
#include "pax_text.h"
#include "pax_types.h"
#include "shapes/pax_arcs.h"
#include "shapes/pax_circles.h"
#include "shapes/pax_lines.h"
#include "shapes/pax_misc.h"
#include "shapes/pax_rects.h"
#include "shapes/pax_tris.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ DEBUG ============ */

// Get the last error reported on this thread.
pax_err_t   pax_get_err();
// Describe error.
char const *pax_desc_err(pax_err_t error);



/* ===== RENDER ENGINE HOUSEKEEPING ==== */

// Wait for all pending drawing operations to finish.
void pax_join();
// Set the render engine to synchronous software renderer.
void pax_set_render_engine_default();
// Enable the asynchronous renderer.
// If `multithreaded` is `true` and `CONFIG_PAX_COMPILE_ASYNC_RENDERER` is set to `2`,
// This will use two threads for rendering instead of just one.
void pax_set_renderer_async(bool multithreaded);


/* ============ BUFFER =========== */

// Get buffer type info.
pax_buf_type_info_t pax_buf_type_info(pax_buf_type_t type) __attribute__((const));

// Determine how much capacity a certain buffer initialisation needs.
static inline size_t pax_buf_calc_size_dynamic(int width, int height, pax_buf_type_t type) {
    pax_buf_type_info_t info = pax_buf_type_info(type);
    return (info.bpp * width * height + 7) >> 3;
}

#define PAX__STATIC_BUF_TYPE_HELPER(bpp, palette, grey, a, r, g, b, name)                                              \
    enum {                                                                                                             \
        name##__BPP     = bpp,                                                                                         \
        name##__PALETTE = palette,                                                                                     \
        name##__GREY    = grey,                                                                                        \
        name##__A       = a,                                                                                           \
        name##__R       = r,                                                                                           \
        name##__G       = g,                                                                                           \
        name##__B       = b,                                                                                           \
    };
#define PAX_DEF_BUF_TYPE_PAL(bpp, name)              PAX__STATIC_BUF_TYPE_HELPER(bpp, 1, 0, 0, 0, 0, 0, name)
#define PAX_DEF_BUF_TYPE_GREY(bpp, name)             PAX__STATIC_BUF_TYPE_HELPER(bpp, 0, 1, 0, 0, 0, 0, name)
#define PAX_DEF_BUF_TYPE_ARGB(bpp, a, r, g, b, name) PAX__STATIC_BUF_TYPE_HELPER(bpp, 0, 0, a, r, g, b, name)
#include "helpers/pax_buf_type.inc"
#undef PAX__STATIC_BUF_TYPE_HELPER

// Get the bits per pixel for the given buffer type.
// Assumes `type` is literally the type name; use `pax_buf_type_info` for dynamic info.
#define PAX_GET_BPP(type)    type##__BPP
// Reflects whether the buffer type is paletted.
// Assumes `type` is literally the type name; use `pax_buf_type_info` for dynamic info.
#define PAX_IS_PALETTE(type) type##__PALETTE
// Reflects whether the buffer type is greyscale.
// Assumes `type` is literally the type name; use `pax_buf_type_info` for dynamic info.
#define PAX_IS_GREY(type)    type##__GREY
// Reflects whether the buffer type is color.
// Assumes `type` is literally the type name; use `pax_buf_type_info` for dynamic info.
#define PAX_IS_COLOR(type)   type##__R
// Whether the buffer type potentially has alpha.
// Assumes `type` is literally the type name; use `pax_buf_type_info` for dynamic info.
#define PAX_IS_ALPHA(type)   (type##__A || type##__PALETTE)

// Determine how much capacity a certain buffer initialisation needs.
// Assumes `type` is literally the type name; use `pax_buf_calc_size_dynamic` for dynamic info.
#define PAX_BUF_CALC_SIZE(width, height, type) ((PAX_GET_BPP(type) * (width) * (height) + 7) >> 3)

// Initialize a buffer where the `pax_buf_t` struct is user-managed.
// If `mem` is `NULL`, a new area is allocated.
bool pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type);
// De-initialize a buffer initialized by `pax_buf_init`.
void pax_buf_destroy(pax_buf_t *buf);

// Set the palette for buffers with palette types.
// Creates an internal copy of the palette.
void             pax_buf_set_palette(pax_buf_t *buf, pax_col_t const *palette, size_t palette_len);
// Set the palette for buffers with palette types.
// Does not create internal copy of the palette.
void             pax_buf_set_palette_rom(pax_buf_t *buf, pax_col_t const *palette, size_t palette_len);
// Get the palette for buffers with palette types.
pax_col_t const *pax_buf_get_palette(pax_buf_t *buf, size_t *palette_len);
// Enable/disable the reversing of endianness for `buf`.
// Some displays might require a feature like this one.
void             pax_buf_reversed(pax_buf_t *buf, bool reversed_endianness);

// Retrieve the width of the buffer.
int            pax_buf_get_width(pax_buf_t const *buf);
// Retrieve the height of the buffer.
int            pax_buf_get_height(pax_buf_t const *buf);
// Retrieve dimensions of the buffer.
pax_vec2i      pax_buf_get_dims(pax_buf_t const *buf);
// Retrieve the width of the buffer without applying orientation.
int            pax_buf_get_width_raw(pax_buf_t const *buf);
// Retrieve the height of the buffer without applying orientation.
int            pax_buf_get_height_raw(pax_buf_t const *buf);
// Retrieve dimensions of the buffer without applying orientation.
pax_vec2i      pax_buf_get_dims_raw(pax_buf_t const *buf);
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

// Clip the buffer to the desired rectangle.
void               pax_clip(pax_buf_t *buf, int x, int y, int width, int height);
// Clip the buffer to the desired rectangle.
static inline void pax_set_clip(pax_buf_t *buf, pax_recti rect) {
    pax_clip(buf, rect.x, rect.y, rect.w, rect.h);
}
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

// Finds the closes color in a palette.
size_t pax_closest_in_palette(pax_col_t const *palette, size_t palette_size, pax_col_t color);



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



#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PAX_GFX_H
