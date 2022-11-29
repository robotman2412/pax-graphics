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

#include "pax_cxx.hpp"
#include "pax_gfx.h"

#ifdef __cplusplus
namespace pax {



// C wrapper function for using C++ shaders.
extern "C" pax_col_t wrapperCallback(pax_col_t existing, pax_col_t tint, int x, int y, float u, float v, void *args) {
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
	cxxShaderCtx.callback = new ShaderFunc(callback);
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


// Deletion operator.
Shader::~Shader() {
	if (isCxx && cxxShaderCtx.callback) delete cxxShaderCtx.callback;
	isCxx  = false;
	active = false;
}


// Apply this shader to a pixel.
pax_col_t Shader::apply(pax_col_t tint, int x, int y, float u, float v) {
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
void Buffer::draw##localName(pax_col_t color, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_draw_##remoteName(internal, color, unpackedArgs);\
}\
void Buffer::draw##localName(Shader *shader, uvType *uvs, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_shade_##remoteName(internal, fillColor, GENERIC_UNWRAP_SHADER(shader), uvs, unpackedArgs);\
}\
void Buffer::draw##localName(pax_col_t color, Shader *shader, uvType *uvs, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_shade_##remoteName(internal, color, GENERIC_UNWRAP_SHADER(shader), uvs, unpackedArgs);\
}\
void Buffer::outline##localName(extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_outline_##remoteName(internal, lineColor, unpackedArgs);\
}\
void Buffer::outline##localName(pax_col_t color, extraArgs) {\
	GENERIC_VALIDITY_CHECK()\
	pax_outline_##remoteName(internal, color, unpackedArgs);\
}


// Fills the entire buffer with the given color.
void Buffer::background(pax_col_t color) {
	GENERIC_VALIDITY_CHECK()
	pax_background(internal, color);
}

GENERIC_WRAPPER_IMPL(Rect,   rect,   pax_quad_t, float x COMMA float y COMMA float width COMMA float height, x COMMA y COMMA width COMMA height)
GENERIC_WRAPPER_IMPL(Tri,    tri,    pax_tri_t,  float x0 COMMA float y0 COMMA float x1 COMMA float y1 COMMA float x2 COMMA float y2, x0 COMMA y0 COMMA x1 COMMA y1 COMMA x2 COMMA y2)
GENERIC_WRAPPER_IMPL(Circle, circle, pax_quad_t, float x COMMA float y COMMA float radius, x COMMA y COMMA radius)
GENERIC_WRAPPER_IMPL(Arc,    arc,    pax_quad_t, float x COMMA float y COMMA float radius COMMA float startAngle COMMA float endAngle, x COMMA y COMMA radius COMMA startAngle COMMA endAngle)

// Outlines an arbitrary shape.
void Buffer::outline(float x, float y, Shape &shape) { outline(lineColor, NULL, x, y, &shape); }
// Outlines an arbitrary shape.
void Buffer::outline(float x, float y, Shape *shape) { outline(lineColor, NULL, x, y, shape); }
// Outlines an arbitrary shape.
void Buffer::outline(pax_col_t color, float x, float y, Shape &shape) { outline(color, NULL, x, y, &shape); }
// Outlines an arbitrary shape.
void Buffer::outline(pax_col_t color, float x, float y, Shape *shape) { outline(color, NULL, x, y, shape); }
// Outlines an arbitrary shape.
void Buffer::outline(pax_col_t color, Shader *shader, float x, float y, Shape &shape) { outline(color, shader, x, y, &shape); }
// Outlines an arbitrary shape.
void Buffer::outline(pax_col_t color, Shader *shader, float x, float y, Shape *shape) {
	if (shape) {
		pax_push_2d(internal);
		pax_apply_2d(internal, matrix_2d_translate(x, y));
		shape->_int_draw(internal, color, shader ? shader->getInternal() : NULL, true);
		pax_pop_2d(internal);
	}
}

// Draws an arbitrary shape.
void Buffer::draw(float x, float y, Shape &shape) { draw(fillColor, NULL, x, y, &shape); }
// Draws an arbitrary shape.
void Buffer::draw(float x, float y, Shape *shape) { draw(fillColor, NULL, x, y, shape); }
// Draws an arbitrary shape.
void Buffer::draw(pax_col_t color, float x, float y, Shape &shape) { draw(color, NULL, x, y, &shape); }
// Draws an arbitrary shape.
void Buffer::draw(pax_col_t color, float x, float y, Shape *shape) { draw(color, NULL, x, y, shape); }
// Draws an arbitrary shape.
void Buffer::draw(pax_col_t color, Shader *shader, float x, float y, Shape &shape) { draw(color, shader, x, y, &shape); }
// Draws an arbitrary shape.
void Buffer::draw(pax_col_t color, Shader *shader, float x, float y, Shape *shape) {
	if (shape) {
		pax_push_2d(internal);
		pax_apply_2d(internal, matrix_2d_translate(x, y));
		shape->_int_draw(internal, color, shader ? shader->getInternal() : NULL, false);
		pax_pop_2d(internal);
	}
}

