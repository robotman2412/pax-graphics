/*
	MIT License

	Copyright (c) 2021 Julian Scheffers

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

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>
#include <math.h>
#define PAX_AUTOREPORT

#ifndef M_PI
#define M_PI 3.141592653589793
#endif //M_PI

/* ====== ERROR DEFS ===== */

// Unknown error.
#define PAX_ERR_UNKNOWN    1
// All is good.
#define PAX_OK             0
// Buffer pointer is null.
#define PAX_ERR_NOBUF     -1
// Out of memory.
#define PAX_ERR_NOMEM     -2
// Invalid parameters.
#define PAX_ERR_PARAM     -3
// Infinite parameters.
#define PAX_ERR_INF       -4
// Out of bounds parameters.
#define PAX_ERR_BOUNDS    -5
// Matrix stack underflow.
#define PAX_ERR_UNDERFLOW -6

/* ============ TYPES ============ */

struct pax_tri;
struct pax_rect;
union  matrix_2d;
struct matrix_stack_2d;
enum   pax_buf_type;
struct pax_buf;

typedef struct pax_tri         pax_tri_t;
typedef struct pax_rect        pax_rect_t;
typedef union  matrix_2d       matrix_2d_t;
typedef struct matrix_stack_2d matrix_stack_2d_t;
typedef enum   pax_buf_type    pax_buf_type_t;
typedef struct pax_buf         pax_buf_t;

typedef int32_t               pax_err_t;
typedef uint32_t              pax_col_t;

struct pax_tri {
	// Triangle points.
	float x0, y0, x1, y1, x2, y2;
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

enum pax_buf_type {
	PAX_BUF_16_565RGB   = 0x00056510,
	PAX_BUF_32_8888ARGB = 0x00888820
};

struct pax_buf {
	// Buffer type, color modes, etc.
	pax_buf_type_t    type;
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

#ifdef __cplusplus
}
#endif //__cplusplus
