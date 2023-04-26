/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef PAX_GFX_H
#define PAX_GFX_H

#include "pax_types.h"
#include "pax_fonts.h"
#include "pax_text.h"
#include "pax_shapes.h"
#include "pax_shaders.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* ============ DEBUG ============ */

// The last error reported.
extern pax_err_t pax_last_error;

// Describe error.
const char *pax_desc_err           (pax_err_t error);



/* ===== MULTI-CORE RENDERING ==== */

// If multi-core rendering is enabled, wait for the other core.
void       pax_join                ();
// Enable multi-core rendering.
// You can specify the core number to use, though this may be irrelevant on some platforms.
void       pax_enable_multicore    (int core);
// Disable multi-core rendering.
void       pax_disable_multicore   ();



/* ============ BUFFER =========== */

extern bool pax_enable_shape_aa;

// Get the bits per pixel for the given buffer type.
#define PAX_GET_BPP(type)         ((type) & 0xff)
// Reflects whether the buffer type is greyscale.
#define PAX_IS_GREY(type)         (((type) & 0xf0000000) == 0x10000000)
// Reflects whether the buffer type is paletted.
#define PAX_IS_PALETTE(type)      (((type) & 0xf0000000) == 0x20000000)
// Reflects whether the buffer type is color.
#define PAX_IS_COLOR(type)        (((type) & 0xf0000000) == 0x00000000)
// Whether the buffer type potentially has alpha.
#define PAX_IS_ALPHA(type)        (((type) & 0x00f00000) || PAX_IS_PALETTE(type))

// Determine how much capacity a certain buffer initialisation needs.
#define   PAX_BUF_CALC_SIZE(width, height, type) \
		((PAX_GET_BPP(type) * (width) * (height) + 7) >> 3)
// Create a new buffer.
// If mem is NULL, a new area is allocated.
void      pax_buf_init            (pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type);
// Enable/disable the reversing of endianness for `buf`.
// Some displays might require a feature like this one.
void      pax_buf_reversed        (pax_buf_t *buf, bool reversed_endianness);
// Destroy the buffer, freeing its memory.
void      pax_buf_destroy         (pax_buf_t *buf);
// WARNING: This is a beta feature and it does not work!
// 
// Convert the buffer to the given new format.
// If dest is NULL or equal to src, src will be converted.
void      pax_buf_convert         (pax_buf_t *dst, pax_buf_t *src, pax_buf_type_t type);

// Retrieve the width of the buffer.
int       pax_buf_get_width       (const pax_buf_t *buf);
// Retrieve the height of the buffer.
int       pax_buf_get_height      (const pax_buf_t *buf);
// Retrieve the width of the buffer.
float     pax_buf_get_widthf      (const pax_buf_t *buf);
// Retrieve the height of the buffer.
float     pax_buf_get_heightf     (const pax_buf_t *buf);
// Retrieve the type of the buffer.
pax_buf_type_t pax_buf_get_type   (const pax_buf_t *buf);

// Get a const pointer to the image data.
// See <../docs/pixelformat.md> for the format.
const void *pax_buf_get_pixels    (const pax_buf_t *buf);
// Get a non-const pointer to the image data.
// See <../docs/pixelformat.md> for the format.
void       *pax_buf_get_pixels_rw (pax_buf_t *buf);
// Get the byte size of the image data.
size_t      pax_buf_get_size      (const pax_buf_t *buf);

// Set rotation of the buffer.
// 0 is not rotated, each unit is one quarter turn counter-clockwise.
void      pax_buf_set_rotation    (pax_buf_t *buf, int rotation);
// Get rotation of the buffer.
// 0 is not rotated, each unit is one quarter turn counter-clockwise.
int       pax_buf_get_rotation    (const pax_buf_t *buf);
// Scroll the buffer, filling with a placeholder color.
void      pax_buf_scroll          (pax_buf_t *buf, pax_col_t placeholder, int x, int y);

// Clip the buffer to the desired rectangle.
void      pax_clip                (pax_buf_t *buf, int x, int y, int width, int height);
// Get the current clip rectangle.
pax_recti pax_get_clip            (const pax_buf_t *buf);
// Clip the buffer to it's full size.
void      pax_noclip              (pax_buf_t *buf);

