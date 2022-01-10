# PAX graphics
An MIT graphics library for the MCH2022 badge firmware
ESP-IDF module

# API cheat sheet
| name | returns | args | desc
| :--- | :------ | :--- | :---
| pax_buf_init | void | buf, mem, width, height, type | Create a new buffer.<br>If mem is NULL, a new area is allocated.
| pax_buf_destroy | void | pax_buf_t  \*buf | Destroy the buffer, freeing its memory.
| pax_buf_convert | void | pax_buf_t  \*dst, pax_buf_t  \*src, pax_buf_type_t type | Convert the buffer to the given new format.<br>If dest is NULL or equal to src, src will be converted.
| pax_clip | void | pax_buf_t  \*buf, float x, float y, float width, float height | Clip the buffer to the desired rectangle.
| pax_noclip | void | pax_buf_t  \*buf | Clip the buffer to it's full size.
| pax_is_dirty | bool | pax_buf_t  \*buf | Check whether the buffer is dirty.
| pax_mark_clean | void | pax_buf_t  \*buf | Mark the entire buffer as clean.
| pax_mark_dirty0 | void | pax_buf_t  \*buf | Mark the entire buffer as dirty.
| pax_mark_dirty1 | void | pax_buf_t  \*buf, int x, int y | Mark a single point as dirty.
| pax_mark_dirty2 | void | pax_buf_t  \*buf, int x, int y, int width, int height | Mark a rectangle as dirty.
| pax_col_rgb | pax_col_t | uint8_t r, uint8_t g, uint8_t b | Combines RGB.
| pax_col_argb | pax_col_t | uint8_t a, uint8_t r, uint8_t g, uint8_t b | Combines ARGB.
| pax_col_hsv | pax_col_t | uint8_t h, uint8_t s, uint8_t v | Converts HSV to ARGB.
| pax_col_lerp | pax_col_t | uint8_t part, pax_col_t from, pax_col_t to | Linearly interpolates between from and to, including alpha.
| pax_col_merge | pax_col_t | pax_col_t base, pax_col_t top | Merges the two colors, based on alpha.
| matrix_2d_is_identity | bool | matrix_2d_t m | Check whether the matrix exactly equals the identity matrix.
| matrix_2d_is_identity1 | bool | matrix_2d_t m | Check whether the matrix represents no more than a translation.
| matrix_2d_is_identity2 | bool | matrix_2d_t m | Check whether the matrix represents no more than a translation and/or scale.
| matrix_2d_identity | matrix_2d_t | | 2D identity matrix: represents no transformation.
| matrix_2d_scale | matrix_2d_t | float x, float y | 2D scale matrix: represents a 2D scaling.
| matrix_2d_translate | matrix_2d_t | float x, float y | 2D translation matrix: represents a 2D movement of the camera.
| matrix_2d_shear | matrix_2d_t | float x, float y | 2D shear matrix: represents a 2D shearing.
| matrix_2d_rotate | matrix_2d_t | float angle | 2D rotation matrix: represents a 2D rotation.
| matrix_2d_multiply | matrix_2d_t | matrix_2d_t a, matrix_2d_t b | 2D matrix: applies the transformation that b represents on to a.
| matrix_2d_transform | void | matrix_2d_t a, float  \*x, float  \*y | 2D matrix: applies the transformation that a represents on to a point.
| pax_apply_2d | void | pax_buf_t  \*buf, matrix_2d_t a | Apply the given matrix to the stack.
| pax_push_2d | void | pax_buf_t  \*buf | Push the current matrix up the stack.
| pax_pop_2d | void | pax_buf_t  \*buf | Pop the top matrix off the stack.
| pax_merge_pixel | void | pax_buf_t  \*buf, pax_col_t color, int x, int y | Set a pixel, merging with alpha.
| pax_set_pixel | void | pax_buf_t  \*buf, pax_col_t color, int x, int y | Set a pixel.
| pax_get_pixel | pax_col_t | pax_buf_t  \*buf, int x, int y | Get a pixel.
| pax_shade_rect | void | pax_buf_t  \*buf, pax_col_t color, pax_shader_t  \*shader, pax_quad_t  \*uvs, float x, float y, float width, float height | Draw a rectangle with a shader.<br>If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
| pax_shade_tri | void | pax_buf_t  \*buf, pax_col_t color, pax_shader_t  \*shader, pax_tri_t  \*uvs, float x0, float y0, float x1, float y1, float x2, float y2 | Draw a triangle with a shader.<br>If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
| pax_shade_arc | void | pax_buf_t  \*buf, pax_col_t color, pax_shader_t  \*shader, pax_quad_t  \*uvs, float x, float y, float r, float a0, float a1 | Draw an arc with a shader, angles in radians.<br>If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
| pax_shade_circle | void | pax_buf_t  \*buf, pax_col_t color, pax_shader_t  \*shader, pax_quad_t  \*uvs, float x, float y, float r | Draw a circle with a shader.<br>If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
| pax_draw_rect | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float width, float height | Draw a rectangle.
| pax_draw_line | void | pax_buf_t  \*buf, pax_col_t color, float x0, float y0, float x1, float y1 | Draw a line.
| pax_draw_tri | void | pax_buf_t  \*buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2 | Draw a triangle.
| pax_draw_arc | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float r, float a0, float a1 | Draw an arc, angles in radians.
| pax_draw_circle | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float r | Draw a circle.
| pax_draw_text | void | pax_buf_t  \*buf, pax_col_t color, pax_font_t  \*font, float font_size, float x, float y, char  \*text | Draw a string with the given font.<br>If font is NULL, the default font (7x9) will be used.
| pax_text_size | pax_vec1_t | pax_font_t  \*font, float font_size, char  \*text | Calculate the size of the string with the given font.<br>Size is before matrix transformation.<br>If font is NULL, the default font (7x9) will be used.
| pax_background | void | pax_buf_t  \*buf, pax_col_t color | Fill the background.
| pax_simple_rect | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float width, float height | Draw a rectangle, ignoring matrix transform.
| pax_simple_line | void | pax_buf_t  \*buf, pax_col_t color, float x0, float y0, float x1, float y1 | Draw a line, ignoring matrix transform.
| pax_simple_tri | void | pax_buf_t  \*buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2 | Draw a triangle, ignoring matrix transform.
| pax_simple_arc | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float r, float a0, float a1 | Draw na arc, ignoring matrix transform.<br>Angles in radians.
| pax_simple_circle | void | pax_buf_t  \*buf, pax_col_t color, float x, float y, float r | Draw a circle, ignoring matrix transform.

# See also
- https://github.com/robotman2412/mch2022-badge-tests
- https://mch2022.org/
- https://badge.team/
