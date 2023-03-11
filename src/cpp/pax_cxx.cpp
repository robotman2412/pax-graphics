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

#include "pax_cxx.hpp"
#include "pax_gfx.h"
#include <string.h>

namespace pax {



// C wrapper function for using C++ shaders.
static Color wrapperCallback(Color existing, Color tint, int x, int y, float u, float v, void *args) {
	ShaderContent *ctx = (ShaderContent *) args;
	return (*ctx->callback)(existing, tint, x, y, u, v, ctx->args);
}

// Make an empty shader.
// This shader won't change anything.
Shader::Shader() {
	isCxx       = false;
	active      = false;
}

// Make a shader using an existing PAX shader.
Shader::Shader(pax_shader_t *existing) {
	internal    = *existing;
	isCxx       = false;
	active      = true;
}

// Make a shader a C++ version of shader callback.
Shader::Shader(ShaderFunc callback, void *args) {
	cxxShaderCtx.callback = std::make_shared<ShaderFunc>(std::move(callback));
	cxxShaderCtx.args     = args;
	internal    = (pax_shader_t) {
		.schema_version    = (uint8_t)  1,
		.schema_complement = (uint8_t) ~1,
		.renderer_id       = PAX_RENDERER_ID_SWR,
		.promise_callback  = NULL,
		.callback          = (void*) wrapperCallback,
		.callback_args     = &cxxShaderCtx,
		.alpha_promise_0   = false,
		.alpha_promise_255 = false,
	};
	isCxx  = true;
	active = true;
}

// Requires explicit copy constructor.
Shader::Shader(const Shader &other) {
	operator=(other);
}

// Requires explicit move constructor.
Shader::Shader(const Shader &&other) {
	operator=(other);
}

// Requires explicit copy operator.
Shader &Shader::operator=(const Shader &other) {
	internal = other.internal;
	cxxShaderCtx = other.cxxShaderCtx;
	isCxx = other.isCxx;
	active = other.active;
	if (isCxx && active) {
		internal.callback_args = &cxxShaderCtx;
	}
	return *this;
}

// Requires explicit move operator.
Shader &Shader::operator=(const Shader &&other) {
	internal = other.internal;
	cxxShaderCtx = std::move(other.cxxShaderCtx);
	isCxx = other.isCxx;
	active = other.active;
	if (isCxx && active) {
		internal.callback_args = &cxxShaderCtx;
	}
	return *this;
}


// Deletion operator.
Shader::~Shader() {
	isCxx  = false;
	active = false;
}


// Apply this shader to a pixel.
Color Shader::apply(Color tint, int x, int y, float u, float v) {
	if (active) {
		// Pass through to the C callback function.
		// TODO
		// return internal.callback(tint, x, y, u, v, internal.callback_args);
		return 0;
	} else {
		return tint;
	}
}

// Get a shader object for using in C PAX APIs.
pax_shader_t *Shader::getInternal() {
	return active ? &internal : NULL;
}



// Make an empty wrapper.
Buffer::Buffer() {
	// Set to safe default.
	internal       = NULL;
	deleteInternal = false;
}

// Make a wrapper using an existing PAX buffer.
Buffer::Buffer(pax_buf_t *existing) {
	internal       = existing;
	deleteInternal = false;
	// Default colors.
	fillColor      = 0xffffffff;
	lineColor      = 0xffffffff;
}

// Make a wrapper of a new buffer.
Buffer::Buffer(int width, int height, pax_buf_type_t type) {
	// Allocate memory.
	internal       = new pax_buf_t;
	deleteInternal = true;
	// Create buffer.
	pax_buf_init(internal, NULL, width, height, type);
	// Default colors.
	fillColor      = 0xffffffff;
	lineColor      = 0xffffffff;
}

// Make a wrapper of a new buffer, with preallocated memory.
// This memory will not be free()d by default.
Buffer::Buffer(void *preallocated, int width, int height, pax_buf_type_t type) {
	// Allocate memory.
	internal       = new pax_buf_t;
	deleteInternal = true;
	// Create buffer.
	pax_buf_init(internal, preallocated, width, height, type);
	// Default colors.
	fillColor      = 0xffffffff;
	lineColor      = 0xffffffff;
}


// Buffer is not trivially movable.
Buffer::Buffer(Buffer &&other) {
	// Take values from other buffer.
	deleteInternal = other.deleteInternal;
	internal       = other.internal;
	fillColor      = other.fillColor;
	lineColor      = other.lineColor;
	
	// Set other buffer to be empty.
	other.deleteInternal = false;
	other.internal       = NULL;
}

// Buffer is not trivially movable.
Buffer& Buffer::operator=(Buffer &&other) {
	// Delete internal buffer, if applicable.
	if (deleteInternal) {
		pax_buf_destroy(internal);
		delete internal;
	}
	
	// Set to safe default.
	internal       = NULL;
	deleteInternal = false;
	
	// Take values from other buffer.
	deleteInternal = other.deleteInternal;
	internal       = other.internal;
	fillColor      = other.fillColor;
	lineColor      = other.lineColor;
	
	// Set other buffer to be empty.
	other.deleteInternal = false;
	other.internal       = NULL;
	
	return *this;
}


// Get an explicit copy-by-value of this buffer.
Buffer Buffer::clone() {
	if (!internal) return Buffer();
	
	// Create a buffer large enough to house the thing.
	Buffer out(internal->width, internal->height, internal->type);
	out.internal->reverse_endianness = internal->reverse_endianness;
	
	// Copy in pixel data.
	size_t cap = (PAX_GET_BPP(internal->type) * internal->width * internal->height + 7) >> 3;
	memcpy(out.getPixelBuffer(), getPixelBuffer(), cap);
	
	// Copy palette.
	if (internal->pallette) {
		size_t pal_cap = sizeof(Color) * internal->pallette_size;
		out.internal->pallette_size = internal->pallette_size;
		out.internal->pallette = (Color *) malloc(pal_cap);
		out.internal->do_free_pal = true;
		memcpy(out.internal->pallette, internal->pallette, pal_cap);
	}
	
	// Copy dirty rect.
	out.internal->dirty_x0 = internal->dirty_x0;
	out.internal->dirty_y0 = internal->dirty_y0;
	out.internal->dirty_x1 = internal->dirty_x1;
	out.internal->dirty_y1 = internal->dirty_y1;
	
	// Copy clip rect.
	out.internal->clip = internal->clip;
	
	// Copy matrix stack.
	auto cur_in  = &internal->stack_2d;
	auto cur_out = &out.internal->stack_2d;
	while (1) {
		*cur_out = *cur_in;
		if (cur_in->parent) {
			cur_out->parent = (matrix_stack_2d_t *) malloc(sizeof(matrix_stack_2d_t));
			cur_out = cur_out->parent;
			cur_in  = cur_in->parent;
		} else {
			break;
		}
	}
	
	return out;
}


// Enable reversed endianness mode.
// This causes endiannes to be internally stored as reverse of native.
// This operation does not update data stored in the buffer; it will become invalid.
void Buffer::reverseEndianness(bool reversed) {
	if (internal) pax_buf_reversed(internal, reversed);
}

// Tells whether the endianness has been reversed.
bool Buffer::isReverseEndianness() {
	return internal && internal->reverse_endianness;
}

// Get a pointer to the underlying C API pax_buf_t*.
// Note: Doing so is less memory safe than to use the C++ API, but still compatible.
pax_buf_t *Buffer::getInternal() {
	return internal;
}

// Get a pointer to the memory stored in the pixel buffer.
// The arrangement is left-to-right then top-to-bottom, packed (sub byte-aligned rows will partially share a byte with the next).
void *Buffer::getPixelBuffer() {
	return internal ? internal->buf : NULL;
}


// Deletion operator.
Buffer::~Buffer() {
	// Delete internal buffer, if applicable.
	if (deleteInternal) {
		pax_buf_destroy(internal);
		delete internal;
	}
	
	// Set to safe default.
	internal       = NULL;
	deleteInternal = false;
}



// Get the width, in pixels, of the buffer.
int Buffer::width() { return internal ? internal->width : -1; }
// Get the height, in pixels, of the buffer.
int Buffer::height() { return internal ? internal->height : -1; }
// Get the type of the buffer.
pax_buf_type_t Buffer::type() { return internal ? internal->type : (pax_buf_type_t) -1; }



#define COMMA ,

#define GENERIC_VALIDITY_CHECK(retval) if (!internal) return retval;

static inline pax_shader_t *GENERIC_UNWRAP_SHADER(Shader *wrapped) {
	return wrapped ? wrapped->getInternal() : NULL;
}

#define GENERIC_WRAPPER_IMPL(localName, remoteName, uvType, extraArgs, unpackedArgs) \
void Buffer::draw##localName(extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_draw_##remoteName(internal, fillColor, unpackedArgs);\
}\
void Buffer::draw##localName(Color color, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_draw_##remoteName(internal, color, unpackedArgs);\
}\
void Buffer::draw##localName(Shader *shader, uvType *uvs, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_shade_##remoteName(internal, fillColor, GENERIC_UNWRAP_SHADER(shader), uvs, unpackedArgs);\
}\
void Buffer::draw##localName(Color color, Shader *shader, uvType *uvs, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_shade_##remoteName(internal, color, GENERIC_UNWRAP_SHADER(shader), uvs, unpackedArgs);\
}\
void Buffer::outline##localName(extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_outline_##remoteName(internal, lineColor, unpackedArgs);\
}\
void Buffer::outline##localName(Color color, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_outline_##remoteName(internal, color, unpackedArgs);\
}


