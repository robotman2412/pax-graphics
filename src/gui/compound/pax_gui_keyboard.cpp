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
	{" ", "Z", "X", "C", "V", "B", "N", "M", " ", {}},
	{" ", "<", " ", " ", " ", " ", " ", ">", " ", {}},
};

// Numbers keymap.
static const std::string km_numbers[4][10] = {
	{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
	{"@", "#", "$", "_", "&", "-", "+", "(", ")", "/"},
	{" ", "*", "\"","'", ":", ";", "!", "?", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Symbols keymap: non-unicode edition.
static const std::string km_symbols[4][10] = {
	{"(", ")", "[", "]", "{", "}", "`", "~", {}},
	{"/", "|", "\\","+", "-", "_", "=", {}},
	{" ", "^", "%", "<", ">", "'", "\""," ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Symbols keymap: unicode edition.
static const std::string km_unicode[4][10] = {
	{"~", "`", "|", "•", "√", "π", "÷", "×", "¶", "∆"},
	{"£", "¢", "€", "¥", "^", "°", "=", "{", "}", "\\"},
	{" ", "%", "©", "®", "™", "✓", "[", "]", " ", {}},
	{" ", ",", " ", " ", " ", " ", " ", ".", " ", {}},
};

// Row purposes...
struct BoardInfo {
	// Selected keymap.
	const std::string (*keymap)[10];
	// Lengths of each row.
	int lengths[4];
	
	BoardInfo(const std::string (*_keymap)[10], int l0, int l1, int l2, int l3):
		keymap(_keymap), lengths{l0, l1, l2, l3} {}
};

const BoardInfo board_lowercase(km_lowercase, 10, 9, 9, 9);
const BoardInfo board_uppercase(km_uppercase, 10, 9, 9, 9);
const BoardInfo board_numbers  (km_numbers, 10, 10, 9, 9);
const BoardInfo board_symbols  (km_symbols, 8, 7, 8, 9);
const BoardInfo board_unicode  (km_unicode, 10, 10, 9, 9);

// Board info tables: non-unicode edition.
static const BoardInfo *boards_nu[] = {
	&board_lowercase,
	&board_uppercase,
	&board_numbers,
	&board_symbols,
};

// Board info tables: non-unicode edition.
static const BoardInfo *boards_u[] = {
	&board_lowercase,
	&board_uppercase,
	&board_numbers,
	&board_unicode,
};



// Perform transformations for key art.
static inline void keyArtTransform(Buffer &buf, int x, int y, int charW, int charH) {
	auto &theme = *getTheme();
	float scale = fminf(theme.fontSize, charW - 2);
	
	buf.translate(x + (charW - scale)/2, y + (charH - scale)/2);
	buf.scale(scale, scale);
}

// Art for the accept key.
static void keyArtAccept(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool held) {
	auto &theme = *getTheme();
	Color col = selected ? held ? theme.backgroundColor : theme.highlightColor : theme.textColor;
	
	buf.pushMatrix();
	keyArtTransform(buf, x, y, charW, charH);
	
	buf.drawLine(col, 0.25, 0.5, 0.5, 1);
	buf.drawLine(col, 0.5,  1,   1,   0);
	
	buf.popMatrix();
}

// Art for the backspace key.
static void keyArtBackspace(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool held) {
	auto &theme = *getTheme();
	Color col = selected ? held ? theme.backgroundColor : theme.highlightColor : theme.textColor;
	
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
static void keyArtShift(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool held, bool shift) {
	auto &theme = *getTheme();
	Color col = selected ? held ? theme.backgroundColor : theme.highlightColor : theme.textColor;
	
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
static void keyArtSelect(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool held, Keyboard::Type type) {
	auto &theme = *getTheme();
	Color col = selected ? held ? theme.backgroundColor : theme.highlightColor : theme.textColor;
	
	// Select string candidates.
	const char *shortStr;
	const char *longStr;
	switch (type) {
		case Keyboard::Type::NUMBERS:
			shortStr = "%";
			longStr  = "%&~";
			break;
			
		case Keyboard::Type::SYMBOLS:
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
	float       font_size = theme.font->default_size;
	Vec2f       dims      = pax_text_size(theme.font, theme.font->default_size, str);
	if (dims.x > charW) {
		font_size /= dims.x / charW;
	}
	dims = pax_text_size(theme.font, font_size, str);
	
	// Draw selected string.
	buf.drawString(
		col,
		theme.font, font_size,
		x+(charW-dims.x)/2,
		y+(charH-font_size)/2,
		str
	);
}

// Art for the space key.
static void keyArtSpace(Buffer &buf, int x, int y, int charW, int charH, bool selected, bool held) {
	auto &theme = *getTheme();
	Color col = selected ? held ? theme.backgroundColor : theme.outlineColor : theme.textColor;
	
	buf.pushMatrix();
	buf.translate(x, y);
	buf.scale(charW, charH);
	
	if (selected && !held) {
		buf.outlineRect(theme.highlightColor, 0, 0, 5, 1);
	} else if (selected && held) {
		buf.drawRect(theme.highlightColor, 0, 0, 5, 1);
	}
	buf.drawRect(col, 0.25, 0.333, 4.5, 0.333);
	
	buf.popMatrix();
}



// Set the type of keyboard being shown.
void Keyboard::setType(Type next) {
	type = next;
}

// Set value of shift key.
void Keyboard::setShift(bool newShift) {
	shift = newShift;
	if (shift && type == Type::LOWERCASE)  setType(Type::UPPERCASE);
	if (shift && type == Type::NUMBERS)    setType(Type::SYMBOLS);
	if (!shift && type == Type::UPPERCASE) setType(Type::LOWERCASE);
	if (!shift && type == Type::SYMBOLS)   setType(Type::NUMBERS);
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
		// Selection box.
		if (selected == i) {
			if (row == 3 && i > 1 && i < len - 2) continue;
			
			if (held == InputButton::ACCEPT) {
				buf.drawRect(
					theme.highlightColor,
					x + charW*i, y,
					charW, charH
				);
			} else {
				buf.outlineRect(
					theme.highlightColor,
					x + charW*i, y,
					charW-1, charH-1
				);
			}
		}
		
		// Draw the (possibly unicode) character.
		if (chars[i][0] != ' ') {
			buf.drawStringCentered(
				held == InputButton::ACCEPT
				&& selected == i
					? theme.backgroundColor
					: theme.textColor,
				theme.font, theme.fontSize,
				x + charW*i + charW/2,
				y + (charH - theme.fontSize)/2,
				chars[i]
			);
		}
	}
	
	// Special case: shift key and backspace key.
	if (row == 2) {
		keyArtShift(buf, x, y, charW, charH, selected == 0, held == InputButton::ACCEPT, shift);
		keyArtBackspace(buf, x+charW*(len-1), y, charW, charH, selected == len-1, held == InputButton::ACCEPT);
	}
	
	// Special case: board select key, spacebar and accept key.
	if (row == 3) {
		keyArtSelect(buf, x, y, charW, charH, selected == 0, held == InputButton::ACCEPT, type);
		keyArtSpace(buf, x+charW*2, y, charW, charH, selected > 1 && selected < len-2, held == InputButton::ACCEPT);
		keyArtAccept(buf, x+charW*(len-1), y, charW, charH, selected == len-1, held == InputButton::ACCEPT);
	}
}


// Make a basic keyboard.
Keyboard::Keyboard(Rectf bounds):
	Element(bounds),
	type(Type::LOWERCASE), shift(0),
	x(0), y(0), held(InputButton::UNKNOWN) {
	setType(Type::LOWERCASE);
}


// Get the current string value.
std::string Keyboard::getValue() {
	return value;
}

// set the current string value.
void Keyboard::setValue(std::string newValue) {
	value = std::move(newValue);
}


// Type some text into the box.
void Keyboard::input(std::string append) {
	value += append;
}

// Delete character to the left.
void Keyboard::backspace() {
	if (!value.empty()) value.pop_back();
}


// Button pressed event.
void Keyboard::buttonDown(InputButton which) {
	// Grab board layout.
	auto &board = showUnicode ? *boards_u[(int) type] : *boards_nu[(int) type];
	held = which;
	buttonTime = 0;
	
	if (which == InputButton::SELECT) {
		// Cycle keyboard type.
		switch (type) {
			default: // Set to lowercase board.
				setType(Type::LOWERCASE);
				shift = 0;
				break;
				
			case Type::LOWERCASE: // Set to uppercase board.
				setType(Type::UPPERCASE);
				shift = 1;
				break;
				
			case Type::UPPERCASE: // Set to numbers board.
				setType(Type::NUMBERS);
				shift = 0;
				break;
				
			case Type::NUMBERS: // Set to symbols board.
				setType(Type::SYMBOLS);
				shift = 1;
				break;
		}
		
	} else if (which == InputButton::CENTER) {
		// Cycle shift key.
		setShift(true);
		
	} else if (which == InputButton::ACCEPT || which == InputButton::TYPE) {
		// Key pressing logic.
		if (y == 2 && x == 0) {
			// Cycle shift key.
			setShift(!shift);
			
		} else if (y == 3 && x == 0) {
			// Cycle keyboard type.
			switch (type) {
				default: // Set to lowercase board.
					setType(Type::LOWERCASE);
					shift = 0;
					break;
					
				case Type::LOWERCASE: // Set to uppercase board.
					setType(Type::NUMBERS);
					shift = 0;
					break;
					
				case Type::UPPERCASE: // Set to numbers board.
					setType(Type::NUMBERS);
					shift = 0;
					break;
					
				case Type::NUMBERS: // Set to symbols board.
					setType(Type::SYMBOLS);
					shift = 1;
					break;
			}
			
		} else if (y == 2 && x == board.lengths[y]-1) {
			// Backspace.
			backspace();
			
		} else if (y == 3 && x == board.lengths[y]-1) {
			// Accept.
			if (onAccept) onAccept(*this);
			
		} else {
			// Some other key.
			input(board.keymap[y][x]);
		}
		
	} else if (which == InputButton::BACK || which == InputButton::BACKSPACE) {
		// The backspace shortcut.
		backspace();
		
	} else if (which == InputButton::UP) {
		// The navigation.
		y --;
		if (y < 0) y = 3;
		
		// Clamp the right-hand side.
		if (x >= board.lengths[y]) x = board.lengths[y]-1;
		
	} else if (which == InputButton::DOWN) {
		// The navigation.
		y ++;
		if (y > 3) y = 0;
		
		// Clamp the right-hand side.
		if (x >= board.lengths[y]) x = board.lengths[y]-1;
		
	} else if (which == InputButton::LEFT) {
		// The navigation.
		x --;
		if (x < 0) x = board.lengths[y] - 1;
		
	} else if (which == InputButton::RIGHT) {
		// The navigation.
		x ++;
		if (x >= board.lengths[y]) x = 0;
		
	}
}

// Button released event.
void Keyboard::buttonUp(InputButton which) {
	if (held == which) held = InputButton::UNKNOWN;
	if (which == InputButton::CENTER) {
		setShift(false);
	}
}


// Callback to run every so often.
// Returns true if the object has to be redrawn.
bool Keyboard::tick(uint64_t millis, uint64_t deltaMillis) {
	if (buttonTime == 0) {
		buttonTime = millis + 500;
	} else if (millis >= buttonTime + 100) {
		buttonDown(held);
		buttonTime = millis;
		return true;
	}
	return false;
}

// Draw this element to `buf`.
void Keyboard::draw(Buffer &buf) {
	auto &theme = *getTheme();
	
	// Compute some size parameters.
	int textBoxHeight = bounds.h / 2;
	int charW = bounds.w / 10;
	int charH = (bounds.h - textBoxHeight) / 4;
	textBoxHeight = bounds.h - charH * 4;
	
	// Select keyboard layout.
	const std::string (*keymap)[10];
	switch (type) {
		default:              keymap = km_lowercase; break;
		case Type::UPPERCASE: keymap = km_uppercase; break;
		case Type::NUMBERS:   keymap = km_numbers;   break;
		case Type::SYMBOLS:   keymap = km_symbols;   break;
	}
	
	// TODO: Draw text box.
	buf.drawString(theme.textColor, theme.font, theme.fontSize, bounds.x, bounds.y, value);
	
	// Draw the base keyboard.
	buf.pushMatrix();
	buf.translate(bounds.x, bounds.y);
	for (int i = 0; i < 4; i++) {
		drawRow(buf, keymap[i], i == y ? x : -1, i, charW, charH);
	}
	buf.popMatrix();
}

} // namespace pax::gui
