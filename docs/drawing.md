# PAX docs: Drawing

The drawing methods are the core of PAX graphics.

Most drawing methods fall in to three categories:
- [Simple](#simple-drawing)
- [Normal](#normal-drawing)
- [Shaded](#shaded-drawing)

Other drawing methods include:
- [Text](#text-drawing)
- [Background](#background)

Each category had its own properties:
- Simple drawing does not apply [transformations](matrices.md).
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
| name                 | arguments                                                            | description
| :------------------- | :------------------------------------------------------------------- | :----------
| pax_draw_image       | pax_buf_t \*buf, pax_buf_t *image, float x, y                        | Draws an image at the image's normal size.
| pax_draw_image_sized | pax_buf_t \*buf, pax_buf_t *image, float x, y, width, height         | Draw an image with a prespecified size.
| pax_draw_rect        | pax_buf_t \*buf, pax_col_t color, float x, y, width, height          | Draws a rectangle with the given dimensions.
| pax_draw_line        | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1               | Draws a line between two points.
| pax_draw_tri         | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1, x2, y2       | Draws a triangle between three points.
| pax_draw_arc         | pax_buf_t \*buf, pax_col_t color, float x, y, radius, angle0, angle1 | Draws an arc between two angles, at a given midpoint.
| pax_draw_circle      | pax_buf_t \*buf, pax_col_t color, float x, y, radius                 | Draws a circle at a given midpoint.

# Outline drawing

Like normal drawing, but only draws the outline of a shape.

List of outline drawing methods:
| name               | arguments                                                            | description
| :----------------- | :------------------------------------------------------------------- | :----------
| pax_outline_rect   | pax_buf_t \*buf, pax_col_t color, float x, y, width, height          | Outlines a rectangle with the given dimensions.
| pax_outline_tri    | pax_buf_t \*buf, pax_col_t color, float x0, y0, x1, y1, x2, y2       | Outlines a triangle between three points.
| pax_outline_arc    | pax_buf_t \*buf, pax_col_t color, float x, y, radius, angle0, angle1 | Outlines an arc between two angles, at a given midpoint. Does not create lines to the center.
| pax_outline_circle | pax_buf_t \*buf, pax_col_t color, float x, y, radius                 | Outlines a circle at a given midpoint.

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

Text drawing works like normal drawing, but with more characters and UTF-8 support.
In PAX, you can draw text as well as calculate it's size.

List of text methods:
| returns    | name               | arguments                                                                                           | description
| :--------- | :----------------- | :-------------------------------------------------------------------------------------------------- | :----------
| void       | pax_draw_text      | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Interpolation depends on the font.
| void       | pax_draw_text_aa   | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Always uses interpolation.
| void       | pax_draw_text_noaa | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Never uses interpolation.
| pax_vec1_t | pax_text_size      | pax_font_t \*font, float font_size, char \*text | Calculate the size of the string with the given font.

Of course, there are some font APIs as well.

First, every font in pax has the attribute `default_size`:
`myfont->default_size`.
This value is the size recommended by the font's creator.

Second, every font has the `recommend_aa` attribute.
This tells PAX whether to or not to use anti-aliasing (or rather interpolation) by default.

There is also this utility function that finds a font for you:
| returns       | name         | arguments
| :------------ | :----------- | :----------
| pax_font_t \* | pax_get_font | char \*name

If your pick isn't found it returns the default font instead.

Current list of fonts:
| name             | id                      | default size            | glyphs
| :--------------- | :---------------------- | :---------------------- | :-----
| Sky Mono         | `"sky mono"` or `"7x9"` | 7 (width) by 9 (height) | ASCII
| Sky Variable     | `"sky"`                 | 9 (height)              | ASCII and latin supplements
| Permanent Marker | `"permanentmarker"`     | 22 (height)             | ASCII and latin supplements
| Saira Regular    | `"saira regular"`       | 18 (height)             | ASCII and latin supplements
| Saira Condensed  | `"saira condensed"`     | 45 (height)             | ASCII and latin supplements


# Background
| name           | arguments                        | description
| :------------- | :------------------------------- | :----------
| pax_background | pax_buf_t \*buf, pax_col_t color | Fills the entire buffer with the given color.
