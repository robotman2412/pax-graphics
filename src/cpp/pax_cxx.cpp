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

namespace pax {



// C wrapper function for using C++ shaders.
extern "C" pax_col_t wrapperCallback(pax_col_t tint, int x, int y, float u, float v, void *args) {
	ShaderContent *ctx = (ShaderContent *) args;
	return (*ctx->callback)(tint, x, y, u, v, ctx->args);
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
		.callback          = wrapperCallback,
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
		return internal.callback(tint, x, y, u, v, internal.callback_args);
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
}

// Make a wrapper of a new buffer.
Buffer::Buffer(int width, int height, pax_buf_type_t type) {
	// Allocate memory.
	internal       = new pax_buf_t;
	deleteInternal = true;
	// Create buffer.
	pax_buf_init(internal, NULL, width, height, type);
}

// Make a wrapper of a new buffer, with preallocated memory.
// This memory will not be free()d by default.
Buffer::Buffer(void *preallocated, int width, int height, pax_buf_type_t type) {
	// Allocate memory.
	internal       = new pax_buf_t;
	deleteInternal = true;
	// Create buffer.
	pax_buf_init(internal, preallocated, width, height, type);
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


#define COMMA ,

#define GENERIC_VALIDITY_CHECK() if (!internal) return;

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

GENERIC_WRAPPER_IMPL(Rect,   rect,   pax_quad_t, float x COMMA float y COMMA float width COMMA float height, x COMMA y COMMA width COMMA height)
GENERIC_WRAPPER_IMPL(Tri,    tri,    pax_tri_t,  float x0 COMMA float y0 COMMA float x1 COMMA float y1 COMMA float x2 COMMA float y2, x0 COMMA y0 COMMA x1 COMMA y1 COMMA x2 COMMA y2)
GENERIC_WRAPPER_IMPL(Circle, circle, pax_quad_t, float x COMMA float y COMMA float radius, x COMMA y COMMA radius)
GENERIC_WRAPPER_IMPL(Arc,    arc,    pax_quad_t, float x COMMA float y COMMA float radius COMMA float startAngle COMMA float endAngle, x COMMA y COMMA radius COMMA startAngle COMMA endAngle)

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

} // namespace pax