// Fills the entire buffer with the given color.
void Buffer::background(Color color) {
	GENERIC_VALIDITY_CHECK()
	pax_background(internal, color);
}

GENERIC_WRAPPER_IMPL(Rect,   rect,   Quadf, float x COMMA float y COMMA float width COMMA float height, x COMMA y COMMA width COMMA height)
GENERIC_WRAPPER_IMPL(Tri,    tri,    Trif,  float x0 COMMA float y0 COMMA float x1 COMMA float y1 COMMA float x2 COMMA float y2, x0 COMMA y0 COMMA x1 COMMA y1 COMMA x2 COMMA y2)
GENERIC_WRAPPER_IMPL(Circle, circle, Quadf, float x COMMA float y COMMA float radius, x COMMA y COMMA radius)
GENERIC_WRAPPER_IMPL(Arc,    arc,    Quadf, float x COMMA float y COMMA float radius COMMA float startAngle COMMA float endAngle, x COMMA y COMMA radius COMMA startAngle COMMA endAngle)

// Draws a line with the default outline color.
void Buffer::drawLine(float x0, float y0, float x1, float y1) {
	GENERIC_VALIDITY_CHECK()
	pax_draw_line(internal, lineColor, x0, y0, x1, y1);
}
// Draws a line with a custom outline color.
void Buffer::drawLine(Color color, float x0, float y0, float x1, float y1) {
	GENERIC_VALIDITY_CHECK()
	pax_draw_line(internal, color, x0, y0, x1, y1);
}

