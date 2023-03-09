/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

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

#ifndef PAX_CXX_HPP
#define PAX_CXX_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

namespace pax {
class Buffer;
typedef pax_col_t Color;
}

#include <functional>
#include <pax_cxx_shape.hpp>
#include <pax_cxx_text.hpp>

// This serves as a C++ wrapper which is fully compatible with the normal C version of PAX graphics.
namespace pax {

// A C++ ShaderFunc thing for use with lambdas and such.
typedef std::function<Color(Color existing, Color tint, int x, int y, float u, float v, void *args)> ShaderFunc;

// A helper data structure for C++ shaders.
typedef struct {
	std::shared_ptr<ShaderFunc> callback;
	void *args;
} ShaderContent;

// Shader class stub.
class Shader {
	protected:
		// The internal shader object to run.
		pax_shader_t internal;
		// Optional C++ shader function.
		ShaderContent cxxShaderCtx;
		// Whether this is a C++ type shader.
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
		Color apply(Color tint, int x, int y, float u, float v);
		// Get a shader object for using in C PAX APIs.
		pax_shader_t *getInternal();
};

class Buffer {
	protected:
		// The PAX buffer this wrapper is of.
		pax_buf_t *internal;
		// Whether to also delete the internal buffer.
		bool deleteInternal;
		
		friend class InlineElement;
		friend class TextElement;
		friend class ImageElement;
		friend class TextBox;
		
	public:
		// Default color to use for drawing filled shapes.
		Color fillColor;
		// Default color to use for drawing outlined shapes.
		Color lineColor;
		
		// Make an empty wrapper.
		Buffer();
		// Make a wrapper using an existing PAX buffer.
		Buffer(pax_buf_t *existing);
		// Make a wrapper of a new buffer.
		Buffer(int width, int height, pax_buf_type_t type);
		// Make a wrapper of a new buffer, with preallocated memory.
		// This memory will not be free()d by default.
		Buffer(void *preallocated, int width, int height, pax_buf_type_t type);
		// Enable reversed endianness mode.
		// This causes endiannes to be internally stored as reverse of native.
		// This operation does not update data stored in the buffer; it will become invalid.
		void reverseEndianness(bool reversed);
		// Get a pointer to the underlying C API pax_buf_t*.
		// Note: Doing so is less memory safe than to use the C++ API, but still compatible.
		pax_buf_t *getInternal();
		// Get a pointer to the memory stored in the pixel buffer.
		// The arrangement is left-to-right then top-to-bottom, packed (sub byte-aligned rows will partially share a byte with the next).
		void *getPixelBuffer();
		
		// Deletion operator.
		~Buffer();
		
		// Get the width, in pixels, of the buffer.
		int width();
		// Get the height, in pixels, of the buffer.
		int height();
		// Get the type of the buffer.
		pax_buf_type_t type();
		
		// Fills the entire buffer with the given color.
		void background(Color color);
		
