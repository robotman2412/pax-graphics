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
class Shape;
}

#include "pax_cxx.hpp"
#include <vector>
#include <memory>

namespace pax {

// An immutable representation of a shape.
class Shape {
	public:
		// A list of points meant to represent a polygon's ouline.
		typedef std::vector<pax_vec1_t> Outline;
		
		// Get a bounding box for this shape.
		virtual pax_rect_t getBounds();
		// Get a copy of the outline that represents this shape.
		virtual Outline getOutline();
		// Create a shape which represents a rounded version of this shape.
		virtual Shape *round(float radius);
		
		// Internal method used for drawing.
		virtual void _int_draw(pax_buf_t *to, pax_col_t *color, const pax_shader_t *shader, bool asOutline);
};

}
#endif // __cplusplus
