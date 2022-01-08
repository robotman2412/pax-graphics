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

#include "pax_gfx.h"
#include <stdio.h>
#include <esp_system.h>
#include <sdkconfig.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdint.h>
#include <malloc.h>
#include <math.h>

pax_err_t pax_last_error = PAX_OK;
static const char *TAG   = "pax";

#ifdef PAX_AUTOREPORT
#define PAX_ERROR(errno) { ESP_LOGE(TAG, "%s", pax_desc_err(errno)); pax_last_error = errno; return; }
#else
#define PAX_ERROR(errno) { pax_last_error = errno; return; }
#endif

#ifdef PAX_AUTOREPORT
#define PAX_ERROR1(errno, retval) { ESP_LOGE(TAG, "%s", pax_desc_err(errno)); pax_last_error = errno; return retval; }
#else
#define PAX_ERROR1(errno, retval) { pax_last_error = errno; return retval; }
#endif

#define PAX_SUCCESS() { pax_last_error = PAX_OK; }



/* =========== HELPERS =========== */

// Buffer sanity check.
#define PAX_BUF_CHECK() { if (!(buf) || !(buf)->buf) PAX_ERROR(PAX_ERR_NOBUF); }
// Buffer sanity check.
#define PAX_BUF_CHECK1(retval) { if (!(buf) || !(buf)->buf) PAX_ERROR1(PAX_ERR_NOBUF, retval); }

// Swap two variables.
#define PAX_SWAP(type, a, b) { type tmp = a; a = b; b = tmp; }
// Swap two points represented by floats.
#define PAX_SWAP_POINTS(x0, y0, x1, y1) { float tmp = x1; x1 = x0; x0 = tmp; tmp = y1; y1 = y0; y0 = tmp; }
// Sort two points represented by floats.
#define PAX_SORT_POINTS(x0, y0, x1, y1) { if (y1 < y0) PAX_SWAP_POINTS(x0, y0, x1, y1) }



/* ======= DRAWING HELPERS ======= */

// Internal method for unshaded triangles.
// Assumes points are sorted by Y.
static inline void pax_tri_unshaded(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	// Find the appropriate Y for y0, y1 and y2 inside the triangle.
	float y_post_0 = (int) (y0 + 0.5) + 0.5;
	float y_post_1 = (int) (y1 + 0.5) + 0.5;
	float y_pre_2  = (int) (y2 - 0.5) + 0.5;
	// And the coefficients for x0->x1, x1->x2 and x0->x2.
	float x0_x1_dx = (x1 - x0) / (y1 - y0);
	float x1_x2_dx = (x2 - x1) / (y2 - y1);
	float x0_x2_dx = (x2 - x0) / (y2 - y0);
	
	// Clip: Y axis.
	if (y_post_0 > buf->clip.y + buf->clip.h) {
		y_post_0 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_post_1 > buf->clip.y + buf->clip.h) {
		y_post_1 = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	if (y_pre_2  > buf->clip.y + buf->clip.h) {
		y_pre_2  = (int) (buf->clip.y + buf->clip.h - 0.5) + 0.5;
	}
	
	if (y_pre_2  < buf->clip.y) {
		y_pre_2  = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_1 < buf->clip.y) {
		y_post_1 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	if (y_post_0 < buf->clip.y) {
		y_post_0 = (int) (buf->clip.y + 0.5) + 0.5;
	}
	
	// Draw top half.
	// This condition is false if no one point is inside the triangle and above y1.
	if (y_post_0 < y_post_1 && y_post_0 >= y0) {
		// Find the X counterparts to the other points we found.
		float x_a = x0 + x0_x1_dx * (y_post_0 - y0);
		float x_b = x0 + x0_x2_dx * (y_post_0 - y0);
		for (int y = y_post_0; y < (int) y_post_1; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
			}
			// Clip: X axis.
			if (x_right > buf->clip.x + buf->clip.w) {
				x_right = buf->clip.x + buf->clip.w;
			}
			if (x_left < buf->clip.x) {
				x_left = buf->clip.x;
			}
			for (int x = x_left + 0.5; x < x_right; x ++) {
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, color, x, y);
			}
			// Move X.
			x_a += x0_x1_dx;
			x_b += x0_x2_dx;
		}
	}
	// Draw bottom half.
	// This condition might be confusing, but it's false if no point at all is inside the triangle.
	if (y_post_0 <= y_pre_2 && y_post_1 >= y1 && y_pre_2 <= y2) {
		// Find the X counterparts to the other points we found.
		float x_a = x1 + x1_x2_dx * (y_post_1 - y1);
		float x_b = x0 + x0_x2_dx * (y_post_1 - y0);
		for (int y = y_post_1; y <= (int) y_pre_2; y++) {
			// Plot the horizontal line.
			float x_left, x_right;
			if (x_a < x_b) {
				x_left  = x_a;
				x_right = x_b;
			} else {
				x_left  = x_b;
				x_right = x_a;
			}
			// Clip: X axis.
			if (x_right > buf->clip.x + buf->clip.w) {
				x_right = buf->clip.x + buf->clip.w;
			}
			if (x_left < buf->clip.x) {
				x_left = buf->clip.x;
			}
			for (int x = x_left + 0.5; x < x_right; x ++) {
				// And simply merge colors accordingly.
				pax_merge_pixel(buf, color, x, y);
			}
			// Move X.
			x_a += x1_x2_dx;
			x_b += x0_x2_dx;
		}
	}
}



