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


// Called when previousBounds != bounds.
void Container::boundsChanged() {}


// Unselect children, if any.
void Container::unselect() {
	if (selected >= 0) {
		children[selected]->focus = FocusState::NONE;
	}
	selected = -1;
}

// Select a specific child by index.
// Returns false if the index is invalid.
bool Container::selectChild(int index) {
	if (index < 0 || index >= children.size()) return false;
	// Unselect any other children.
	if (selected != index) {
		unselect();
	}
	// Select new child.
	selected = index;
	if (children[selected]->focus < FocusState::HIGHLIGHTED) {
		children[selected]->focus = FocusState::HIGHLIGHTED;
	}
	return true;
}

// Select a specific child by value.
// Returns false if the element is not a child.
bool Container::selectChild(std::shared_ptr<Element> child) {
	for (std::size_t i = 0; i < children.size(); i++) {
		if (children[i] == child) {
			// Unselect any other children.
			if (selected != i) {
				unselect();
			}
			// Select new child.
			selected = i;
			if (children[selected]->focus < FocusState::HIGHLIGHTED) {
				children[selected]->focus = FocusState::HIGHLIGHTED;
			}
			return true;
		}
	}
	return false;
}

// Obtain the index of a selected child, if any.
// Returns -1 when not selected.
int Container::selectedIndex() {
	if (selected >= children.size()) {
		selected = -1;
	}
	return selected;
}

// Obtain the pointer of a selected child, if any.
std::shared_ptr<Element> Container::selectedElement() {
	if (selected >= children.size()) {
		selected = -1;
		return nullptr;
	}
	if (selected >= 0) return children[selected];
	else return nullptr;
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
	
	// Unselect the thing.
	if (iter - children.begin() == selected) {
		children[selected]->focus = FocusState::NONE;
		selected = -1;
	}
	// If match, remove from list.
	children.erase(iter);
	return false;
}


// Button pressed event.
void Container::buttonDown(InputButton which) {
	if (selected >= 0 && selected < children.size()) {
		if (children[selected]->focus < FocusState::FOCUSSED) {
			children[selected]->focus = FocusState::FOCUSSED;
		}
		children[selected]->buttonDown(which);
	}
}

// Button released event.
void Container::buttonUp(InputButton which) {
	if (selected >= 0 && selected < children.size()) {
		children[selected]->buttonUp(which);
	}
}


// Draw this element to `buf`.
void Container::draw(Buffer &buf) {
	if (bounds != previousBounds) {
		boundsChanged();
		previousBounds = bounds;
	}
	
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
		if (iter->visible && i != selected) {
			iter->draw(buf);
		}
	}
	// Draw selected child on top.
	if ((unsigned) selected < children.size() && children[selected]->visible) {
		children[selected]->draw(buf);
	}
	
	// Restore transformation.
	buf.popMatrix();
}

// Callback to run every so often.
// Returns true if the object has to be redrawn.
bool Container::tick(uint64_t millis, uint64_t deltaMillis) {
	bool dirty = false;
	if (bounds != previousBounds) {
		boundsChanged();
		previousBounds = bounds;
		dirty = true;
	}
	
	// Tick all children.
	for (auto &child: children) {
		dirty |= child->tick(millis, deltaMillis);
	}
	return dirty;
}



// Called when previousBounds != bounds.
void ListContainer::boundsChanged() {
	updateChildren();
}

// Update the positions of all children.
void ListContainer::updateChildren() {
	float height = 0;
	
	for (auto &child: children) {
		if (!child->visible) continue;
		child->bounds.x = 0;
		child->bounds.y = height;
		height += child->bounds.h;
	}
}


// Add a child element (from exiting shared_ptr edition).
// Overrides the position of `child` to fit in the list.
void ListContainer::appendChild(std::shared_ptr<Element> child) {
	children.push_back(child);
	updateChildren();
}


