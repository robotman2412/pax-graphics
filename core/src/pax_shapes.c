
// SPDX-License-Identifier: MIT

#include "pax_shapes.h"

#include "pax_internal.h"

#include <malloc.h>
#include <string.h>

static char const *TAG = "pax-shapes";

// This is only applicable during shape triangulation.
typedef struct indexed_point {
    union {
        struct {
            float x, y;
        };
        pax_vec2f vector;
    };
    size_t index;
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
static inline bezier_point_t
    pax_calc_bezier0(float part, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
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

    return (bezier_point_t){.x = x, .y = y, .part = part};
}

// Macro for calculating a point on a given bezier curve.
static inline bezier_point_t pax_calc_bezier(float part, pax_4vec2f ctl) {
    return pax_calc_bezier0(part, ctl.x0, ctl.y0, ctl.x1, ctl.y1, ctl.x2, ctl.y2, ctl.x3, ctl.y3);
}

/*
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
    float delta = a->part - b->part;
    return delta / fabs(delta);
}
*/



/* ============ CURVES =========== */

#if CONFIG_PAX_COMPILE_BEZIER

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to are from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(pax_vec2f *ptr, size_t max_points, pax_4vec2f control_points, float t_from, float t_to) {
    if (max_points < 4)
        PAX_ERROR(PAX_ERR_PARAM);
    if (!ptr) {
        PAX_ERROR(PAX_ERR_PARAM);
        return;
    }

    #if CONFIG_PAX_USE_EXPENSIVE_BEZIER

    // Start with just three points: start, T=0.5 and end.
    bezier_point_t *points = malloc(sizeof(bezier_point_t) * max_points);
    if (!points) {
        PAX_ERROR(PAX_ERR_NOMEM);
        return;
    }
    points[0]       = pax_calc_bezier(t_from, control_points);
    points[1]       = pax_calc_bezier((t_from + t_to) * 0.5, control_points);
    points[2]       = pax_calc_bezier(t_to, control_points);
    size_t n_points = 3;

    // Turn the points into lines.
    bezier_segment_t *lines = malloc(sizeof(bezier_segment_t) * (max_points - 1));
    if (!lines) {
        free(points);
        PAX_ERROR(PAX_ERR_NOMEM);
        return;
    }
    lines[0]       = (bezier_segment_t){.from = &points[0], .to = &points[1]};
    lines[1]       = (bezier_segment_t){.from = &points[1], .to = &points[2]};
    size_t n_lines = 2;

    // Bifurcate the longest line segment continuously.
    while (n_points < max_points) {
        // Find the segment to bifurcate.
        qsort(lines, n_lines, sizeof(bezier_segment_t), bezier_bifurcate_comp);
        float new_part   = (lines[0].from->part + lines[0].to->part) * 0.5;
        // Create a point at the average T.
        points[n_points] = pax_calc_bezier(new_part, control_points);
        // Bifurcate the line.
        lines[n_lines]   = (bezier_segment_t){.from = &points[n_points], .to = lines[0].to};
        lines[0].to      = &points[n_points];
        // The incrementing.
        n_points++;
        n_lines++;
    }

    // Sort points by T value.
    qsort(points, n_points, sizeof(bezier_point_t), bezier_point_t_comp);
    // Output our GARBAGE.
    for (size_t i = 0; i < n_points; i++) {
        ptr[i] = (pax_vec2f){.x = points[i].x, .y = points[i].y};
    }
    free(points);
    free(lines);

    #else

    float delta = (t_to - t_from) / (max_points - 1);
    float part  = t_from;
    for (size_t i = 0; i < max_points; i++) {
        bezier_point_t point  = pax_calc_bezier(part, control_points);
        ptr[i]                = (pax_vec2f){point.x, point.y};
        part                 += delta;
    }

    #endif
}

// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec2f *output, size_t max_points, pax_4vec2f control_points) {
    pax_vectorise_bezier_part(output, max_points, control_points, 0, 1);
}

