
#include "pax_gui_base.hpp"

namespace pax::gui {

// Theme storage object.
Theme theme = {
	// Background color of the canvas, if any.
	.backgroundColor = 0x00000000,
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
