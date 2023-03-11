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
	protected:
		// Previous value of the bounds variable.
		// Checked in draw() and tick().
		Rectf previousBounds;
		// Index of selected child, if any.
		int selected;
		// Child elements.
		// Position of children is relative to parent.
		std::vector<std::shared_ptr<Element>> children;
		
		// Called when previousBounds != bounds.
		virtual void boundsChanged();
		
	public:
		// Element background color, if any.
		Color background;
		
		// Make an empty container.
		Container(Rectf bounds = {0, 0, 100, 100}):
			Element(bounds), selected(-1), background(0) {}
		
		// This is required to allow subclasses with virtuals.
		virtual ~Container() = default;
		
		// Unselect children, if any.
		void unselect();
		// Select a specific child by index.
		// Returns false if the index is invalid.
		bool selectChild(int index);
		// Select a specific child by value.
		// Returns false if the element is not a child.
		bool selectChild(std::shared_ptr<Element> child);
		// Obtain the index of a selected child, if any.
		// Returns -1 when not selected.
		int selectedIndex();
		// Obtain the pointer of a selected child, if any.
		std::shared_ptr<Element> selectedElement();
		
		// Add a chile (templated move edition).
		// Returns a shared_ptr to the child.
		template<typename T>
		std::shared_ptr<T> appendChildT(T &&child) {
			std::shared_ptr<T> ptr = std::make_shared<T>(std::move(child));
			appendChild(ptr);
			return ptr;
		}
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
		virtual void draw(Buffer &buf) override;
		// Callback to run every so often.
		// Returns true if the object has to be redrawn.
		virtual bool tick(uint64_t millis, uint64_t deltaMillis) override;
};

// A container that stores elements in vertical list form.
class ListContainer: public Container {
	protected:
		// Called when previousBounds != bounds.
		virtual void boundsChanged() override;
		// Update the positions of all children.
		void updateChildren();
		
	public:
		// Whether to wrap when the user presses down at the bottom / up at the top.
		bool doWrap;
		// Make an empty container.
		ListContainer(Rectf bounds = {0, 0, 100, 100}): Container(bounds) {}
		
		// Add a child element (move edition).
		// Returns a shared_ptr to the child.
		// Overrides the position of `child` to fit in the list.
		virtual std::shared_ptr<Element> appendChild(Element &&child) override;
		// Add a child element (from exiting shared_ptr edition).
		// Overrides the position of `child` to fit in the list.
		virtual void appendChild(std::shared_ptr<Element> child) override;
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) override;
		// Button released event.
		virtual void buttonUp(InputButton which) override;
};

// A container that stores elements in a 2D grid.
// Each gets the same space allocation.
class GridContainer: public Container {
	public:
		// Position on grid type.
		struct Pos {
			int x, y;
			bool operator==(Pos other) const {
				return x == other.x && y == other.y;
			}
		};
		
	protected:
		// Called when previousBounds != bounds.
		virtual void boundsChanged() override;
		
		// Helper type that iterates over positions.
		class PosIter {
			protected:
				// Position.
				int x, y;
				// Bounds.
				int w, h;
				
			public:
				PosIter(int x, int y, int w, int h):
					x(x), y(y), w(w), h(h) {}
				
				// Advance to next position and get current.
				Pos next() {
					Pos out = {x, y};
					x ++;
					y += x > w;
					x %= w;
					y %= h;
					return out;
				}
				// Skip N positions.
				void skip(int n) {
					x += n;
					y += x / w;
					x %= w;
					y %= h;
				}
		};
		
		// Which grid positions have been given to the children.
		std::vector<Pos> childPos;
		// Dimensions of the grid.
		int _width, _height;
		
		// Find the first empty position.
		Pos findFirstEmpty();
		// Find the next empty position.
		Pos findNextEmpty();
		// Update the position of a specific child by index.
		void updateChild(int index);
		// Update the positions of all children.
		void updateChildren();
		
	public:
		// Whether to wrap when the user presses directions off the edge.
		bool doWrap;
		
		// Make an empty container.
		GridContainer(Rectf bounds, int width, int height);
		// Make an empty container.
		GridContainer(int width, int height):
			GridContainer({0, 0, 100, 100}, width, height) {}
		
		// Get size of grid.
		int width() const;
		// Get size of grid.
		int height() const;
		
		using Container::selectChild;
		// Select a specific child by grid position.
		// Returns false if there is no child found.
		bool selectChild(int x, int y);
		
		// Add a child element (move edition).
		// Returns a shared_ptr to the child.
		// Overrides the position of `child` to fit in the grid.
		virtual std::shared_ptr<Element> appendChild(Element &&child) override;
		// Add a child element (from exiting shared_ptr edition).
		// Overrides the position of `child` to fit in the grid.
		virtual void appendChild(std::shared_ptr<Element> child) override;
		
		// Add a child element to a specific grid position (move edition).
		// Returns a shared_ptr to the child.
		// Overrides the position of `child` to fit in the grid.
		template<typename T>
		std::shared_ptr<T> attachT(T &&child, int x, int y) {
			std::shared_ptr<T> ptr = std::make_shared<T>(std::move(child));
			attach(ptr, x, y);
			return ptr;
		}
		// Add a child element to a specific grid position (move edition).
		// Returns a shared_ptr to the child.
		// Overrides the position of `child` to fit in the grid.
		virtual std::shared_ptr<Element> attach(Element &&child, int x, int y);
		// Add a child element to a specific grid position.
		// Overrides the position of `child` to fit in the grid.
		virtual void attach(std::shared_ptr<Element> child, int x, int y);
		
		// Remove a child element by grid position.
		// Returns whether a matching child was removed.
		virtual bool removeChild(int x, int y);
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) override;
		// Button released event.
		virtual void buttonUp(InputButton which) override;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_CONTAINER_HPP
