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

#ifndef PAX_GUI_CONTAINER_HPP
#define PAX_GUI_CONTAINER_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>

#include <pax_gui_base.hpp>

namespace pax::gui {

// An element that contains one or more other elements.
class Container: public Element {
	public:
		// Child elements.
		// Position of children is relative to parent.
		std::vector<std::shared_ptr<Element>> children;
		// Element background color, if any.
		Color background;
		// Index of selected child, if any.
		int selectedChild;
		
		// This is required to allow subclasses with virtuals.
		virtual ~Container() = default;
		
		// Add a child element (move edition).
		// Returns a shared_ptr to the child.
		virtual std::shared_ptr<Element> appendChild(Element &&child);
		// Add a child element (from exiting shared_ptr edition).
		virtual void appendChild(std::shared_ptr<Element> child);
		// Remove a child element.
		// Returns whether a matching child was removed.
		virtual bool removeChild(std::shared_ptr<Element> child);
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) override;
		// Button released event.
		virtual void buttonUp(InputButton which) override;
		
		// Draw this element to `buf`.
		// When selected by user interaction, `selected` is true.
		virtual void draw(Buffer &buf, bool selected=false) override;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_CONTAINER_HPP
