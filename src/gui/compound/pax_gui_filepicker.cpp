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
#include <icons.hpp>

#include <dirent.h>
#include <sys/stat.h>

#include <string.h>

// This feature comes when codecs are finally integrated normally.
#define PAX_COMPILE_CODECS 0

#if PAX_COMPILE_CODECS
// Decodes a PNG file into a PAX buffer with the specified type.
// Returns 1 on successful decode, refer to pax_last_error otherwise.
// It is not gauranteed the type equals buf_type.
extern "C" bool pax_decode_png_fd(pax_buf_t *buf, FILE *fd, pax_buf_type_t buf_type, int flags);
#endif

static const char *TAG = "pax-filepicker";

namespace pax::gui {

// A list of getters called when a FileEntry is created.
std::vector<IconGetter> iconGetters;

// Case-insensitive character compare.
static inline bool iequals(char a, char b) {
	if (a == b) return true;
	a |= 0x20;
	b |= 0x20;
	return a == b && a >= 'a' && a <= 'z';
}

// Tests whether an extension matches.
static bool matchExtension(const std::string &name, const std::string &ext) {
	// Find last '.' character.
	auto index = name.find_last_of('.');
	if (index >= name.length()) return false;
	
	// Bounds check.
	if (name.length() - index - 1 != ext.length()) return false;
	
	// Case-insensitive comparison of `name` and `ext`.
	for (std::size_t i = 0; i < ext.length(); i++) {
		if (!iequals(ext[i], name[index+i+1])) return false;
	}
	return true;
}

// Registers a standard set of icon getters to the list.
void addDefaultIconGetters() {
	// Icon: folder.
	iconGetters.push_back([](const std::string &dir, const std::string &name) -> std::shared_ptr<Buffer> {
		if (name == "." || name == "..") {
			return icon::folder();
		}
		
		// Reconstruct pathname.
		std::string pathname = dir + '/';
		pathname += name;
		
		// Get statistics.
		auto dirp = opendir(pathname.c_str());
		if (dirp) {
			closedir(dirp);
			return icon::folder();
		} else {
			return nullptr;
		}
	});
	
	// Icon: text.
	iconGetters.push_back([](const std::string &dir, const std::string &name) -> std::shared_ptr<Buffer> {
		return matchExtension(name, "txt")
			|| matchExtension(name, "text")
			? icon::text() : nullptr;
	});
	
	// Icon: data.
	iconGetters.push_back([](const std::string &dir, const std::string &name) -> std::shared_ptr<Buffer> {
		return matchExtension(name, "c")
			|| matchExtension(name, "cc")
			|| matchExtension(name, "cpp")
			|| matchExtension(name, "h")
			|| matchExtension(name, "hh")
			|| matchExtension(name, "cpp")
			|| matchExtension(name, "py")
			|| matchExtension(name, "java")
			|| matchExtension(name, "rs")
			|| matchExtension(name, "html")
			|| matchExtension(name, "js")
			|| matchExtension(name, "css")
			|| matchExtension(name, "json")
			|| matchExtension(name, "xml")
			|| matchExtension(name, "yml")
			|| matchExtension(name, "ini")
			? icon::data() : nullptr;
	});
	
	// Icon: app.
	iconGetters.push_back([](const std::string &dir, const std::string &name) -> std::shared_ptr<Buffer> {
		return matchExtension(name, "app")
			|| matchExtension(name, "exe")
			? icon::app() : nullptr;
	});
	
#if PAX_COMPILE_CODECS
	// Icon: PNG.
	iconGetters.push_back([](const std::string &dir, const std::string &name) -> std::shared_ptr<Buffer> {
		std::string pathname = dir + '/';
		pathname += name;
		
		if (!matchExtension(name, "png")) return nullptr;
		FILE *fd = fopen(pathname.c_str(), "rb");
		if (!fd) return nullptr;
		pax_buf_t tmp;
		if (pax_decode_png_fd(&tmp, fd, PAX_BUF_16_4444ARGB, 0)) {
			auto ptr = std::make_shared<Buffer>(tmp.buf, tmp.width, tmp.height, tmp.type);
			*ptr->getInternal() = tmp;
			return ptr;
		} else {
			fclose(fd);
			return nullptr;
		}
	});
#endif
}

// Resolve a file entry icon.
static std::shared_ptr<Buffer> resolveIcon(const std::string &dir, const std::string &name) {
	for (auto getter: iconGetters) {
		auto ptr = getter(dir, name);
		if (ptr) return ptr;
	}
	return icon::file();
}



// Create an entry and look up an icon.
FileEntry::FileEntry(Rectf _bounds, const std::string &dir, const std::string &file):
	Element(_bounds), name(file) {
	// Ask all the icon getters.
	icon = resolveIcon(dir, file);
}

// Button pressed event.
void FileEntry::buttonDown(InputButton which) {
	if (which == InputButton::ACCEPT && onPress) {
		onPress(*this);
	}
}

// Button released event.
void FileEntry::buttonUp(InputButton which) {}


// Draw this element to `buf`.
void FileEntry::draw(Buffer &buf) {
	auto &theme = *getTheme();
	
	// Draw icon, if any.
	if (icon) {
		// TODO: Smarter icon alignment.
		buf.drawImage(*icon, bounds.x, bounds.y, bounds.h, bounds.h);
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
	Element(_bounds), ListContainer(_bounds),
	dirSelectable(true), fileSelectable(true) {
	if (dir == "") dir = "/";
	changeDir(dir);
	if (filename != "") {
		changeFilename(filename);
	}
}


// DIR * closer.
struct dir_deletor {
	// Calls `closedir` on `__ptr`
	void operator()(DIR* __ptr) const {
		closedir(__ptr);
	}
};

// Change directory.
// Returns success status.
bool FilePicker::changeDir(std::string _dir, bool force) {
	// The directory reader does not want a '/' at the end.
	if (_dir != "/" && _dir.back() == '/') {
		_dir.pop_back();
	}
	// Do nothing if path doesn't change.
	if (dir == _dir && !force) return true;
	dir = std::move(_dir);
	
	// Clear out the list.
	unselect();
	children.clear();
	
	// Add the "current directory" entry.
	{
		FileEntry elem({0, 0, bounds.w, 32}, dir, ".");
		elem.type = "Folder";
		elem.name = dir == "/" ? "/" : ".";
		elem.onPress = [this](FileEntry &ent) {
			nextPath = dir + "/.";
		};
		appendChildT(std::move(elem));
		selectChild(0);
	}
	
	// If not root directory, add the "going up" entry.
	if (dir != "/") {
		FileEntry elem({0, 0, bounds.w, 32}, dir, "..");
		elem.type = "Folder";
		elem.onPress = [this](FileEntry &ent) {
			if (ent.focus >= FocusState::HIGHLIGHTED) {
				nextPath = dir;
			} else {
				nextPath = dir + "/..";
			}
		};
		appendChildT(std::move(elem));
	}
	
	// Try to open directory.
	std::unique_ptr<DIR, dir_deletor> dirp{opendir(dir.c_str())};
	if (!dirp) {
		PAX_LOGE(TAG, "Error reading directory `%s`: %s\n", dir.c_str(), strerror(errno));
		return false;
	}
	
	// Read directory and build a list.
	bool first = true;
	while (1) {
		// Try to read an entry.
		errno = 0;
		auto ent = readdir(dirp.get());
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
		
		// Skip regular files if `fileSelectable` is false.
		if (!fileSelectable && ent->d_type != DT_DIR) {
			continue;
		}
		
		// Translate this entry into an element.
		std::size_t name_len = strnlen(ent->d_name, 256);
		FileEntry elem({0, 0, bounds.w, 32}, dir, {ent->d_name, name_len});
		
		// And the type.
		elem.type = ent->d_type == DT_DIR ? "Folder" : "File";
		
		// Attach the selection logic.
		if (ent->d_type == DT_DIR) {
			// Highlight / enter logic.
			elem.onPress = [this](FileEntry &ent) {
				if (ent.focus >= FocusState::HIGHLIGHTED) {
					nextPath = dir + '/' + ent.name + '/';
				} else {
					nextPath = dir + '/' + ent.name;
				}
			};
			
		} else {
			// Highlight logic.
			elem.onPress = [this](FileEntry &ent) {
				if (fileSelectable) {
					nextPath = dir + '/' + ent.name;
				}
			};
		}
		
		// Append the new element.
		auto ptr = appendChildT(std::move(elem));
		if (first) {
			filename = ptr->name;
			selectChild(ptr);
		}
		first = false;
	}
}

// Change the currently selected path.
// Returns success status.
bool FilePicker::changePath(std::string _path) {
	std::size_t index = _path.find_last_of('/');
	if (index < _path.size()) {
		auto res = changeDir(_path.substr(0, index+1));
		if (_path.size() > index + 1) {
			res &= changeFilename(_path.substr(index+1));
		}
		return res;
	} else {
		return changeDir(std::move(_path));
	}
}

// Change the currently selected filename.
// Returns success status.
bool FilePicker::changeFilename(std::string _filename) {
	// Special condition: Empty filename means unselect.
	if (_filename == "") {
		filename.clear();
		unselect();
		return true;
	}
	
	// Look for entry in list.
	for (auto &ptr: children) {
		auto &elem = *(FileEntry *) ptr.get();
		if (elem.name == _filename) {
			filename = std::move(_filename);
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
	auto out = dir + '/';
	out += filename;
	return out;
}

// Get the currently selected filename.
std::string FilePicker::getFilename() {
	return filename;
}


// Button pressed event.
void FilePicker::buttonDown(InputButton which) {
	if (which == InputButton::SELECT) {
		if (onSelect) {
			onSelect(*this);
		}
	} else {
		nextPath.clear();
		ListContainer::buttonDown(which);
		if (nextPath.size()) {
			changePath(nextPath);
		}
	}
}

// Button released event.
void FilePicker::buttonUp(InputButton which) {
	ListContainer::buttonUp(which);
}

}