// Outlines an arbitrary shape.
void Buffer::outline(float x, float y, Shape &shape) { outline(lineColor, NULL, x, y, shape); }
// Outlines an arbitrary shape.
void Buffer::outline(Color color, float x, float y, Shape &shape) { outline(color, NULL, x, y, shape); }
// Outlines an arbitrary shape.
void Buffer::outline(Color color, Shader *shader, float x, float y, Shape &shape) {
	pax_push_2d(internal);
	pax_apply_2d(internal, matrix_2d_translate(x, y));
	shape._int_draw(internal, color, shader ? shader->getInternal() : NULL, true);
	pax_pop_2d(internal);
}

// Draws an arbitrary shape.
void Buffer::draw(float x, float y, Shape &shape) { draw(fillColor, NULL, x, y, shape); }
// Draws an arbitrary shape.
void Buffer::draw(Color color, float x, float y, Shape &shape) { draw(color, NULL, x, y, shape); }
// Draws an arbitrary shape.
void Buffer::draw(Color color, Shader *shader, float x, float y, Shape &shape) {
	pax_push_2d(internal);
	pax_apply_2d(internal, matrix_2d_translate(x, y));
	shape._int_draw(internal, color, shader ? shader->getInternal() : NULL, false);
	pax_pop_2d(internal);
}

