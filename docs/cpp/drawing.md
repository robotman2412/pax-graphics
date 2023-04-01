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

The basic drawing functions are the most commonly used functions.

## Drawing function overview

All functions follow one of the following templates:
| prefix  | arguments                   | description
| :------ | :-------------------------- | :----------
| draw    | ...                         | Fill in a shape using `fillColor`.
| draw    | Color, ...                  | Fill in a shape given a color.
| draw    | Shader\*, UVs\*, ...        | Fill in a shape using `fillColor`, given a [shader](#shaded-drawing).
| draw    | Color, Shader\*, UVs\*, ... | Fill in a shape given a color and [shader](#shaded-drawing).
| outline | ...                         | Outline a shape using `fillColor`.
| outline | Color, ...                  | Outline a shape given a color.
| outline | Shader\*, UVs\*, ...        | Outline a shape using `fillColor`, given a [shader](#shaded-drawing).
| outline | Color, Shader\*, UVs\*, ... | Outline a shape given a color and [shader](#shaded-drawing).

The following table species the full names, type of UVs and last arguments (the '...') of drawing functions.

**Note: Type of UVS is unspecified when they are implicit based on the arguments.**

| name            | type of UVs    | last arguments
| :-------------- | :------------- | :--------
| `drawCircle`    | `pax::Quadf`   | float x, float y, float radius
| `outlineCircle` | `pax::Quadf`   | float x, float y, float radius
| `drawArc`       | `pax::Quadf`   | float x, float y, float radius
| `outlineArc`    | `pax::Quadf`   | float x, float y, float radius
| `drawTri`       | `pax::Trif`    | float x0, float y0, float x1, float y1, float x2, float y2
| `outlineTri`    | `pax::Trif`    | float x0, float y0, float x1, float y1, float x2, float y2
| `drawRect`      | `pax::Quadf`   | float x, float y, float width, float height
| `outlineRect`   | `pax::Quadf`   | float x, float y, float width, float height
| `drawLine`      | `pax::Linef`   | float x0, float y0, float x1, float y1
| `draw`          |                | `pax::Shape &`
| `outline`       |                | `pax::Shape &`

## Circles
## Arcs
## Triangles
## Rectangles
## Lines
## Arbitrary shapes



# Text
# Images
# Shaded drawing
# Pixel manipulation
