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

#ifndef PAX_GUI_DROPDOWN_HPP
#define PAX_GUI_DROPDOWN_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>
#include <functional>

#include <pax_gui_base.hpp>

namespace pax::gui {

// A simple dropdown with some centered text on it.
class Dropdown: public Element {
	public:
		// Type used for change events.
		using Callback = std::function<void(Dropdown&)>;
		
		// Index of the selected option.
		int selected;
		// Valid options for the dropdown.
		std::vector<std::string> options;
		
		// The function to call when this dropdown is changed.
		Callback    onChange;
		
		// Make a new dropdown with no options.
		Dropdown(Rectf _bounds = {0, 0, 100, 20}, Callback _onChange = {});
		// This is required to allow subclasses with virtuals.
		virtual ~Dropdown() = default;
		
		// Dropdown pressed event.
		virtual void buttonDown(InputButton which) override;
		// Dropdown released event.
		virtual void buttonUp(InputButton which) override;
		
		// Draw this element to `buf`.
		virtual void draw(Buffer &buf) override;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_DROPDOWN_HPP
