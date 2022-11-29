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
#include <string.h>

namespace pax {



// Recalculate the bounds of this shape.
void Shape::updateBounds() {
	float x0 = infinityf(), y0 = infinityf(), x1 = -infinityf(), y1 = -infinityf();
	
	for (size_t i = 0; i < outline.size(); i++) {
		pax_vec1_t p = outline[i];
		if (p.x < x0) x0 = p.x;
		if (p.x > x1) x1 = p.x;
		if (p.y < y0) y0 = p.y;
		if (p.y > y1) y1 = p.y;
	}
	
	bounds = (pax_rect_t) {
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


// The default shape, a rectangle.
Shape::Shape() {
	outline.push_back((pax_vec1_t) {-1, -1});
	outline.push_back((pax_vec1_t) { 1, -1});
	outline.push_back((pax_vec1_t) { 1,  1});
	outline.push_back((pax_vec1_t) {-1,  1});
	bounds = (pax_rect_t) { -1, -1, 2, 2 };
	triang.push_back(0);
	triang.push_back(1);
	triang.push_back(2);
	triang.push_back(0);
	triang.push_back(2);
	triang.push_back(3);
	triangSuccess = true;
	triangDone    = true;
}

// Make a shape from an outline.
Shape::Shape(Outline outline) {
	this->outline = outline;
	updateBounds();
	triangDone = false;
}

// Default deconstructor.
Shape::~Shape() {}

// Get a bounding box for this shape.
pax_rect_t Shape::getBounds() {
	return (pax_rect_t) {0, 0, 0, 0};
}

// Get a copy of the outline that represents this shape.
Outline Shape::getOutline() {
	return outline;
}


// Internal method used for drawing.
void Shape::_int_draw(pax_buf_t *to, pax_col_t color, const pax_shader_t *shader, bool asOutline) {
	// TODO: Shader support.
	if (!asOutline && !triangDone) updateTriang();
	if (asOutline || (triangDone && !triangSuccess)) {
		pax_outline_shape_cl(to, color, outline.size(), outline.data(), true);
	} else {
		if (triangDone && triangSuccess) pax_draw_shape_triang(to, color, outline.size(), outline.data(), triang.size()/3, triang.data());
	}
}


// Equality operator.
bool Shape::operator==(Shape const &other) {
	if (other.outline.size() != outline.size()) return false;
	
	for (size_t i = 0; i < outline.size(); i++) {
		if (outline[i].x != other.outline[i].x || outline[i].y != outline[i].y) return false;
	}
	
	return true;
}



// Make a circle with a resolution. And a radius.
void Circle::init(float radius, size_t resolution) {
	if (resolution < 3) resolution = 3;
	
	// Use internal vectoriser.
	pax_vec1_t tmp[resolution+1];
	pax_vectorise_circle(tmp, resolution+1, 0, 0, radius);
	
	// Determine triangle count.
	size_t n_tri = resolution-2;
	triang.resize(n_tri*3);
	
	// Simple triangulation.
	for (size_t i = 0; i < n_tri; i++) {
		triang[i*3+0] = 0;
		triang[i*3+1] = i;
		triang[i*3+2] = i+1;
	}
	
	// Simple bounds computation.
	bounds = (pax_rect_t) {
		.x = -radius,
		.y = -radius,
		.w = 2 * radius,
		.h = 2 * radius,
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
	triangDone = false;
}

// Create a circle with a certain radius.
Circle::Circle(float radius) {
	init(radius, 32);
	triangDone = false;
}

// Create a circle with a certain radius.
Circle::Circle(float radius, size_t resolution) {
	init(radius, resolution);
	triangDone = false;
}

// Get the radius of this circle.
float Circle::radius() {
	return currentRadius;
}

// Internal method used for drawing.
void Circle::_int_draw(pax_buf_t *to, pax_col_t color, const pax_shader_t *shader, bool asOutline) {
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
	outline.push_back((pax_vec1_t) {x + w, y    });
	outline.push_back((pax_vec1_t) {x,     y    });
	outline.push_back((pax_vec1_t) {x,     y + h});
	outline.push_back((pax_vec1_t) {x + w, y + h});
	
	// Generate a triangulation.
	triang.clear();
	triang.push_back(0);
	triang.push_back(1);
	triang.push_back(2);
	triang.push_back(0);
	triang.push_back(2);
	triang.push_back(3);
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
void Rectangle::_int_draw(pax_buf_t *to, pax_col_t color, const pax_shader_t *shader, bool asOutline) {
	if (asOutline) {
		pax_outline_rect(to, color, x, y, width, height);
	} else {
		pax_draw_rect(to, color, x, y, width, height);
	}
}



// Inserts a number of interpolated points at a given index.
void LerpShape::insertPoints(Outline &outline, size_t index, size_t count) {
	pax_vec1_t last  = outline[index];
	pax_vec1_t first = outline[(index + 1) % outline.size()];
	
	for (size_t i = 0; i < count; i++) {
		float      coeff  = (i + 1) / (float) (count + 1);
		pax_vec1_t center = {
			last.x + (first.x - last.x) * coeff,
			last.y + (first.y - last.y) * coeff,
		};
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
	// insertPoints(shorter, shorter.size() - 1, interp_count);
	distributePoints(shorter, interp_count);
	// pax_vec1_t last  = shorter[shorter.size() - 1];
	// pax_vec1_t first = shorter[0];
	
	// // Expand the shorter by interpolating some points.
	// for (size_t i = 0; i < interp_count; i++) {
	// 	float      coeff  = (i + 1) / (float) (interp_count + 1);
	// 	pax_vec1_t center = {
	// 		last.x + (first.x - last.x) * coeff,
	// 		last.y + (first.y - last.y) * coeff,
	// 	};
	// 	shorter.push_back(center);
	// }
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
		pax_vec1_t original = originalOutline[i];
		pax_vec1_t target   = targetOutline[i];
		pax_vec1_t center   = {
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
