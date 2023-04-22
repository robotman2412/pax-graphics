# PAX docs: Drawing

The drawing methods are the core of PAX graphics.

Most drawing methods fall in to three categories:
- [Simple](#simple-drawing)
- [Normal](#normal-drawing)
- [Shaded](#shaded-drawing)

Other drawing methods include:
- [Text](#text-drawing)
- [Complex shapes](#complex-shapes)
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
| pax_draw_image       | pax_buf_t \*buf, pax_buf_t \*image, float x, y                       | Draws an image at the image's normal size.
| pax_draw_image_sized | pax_buf_t \*buf, pax_buf_t \*image, float x, y, width, height        | Draw an image with a prespecified size.
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

List of shaded drawing methods:
| name             | arguments                                                                                                     | description
| :--------------- | :------------------------------------------------------------------------------------------------------------ | :----------
| pax_shade_rect   | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quadf \*uvs, float x, y, width, height          | Draws a rectangle with the given dimensions.
| pax_shade_line   | pax_buf_t \*buf, pax_col_t color, const pax_shader_t \*shader, const pax_linef *uvs, float x0, y0, x1, y1 | Draws a line between two points.
| pax_shade_tri    | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_trif  \*uvs, float x0, y0, x1, y1, x2, y2       | Draws a triangle between three points.
| pax_shade_arc    | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quadf \*uvs, float x, y, radius, angle0, angle1 | Draws an arc between two angles, at a given midpoint.
| pax_shade_circle | pax_buf_t \*buf, pax_col_t color, pax_shader_t \*shader, pax_quadf \*uvs, float x, y, radius                 | Draws a circle at a given midpoint.

# Shaded outline drawing

You can combine the shaded drawing with the outline drawing.

List of shaded outline drawing methods:
| name               | arguments                                                            | description
| :----------------- | :------------------------------------------------------------------- | :----------
| pax_shade_outline_rect   | pax_buf_t \*buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, y, width, height          | Outlines a rectangle with the given dimensions.
| pax_shade_outline_tri    | pax_buf_t \*buf, pax_col_t color, const pax_shader_t *shader, const pax_trif *uvs, float x0, y0, x1, y1, x2, y2       | Outlines a triangle between three points.
| pax_shade_outline_arc    | pax_buf_t \*buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, y, radius, angle0, angle1 | Outlines an arc between two angles, at a given midpoint. Does not create lines to the center.
| pax_shade_outline_circle | pax_buf_t \*buf, pax_col_t color, const pax_shader_t *shader, const pax_quadf *uvs, float x, y, radius                 | Outlines a circle at a given midpoint.

# Text drawing

Text drawing works like normal drawing, but with more characters and UTF-8 support.
In PAX, you can draw text as well as calculate it's size.

List of text methods:
| returns    | name               | arguments                                                                                           | description
| :------    | :---               | :--------                                                                                           | :----------
| void       | pax_center_text    | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. This text will be horizontally center-aligned.
| void       | pax_draw_text      | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Interpolation depends on the font.
| void       | pax_draw_text_aa   | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Always uses interpolation.
| void       | pax_draw_text_noaa | pax_buf_t \*buf, pax_col_t color, pax_font_t \*font, float font_size, float x, float y, char \*text | Draws text with a given font. Never uses interpolation.
| pax_vec2f | pax_text_size      | pax_font_t \*font, float font_size, char \*text | Calculate the size of the string with the given font.

Of course, there are some font APIs as well.

First, every font in pax has the attribute `default_size`:
`myfont->default_size`.
This value is the size recommended by the font's creator.

Second, every font has the `recommend_aa` attribute.
This tells PAX whether to or not to use anti-aliasing (or rather interpolation) by default.

Current list of fonts:
| name             | id                         | default size            | glyphs
| :---             | :-                         | :---------------------- | :-----
| Sky Mono         | `pax_font_sky_mono`        | 7 (width) by 9 (height) | ASCII
| Sky Variable     | `pax_font_sky`             | 9 (height)              | ASCII and latin supplements
| Permanent Marker | `pax_font_marker`          | 22 (height)             | ASCII and latin supplements
| Saira Regular    | `pax_font_saira_regular`   | 18 (height)             | ASCII and latin supplements
| Saira Condensed  | `pax_font_saira_condensed` | 45 (height)             | ASCII and latin supplements

## Font loading

PAX is capable of both importing and exporting a custom font file format.
The purpose of this is to allow dynamic loading of fonts, in addition to not having to statically allocate large fonts you'll use only a few times.

| returns       | name           | arguments                          | description
| :------       | :---           | :--------                          | :----------
| pax_font_t \* | pax_load_font  | FILE \*fd                          | Loads a font from a file. The font can be destroyed by simply using `free` on it.
| void          | pax_store_font | FILE \*fd, const pax_font_t \*font | Stores a font to a file.



# Complex shapes

There is a small collection of advanced shape manipulation, generation and drawing methods.

These include:
- Vectorisation (to generate an outline for a shape)
- Drawing of non-builtins (not a circle, nor triangle, nor rectangle, etc.)
- Manipulation (to transform a shape into a new one)

## Vectorisation

You can create an outline defines by a list of points. This works for arcs, circles and bezier curves, and you can give the generated outlines a position offset.
The last point generated will *not* overlap the first.

| name                      | arguments                                                                               | description
| :---                      | :--------                                                                               | :----------
| pax_vectorise_circle      | pax_vec2f \*output, size_t num_points, float x, float y, float r                       | Creates a circle outline around the point (x, y).
| pax_vectorise_arc         | pax_vec2f \*output, size_t num_points, float x, float y, float r, float a0, float a1   | Creates an arc outline around the point (x, y), between two given angles.
| pax_vectorise_bezier      | pax_vec2f \*output, size_t num_points, pax_4vec2f control_points                       | Creates a line from a quadratic bezier curve.
| pax_vectorise_bezier_part | pax_vec2f \*output, size_t num_points, pax_4vec2f control_points, float from, float to | Creates a line from part of a quadratic bezier curve.

## Drawing of non-builtins

You can use `pax_outline_shape` to draw a line based on an array of `pax_vec2f`:
| name                   | arguments                                                                                            | description
| :---                   | :--------                                                                                            | :----------
| pax_outline_shape      | pax_buf_t \*buf, pax_col_t color, size_t num_points, const pax_vec2f \*points                       | Draws a line through all the points. Does draw a line back from the last point to the first.
| pax_outline_shape_part | pax_buf_t \*buf, pax_col_t color, size_t num_points, const pax_vec2f \*points, float from, float to | Does a fraction of the former, where `from` and `to` are a fraction of the total line length.

Similarly, you can use `pax_draw_shape` to fill in said outline:
| name           | arguments                                                                    | description
| :---           | :--------                                                                    | :----------
| pax_draw_shape | pax_buf_t *buf, pax_col_t color, size_t num_points, const pax_vec2f *points | Fills in an outline defined by an array of points. It will not work for self-overlapping.

For more performance, you can triangulate a shape ahead of time:
| returns | name               | arguments                                                       | description
| :------ | :---               | :--------                                                       | :----------
| size_t  | pax_triang_concave | size_t \*\*output, size_t num_points, const pax_vec2f \*points | Calculates a list of triangles to fill in the outline. Returns the amount of triangles generated.

And then draw it:
| name                  | arguments                                                                                                               | description
| :---                  | :--------                                                                                                               | :----------
| pax_draw_shape_triang | pax_buf_t \*buf, pax_col_t color, size_t num_points, const pax_vec2f \*points, size_t num_tris, const size_t \*indices | Draws a list of triangles generated by `pax_triang_concave`.

*Note: This doesn't work for shapes that intersect themselves.*

## Manipulation

There is a helper which applies a given 2D matrix to all points in an array:
| name                | arguments                                                  | description
| :---                | :--------                                                  | :----------
| pax_transform_shape | size_t num_points, pax_vec2f \*points, matrix_2d_t matrix | Transforms a list of points using a given 2D matrix. Overwrites the list's contents.

More are in the works, but not yet finished.


# Background

Usually used before drawing other things, `pax_background` will fill the entire buffer at once, ignoring alpha.
If the buffer stores alpha, then the buffer's alpha is set to the alpha of the provided color.
| name           | arguments                        | description
| :---           | :--------                        | :----------
| pax_background | pax_buf_t \*buf, pax_col_t color | Fills the entire buffer with the given color.