// Check whether the buffer is dirty.
bool      pax_is_dirty            (const pax_buf_t *buf);
// Get a copy of the dirty rectangle.
pax_recti pax_get_dirty           (const pax_buf_t *buf);
// Mark the entire buffer as clean.
void      pax_mark_clean          (pax_buf_t *buf);
// Mark the entire buffer as dirty.
void      pax_mark_dirty0         (pax_buf_t *buf);
// Mark a single point as dirty.
void      pax_mark_dirty1         (pax_buf_t *buf, int x, int y);
// Mark a rectangle as dirty.
void      pax_mark_dirty2         (pax_buf_t *buf, int x, int y, int width, int height);



/* ======= ROTATION HELPERS ====== */

// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2f pax_rotate_ccw1_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		vec.y,
		buf->height - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_rotate_ccw2_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width  - vec.x,
		buf->height - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2f pax_rotate_ccw3_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	return (pax_vec2f) {
		buf->width - vec.y,
		vec.x,
	};
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_vec2f pax_rotate_det_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 1: return pax_rotate_ccw1_vec2f(buf, vec);
		case 2: return pax_rotate_ccw2_vec2f(buf, vec);
		case 3: return pax_rotate_ccw3_vec2f(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_vec2f pax_unrotate_det_vec2f(const pax_buf_t *buf, pax_vec2f vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 3: return pax_rotate_ccw1_vec2f(buf, vec);
		case 2: return pax_rotate_ccw2_vec2f(buf, vec);
		case 1: return pax_rotate_ccw3_vec2f(buf, vec);
	}
	#else
	return vec;
	#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_rectf pax_rotate_ccw1_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		vec.y,
		buf->height - vec.x,
		vec.h,
		-vec.w,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_rotate_ccw2_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width  - vec.x,
		buf->height - vec.y,
		-vec.w,
		-vec.h,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_rectf pax_rotate_ccw3_rectf(const pax_buf_t *buf, pax_rectf vec) {
	return (pax_rectf) {
		buf->width - vec.y,
		vec.x,
		-vec.h,
		vec.w,
	};
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_rectf pax_rotate_det_rectf(const pax_buf_t *buf, pax_rectf vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 1: return pax_rotate_ccw1_rectf(buf, vec);
		case 2: return pax_rotate_ccw2_rectf(buf, vec);
		case 3: return pax_rotate_ccw3_rectf(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_rectf pax_unrotate_det_rectf(const pax_buf_t *buf, pax_rectf vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 3: return pax_rotate_ccw1_rectf(buf, vec);
		case 2: return pax_rotate_ccw2_rectf(buf, vec);
		case 1: return pax_rotate_ccw3_rectf(buf, vec);
	}
	#else
	return vec;
	#endif
}


// Transforms the co-ordinates as 1x counter-clockwise rotation.
static inline pax_vec2i pax_rotate_ccw1_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		vec.y,
		buf->height - 1 - vec.x,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_rotate_ccw2_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width  - 1 - vec.x,
		buf->height - 1 - vec.y,
	};
}

// Transforms the co-ordinates as 2x counter-clockwise rotation.
static inline pax_vec2i pax_rotate_ccw3_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	return (pax_vec2i) {
		buf->width - 1 - vec.y,
		vec.x,
	};
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_vec2i pax_rotate_det_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 1: return pax_rotate_ccw1_vec2i(buf, vec);
		case 2: return pax_rotate_ccw2_vec2i(buf, vec);
		case 3: return pax_rotate_ccw3_vec2i(buf, vec);
	}
	#else
	return vec;
	#endif
}

// Detects rotations and transforms co-ordinates accordingly.
static inline pax_vec2i pax_unrotate_det_vec2i(const pax_buf_t *buf, pax_vec2i vec) {
	#if PAX_COMPILE_ROTATE
	switch (buf->rotation) {
		default:
		case 0: return vec;
		case 3: return pax_rotate_ccw1_vec2i(buf, vec);
		case 2: return pax_rotate_ccw2_vec2i(buf, vec);
		case 1: return pax_rotate_ccw3_vec2i(buf, vec);
	}
	#else
	return vec;
	#endif
}



/* ============ COLORS =========== */

