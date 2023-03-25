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

// Lowercase keymap.
static const std::string km_lowercase[4][10] = {
	{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"},
	{"a", "s", "d", "f", "g", "h", "j", "k", "l", {}},
	{" ", "z", "x", "c", "v", "b", "n", "m", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Uppercase keymap.
static const std::string km_uppercase[4][10] = {
	{"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
	{"A", "S", "D", "F", "G", "H", "J", "K", "L", {}},
	{"Z", "X", "C", "V", "B", "N", "M", {}},
	{" ", "<", " ", " ", " ", " ", " ", ">", " ", {}},
};

// Numbers keymap.
static const std::string km_numbers[4][10] = {
	{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
	{"@", "#", "$", "_", "&", "-", "+", "(", ")", "/"},
	{" ", "*", "\"", "'", ":", ";", "!", "?", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Symbols keymap: non-unicode edition.
static const std::string km_symbols[4][10] = {
	{"(", ")", "[", "]", "{", "}", "`", "~", {}},
	{"/", "|", "\\", "+", "-", "_", "=", {}},
	{" ", "^", "%", "<", ">", "'", "\"", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Symbols keymap: unicode edition.
static const std::string km_unicode[4][10] = {
	{"~", "`", "|", "•", "√", "π", "÷", "×", "¶", "∆"},
	{"£", "¢", "€", "¥", "^", "°", "=", "{", "}", "\\"},
	{" ", "%", "©", "®", "™", "✓", "[", "]", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Type to keymap list.
static const std::string (*keymaps[5])[10] = {
	km_lowercase,
	km_uppercase,
	km_numbers,
	km_symbols,
	km_unicode,
};



// Perform transformations for key art.
static inline void keyArtTransform(Buffer &buf, int x, int y, int charW, int charH) {
	auto &theme = *getTheme();
	float scale = fminf(theme.fontSize, charW - 2);
	
	buf.translate(x + (charW - scale)/2, y + (charH - scale)/2);
	buf.scale(scale, scale);
}

// Art for the accept key.
static void keyArtAccept(Buffer &buf, int x, int y, int charW, int charH, bool selected) {
	auto &theme = *getTheme();
	Color col = selected ? theme.highlightColor : theme.textColor;
	
	buf.pushMatrix();
	keyArtTransform(buf, x, y, charW, charH);
	
	buf.drawLine(col, 0.25, 0.5, 0.5, 1);
	buf.drawLine(col, 0.5,  1,   1,   0);
	
	buf.popMatrix();
}

// Art for the backspace key.
static void keyArtBackspace(Buffer &buf, int x, int y, int charW, int charH, bool selected) {
	auto &theme = *getTheme();
	Color col = selected ? theme.highlightColor : theme.textColor;
	
	buf.pushMatrix();
	keyArtTransform(buf, x, y, charW, charH);
	
	// The stopper.
	buf.drawLine(col, 0,   0.25,  0,    0.75);
	// The arrow.
	buf.drawLine(col, 0.1, 0.5,   0.35, 0.25);
	buf.drawLine(col, 0.1, 0.5,   0.35, 0.75);
	buf.drawLine(col, 0.1, 0.5,   1,    0.5);
	
	buf.popMatrix();
}

// Art for the shift key.
static void keyArtShift(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool shift) {
	auto &theme = *getTheme();
	Color col = selected ? theme.highlightColor : theme.textColor;
	
	buf.pushMatrix();
	keyArtTransform(buf, x, y, charW, charH);
	buf.translate(0.5, 0);
	
	if (shift) {
		// Filled in shift key.
		buf.drawTri (col, -0.5, 0.5, 0, 0, 0.5, 0.5);
		buf.drawRect(col, -0.25, 0.5, 0.5, 0.5);
	} else {
		// Outlined shift key.
		buf.drawLine(col,  0,    0,    0.5,  0.5);
		buf.drawLine(col,  0.5,  0.5,  0.25, 0.5);
		buf.drawLine(col,  0.25, 0.5,  0.25, 1);
		buf.drawLine(col,  0.25, 1,   -0.25, 1);
		buf.drawLine(col, -0.25, 0.5, -0.25, 1);
		buf.drawLine(col, -0.5,  0.5, -0.25, 0.5);
		buf.drawLine(col,  0,    0,   -0.5,  0.5);
	}
	
	buf.popMatrix();
}

// Art for the board selection key.
static void keyArtSelect(Buffer &buf, int x, int y, int charW, int charH, bool selected, Keyboard::Type type) {
	auto &theme = *getTheme();
	Color col = selected ? theme.highlightColor : theme.textColor;
	
	// Select string candidates.
	const char *shortStr;
	const char *longStr;
	switch (type) {
		case Keyboard::Type::NUMBERS:
			shortStr = "%";
			longStr  = "%&~";
			break;
			
		case Keyboard::Type::SYMBOLS:
		case Keyboard::Type::UNICODE:
			shortStr = "A";
			longStr  = "Abc";
			break;
			
		default:
			shortStr = "#";
			longStr  = "123";
			break;
	}
	
	// Compute position and font size.
	const char *str       = longStr;
	Vec2f       dims      = pax_text_size(theme.font, theme.font->default_size, str);
	int         font_size = 9;//(dx - 4) / dims.x * dims.y;
	if (font_size < dims.y) {
		str       = shortStr;
		dims      = pax_text_size(theme.font, theme.font->default_size, str);
		font_size = 9;//(dx - 4) / dims.x * dims.y;
	}
	dims = pax_text_size(theme.font, font_size, str);
	
	// Draw selected string.
	buf.drawString(
		col,
		theme.font, theme.fontSize,
		x+(charW-dims.x)/2,
		y+(charH-font_size)/2,
		str
	);
}

// Art for the space key.
static void keyArtSpace(Buffer &buf, int x, int y, int charW, int charH, bool selected) {
	auto &theme = *getTheme();
	Color col = selected ? theme.highlightColor : theme.textColor;
	
	buf.pushMatrix();
	buf.translate(x, y);
	buf.scale(charW, charH);
	
	buf.drawRect(col, 0.25, 0.333, 4.5, 0.333);
	
	buf.popMatrix();
}



// Set the type of keyboard being shown.
void Keyboard::setType(Type next) {
	type = next;
}

// Draws a row of the keyboard.
void Keyboard::drawRow(Buffer &buf, const std::string *chars, int selected, int row, int charW, int charH) {
	auto &theme = *getTheme();
	int y = bounds.h + (row - 4) * charH;
	
	// Count number of stuf in chars.
	int len;
	for (len = 0; len < 10; len++) {
		if (chars[len].empty()) break;
	}
	
	// Draw a series of boxes with characters.
	int x = (bounds.w - charW * len) / 2;
	for (int i = 0; i < len; i++) {
		// Optional box.
		if (selected == i) {
			buf.outlineRect(
				theme.highlightColor,
				x + charW*i, y,
				charW, charH
			);
		}
		
		// The CHAR.
		if (chars[i][0] != ' ') {
			buf.drawStringCentered(
				theme.textColor,
				theme.font, theme.fontSize,
				x + charW*i + charW/2,
				y + (charH - theme.fontSize)/2,
				chars[i]
			);
		}
	}
	
	// Special case: shift key and backspace key.
	if (row == 2) {
		keyArtShift(buf, x, y, charW, charH, selected == 0, shift);
		keyArtBackspace(buf, x+charW*(len-1), y, charW, charH, selected == len-1);
	}
	
	// Special case: board select key, spacebar and accept key.
	if (row == 3) {
		keyArtSelect(buf, x, y, charW, charH, selected == 0, type);
		keyArtSpace(buf, x+charW*2, y, charW, charH, selected > 1 && selected < len-1);
		keyArtAccept(buf, x+charW*(len-1), y, charW, charH, selected == len-1);
	}
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
	// Compute some size parameters.
	int textBoxHeight = bounds.h / 2;
	int charW = bounds.w / 10;
	int charH = (bounds.h - textBoxHeight) / 4;
	textBoxHeight = bounds.h - charH * 4;
	
	// Select keyboard layout.
	const std::string (*keymap)[10] = keymaps[(int) type];
	
	// TODO: Draw text box.
	
	// Draw the base keyboard.
	for (int i = 0; i < 4; i++) {
		drawRow(buf, keymap[i], i == y ? x : -1, i, charW, charH);
	}
}

} // namespace pax::gui
