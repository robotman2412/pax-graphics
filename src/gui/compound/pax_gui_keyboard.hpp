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

#ifndef PAX_GUI_KEYBOARD_HPP
#define PAX_GUI_KEYBOARD_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>
#include <functional>

#include <pax_gui_base.hpp>
#include <pax_gui_container.hpp>

namespace pax::gui {

class Keyboard: protected Container, public virtual Element {
	public:
		// Board selection.
		enum class Type {
			// Lowercase letters, ',' and '.'.
			LOWERCASE,
			// Uppercase letters, '<' and '>.
			UPPERCASE,
			// Numbers and common symbols.
			NUMBERS,
			// Less common symbols.
			SYMBOLS,
			// Numbers only.
			NUMPAD,
		};
		// Action callback type.
		using Callback = std::function<void(Keyboard&)>;
		
	protected:
		// Current keyboard type.
		Type type;
		// Current position.
		int x, y;
		
	public:
		// Callback to run when an input is accepted.
		Callback onAccept;
		// Callback to run when an input is rejected.
		Callback onReject;
		
		// Whether to display non-ascii (unicode) characters.
		bool showUnicode;
		// Whether to restrict to number input.
		bool isNumeric;
		
		// Make a basic keyboard.
		Keyboard(Rectf bounds={0, 0, 200, 200});
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) override;
		// Button released event.
		virtual void buttonUp(InputButton which) override;
		
		// Callback to run every so often.
		// Returns true if the object has to be redrawn.
		virtual bool tick(uint64_t millis, uint64_t deltaMillis) override;
		// Draw this element to `buf`.
		virtual void draw(Buffer &buf) override;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_KEYBOARD_HPP
