
#include "pax_gui.hpp"

namespace pax::gui {

// Make a bland new button.
Button::Button() {}

// Make a new button with some text on it.
Button::Button(Rectf _bounds, std::string _text, Callback _onPress):
	Element(_bounds), text(_text), onPress(_onPress) {}


// Button pressed event.
void Button::buttonDown(InputButton which) {
	if (which == ACCEPT) onPress(*this);
}


// Draw this element to `buf`.
// When selected by user interaction, `selected` is true.
void Button::draw(Buffer &buf, bool selected) {\
	const pax_font_t *font = pax_font_saira_regular;
	buf.drawRect(0xffffffff, bounds.x, bounds.y, bounds.w, bounds.h);
	buf.outlineRect(0xff000000, bounds.x, bounds.y, bounds.w, bounds.h);
	buf.drawStringCentered(0xff000000, font, font->default_size, bounds.x + bounds.w * 0.5, bounds.y, text);
}

} // namespace pax::gui