// Draws an image stored in another buffer.
void Buffer::drawImage(pax_buf_t *image, float x, float y) {
	GENERIC_VALIDITY_CHECK();
	pax_draw_image(internal, image, x, y);
}
// Draws an image stored in another buffer.
void Buffer::drawImage(pax_buf_t *image, float x, float y, float width, float height) {
	GENERIC_VALIDITY_CHECK();
	pax_draw_image_sized(internal, image, x, y, width, height);
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
Vec2f Buffer::stringSize(const pax_font_t *font, float font_size, std::string text) {
	return pax_text_size(font, font_size, text.c_str());
}
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
Vec2f Buffer::drawString(const pax_font_t *font, float font_size, float x, float y, std::string text) {
	GENERIC_VALIDITY_CHECK(Vec2f())
	return pax_draw_text(internal, fillColor, font, font_size, x, y, text.c_str());
}
// Draw a string with the given font and return it's size.
// Size is before matrix transformation.
Vec2f Buffer::drawString(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text) {
	GENERIC_VALIDITY_CHECK(Vec2f())
	return pax_draw_text(internal, color, font, font_size, x, y, text.c_str());
}
// Draw a string with the given font and return it's size, center-aligning every line individually.
// Size is before matrix transformation.
Vec2f Buffer::drawStringCentered(const pax_font_t *font, float font_size, float x, float y, std::string text) {
	GENERIC_VALIDITY_CHECK(Vec2f())
	return pax_center_text(internal, fillColor, font, font_size, x, y, text.c_str());
}
// Draw a string with the given font and return it's size, center-aligning every line individually.
// Size is before matrix transformation.
Vec2f Buffer::drawStringCentered(Color color, const pax_font_t *font, float font_size, float x, float y, std::string text) {
	GENERIC_VALIDITY_CHECK(Vec2f())
	return pax_center_text(internal, color, font, font_size, x, y, text.c_str());
}

// Push the matrix stack.
void Buffer::pushMatrix() {
	GENERIC_VALIDITY_CHECK()
	pax_push_2d(internal);
}
// Pop the matrix stack.
void Buffer::popMatrix() {
	GENERIC_VALIDITY_CHECK()
	pax_pop_2d(internal);
}
// Clear the matrix stack (no mode popMatix calls left) and reset the current matrix to identity (no transformation).
void Buffer::clearMatrix() {
	GENERIC_VALIDITY_CHECK()
	pax_reset_2d(internal, true);
}
// If full: clears the entire matrix stack,
// Otherwise clears just the current matrix.
void Buffer::clearMatrix(bool full) {
	GENERIC_VALIDITY_CHECK()
	pax_reset_2d(internal, full);
}

// Applies a given 2D matrix to the current by matrix multiplication.
void Buffer::applyMatrix(matrix_2d_t matrix) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix);
}
// Scales the current view.
void Buffer::scale(float x, float y) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_scale(x, y));
}
// Scales the current view.
void Buffer::scale(float factor) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_scale(factor, factor));
}
// Moves around the current view.
void Buffer::translate(float x, float y) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_translate(x, y));
}
// Shears the current view.
// Positive X causes the points above the origin to move to the right.
// Positive Y causes the points to the right of the origin to move down.
void Buffer::shear(float x, float y) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_shear(x, y));
}
// Rotates the current view around the origin, angles in radians.
void Buffer::rotate(float angle) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_rotate(angle));
}
// Rotates the current view around a given point.
void Buffer::rotateAround(float x, float y, float angle) {
	GENERIC_VALIDITY_CHECK()
	pax_apply_2d(internal, matrix_2d_translate(-x, -y));
	pax_apply_2d(internal, matrix_2d_rotate(angle));
	pax_apply_2d(internal, matrix_2d_translate(x, y));
}

