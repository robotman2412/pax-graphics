
#include "pax_gui_container.hpp"

#include <algorithm>

namespace pax::gui {

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
	if (selectedChild >= 0 && selectedChild < children.size()) {
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
void Container::draw(Buffer &buf, bool selected) {
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
			iter->draw(buf, selected && selectedChild == i);
		}
	}
	
	// Restore transformation.
	buf.popMatrix();
}

} // namespace pax::gui