// Draw a cubic bezier curve.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points, float from, float to) {
    size_t n_points = 64;
    if (to < from) {
        PAX_SWAP(float, to, from);
    }
    #if CONFIG_PAX_USE_EXPENSIVE_BEZIER
    // Vectorise the bezier curve first.
    pax_vec2f *points;
    pax_vectorise_bezier(&points, n_points, control_points);
    if (!points)
        return;

    // Then draw it.
    for (size_t i = 0; i < n_points - 1; i++) {
        pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
    }
    free(points);
    #else
    // Draw the curve in a more simple manner.
    bezier_point_t last_point = pax_calc_bezier(from, control_points);
    float          delta      = (to - from) / (n_points - 2);
    float          part       = from;
    for (size_t i = 0; i < n_points - 1; i++) {
        bezier_point_t point = pax_calc_bezier(part, control_points);
        pax_draw_line(buf, color, last_point.x, last_point.y, point.x, point.y);
        last_point  = point;
        part       += delta;
    }
    #endif
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points) {
    pax_draw_bezier_part(buf, color, control_points, 0, 1);
}

#else

static bool bezier_warning = false;
static void pax_bezier_warn() {
    if (!bezier_warning) {
        PAX_LOGE(TAG, "Failed: Bezier curves not compiled, please define CONFIG_PAX_COMPILE_BEZIER.");
    }
}

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to are from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(
    pax_vec2f **output, pax_4vec2f control_points, size_t max_points, float t_from, float t_to
) {
    // Not compiled in, but keep the method for API compatibility.
    pax_bezier_warn();
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}

// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec2f **output, pax_4vec2f control_points, size_t max_points) {
    // Not compiled in, but keep the method for API compatibility.
    pax_bezier_warn();
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}

// Draw a cubic bezier curve.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points, float from, float to) {
    // Not compiled in, but keep the method for API compatibility.
    pax_bezier_warn();
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}

// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points) {
    // Not compiled in, but keep the method for API compatibility.
    pax_bezier_warn();
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}

#endif // CONFIG_PAX_COMPILE_BEZIER



/* =========== OUTLINES ========== */

// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// Does not close the shape: this must be done manually.
void pax_outline_shape_part(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, float from, float to
) {
    pax_outline_shape_part_cl(buf, color, num_points, points, false, from, to);
}

// Outline a shape defined by a list of points.
// Does not close the shape: this must be done manually.
void pax_outline_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points) {
    for (size_t i = 0; i < num_points - 1; i++) {
        pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
    }
}

// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// Closes the shape: there is a line from the first to last point.
void pax_outline_shape_part_cl(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, bool close, float from, float to
) {
    // El simplify.
    if (to < from) {
        PAX_SWAP(float, to, from);
    }
    if (from <= 0 && to >= 1) {
        pax_outline_shape(buf, color, num_points, points);
        return;
    }

    // Calculate total distance.
    float *dist       = malloc(sizeof(float) * num_points);
    float  total_dist = 0;
    float  start_dist = 0;
    if (!dist) {
        PAX_ERROR(PAX_ERR_NOMEM);
    }
    for (size_t i = 0; i < num_points - 1; i++) {
        float dx    = points[i + 1].x - points[i].x;
        float dy    = points[i + 1].y - points[i].y;
        dist[i]     = sqrtf(dx * dx + dy * dy);
        total_dist += dist[i];
    }
    // Count the returning line if the shape is closed.
    if (close && num_points >= 2) {
        float dx              = points[num_points - 1].x - points[0].x;
        float dy              = points[num_points - 1].y - points[0].y;
        dist[num_points - 1]  = sqrtf(dx * dx + dy * dy);
        total_dist           += dist[num_points - 1];
    }

    // Do distance calculations.
    start_dist  = total_dist * from;
    total_dist *= to;

    // Draw until the maximum length is reached.
    for (size_t i = 0; i < num_points - !close; i++) {
        size_t i1 = (i + 1) % num_points;
        if (start_dist > dist[i]) {
            // Skip the line segment.
        } else if (start_dist > 0) {
            float dx    = points[i1].x - points[i].x;
            float dy    = points[i1].y - points[i].y;
            float part0 = start_dist / dist[i];
            if (total_dist > dist[i]) {
                // Draw the end of a segment.
                pax_draw_line(
                    buf,
                    color,
                    points[i].x + dx * part0,
                    points[i].y + dy * part0,
                    points[i1].x,
                    points[i1].y
                );
            } else {
                // Draw the middle of a segment.
                float part1 = total_dist / dist[i];
                pax_draw_line(
                    buf,
                    color,
                    points[i].x + dx * part0,
                    points[i].y + dy * part0,
                    points[i].x + dx * part1,
                    points[i].y + dy * part1
                );
            }
        } else if (dist[i] < total_dist) {
            // Draw the entire segment.
            pax_draw_line(buf, color, points[i].x, points[i].y, points[i1].x, points[i1].y);
        } else {
            // Draw the start of a segment.
            float dx   = points[i1].x - points[i].x;
            float dy   = points[i1].y - points[i].y;
            float part = total_dist / dist[i];
            pax_draw_line(buf, color, points[i].x, points[i].y, points[i].x + dx * part, points[i].y + dy * part);
            break;
        }
        total_dist -= dist[i];
        start_dist -= dist[i];
    }

    free(dist);
}

