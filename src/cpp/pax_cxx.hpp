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
typedef std::function<Color(Color tint, Color existing, int x, int y, float u, float v, void *args)> ShaderFunc;

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
		// Requires explicit copy constructor.
		Shader(const Shader &other);
		// Requires explicit move constructor.
		Shader(const Shader &&other);
		// Requires explicit copy operator.
		Shader &operator=(const Shader &other);
		// Requires explicit move operator.
		Shader &operator=(const Shader &&other);
		
		// Deletion operator.
		~Shader();
		
		// Apply this shader to a pixel.
		Color apply(Buffer &buffer, Color tint, Color existing, int x, int y, float u, float v) const;
		// Get a shader object for using in C PAX APIs.
		pax_shader_t *getInternal();
		// Get a shader object for using in C PAX APIs.
		const pax_shader_t *getInternal() const;
};

class Buffer {
	protected:
		// The PAX buffer this wrapper is of.
		pax_buf_t *internal;
		// Whether to also delete the internal buffer.
		bool deleteInternal;
		
		friend class InlineElement;
		friend class TextElement;
		friend class InlineImage;
		friend class TextBox;
		
	public:
		// Default color to use for drawing filled shapes.
		Color fillColor;
		// Default color to use for drawing outlined shapes.
		Color lineColor;
		
		// Compute required size of a certain buffer type.
		// Returns byte capacity requirement for preallocated buffers.
		static constexpr size_t computeSize(int width, int height, pax_buf_type_t type) {
			return PAX_BUF_CALC_SIZE(width, height, type);
		}
		
		// Make an empty wrapper.
		Buffer();
		// Make a wrapper using an existing PAX buffer.
		Buffer(pax_buf_t *existing);
		// Make a wrapper of a new buffer.
		Buffer(int width, int height, pax_buf_type_t type);
		// Make a wrapper of a new buffer, with preallocated memory.
		// This memory will not be free()d by default.
		Buffer(void *preallocated, int width, int height, pax_buf_type_t type);
		
		// Buffer is not implicitly copyable.
		Buffer(Buffer &) = delete;
		// Buffer is not trivially movable.
		Buffer(Buffer &&);
		// Buffer is not implicitly copyable.
		Buffer& operator=(Buffer &) = delete;
		// Buffer is not trivially movable.
		Buffer& operator=(Buffer &&);
		
		// Get an explicit copy-by-value of this buffer.
		Buffer clone() const;
		
		// Set rotation of the buffer.
		// 0 is not rotated, each unit is one quarter turn counter-clockwise.
		void setRotation(int rotation);
		// Get rotation of the buffer.
		// 0 is not rotated, each unit is one quarter turn counter-clockwise.
		int getRotation() const;
		// Scroll the buffer, filling with a placeholder color.
		void scroll(Color placeholder, int x, int y);
		// Scroll the buffer using fill color as placeholder.
		void scroll(int x, int y) { scroll(fillColor, x, y); }
		// Enable reversed endianness mode.
		// This causes endiannes to be internally stored as reverse of native.
		// This operation does not update data stored in the buffer; it will become invalid.
		void reverseEndianness(bool reversed);
		// Tells whether the endianness has been reversed.
		bool isReverseEndianness();
		// Get a pointer to the underlying C API pax_buf_t*.
		// Note: Doing so is less memory safe than to use the C++ API, but still compatible.
		pax_buf_t *getInternal();
		// Get a pointer to the underlying C API pax_buf_t*.
		// Note: Doing so is less memory safe than to use the C++ API, but still compatible.
		const pax_buf_t *getInternal() const;
		// Get a pointer to the image data.
		// See <../docs/pixelformat.md> for the format.
		void *getPixelBuffer();
		// Get a pointer to the image data.
		// See <../docs/pixelformat.md> for the format.
		const void *getPixelBuffer() const;
		// Get the byte size of the image data.
		size_t getPixelBufferSize() const;
		
		// Deletion operator.
		~Buffer();
		
