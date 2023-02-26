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



// Get a bounding box for this shape.
pax_rect_t Shape::getBounds() {
	return (pax_rect_t) {0, 0, 0, 0};
}

// Get a copy of the outline that represents this shape.
Shape::Outline Shape::getOutline() {
	return Outline();
}


// Create a shape which represents a rounded version of this shape.
Shape *Shape::round(float radius) {
	return new Shape();
}

// Internal method used for drawing.
void Shape::_int_draw(pax_buf_t *to, pax_col_t *color, const pax_shader_t *shader, bool asOutline) {}



} // namespace pax
#endif // __cplusplus
