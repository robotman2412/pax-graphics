# PAX Docs: Drawing

This document covers the drawing functions of `pax::Buffer`.
For setup and other functions, see [`pax::Buffer`](buffer.md).

In this document:
- [Background filling](#background-filling)
- [Basic drawing functions](#basic-drawing-functions)
  - [Circles](#circles)
  - [Arcs](#arcs)
  - [Triangles](#triangles)
  - [Rectangles](#rectangles)
  - [Lines](#rectangles)
  - [Arbitrary shapes](#arbitrary-shapes)
- [Text](#text)
- [Images](#images)
- [Shader drawing functions](#shaded-drawing)
- [Pixel manipulation](#pixel-manipulation)



# Background filling

The simple `background` function is so important that is deserves a section of its own.

This function is commonly used as the very first when starting to render a new frame, and it can be beneficial to optimise your software around this feature.

### `void background(Color color)`

Fills the entirety of the image data with a single specified color, overriding alpha channel (if the [pixel type](../pixelformat.md) has one).

## Relation to double buffering

*Double buffering* is to have two distinct framebuffers such that one is used for rendering while the other is used to update the display.

The `background` function would then obscure that fact that there are two buffers by replacing the switched-in buffer with a solid color.
If you didn't clean the slate before drawing, a copy would be required, which is slightly slower than filling with a solid color.



# Basic drawing functions

This chapter begins with a basic overview of the functions, followed by an exact specification of every function:
- [Quick overview](#drawing-function-overview)
- [Circle functions](#circles)
- [Arc functions](#arcs)
- [Triangle functions](#triangles)
- [Rectangle functions](#rectangles)
- [Line functions](#rectangles)
- [Arbitrary shapes](#arbitrary-shapes)

## Drawing function overview

All functions follow one of the following templates:
| prefix  | arguments                  | description
| :------ | :------------------------- | :----------
| draw    | ...                        | Fill in a shape using `fillColor`.
| draw    | Color, ...                 | Fill in a shape given a color.
| draw    | Shader&, UVs\*, ...        | Fill in a shape using `fillColor`, given a [shader](#shaded-drawing).
| draw    | Color, Shader&, UVs\*, ... | Fill in a shape given a color and [shader](#shaded-drawing).
| outline | ...                        | Outline a shape using `fillColor`.
| outline | Color, ...                 | Outline a shape given a color.
| outline | Shader&, UVs\*, ...        | Outline a shape using `fillColor`, given a [shader](#shaded-drawing).
| outline | Color, Shader&, UVs\*, ... | Outline a shape given a color and [shader](#shaded-drawing).

The following table species the full names, type of UVs and last arguments (the '...') of drawing functions.

**Note: Type of UVS is unspecified when they are implicit based on the arguments.**

| name            | type of UVs    | last arguments
| :-------------- | :------------- | :--------
| `drawCircle`    | `pax::Quadf`   | float x, float y, float radius
| `drawArc`       | `pax::Quadf`   | float x, float y, float radius
| `drawTri`       | `pax::Trif`    | float x0, float y0, float x1, float y1, float x2, float y2
| `drawRect`      | `pax::Quadf`   | float x, float y, float width, float height
| `drawLine`      | `pax::Linef`   | float x0, float y0, float x1, float y1
| `draw`          |                | `pax::Shape &`
| `outlineCircle` | `pax::Quadf`   | float x, float y, float radius
| `outlineArc`    | `pax::Quadf`   | float x, float y, float radius
| `outlineTri`    | `pax::Trif`    | float x0, float y0, float x1, float y1, float x2, float y2
| `outlineRect`   | `pax::Quadf`   | float x, float y, float width, float height
| `outline`       |                | `pax::Shape &`

## Circles

The `drawCircle` and `outlineCircle` functions draw and outline respectively a circle of unspecified resolution.
Like most `draw` functions, there are optional colors, shaders and an `outline` variant.

Most drawing functions (including `drawCircle` and `outlineCircle`) are member functions of `pax::Buffer`.

---
### `void drawCircle(float x, float y, float radius)`
Fills in a circle with color as `fillColor` setting, midpoint (`x`, `y`) and radius `radius`.

---
### `void drawCircle(Color color, float x, float y, float radius)`
Fills in a circle with color `color`, midpoint (`x`, `y`) and radius `radius`.

---
### `void drawCircle(const Shader &shader, const Quadf *uvs, float x, float y, float radius)`
Fills in a circle with color as `fillColor` setting, midpoint (`x`, `y`) and radius `radius`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void drawCircle(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius)`
Fills in a circle with color `color`, midpoint (`x`, `y`) and radius `radius`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void outlineCircle(float x, float y, float radius)`
Outlines a circle with color as `lineColor` setting, midpoint (`x`, `y`) and radius `radius`.

---
### `void outlineCircle(Color color, float x, float y, float radius)`
Outlines a circle with color `color`, midpoint (`x`, `y`) and radius `radius`.

---
### `void outlineCircle(const Shader &shader, const Quadf *uvs, float x, float y, float radius)`
Outlines a circle with color as `lineColor` setting, midpoint (`x`, `y`) and radius `radius`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void outlineCircle(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius)`
Outlines a circle with color `color`, midpoint (`x`, `y`) and radius `radius`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.


## Arcs

The `drawArc` and `outlineArc` functions draw and outline respectively an arc of unspecified resolution.
Like most `draw` functions, there are optional colors, shaders and an `outline` variant.

Most drawing functions (including `drawArc` and `outlineArc`) are member functions of `pax::Buffer`.

Angle units are in radians: Starting relatively +X, rotating one turn counter-clockwise every `M_PI*2` radians.

---
### `void drawArc(float x, float y, float radius, float startangle, float endangle)`
Fills in an arc with color as `fillColor` setting, midpoint (`x`, `y`) and radius `radius`.
The arc shall be filled in like a pie between `startAngle` and `endAngle`.

---
### `void drawArc(Color color, float x, float y, float radius, float startangle, float endangle)`
Fills in an arc with color `color`, midpoint (`x`, `y`) and radius `radius`.
The arc shall be filled in like a pie between `startAngle` and `endAngle`.

---
### `void drawArc(const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle)`
Fills in an arc with color as `fillColor` setting, midpoint (`x`, `y`) and radius `radius`.
The arc shall be filled in like a pie between `startAngle` and `endAngle`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.
The arc shall be filled in like a pie between `startAngle` and `endAngle`.

---
### `void drawArc(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle)`
Fills in an arc with color `color`, midpoint (`x`, `y`) and radius `radius`.
The arc shall be filled in like a pie between `startAngle` and `endAngle`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void outlineArc(float x, float y, float radius, float startangle, float endangle)`
Outlines an arc with color as `lineColor` setting, midpoint (`x`, `y`) and radius `radius`.
The arc shall be drawn like a partial circle outline between `startAngle` and `endAngle`.

---
### `void outlineArc(Color color, float x, float y, float radius, float startangle, float endangle)`
Outlines an arc with color `color`, midpoint (`x`, `y`) and radius `radius`.
The arc shall be drawn like a partial circle outline between `startAngle` and `endAngle`.

---
### `void outlineArc(const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle)`
Outlines an arc with color as `lineColor` setting, midpoint (`x`, `y`) and radius `radius`.
The arc shall be drawn like a partial circle outline between `startAngle` and `endAngle`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void outlineArc(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius)`
Outlines an arc with color `color`, midpoint (`x`, `y`) and radius `radius`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.


## Triangles

The `drawTri` and `outlineTri` functions draw and outline respectively a triangle defined by its three corners.
Like most `draw` functions, there are optional colors, shaders and an `outline` variant.

Most drawing functions (including `drawTri` and `outlineTri`) are member functions of `pax::Buffer`.

---
### `void drawTri(float x0, float y0, float x1, float y1, float x2, float y2)`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `fillColor` setting.

---
### `void drawTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2)`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

---
### `void drawTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2)`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `fillColor` setting.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.

---
### `void drawTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2)`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.


---
### `void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2)`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `lineColor` setting.

---
### `void outlineTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2)`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

---
### `void outlineTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2)`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `lineColor` setting.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.

---
### `void outlineTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2)`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.


## Rectangles

The `drawRect` and `outlineRect` functions draw and outline respectively a rectangle defined by its position and size.
Like most `draw` functions, there are optional colors, shaders and an `outline` variant.

Most drawing functions (including `drawRect` and `outlineRect`) are member functions of `pax::Buffer`.

---
### `void drawRect(float x, float y, float width, float height)`
Fills in a rectangle with position (`x`, `y`) and size `width` by `height` with color as `fillColor` setting.
`width` and/or `height` may be negative numbers.

---
### `void drawRect(Color color, float x, float y, float width, float height)`
Fills in a rectangle with position (`x`, `y`) and size `width` by `height` with color `color`.
`width` and/or `height` may be negative numbers.

---
### `void drawRect(const Shader &shader, const Quadf *uvs, float x, float y, float width, float height)`
Fills in a rectangle with position (`x`, `y`) and size `width` by `height` with color as `fillColor` setting.
`width` and/or `height` may be negative numbers.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void drawRect(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float width, float height)`
Fills in a rectangle with position (`x`, `y`) and size `width` by `height` with color `color`.
`width` and/or `height` may be negative numbers.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.



---
### `void outlineRect(float x, float y, float width, float height)`
Outlines a rectangle with position (`x`, `y`) and size `width` by `height` with color as `lineColor` setting.
`width` and/or `height` may be negative numbers.

---
### `void outlineRect(Color color, float x, float y, float width, float height)`
Outlines a rectangle with position (`x`, `y`) and size `width` by `height` with color `color`.
`width` and/or `height` may be negative numbers.

---
### `void outlineRect(const Shader &shader, const Quadf *uvs, float x, float y, float width, float height)`
Outlines a rectangle with position (`x`, `y`) and size `width` by `height` with color as `lineColor` setting.
`width` and/or `height` may be negative numbers.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.

---
### `void outlineRect(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float width, float height)`
Outlines a rectangle with position (`x`, `y`) and size `width` by `height` with color `color`.
`width` and/or `height` may be negative numbers.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 1,1; 0,1) will be used.


## Lines

The `drawLine` function draws straight lines between two end points.
Because the shape is already an outline, there is no `outline` variant.
For the same reason, the default color value is the `lineColor` setting instead of the `fillColor` setting.

The `outline` functions are all based on the `drawLine` functions.

Most drawing functions (including `drawLine`) are member functions of `pax::Buffer`.

---
### `void drawLine(float x0, float y0, float x1, float y1)`
Draws a straight line between the points (x0, y0) and (x1, y1) with color as `lineColor` setting.

---
### `void drawLine(Color color, float x0, float y0, float x1, float y1)`
Draws a straight line between the points (x0, y0) and (x1, y1) with color `color`.

---
### `void drawLine(const Shader &shader, Linef* uvs, float x0, float y0, float x1, float y1)`
Draws a straight line between the points (x0, y0) and (x1, y1) with color as `lineColor` setting.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0) will be used.

---
### `void drawLine(Color color, const Shader &shader, Linef* uvs, float x0, float y0, float x1, float y1)`
Draws a straight line between the points (x0, y0) and (x1, y1) with color `color`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0) will be used.


## Arbitrary shapes
TODO.


# Text

Text is an ubuquitous part of UI (User Interface) design.
This chapter focusses on the simple `drawString` API.

Like the simple drawing functions, the `color` parameter is optional and the default color is `fillColor`.
There is also a center-aligned version of `drawString` (`drawStringCentered`; useful for things like buttons).

For more advanced text options, see [`pax::TextBox`](textbox.md).

---
### `static Vec2f stringSize(const pax_font_t *font, float font_size, std::string text)`
Determines the dimentions of string `text` when rendered with font `font` and font size `font_size`.

Because the results aren't affected by the buffer's settings, it is a *static member function*; Call it as `pax::Buffer::stringSize(...)`.

---
### `Vec2f drawString(const pax_font_t *font, float font_size, float x, float y, std::string text)`
Draws the string `text` with font `font` size `font_size` with color as current `fillColor` setting.
Text is left-aligned.

---
### `Vec2f drawString(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text)`
Draws the string `text` with font `font` size `font_size` with color `color`.
Text is left-aligned.

---
### `Vec2f drawStringCentered(const pax_font_t *font, float font_size, float x, float y, std::string text)`
Draws the string `text` with font `font` size `font_size` with color as current `fillColor` setting.
Text is center-aligned on each line individually.

---
### `Vec2f drawStringCentered(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text)`
Draws the string `text` with font `font` size `font_size` with color `color`.
Text is center-aligned on each line individually.


# Images

Images are useful for many things, most commonly used for icons, logos and pre-rendered graphics.

There is a is a small list of image drawing options including:
- Unspecified dimensions (uses image's dimensions)
- Specified dimensions
- Opaque images (ignores opacity data)

**Note: The behaviour of drawing a buffer to itself is *unspecified*.**

---
### `void drawImage(const pax_buf_t *image, float x, float y);`
Draws an image stored in a `const pax_buf_t *` (from the C-style API).
Uses the image's width and height.

---
### `void drawImage(const pax_buf_t *image, float x, float y, float width, float height);`
Draws an image stored in a `const pax_buf_t *` (from the C-style API).
Uses user-specified width (`width` parameter) and height (`height` parameter).

---
### `void drawImage(const Buffer &image, float x, float y) { drawImage(image.internal, x, y); }`
Draws an image stored in another `pax::Buffer`.
Uses the image's width and height.

---
### `void drawImage(const Buffer &image, float x, float y, float width, float height) { drawImage(image.internal, x, y, width, height); }`
Draws an image stored in another `pax::Buffer`.
Uses user-specified width (`width` parameter) and height (`height` parameter).

---
### `void drawImageOpaque(const pax_buf_t *image, float x, float y);`
Draws an image stored in a `const pax_buf_t *` (from the C-style API).
Uses the image's width and height.

Assumes the image is opaque and ignores and transparency if present.

---
### `void drawImageOpaque(const pax_buf_t *image, float x, float y, float width, float height);`
Draws an image stored in a `const pax_buf_t *` (from the C-style API).
Uses user-specified width (`width` parameter) and height (`height` parameter).

Assumes the image is opaque and ignores and transparency if present.

---
### `void drawImageOpaque(const Buffer &image, float x, float y) { drawImage(image.internal, x, y); }`
Draws an image stored in another `pax::Buffer`.
Uses the image's width and height.

Assumes the image is opaque and ignores and transparency if present.

---
### `void drawImageOpaque(const Buffer &image, float x, float y, float width, float height) { drawImage(image.internal, x, y, width, height); }`
Draws an image stored in another `pax::Buffer`.
Uses user-specified width (`width` parameter) and height (`height` parameter).

Assumes the image is opaque and ignores and transparency if present.


# Shaded drawing
TODO: Put shader code examples here.

For more information, see [`pax::Shader`](shaders.md).

# Pixel manipulation
For certain effects (like particles in games or applying dithering), direct plotting of single pixels may be required.

PAX has a simple, three-part API for directly manipulating pixel data:
- [Getting/Setting](#color-getpixelint-x-int-y-const)
- [Raw pixel data access]()
- [Setting with alpha](#void-mergepixelcolor-color-int-x-int-y)

*Note: Make sure to call `pax::join()` before using these functions to avoid graphical artefacts.*

---
### `Color getPixel(int x, int y) const`
Gets the ARGB representation of the pixel data at (`x`, `y`).
Applies palette lookup for palette pixel types.

---
### `void setPixel(Color color, int x, int y)`
Sets the color at (`x`, `y`) to `color`.
Overrides current opacity value, if applicable.
Does no color conversion for palette pixel types.

---
### `Color getPixelRaw(int x, int y) const`
Gets the raw pixel data at (`x`, `y`).
Does no color conversion.

---
### `void setPixelRaw(Color color, int x, int y)`
Sets the raw pixel data at (`x`, `y`) to `color`.
Overrides current opacity value, if applicable.
Does no color conversion.

---
### `void mergePixel(Color color, int x, int y)`
Performs an alpha overlay to pixel at (`x`, `y`).

Behaves differently for palette pixel types; set pixel if `color & 0xff000000 > 0`, does nothing if `color & 0xff000000 == 0`.

