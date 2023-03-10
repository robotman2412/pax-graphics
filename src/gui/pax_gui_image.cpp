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

#include "pax_gui_image.hpp"

namespace pax::gui {

// Make an empty image.
Image::Image() {}

// Make an image from existing buffer data.
Image::Image(std::shared_ptr<Buffer> data):
	Element({0, 0, data->width(), data->height()}),
	data(std::move(_data)) {}

// Make an image from existing buffer data.
Image::Image(Vec2f pos, std::shared_ptr<Buffer> data):
	Element({pos.x, pos.y, data->width(), data->height()}),
	data(std::move(_data)) {}

// Make an image from existing buffer data.
Image::Image(Rectf bounds, std::shared_ptr<Buffer> data)
	Element(bounds),
	data(std::move(_data)) {}


// Draw this element to `buf`.
// When selected by user interaction, `selected` is true.
void Image::draw(Buffer &buf) {
	if (data) {
		
	}
}


} // namespace pax::gui
