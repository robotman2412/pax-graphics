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

#include "pax_internal.h"


/* ============ MATRIX =========== */

// 2D rotation matrix: represents a 2D shearing.
matrix_2d_t matrix_2d_rotate(float angle) {
	float _cos = cosf(-angle);
	float _sin = sinf(-angle);
	return (matrix_2d_t) { .arr = {
		_cos, -_sin, 0,
		_sin,  _cos, 0
	}};
}

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) {
	// [a b c] [p q r] [ap+bs aq+bt ar+bu+c]
	// [d e f]*[s t u]=[dp+es dq+et dr+eu+f]
	// [0 0 1] [0 0 1] [0     0     1      ]
	return (matrix_2d_t) { .arr = {
		a.a0*b.a0 + a.a1*b.b0,   a.a0*b.a1 + a.a1*b.b1,  a.a0*b.a2 + a.a1*b.b2 + a.a2,
		a.b0*b.a0 + a.b1*b.b0,   a.b0*b.a1 + a.b1*b.b1,  a.b0*b.a2 + a.b1*b.b2 + a.b2
	}};
}

// 2D matrix: applies the transformation that a represents on to a point.
void matrix_2d_transform(matrix_2d_t a, float *x, float *y) {
	// [a b c] [x]  [a]  [b] [c] [ax+by+c]
	// [d e f]*[y]=x[d]+y[e]+[f]=[dx+ey+f]
	// [0 0 1] [1]  [0]  [0] [1] [1      ]
	float _x = *x, _y = *y;
	*x = a.a0*_x + a.a1*_y + a.a2;
	*y = a.b0*_x + a.b1*_y + a.b2;
}

// 2D vector: unifies a given vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
pax_vec1_t vec1_unify(pax_vec1_t vec) {
	float magnitude = sqrtf(vec.x*vec.x + vec.y*vec.y);
	return (pax_vec1_t) {
		.x = vec.x / magnitude,
		.y = vec.y / magnitude
	};
}

// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a) {
	PAX_BUF_CHECK("pax_apply_2d");
	buf->stack_2d.value = matrix_2d_multiply(buf->stack_2d.value, a);
	PAX_SUCCESS();
}

// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_push_2d");
	matrix_stack_2d_t *parent = malloc(sizeof(matrix_stack_2d_t));
	if (!parent) PAX_ERROR("pax_push_2d", PAX_ERR_NOMEM);
	*parent = buf->stack_2d;
	buf->stack_2d.parent = parent;
	PAX_SUCCESS();
}

// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_pop_2d");
	matrix_stack_2d_t *parent = buf->stack_2d.parent;
	if (!parent) PAX_ERROR("pax_pop_2d", PAX_ERR_UNDERFLOW);
	buf->stack_2d = *parent;
	free(parent);
	PAX_SUCCESS();
}

// Reset the matrix stack.
// If full is true, the entire stack gets cleared.
// Else, only the top element gets cleared.
void pax_reset_2d(pax_buf_t *buf, bool full) {
	if (full) {
		matrix_stack_2d_t *current = buf->stack_2d.parent;
		while (current) {
			matrix_stack_2d_t *next = current->parent;
			free(current);
			current = next;
		}
		buf->stack_2d.parent = NULL;
	}
	buf->stack_2d.value = matrix_2d_identity();
}