// Draws a line with the default outline color.
void Buffer::drawLine(float x0, float y0, float x1, float y1) {
	GENERIC_VALIDITY_CHECK()
	pax_draw_line(internal, lineColor, x0, y0, x1, y1);
}

// Draws a line with a custom outline color.
void Buffer::drawLine(pax_col_t color, float x0, float y0, float x1, float y1) {
	GENERIC_VALIDITY_CHECK()
	pax_draw_line(internal, color, x0, y0, x1, y1);
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
pax_col_t Buffer::getPixel(int x, int y) {
	GENERIC_VALIDITY_CHECK(0)
	return pax_get_pixel(internal, x, y);
}
// Sets color at the given point.
void Buffer::setPixel(pax_col_t color, int x, int y) {
	GENERIC_VALIDITY_CHECK()
	pax_set_pixel(internal, color, x, y);
}
// Overlays the color at the given point (for transparent drawing).
void Buffer::mergePixel(pax_col_t color, int x, int y) {
	GENERIC_VALIDITY_CHECK()
	pax_merge_pixel(internal, color, x, y);
}

// Whether or not there has been drawing since last markClean call.
bool Buffer::isDirty() {
	GENERIC_VALIDITY_CHECK(false)
	return pax_is_dirty(internal);
}

// Gets the rectangle in which it is dirty.
pax_rect_t Buffer::getDirtyRect() {
	GENERIC_VALIDITY_CHECK((pax_rect_t) {0 COMMA 0 COMMA 0 COMMA 0})
	return (pax_rect_t) {
		.x = (float) internal->dirty_x0,
		.y = (float) internal->dirty_y0,
		.w = (float) internal->dirty_x1 - internal->dirty_x0,
		.h = (float) internal->dirty_y1 - internal->dirty_y0,
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



// A linear interpolation based only on ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
	// This funny line converts part from 0-255 to 0-256.
	// Then, it applies an integer multiplication and the result is shifted right by 8.
	return from + (( (to - from) * (part + (part >> 7)) ) >> 8);
}

// Converts HSV to ARGB.
pax_col_t hsv(uint8_t h, uint8_t s, uint8_t v) {
	return ahsv(255, h, s, v);
}

// Converts AHSV to ARGB.
pax_col_t ahsv(uint8_t a, uint8_t c_h, uint8_t s, uint8_t v) {
	uint16_t h     = c_h * 6;
	uint16_t phase = h >> 8;
	// Parts of HSV.
	uint8_t up, down, other;
	other  = ~s;
	if (h & 0x100) {
		// Down goes away.
		up     = 0xff;
		down   = pax_lerp(s, 0xff, ~h & 0xff);
	} else {
		// Up comes in.
		up     = pax_lerp(s, 0xff,  h & 0xff);
		down   = 0xff;
	}
	// Apply brightness.
	up    = pax_lerp(v, 0, up);
	down  = pax_lerp(v, 0, down);
	other = pax_lerp(v, 0, other);
	// Apply to RGB.
	uint8_t r, g, b;
	switch (phase >> 1) {
		case 0:
			// From R to G.
			r = down; g = up; b = other;
			break;
		case 1:
			// From G to B.
			r = other; g = down; b = up;
			break;
		case 2:
			// From B to R.
			r = up; g = other; b = down;
			break;
		default:
			// The compiler isn't aware that this case is never reached.
			return 0;
	}
	// Merge.
	return (a << 24) | (r << 16) | (g << 8) | b;
}

// Linearly interpolates between from and to, including alpha.
pax_col_t lerp(uint8_t part, pax_col_t from, pax_col_t to) {
	return (pax_lerp(part, from >> 24, to >> 24) << 24)
		 | (pax_lerp(part, from >> 16, to >> 16) << 16)
		 | (pax_lerp(part, from >>  8, to >>  8) <<  8)
		 |  pax_lerp(part, from,       to);
}

// Merges the two colors, based on alpha.
pax_col_t merge(pax_col_t base, pax_col_t top) {
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
pax_col_t tint(pax_col_t col, pax_col_t tint) {
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
#endif // __cplusplus
