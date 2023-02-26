
#include "pax_gui.hpp"

namespace pax::gui {

// Make a new label with some text on it.
Label::Label(Rectf _bounds, std::string _text, TextAlign _align):
	Element(_bounds), text(_text), align(_align) {}

// Draw this element to `buf`.
// When selected by user interaction, `selected` is true.
void Label::draw(Buffer &buf, bool selected) {
	const pax_font_t *font = pax_font_saira_regular;
	buf.drawStringCentered(0xff000000, font, font->default_size, bounds.x + bounds.w * 0.5, bounds.y, text);
}

}