		// Draws a rectangle with the default color.
		void drawRect(float x, float y, float width, float height);
		// Draws a rectangle with a custom color.
		void drawRect(Color color, float x, float y, float width, float height);
		// Draws a rectangle with a custom shader.
		// Shader is ignored if NULL.
		void drawRect(Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Draws a rectangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawRect(Color color, Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Outlines a rectangle with the default outline color.
		void outlineRect(float x, float y, float width, float height);
		// Outlines a rectangle with a custom outline color.
		void outlineRect(Color color, float x, float y, float width, float height);
		
		// Draws a triangle with the default color.
		void drawTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color.
		void drawTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom shader.
		// Shader is ignored if NULL.
		void drawTri(Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawTri(Color color, Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with the default outline color.
		void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with a custom outline color.
		void outlineTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);
		
		// Draws a circle around the given point with the default color.
		void drawCircle(float x, float y, float radius);
		// Draws a circle around the given point with a custom color.
		void drawCircle(Color color, float x, float y, float radius);
		// Draws a circle around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawCircle(Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Draws a circle around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawCircle(Color color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Outlines a circle with the default outline color.
		void outlineCircle(float x, float y, float radius);
		// Outlines a circle with a custom outline color.
		void outlineCircle(Color color, float x, float y, float radius);
		
		// Draws an arc around the given point with the default color.
		void drawArc(float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color.
		void drawArc(Color color, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawArc(Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawArc(Color color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with the default outline color.
		void outlineArc(float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with a custom outline color.
		void outlineArc(Color color, float x, float y, float radius, float startangle, float endangle);
		
		// Draws a line with the default outline color.
		void drawLine(float x0, float y0, float x1, float y1);
		// Draws a line with a custom outline color.
		void drawLine(Color color, float x0, float y0, float x1, float y1);
		
		// Outlines an arbitrary shape.
		void outline(float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(float x, float y, Shape *shape);
		// Outlines an arbitrary shape.
		void outline(Color color, float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(Color color, float x, float y, Shape *shape);
		// Outlines an arbitrary shape.
		void outline(Color color, Shader *shader, float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(Color color, Shader *shader, float x, float y, Shape *shape);
		
		// Draws an arbitrary shape.
		void draw(float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(float x, float y, Shape *shape);
		// Draws an arbitrary shape.
		void draw(Color color, float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(Color color, float x, float y, Shape *shape);
		// Draws an arbitrary shape.
		void draw(Color color, Shader *shader, float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(Color color, Shader *shader, float x, float y, Shape *shape);
		
		// Draws an image stored in another buffer.
		void drawImage(pax_buf_t *image, float x, float y);
		// Draws an image stored in another buffer.
		void drawImage(pax_buf_t *image, float x, float y, float width, float height);
		// Draws an image stored in another buffer.
		void drawImage(Buffer &image, float x, float y) { drawImage(image.internal, x, y); }
		// Draws an image stored in another buffer.
		void drawImage(Buffer &image, float x, float y, float width, float height) { drawImage(image.internal, x, y, width, height); }
		
		// Calculate the size of the string with the given font.
		// Size is before matrix transformation.
		static Vec2f stringSize(const pax_font_t *font, float font_size, std::string text);
		// Draw a string with the given font and return it's size.
		// Size is before matrix transformation.
		Vec2f drawString(const pax_font_t *font, float font_size, float x, float y, std::string text);
		// Draw a string with the given font and return it's size.
		// Size is before matrix transformation.
		Vec2f drawString(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text);
		// Draw a string with the given font and return it's size, center-aligning every line individually.
		// Size is before matrix transformation.
		Vec2f drawStringCentered(const pax_font_t *font, float font_size, float x, float y, std::string text);
		// Draw a string with the given font and return it's size, center-aligning every line individually.
		// Size is before matrix transformation.
		Vec2f drawStringCentered(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text);
		
		// Push the matrix stack.
		void pushMatrix();
		// Pop the matrix stack.
		void popMatrix();
		// Clear the matrix stack (no more popMatix required) and reset the current matrix to identity (no transformation).
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
		Color getPixel(int x, int y);
		// Sets color at the given point.
		void setPixel(Color color, int x, int y);
		// Overlays the color at the given point (for transparent drawing).
		void mergePixel(Color color, int x, int y);
		
		// Whether or not there has been drawing since last markClean call.
		bool isDirty();
		// Gets the rectangle in which it is dirty.
		Rectf getDirtyRect();
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

/* ============ COLORS =========== */

// Multiplicatively decreases alpha based on a float.
static inline Color reduceAlpha(Color in, float coeff) {
	return ((Color) (((in & 0xff000000) * coeff)) & 0xff000000) | (in & 0x00ffffff);
}
// Combines RGB.
static inline Color rgb(uint8_t r, uint8_t g, uint8_t b) {
	return 0xff000000 | (r << 16) | (g << 8) | b;
}
// Combines ARGB.
static inline Color argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
	return (a << 24) | (r << 16) | (g << 8) | b;
}
// Converts HSV to RGB, ranges are 0-255.
Color hsv  (uint8_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255.
Color ahsv (uint8_t a, uint8_t h, uint8_t s, uint8_t v);
// Converts HSV to RGB, ranges are 0-360, 0-100, 0-100.
Color hsv_alt (uint16_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255, 0-360, 0-100, 0-100.
Color ahsv_alt(uint8_t a, uint16_t h, uint8_t s, uint8_t v);
// Linearly interpolates between from and to, including alpha.
Color lerp (uint8_t part, Color from, Color to);
// Merges the two colors, based on alpha.
Color merge(Color base, Color top);
// Tints the color, commonly used for textures.
Color tint (Color col, Color tint);

// If multi-core rendering is enabled, wait for the other core.
static inline void join() { pax_join(); }
// Enable multi-core rendering.
static inline void enableMulticore(int core) { pax_enable_multicore(core); }
// Disable multi-core rendering.
static inline void disableMulticore() { pax_disable_multicore(); }

} // namespace pax

#endif // __cplusplus

#endif // PAX_CXX_HPP
