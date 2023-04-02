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
void pax_vectorise_bezier     (pax_vec2f *output, size_t num_points, pax_4vec2f control_points);
// Draw a cubic bezier curve.
// From and to range from 0 to 1, but any value is accepted.
void pax_draw_bezier_part     (pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points, float from, float to);
// Draw a cubic bezier curve.
void pax_draw_bezier          (pax_buf_t *buf, pax_col_t color, pax_4vec2f control_points);

/* ============= ARCS ============ */

// Vectorise an arc outline, angle in radians.
void pax_vectorise_arc        (pax_vec2f *output, size_t num_points, float x, float y, float r, float a0, float a1);
// Vectorise a circle outline.
void pax_vectorise_circle     (pax_vec2f *output, size_t num_points, float x, float y, float r);

/* ======= OUTLINE EDITIONS ====== */

// Draw a rectangle outline.
void pax_outline_rect         (pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height);
// Draw a triangle.
void pax_outline_tri          (pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw an arc outline, angle in radians.
void pax_outline_arc          (pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1);
// Draw a circle outline.
void pax_outline_circle       (pax_buf_t *buf, pax_col_t color, float x, float y, float r);

// Draw a rectangle outline with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_rect   (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, float y, float width, float height);
// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_tri    (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
// Draw an arc outline with a shader, angle in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_arc    (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, float y, float r, float a0, float a1);
// Draw a circle outline with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_outline_circle (pax_buf_t *buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, float y, float r);

/* =========== OUTLINES ========== */

// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// Does not close the shape: this must be done manually.
void pax_outline_shape_part   (pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points, float from, float to);
// Outline a shape defined by a list of points.
// Does not close the shape: this must be done manually.
void pax_outline_shape        (pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points);
// Partially outline a shape defined by a list of points.
// From and to range from 0 to 1, outside this range is ignored.
// When close is true, closes the shape; there is a line from the first to last point.
void pax_outline_shape_part_cl(pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points, bool close, float from, float to);
// Outline a shape defined by a list of points.
// When close is true, closes the shape; there is a line from the first to last point.
void pax_outline_shape_cl     (pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points, bool close);

/* ===== POLYGON MANIPULATION ==== */

// Transforms a list of points using a given 2D matrix.
// Overwrites the list's contents.
void pax_transform_shape(size_t num_points, pax_vec2f *points, matrix_2d_t matrix);
// WARNING: This is a beta feature and it does not work!
// 
// Rounds a polygon with a uniform radius applied to all corners.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape_uniform(pax_vec2f **output, size_t num_points, pax_vec2f *points, float radius);
// WARNING: This is a beta feature and it does not work!
// 
// Rounds a polygon with a specific radius per corner.
// Each corner can be rounded up to 50% of the edges it is part of.
// Capable of dealing with self-intersecting shapes.
// Returns the amount of points created.
size_t pax_round_shape(pax_vec2f **output, size_t num_points, pax_vec2f *points, float *radii);

/* ======== TRIANGULATION ======== */

// WARNING: This is a beta feature and it does not work!
// 
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
// WARNING: Does not work for self-intersecting polygons.
// 
// Triangulates a shape based on an outline (concave, non self-intersecting only).
// In effect, this creates triangles which completely fill the shape.
// Closes the shape: no need to have the last point overlap the first.
// Assumes the shape does not intersect itself.
//
// Stores triangles as triple-index pairs in output, which is a dynamically allocated size_t array.
// Returns the number of triangles created.
size_t pax_triang_concave     (size_t **output, size_t num_points, const pax_vec2f *points);
// WARNING: Does not work for self-intersecting polygons.
// 
// Draw a shape based on an outline.
// Closes the shape: no need to have the last point overlap the first.
void pax_draw_shape           (pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points);

// Draws a shape which has been previously triangulated.
// The number of triangles is num_points - 2.
void pax_draw_shape_triang    (pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points, size_t num_tris, const size_t *indices);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_SHAPES_H
