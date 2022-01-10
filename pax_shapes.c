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

#include "pax_shapes.h"
#include <stdlib.h>

#define pax_calc_bezier(part, ctl) pax_calc_bezier0(part, (ctl).x0, (ctl).y0, (ctl).x1, (ctl).y1, (ctl).x2, (ctl).y2, (ctl).x3, (ctl).y3)
static inline pax_vec1_t pax_calc_bezier0(float part, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
	// This is literally just a series of linear interpolations.
	float xa = x0 + (x1 - x0) * part;
	float xb = x1 + (x2 - x1) * part;
	float xc = x2 + (x3 - x2) * part;
	float xp = xa + (xb - xa) * part;
	float xq = xb + (xc - xb) * part;
	float x  = xp + (xq - xp) * part;
	
	float ya = y0 + (y1 - y0) * part;
	float yb = y1 + (y2 - y1) * part;
	float yc = y2 + (y3 - y2) * part;
	float yp = ya + (yb - ya) * part;
	float yq = yb + (yc - yb) * part;
	float y  = yp + (yq - yp) * part;
	
	return (pax_vec1_t) { .x = x, .y = y };
}

static int pax_vectorise_bezier_comp(const void *e0, const void *e1) {
	const pax_vec2_t *a = e0;
	const pax_vec2_t *b = e1;
	float x_a = a->x1 - a->x0;
	float y_a = a->y1 - a->y0;
	float len_a = x_a * x_a + y_a * y_a;
	float x_b = b->x1 - b->x0;
	float y_b = b->y1 - b->y0;
	float len_b = x_b * x_b + y_b * y_b;
	if (len_a < len_b) return -1;
	if (len_a > len_b) return 1;
	return 0;
}

// Convert a cubic bezier curve to line segments.
// Returns the number of segments created.
size_t pax_vectorise_bezier(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points) {
	// Start with just two lines: start to T=0.5 to end.
	pax_vec2_t *buf = malloc(sizeof(pax_vec2_t) * (max_points - 1));
	pax_vec1_t center = pax_calc_bezier(0.5, control_points);
	size_t n_lines = 2;
	buf[0] = (pax_vec2_t) { .x0 = control_points.x0, .y0 = control_points.y0, .x1 = center.x, .y1 = center.y };
	buf[1] = (pax_vec2_t) { .x0 = center.x, .y0 = center.y, .x1 = control_points.x3, .y1 = control_points.y3 };
	
	// Iterate continuously: take the longest line segment and then split it.
	size_t n_iter = max_points - 3;
	for (size_t i = 0; i < n_iter; i++) {
		qsort(buf, n_lines, sizeof(pax_vec2_t), pax_vectorise_bezier_comp);
	}
	
	// Convert lines back to points.
	size_t index;
	pax_vec1_t *point_buf = malloc(sizeof(pax_vec1_t) * (n_lines + 1));
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points) {
	pax_vec1_t *points;
	size_t n_points = pax_vectorise_bezier(&points, control_points, 64);
	for (size_t i = 0; i < n_points - 1; i++) {
		pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}
}