// Multiplicatively decreases alpha based on a float.
static inline pax_col_t pax_col_reduce_alpha(pax_col_t in, float coeff) {
	return ((pax_col_t) (((in & 0xff000000) * coeff)) & 0xff000000) | (in & 0x00ffffff);
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
pax_col_t pax_col_hsv             (uint8_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255.
pax_col_t pax_col_ahsv            (uint8_t a, uint8_t h, uint8_t s, uint8_t v);
// Converts HSV to RGB, ranges are 0-359, 0-99, 0-99.
pax_col_t pax_col_hsv_alt         (uint16_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255, 0-359, 0-99, 0-99.
pax_col_t pax_col_ahsv_alt        (uint8_t a, uint16_t h, uint8_t s, uint8_t v);

// Converts ARGB into AHSV, ranges are 0-255.
void pax_undo_ahsv    (pax_col_t in, uint8_t *a, uint8_t *h, uint8_t *s, uint8_t *v);
// Converts RGB into HSV, ranges are 0-255.
void pax_undo_hsv     (pax_col_t in, uint8_t *h, uint8_t *s, uint8_t *v);
// Converts ARGB into AHSV, ranges are 0-255, 0-359, 0-99, 0-99.
void pax_undo_ahsv_alt(pax_col_t in, uint8_t *a, uint16_t *h, uint8_t *s, uint8_t *v);
// Converts RGB into HSV, ranges are 0-359, 0-99, 0-99.
void pax_undo_hsv_alt (pax_col_t in, uint16_t *h, uint8_t *s, uint8_t *v);

// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp            (uint8_t part, pax_col_t from, pax_col_t to);
// Merges the two colors, based on alpha.
pax_col_t pax_col_merge           (pax_col_t base, pax_col_t top);
// Tints the color, commonly used for textures.
pax_col_t pax_col_tint            (pax_col_t col, pax_col_t tint);



/* ============ MATRIX =========== */

// Apply the given matrix to the stack.
void        pax_apply_2d          (pax_buf_t *buf, matrix_2d_t a);
// Push the current matrix up the stack.
void        pax_push_2d           (pax_buf_t *buf);
// Pop the top matrix off the stack.
void        pax_pop_2d            (pax_buf_t *buf);
// Reset the matrix stack.
// If full is true, the entire stack gets cleared instead of just the top.
void        pax_reset_2d          (pax_buf_t *buf, bool full);



/* ======== DRAWING: PIXEL ======= */

// Set a pixel, merging with alpha.
void        pax_merge_pixel         (pax_buf_t *buf, pax_col_t color, int x, int y);
// Set a pixel.
void        pax_set_pixel           (pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel (does palette lookup if applicable).
pax_col_t   pax_get_pixel           (const pax_buf_t *buf, int x, int y);
// Set a pixel without color conversion.
void        pax_set_pixel_raw       (pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel without color conversion.
pax_col_t   pax_get_pixel_raw       (const pax_buf_t *buf, int x, int y);



/* ========= DRAWING: 2D ========= */

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void        pax_shade_rect          (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, float y, float width, float height);
// Draw a line with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0).
void        pax_shade_line          (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_linef *uvs, float x0, float y0, float x1, float y1);
// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void        pax_shade_tri           (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_trif  *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void        pax_shade_arc           (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x,  float y,  float r,  float a0, float a1);
// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void        pax_shade_circle        (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x,  float y,  float r);

// Draws an image at the image's normal size.
void        pax_draw_image          (pax_buf_t *buf, const pax_buf_t *image, float x, float y);
// Draw an image with a prespecified size.
void        pax_draw_image_sized    (pax_buf_t *buf, const pax_buf_t *image, float x, float y, float width, float height);
// Draws an image at the image's normal size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void        pax_draw_image_op       (pax_buf_t *buf, const pax_buf_t *image, float x, float y);
// Draw an image with a prespecified size.
// Assumes the image is completely opaque, any transparent parts are drawn opaque.
void        pax_draw_image_sized_op (pax_buf_t *buf, const pax_buf_t *image, float x, float y, float width, float height);
// Draw a rectangle.
void        pax_draw_rect           (pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);
// Draw a line.
void        pax_draw_line           (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);
// Draw a triangle.
void        pax_draw_tri            (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw an arc, angles in radians.
void        pax_draw_arc            (pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1);
// Draw a circle.
void        pax_draw_circle         (pax_buf_t *buf, pax_col_t color, float x,  float y,  float r);



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void        pax_background          (pax_buf_t *buf, pax_col_t color);

// Draw a rectangle, ignoring matrix transform.
void        pax_simple_rect         (pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);

// Draw a line, ignoring matrix transform.
void        pax_simple_line         (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);

// Draw a triangle, ignoring matrix transform.
void        pax_simple_tri          (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Draw na arc, ignoring matrix transform.
// Angles in radians.
void        pax_simple_arc          (pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1);
// Draw a circle, ignoring matrix transform.
void        pax_simple_circle       (pax_buf_t *buf, pax_col_t color, float x,  float y,  float r);



#ifdef __cplusplus
} // extern "C"

#include "cpp/pax_cxx.hpp"
#endif //__cplusplus

#endif //PAX_GFX_H
