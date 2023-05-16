# PAX graphics documentation: C++ API

The PAX graphics stack is being developed for the [MCH2022 badge](https://bodge.team/docs/badges/mch2022/).
It's goal is to allow anyone to, in C and/or C++, use a powerful list of drawing features with good optimisation.

This library is the successor of the revised graphics API for [the old badge.team firmware](https://github.com/badgeteam/ESP32-platform-firmware).

For supported platforms, [see this link](../supported-platforms.md).

**Note: The C++ API is not compiled by default outside of ESP-IDF, define `PAX_COMPILE_CXX` in cmake to include it.**


## PAX Docs overview

- [Getting started](#getting-started) (TODO)
- [API overview](#api-overview)
  - [Drawing functions](drawing.md)
  - [`pax::Buffer`](buffer.md)
  - [`pax::TextBox`](textbox.md)
  - [Shaders](shaders.md)
  - [Vector types](matrix.md)
- [Example code](#example-code)
  - [`pax::Buffer`](#example-paxbuffer)
  - [`pax::Shader`](#example-paxshader)
  - [`pax::Vec2f` and other vector types](#example-pax-vectors)



# Getting started
TODO.



# API overview

This chapter contains links to complete specifications of every type and every function in and used by the C++ API:
- [Drawing functions](drawing.md)
  - [Basic drawing functions](drawing.md#basic-drawing-functions)
    - [Circles](drawing.md#circles)
    - [Arcs](drawing.md#arcs)
    - [Triangles](drawing.md#triangles)
    - [Rectangles](drawing.md#rectangles)
    - [Lines](drawing.md#rectangles)
    - [Arbitrary shapes](drawing.md#arbitrary-shapes)
  - [Text](drawing.md#text)
  - [Images](drawing.md#images)
  - [Shader drawing functions](drawing.md#shaded-drawing)
  - [Pixel manipulation](drawing.md#pixel-manipulation)
- [`pax::Buffer`](buffer.md)
  - [Setup functions](buffer.md#setup-functions)
  - [Clipping](buffer.md#clipping)
  - [Dirty area](buffer.md#dirty-area)
  - [Rotation](buffer.md#rotation)
  - [Scrolling](buffer.md#scrolling)
  - [Transformations](buffer.md#transformations)
- [`pax::TextBox`](textbox.md)
  - [Purpose](textbox.md#purpose)
  - [Custom elements](textbox.md#custom-elements)
- [Shaders](shaders.md)
  - [Shader drawing functions](drawing.md#shaded-drawing)
  - [`pax::Shader`](shaders.md#paxshader)
  - [Purpose](shaders.md#purpose)
  - [Creation](shaders.md#creation)
- [Vector types](matrix.md)
  - [`pax::Vec2f` and `pax::Pointf`](matrix.md#paxvec2f)
  - [`pax::BiVec2f` and `pax::Linef`](matrix.md#paxbivec2f)
  - [`pax::TriVec2f` and `pax::Trif`](matrix.md#paxtrivec2f)
  - [`pax::QuadVec2f` and `pax::Quadf`](matrix.md#paxquadvec2f)
  - [`pax::Rectf`](matrix.md#paxrectf)
  - [`pax::Vec2i`](matrix.md#paxvec2i)
  - [`pax::Recti`](matrix.md#paxrecti)
  - [`pax::Matrix2f`](matrix.md#paxmatrix2f)



# Example code

This chapter contains several examples, at least one per major feature:
- [`pax::Buffer`](#example-paxbuffer)
  - [Setup functions](#example-setup)
  - [Basic drawing functions](#example-basic-drawing)
  - [Text drawing](#example-text-drawing)
  - [Scrolling function](#example-scrolling)
- [`pax::Shader`](#example-paxshader)
  - [Purpose](#example-paxshader)
  - [List of uses](#uses-for-paxshader)
  - [Conversion](#conversions-for-paxshader)
- [`pax::Vec2f` and other vector types](#example-pax-vectors)
  - [`List of uses`](#example-pax-vectors)
  - [`pax::Vec2f`](#example-paxvec2f)
  - [Multiple-vector types](#multiple-vector-types)
  - [`pax::Rectf`](#example-paxrectf)
  - [`pax::Vec2i` and `pax::Recti`](#integer-vector-counterparts)



# Example: `pax::Buffer`

The buffer is the primary type in PAX.
It stores the image data as well as any information required to manipulate it.

This section covers:
- [Setup functions](#example-setup) (how to create your `pax::Buffer`)
- [Basic drawing functions](#example-basic-drawing) (the bulk of what `pax::Buffer` does)
- [Text drawing](#example-text-drawing) (basic version)
- [Scrolling function](#example-scrolling) (useful for games and terminals)


## Example: setup

For full explanation, [see this link](buffer.md#setup-functions).

The most basic way to initialise a buffer is with a size and a type.
For most users, this is all you'll need.

```cpp
// This is a 320x240 buffer with 5, 6 and 5 bits for Red, Green and Blue respectively.
pax::Buffer myBuffer(320, 240, PAX_BUF_16_565RGB);
```

There is also the option to give a preallocated framebuffer (before the size argument).
PAX assumes you made it big enough (width * height * bits per pixel, divided by and rounded up to bytes).
This function is useful to reduce dynamic allocation or if you happen to already have a framebuffer.

```cpp
// Make some storage big enough.
uint16_t framebuffer[320*240];
// Similar initialisation.
pax::Buffer myBuffer(framebuffer, 320, 240, PAX_BUF_16_565RGB);
```

Some displays have a different *endianness* (order of bytes for multi-byte numbers) than the host processor.
For these types of displays, `reverseEndianness` was created.
All this does is reverse the stored endianness of colors, where applicable.

**Note: This does not apply to the image retroactively.**

```cpp
// If you were to read the raw pixel data, the endianness would be the opposite of what it is with this setting disabled.
myBuffer.reverseEndianness(true);
// This function retrieves the current endianness setting.
bool reversed = myBuffer.isReverseEndianness();
```

Depending on how your display is oriented, you may want to *rotate* the drawn image in quarter-turn increments.
Doing this has been made possible with `setRotation`.

```cpp
// This will cause future drawing to appear as rotated one (1) quarter-turn counter-clockwise.
// Similarly, 2 is a half-turn (AKA two quarter-turns) counter-clockwise.
// 0 is not rotated.
myBuffer.setRotation(1);
// This function retrieves the current rotation setting.
int rotation = myBuffer.getRotation();
```

## Example: basic drawing functions

The vast majority of API calls will be basic drawing calls.
Whether this is to draw some fine art or simply to blank the canvas,
understanding all of these functions is paramount.

Suppose you'd want to draw a circle.
This is the `drawCircle` function.
It takes (in order) an optional color, X position, Y position and radius.
If the color parameter is omitted, the current `fillColor` is used.

*Note: The color parameter omission rule applies to all `draw*` functions.*

```cpp
// Draw a red circle at 20 from the left, 50 from the top.
// 30-pixel radius.
// Color format is AARRGGBB.
myBuffer.drawCircle(0xffff0000, 20, 50, 30);
```

You can do the same with the outline of a circle.
In this cause, the current `lineColor` is the default color.

*Note: Again, the color parameter omission rule applies to all `outline*` functions.*

```cpp
// The same circle, but as an outline instead of filled in.
myBuffer.outlineCircle(0xffff0000, 20, 50, 30);
```

A slightly more fancy circle is an *arc* (AKA part of a circle).
This has two more parameters at the end: starting angle and ending angle.
All angles in PAX are radians (0 is not rotated, 2*M_PI is one full counter-clockwise rotation).
Additionally, radians enforce that the `startAngle` of 0 shall mean the arc starts on the right-hand (positive X) side.

```cpp
// This is a green arc.
// It is like a pie with a slice removed from it.
myBuffer.drawArc(20, 50, 30, 1/4*M_PI, 2*M_PI);
```

You get the point. Rectangles: optional color, X, Y, width, height.

```cpp
// Maybe you fancy a blue rectangle?
// Position (10, 10), size 100 wide by 20 tall.
myBuffer.drawRectangle(0xff0000ff, 10, 10, 100, 20);
```

When you get to drawing a single, straight line, the rules change slightly.
Because a line has no "shape" you can specify relative to its position, each point between which the line spans is an arguments.

Put simply, give (X, Y) from and (X, Y) to.

*Note: Lines have no `outline` alternative because they are already an outline. This also means that they use the `lineColor` as default instead of `fillColor`.*

```cpp
// Draw a line between (30, 50) and (100, 90).
myBuffer.drawLine(30, 50, 100, 90);
```

Finally, there is the triangle.
This *is* a filled shape, but, like lines, you specify three points between which to draw a triangle.

```cpp
// Draw a triangle between (20, 50), (100, 90) and (20, 80).
myBuffer.drawLine(20, 50, 100, 90, 20, 80);
```


## Example: Text drawing

Text is an ubiquitous part of user interface.
Similar to basic shapes, the first argument, the color, is optional.
The default for this is, once again, `fillColor`.

Text takes a *font* argument, [see this link](drawing.md#fonts) for some built-in options.

*Note: There is no `outline` version of drawString*

```cpp
// Draw "Hello, World!" in font "Sky" at 18pt (twice the default size of Sky).
// Like basic drawing, the last parameters are (in order):
// Position (10, 10) and "shape" (the text "Hello, World!").
myBuffer.drawString(pax_font_sky, 18, 10, 10, "Hello, World!");
```

You can have multiple lines, split by your choice of CR, LF and CRLF:

```cpp
// Four lines of text, showing all three line separators.
// \r, \n and \r\n are CR, LF and CRLF respectively.
myBuffer.drawString(pax_font_sky, 18, 10, 10, "1\r2\n3\r\n4");
```

Similarly, `drawStringCentered` draws text, *center-aligned* horizonally (on each line individually).

```cpp
// Some more simple text, center-aligned.
myBuffer.drawStringCentered(pax_font_sky, 18, 160, 10, "Button text is often center-aligned");
```

For some purposes, you may want to determine the size of a string *before* drawing it.
The function for this, `stringSize`, applies to both `drawString` and `drawStringCentered`.

*Note: Because the drawn size is not affected by any setting in `pax::Buffer`, `stringSize` is a static member function.*

```cpp
// In this particular case, this will return 146 wide by 18 tall.
pax::Vec2f size = pax::Buffer::stringSize(pax_font_sky, 18, "Hello, World!");
```



## Example: scrolling

This relatively niche function is useful for 2D games and terminal emulators.
It scrolls the content of the buffer with some specified offset, optionally filling the edges with a placeholder color.
If you do not specify a placeholder color, the area revealed by this scrolling has *unspecified* data.

```cpp
// This will scroll the content 3 pixels to the right, 5 pixels up.
// Uses current `fillColor` value as the placeholder.
myBuffer.scroll(3, -5);
// The same, but uses red as placeholder.
myBuffer.scroll(0xffff0000, 3, -5);
// The same again, with no placeholder (due to alpha=0).
myBuffer.scroll(0x00000000, 3, -5);
```



# Example: `pax::Shader`

Shaders are for when you want to draw a shape with non-uniform coloring.
Fundamentally, a shader determines what color each pixel in a shape should be.

*For example, text is internally implemented with shaders.
If you were to draw text as opaque instead of with shaders, you would see a bunch of rectangles where glyphs should be.*

TODO: Shader example code.

## Uses for `pax::Shader`

Shaders are internally used for drawing text and images.
This approach allows for both to be affected by [transformations](matrix.md).

Shaders are also used for one of the scenes in the [MCH2022 badge tech demo](https://www.youtube.com/watch?v=-yk3I0oce4k).
The effect in question is the "rainbow explosion" transition to the gears scene.

If you wanted a gradient, you could use a shader to achieve such an effect.

## Conversions for `pax::Shader`

TODO.



# Example: PAX vectors

There is a small collection of vector types in PAX.
Most of them represent a list of (up to four) points.

Vector types are used to group a list of numbers into a single type.
*For example, UV co-ordinates to shaded drawing. Another example is the size returned by `pax::Buffer::stringSize`*.

## Example: `pax::Vec2f`

This is the most common vector type.
It is used in a variety of functions to returns either position or size.

You can initialise one like an array:

```cpp
// Initialises myVector with x=0, y=10.
pax::Vec2f myVector {0, 10};
```

Furthermore, you can apply math operators to it, either a constant value or another vector using: `==`, `!=`, `+`, `-`, `*`, `/`, `+=`, `-=`, `*=` and `/=`.
These operators apply to all `pax::*Vec2f` types.

Finally, there are some unique functions for the *magnitude* of a vector (distance from the origin assuming the vector is a position).

```cpp
// Calculate the magnitude of a vector.
float magnitude = myVector.magnitude();
// Same thing but square the result.
// This is faster than `magnitude` because it eliminates a square root.
float sqrMagnitude = myVector.squareMagnitude();
// Divive `myVector` by its magnitude.
myVector.unify();
```

## Multiple-vector types

The longer vectors (`pax::BiVec2f`, `pax::triVec2f` and `pax::QuadVec2f`) are collections of multiple `pax::Vec2f`.
They can be array-indexed, which will yield a read/write `pax::Vec2f` reference.

Because there are multiple values, the magnitude-related functions don't exist in these types.

The counter to this is that you can now take the average over these vectors, which will yield a `pax::Vec2f` average of their components.


## Example: `pax::Rectf`

This is a simple container type for a rectangle defined by position and size.
It only has equality operators.

The `average` function returns a `pax::Vec2f` representing the center point of the rectangle.

Functions `position` and `size` return read/write references to their respective components of the rectangle.

The `toQuad` function creates a `pax::QuadVec2f` that represents the outline of the rectangle.

Finally, the `fixSize` function returns a copy of the rectangle which occupies the same location but is gauranteed to have non-negative width and height.

## Integer vector counterparts

There are integer counterparts to `pax::Vec2f` (integer version `pax::Vec2i`) and `pax::Rectf` (integer version `pax::Recti`).

These have functions that only make sense as floating-point (e.g. `magnitude`) removed.
