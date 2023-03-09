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

#include "pax_gui_base.hpp"

namespace pax::gui {

// Theme storage object.
Theme theme = {
	// Background color of the canvas, if any.
	.backgroundColor = 0xffffffff,
	// Foreground color.
	.foregroundColor = 0xffffffff,
	// Foregrount color for pressed buttons.
	.pressedColor = 0xffa0a0a0,
	// Outline color, if any.
	.outlineColor = 0xff000000,
	// Outline color for highlights, if any.
	.highlightColor = 0xff007fff,
	// Text color.
	.textColor = 0xff000000,
	
	// Text font.
	.font = pax_font_sky,
	// Text size.
	.fontSize = (float) pax_font_sky->default_size,
};

// Global theme setting.
Theme *getTheme() {
	return &theme;
}

// Global theme setting.
void setTheme(Theme newTheme) {
	theme = newTheme;
}

} // namespace pax::gui
