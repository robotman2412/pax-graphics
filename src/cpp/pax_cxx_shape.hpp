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

#pragma once

#include <pax_types.h>

#ifdef __cplusplus

#include <vector>

namespace pax {
// A list of points meant to represent a polygon's ouline.
typedef std::vector<pax_vec1_t> Outline;
// An immutable representation of a shape.
class Shape;
}

#include "pax_cxx.hpp"

namespace pax {

// An immutable representation of a shape.
// Classes inheriting shape should aim to be immutable, but it is not required.
class Shape {
	protected:
		// The outline that represents this shape.
		Outline outline;
		// The bounding box of this shape.
		pax_rect_t bounds;
		// The optional triangulation that represents this shape.
		std::vector<size_t> triang;
		// Whether triangulation was successful.
		bool triangSuccess;
		// Whether triangulation was done.
		bool triangDone;
		
		// Recalculate the bounds of this shape.
		virtual void updateBounds();
		// Recalculate the triangulation of this shape.
		virtual void updateTriang();
		
	public:
		// The default shape is a rectangle.
		Shape();
		// Make a shape from an outline.
		Shape(Outline outline);
		// Default deconstructor.
		virtual ~Shape();
		
		// Get a bounding box for this shape.
		pax_rect_t getBounds();
		// Get a copy of the outline that represents this shape.
		Outline getOutline();
		
		// Internal method used for drawing.
		virtual void _int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline);
		
		// Equality operator.
		virtual bool operator==(Shape const &other);
};

// An immutable circle shape.
class Circle: public Shape {
	protected:
		// Radius. Currently.
		float currentRadius;
		
		// Make a circle with a resolution. And a radius.
		void init(float radius, size_t resolution);
		
	public:
		// The default is a unit circle.
		Circle();
		// Create a circle with a certain radius.
		Circle(float radius);
		// Create a circle with a certain radius.
		Circle(float radius, size_t resolution);
		
		// Get the radius of this circle.
		float radius();
		// Internal method used for drawing.
		virtual void _int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline);
};

// An immutable rectangle shape.
class Rectangle: public Shape {
	protected:
		// Position and dimensions.
		float x, y, width, height;
		// Internal rectangle generator.
		void init(float x, float y, float w, float h);
		
	public:
		// The default is a 2x2 rectangle centered around (0,0).
		Rectangle();
		// Create a rectangle with a position and size.
		Rectangle(float x, float y, float width, float height);
		// Create a rectangle centered around (0,0).
		Rectangle(float width, float height);
		
		// Internal method used for drawing.
		virtual void _int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline);
};

// An immutable representation of a shape being interpolated into another.
class LerpShape: public Shape {
	protected:
		// An analogy of the outline of the original shape.
		Outline originalOutline;
		// An analogy of the outline of the target shape.
		Outline targetOutline;
		
		// Current interpolation coefficient.
		float currentCoeff;
		
		// Inserts a number of interpolated points at a given index.
		void insertPoints(Outline &outline, size_t index, size_t count);
		// Distributes a number of subdivisions around the outline.
		void distributePoints(Outline &outline, size_t count);
		// Sets original and target in a way such they have an equal amount of vertices.
		void setShapes();
		// Sets the current outline to an interpolated form of the source outlines.
		void interpolate(float coeff);
		
	public:
		// The default shape is a rectangle.
		LerpShape();
		// Interpolate between two shapes.
		// It is more performant to call withCoefficient than it is to call this constructor again.
		LerpShape(Shape &original, Shape &target, float coeff);
		// Get a copy of the interpolated shape with a different coefficient.
		LerpShape(LerpShape &original, float coeff);
		// Get a copy of the interpolated shape with a different coefficient.
		LerpShape withCoeff(float coeff);
		
		// Get the original that helped create this shape.
		Shape original();
		// Get the target that helped create this shape.
		Shape target();
		// Get the interpolation coefficient that helped create this shape.
		float coeff();
};

}
#endif // __cplusplus
