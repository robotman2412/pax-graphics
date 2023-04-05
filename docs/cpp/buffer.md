# PAX Docs: `pax::Buffer`

This document covers the setup and related functions of `pax::Buffer`.
For drawing functions, see [drawing functions](drawing.md).

In this document:
- [Construction](#construction)
  - [From existing](#from-existing)
  - [Preallocated](#preallocated)
  - [Dynamically allocated](#dynamically-allocated)
  - [Width, height and type getters](#width-height-and-type-getters)
- [Format settings](#format-settings)
  - [Rotation](#rotation)
  - [Endianness](#endianness)
- [Image data getter](#image-data-getter)
- [Matrix transformation](#matrix-transformation)
- [Non-drawing graphics features](#non-drawing-graphics-features)
  - [Dirty area](#dirty-area)
  - [Clipping](#clipping)
  - [Scrolling](#scrolling)



# Construction

## From existing
### `Buffer clone() const`
You can call `clone` on a `pax::Buffer` object to get an exact copy (image data, settings and matrix stack).

---
### `Buffer(pax_buf_t *existing)`
Creates a **non-owning** reference/wrapper to/around a C API `pax_buf_t`.
This allows C++ API-based function to be called using the C API `pax_buf_t`.

## Preallocated
### `Buffer(void *preallocated, int width, int height, pax_buf_type_t type)`
Creates a buffer object with a preallocated framebuffer.

PAX assumes you know the required size, which can be computed using `pax::Buffer::computeSize`.

---
### `static constexpr size_t computeSize(int width, int height, pax_buf_type_t type)`
Computes the required byte capacity to store image data of a certain size and type.
Things like endianness and rotation do not affect this number (and aren't arguments for that reason).


## Dynamically allocated
### `Buffer(int width, int height, pax_buf_type_t type)`
Creates a buffer using dynamically allocated memory (which PAX does for you).
This is the easiest constructor to use correctly.


## Width, height and type getters
### `int width() const`
Get the width, in pixels, of the buffer.

---
### `int height() const`
Get the height, in pixels, of the buffer.

---
### `float widthf() const`
Get the width, in pixels, of the buffer.
Casts output to `float`.

---
### `float heightf() const`
Get the height, in pixels, of the buffer.
Casts output to `float`.

---
### `pax_buf_type_t type() const`
Get the type of the buffer.



# Format settings

## Rotation
Depending on the mounted orientation of your display,
you may want to rotate the graphics to be right-side up.

### `void setRotation(int rotation)`
Set rotation of the buffer.
0 is not rotated, each unit is one quarter turn counter-clockwise.

---
### `int getRotation() const`
SGt rotation of the buffer.
0 is not rotated, each unit is one quarter turn counter-clockwise.


## Endianness
If your display has a different *endianness* than your CPU, activate `reverseEndianness`.
Having such a disagreement causes your graphics have the right shape but wrong colors.

### `void reverseEndianness(bool reversed)`
Enable reversed endianness mode.
This causes endiannes to be internally stored as reverse of native.
This operation does not update data stored in the buffer; it will become invalid.

---
### `bool isReverseEndianness()`
Tells whether the endianness has been reversed.



# Image data getter
If you use PAX and dynamically allocate the buffer, you can use `getPixelBuffer` to retrieve a pointer o the allocated buffer.

This operation is useful when e.g. you need to update a display with a new image from your buffer.

### `void *getPixelBuffer()`
### `const void *getPixelBuffer() const`
Gets a pointer to the image data.
See [this link](../pixelformat.md) for the format.

---
### `size_t getPixelBufferSize() const`
Get the byte size of the image data.



# Matrix transformation
*Matrix transformation* is one of PAX' flagship features.
It allows the user to easily change the position, shape and size of almost any drawing and allows for unique effects especially with images and text.

Specifically, PAX uses
*[affine transformations](https://en.wikipedia.org/wiki/Affine_transformation)
([video explanation](https://www.youtube.com/watch?v=kYB8IZa5AuE))*
stored in a stack of matrices.
For more details, see [`pax::Matrix2f`](matrix.md#paxmatrix2f).


### `void pushMatrix()`
Push the current matrix to the matrix stack.

---
### `void popMatrix()`
Pop a matrix from the stack and set the current matrix to it.

---
### `void clearMatrix()`
Clear the matrix stack (no more popMatix required) and reset the current matrix to identity (no transformation).

---
### `void clearMatrix(bool full)`
If full: clears the entire matrix stack,
Otherwise clears just the current matrix.

---
### `void applyMatrix(Matrix2f matrix)`
Applies the given 2D matrix to the current transformation using matrix multiplication.
Internally, this is what `scale`, `translate`, etc. call to do their jobs.

---
### `void scale(float x, float y)`
Scale the current view non-uniformly (`x > 1` is wider, `y > 1` is taller).

---
### `void scale(float factor)`
Scale up the current view uniformly (`factor > 1` is larger).

---
### `void translate(float x, float y)`
Moves around the current view.

---
### `void shear(float x, float y)`
Shears the current view, causing squares to warp into diamond shapes.
Positive X causes the points above the origin to move to the right.
Positive Y causes the points to the right of the origin to move down.

---
### `void rotate(float angle)`
Rotate the current view around the origin, angles in radians.

---
### `void rotateAround(float x, float y, float angle)`
Helper to rotate the current view around a point relative to the origin.



# Non-drawing graphics features

## Dirty area
The *dirty area* feature is designed to allow for optimisation of both *drawing* and *writing to display*.
Fundamentally, it is a rectangle which tells you the area in which there has been drawing since last `markClean` call.

*Note: The dirty area is one of the features not affected by [transformations](#matrix-transformation).*

### `bool isDirty() const`
Whether or not there has been drawing since last markClean call.

**Note: `scroll` and `background` *do not* mark the buffer as dirty.**

---
### `Recti getDirtyRect() const`
Get the rectangle in which the buffer may be dirty.
Returns garbage if the buffer is not dirty.

*Note: Implementations are allowed to be greedy, but not conservative, with this value.*

---
### `void markClean()`
Mark the buffer as clean.

---
### `void markDirty()`
Mark the entire buffer as dirty.

---
### `void markDirty(int x, int y)`
Mark a single pixel as dirty.

---
### `void markDirty(int x, int y, int width, int height)`
Mark a rectangular region as dirty.


## Clipping
*Clipping* can be used as a complementary feature to the *dirty area*,
but is also useful on its own.

Setting the clipping window to equal the dirty area can be an effective way of only updating that which needs to be.
Otherwise, it can be used for things like preventing overflow of text.

It works by preventing the drawing of shapes outside the clipping rectangle, and is effectively a free setting to enable.

*Note: Clipping, like the dirty area, is one of the features not affected by [transformations](#matrix-transformation).*

### `void clip(int x, int y, int width, int height)`
Apply a clipping rectangle to the buffer, overriding any previous settings.

---
### `Recti getClip() const`
Obtain the current clipping rectangle.
If clipping is disabled, this rectangle will cover the entire buffer.

---
### `void noClip()`
Disable the clipping rectangle.

*Note: Identical to calling `myBuffer.clip(0, 0, myBuffer.widthf(), myBuffer.heightf())`.*


## Scrolling
*Scrolling* is a relatively obscure feature that's useful for things like 2D games and terminal emulators.

You can scroll the contents of a buffer in any direction, including X and Y simultaneously.

### `void scroll(Color placeholder, int x, int y)`
Scroll the contents of the buffer, filling the "revealed area" with a placeholder color.
If `placeholder` has an alpha of 0, the "revealed area" is not filled.

---
### `void scroll(int x, int y)`
Scroll the contents of the buffer, filling the "revealed area" with the current `fillColor` setting.
