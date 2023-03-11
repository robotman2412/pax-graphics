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

#ifndef PAX_GUI_FILEPICKER_HPP
#define PAX_GUI_FILEPICKER_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>
#include <functional>

#include <pax_gui_base.hpp>
#include <pax_gui_container.hpp>

namespace pax::gui {

class FileEntry: public Element {
	public:
		// Callback type.
		using Callback = std::function<void(FileEntry*)>;
		
		// File icon, if any.
		std::shared_ptr<Buffer> icon;
		// Filename.
		std::string name;
		// Determined file type.
		std::string type;
		// Callback for when the user selects this file.
		Callback onPress;
		
		FileEntry(Rectf _bounds = {0,0,100,20}): Element(_bounds) {}
		
		// Button pressed event.
		virtual void buttonDown(InputButton which) override;
		// Button released event.
		virtual void buttonUp(InputButton which) override;
		
		// Draw this element to `buf`.
		virtual void draw(Buffer &buf) override;
};

class FilePicker: protected ListContainer, public virtual Element {
	protected:
		// Current directory.
		std::string dir;
		// Current selected filename, if any.
		std::string filename;
		
	public:
		// Make a file picker in a certain directory.
		FilePicker(Rectf bounds={0, 0, 200, 200}, std::string dir="/", std::string filename="");
		
		// Change directory.
		// Returns success status.
		bool changeDir(std::string dir);
		// Change the currently selected path.
		// Returns success status.
		bool changePath(std::string path);
		// Change the currently selected filename.
		// Returns success status.
		bool changeFilename(std::string filename);
		
		// Get the current directory.
		std::string getDir();
		// Get the currently selected path.
		std::string getPath();
		// Get the currently selected filename.
		std::string getFilename();
		
		using ListContainer::tick;
		using ListContainer::buttonDown;
		using ListContainer::buttonUp;
		using ListContainer::draw;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_FILEPICKER_HPP
