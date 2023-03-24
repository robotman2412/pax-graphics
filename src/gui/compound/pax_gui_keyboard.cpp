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

#include <pax_gui_keyboard.hpp>

namespace pax::gui {

using KmType = const std::string[4][10];

// Lowercase keymap.
static const std::string km_lowercase[4][10] = {
	{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"},
	{"a", "s", "d", "f", "g", "h", "j", "k", "l", ""},
	{"z", "x", "c", "v", "b", "n", "m", ""},
	{",", "."},
};

// Uppercase keymap.
static const std::string km_uppercase[4][10] = {
	{"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
	{"A", "S", "D", "F", "G", "H", "J", "K", "L", ""},
	{"Z", "X", "C", "V", "B", "N", "M", ""},
	{"<", ">"},
};

// Numbers keymap.
static const std::string km_numbers[4][10] = {
	{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
	{"@", "#", "$", "_", "&", "-", "+", "(", ")", "/"},
	{"*", "\"", "'", ":", ";", "!", "?", ""},
	{",", "."},
};

// Symbols keymap: non-unicode edition.
static const std::string km_symbols[4][10] = {
	{"(", ")", "[", "]", "{", "}", "`", "~", ""},
	{"/", "|", "\\", "+", "-", "_", "=", ""},
	{"^", "%", "<", ">", "'", "\"", ""},
	{",", "."},
};

// Symbols keymap: unicode edition.
static const std::string km_unicode[4][10] = {
	{"~", "`", "|", "•", "√", "π", "÷", "×", "¶", "∆"},
	{"£", "¢", "€", "¥", "^", "°", "=", "{", "}", "\\"},
	{"%", "©", "®", "™", "✓", "[", "]", ""},
	{",", "."},
};

// Type to keymap list.
static const std::string (*keymaps[5])[10] = {
	km_lowercase,
	km_uppercase,
	km_numbers,
	km_symbols,
	km_unicode,
};



// Set the type of keyboard being shown.
void Keyboard::setType(Type next) {
	type = next;
}


// Make a basic keyboard.
Keyboard::Keyboard(Rectf bounds):
	Element(bounds) {
	setType(Type::LOWERCASE);
}


// Button pressed event.
void Keyboard::buttonDown(InputButton which) {
	
}

// Button released event.
void Keyboard::buttonUp(InputButton which) {
	
}


// Callback to run every so often.
// Returns true if the object has to be redrawn.
bool Keyboard::tick(uint64_t millis, uint64_t deltaMillis) {
	return false;
}

// Draw this element to `buf`.
void Keyboard::draw(Buffer &buf) {
	
}

} // namespace pax::gui