// Gets color at the given point.
Color Buffer::getPixel(int x, int y) {
	GENERIC_VALIDITY_CHECK(0)
	return pax_get_pixel(internal, x, y);
}
// Sets color at the given point.
void Buffer::setPixel(Color color, int x, int y) {
	GENERIC_VALIDITY_CHECK()
	pax_set_pixel(internal, color, x, y);
}
// Overlays the color at the given point (for transparent drawing).
void Buffer::mergePixel(Color color, int x, int y) {
	GENERIC_VALIDITY_CHECK()
	pax_merge_pixel(internal, color, x, y);
}

// Whether or not there has been drawing since last markClean call.
bool Buffer::isDirty() {
	GENERIC_VALIDITY_CHECK(false)
	return pax_is_dirty(internal);
}
// Gets the rectangle in which it is dirty.
Recti Buffer::getDirtyRect() {
	GENERIC_VALIDITY_CHECK(Recti())
	return (Recti) {
		internal->dirty_x0,
		internal->dirty_y0,
		internal->dirty_x1 - internal->dirty_x0 + 1,
		internal->dirty_y1 - internal->dirty_y0 + 1,
	};
}
// Mark the buffer as clean.
void Buffer::markClean() {
	GENERIC_VALIDITY_CHECK()
	pax_mark_clean(internal);
}
// Mark the entire buffer as dirty.
void Buffer::markDirty() {
	GENERIC_VALIDITY_CHECK()
	pax_mark_dirty0(internal);
}
// Mark a single pixel as dirty.
void Buffer::markDirty(int x, int y) {
	GENERIC_VALIDITY_CHECK()
	pax_mark_dirty1(internal, x, y);
}
// Mark a rectangular region as dirty.
void Buffer::markDirty(int x, int y, int width, int height) {
	GENERIC_VALIDITY_CHECK()
	pax_mark_dirty2(internal, x, y, width, height);
}

// Apply a clip rectangle to the buffer.
// Anothing outside of the clip will not be drawn.
// This is an operation that ignores matrix transforms (translate, rotate, etc.).
void Buffer::clip(int x, int y, int width, int height) {
	GENERIC_VALIDITY_CHECK()
	pax_clip(internal, x, y, width, height);
}

// Disable clipping.
// Any effects of previous clip calls are nullified.
void Buffer::noClip() {
	GENERIC_VALIDITY_CHECK()
	pax_noclip(internal);
}

// Obtain a copy of the current clip rect.
Recti Buffer::getClip() {
	GENERIC_VALIDITY_CHECK(Recti())
	return internal->clip;
}


// A linear interpolation based only on ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
	// This funny line converts part from 0-255 to 0-256.
	// Then, it applies an integer multiplication and the result is shifted right by 8.
	return from + (( (to - from) * (part + (part >> 7)) ) >> 8);
}

// Internal method for AHSV to ARGB.
// Ranges are 0xff, 0x5ff, 0xff, 0xff.
extern "C" pax_col_t PRIVATE_pax_col_hsv(uint8_t a, uint16_t h, uint8_t s, uint8_t v);

// Internal method for RGB to HSV.
// Ranges are 0x5ff, 0xff, 0xff.
extern "C" void PRIVATE_pax_undo_col_hsv(pax_col_t in, uint16_t *h, uint8_t *s, uint8_t *v);

// Converts HSV to ARGB, ranges are 0-255.
Color hsv(uint8_t h, uint8_t s, uint8_t v) {
	return PRIVATE_pax_col_hsv(255, h*6, s, v);
}