/* ============ DEBUG ============ */

// Describe error.
char *pax_desc_err(pax_err_t error) {
	char *unknown = "Unknown error";
	char *desc[] = {
		"Success",
		"No framebuffer",
		"No memory",
		"Invalid parameters",
		"Infinite parameters",
		"Out of bounds",
		"Matrix stack underflow"
	};
	if (error > 0 || error < -6) return unknown;
	else return desc[-error];
}



/* ============ BUFFER =========== */

#define PAX_GET_BPP(type) ((type) & 0xff)

// Create a new buffer.
// If mem is NULL, a new area is allocated.
void pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type) {
	if (!mem) {
		// Allocate the right amount of bytes.
		mem = malloc((PAX_GET_BPP(type) * width * height) >> 3);
		if (!mem) PAX_ERROR(PAX_ERR_NOMEM);
	}
	*buf = (pax_buf_t) {
		.type       = type,
		.buf        = mem,
		.width      = width,
		.height     = height,
		.bpp        = PAX_GET_BPP(type),
		.stack_2d   = {
			.parent = NULL,
			.value  = matrix_2d_identity()
		}
	};
	pax_mark_clean(buf);
	pax_noclip(buf);
	PAX_SUCCESS();
}

// Destroy the buffer, freeing its memory.
void pax_buf_destroy(pax_buf_t *buf) {
	PAX_BUF_CHECK();
	
	matrix_stack_2d_t *current = buf->stack_2d.parent;
	while (current) {
		matrix_stack_2d_t *next = current->parent;
		free(current);
		current = next;
	}
	free(buf->buf);
	
	PAX_SUCCESS();
}

// Clip the buffer to the desired rectangle.
void pax_clip(pax_buf_t *buf, float x, float y, float width, float height) {
	// Make width and height positive.
	if (width < 0) {
		x += width;
		width = -width;
	}
	if (height < 0) {
		y += height;
		height = -height;
	}
	// Clip the entire rectangle to be at most the buffer's size.
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > buf->width) {
		width = buf->width - x;
	}
	if (y + height > buf->height) {
		height = buf->height - y;
	}
	// Apply the clip.
	buf->clip = (pax_rect_t) {
		.x = x,
		.y = y,
		.w = width,
		.h = height
	};
}

