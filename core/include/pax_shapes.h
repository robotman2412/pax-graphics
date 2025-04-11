
// SPDX-License-Identifier: MIT

#ifndef PAX_SHAPES_H
#define PAX_SHAPES_H

#include "pax_types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


/* ============ CURVES =========== */

// Convert a cubic bezier curve to line segments, with the given number of points.
// From and to range from 0 to 1, but any value is accepted.
void pax_vectorise_bezier_part(pax_vec2f *output, size_t num_points, pax_4vec2f control_points, float from, float to);
// Convert a cubic bezier curve to line segments, with the given number of points.
void pax_vectorise_bezier(pax_vec2f *output, size_t num_points, pax_4vec2f control_points);
// Draw a cubic bezier curve.
// From and to range from 0 to 1, but any value is accepted.
void pax_draw_bezier_part(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points, float from, float to);
// Draw a cubic bezier curve.
void pax_draw_bezier(pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points);


/* =========== OUTLINES ========== */

// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// Does not close the shape: this must be done manually.
void pax_outline_shape_part(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, float from, float to
);
// Outline a shape defined by a list of points.
// Does not close the shape: this must be done manually.
void pax_outline_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points);
// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// When close is true, closes the shape; there is a line from the first to last point.
void pax_outline_shape_part_cl(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, bool close, float from, float to
);
// Outline a shape defined by a list of points.
// When close is true, closes the shape; there is a line from the first to last point.
void pax_outline_shape_cl(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, bool close);


/* ===== POLYGON MANIPULATION ==== */

// Transforms a list of points using a given 2D matrix.
// Overwrites the list's contents.
void pax_transform_shape(size_t num_points, pax_vec2f *points, matrix_2d_t matrix);
/*
// Planned for a future version, but not implemented yet.

// Rounds a polygon with a uniform radius applied to all corners.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape_uniform(pax_vec2f **output, size_t num_points, pax_vec2f *points, float radius);
// Rounds a polygon with a specific radius per corner.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape(pax_vec2f **output, size_t num_points, pax_vec2f *points, float *radii);
*/


/* ======== TRIANGULATION ======== */

/*
// Planned for a future version, but not implemented yet.

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
// Returns the number of additional points created.
size_t pax_triang_complete(size_t **output, pax_vec2f **additional_points, size_t num_points, pax_vec2f *points);
*/

// WARNING: Does not work for self-intersecting polygons.
//
// Triangulates a shape based on an outline (concave, non self-intersecting only).
// In effect, this creates triangles which completely fill the shape.
// Closes the shape: no need to have the last point overlap the first.
// Assumes the shape does not intersect itself.
//
// Stores triangles as triple-index pairs in output, which is a dynamically allocated size_t array.
// Returns the number of triangles created.
size_t pax_triang_concave(size_t **output, size_t num_points, pax_vec2f const *points);
// WARNING: Does not work for self-intersecting polygons.
//
// Draw a shape based on an outline.
// Closes the shape: no need to have the last point overlap the first.
void   pax_draw_shape(pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points);

// Draws a shape which has been previously triangulated.
// The number of triangles is num_points - 2.
void pax_draw_shape_triang(
    pax_buf_t *buf, pax_col_t color, size_t num_points, pax_vec2f const *points, size_t num_tris, size_t const *indices
);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // PAX_SHAPES_H
