# PAX Docs: Drawing

This document covers:
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

---
## Example: Circles
TODO.


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


---
## Example: Arcs
TODO.


## Triangles

The `drawCircle` and `outlineCircle` functions draw and outline respectively a triangle defined by its three corners.
Like most `draw` functions, there are optional colors, shaders and an `outline` variant.

Most drawing functions (including `drawTri` and `outlineTri`) are member functions of `pax::Buffer`.

---
### `void drawTri(float x0, float y0, float x1, float y1, float x2, float y2);`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `fillColor` setting.

---
### `void drawTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

---
### `void drawTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `fillColor` setting.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.

---
### `void drawTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);`
Fills in a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.


---
### `void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2);`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `lineColor` setting.

---
### `void outlineTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

---
### `void outlineTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with the color as `lineColor` setting.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.

---
### `void outlineTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);`
Outlines a triangle defined by the points (x0, y0), (x1, y1) and (x2, y2) with color `color`.

Applies shader `shader` to the shape with UV co-ordinates optionally provided in `uvs`.
If `uvs` is `nullptr`, a default (0,0; 1,0; 0,1) will be used.


## Example: Triangles
TODO.


## Rectangles
## Example: Rectangles
TODO.


## Lines
## Example: Lines
TODO.


## Arbitrary shapes
## Example: Arbitrary shapes
TODO.





# Text
# Images
# Shaded drawing
# Pixel manipulation
