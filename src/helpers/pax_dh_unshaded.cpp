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

#include "pax_internal.h"

/* ======= UNSHADED DRAWING ====== */

// Internal method for unshaded triangles.
#define PDHG_NAME pax_tri_unshaded
#include "pax_dh_generic_tri.hpp"

// Internal method for rectangle drawing.
#define PDHG_NAME pax_rect_unshaded
#include "pax_dh_generic_rect.hpp"

// Internal method for line drawing.
void pax_line_unshaded(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	
	pax_index_setter_t setter = pax_get_setter(buf, &color, NULL);
	if (!setter) return;
	
	if (y1 < y0) {
		PAX_SWAP(float, x0, x1)
		PAX_SWAP(float, y0, y1)
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
			setter(buf, color, (x>>16)+(y>>16)*buf->width);
			x += idx;
			y += idy;
		}
	}
}
