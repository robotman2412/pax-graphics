# PAX Docs: `pax::Buffer`

This document covers the setup and related functions of `pax::Buffer`.
For drawing functions, see [drawing functions](drawing.md).

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
TODO.



# Non-drawing graphics features

## Dirty area

## Clipping

## Scrolling