// Clip the buffer to it's full size.
void pax_noclip(pax_buf_t *buf) {
	buf->clip = (pax_rect_t) {
		.x = 0,
		.y = 0,
		.w = buf->width,
		.h = buf->height
	};
}

// Check whether the buffer is dirty.
bool pax_is_dirty(pax_buf_t *buf) {
	return buf->dirty_x0 < buf->dirty_x1;
}

// Mark the entire buffer as clean.
void pax_mark_clean(pax_buf_t *buf) {
	PAX_BUF_CHECK();
	buf->dirty_x0 = buf->width - 1;
	buf->dirty_y0 = buf->height - 1;
	buf->dirty_x1 = 0;
	buf->dirty_y1 = 0;
	PAX_SUCCESS();
}

// Mark the entire buffer as dirty.
void pax_mark_dirty0(pax_buf_t *buf) {
	PAX_BUF_CHECK();
	buf->dirty_x0 = 0;
	buf->dirty_y0 = 0;
	buf->dirty_x1 = buf->width;
	buf->dirty_y1 = buf->height;
	PAX_SUCCESS();
}

// Mark a single point as dirty.
void pax_mark_dirty1(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK();
	
	if (x < buf->dirty_x0) buf->dirty_x0 = x;
	if (x > buf->dirty_x1) buf->dirty_x1 = x;
	if (y < buf->dirty_x0) buf->dirty_y0 = y;
	if (y > buf->dirty_x1) buf->dirty_y1 = y;
	
	PAX_SUCCESS();
}

// Mark a rectangle as dirty.
void pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height) {
	PAX_BUF_CHECK();
	
	if (x              < buf->dirty_x0) buf->dirty_x0 = x;
	if (x + width  - 1 > buf->dirty_x1) buf->dirty_x1 = x + width  - 1;
	if (y              < buf->dirty_x0) buf->dirty_y0 = y;
	if (y + height - 1 > buf->dirty_x1) buf->dirty_y1 = y + height - 1;
	
	PAX_SUCCESS();
}



/* ============ COLORS =========== */

// A linear interpolation based only in ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
	return from + (( (to - from) * (part + (part >> 7)) ) >> 8);
}

// Converts HSV to ARGB.
pax_col_t pax_col_hsv(uint8_t _h, uint8_t s, uint8_t v) {
	uint16_t h     = _h * 6;
	uint16_t phase = h >> 8;
	// Parts of HSV.
	uint8_t up, down, other;
	other  = ~s;
	if (h & 0x100) {
		// Down goes away.
		up     = 0xff;
		down   = pax_lerp(s, 0xff, ~h & 0xff);
	} else {
		// Up comes in.
		up     = pax_lerp(s, 0xff,  h & 0xff);
		down   = 0xff;
	}
	// Apply brightness.
	up    = pax_lerp(v, 0, up);
	down  = pax_lerp(v, 0, down);
	other = pax_lerp(v, 0, other);
	// Apply to RGB.
	uint8_t r, g, b;
	switch (phase >> 1) {
		case 0:
			// From R to G.
			r = down; g = up; b = other;
			break;
		case 1:
			// From G to B.
			r = other; g = down; b = up;
			break;
		case 2:
			// From B to R.
			r = up; g = other; b = down;
			break;
		default:
			// Shut up, compiler.
			return 0;
	}
	// Merge.
	return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp(uint8_t part, pax_col_t from, pax_col_t to) {
	return (pax_lerp(part, from >> 24, to >> 24) << 24)
		 | (pax_lerp(part, from >> 16, to >> 16) << 16)
		 | (pax_lerp(part, from >>  8, to >>  8) <<  8)
		 |  pax_lerp(part, from,       to);
}

// Merges the two colors, based on alpha.
pax_col_t pax_col_merge(pax_col_t base, pax_col_t top) {
	if (!(top >> 24)) return base;
	if ((top >> 24) == 255) return top;
	uint8_t part = top >> 24;
	return (pax_lerp(part, base >> 24, 255)       << 24)
		 | (pax_lerp(part, base >> 16, top >> 16) << 16)
		 | (pax_lerp(part, base >>  8, top >>  8) <<  8)
		 |  pax_lerp(part, base,       top);
}



/* ============ MATRIX =========== */

// 2D rotation matrix: represents a 2D shearing.
matrix_2d_t matrix_2d_rotate(float angle) {
	float _cos = cosf(-angle);
	float _sin = sinf(-angle);
	// return (matrix_2d_t) { .arr = {
	// 	_cos, -_sin, 0,
	// 	_sin,  _cos, 0
	// }};
	return (matrix_2d_t) { .arr = {
		_cos, -_sin, 0,
		_sin,  _cos, 0
	}};
}

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) {
	// [a b c] [p q r] [ap+bs aq+bt ar+bu+c]
	// [d e f]*[s t u]=[dp+es dq+et dr+eu+f]
	// [0 0 1] [0 0 1] [0     0     1      ]
	return (matrix_2d_t) { .arr = {
		a.a0*b.a0 + a.a1*b.b0,   a.a0*b.a1 + a.a1*b.b1,  a.a0*b.a2 + a.a1*b.b2 + a.a2,
		a.b0*b.a0 + a.b1*b.b0,   a.b0*b.a1 + a.b1*b.b1,  a.b0*b.a2 + a.b1*b.b2 + a.b2
	}};
}

