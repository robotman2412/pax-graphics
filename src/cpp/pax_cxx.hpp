/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once

#include <pax_types.h>

#ifdef __cplusplus

namespace pax {
class Buffer;
}

#include <functional>
#include <pax_cxx_shape.hpp>

// This serves as a C++ wrapper which is fully compatible with the normal C version of PAX graphics.
namespace pax {

// A C++ ShaderFunc thing for use with lambdas and such.
typedef std::function<pax_col_t(pax_col_t tint, int x, int y, float u, float v, void *args)> ShaderFunc;

// A helper data structure for C++ shaders.
typedef struct {
	ShaderFunc *callback;
	void *args;
} ShaderContent;

// Shader class stub.
class Shader {
	protected:
		// The internal shader object to run.
		pax_shader_t internal;
		// Optional C++ shader function.
		ShaderContent cxxShaderCtx;
		// Wehther this is a C++ type shader.
		bool isCxx;
		// Whether this shader actually does anything.
		bool active;
		
	public:
		// Make an empty shader.
		// This shader won't change anything.
		Shader();
		// Make a shader using an existing PAX shader.
		Shader(pax_shader_t *existing);
		// Make a shader a C++ version of shader callback.
		Shader(ShaderFunc callback, void *context);
		
		// Deletion operator.
		~Shader();
		
		// Apply this shader to a pixel.
		pax_col_t apply(pax_col_t tint, int x, int y, float u, float v);
		// Get a shader object for using in C PAX APIs.
		pax_shader_t *getInternal();
};

class Buffer {
	protected:
		// The PAX buffer this wrapper is of.
		pax_buf_t *internal;
		// Whether to also delete the internal buffer.
		bool deleteInternal;
		
	public:
		// Default color to use for drawing filled shapes.
		pax_col_t fillColor;
		// Default color to use for drawing outlined shapes.
		pax_col_t lineColor;
		
		// Make an empty wrapper.
		Buffer();
		// Make a wrapper using an existing PAX buffer.
		Buffer(pax_buf_t *existing);
		// Make a wrapper of a new buffer.
		Buffer(int width, int height, pax_buf_type_t type);
		// Make a wrapper of a new buffer, with preallocated memory.
		// This memory will not be free()d by default.
		Buffer(void *preallocated, int width, int height, pax_buf_type_t type);
		
		// Deletion operator.
		~Buffer();
		
		// Fills the entire buffer with the given color.
		void background(pax_col_t color);
		
		// Draws a rectangle with the default color.
		void drawRect(float x, float y, float width, float height);
		// Draws a rectangle with a custom color.
		void drawRect(pax_col_t color, float x, float y, float width, float height);
		// Draws a rectangle with a custom shader.
		// Shader is ignored if NULL.
		void drawRect(Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Draws a rectangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawRect(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Outlines a rectangle with the default outline color.
		void outlineRect(float x, float y, float width, float height);
		// Outlines a rectangle with a custom outline color.
		void outlineRect(pax_col_t color, float x, float y, float width, float height);
		
		// Draws a triangle with the default color.
		void drawTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color.
		void drawTri(pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom shader.
		// Shader is ignored if NULL.
		void drawTri(Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawTri(pax_col_t color, Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with the default outline color.
		void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with a custom outline color.
		void outlineTri(pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
		
		// Draws a circle around the given point with the default color.
		void drawCircle(float x, float y, float radius);
		// Draws a circle around the given point with a custom color.
		void drawCircle(pax_col_t color, float x, float y, float radius);
		// Draws a circle around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawCircle(Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Draws a circle around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawCircle(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Outlines a circle with the default outline color.
		void outlineCircle(float x, float y, float radius);
		// Outlines a circle with a custom outline color.
		void outlineCircle(pax_col_t color, float x, float y, float radius);
		
		// Draws an arc around the given point with the default color.
		void drawArc(float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color.
		void drawArc(pax_col_t color, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawArc(Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawArc(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with the default outline color.
		void outlineArc(float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with a custom outline color.
		void outlineArc(pax_col_t color, float x, float y, float radius, float startangle, float endangle);
		
		// Draws a line with the default outline color.
		void drawLine(float x0, float y0, float x1, float y1);
		// Draws a line with a custom outline color.
		void drawLine(pax_col_t color, float x0, float y0, float x1, float y1);
		
		// Push the matrix stack.
		void pushMatrix();
		// Pop the matrix stack.
		void popMatrix();
		// Clear the matrix stack (no mode popMatix calls left) and reset the current matrix to identity (no transformation).
		void clearMatrix();
		// If full: clears the entire matrix stack,
		// Otherwise clears just the current matrix.
		void clearMatrix(bool full);
		
		// Applies a given 2D matrix to the current by matrix multiplication.
		void applyMatrix(matrix_2d_t matrix);
		// Scales the current view.
		void scale(float x, float y);
		// Scales the current view.
		void scale(float factor);
		// Moves around the current view.
		void translate(float x, float y);
		// Shears the current view.
		// Positive X causes the points above the origin to move to the right.
		// Positive Y causes the points to the right of the origin to move down.
		void shear(float x, float y);
		// Rotates the current view around the origin, angles in radians.
		void rotate(float angle);
		// Rotates the current view around a given point.
		void rotateAround(float x, float y, float angle);
		
		// Gets color at the given point.
		pax_col_t getPixel(int x, int y);
		// Sets color at the given point.
		void setPixel(pax_col_t color, int x, int y);
		// Overlays the color at the given point (for transparent drawing).
		void mergePixel(pax_col_t color, int x, int y);
		
		// Whether or not there has been drawing since last markClean call.
		bool isDirty();
		// Gets the rectangle in which it is dirty.
		pax_rect_t getDirtyRect();
		// Mark the buffer as clean.
		void markClean();
		// Mark the entire buffer as dirty.
		void markDirty();
		// Mark a single pixel as dirty.
		void markDirty(int x, int y);
		// Mark a rectangular region as dirty.
		void markDirty(int x, int y, int width, int height);
		
		// Apply a clip rectangle to the buffer.
		// Anothing outside of the clip will not be drawn.
		// This is an operation that ignores matrix transforms (translate, rotate, etc.).
		void clip(int x, int y, int width, int height);
		// Disable clipping.
		// Any effects of previous clip calls are nullified.
		void noClip();
};

// Multiplicatively decreases alpha based on a float.
static inline pax_col_t reduceAlpha(pax_col_t in, float coeff) {
	return ((pax_col_t) (((in & 0xff000000) * coeff)) & 0xff000000) | (in & 0x00ffffff);
}
// Combines RGB.
static inline pax_col_t rgb(uint8_t r, uint8_t g, uint8_t b) {
	return 0xff000000 | (r << 16) | (g << 8) | b;
}
// Combines ARGB.
static inline pax_col_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
	return (a << 24) | (r << 16) | (g << 8) | b;
}
// Converts HSV to RGB.
pax_col_t hsv             (uint8_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB.
pax_col_t ahsv            (uint8_t a, uint8_t h, uint8_t s, uint8_t v);
// Linearly interpolates between from and to, including alpha.
pax_col_t lerp            (uint8_t part, pax_col_t from, pax_col_t to);
// Merges the two colors, based on alpha.
pax_col_t merge           (pax_col_t base, pax_col_t top);
// Tints the color, commonly used for textures.
pax_col_t tint            (pax_col_t col, pax_col_t tint);

}
#endif // __cplusplus