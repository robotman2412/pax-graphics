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

#ifndef PAX_GUI_BASE_HPP
#define PAX_GUI_BASE_HPP

#include <pax_gfx.h>
#include <typeinfo>

#ifdef __cplusplus

namespace pax::gui {

// A little box containing style options for the GUI.
struct Theme {
	// Background color of the canvas, if any.
	Color backgroundColor;
	// Foreground color.
	Color foregroundColor;
	// Foregrount color for pressed buttons.
	Color pressedColor;
	// Outline color, if any.
	Color outlineColor;
	// Outline color for highlights, if any.
	Color highlightColor;
	// Text color.
	Color textColor;
	
	// Text font.
	const pax_font_t *font;
	// Text size.
	float fontSize;
};

// Get the global theme setting.
Theme *getTheme();
// Update the global theme setting.
void setTheme(Theme newTheme);

// A set of focus states.
enum FocusState {
	// Not focussed on this element.
	NONE,
	// This element is highlighted.
	HIGHLIGHTED,
	// This element is focussed (previously received button press).
	FOCUSSED,
	// This element is focussed, capturing navigation inputs.
	CAPTURED,
	// This element is delegating navigation inputs to another.
	DELEGATED,
};

// A list of common inputs that GUI responds to.
enum InputButton {
	// Use when you don't know what button it is.
	UNKNOWN = 0,
	// Directional inputs.
	UP, DOWN, LEFT, RIGHT, CENTER,
	// Navigation inputs.
	ACCEPT, BACK, MENU, HOME, START, SELECT,
	// Typing inputs.
	TYPE, BACKSPACE, DELETE, SHIFT,
};

// A base class for things that respond to inputs.
class InputAcceptor {
	public:
		// This is required to allow subclasses with virtuals.
		virtual ~InputAcceptor() = default;
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) {}
		// Button released event.
		virtual void buttonUp(InputButton which) {}
		// Button changed event.
		void buttonChange(InputButton which, bool newState) {
			if (newState) buttonDown(which);
			else buttonUp(which);
		}
};

// A base class for things that update periodically.
class Active {
	public:
		// This is required to allow subclasses with virtuals.
		virtual ~Active() = default;
		
		// Callback to run every so often.
		// Returns true if the object has to be redrawn.
		virtual bool tick(uint64_t millis, uint64_t deltaMillis) { return false; }
};

// A base class for all GUI elements.
// This base class has stubs for all virtual methods.
class Element: public InputAcceptor, public Active {
	public:
		// Assigned position and size.
		Rectf bounds;
		// Whether this element is visible.
		// Helper variable for containers and related types.
		// Calling `draw` on an element with `visible=false` should always draw it, despite this variable.
		bool visible;
		// The type of focus this element has been given, if any.
		// The element may change the focus state to a higher one after button press.
		FocusState focus;
		
		// Element from a bounds.
		Element(Rectf _bounds = {0, 0, 0, 0}):
			bounds(_bounds), visible(true), focus(FocusState::NONE) {}
		
		// This is required to allow subclasses with virtuals.
		virtual ~Element() = default;
		
		// Draw this element to `buf`.
		virtual void draw(Buffer &buf) {}
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_BASE_HPP
