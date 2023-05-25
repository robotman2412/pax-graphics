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

#ifndef PAX_GFX_C
#ifndef ARDUINO
#pragma message "This file should not be compiled on it's own."
#endif
#else

#include "pax_internal.h"

/* ======= UNSHADED DRAWING ====== */

// Internal method for unshaded triangles.
// Assumes points are sorted by Y.
void pax_tri_unshaded(pax_buf_t *buf, pax_col_t color,
		float x0, float y0, float x1, float y1, float x2, float y2) {
	
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	
	// Sort points by height.
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
	}
	if (y2 < y0) {
		PAX_SWAP_POINTS(x0, y0, x2, y2);
	}
	if (y2 < y1) {
		PAX_SWAP_POINTS(x1, y1, x2, y2);
	}
	
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
		int delta = ((int) y_post_0) * buf->width;
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
			x_right -= 0.5;
			for (int x = x_left + 0.5; x <= x_right; x ++) {
				// And simply merge colors accordingly.
				setter(buf, color, x+delta);
			}
			// Move X.
			x_a   += x0_x1_dx;
			x_b   += x0_x2_dx;
			delta += buf->width;
		}
	}
	// Draw bottom half.
	// This condition might be confusing, but it's false if no point at all is inside the triangle.
	if (y_post_0 <= y_pre_2 && y_post_1 >= y1 && y_pre_2 <= y2) {
		// Find the X counterparts to the other points we found.
		float x_a = x1 + x1_x2_dx * (y_post_1 - y1);
		float x_b = x0 + x0_x2_dx * (y_post_1 - y0);
		int delta = ((int) y_post_1) * buf->width;
		for (int y = y_post_1; y <= y_pre_2; y++) {
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
			x_right -= 0.5;
			for (int x = x_left + 0.5; x <= x_right; x ++) {
				// And simply merge colors accordingly.
				setter(buf, color, x+delta);
			}
			// Move X.
			x_a   += x1_x2_dx;
			x_b   += x0_x2_dx;
			delta += buf->width;
		}
	}
}

#define PDHG_NAME pax_rect_unshaded
#include "pax_dh_generic_rect.h"
/*
// Internal method for rectangle drawing.
void pax_rect_unshaded(pax_buf_t *buf, pax_col_t color,
		float x, float y, float width, float height) {
	
	// pax_setter_t setter = color >= 0xff000000 ? pax_set_pixel : pax_merge_pixel;
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	
	// Pixel time.
	int delta = (int) (y + 0.5) * buf->width;
	for (int c_y = y + 0.5; c_y <= y + height - 0.5; c_y ++) {
		for (int c_x = x + 0.5; c_x <= x + width - 0.5; c_x ++) {
			setter(buf, color, c_x+delta);
		}
		delta += buf->width;
	}
}
*/

// Internal method for line drawing.
void pax_line_unshaded(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
	}
	
	// Clip: left.
	if (x0 < x1 && x0 < buf->clip.x) {
		if (x1 < buf->clip.x) return;
		// Adjust X0 against left clip.
		y0 = y0 + (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x0 = buf->clip.x;
	} else if (x1 < x0 && x1 < buf->clip.x) {
		if (x0 < buf->clip.x) return;
		// Adjust X1 against left clip.
		y1 = y1 + (y0 - y1) * (buf->clip.x - x1) / (x0 - x1);
		x1 = buf->clip.x;
	}
	
	// Clip: right.
	if (x1 > x0 && x1 > buf->clip.x + buf->clip.w - 1) {
		if (x0 > buf->clip.x + buf->clip.w) return;
		// Adjust X1 against right of clip.
		y1 = y0 + (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x1 = buf->clip.x + buf->clip.w - 1;
	} else if (x0 > x1 && x0 > buf->clip.x + buf->clip.w - 1) {
		if (x1 > buf->clip.x + buf->clip.w) return;
		// Adjust X0 against right of clip.
		y0 = y1 + (y0 - y1) * (buf->clip.x + buf->clip.w - 1 - x1) / (x0 - x1);
		x0 = buf->clip.x + buf->clip.w - 1;
	}
	
	// Clip: top.
	if (y0 < buf->clip.y) {
		if (y1 < buf->clip.y) return;
		// Adjust Y0 against top of clip.
		x0 = x0 + (x1 - x0) * (buf->clip.y - y0) / (y1 - y0);
		y0 = buf->clip.y;
	}
	
	// Clip: bottom.
	if (y1 > buf->clip.y + buf->clip.h - 1) {
		if (y0 > buf->clip.y + buf->clip.h - 1) return;
		// Adjust Y1 against bottom of clip.
		x1 = x1 + (x1 - x0) * (buf->clip.y + buf->clip.h - 1 - y1) / (y1 - y0);
		y1 = buf->clip.y + buf->clip.h - 1;
	}
	
	// Determine whether the line is "steep" (dx*dx > dy*dy).
	float dx = x1 - x0;
	float dy = y1 - y0;
	bool is_steep = fabsf(dx) < fabsf(dy);
	int nIter;
	
	// Determine the number of iterations.
	nIter = ceilf(fabsf(is_steep ? dy : dx));
	if (nIter < 1) nIter = 1;
	
	// Adjust dx and dy.
	dx /= nIter;
	dy /= nIter;
	
	if (y0 == y1) {
		int index = (int) y0 * buf->width;
		if (dx < 0) {
			PAX_SWAP(float, x0, x1);
		}
		for (int i = x0; i <= x1; i++) {
			setter(buf, color, index + i);
		}
	} else if (x0 == x1) {
		int index = x0 + (int) y0 * buf->width;
		for (int i = y0; i <= y1; i++, index += buf->width) {
			setter(buf, color, index);
		}
	} else {
		int_fast32_t x   = x0 * 0x10000;
		int_fast32_t y   = y0 * 0x10000;
		int_fast32_t idx = dx * 0x10000;
		int_fast32_t idy = dy * 0x10000;
		for (int i = 0; i <= nIter; i++) {
			setter(buf, color, x/0x10000+y/0x10000*buf->width);
			x += idx;
			y += idy;
		}
	}
}

#endif
