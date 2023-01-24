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



// Recalculate the bounds of this shape.
void Shape::updateBounds() {
	float x0 = infinityf(), y0 = infinityf(), x1 = -infinityf(), y1 = -infinityf();
	
	// Iterate over outline points.
	for (size_t i = 0; i < outline.size(); i++) {
		Vec2f p = outline[i];
		// Find minimum and maximum X.
		if (p.x < x0) x0 = p.x;
		if (p.x > x1) x1 = p.x;
		// Find minimum and maximum Y.
		if (p.y < y0) y0 = p.y;
		if (p.y > y1) y1 = p.y;
	}
	
	// Calculate rectangle from extremes.
	bounds = (Rectf) {
		x0,
		y0,
		x1 - x0,
		y1 - y0,
	};
}

// Recalculate the triangulation of this shape.
void Shape::updateTriang() {
	// Call the basic triangulation.
	size_t *tris   = NULL;
	size_t  n_tris = pax_triang_concave(&tris, outline.size(), outline.data());
	
	triangDone    = true;
	triangSuccess = n_tris > 0;
	
	if (triangSuccess) {
		// Store the triangulated data.
		triang.assign(tris, tris + 3*n_tris);
		free(tris);
	}
}


// The default shape is a rectangle.
Shape::Shape() {
	// Construct a 2x2 rectangle outline centered around (0,0).
	outline.push_back((Vec2f) {-1, -1});
	outline.push_back((Vec2f) { 1, -1});
	outline.push_back((Vec2f) { 1,  1});
	outline.push_back((Vec2f) {-1,  1});
	// With associated bounds, which are constant.
	bounds = (Rectf) { -1, -1, 2, 2 };
	// First triangle.
	triang.push_back(0);
	triang.push_back(1);
	triang.push_back(2);
	// Second triangle.
	triang.push_back(0);
	triang.push_back(2);
	triang.push_back(3);
	// Mark as triangulated.
	triangSuccess = true;
	triangDone    = true;
}

// Make a shape from an outline.
Shape::Shape(Outline outline) {
	// Copy the outline.
	this->outline = outline;
	// Compute bounds.
	updateBounds();
	// Not triangulated by default.
	triangDone = false;
}

// Default deconstructor.
Shape::~Shape() {}

// Get a bounding box for this shape.
Rectf Shape::getBounds() {
	return bounds;
}

// Get a copy of the outline that represents this shape.
Outline Shape::getOutline() {
	return outline;
}


// Internal method used for drawing.
void Shape::_int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline) {
	// TODO: Shader support.
	
	// If drawing as filled, try to triangulate.
	if (!asOutline && !triangDone) updateTriang();
	
	if (asOutline || (triangDone && !triangSuccess)) {
		// When explicitly drawing as outline, or when triangulation fails.
		pax_outline_shape_cl(to, color, outline.size(), outline.data(), true);
	} else {
		// When drawing as filled and triangulation success.
		pax_draw_shape_triang(to, color, outline.size(), outline.data(), triang.size()/3, triang.data());
	}
}


// Equality operator.
bool Shape::operator==(Shape const &other) {
	// Yes yes very complicated I know.
	return other.outline == outline;
}



// Make a circle with a resolution. And a radius.
void Circle::init(float radius, size_t resolution) {
	if (resolution < 3) resolution = 3;
	
	// Use internal vectoriser.
	Vec2f tmp[resolution+1];
	pax_vectorise_circle(tmp, resolution+1, 0, 0, radius);
	
	// Determine triangle count.
	size_t n_tri = resolution-2;
	triang.resize(n_tri*3);
	
	// Simple triangulation because a circle is convex.
	for (size_t i = 0; i < n_tri; i++) {
		triang[i*3+0] = 0;
		triang[i*3+1] = i;
		triang[i*3+2] = i+1;
	}
	
	// Simple bounds computation.
	bounds = (Rectf) {
		-radius,
		-radius,
		2 * radius,
		2 * radius,
	};
	
	// Update internal values.
	triangDone    = true;
	triangSuccess = true;
	currentRadius = radius;
	outline.assign(tmp, tmp + resolution);
}

// The default is a unit circle.
Circle::Circle() {
	init(1, 32);
}

// Create a circle with a certain radius.
Circle::Circle(float radius) {
	init(radius, 32);
}

// Create a circle with a certain radius.
Circle::Circle(float radius, size_t resolution) {
	init(radius, resolution);
}

// Get the radius of this circle.
float Circle::radius() {
	return currentRadius;
}

// Internal method used for drawing.
void Circle::_int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline) {
	if (asOutline) {
		pax_outline_circle(to, color, 0, 0, currentRadius);
	} else {
		pax_draw_circle(to, color, 0, 0, currentRadius);
	}
}



