# PAX Docs: Matrices

PAX uses matrices to apply transformations.
Common examples of this are rotating images and scaling vector graphics.
Every `pax_draw_` or `pax_shade_` method applies transformations.

The matrix functions are split into two main parts:
- [The matrix stack](#matrix-stack)
- [Creating matrices](#types-of-matrices)

# Matrix stack

The matrix stack makes a powerful tool even more powerful.
In PAX, you can make the stack as big as will fit in memory.

Stack methods:
| name         | arguments                           | description
| :----------- | :---------------------------------- | :----------
| pax_apply_2d | pax_buf_t \*buf, matrix_2d_t matrix | Applies the given matrix to the stack. Combines the previous matrix with another transformation.
| pax_push_2d  | pax_buf_t \*buf                     | Duplicates the top matrix and pushes it to the stack.
| pax_pop_2d   | pax_buf_t \*buf                     | Removes the top matrix from the stack.
| pax_reset_2d | pax_buf_t \*buf, bool full          | Clears a part of or the entire stack. Use `PAX_RESET_TOP` (top matrix only) or `PAX_RESET_ALL` (clear everything) to specify what to clear.

If you need more explanation, [have a look at this](README.md#api-reference-matrix-transformations).

# Types of matrices

There are a few options for creating transformation matrices.

These include the built-in:
| returns     | name                | arguments   | description
| :---------- | :------------------ | :---------- | :----------
| matrix_2d_t | matrix_2d_identity  |             | Represents no transformation. Has no effect with `pax_apply_2d`.
| matrix_2d_t | matrix_2d_scale     | float x, y  | Scale everything by x and y.
| matrix_2d_t | matrix_2d_translate | float x, y  | Move everything with an offset x and y.
| matrix_2d_t | matrix_2d_shear     | float x, y  | Shear everything by x and y.
| matrix_2d_t | matrix_2d_rotate    | float angle | Rotate everything. Angles are in radians, positive is counter-clockwise.

The other option is to create one yourself.
The matrices are internally treated like a 3x3 matrix:
```c
matrix_2d_t my_matrix = (matrix_2d_t) { .arr = {
    1, 0, 0,
    0, 1, 0,
//  0, 0, 1  (implicit)
}};
```
The last (commented out) row is implicit and cannot be changed.
If you don't understand how creating a matrix manually works, you won't need to.

# Related functions

There are two more functions related to matrices which you can use.
These functions mostly used internally and you don't need to understand how to use them.

They are:
| returns     | name                | arguments                         | description
| :---------- | :------------------ | :-------------------------------- | :----------
| matrix_2d_t | matrix_2d_multiply  | matrix_2d_t a, b                  | Multiplies the matrix a by b.
| void        | matrix_2d_transform | matrix_2d_t a, float *x, float *y | Applies the transformation a to the point defined by x and y.