// Outline a shape defined by a list of points.
// Closes the shape: there is a line from the first to last point.
void pax_outline_shape_cl(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, bool close) {
    for (size_t i = 0; i < num_points - 1; i++) {
        pax_draw_line(buf, color, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
    }
    if (close && num_points >= 2) {
        pax_draw_line(buf, color, points[0].x, points[0].y, points[num_points - 1].x, points[num_points - 1].y);
    }
}



/* ===== POLYGON MANIPULATION ==== */

// Transforms a list of points using a given 2D matrix.
// Overwrites the list's contents.
void pax_transform_shape(size_t num_points, pax_vec2f *points, matrix_2d_t matrix) {
    for (size_t i = 0; i < num_points; i++) {
        matrix_2d_transform(matrix, &points[i].x, &points[i].y);
    }
}

/*
// Rounds a polygon with a uniform radius applied to all corners.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape_uniform(pax_vec2f **output, size_t num_points, pax_vec2f *points, float radius) {
    // Fill out an array with the same radius.
    float *radii = malloc(sizeof(float) * num_points);
    if (!radii) {
        PAX_ERROR(PAX_ERR_NOMEM, 0);
    }
    for (size_t i = 0; i < num_points; i++) {
        radii[i] = radius;
    }

    // Just use the more specific operation with simpler parameters.
    size_t n_out = pax_round_shape(output, num_points, points, radii);
    free(radii);
    return n_out;
}

// Rounds a polygon with a specific radius per corner.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape(pax_vec2f **output, size_t num_points, pax_vec2f *points, float *radii) {
}
*/



/* ======== TRIANGULATION ======== */

#if CONFIG_PAX_COMPILE_TRIANGULATE
// Determine whether the points go clockwise or counter-clockwise.
// Does not work for less then 3 points.
static bool is_clockwise(int num_points, indexed_point_t *points, int index, int num_test, float dy) {
    float result = 0;

    // Simple but unoptimised loop.
    for (int i = 0; i < num_test; i++) {
        int index0  = i + index;
        int index1  = (i + 1) % num_test + index;
        index0     %= num_points;
        index1     %= num_points;
        result     += (points[index1].x - points[index0].x) * (points[index1].y + points[index0].y + dy);
    }

    // Clockwise if result < 0.
    return result < 0;
}

// Gets the slope of a line.
// Returns +/- infinity for vertical lines.
static inline float line_slope(pax_2vec2f line) {
    return (line.y1 - line.y0) / (line.x1 - line.x0);
}

// Creates a bounding rectangle for a line.
static pax_rectf line_bounding_box(pax_2vec2f line) {
    // Create a simple bounding box.
    pax_rectf box = {
        .x = line.x0,
        .y = line.y0,
        .w = line.x1 - line.x0,
        .h = line.y1 - line.y0,
    };

    // Fix width/height so they are positive.
    if (box.w < 0) {
        box.x += box.w;
        box.w  = -box.w;
    }
    if (box.h < 0) {
        box.y += box.h;
        box.h  = -box.h;
    }

    return box;
}

// Determines whether a point is in the bounding box, but not on it's edge.
static inline bool bounding_box_contains(pax_rectf box, pax_vec2f point) {
    if (box.w == 0 && box.h == 0) {
        return point.x == box.x && point.x == box.y;
    } else if (box.w == 0) {
        return point.x >= box.x && point.y > box.y && point.x <= box.x + box.w && point.y < box.y + box.h;
    } else if (box.h == 0) {
        return point.x > box.x && point.y >= box.y && point.x < box.x + box.w && point.y <= box.y + box.h;
    } else {
        return point.x > box.x && point.y > box.y && point.x < box.x + box.w && point.y < box.y + box.h;
    }
}

// Tests whether lines A and B intersect.
// Does not consider touching lines to intersect.
static bool line_intersects_line(pax_2vec2f line_a, pax_2vec2f line_b, pax_vec2f *intersection) {
    // If slopes are equal, then it will never intersect.
    float rc_a = line_slope(line_a);
    float rc_b = line_slope(line_b);
    if (rc_a == rc_b || (isinf(rc_a) && isinf(rc_b)))
        return false;

    // Determine b in Y=a*X+b line formulas.
    float dy_a = line_a.y0 - rc_a * line_a.x0;
    float dy_b = line_b.y0 - rc_b * line_b.x0;

    // Determine bounding boxes.
    pax_rectf box_a = line_bounding_box(line_a);
    pax_rectf box_b = line_bounding_box(line_b);

    // Special cases for one of two lines is vertical.
    if (isinf(rc_a)) {
        float y = rc_b * line_a.x0 + dy_b;
        if (y > box_a.y && y < box_a.y + box_a.h) {
            if (intersection) {
                *intersection = (pax_vec2f){box_a.x, y};
            }
            return true;
        }
    }
    if (isinf(rc_b)) {
        float y = rc_a * line_b.x0 + dy_a;
        if (y > box_b.y && y < box_b.y + box_b.h) {
            if (intersection) {
                *intersection = (pax_vec2f){box_b.x, y};
            }
            return true;
        }
    }

    // Find the intersection point, assuming infinitely long lines.
    float x = (dy_b - dy_a) / (rc_a - rc_b);
    float y = x * rc_a + dy_a;

    // If this lies within both bounding boxes, the lines intersect.
    bool intersects
        = bounding_box_contains(box_a, (pax_vec2f){x, y}) && bounding_box_contains(box_b, (pax_vec2f){x, y});
    if (intersects && intersection) {
        *intersection = (pax_vec2f){x, y};
    }
    return intersects;
}

// Tests whether a line intersects any of the lines in the dataset.
// Intersection is NOT counted when only the end points touch.
static bool line_intersects_outline(size_t num_points, pax_vec2f const *raw_points, pax_vec2f start, pax_vec2f end) {
    for (size_t i = 0; i < num_points; i++) {
        size_t index1 = (i + 1) % num_points;
        if (line_intersects_line(
                (pax_2vec2f){start.x, start.y, end.x, end.y},
                (pax_2vec2f){raw_points[i].x, raw_points[i].y, raw_points[index1].x, raw_points[index1].y},
                NULL
            ))
            return true;
    }
    return false;
}

/*
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
size_t pax_triang_complete(size_t **output, pax_vec2f **additional_points, size_t num_points, pax_vec2f *points) {
}
*/

// Triangulates a shape based on an outline (concave, non self-intersecting only).
// In effect, this creates triangles which completely fill the shape.
// Closes the shape: no need to have the last point overlap the first.
// Assumes the shape does not intersect itself.
//
// Stores triangles as triple-index pairs in output, which is a dynamically allocated size_t array.
// Returns the number of triangles created.
size_t pax_triang_concave(size_t **output, size_t raw_num_points, pax_vec2f const *raw_points) {
    // Cannot triangulate with less than 3 points.
    if (raw_num_points < 3) {
        *output = NULL;
        return 0;
    }

    // Find an annoying variable.
    float            dy         = 0;
    // Create another handy dandy points array which includes their original index.
    size_t           num_points = raw_num_points;
    indexed_point_t *points     = malloc(sizeof(indexed_point_t) * num_points);
    if (points == NULL) {
        *output = NULL;
        return 0;
    }

    for (size_t i = 0; i < num_points; i++) {
        points[i] = (indexed_point_t){.vector = raw_points[i], .index = i};
        dy        = fmaxf(dy, -points[i].y);
    }
    // The annoy extendsm.
    dy *= 2;
    dy += 2;

    // The number of triangles is always 2 less than the number of points.
    size_t  n_tris    = num_points - 2;
    size_t  tri_index = 0;
    size_t *tris      = malloc(sizeof(size_t) * n_tris * 3);
    if (!tris) {
        PAX_LOGE(TAG, "Out of memory for triangulation!");
        *output = NULL;
        return 0;
    }
    // Find the funny ordering.
    bool clockwise = is_clockwise(num_points, points, 0, num_points, dy);

    // LOCATE all EARS conTINUousLY.
    for (size_t i = 0; i < n_tris; i++) {
        // LOOK for an EAR.
        for (size_t i = 0; i < num_points; i++) {
            // bool attempt = is_clockwise3(num_points, points, i);
            bool attempt = is_clockwise(num_points, points, i, 3, dy);

            // It is an ear when the clockwisedness matches and the line does not intersect any of the source lines.
            bool is_ear = clockwise == attempt
                          && !line_intersects_outline(
                              raw_num_points,
                              raw_points,
                              points[i].vector,
                              points[(i + 2) % num_points].vector
                          );
            if (is_ear) {
                // We found an EAR, now we CONVERT IT.
                tris[tri_index] = points[i % num_points].index;
                tri_index++;
                tris[tri_index] = points[(i + 1) % num_points].index;
                tri_index++;
                tris[tri_index] = points[(i + 2) % num_points].index;
                tri_index++;

                // REMOVE the ear's CENTER POINT.
                int remove = (i + 1) % num_points;
                int post   = num_points - remove - 1;
                // By means of MEMCPY.
                memcpy(&points[remove], &points[remove + 1], sizeof(indexed_point_t) * post);
                num_points--;
                // Now, we CONTINUE FINIDIGN ERA.
                break;
            }
        }
    }

    // Did we find everything?
    if (tri_index < n_tris * 3) {
        // No; abort.
        PAX_LOGE(TAG, "Cannot handle shape for triangulation!");
        free(tris);
        *output = NULL;
        return 0;

    } else {
        // Yes.
        *output = tris;
        return n_tris;
    }
}

// Draws a shape which has been previously triangulated.
// The number of triangles is num_points - 2.
void pax_draw_shape_triang(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, size_t n_tris, size_t const *tris
) {
    // TODO: The num_points parameter was intended to check bounds but never implemented. Remove?
    (void)num_points;
    // Then draw all triangles.
    for (size_t i = 0, tri_index = 0; i < n_tris; i++) {
        pax_draw_tri(
            buf,
            color,
            points[tris[tri_index]].x,
            points[tris[tri_index]].y,
            points[tris[tri_index + 1]].x,
            points[tris[tri_index + 1]].y,
            points[tris[tri_index + 2]].x,
            points[tris[tri_index + 2]].y
        );
        tri_index += 3;
    }
}

// Draw a shape based on an outline.
// Closes the shape: no need to have the last point overlap the first.
void pax_draw_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points) {
    // Simply outsource the triangulation.
    size_t *tris   = NULL;
    size_t  n_tris = pax_triang_concave(&tris, num_points, points);
    if (!tris) {
        return;
    }
    pax_draw_shape_triang(buf, color, num_points, points, n_tris, tris);
    // And free el triangles.
    free(tris);
}
#else
// Stub method because the real one isn't compiled in.
size_t pax_triang_complete(size_t **output, pax_vec2f **additional_points, size_t num_points, pax_vec2f *points) {
    PAX_ERROR(PAX_ERR_UNSUPPORTED, 0);
}
// Stub method because the real one isn't compiled in.
void pax_triang_concave(size_t **output, size_t num_points, pax_vec2f *points) {
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}
// Stub method because the real one isn't compiled in.
void pax_draw_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f *points) {
    PAX_ERROR(PAX_ERR_UNSUPPORTED);
}
#endif
