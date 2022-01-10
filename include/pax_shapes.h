/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

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

#ifndef PAX_SHAPES_H
#define PAX_SHAPES_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pax_types.h"
#include "pax_gfx.h"

/* ============ CURVES =========== */

// Convert a cubic bezier curve to line segments.
// Returns the number of segments created.
size_t pax_vectorise_bezier(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points);

// Draw a cubic bezier curve.
void   pax_draw_bezier     (pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_SHAPES_H
