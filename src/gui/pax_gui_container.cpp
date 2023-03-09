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

#include "pax_gui_container.hpp"

#include <algorithm>

namespace pax::gui {

// Unselect children, if any.
void Container::unselect() {
	if (selectedChild >= 0) {
		children[selectedChild]->focus = FocusState::NONE;
	}
	selectedChild = -1;
}

// Select a specific child by index.
// Returns false if the index is invalid.
bool Container::selectChild(int index) {
	if (index < 0 || index >= children.size()) return false;
	unselect();
	selectedChild = index;
	children[selectedChild]->focus = FocusState::HIGHLIGHTED;
	return true;
}

// Select a specific child by value.
// Returns false if the element is not a child.
bool Container::selectChild(std::shared_ptr<Element> child) {
	for (std::size_t i = 0; i < children.size(); i++) {
		if (children[i] == child) {
			unselect();
			selectedChild = i;
			children[selectedChild]->focus = FocusState::HIGHLIGHTED;
			return true;
		}
	}
	return false;
}

// Obtain the index of a selected child, if any.
// Returns -1 when not selected.
int Container::selectedIndex() {
	return selectedChild;
}

// Obtain the pointer of a selected child, if any.
std::shared_ptr<Element> Container::selectedElement() {
	if (selectedChild >= 0) return children[selectedChild];
	else return nullptr;
}


// Add a child element (move edition).
std::shared_ptr<Element> Container::appendChild(Element &&child) {
	auto ptr = std::make_shared<Element>(std::move(child));
	children.push_back(ptr);
	return ptr;
}

// Add a child element (from exiting shared_ptr edition).
void Container::appendChild(std::shared_ptr<Element> child) {
	children.push_back(child);
}

// Remove a child element.
bool Container::removeChild(std::shared_ptr<Element> child) {
	// Look for child.
	auto iter = std::find(children.begin(), children.end(), child);
	// If no match, return false.
	if (iter == children.end()) return false;
	
	// If match, remove from list.
	children.erase(iter);
	return true;
}


// Button pressed event.
void Container::buttonDown(InputButton which) {
	if (selectedChild >= 0 && selectedChild < children.size()) {\
		children[selectedChild]->buttonDown(which);
	}
}

// Button released event.
void Container::buttonUp(InputButton which) {
	if (selectedChild >= 0 && selectedChild < children.size()) {
		children[selectedChild]->buttonUp(which);
	}
}


// Draw this element to `buf`.
void Container::draw(Buffer &buf) {
	// Draw background, if any.
	if (background & 0xff000000) {
		buf.drawRect(background, bounds.x, bounds.y, bounds.w, bounds.h);
	}
	
	// Apply translation.
	buf.pushMatrix();
	buf.translate(bounds.x, bounds.y);
	
	// Draw all children.
	for (int i = 0; i < children.size(); i++) {
		auto &iter = children[i];
		if (iter->visible) {
			iter->draw(buf);
		}
	}
	
	// Restore transformation.
	buf.popMatrix();
}

} // namespace pax::gui