// Internal rectangle generator.
void Rectangle::init(float x, float y, float w, float h) {
	// Set parameters.
	this->x      = x;
	this->y      = y;
	this->width  = w;
	this->height = h;
	
	// Generate an outline.
	outline.clear();
	outline.push_back((Vec2f) {x + w, y    });
	outline.push_back((Vec2f) {x,     y    });
	outline.push_back((Vec2f) {x,     y + h});
	outline.push_back((Vec2f) {x + w, y + h});
	
	// Generate a triangulation.
	triang.clear();
	// First triangle.
	triang.push_back(0);
	triang.push_back(1);
	triang.push_back(2);
	// Second triangle.
	triang.push_back(0);
	triang.push_back(2);
	triang.push_back(3);
	// Mark as triangulated.
	triangDone    = true;
	triangSuccess = true;
}

// The default is a 2x2 rectangle centered around (0,0).
Rectangle::Rectangle() {
	init(-1, -1, 2, 2);
}

// Create a rectangle with a position and size.
Rectangle::Rectangle(float x, float y, float width, float height) {
	init(x, y, width, height);
}

// Create a rectangle centered around (0,0).
Rectangle::Rectangle(float width, float height) {
	init(-width/2, -height/2, width, height);
}

// Internal method used for drawing.
void Rectangle::_int_draw(pax_buf_t *to, Color color, const pax_shader_t *shader, bool asOutline) {
	if (asOutline) {
		pax_outline_rect(to, color, x, y, width, height);
	} else {
		pax_draw_rect(to, color, x, y, width, height);
	}
}



// Inserts a number of interpolated points at a given index.
void LerpShape::insertPoints(Outline &outline, size_t index, size_t count) {
	// Find the point after which to insert.
	Vec2f last  = outline[index];
	// And the point before which to insert.
	Vec2f first = outline[(index + 1) % outline.size()];
	
	for (size_t i = 0; i < count; i++) {
		// Calculate interpolation coefficient.
		float      coeff  = (i + 1) / (float) (count + 1);
		// Linearly interpolate between last and first.
		Vec2f center = {
			last.x + (first.x - last.x) * coeff,
			last.y + (first.y - last.y) * coeff,
		};
		// Insert calculated point into the outline.
		outline.insert(outline.begin() + index + i + 1, center);
	}
}

// Distributes a number of subdivisions around the outline.
void LerpShape::distributePoints(Outline &outline, size_t count) {
	size_t len    = outline.size();
	size_t offset = 0;
	
	// Tally up where to distribute points.
	size_t toDistribute = count;
	size_t distribution[len];
	memset(distribution, 0, sizeof(distribution));
	while (toDistribute) {
		for (size_t i = 0; i < len && toDistribute; i++, toDistribute--) {
			distribution[i] ++;
		}
	}
	
	// Fill in the points.
	for (size_t i = 0; i < len; i++) {
		insertPoints(outline, i + offset, distribution[i]);
		offset += distribution[i];
	}
}

// Sets original and target in a way such they have an equal amount of vertices.
void LerpShape::setShapes() {
	// Find which outline is shorter than the other.
	bool origShorter = originalOutline.size() < targetOutline.size();
	Outline &shorter = origShorter ? originalOutline : targetOutline;
	
	// Calculate points to interpolate.
	size_t interp_count = labs(originalOutline.size() - targetOutline.size());
	distributePoints(shorter, interp_count);
}

// Sets the current outline to an interpolated form of the source outlines.
void LerpShape::interpolate(float coeff) {
	currentCoeff = coeff;
	
	// Ensure the outlines have an equal length.
	if (originalOutline.size() != targetOutline.size()) {
		setShapes();
	}
	
	// Resize outline to correct size.
	outline.resize(originalOutline.size());
	
	// Linearly interpolate coordinates between original and target.
	for (size_t i = 0; i < originalOutline.size(); i++) {
		Vec2f original = originalOutline[i];
		Vec2f target   = targetOutline[i];
		Vec2f center   = {
			original.x + (target.x - original.x) * coeff,
			original.y + (target.y - original.y) * coeff,
		};
		outline[i] = center;
	}
	
	// Clear the triangulated flag.
	triangDone = false;
}


// The default shape is a rectangle.
LerpShape::LerpShape() {
	Shape default_lol;
	originalOutline = default_lol.getOutline();
	targetOutline   = default_lol.getOutline();
	outline         = default_lol.getOutline();
	bounds          = default_lol.getBounds();
}

// Interpolate between two shapes.
// It is more performant to call withCoefficient than it is to call this constructor again.
LerpShape::LerpShape(Shape &original, Shape &target, float coeff) {
	originalOutline = original.getOutline();
	targetOutline   = target.getOutline();
	interpolate(coeff);
	updateBounds();
}

// Get a copy of the other interpolated shape with a different coefficient.
LerpShape::LerpShape(LerpShape &original, float coeff) {
	originalOutline = original.originalOutline;
	targetOutline   = original.targetOutline;
	interpolate(coeff);
	updateBounds();
}

// Get a copy of the interpolated shape with a different coefficient.
LerpShape LerpShape::withCoeff(float coeff) {
	return LerpShape(*this, coeff);
}


// Get the original that helped create this shape.
Shape LerpShape::original() {
	return Shape(originalOutline);
}

// Get the target that helped create this shape.
Shape LerpShape::target() {
	return Shape(targetOutline);
}

// Get the interpolation coefficient that helped create this shape.
float LerpShape::coeff() {
	return currentCoeff;
}



} // namespace pax
