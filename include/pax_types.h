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

#ifndef PAX_TYPES_H
#define PAX_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pax_config.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif //M_PI

/* ========= ERROR DEFS ========== */

// Unknown error.
#define PAX_ERR_UNKNOWN      1
// All is good.
#define PAX_OK               0
// Buffer pointer is null.
#define PAX_ERR_NOBUF       -1
// Out of memory.
#define PAX_ERR_NOMEM       -2
// Invalid parameters.
#define PAX_ERR_PARAM       -3
// Infinite parameters.
#define PAX_ERR_INF         -4
// Out of bounds parameters.
#define PAX_ERR_BOUNDS      -5
// Matrix stack underflow.
#define PAX_ERR_UNDERFLOW   -6
// Out of data.
#define PAX_ERR_NODATA      -7
// Image decoding error.
#define PAX_ERR_DECODE      -8
// Unsupported operation (or not compiled in).
#define PAX_ERR_UNSUPPORTED -9

/* ============ TYPES ============ */

// More verbose way of saying reset only the top matrix.
#define PAX_RESET_TOP 0
// More verbose way of saying reset the whole matrix stack.
#define PAX_RESET_ALL 1

struct pax_vec3;
struct pax_vec4;
struct pax_rect;
union  matrix_2d;
struct matrix_stack_2d;
enum   pax_buf_type {
	PAX_BUF_1_PAL       = 0x20000001,
	PAX_BUF_2_PAL       = 0x20000002,
	PAX_BUF_4_PAL       = 0x20000004,
	PAX_BUF_8_PAL       = 0x20000008,
	PAX_BUF_16_PAL      = 0x20000008,
	
	PAX_BUF_1_GREY      = 0x10000001,
	PAX_BUF_2_GREY      = 0x10000202,
	PAX_BUF_4_GREY      = 0x10000004,
	PAX_BUF_8_GREY      = 0x10000008,
	
	PAX_BUF_8_332RGB    = 0x00033208,
	PAX_BUF_16_565RGB   = 0x00056510,
	
	PAX_BUF_4_1111ARGB  = 0x00111104,
	PAX_BUF_8_2222ARGB  = 0x00444408,
	PAX_BUF_16_4444ARGB = 0x00444410,
	PAX_BUF_32_8888ARGB = 0x00888820
};

struct pax_buf;
struct pax_shader;
// Type of task to do.
// Things like text and arcs will decompose to rects and triangles.
enum   pax_task_type {
	// Rectangle draw.
	PAX_TASK_RECT,
	// Triangle draw.
	PAX_TASK_TRI
};

struct pax_task;

typedef struct pax_vec1        pax_vec1_t;
typedef struct pax_vec2        pax_vec2_t;
typedef struct pax_vec3        pax_vec3_t;
typedef struct pax_vec4        pax_vec4_t;
typedef struct pax_vec3        pax_tri_t;
typedef struct pax_vec4        pax_quad_t;
typedef struct pax_rect        pax_rect_t;
typedef union  matrix_2d       matrix_2d_t;
typedef struct matrix_stack_2d matrix_stack_2d_t;
typedef enum   pax_buf_type    pax_buf_type_t;
typedef struct pax_buf         pax_buf_t;
typedef struct pax_shader      pax_shader_t;
typedef enum   pax_task_type   pax_task_type_t;
typedef struct pax_task        pax_task_t;

typedef int32_t               pax_err_t;
typedef uint32_t              pax_col_t;

// Function pointer for shader callback.
// Tint is the color parameter to the pax_shade_xxx function.
typedef pax_col_t (*pax_shader_func_t)(pax_col_t tint, int x, int y, float u, float v, void *args);
// Function pointer for transformer callback.
// It's job is to optionally move the triangle vertices.
typedef void (*pax_transf_func_t)(pax_tri_t *tri, pax_tri_t *uvs, void *args);

struct pax_vec1 {
	// Single point.
	float x, y;
};

struct pax_vec2 {
	// Line points.
	float x0, y0, x1, y1;
};

struct pax_vec3 {
	// Triangle points.
	float x0, y0, x1, y1, x2, y2;
};

struct pax_vec4 {
	// Quad points.
	float x0, y0, x1, y1, x2, y2, x3, y3;
};

struct pax_rect {
	// Reactangle points.
	float x, y, w, h;
};

union matrix_2d {
	struct {
		float a0, a1, a2;
		float b0, b1, b2;
	};
	float arr[6];
};

struct matrix_stack_2d {
	matrix_stack_2d_t *parent;
	matrix_2d_t        value;
};

struct pax_buf {
	// Buffer type, color modes, etc.
	pax_buf_type_t    type;
	// Whether to perform free on the buffer on deinit.
	bool              do_free;
	// Whether to reverse the endianness of the buffer.
	bool              reverse_endianness;
	union {
		// Shorthand for 8bpp buffer.
		uint8_t      *buf_8bpp;
		// Shorthand for 16bpp buffer.
		uint16_t     *buf_16bpp;
		// Shorthand for 32bpp buffer.
		uint32_t     *buf_32bpp;
		// Buffer pointer.
		void         *buf;
	};
	// Bits per pixel.
	int               bpp;
	// Pallette for buffers with a pallette type.
	pax_col_t        *pallette;
	// The number of colors in the pallette.
	size_t            pallette_size;
	
	// Width in pixels.
	int               width;
	// Height    in pixels.
	int               height;
	
	// Dirty x (top left).
	int               dirty_x0;
	// Dirty y (top left).
	int               dirty_y0;
	// Dirty x (bottom right).
	int               dirty_x1;
	// Dirty y (bottom right).
	int               dirty_y1;
	
	// Clip rect.
	pax_rect_t        clip;
	// Matrix stack.
	matrix_stack_2d_t stack_2d;
};

struct pax_shader {
	// Shader callback.
	pax_shader_func_t callback;
	// Shader arguments.
	void             *callback_args;
	// Whether to promise that an alpha of 0 in tint will return a fully transparent.
	bool              alpha_promise_0;
	// Whether to promise that an alpha of 255 in tint will return a fully opaque.
	bool              alpha_promise_255;
};

// A task to perform.
// Every task has pre-transformed co-ordinates.
// If you change the shader object's content (AKA the value that args points to),
// You should ran pax_join before making the change.
struct pax_task {
	// The buffer to apply this task to.
	pax_buf_t      *buffer;
	// The type of thing to do.
	pax_task_type_t type;
	// Color to use.
	pax_col_t       color;
	// Shader to use.
	pax_shader_t   *shader;
	// UVs to use.
	union {
		// UVs to use for rects and arcs.
		pax_quad_t *quad_uvs;
		// UVs to use for triangle.
		pax_tri_t  *tri_uvs;
	};
	// Additional parameters.
	// This is an array of floats for X, Y, and dimensions of shapes.
	float          *shape;
	// Number of floats in the shape array.
	size_t          shape_len;
};

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_TYPES_H
