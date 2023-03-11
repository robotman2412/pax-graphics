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

#include <pax_internal.h>
#include <pax_gui_filepicker.hpp>
#include <dirent.h>
#include <string.h>

static const char *TAG = "pax-filepicker";

namespace pax::gui {

// Button pressed event.
void FileEntry::buttonDown(InputButton which) {
	if (which == InputButton::ACCEPT && onPress) {
		onPress(this);
	}
}

// Button released event.
void FileEntry::buttonUp(InputButton which) {}


// Draw this element to `buf`.
void FileEntry::draw(Buffer &buf) {
	auto &theme = *getTheme();
	
	// Draw icon, if any.
	if (icon) {
		buf.drawImage(*icon, bounds.x+1, bounds.y+1);
	}
	
	// Compute name alignment.
	auto  size = Buffer::stringSize(theme.font, theme.fontSize, name);
	float dx   = bounds.h;
	float dy   = (bounds.h - size.y) / 2;
	
	// Draw name.
	buf.drawString(
		theme.textColor, theme.font, theme.fontSize,
		bounds.x + dx, bounds.y + dy,
		name
	);
	
	// Compute type alignment.
	size = Buffer::stringSize(theme.font, theme.fontSize, type);
	dx   = bounds.w - size.x - 2;
	dy   = (bounds.h - size.y) / 2;
	
	// Draw type.
	buf.drawString(
		theme.textColor, theme.font, theme.fontSize,
		bounds.x + dx, bounds.y + dy,
		type
	);
	
	// Draw outline.
	if (focus >= FocusState::HIGHLIGHTED) {
		buf.outlineRect(
			focus >= FocusState::FOCUSSED
				? theme.highlightColor
				: theme.outlineColor,
			bounds.x,   bounds.y,
			bounds.w-1, bounds.h-1
		);
	}
}



// Make a file picker in a certain directory.
FilePicker::FilePicker(Rectf _bounds, std::string dir, std::string filename):
	Element(_bounds), ListContainer(_bounds) {
	if (dir == "") dir = "/";
	changeDir(dir);
	changeFilename(filename);
}


// Change directory.
// Returns success status.
bool FilePicker::changeDir(std::string _dir) {
	if (dir == _dir) return true;
	dir = _dir;
	if (dir.back() != '/') dir += '/';
	
	// Clear out the list.
	unselect();
	children.clear();
	
	// And build a new list.
	DIR *dirp = opendir(dir.c_str());
	if (!dirp) {
		PAX_LOGE(TAG, "Error reading directory `%s`: %s\n", dir.c_str(), strerror(errno));
		return false;
	}
	errno = 0;
	while (1) {
		// Try to read an entry.
		auto ent = readdir(dirp);
		if (!ent) {
			if (errno) {
				// If errno is set, report error and return.
				PAX_LOGE(TAG, "Error reading directory `%s`: %s\n", dir.c_str(), strerror(errno));
				return children.size() > 0;
			} else {
				// If not, return success.
				return true;
			}
		}
		
		// Translate this entry into an element.
		FileEntry elem({0, 0, bounds.w, 20});
		
		// Get the name.
		std::size_t name_len = strnlen(ent->d_name, 256);
		elem.name = std::string(ent->d_name, name_len);
		
		// And the type.
		elem.type = ent->d_type == DT_DIR ? "Folder" : "File";
		
		// Set a callback.
		elem.onPress = [this](FileEntry *ent) {
			changeFilename(ent->name);
		};
		appendChildT(std::move(elem));
	}
}

// Change the currently selected path.
// Returns success status.
bool FilePicker::changePath(std::string _path) {
	std::size_t index = _path.find_last_of('/');
	if (index < _path.size()) {
		return changeDir(_path.substr(0, index))
			&& changeFilename(_path.substr(index+1));
	} else {
		return changeDir(_path);
	}
}

// Change the currently selected filename.
// Returns success status.
bool FilePicker::changeFilename(std::string _filename) {
	// Special condition: Empty filename means unselect.
	if (_filename == "") {
		filename = "";
		unselect();
		return true;
	}
	
	// Look for entry in list.
	for (auto &ptr: children) {
		auto &elem = *(FileEntry *) ptr.get();
		if (elem.name == _filename) {
			filename = _filename;
			selectChild(ptr);
			return true;
		}
	}
	
	return false;
}


// Get the current directory.
std::string FilePicker::getDir() {
	return dir;
}

// Get the currently selected path.
std::string FilePicker::getPath() {
	return dir + filename;
}

// Get the currently selected filename.
std::string FilePicker::getFilename() {
	return filename;
}

}
