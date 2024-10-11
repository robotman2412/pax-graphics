
// SPDX-License-Identifier: MIT

#include "pax_internal.h"


/* ============ MATRIX =========== */

// 2D rotation matrix: represents a 2D shearing.
matrix_2d_t matrix_2d_rotate(float angle) {
    float cos_res = cosf(-angle);
    float sin_res = sinf(-angle);
    return (matrix_2d_t){.arr = {cos_res, -sin_res, 0, sin_res, cos_res, 0}};
}

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) {
    // [a b c] [p q r] [ap+bs aq+bt ar+bu+c]
    // [d e f]*[s t u]=[dp+es dq+et dr+eu+f]
    // [0 0 1] [0 0 1] [0     0     1      ]
    return (matrix_2d_t
    ){.arr
      = {a.a0 * b.a0 + a.a1 * b.b0,
         a.a0 * b.a1 + a.a1 * b.b1,
         a.a0 * b.a2 + a.a1 * b.b2 + a.a2,
         a.b0 * b.a0 + a.b1 * b.b0,
         a.b0 * b.a1 + a.b1 * b.b1,
         a.b0 * b.a2 + a.b1 * b.b2 + a.b2}};
}

// 2D matrix: applies the transformation that a represents on to a point.
void matrix_2d_transform(matrix_2d_t a, float *x, float *y) {
    // [a b c] [x]  [a]  [b] [c] [ax+by+c]
    // [d e f]*[y]=x[d]+y[e]+[f]=[dx+ey+f]
    // [0 0 1] [1]  [0]  [0] [1] [1      ]
    float c_x = *x, c_y = *y;
    *x = a.a0 * c_x + a.a1 * c_y + a.a2;
    *y = a.b0 * c_x + a.b1 * c_y + a.b2;
}

// 2D matrix: applies the transformation that a represents on to a point.
pax_vec2f matrix_2d_transform_alt(matrix_2d_t a, pax_vec2f b) {
    return (pax_vec2f){
        a.a0 * b.x + a.a1 * b.y + a.a2,
        a.b0 * b.x + a.b1 * b.y + a.b2,
    };
}

// Convert the rectangle to one that covers the same area but with positive size.
pax_recti pax_recti_abs(pax_recti a) {
    if (a.w < 0) {
        a.x += a.w;
        a.w  = -a.w;
    }
    if (a.h < 0) {
        a.y += a.h;
        a.h  = -a.h;
    }
    return a;
}

// Convert the rectangle to one that covers the same area but with positive size.
pax_rectf pax_rectf_abs(pax_rectf a) {
    if (a.w < 0) {
        a.x += a.w;
        a.w  = -a.w;
    }
    if (a.h < 0) {
        a.y += a.h;
        a.h  = -a.h;
    }
    return a;
}

#define min_macro(a, b) ((a) < (b) ? (a) : (b))
#define max_macro(a, b) ((a) > (b) ? (a) : (b))
#define intersect_helper(pos, size, a, b)                                                                              \
    do {                                                                                                               \
        if ((a).pos + (a).size > (b).pos && (a).pos < (b).pos + (b).size) {                                            \
            (a).size  = min_macro((a).pos + (a).size, (b).pos + (b).size);                                             \
            (a).pos   = max_macro((a).pos, (b).pos);                                                                   \
            (a).size -= (a).pos;                                                                                       \
        } else {                                                                                                       \
            goto nope;                                                                                                 \
        }                                                                                                              \
    } while (0)

// Get the intersection between two rectangles.
// Returns {0, 0, 0, 0} if there is no intersection.
pax_recti pax_recti_intersect(pax_recti a, pax_recti b) {
    a = pax_recti_abs(a);
    b = pax_recti_abs(b);
    intersect_helper(x, w, a, b);
    intersect_helper(y, h, a, b);
    return a;
nope:
    return (pax_recti){0, 0, 0, 0};
}

// Get the intersection between two rectangles.
// Returns {0, 0, 0, 0} if there is no intersection.
pax_rectf pax_rectf_intersect(pax_rectf a, pax_rectf b) {
    a = pax_rectf_abs(a);
    b = pax_rectf_abs(b);
    intersect_helper(x, w, a, b);
    intersect_helper(y, h, a, b);
    return a;
nope:
    return (pax_rectf){0, 0, 0, 0};
}

// 2D vector: unifies a given vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
pax_vec2f pax_vec2f_unify(pax_vec2f vec) {
    float magnitude = sqrtf(vec.x * vec.x + vec.y * vec.y);
    return (pax_vec2f){.x = vec.x / magnitude, .y = vec.y / magnitude};
}



// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a) {
    PAX_BUF_CHECK(buf);
    buf->stack_2d.value = matrix_2d_multiply(buf->stack_2d.value, a);
}

// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf) {
    PAX_BUF_CHECK("pax_push_2d");
    matrix_stack_2d_t *parent = malloc(sizeof(matrix_stack_2d_t));
    if (!parent)
        PAX_ERROR(PAX_ERR_NOMEM);
    *parent              = buf->stack_2d;
    buf->stack_2d.parent = parent;
}

// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf) {
    PAX_BUF_CHECK(buf);
    matrix_stack_2d_t *parent = buf->stack_2d.parent;
    if (!parent)
        PAX_ERROR(PAX_ERR_UNDERFLOW);
    buf->stack_2d = *parent;
    free(parent);
}

// Reset the matrix stack.
// If full is true, the entire stack gets cleared.
// Else, only the top element gets cleared.
void pax_reset_2d(pax_buf_t *buf, bool full) {
    PAX_BUF_CHECK(buf);
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