		// Get the width, in pixels, of the buffer.
		int width() const;
		// Get the height, in pixels, of the buffer.
		int height() const;
		// Get the width, in pixels, of the buffer.
		float widthf() const { return (float) width(); }
		// Get the height, in pixels, of the buffer.
		float heightf() const { return (float) height(); }
		// Get the type of the buffer.
		pax_buf_type_t type() const;
		
		// Fills the entire buffer with the given color.
		void background(Color color);
		
		// Draws a rectangle with the default color.
		void drawRect(float x, float y, float width, float height);
		// Draws a rectangle with a custom color.
		void drawRect(Color color, float x, float y, float width, float height);
		// Draws a rectangle with a custom shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawRect(const Shader &shader, const Quadf *uvs, float x, float y, float width, float height);
		// Draws a rectangle with a custom color and shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawRect(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float width, float height);
		// Outlines a rectangle with the default outline color.
		void outlineRect(float x, float y, float width, float height);
		// Outlines a rectangle with a custom outline color.
		void outlineRect(Color color, float x, float y, float width, float height);
		// Outlines a rectangle with the default outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineRect(const Shader &shader, const Quadf *uvs, float x, float y, float width, float height);
		// Outlines a rectangle with a custom outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineRect(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float width, float height);
		
		// Draws a triangle with the default color.
		void drawTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color.
		void drawTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
		void drawTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color and shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
		void drawTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with the default outline color.
		void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with a custom outline color.
		void outlineTri(Color color, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with the default outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
		void outlineTri(const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with a custom outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
		void outlineTri(Color color, const Shader &shader, const Trif *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		
		// Draws a circle around the given point with the default color.
		void drawCircle(float x, float y, float radius);
		// Draws a circle around the given point with a custom color.
		void drawCircle(Color color, float x, float y, float radius);
		// Draws a circle around the given point with a custom shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawCircle(const Shader &shader, const Quadf *uvs, float x, float y, float radius);
		// Draws a circle around the given point with a custom color and shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawCircle(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius);
		// Outlines a circle with the default outline color.
		void outlineCircle(float x, float y, float radius);
		// Outlines a circle with a custom outline color.
		void outlineCircle(Color color, float x, float y, float radius);
		// Outlines a circle with the default outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineCircle(const Shader &shader, const Quadf *uvs, float x, float y, float radius);
		// Outlines a circle with a custom outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineCircle(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius);
		
		// Draws an arc around the given point with the default color.
		void drawArc(float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color.
		void drawArc(Color color, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawArc(const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color and shader.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void drawArc(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with the default outline color.
		void outlineArc(float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with a custom outline color.
		void outlineArc(Color color, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with the default outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineArc(const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with a custom outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
		void outlineArc(Color color, const Shader &shader, const Quadf *uvs, float x, float y, float radius, float startangle, float endangle);
		
		// Draws a line with the default outline color.
		void drawLine(float x0, float y0, float x1, float y1);
		// Draws a line with a custom outline color.
		void drawLine(Color color, float x0, float y0, float x1, float y1);
		// Draws a line with the default outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0).
		void drawLine(const Shader &shader, Linef* uvs, float x0, float y0, float x1, float y1);
		// Draws a line with a custom outline color.
		// If uvs is NULL, a default will be used (0,0; 1,0).
		void drawLine(Color color, const Shader &shader, Linef* uvs, float x0, float y0, float x1, float y1);
		
		// Outlines an arbitrary shape.
		void outline(float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(Color color, float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(const Shader &shader, float x, float y, Shape &shape);
		// Outlines an arbitrary shape.
		void outline(Color color, const Shader &shader, float x, float y, Shape &shape);
		
		// Draws an arbitrary shape.
		void draw(float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(Color color, float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(const Shader &shader, float x, float y, Shape &shape);
		// Draws an arbitrary shape.
		void draw(Color color, const Shader &shader, float x, float y, Shape &shape);
		
		// Draws an image stored in another buffer.
		void drawImage(const pax_buf_t *image, float x, float y);
		// Draws an image stored in another buffer.
		void drawImage(const pax_buf_t *image, float x, float y, float width, float height);
		// Draws an image stored in another buffer.
		void drawImage(const Buffer &image, float x, float y) { drawImage(image.internal, x, y); }
		// Draws an image stored in another buffer.
		void drawImage(const Buffer &image, float x, float y, float width, float height) { drawImage(image.internal, x, y, width, height); }
		
		// Draws an image stored in another buffer.
		// Assumes the image is opaque and ignores and transparency.
		void drawImageOpaque(const pax_buf_t *image, float x, float y);
		// Draws an image stored in another buffer.
		// Assumes the image is opaque and ignores and transparency.
		void drawImageOpaque(const pax_buf_t *image, float x, float y, float width, float height);
		// Draws an image stored in another buffer.
		// Assumes the image is opaque and ignores and transparency.
		void drawImageOpaque(const Buffer &image, float x, float y) { drawImageOpaque(image.internal, x, y); }
		// Draws an image stored in another buffer.
		// Assumes the image is opaque and ignores and transparency.
		void drawImageOpaque(const Buffer &image, float x, float y, float width, float height) { drawImageOpaque(image.internal, x, y, width, height); }
		
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
		void applyMatrix(Matrix2f matrix);
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
		Color getPixel(int x, int y) const;
		// Sets color at the given point.
		void setPixel(Color color, int x, int y);
		// Gets the raw pixel data (before color converion) at the given point.
		Color getPixelRaw(int x, int y) const;
		// Sets raw pixel data (before color converion) at the given point.
		void setPixelRaw(Color color, int x, int y);
		// Overlays the color at the given point (for transparent drawing).
		void mergePixel(Color color, int x, int y);
		
		// Whether or not there has been drawing since last markClean call.
		bool isDirty() const;
		// Gets the rectangle in which it is dirty.
		Recti getDirtyRect() const;
		// Mark the buffer as clean.
		void markClean();
		// Mark the entire buffer as dirty.
		void markDirty();
		// Mark a single pixel as dirty.
		void markDirty(int x, int y);
		// Mark a rectangular region as dirty.
		void markDirty(int x, int y, int width, int height);
		
		// Apply a clip rectangle to the buffer.
		// Anything outside of the clip will not be drawn.
		// This is an operation that ignores matrix transforms (translate, rotate, etc.).
		void clip(int x, int y, int width, int height);
		// Disable clipping.
		// Any effects of previous clip calls are nullified.
		void noClip();
		// Obtain a copy of the current clip rect.
		Recti getClip() const;
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

// Splits ARGB.
static inline void undo_argb(Color in, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b) {
	a = in >> 24;
	r = in >> 16;
	g = in >> 8;
	b = in;
}
// Splits RGB.
static inline void undo_rgb(Color in, uint8_t &r, uint8_t &g, uint8_t &b) {
	r = in >> 16;
	g = in >> 8;
	b = in;
}

// Converts HSV to RGB, ranges are 0-255.
Color hsv     (uint8_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255.
Color ahsv    (uint8_t a, uint8_t h, uint8_t s, uint8_t v);
// Converts HSV to RGB, ranges are 0-359, 0-99, 0-99.
Color hsv_alt (uint16_t h, uint8_t s, uint8_t v);
// Converts AHSV to ARGB, ranges are 0-255, 0-359, 0-99, 0-99.
Color ahsv_alt(uint8_t a, uint16_t h, uint8_t s, uint8_t v);

// Converts ARGB into AHSV, ranges are 0-255.
void undo_ahsv    (Color in, uint8_t &a, uint8_t &h, uint8_t &s, uint8_t &v);
// Converts RGB into HSV, ranges are 0-255.
void undo_hsv     (Color in, uint8_t &h, uint8_t &s, uint8_t &v);
// Converts ARGB into AHSV, ranges are 0-359, 0-99, 0-99.
void undo_ahsv_alt(Color in, uint8_t &a, uint8_t &h, uint8_t &s, uint8_t &v);
// Converts RGB into HSV, ranges are 0-255, 0-359, 0-99, 0-99.
void undo_hsv_alt (Color in, uint8_t &h, uint8_t &s, uint8_t &v);

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
