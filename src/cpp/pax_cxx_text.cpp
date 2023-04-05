
#include "pax_cxx_text.hpp"
#include <pax_gfx.h>

namespace pax {

static const Matrix2f italicMtx = Matrix2f::shear(0, -0.2);



// Wow very complicated.
TextElement::TextElement(std::string str) {
	updateText(str);
}

// Update text.
void TextElement::updateText(std::string str) {
	text = str;
}


// Get ascent above baseline.
float TextElement::getAscent(TextBox &ctx, TextStyle &style) const {
	return style.fontSize;
}

// Get descent below baseline.
float TextElement::getDescent(TextBox &ctx, TextStyle &style) const {
	return 0;
}

// Compute and get width.
// This is called once after the start of drawing.
void TextElement::calcSize(TextBox &ctx, TextStyle &style) {
	textWidth = pax_text_size(style.font, style.fontSize, text.c_str()).x;
}

// Get width after computation.
float TextElement::getWidth(TextBox &ctx, TextStyle &style) const {
	return textWidth;
}

// Draw the element.
void TextElement::draw(Buffer &to, TextBox &ctx, TextStyle &style) {
	if (style.italic) {
		to.pushMatrix();
		to.applyMatrix(italicMtx);
		to.drawString(style.color, style.font, style.fontSize, 0, -style.fontSize, text);
		to.popMatrix();
	} else {
		to.drawString(style.color, style.font, style.fontSize, 0, -style.fontSize, text);
	}
}



// Compute and get dimensions.
// This is called once after the start of drawing or when this element's style changes.
void SpaceElement::calcSize(TextBox &ctx, TextStyle &style) {
	width = pax_text_size(style.font, style.fontSize, " ").x;
}

// Get width after computation.
float SpaceElement::getWidth(TextBox &ctx, TextStyle &style) const {
	return width;
}



// Wow very complicated.
InlineImage::InlineImage(pax_buf_t *image) {
	this->image = image;
}

// Wow very complicated.
InlineImage::InlineImage(Buffer *image) {
	this->image = image->internal;
}


// Get ascent above baseline.
float InlineImage::getAscent(TextBox &ctx, TextStyle &style) const {
	return image ? image->height : 0;
}

// Get descent below baseline.
float InlineImage::getDescent(TextBox &ctx, TextStyle &style) const {
	return 0;
}

// Compute and get dimensions.
// This is called once after the start of drawing or when this element's style changes.
void InlineImage::calcSize(TextBox &ctx, TextStyle &style) {}

// Get width after computation.
float InlineImage::getWidth(TextBox &ctx, TextStyle &style) const {
	return image ? image->width : 0;
}

// Draw the element.
void InlineImage::draw(Buffer &to, TextBox &ctx, TextStyle &style)  {
	if (image) {
		pax_draw_image(to.internal, image, 0, -image->height);
	}
}



// Skip whitespace and append any spaces and newlines found.
static void skipWhitespace(TextBox &box, std::string &text, size_t &startIndex) {
	bool hasSpaces = false;
	while (startIndex < text.length()) {
		if (text[startIndex] == '\r') {
			// Append spaces, if any.
			if (hasSpaces) {
				hasSpaces = false;
				box.append(SpaceElement());
			}
			
			// Check for LF after CR.
			if (startIndex < text.length() - 1 && text[startIndex+1] == '\n') {
				startIndex ++;
			}
			// Append newline token.
			box.append(NewlineElement());
			
		} else if (text[startIndex] == '\n') {
			// Append spaces, if any.
			if (hasSpaces) {
				hasSpaces = false;
				box.append(SpaceElement());
			}
			
			// Append newline token.
			box.append(NewlineElement());
			
		} else if (!TextBox::isWhitespace(text[startIndex])) {
			// Append spaces, if any.
			if (hasSpaces) {
				hasSpaces = false;
				box.append(SpaceElement());
			}
			break;
			
		} else /* TextBox::isWhitespace(text[startIndex]) */ {
			// Mark as having spaces.
			hasSpaces = true;
		}
		startIndex++;
	}
}

// Append a string of text to the grand list.
// Words are broken at whitespace, to override use the non-breaking space character.
void TextBox::appendText(std::string text) {
	// The start index used for substrings.
	size_t startIndex = 0;
	
	// Skip leading whitespace.
	skipWhitespace(*this, text, startIndex);
	if (text.length() <= startIndex) return;
	
	// Continuously try to split into words.
	while (startIndex < text.length()) {
		size_t endIndex = startIndex;
		
		// Look for next whitespace.
		while (endIndex < text.length() && !isWhitespace(text[endIndex])) endIndex++;
		// Append the stuff inbetween.
		append(TextElement(text.substr(startIndex, endIndex-startIndex)));
		
		// Skip leading whitespace.
		startIndex = endIndex;
		skipWhitespace(*this, text, startIndex);
	}
}

// Append a style change before the next element.
void TextBox::appendStyle(TextStyle newStyle) {
	textStyle = newStyle;
}


// Draw the `TextBox` and all it's contents.
void TextBox::draw(Buffer &to) {
	if (!list.size()) return;
	
	// Ensure positive dimensions for bounds.
	bounds = bounds.fixSize();
	to.pushMatrix();
	to.translate(bounds.x, bounds.y);
	
	// Index to start drawing the current line.
	size_t startIndex  = 0;
	// One past last index to draw the current line.
	size_t endIndex    = 0;
	// Last index of multiple space character in the current line.
	size_t lastSpace   = 0;
	// Whether the current line ends with space characters.
	bool endsWithSpace = false;
	// Count of non-whitespace characters in current line.
	size_t elemCount   = 0;
	
	// Current text style.
	TextStyle &style  = list[0].first;
	
	// Current size of space.
	Vec2f spaceSize   = pax_text_size(style.font, style.fontSize, " ");
	// Current line's width.
	float lineWidth   = 0, elementWidth = 0;
	// Current line's height.
	float lineAscent  = 0, lineDescent  = 0;
	// Current total height.
	float totalHeight = 0;
	
	while (startIndex < list.size()) {
		// Test whether another element still fits.
		if (endIndex < list.size()) {
			// Grab a new element from the list.
			InlineElement &elem = *list[endIndex].second;
			TextStyle &style = list[endIndex].first;
			
			// Determine whether it shall fit on this line.
			bool doesItFit = elem.getWidth(*this, style) + lineWidth <= bounds.w;
			// Edge case: Newlines "never fit" to force line break.
			if (elem.type() == InlineElement::NEWLINE) doesItFit = false;
			// Edge case: It is too wide to fit, but the line is empty.
			if (startIndex == endIndex) doesItFit = true;
			
			if (doesItFit) {
				// If so, count it up and continue going.
				
				// Minimum possible width.
				lineWidth += elem.getWidth(*this, style);
				// Non-whitespace width (used for justification).
				if (elem.type() != InlineElement::SPACE) elementWidth += elem.getWidth(*this, style);
				
				// Line ascent.
				float asc = elem.getAscent(*this, style);
				if (asc > lineAscent) lineAscent = asc;
				// Line descent.
				float desc = elem.getDescent(*this, style);
				if (desc > lineDescent) lineDescent = desc;
				
				// Special spaces logic.
				if (elem.type() == InlineElement::SPACE) {
					if (!endsWithSpace) {
						// Update spaces stretcher index.
						lastSpace = endIndex;
					}
					endsWithSpace = true;
					
				} else {
					endsWithSpace = false;
				}
				
				// Update non-whitespace elements count.
				if (elem.type() == InlineElement::TEXT || elem.type() == InlineElement::GENERIC) {
					elemCount ++;
				}
				
				// Next element please!
				endIndex ++;
				continue;
			}
		}
		
		// If no more elements fit, it is time to draw the entire line.
		to.pushMatrix();
		
		// Determine horizontal spacing.
		float spacing = spaceSize.x;
		if (alignment == TextAlign::JUSTIFY && elemCount > 1) {
			spacing = (bounds.w - elementWidth) / (elemCount - 1);
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
		
		// Was the previously drawn element a space?
		bool wasSpace = true;
		// Draw individual words.
		for (size_t i = startIndex; i < endIndex; i++) {
			// Draw word.
			Entry &pair = list[i];
			InlineElement &elem = *pair.second;
			style = pair.first;
			elem.draw(to, *this, style);
			float width = elem.getWidth(*this, style);
			
			// Draw underline, strikethrough and overline.
			float decorHeight = 1;
			if (elem.type() == InlineElement::TEXT || elem.type() == InlineElement::SPACE) {
				// Edge case: Space characters on justify.
				if (alignment == JUSTIFY && elem.type() == InlineElement::SPACE) {
					if (wasSpace) {
						// Corner case: Repeated space on justified lines.
						width = 0;
						
					} else if (endsWithSpace && i >= lastSpace) {
						// Corner case: Justified line ends with space.
						width = 0;
						
					} else {
						// Just set it to normal spacing.
						width = spacing;
					}
				}
				
				// Handle special text styling.
				if (width) {
					if (style.underline) {
						// Draw underline.
						to.drawRect(style.color, 0, 0, width, decorHeight);
					}
					if (style.overline) {
						// Draw overline.
						to.drawRect(style.color, 0, -elem.getAscent(*this, style), width, decorHeight);
					}
					if (style.strikethrough) {
						// Draw strikethrough.
						to.drawRect(style.color, 0, -elem.getAscent(*this, style)/2, width, decorHeight);
					}
				}
			}
			
			// Update previous is space flag.
			wasSpace = elem.type() == InlineElement::SPACE;
			
			// Move to the right a bit.
			to.translate(width, 0);
		}
		
		to.popMatrix();
		
		// Count in the new dimensions.
		totalHeight += lineAscent + lineDescent;
		lineAscent=lineDescent=lineWidth=elementWidth=elemCount=0;
		startIndex = endIndex;
	}
	
	to.popMatrix();
}

} // namespace pax
