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
#include <string.h>
#include <malloc.h>

static const char *TAG = "pax-shapes";

// This is only applicable during shape triangulation.
typedef struct indexed_point {
	union {
		struct {
			float  x, y;
		};
		pax_vec1_t vector;
	};
	size_t         index;
} indexed_point_t;

// This is only applicable during bezier vectorisation.
typedef struct bezier_point {
	float x, y;
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



/* ============ CURVES =========== */

#if PAX_COMPILE_BEZIER

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to are from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(pax_vec1_t *ptr, size_t max_points, pax_vec4_t control_points, float t_from, float t_to) {
	if (max_points < 4) PAX_ERROR("pax_vectorise_bezier", PAX_ERR_PARAM);
	if (!ptr) {
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_PARAM);
		return;
	}
	
	// Start with just three points: start, T=0.5 and end.
	bezier_point_t *points = malloc(sizeof(bezier_point_t) * max_points);
	if (!points) {
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_NOMEM);
		return;
	}
	points[0] = pax_calc_bezier(t_from,                control_points);
	points[1] = pax_calc_bezier((t_from + t_to) * 0.5, control_points);
	points[2] = pax_calc_bezier(t_to,                  control_points);
	size_t n_points = 3;
	
	// Turn the points into lines.
	bezier_segment_t *lines = malloc(sizeof(bezier_segment_t) * (max_points - 1));
	if (!lines) {
		free(points);
		PAX_ERROR("pax_vectorise_bezier_part", PAX_ERR_NOMEM);
		return;
	}
	lines[0] = (bezier_segment_t) { .from = &points[0], .to = &points[1] };
	lines[1] = (bezier_segment_t) { .from = &points[1], .to = &points[2] };
	size_t n_lines = 2;
	
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
	for (size_t i = 0; i < n_points; i++) {
		ptr[i] = (pax_vec1_t) { .x = points[i].x, .y = points[i].y };
	}
	free(points);
	free(lines);
	
	PAX_SUCCESS();
}

// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec1_t *output, size_t max_points, pax_vec4_t control_points) {
	pax_vectorise_bezier_part(output, max_points, control_points, 0, 1);
}

// Draw a cubic bezier curve.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points, float from, float to) {
	size_t n_points = 64;
	if (to < from) {
		PAX_SWAP(float, to, from);
	}
#if PAX_USE_EXPENSIVE_BEZIER
	// Vectorise the bezier curve first.
	pax_vec1_t *points;
	pax_vectorise_bezier(&points, n_points, control_points);
	if (!points) return;
	
	// Then draw it.
	for (size_t i = 0; i < n_points - 1; i++) {
		pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}
	free(points);
#else
	// Draw the curve in a more simple manner.
	bezier_point_t last_point = pax_calc_bezier(from, control_points);
	float delta = (to - from) / (n_points - 2);
	float part  = from;
	for (size_t i = 0; i < n_points - 1; i++) {
		bezier_point_t point = pax_calc_bezier(part, control_points);
		pax_draw_line(buf, color, last_point.x, last_point.y, point.x, point.y);
		last_point = point;
		part += delta;
	}
#endif
	
	PAX_SUCCESS();
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_vec4_t control_points) {
	pax_draw_bezier_part(buf, color, control_points, 0, 1);
}

#else