// 2D matrix: applies the transformation that a represents on to a point.
void matrix_2d_transform(matrix_2d_t a, float *x, float *y) {
	// [a b c] [x]  [a]  [b] [c] [ax+by+c]
	// [d e f]*[y]=x[d]+y[e]+[f]=[dx+ey+f]
	// [0 0 1] [1]  [0]  [0] [1] [1      ]
	float _x = *x, _y = *y;
	*x = a.a0*_x + a.a1*_y + a.a2;
	*y = a.b0*_x + a.b1*_y + a.b2;
}

// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a) {
	PAX_BUF_CHECK();
	buf->stack_2d.value = matrix_2d_multiply(buf->stack_2d.value, a);
	PAX_SUCCESS();
}

// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK();
	matrix_stack_2d_t *parent = malloc(sizeof(matrix_stack_2d_t));
	if (!parent) PAX_ERROR(PAX_ERR_NOMEM);
	*parent = buf->stack_2d;
	buf->stack_2d.parent = parent;
	PAX_SUCCESS();
}

// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK();
	matrix_stack_2d_t *parent = buf->stack_2d.parent;
	if (!parent) PAX_ERROR(PAX_ERR_UNDERFLOW);
	buf->stack_2d = *parent;
	free(parent);
	PAX_SUCCESS();
}



/* ======== DRAWING: PIXEL ======= */

static inline uint32_t pax_col2buf(pax_buf_t *buf, pax_col_t color) {
	if (buf->type == PAX_BUF_16_565RGB) {
		// 16BPP 565-RGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                       Rrrr rGgg gggB bbbb
		uint16_t value = ((color >> 8) & 0xf800) | ((color >> 5) & 0x07e0) | ((color >> 3) & 0x001f);
		return (value >> 8) | ((value << 8) & 0xff00);
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return color;
	}
	PAX_ERROR1(PAX_ERR_PARAM, 0);
}

static inline uint32_t pax_buf2col(pax_buf_t *buf, uint32_t value) {
	if (buf->type == PAX_BUF_16_565RGB) {
		// 16BPP 565-RGB
		// From:                     Rrrr rGgg gggB bbbb
		// To:   .... .... Rrrr r... Gggg gg.. Bbbb b...
		// Add:  .... .... .... .Rrr .... ..Gg .... .Bbb
		// Take the existing information.
		pax_col_t color = ((value << 8) & 0x00f80000) | ((value << 5) | 0x0000fc00) | ((value << 3) & 0x000000f8);
		// Now, fill in some missing bits.
		color |= ((value << 3) & 0x00070000) | ((value >> 1) & 0x00000300) | ((value >> 2) & 0x00000007);
		return color | 0xff000000;
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return value;
	}
	PAX_ERROR1(PAX_ERR_PARAM, 0);
}

