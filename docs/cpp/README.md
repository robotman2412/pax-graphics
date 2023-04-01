# PAX graphics documentation: C++ API

The PAX graphics stack is being developed for the [MCH2022 badge](https://bodge.team/docs/badges/mch2022/).
It's goal is to allow anyone to, in C and/or C++, use a powerful list of drawing features with good optimisation.

This library is the successor of the revised graphics API for [the old badge.team firmware](https://github.com/badgeteam/ESP32-platform-firmware).

For supported platforms, [see this link](../supported-platforms.md).


## PAX Docs overview

- [Getting started](#getting-started)
- [API overview](#api-overview)
- [Example code](#example-code)
  - [`pax::Buffer`](#example-pax-buffer)
    - [`Setup functions`](#example-setup)
    - [`Scrolling function`](#example-scrolling)
    - [`Basic drawing functions`](#example-basic-drawing)
  - [`pax::Shader`](#example-pax-shader)
  - [Vector types](#example-pax-vectors)
    - [`pax::Vec2f`](#example-pax-vec2f)
    - [`pax::Rectf`](#example-pax-rectf)
    - And more
  - [`pax::Matrix2f`](#example-pax-matrix2f)



# Getting started



# API overview

This chapter contains links to complete specifications of every type and every function in and used by the C++ API:
- Drawing functions:
  - [Basic drawing functions](drawing.md#basic-drawing-functions)
    - [Circles](drawing.md#circles)
    - [Arcs](drawing.md#arcs)
    - [Rectangles](drawing.md#rectangles)
    - [Lines](drawing.md#rectangles)
  - [Outlines](drawing.md#outlines)
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
  - [`pax::Shader`](shaders.md#pax-shader)
  - [Purpose](shaders.md#purpose)
  - [Creation](shaders.md#creation)
- [Vector types](matrix.md)
  - [`pax::Vec2f` and `pax::Pointf`](matrix.md#pax-vec2f)
  - [`pax::BiVec2f` and `pax::Linef`](matrix.md#pax-bivec2f)
  - [`pax::TriVec2f` and `pax::Trif`](matrix.md#pax-trivec2f)
  - [`pax::QuadVec2f` and `pax::Quadf`](matrix.md#pax-quadvec2f)
  - [`pax::Vec2i`](matrix.md#pax-vec2i)
- [`pax::Rectf`](matrix.md#pax-rectf)
- [`pax::Recti`](matrix.md#pax-recti)
- [`pax::Matrix2f`](matrix.md#pax-matrix2f)



# Example code

This chapter contains several examples, at least one per major feature:
- [`pax::Buffer`](#example-pax-buffer)
  - [Setup functions](#example-setup)
  - [Scrolling function](#example-scrolling)
  - [Basic drawing functions](#example-basic-drawing)
  - [Text drawing](#example-text-drawing)
- [`pax::Shader`](#example-pax-shader)
  - [Purpose](#example-pax-shader)
  - [List of uses](#uses-for-pax-shader)
  - [Conversion](#conversions-for-pax-shader)
- [`pax::Vec2f` and other vector types](#example-pax-vectors)
  - [`Purpose`](#example-pax-vectors)
  - [`List of uses`](#uses-for-vectors)
  - [`pax::Vec2f` and `pax::Pointf`](#example-pax-vec2f)
  - [`pax::BiVec2f` and `pax::Linef`](#example-pax-bivec2f)
  - [`pax::TriVec2f` and `pax::Trif`](#example-pax-trivec2f)
  - [`pax::QuadVec2f` and `pax::Quadf`](#example-pax-quadvec2f)
  - [`pax::Rectf`](#example-pax-rectf)
  - [`pax::Vec2i` and `pax::Recti`](#integer-vector-counterparts)
- [`pax::Matrix2f`](#example-pax-matrix2f)
  - [Purpose](#example-pax-matrix2f)
  - [List of uses](#uses-for-pax-matrix2f)



# Example: `pax::Buffer`

The buffer is the primary type in PAX.
It stores the image data as well as any information required to manipulate it.

This section is divided into four parts:
- [Setup functions](#example-setup) (how to create your `pax::Buffer`)
- [Scrolling function](#example-scrolling) (useful for games and terminals)
- [Basic drawing functions](#example-basic-drawing) (the bulk of what `pax::Buffer` does)
- [Text drawing](#example-text-drawing) (simpler edition of text drawing)
