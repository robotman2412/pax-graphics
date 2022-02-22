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
#include "pax_internal.h"

// This is only applicable during bezier vectorisation.
typedef struct bezier_point {
	float x,  y;
	// float dx, dy;
	float part;
} bezier_point_t;

// This is only applicable during bezier vectorisation.
typedef struct bezier_segment {
	bezier_point_t *from;
	bezier_point_t *to;
} bezier_segment_t;

// Calcultesa point on a bezier curve based on given control points.
static inline bezier_point_t pax_calc_bezier0(float part, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
	// First set of interpolations.
	float xa = x0 + (x1 - x0) * part;
	float xb = x1 + (x2 - x1) * part;
	float xc = x2 + (x3 - x2) * part;
	// Second set of interpolations.
	float xp = xa + (xb - xa) * part;
	float xq = xb + (xc - xb) * part;
	// Final interpolation.
	float x  = xp + (xq - xp) * part;
	
	// First set of interpolations.
	float ya = y0 + (y1 - y0) * part;
	float yb = y1 + (y2 - y1) * part;
	float yc = y2 + (y3 - y2) * part;
	// Second set of interpolations.
	float yp = ya + (yb - ya) * part;
	float yq = yb + (yc - yb) * part;
	// Final interpolation.
	float y  = yp + (yq - yp) * part;
	
	return (bezier_point_t) { .x = x, .y = y, .part = part };
}

// Macro for calculating a point on a given bezier curve.
static inline bezier_point_t pax_calc_bezier(float part, pax_vec4_t ctl) {
	return pax_calc_bezier0(part, ctl.x0, ctl.y0, ctl.x1, ctl.y1, ctl.x2, ctl.y2, ctl.x3, ctl.y3);
}

// Comparison function for two line segments.
// Sorts by absolute line length.
// TODO: Convert to delta over length?
static int bezier_bifurcate_comp(const void *e0, const void *e1) {
	// Pointer conversion.
	const bezier_segment_t *a = e0;
	const bezier_segment_t *b = e1;
	// Length of line A.
	float x_a = a->to->x - a->from->x;
	float y_a = a->to->y - a->from->y;
	float len_a = x_a * x_a + y_a * y_a;
	// Length of line B.
	float x_b = b->to->x - b->from->x;
	float y_b = b->to->y - b->from->y;
	// Sort them.
	float len_b = x_b * x_b + y_b * y_b;
	if (len_a < len_b) return 1;
	if (len_a > len_b) return -1;
	return 0;
}

// Comparison function for two points.
// Sorts by T value (part along curve).
static int bezier_point_t_comp(const void *e0, const void *e1) {
	// Pointer conversion.
	const bezier_point_t *a = e0;
	const bezier_point_t *b = e1;
	// Comparing is the same as subtracting.
	float delta = b->part - a->part;
	return delta / fabs(delta);
}

// Comparison function for two line segments.
// Sorts by T value (part along curve).
static int bezier_line_t_comp(const void *e0, const void *e1) {
	// Pointer conversion.
	const bezier_segment_t *a = e0;
	const bezier_segment_t *b = e1;
	// Comparing is the same as subtracting.
	float delta = b->from->part - a->from->part;
	return delta / fabs(delta);
}



/* ============ CURVES =========== */

#ifdef PAX_COMPILE_BEZIER

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to are from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points, float t_from, float t_to) {
	if (max_points < 4) PAX_ERROR1("pax_vectorise_bezier", PAX_ERR_PARAM, 0);
	
	// Start with just three points: start, T=0.5 and end.
	bezier_point_t *points = malloc(sizeof(bezier_point_t) * max_points);
	points[0] = pax_calc_bezier(t_from,                control_points);
	points[1] = pax_calc_bezier((t_from + t_to) * 0.5, control_points);
	points[2] = pax_calc_bezier(t_to,                  control_points);
	size_t n_points = 3;
	if (!points) {
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_NOMEM);
		return;
	}
	
	// Turn the points into lines.
	bezier_segment_t *lines = malloc(sizeof(bezier_segment_t) * (max_points - 1));
	lines[0] = (bezier_segment_t) { .from = &points[0], .to = &points[1] };
	lines[1] = (bezier_segment_t) { .from = &points[1], .to = &points[2] };
	size_t n_lines = 2;
	if (!points) {
		free(points);
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_NOMEM);
		return;
	}
	
	// Bifurcate the longest line segment continuously.
	while (n_points < max_points) {
		// Find the segment to bifurcate.
		qsort(lines, n_lines, sizeof(bezier_segment_t), bezier_bifurcate_comp);
		float new_part   = (lines[0].from->part + lines[0].to->part) * 0.5;
		// Create a point at the average T.
		points[n_points] = pax_calc_bezier(new_part, control_points);
		// Bifurcate the line.
		lines[n_lines]   = (bezier_segment_t) { .from = &points[n_points], .to = lines[0].to };
		lines[0].to      = &points[n_points];
		// The incrementing.
		n_points ++;
		n_lines ++;
	}
	
	// Sort points by T value.
	qsort(points, n_points, sizeof(bezier_point_t), bezier_point_t_comp);
	// Output our GARBAGE.
	pax_vec1_t *ptr = malloc(sizeof(pax_vec1_t) * n_points);
	if (!points) {
		free(points);
		free(lines);
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_NOMEM);
		return;
	}
	for (size_t i = 0; i < n_points; i++) {
		ptr[i] = (pax_vec1_t) { .x = points[i].x, .y = points[i].y };
	}
	*output = ptr;
	free(points);
	free(lines);
}

// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points) {
	pax_vectorise_bezier_part(output, control_points, max_points, 0, 1);
}

// Draw a cubic bezier curve.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points, float from, float to) {
	pax_vec1_t *points;
	size_t n_points = 64;
	pax_vectorise_bezier(&points, control_points, n_points);
	if (!points) return;
	for (size_t i = 0; i < n_points - 1; i++) {
		pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}
	free(points);
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points) {
	pax_draw_bezier_part(buf, color, control_points, 0, 1);
}

#else

static bool bezier_warning = false;
static void pax_bezier_warn() {
	if (!bezier_warning) {
		ESP_LOGE(TAG, "Failed: Bezier curves not compiled, please define PAX_COMPILE_BEZIER.");
	}
}

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to are from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points, float t_from, float t_to) {
	// Not compiled in, but keep the method for API compatibility.
	pax_bezier_warn();
	PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_UNSUPPORTED);
}

// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points) {
	// Not compiled in, but keep the method for API compatibility.
	pax_bezier_warn();
	PAX_ERROR("pax_vectorise_bezier", PAX_ERR_UNSUPPORTED);
}

// Draw a cubic bezier curve.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points, float from, float to) {
	// Not compiled in, but keep the method for API compatibility.
	pax_bezier_warn();
	PAX_ERROR("pax_draw_bezier_part", PAX_ERR_UNSUPPORTED);
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points) {
	// Not compiled in, but keep the method for API compatibility.
	pax_bezier_warn();
	PAX_ERROR("pax_draw_bezier", PAX_ERR_UNSUPPORTED);
}

#endif //PAX_COMPILE_BEZIER
