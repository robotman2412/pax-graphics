/*
	MIT License

	Copyright (c) 2021 Julian Scheffers

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

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "pax_types.h"

/* ============ DEBUG ============ */

extern pax_err_t pax_last_error;

// Describe error.
char      *pax_desc_err       (pax_err_t error);

/* ============ BUFFER =========== */

// Create a new buffer.
// If mem is NULL, a new area is allocated.
void      pax_buf_init   (pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type);
// Destroy the buffer, freeing its memory.
void      pax_buf_destroy(pax_buf_t *buf);

// Check whether the buffer is dirty.
bool      pax_is_dirty   (pax_buf_t *buf);
// Mark the entire buffer as clean.
void      pax_mark_clean (pax_buf_t *buf);
// Mark the entire buffer as dirty.
void      pax_mark_dirty0(pax_buf_t *buf);
// Mark a single point as dirty.
void      pax_mark_dirty1(pax_buf_t *buf, int x, int y);
// Mark a rectangle as dirty.
void      pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height);

/* ============ COLORS =========== */

// Converts HSV to ARGB.
pax_col_t pax_col_hsv    (uint8_t h, uint8_t s, uint8_t v);
// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp   (uint8_t part, pax_col_t from, pax_col_t to);

/* ============ MATRIX =========== */

/* ======== DRAWING: PIXEL ======= */

// Set a pixel.
void      pax_set_pixel    (pax_buf_t *buf, pax_col_t color, int x, int y);
// Get a pixel.
pax_col_t pax_get_pixel    (pax_buf_t *buf, int x, int y);

/* ========= DRAWING: 2D ========= */

/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void      pax_background   (pax_buf_t *buf, pax_col_t color);

// Draw a rectangle, ignoring matrix transform.
void      pax_simple_rect  (pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);

// Draw a line, ignoring matrix transform.
void      pax_simple_line  (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1);

// Draw a triangle, ignoring matrix transform.
void      pax_simple_tri   (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);

// Draw a arc, ignoring matrix transform.
// Angles in radians.
void      pax_simple_arc   (pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1);
// Draw a circle, ignoring matrix transform.
void      pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r);

#ifdef __cplusplus
}
#endif //__cplusplus