// Set a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline void pax_set_pixel_u(pax_buf_t *buf, uint32_t color, int x, int y) {
	if (buf->type == PAX_BUF_16_565RGB) {
		buf->buf_16bpp[x + y * buf->width] = color;
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		buf->buf_32bpp[x + y * buf->width] = color;
	} else {
		PAX_ERROR(PAX_ERR_PARAM);
	}
}

// Get a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline uint32_t pax_get_pixel_u(pax_buf_t *buf, int x, int y) {
	if (buf->type == PAX_BUF_16_565RGB) {
		return buf->buf_16bpp[x + y * buf->width];
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return buf->buf_32bpp[x + y * buf->width];
	} else {
		PAX_ERROR1(PAX_ERR_PARAM, 0);
	}
}

// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK();
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	pax_col_t base = pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
	pax_set_pixel_u(buf, pax_col2buf(buf, pax_col_merge(base, color)), x, y);
}

// Set a pixel.
void pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK();
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	pax_set_pixel_u(buf, pax_col2buf(buf, color), x, y);
}

// Get a pixel.
pax_col_t pax_get_pixel(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK1(0);
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return 0;
	}
	PAX_SUCCESS();
	return pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
}



/* ========= DRAWING: 2D ========= */

// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK();
	if (matrix_2d_is_identity2(buf->stack_2d.value)) {
		// This can be simplified significantly.
		matrix_2d_transform(buf->stack_2d.value, &x, &y);
		width  *= buf->stack_2d.value.a0;
		height *= buf->stack_2d.value.b1;
		pax_simple_rect(buf, color, x, y, width, height);
	} else {
		// We need to go full quad.
		float x0 = x,         y0 = y;
		float x1 = x + width, y1 = y;
		float x2 = x + width, y2 = y + height;
		float x3 = x,         y3 = y + height;
		matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
		matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
		matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
		matrix_2d_transform(buf->stack_2d.value, &x3, &y3);
		pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
		pax_simple_tri(buf, color, x0, y0, x3, y3, x2, y2);
	}
}

// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK();
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	pax_simple_line(buf, color, x0, y0, x1, y1);
}

// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK();
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
	pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
}

// Draw na arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1) {
	PAX_BUF_CHECK();
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	matrix_2d_t matrix = buf->stack_2d.value;
	float _r = r * sqrtf(matrix.a0*matrix.a0 + matrix.b0*matrix.b0) * sqrtf(matrix.a1*matrix.a1 + matrix.b1*matrix.b1);
	if (_r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	else n_div = (a1 - a0) / M_PI * 16 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_draw_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r) {
	pax_draw_arc(buf, color, x, y, r, 0, M_PI * 2);
}



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color) {
	PAX_BUF_CHECK();
	
	uint32_t value = pax_col2buf(buf, color);
	
	if (buf->bpp == 16) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_16bpp[i] = value;
		}
	} else if (buf->bpp == 32) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_32bpp[i] = value;
		}
	}
	
	PAX_SUCCESS();
}

// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK();
	
	// Fix rect dimensions.
	if (width < 0) {
		width = -width;
		x -= width;
	}
	if (height < 0) {
		height = -height;
		y -= height;
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		width += buf->clip.x - x;
		x = buf->clip.x;
	}
	if (y < buf->clip.y) {
		height += buf->clip.y - y;
		y = buf->clip.y;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		height = buf->clip.y + buf->clip.h - y;
	}
	
	pax_mark_dirty2(buf, x + 0.5, y + 0.5, width + 0.5, height + 0.5);
	
	// Pixel time.
	for (int _y = y + 0.5; _y < y + height + 0.5; _y ++) {
		for (int _x = x + 0.5; _x < x + width + 0.5; _x ++) {
			pax_merge_pixel(buf, color, _x, _y);
		}
	}
	
	PAX_SUCCESS();
}

// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK();
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	if (y0 > y1) PAX_SWAP_POINTS(x0, y0, x1, y1);
	
	// Determine whether the line might fall within the clip rect.
	if (!buf->clip.w || !buf->clip.h) goto noneed;
	if (y1 < buf->clip.y || y0 > buf->clip.y + buf->clip.h - 1) goto noneed;
	if (x0 == x1 && (x0 < buf->clip.x || x0 > buf->clip.x + buf->clip.w - 1)) goto noneed;
	if (x0 < buf->clip.x && x1 < buf->clip.x) goto noneed;
	if (x0 > buf->clip.x + buf->clip.w - 1 && x1 > buf->clip.x + buf->clip.w - 1) goto noneed;
	
	// Clip top.
	if (y0 < buf->clip.y) {
		x0 = (x1 - x0) * (buf->clip.y - y0) / (y1 - y0);
		y0 = buf->clip.y;
	}
	// Clip bottom.
	if (y1 > buf->clip.y + buf->clip.h - 1) {
		x1 = (x1 - x0) * (buf->clip.y + buf->clip.h - 1 - y0) / (y1 - y0);
		y1 = buf->clip.y + buf->clip.h - 1;
	}
	// Clip left.
	if (x1 < buf->clip.x) {
		y1 = (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x1 = buf->clip.x;
	} else if (x0 < buf->clip.x) {
		y0 = (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x0 = buf->clip.x;
	}
	// Clip right.
	if (x1 > buf->clip.x + buf->clip.w - 1) {
		y1 = (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x1 = buf->clip.x + buf->clip.w - 1;
	} else if (x0 > buf->clip.x + buf->clip.w - 1) {
		y0 = (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x0 = buf->clip.x + buf->clip.w - 1;
	}
	
	// If any point is outside clip now, we don't draw a line.
	if (y0 < buf->clip.y || y1 > buf->clip.y + buf->clip.h - 1) goto noneed;
	
	// Determine whether the line is "steep" (dx*dx > dy*dy).
	float dx = x1 - x0;
	float dy = y1 - y0;
	bool is_steep = fabs(dx) < fabs(dy);
	int nIter;
	
	if (is_steep) nIter = fabs(dy) + 0.5;
	else nIter = fabs(dx) + 0.5;
	
	// Adjust dx and dy.
	dx /= nIter;
	dy /= nIter;
	float x = x0, y = y0;
	for (int i = 0; i < nIter; i++) {
		x += dx;
		y += dy;
		pax_merge_pixel(buf, color, x + 0.5, y + 0.5);
	}
	
	// This label is used if there's no need to try to draw a line.
	noneed:
	PAX_SUCCESS();
}

// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK();
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	// Sort points by height.
	PAX_SORT_POINTS(x0, y0, x1, y1);
	PAX_SORT_POINTS(x0, y0, x2, y2);
	PAX_SORT_POINTS(x1, y1, x2, y2);
	
	if (y2 == y0 || (x2 == x0 && x1 == x0)) {
		// We can't draw a flat triangle.
		PAX_SUCCESS();
		return;
	}
	
	pax_tri_unshaded(buf, color, x0, y0, x1, y1, x2, y2);
	
	PAX_SUCCESS();
}

// Draw a arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
	PAX_BUF_CHECK();
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	if (r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	if (r > 20) n_div = (a1 - a0) / M_PI * 16 + 1;
	else n_div = (a1 - a0) / M_PI * 8 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_simple_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
	pax_simple_arc(buf, color, x, y, r, 0, M_PI);
}