static bool bezier_warning = false;
static void pax_bezier_warn() {
	if (!bezier_warning) {
		PAX_LOGE(TAG, "Failed: Bezier curves not compiled, please define PAX_COMPILE_BEZIER.");
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



/* ============= ARCS ============ */

// Vectorise an arc outline, angle in radians.
void pax_vectorise_arc(pax_vec1_t *ptr, size_t n_div, float x, float y, float r, float a0, float a1) {
	// Allocate output array.
	if (!ptr) {
		PAX_ERROR("pax_vectorise_arc", PAX_ERR_PARAM);
	}
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / (n_div - 1);
	float c_sin = sinf(div_angle);
	float c_cos = cosf(div_angle);
	
	// Start with a unit vector according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * c_cos - y0 * c_sin;
		float y1 = x0 * c_sin + y0 * c_cos;
		// Store to array.
		// We subtract y0 and y1 from y because our up is -y.
		ptr[i] = (pax_vec1_t) { .x = x + x0 * r, .y = y - y0 * r };
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Vectorise a circle outline.
void pax_vectorise_circle(pax_vec1_t *output, size_t num_points, float x, float y, float r) {
	pax_vectorise_arc(output, num_points, x, y, r, 0, M_PI * 2);
}



/* =========== OUTLINES ========== */

// Draw a rectangle outline.
void pax_outline_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	pax_push_2d(buf);
	pax_apply_2d(buf, matrix_2d_translate(x, y));
	pax_draw_line(buf, color, 0,     0,      width, 0);
	pax_draw_line(buf, color, 0,     height, width, height);
	pax_draw_line(buf, color, 0,     0,      0,     height);
	pax_draw_line(buf, color, width, 0,      width, height);
	pax_pop_2d(buf);
}

// Draw a triangle outline.
void pax_outline_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	pax_draw_line(buf, color, x0, y0, x1, y1);
	pax_draw_line(buf, color, x2, y2, x1, y1);
	pax_draw_line(buf, color, x0, y0, x2, y2);
}

// Draw an arc outline, angle in radians.
void pax_outline_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
	PAX_BUF_CHECK("pax_outline_arc");
	
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
	float c_r = r * sqrtf(matrix.a0*matrix.a0 + matrix.b0*matrix.b0) * sqrtf(matrix.a1*matrix.a1 + matrix.b1*matrix.b1);
	if (c_r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	else n_div = (a1 - a0) / M_PI * 16 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float c_sin = sinf(div_angle);
	float c_cos = cosf(div_angle);
	
	// Start with a unit vector according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * c_cos - y0 * c_sin;
		float y1 = x0 * c_sin + y0 * c_cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_draw_line(buf, color, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle outline.
void pax_outline_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
	pax_outline_arc(buf, color, x, y, r, 0, M_PI * 2);
}

// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// Does not close the shape: this must be done manually.
void pax_outline_shape_part(pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec1_t *points, float from, float to) {
	// El simplify.
	if (to < from) {
		PAX_SWAP(float, to, from);
	}
	if (from <= 0 && to >= 1) {
		pax_outline_shape(buf, color, num_points, points);
		return;
	}
	
	// Calculate total distance.
	float *dist       = malloc(sizeof(float) * (num_points - 1));
	float  total_dist = 0;
	float  start_dist = 0;
	if (!dist) {
		PAX_ERROR("pax_outline_shape_part", PAX_ERR_NOMEM);
	}
	for (size_t i = 0; i < num_points - 1; i++) {
		float dx    = points[i + 1].x - points[i].x;
		float dy    = points[i + 1].y - points[i].y;
		dist[i]     = sqrtf(dx*dx + dy*dy);
		total_dist += dist[i];
	}
	
	// Do distance calculations.
	start_dist  = total_dist * from;
	total_dist *= to;
	
	// Draw until the maximum length is reached.
	for (size_t i = 0; i < num_points - 1; i++) {
		if (start_dist > dist[i]) {
			// Skip the line segment.
		} else if (start_dist > 0) {
			float dx    = points[i + 1].x - points[i].x;
			float dy    = points[i + 1].y - points[i].y;
			float part0 = start_dist / dist[i];
			if (total_dist > dist[i]) {
				// Draw the end of a segment.
				pax_draw_line(
					buf, color,
					points[i].x + dx * part0,
					points[i].y + dy * part0,
					points[i + 1].x,
					points[i + 1].y
				);
			} else {
				// Draw the middle of a segment.
				float part1 = total_dist / dist[i];
				pax_draw_line(
					buf, color,
					points[i].x + dx * part0,
					points[i].y + dy * part0,
					points[i].x + dx * part1,
					points[i].y + dy * part1
				);
			}
		} else if (dist[i] < total_dist) {
			// Draw the entire segment.
			pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
		} else {
			// Draw the start of a segment.
			float dx    = points[i + 1].x - points[i].x;
			float dy    = points[i + 1].y - points[i].y;
			float part  = total_dist / dist[i];
			pax_draw_line(
				buf, color,
				points[i].x,
				points[i].y,
				points[i].x + dx * part,
				points[i].y + dy * part
			);
			break;
		}
		total_dist -= dist[i];
		start_dist -= dist[i];
	}
	
	free(dist);
	PAX_SUCCESS();
}

// Outline a shape defined by a list of points.
// Does not close the shape: this must be done manually.
void pax_outline_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec1_t *points) {
	for (size_t i = 0; i < num_points - 1; i++) {
		pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}
}



/* ======== TRIANGULATION ======== */

#if PAX_COMPILE_TRIANGULATE
// Determine whether the points go clockwise or counter-clockwise.
// Does not work for less then 3 points.
static bool is_clockwise(int num_points, indexed_point_t *points, int index, int num_test, float dy) {
	float result = 0;
	
	// Simple but unoptimised loop.
	for (int i = 0; i < num_test; i++) {
		int index0 = i+index;
		int index1 = (i+1) % num_test +index;
		index0 %= num_points;
		index1 %= num_points;
		result += (points[index1].x - points[index0].x) * (points[index1].y + points[index0].y + dy);
	}
	
	// Clockwise if result < 0.
	return result < 0;
}

// Gets the slope of a line.
// Returns +/- infinity for vertical lines.
static inline float line_slope(pax_vec2_t line) {
	return (line.y1 - line.y0) / (line.x1 - line.x0);
}

// Creates a bounding rectangle for a line.
static pax_rect_t line_bounding_box(pax_vec2_t line) {
	// Create a simple bounding box.
	pax_rect_t box = {
		.x = line.x0,
		.y = line.y0,
		.w = line.x1 - line.x0,
		.h = line.y1 - line.y0,
	};
	
	// Fix width/height so they are positive.
	if (box.w < 0) {
		box.x += box.w;
		box.w = -box.w;
	}
	if (box.h < 0) {
		box.y += box.h;
		box.h = -box.h;
	}
	
	return box;
}

// Determines whether a point is in the bounding box, but not on it's edge.
static inline bool bounding_box_contains(pax_rect_t box, pax_vec1_t point) {
	return point.x > box.x && point.y > box.y && point.x < box.x + box.w && point.y < box.y + box.h;
}

// Tests whether lines A and B intersect.
// Does not consider touching lines to intersect.
static bool line_intersects_line(pax_vec2_t line_a, pax_vec2_t line_b) {
	// If slopes are equal, then it will never intersect.
	float rc_a = line_slope(line_a);
	float rc_b = line_slope(line_b);
	if (rc_a == rc_b || (isinf(rc_a) && isinf(rc_b))) return false;
	
	// Determine b in Y=a*X+b line formulas.
	float dy_a = line_a.y0 - rc_a * line_a.x0;
	float dy_b = line_b.y0 - rc_b * line_b.x0;
	
	// Determine bounding boxes.
	pax_rect_t box_a = line_bounding_box(line_a);
	pax_rect_t box_b = line_bounding_box(line_b);
	
	// Special cases for one of two lines is vertical.
	if (isinf(rc_a)) {
		float y = rc_b * line_a.x0 + dy_b;
		if (y > box_a.y && y < box_a.y + box_a.h) return true;
	}
	if (isinf(rc_b)) {
		float y = rc_a * line_b.x0 + dy_a;
		if (y > box_b.y && y < box_b.y + box_b.h) return true;
	}
	
	// Find the intersection point, assuming infinitely long lines.
	float x = (dy_b - dy_a) / (rc_a - rc_b);
	float y = x * rc_a + dy_a;
	
	// If this lies within both bounding boxes, the lines intersect.
	return bounding_box_contains(box_a, (pax_vec1_t) {x, y}) && bounding_box_contains(box_b, (pax_vec1_t) {x, y});
}

// Tests whether a line intersects any of the lines in the dataset.
// Intersection is NOT counted when only the end points touch.
static bool line_intersects_outline(size_t num_points, pax_vec1_t *raw_points, pax_vec1_t start, pax_vec1_t end) {
	for (size_t i = 0; i < num_points; i++) {
		size_t index1 = (i + 1) % num_points;
		if (line_intersects_line(
			(pax_vec2_t) {start.x, start.y, end.x, end.y},
			(pax_vec2_t) {raw_points[i].x, raw_points[i].y, raw_points[index1].x, raw_points[index1].y}
		)) return true;
	}
	return false;
}

// Triangulates a shape based on an outline (any shape).
// In effect, this creates triangles which completely fill the shape.
// Closes the shape: no need to have the last point overlap the first.
//
// Capable of dealing with self-intersecting shapes:
// Returns a set of additional points, positioned at every intersection.
// These points are to be treated as concatenated to the original points array.
//
// Stores triangles as triple-index pairs in output, which is a dynamically allocated size_t array.
// The number of triangles created is num_points - 2.
size_t pax_triang_complete(size_t **output, pax_vec1_t **additional_points, size_t num_points, pax_vec1_t *points) {
	return 0;
}

// Triangulates a shape based on an outline (concave, non self-intersecting only).
// In effect, this creates triangles which completely fill the shape.
// Closes the shape: no need to have the last point overlap the first.
// Assumes the shape does not intersect itself.
//
// Stores triangles as triple-index pairs in output, which is a dynamically allocated size_t array.
// The number of triangles created is num_points - 2.
void pax_triang_concave(size_t **output, size_t raw_num_points, pax_vec1_t *raw_points) {
	// Cannot triangulate with less than 3 points.
	if (raw_num_points < 3) {
		*output = NULL;
		return;
	}
	
	// Find an annoying variable.
	float dy = 0;
	// Create another handy dandy points array which includes their original index.
	size_t num_points = raw_num_points;
	indexed_point_t *points = malloc(sizeof(indexed_point_t) * num_points);
	for (size_t i = 0; i < num_points; i++) {
		points[i] = (indexed_point_t) {
			.vector = raw_points[i],
			.index  = i
		};
		dy = fmaxf(dy, -points[i].y);
	}
	// The annoy extendsm.
	dy *= 2;
	dy += 2;
	
	// The number of triangles is always 2 less than the number of points.
	size_t n_tris    = num_points - 2;
	int    tri_index = 0;
	size_t *tris     = malloc(sizeof(size_t) * n_tris * 3);
	if (!tris) {
		PAX_ERROR("pax_triangulate_shape", PAX_ERR_NOMEM);
	}
	// Find the funny ordering.
	bool clockwise = is_clockwise(num_points, points, 0, num_points, dy);
	
	// LOCATE all EARS conTINUousLY.
	for (size_t i = 0; i < n_tris; i++) {
		PAX_LOGW(TAG, "Tri %zd", i);
		// LOOK for an EAR.
		for (size_t i = 0; i < num_points; i++) {
			// bool attempt = is_clockwise3(num_points, points, i);
			bool attempt = is_clockwise(num_points, points, i, 3, dy);
			
			// It is an ear when the clockwisedness matches and the line does not intersect any of the source lines.
			bool is_ear = clockwise == attempt && !line_intersects_outline(raw_num_points, raw_points, points[i].vector, points[(i+2)%num_points].vector);
			if (is_ear) {
				// We found an EAR, now we CONVERT IT.
				tris[tri_index] = points[ i    % num_points].index;
				tri_index ++;
				tris[tri_index] = points[(i+1) % num_points].index;
				tri_index ++;
				tris[tri_index] = points[(i+2) % num_points].index;
				tri_index ++;
				
				// REMOVE the ear's CENTER POINT.
				int remove = (i+1) % num_points;
				int post   = num_points - remove - 1;
				// By means of MEMCPY.
				memcpy(&points[remove], &points[remove + 1], sizeof(indexed_point_t) * post);
				num_points --;
				// Now, we CONTINUE FINIDIGN ERA.
				break;
			}
		}
	}
	
	*output = tris;
}

// Draw a shape based on an outline.
// Closes the shape: no need to have the last point overlap the first.
void pax_draw_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec1_t *points) {
	// Simply outsource the triangulation.
	size_t *tris   = NULL;
	size_t  n_tris = num_points - 2;
	pax_triang_concave(&tris, num_points, points);
	if (!tris) {
		return;
	}
	// Then draw all triangles.
	for (size_t i = 0, tri_index = 0; i < n_tris; i++) {
		pax_draw_tri(
			buf, color,
			points[tris[tri_index  ]].x, points[tris[tri_index  ]].y,
			points[tris[tri_index+1]].x, points[tris[tri_index+1]].y,
			points[tris[tri_index+2]].x, points[tris[tri_index+2]].y
		);
		tri_index += 3;
	}
	// And free el triangles.
	free(tris);
}
#else
// Stub method because the real one isn't compiled in.
size_t pax_triang_complete(size_t **output, pax_vec1_t **additional_points, size_t num_points, pax_vec1_t *points) {
	PAX_ERROR1("pax_draw_shape", PAX_ERR_UNSUPPORTED, 0);
}
// Stub method because the real one isn't compiled in.
void pax_triang_concave(size_t **output, size_t num_points, pax_vec1_t *points) {
	PAX_ERROR("pax_draw_shape", PAX_ERR_UNSUPPORTED);
}
// Stub method because the real one isn't compiled in.
void pax_draw_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec1_t *points) {
	PAX_ERROR("pax_draw_shape", PAX_ERR_UNSUPPORTED);
}
#endif
