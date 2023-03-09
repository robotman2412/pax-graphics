
#include "pax_gui_base.hpp"

namespace pax::gui {

// Theme storage object.
Theme theme;

// Global theme setting.
Theme *getTheme() {
	return &theme;
}

// Global theme setting.
void setTheme(Theme newTheme) {
	theme = newTheme;
}

} // namespace pax::gui