// Button pressed event.
void ListContainer::buttonDown(InputButton which) {
	if (selected == -1) {
		// Initial selection logic.
		switch (which) {
			case UP...CENTER:
			case ACCEPT:
				if (selectChild(0)) {
					focus = FocusState::CAPTURED;
				}
				break;
			default: break;
		}
		
	} else if (children[selected]->focus >= FocusState::CAPTURED) {
		// Forward the inputs.
		children[selected]->buttonDown(which);
		
	} else if (which == InputButton::ACCEPT) {
		// This will potentially cause child to capture inputs.
		if (children[selected]->focus < FocusState::FOCUSSED) {
			children[selected]->focus = FocusState::FOCUSSED;
		}
		children[selected]->buttonDown(which);
		
	} else if (which == InputButton::DOWN) {
		// Scroll down.
		if (doWrap) {
			selectChild((selected + 1) % children.size());
		} else if (selected < children.size() - 1) {
			selectChild(selected + 1);
		}
		
	} else if (which == InputButton::UP) {
		// Scroll up.
		if (doWrap) {
			selectChild((selected - 1) % children.size());
		} else if (selected > 0) {
			selectChild(selected - 1);
		}
	}
}

// Button released event.
void ListContainer::buttonUp(InputButton which) {
	if (selected == -1) return;
	
	if (children[selected]->focus >= FocusState::CAPTURED) {
		// Forward the inputs.
		children[selected]->buttonUp(which);
		
	} else if (which == InputButton::ACCEPT) {
		// This will potentially cause child to capture inputs.
		children[selected]->buttonUp(which);
	}
}



// Called when previousBounds != bounds.
void GridContainer::boundsChanged() {
	updateChildren();
}

// Find the first empty position.
auto GridContainer::findFirstEmpty() -> Pos {
	PosIter iter(0, 0, _width, _height);
	
	// Search positions list until there is no match.
	for (int i = 0; i < _width * _height; i++) {
		auto next = iter.next();
		if (std::find(childPos.begin(), childPos.end(), next) == childPos.end()) {
			// Free position found.
			return next;
		}
	}
	
	// Nothing found.
	return { -1, -1 };
}

// Find the next empty position.
auto GridContainer::findNextEmpty() -> Pos {
	PosIter iter(0, 0, _width, _height);
	iter.skip(children.size());
	
	// Search positions list until there is no match.
	for (int i = children.size(); i < _width * _height; i++) {
		auto next = iter.next();
		if (std::find(childPos.begin(), childPos.end(), next) == childPos.end()) {
			// Free position found.
			return next;
		}
	}
	
	// Nothing found.
	return { -1, -1 };
}

// Update the position of a specific child by index.
void GridContainer::updateChild(int index) {
	Pos      pos   = childPos[index];
	Element &child = *children[index];
	float cellW = bounds.w / _width;
	float cellH = bounds.h / _height;
	
	// Give the child a center-aligned position in it's cell.
	child.bounds.x = cellW * pos.x + (cellW - child.bounds.w) / 2;
	child.bounds.y = cellH * pos.y + (cellH - child.bounds.h) / 2;
}

// Update the positions of all children.
void GridContainer::updateChildren() {
	for (int i = 0; i < children.size(); i++) {
		updateChild(i);
	}
}


// Make an empty container.
GridContainer::GridContainer(Rectf _bounds, int width, int height):
	Container(_bounds), _width(width), _height(height) {}


// Get size of grid.
int GridContainer::width() const {
	return _width;
}

// Get size of grid.
int GridContainer::height() const {
	return _height;
}


// Select a specific child by grid position.
// Returns false if there is no child found.
bool GridContainer::selectChild(int x, int y) {
	// Look up position in pos list.
	for (int i = 0; i < childPos.size(); i++) {
		if (childPos[i].x == x && childPos[i].y == y) {
			return selectChild(i);
		}
	}
	return false;
}


// Add a child element (from exiting shared_ptr edition).
// Overrides the position of `child` to fit in the grid.
void GridContainer::appendChild(std::shared_ptr<Element> child) {
	Pos next = findNextEmpty();
	attach(child, next.x, next.y);
}