// Converts AHSV to ARGB, ranges are 0-255.
Color ahsv(uint8_t a, uint8_t h, uint8_t s, uint8_t v) {
	return PRIVATE_pax_col_hsv(a, h*6, s, v);
}

// Converts HSV to ARGB, ranges are 0-360, 0-100, 0-100.
Color hsv_alt(uint16_t h, uint8_t s, uint8_t v) {
	return PRIVATE_pax_col_hsv(255, h%360*6*255/359, s*255/100, v*255/100);
}

// Converts AHSV to ARGB, ranges are 0-255, 0-360, 0-100, 0-100.
Color ahsv_alt(uint8_t a, uint16_t h, uint8_t s, uint8_t v) {
	return PRIVATE_pax_col_hsv(a, h%360*6*255/359, s*255/100, v*255/100);
}


// Converts ARGB into AHSV, ranges are 0-255.
void undo_ahsv(Color in, uint8_t &a, uint8_t &h, uint8_t &s, uint8_t &v) {
	a = in >> 24;
	uint16_t l_h;
	PRIVATE_pax_undo_col_hsv(in, &l_h, &s, &v);
	h = (l_h + 3) / 6;
}

// Converts RGB into HSV, ranges are 0-255.
void undo_hsv(Color in, uint8_t &h, uint8_t &s, uint8_t &v) {
	uint16_t l_h;
	PRIVATE_pax_undo_col_hsv(in, &l_h, &s, &v);
	h = (l_h + 3) / 6;
}

// Converts ARGB into AHSV, ranges are 0-359, 0-99, 0-99.
void undo_ahsv_alt(Color in, uint8_t &a, uint8_t &h, uint8_t &s, uint8_t &v) {
	a = in >> 24;
	uint16_t l_h;
	PRIVATE_pax_undo_col_hsv(in, &l_h, &s, &v);
	h = (l_h + 3) * 359 / 255 / 6;
	s = s * 100 / 255;
	v = v * 100 / 255;
}

// Converts RGB into HSV, ranges are 0-255, 0-359, 0-99, 0-99.
void undo_hsv_alt(Color in, uint8_t &h, uint8_t &s, uint8_t &v) {
	uint16_t l_h;
	PRIVATE_pax_undo_col_hsv(in, &l_h, &s, &v);
	h = (l_h + 3) * 359 / 255 / 6;
	s = s * 100 / 255;
	v = v * 100 / 255;
}


// Linearly interpolates between from and to, including alpha.
Color lerp(uint8_t part, Color from, Color to) {
	return (pax_lerp(part, from >> 24, to >> 24) << 24)
		 | (pax_lerp(part, from >> 16, to >> 16) << 16)
		 | (pax_lerp(part, from >>  8, to >>  8) <<  8)
		 |  pax_lerp(part, from,       to);
}

// Merges the two colors, based on alpha.
Color merge(Color base, Color top) {
	// If top is transparent, return base.
	if (!(top >> 24)) return base;
	// If top is opaque, return top.
	if ((top >> 24) == 255) return top;
	// Otherwise, do a full alpha blend.
	uint8_t part = top >> 24;
	return (pax_lerp(part, base >> 24, 255)       << 24)
		 | (pax_lerp(part, base >> 16, top >> 16) << 16)
		 | (pax_lerp(part, base >>  8, top >>  8) <<  8)
		 |  pax_lerp(part, base,       top);
}

// Tints the color, commonly used for textures.
Color tint(Color col, Color tint) {
	// If tint is 0, return 0.
	if (!tint) return 0;
	// If tint is opaque white, return input.
	if (tint == -1) return col;
	// Otherwise, do a full tint.
	return (pax_lerp(tint >> 24, 0, col >> 24) << 24)
		 | (pax_lerp(tint >> 16, 0, col >> 16) << 16)
		 | (pax_lerp(tint >>  8, 0, col >>  8) <<  8)
		 |  pax_lerp(tint,       0, col);
}



} // namespace pax
