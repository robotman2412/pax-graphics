
#include "pax_cxx_text.hpp"
#include <pax_gfx.h>

namespace pax {

// Wow very complicated.
TextElement::TextElement(std::string str) {
	updateText(str);
}

// Update text.
void TextElement::updateText(std::string str) {
	text = str;
}


// Get ascent above baseline.
float TextElement::getAscent(TextBox &ctx, TextStyle &style) {
	return style.fontSize;
}

// Get descent below baseline.
float TextElement::getDescent(TextBox &ctx, TextStyle &style) {
	return 0;
}

// Compute and get width.
// This is called once after the start of drawing.
void TextElement::calcSize(TextBox &ctx, TextStyle &style) {
	textWidth = pax_text_size(style.font, style.fontSize, text.c_str()).x;
}

// Get width after computation.
float TextElement::getWidth(TextBox &ctx, TextStyle &style) {
	return textWidth;
}

// Draw the element.
void TextElement::draw(Buffer &to, TextBox &ctx, TextStyle &style) {
	to.drawString(style.color, style.font, style.fontSize, 0, -style.fontSize, text);
}



// Append a string of text to the grand list.
// Words are broken at whitespace, to override use the non-breaking space character.
void TextBox::appendText(std::string text) {
	// The start index used for substrings.
	size_t startIndex = 0;
	
	// Skip leading whitespace.
	while (startIndex < text.length() && isWhitespace(text[startIndex])) startIndex++;
	if (text.length() <= startIndex) return;
	
	// Continuously try to split into words.
	while (startIndex < text.length()) {
		size_t endIndex = startIndex;
		
		// Look for next whitespace.
		while (endIndex < text.length() && !isWhitespace(text[endIndex])) endIndex++;
		// Append the stuff inbetween.
		append<TextElement>(TextElement(text.substr(startIndex, endIndex-startIndex)));
		
		// Skip leading whitespace.
		startIndex = endIndex;
		while (startIndex < text.length() && isWhitespace(text[startIndex])) startIndex++;
	}
}

// Append a style change before the next element.
void TextBox::appendStyle(TextStyle newStyle) {
	textStyle = newStyle;
}


// A single line's worth of `draw()`.
Rectf TextBox::drawLine(Buffer &to, Vec2f pos, size_t startIndex, size_t endIndex) {
	Rectf bounds{pos, {0,0}};
	return bounds;
}

// Draw the `TextBox` and all it's contents.
void TextBox::draw(Buffer &to) {
	if (!list.size()) return;
	
	// Ensure positive dimensions for bounds.
	bounds = bounds.fixSize();
	to.pushMatrix();
	to.translate(bounds.x, bounds.y);
	
	// Index to start drawing the current line.
	size_t startIndex = 0;
	// One past last index to draw the current line.
	size_t endIndex = 0;
	// Whether text styling is currently being applied.
	bool isText = list[0].second->isText();
	// Index at which text styling started.
	size_t styleIndex = 0;
	// Current text style.
	TextStyle &currentStyle = list[0].first;
	// Current size of space.
	Vec2f spaceSize = pax_text_size(currentStyle.font, currentStyle.fontSize, " ");
	// Current line's width.
	float lineWidth = 0, elementWidth = 0;
	// Current line's height.
	float lineAscent = 0, lineDescent = 0;
	// Current total height.
	float totalHeight = 0;
	
	while (startIndex < list.size()) {
		// Add a space into the minimum width calculation.
		float theSpace = (endIndex > startIndex) ? spaceSize.x : 0;
		
		// Test whether another element still fits.
		if (endIndex < list.size()) {
			InlineElement &elem = *list[endIndex].second;
			if (elem.getWidth(*this, currentStyle) + lineWidth + theSpace <= bounds.w) {
				// If so, count it up and continue going.
				
				// Minimum possible width.
				lineWidth += theSpace + elem.getWidth(*this, currentStyle);
				// Element-only width (used for justification).
				elementWidth += elem.getWidth(*this, currentStyle);
				
				// Line ascent.
				float asc = elem.getAscent(*this, currentStyle);
				if (asc > lineAscent) lineAscent = asc;
				// Line descent.
				float desc = elem.getDescent(*this, currentStyle);
				if (desc > lineDescent) lineDescent = desc;
				
				// Next element please!
				endIndex ++;
				continue;
			}
		}
		
		// If no more elements fit, it is time to draw the entire line.
		to.pushMatrix();
		
		// Determine horizontal spacing.
		float spacing = spaceSize.x;
		if (alignment == TextAlign::JUSTIFY && endIndex > startIndex + 1) {
			spacing = (bounds.w - elementWidth) / (endIndex - startIndex - 1);
		}
		
		// Translate to match alignment.
		switch (alignment) {
			default: // TextAlign::LEFT, TextAlign::JUSTIFY
				// Left-aligned.
				to.translate(0, lineAscent + totalHeight);
				break;
				
			case TextAlign::CENTER:
				// Center-aligned.
				to.translate((bounds.w - lineWidth) / 2, lineAscent + totalHeight);
				break;
				
			case TextAlign::RIGHT:
				// Right-aligned.
				to.translate(bounds.w - lineWidth, lineAscent + totalHeight);
				break;
		}
		
		// Draw individual words.
		for (size_t i = startIndex; i < endIndex; i++) {
			// Draw word.
			Entry &pair = list[i];
			pair.second->draw(to, *this, pair.first);
			
			// Move to the right a bit.
			to.translate(pair.second->getWidth(*this, pair.first) + spacing, 0);
		}
		
		to.popMatrix();
		
		// Count in the new dimensions.
		totalHeight += lineAscent + lineDescent;
		lineAscent=lineDescent=lineWidth=elementWidth=0;
		startIndex = endIndex;
	}
	
	to.popMatrix();
}

} // namespace pax