// Add a child element to a specific grid position.
// Overrides the position of `child` to fit in the grid.
void GridContainer::attach(std::shared_ptr<Element> child, int x, int y) {
	// Clamp position to grid size.
	if (x < 0) x = 0;
	if (x >= _width) x = _width - 1;
	if (y < 0) y = 0;
	if (y >= _height) y = _height - 1;
	
	// Remove children with the same position.
	removeChild(x, y);
	
	// Add it to the lists.
	children.push_back(child);
	childPos.push_back({x, y});
	updateChild(children.size()-1);
}


// Remove a child element by grid position.
// Returns whether a matching child was removed.
bool GridContainer::removeChild(int x, int y) {
	// Look up position in pos list.
	int i;
	for (i = 0; i < childPos.size(); i++) {
		if (childPos[i].x == x && childPos[i].y == y) break;
	}
	if (i >= childPos.size()) return false;
	
	// Remove child by index.
	if (selected == i) {
		children[i]->focus = FocusState::NONE;
		selected = -1;
	}
	children.erase(children.begin() + i);
	childPos.erase(childPos.begin() + i);
	
	return true;
}


// Button pressed event.
void GridContainer::buttonDown(InputButton which) {
	// Current position.
	int x = 0, y = 0;
	if (selected > -1) {
		x = childPos[selected].x;
		y = childPos[selected].y;
	}
	// Original position.
	int ox = x, oy = y;
	
	if (selected == -1) {
		// Initial selection logic.
		switch (which) {
			case UP...CENTER:
			case ACCEPT:
				if (selectChild(0)) {
					focus = FocusState::CAPTURED;
				}
				break;
			default: break;
		}
		
	} else if (children[selected]->focus >= FocusState::CAPTURED) {
		// Forward the inputs.
		children[selected]->buttonDown(which);
		
	} else if (which == InputButton::ACCEPT) {
		if (children[selected]->focus < FocusState::FOCUSSED) {
			children[selected]->focus = FocusState::FOCUSSED;
		}
		// This will potentially cause child to capture inputs.
		children[selected]->buttonDown(which);
		
	} else if (which == InputButton::UP) {
		// Scroll up.
		if (doWrap) {
			// Try to select some child on this column.
			do {
				y += _height - 1, y %= _height;
				if (selectChild(x, y)) {
					return;
				}
			} while (y != oy);
		} else {
			// Try to select some child on this column.
			for (y--; y >= 0; y--) {
				if (selectChild(x, y)) {
					return;
				}
			}
		}
		
	} else if (which == InputButton::DOWN) {
		// Scroll down.
		if (doWrap) {
			// Try to select some child on this column.
			do {
				y++, y %= _height;
				if (selectChild(x, y)) {
					return;
				}
			} while (y != oy);
		} else {
			// Try to select some child on this column.
			for (y++; y < _height; y++) {
				if (selectChild(x, y)) {
					return;
				}
			}
		}
		
	} else if (which == InputButton::LEFT) {
		// Scroll up.
		if (doWrap) {
			// Try to select some child on this row.
			do {
				x += _width - 1, x %= _width;
				if (selectChild(x, y)) {
					return;
				}
			} while (x != ox);
		} else {
			// Try to select some child on this row.
			for (x--; x >= 0; x--) {
				if (selectChild(x, y)) {
					return;
				}
			}
		}
		
	} else if (which == InputButton::RIGHT) {
		// Scroll down.
		if (doWrap) {
			// Try to select some child on this row.
			do {
				x++, x %= _width;
				if (selectChild(x, y)) {
					return;
				}
			} while (x != ox);
		} else {
			// Try to select some child on this row.
			for (x++; x < _width; x++) {
				if (selectChild(x, y)) {
					return;
				}
			}
		}
		
	}
}

// Button released event.
void GridContainer::buttonUp(InputButton which) {
	if (selected == -1) return;
	
	if (children[selected]->focus >= FocusState::CAPTURED) {
		// Forward the inputs.
		children[selected]->buttonUp(which);
		
	} else if (which == InputButton::CENTER || which == InputButton::ACCEPT) {
		// This will potentially cause child to capture inputs.
		children[selected]->buttonUp(which);
	}
}


} // namespace pax::gui
