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
#include <stdio.h>
#include <esp_system.h>
#include <sdkconfig.h>
#include <esp_err.h>
#include <esp_log.h>

/* =========== HELPERS =========== */

pax_err_t pax_last_error = PAX_OK;
static const char *TAG   = "pax";

#ifdef PAX_AUTOREPORT
#define PAX_ERROR(where, errno) { ESP_LOGE(TAG, "@ %s: %s", where, pax_desc_err(errno)); pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { ESP_LOGE(TAG, "@ %s: %s", where, pax_desc_err(errno)); pax_last_error = errno; return retval; }
#else
#define PAX_ERROR(where, errno) { pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { pax_last_error = errno; return retval; }
#endif

#define PAX_SUCCESS() { pax_last_error = PAX_OK; }

// Buffer sanity check.
#define PAX_BUF_CHECK(where) { if (!(buf) || !(buf)->buf) PAX_ERROR(where, PAX_ERR_NOBUF); }
// Buffer sanity check.
#define PAX_BUF_CHECK1(where, retval) { if (!(buf) || !(buf)->buf) PAX_ERROR1(where, PAX_ERR_NOBUF, retval); }

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

#define pax_calc_bezier(part, ctl) pax_calc_bezier0(part, (ctl).x0, (ctl).y0, (ctl).x1, (ctl).y1, (ctl).x2, (ctl).y2, (ctl).x3, (ctl).y3)
static inline bezier_point_t pax_calc_bezier0(float part, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
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
	
	return (bezier_point_t) { .x = x, .y = y, .part = part };
}

static int pax_vectorise_bezier_segment_comp(const void *e0, const void *e1) {
	const bezier_segment_t *a = e0;
	const bezier_segment_t *b = e1;
	float x_a = a->to->x - a->from->x;
	float y_a = a->to->y - a->from->y;
	float len_a = x_a * x_a + y_a * y_a;
	float x_b = b->to->x - b->from->x;
	float y_b = b->to->y - b->from->y;
	float len_b = x_b * x_b + y_b * y_b;
	if (len_a < len_b) return -1;
	if (len_a > len_b) return 1;
	return 0;
}

static int pax_vectorise_bezier_point_comp(const void *e0, const void *e1) {
	const bezier_point_t *a = e0;
	const bezier_point_t *b = e1;
	float delta = b->part - a->part;
	return delta / fabs(delta);
}

/* ============ CURVES =========== */

// Convert a cubic bezier curve to line segments.
// Returns the number of segments created.
size_t pax_vectorise_bezier(pax_vec1_t **output, pax_vec4_t control_points, size_t max_points) {
	if (max_points < 4) PAX_ERROR1("pax_vectorise_bezier", PAX_ERR_PARAM, 0);
	
	// Start with just three points: start, T=0.5 and end.
	bezier_point_t *points = malloc(sizeof(bezier_point_t) * max_points);
	points[0] = (bezier_point_t) { .x = control_points.x0, .y = control_points.y0, .part = 0 };
	points[1] = pax_calc_bezier(0.5, control_points);
	points[2] = (bezier_point_t) { .x = control_points.x3, .y = control_points.y3, .part = 1 };
	size_t n_points = 3;
	
	// Turn the points into lines.
	bezier_segment_t *lines = malloc(sizeof(bezier_segment_t) * max_points);
	lines[0] = (bezier_segment_t) { .from = &points[0], .to = &points[1] };
	lines[1] = (bezier_segment_t) { .from = &points[1], .to = &points[2] };
	size_t n_lines = 2;
	
	// Bifurcate the longest line segment continuously.
	size_t n_iter = max_points - 3;
	for (size_t i = 0; i < n_iter; i++) {
		qsort(lines, n_lines, sizeof(bezier_segment_t), pax_vectorise_bezier_segment_comp);
	}
	
	return 0;
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points) {
	pax_vec1_t *points;
	size_t n_points = pax_vectorise_bezier(&points, control_points, 64);
	for (size_t i = 0; i < n_points - 1; i++) {
		pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}
}
