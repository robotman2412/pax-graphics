
#include "pax_gui.hpp"

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
		onPress(*this);
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
void Button::draw(Buffer &buf, bool selected) {
	const auto &theme = *getTheme();
	
	// Button background.
	buf.drawRect(
		pressed ? theme.pressedColor : theme.foregroundColor,
		bounds.x, bounds.y, bounds.w, bounds.h
	);
	// Button outline.
	buf.outlineRect(
		selected && !pressed ? theme.highlightColor : theme.backgroundColor,
		bounds.x, bounds.y, bounds.w, bounds.h
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
