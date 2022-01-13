# PAX docs: drawing

The drawing methods are the core of PAX graphics.

Most drawing methods fall in to three categories:
- [Simple](#simple-drawing)
- [Normal](#normal-drawing)
- [Shaded](#shaded-drawing)

Other drawing methods include:
- [Text](#text-drawing)
- [Background](#background)

Each category had its own properties:
- Simple drawing does not apply [transformations](#matrices.md).
- Normal and simple drawing use a uniform color.
- Shaded drawing can have more than one color.

# Simple drawing

Simple drawing draws the given shape directly without applying [transformations](matrices.md).
It also has only one color for the entire shape.

List of simple drawing methods:
| name              | arguments                                                            | description
| :---------------- | :------------------------------------------------------------------- | :----------
| pax_simple_rect   | pax_buf_t \*buf, pax_col_t color, float x, y, width, height          | Draws a rectangle with the given dimensions.
| pax_simple_line   | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1               | Draws a line between two points.
| pax_simple_tri    | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1, x2, y2       | Draws a triangle between three points.
| pax_simple_arc    | pax_buf_t \*buf, pax_col_t color, float x, y, radius, angle0, angle1 | Draws an arc between two angles, at a given midpoint.
| pax_simple_circle | pax_buf_t \*buf, pax_col_t color, float x, y, radius                 | Draws a circle at a given midpoint.

# Normal drawing

Normal drawing draws shapes after applying [transformations](matrices.md).
It also has only one color for the entire shape.

List of normal drawing methods:
| name            | arguments                                                            | description
| :-------------- | :------------------------------------------------------------------- | :----------
| pax_draw_rect   | pax_buf_t \*buf, pax_col_t color, float x, y, width, height          | Draws a rectangle with the given dimensions.
| pax_draw_line   | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1               | Draws a line between two points.
| pax_draw_tri    | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1, x2, y2       | Draws a triangle between three points.
| pax_draw_arc    | pax_buf_t \*buf, pax_col_t color, float x, y, radius, angle0, angle1 | Draws an arc between two angles, at a given midpoint.
| pax_draw_circle | pax_buf_t \*buf, pax_col_t color, float x, y, radius                 | Draws a circle at a given midpoint.

# Shaded drawing

Shaded drawing draws shapes after applying [transformations](matrices.md).
It takes a shader and a UVs argument so that the shape may have multiple colors.
The UVs, final x and y position and color are passed to the shader to allow this.

There is no shaded version of a line.

List of shaded drawing methods:
| name             | arguments                                                                                                     | description
| :--------------- | :------------------------------------------------------------------------------------------------------------ | :----------
| pax_shade_rect   | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quad_t \*uvs, float x, y, width, height          | Draws a rectangle with the given dimensions.
| pax_shade_tri    | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_tri_t  \*uvs, float x0, y0, x1, y1, x2, y2       | Draws a triangle between three points.
| pax_shade_arc    | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quad_t \*uvs, float x, y, radius, angle0, angle1 | Draws an arc between two angles, at a given midpoint.
| pax_shade_circle | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quad_t \*uvs, float x, y, radius                 | Draws a circle at a given midpoint.

# Text drawing

Text drawing works like normal drawing, but with more characters.
In PAX, you can draw text as well as calculate it's size.

List of text methods:
| returns    | name          | arguments                                                                                           | description
| :--------- | :------------ | :-------------------------------------------------------------------------------------------------- | :----------
| void       | pax_draw_text | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font.
| pax_vec1_t | pax_text_size | pax_font_t \*font, float font_size, char \*text | Calculate the size of the string with the given font.

# Background
| name           | arguments                        | description
| :------------- | :------------------------------- | :----------
| pax_background | pax_buf_t \*buf, pax_col_t color | Fills the entire buffer with the given color.
