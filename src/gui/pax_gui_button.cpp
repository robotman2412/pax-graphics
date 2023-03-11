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

#include "pax_gui_button.hpp"

namespace pax::gui {

// Make a bland new button.
Button::Button(): Element({0, 0, 100, 20}), text("Button"), pressed(false) {}

// Make a new button with some text on it.
Button::Button(Rectf _bounds, std::string _text, Callback _onPress):
	Element(_bounds), text(_text), onPress(_onPress), pressed(false) {}


// Button pressed event.
void Button::buttonDown(InputButton which) {
	if (which == ACCEPT) {
		pressed = true;
		if (onPress) onPress(*this);
	}
}

// Button released event.
void Button::buttonUp(InputButton which) {
	if (which == ACCEPT) {
		pressed = false;
	}
}


// Draw this element to `buf`.
// When selected by user interaction, `selected` is true.
void Button::draw(Buffer &buf) {
	const auto &theme = *getTheme();
	
	// Button background.
	buf.drawRect(
		pressed ? theme.pressedColor : theme.foregroundColor,
		bounds.x, bounds.y, bounds.w, bounds.h
	);
	// Button outline.
	buf.outlineRect(
		focus == FocusState::HIGHLIGHTED && !pressed
			? theme.highlightColor : theme.outlineColor,
		bounds.x, bounds.y, bounds.w-1, bounds.h-1
	);
	
	// Compute centered text position.
	Vec2f textSz = Buffer::stringSize(theme.font, theme.fontSize, text);
	float textX  = bounds.x + (bounds.w - textSz.x) / 2.0f;
	float textY  = bounds.y + (bounds.h - textSz.y) / 2.0f;
	// Button text.
	buf.drawString(
		theme.textColor,
		theme.font, theme.fontSize,
		textX, textY,
		text
	);
}

} // namespace pax::gui
