
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
	if (style.italic) {
		to.pushMatrix();
		to.applyMatrix(italicMtx);
		to.drawString(style.color, style.font, style.fontSize, 0, -style.fontSize, text);
		to.popMatrix();
	} else {
		to.drawString(style.color, style.font, style.fontSize, 0, -style.fontSize, text);
	}
}



// Wow very complicated.
ImageElement::ImageElement(pax_buf_t *image) {
	this->image = image;
}

// Wow very complicated.
ImageElement::ImageElement(Buffer *image) {
	this->image = image->internal;
}


// Get ascent above baseline.
float ImageElement::getAscent(TextBox &ctx, TextStyle &style)  {
	return image ? image->height : 0;
}

// Get descent below baseline.
float ImageElement::getDescent(TextBox &ctx, TextStyle &style)  {
	return 0;
}

// Compute and get dimensions.
// This is called once after the start of drawing or when this element's style changes.
void ImageElement::calcSize(TextBox &ctx, TextStyle &style)  {}

// Get width after computation.
float ImageElement::getWidth(TextBox &ctx, TextStyle &style)  {
	return image ? image->width : 0;
}

// Draw the element.
void ImageElement::draw(Buffer &to, TextBox &ctx, TextStyle &style)  {
	if (image) {
		pax_draw_image(to.internal, image, 0, -image->height);
	}
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
	size_t endIndex   = 0;
	// Whether text styling is currently being applied.
	bool isText       = list[0].second->isText();
	
	// Current text style.
	TextStyle &style    = list[0].first;
	
	// Current size of space.
	Vec2f spaceSize   = pax_text_size(style.font, style.fontSize, " ");
	// Current line's width.
	float lineWidth   = 0, elementWidth = 0;
	// Current line's height.
	float lineAscent  = 0, lineDescent  = 0;
	// Current total height.
	float totalHeight = 0;
	
	while (startIndex < list.size()) {
		// Add a space into the minimum width calculation.
		float theSpace = (endIndex > startIndex) ? spaceSize.x : 0;
		
		// Test whether another element still fits.
		if (endIndex < list.size()) {
			InlineElement &elem = *list[endIndex].second;
			TextStyle &style = list[endIndex].first;
			if (elem.getWidth(*this, style) + lineWidth + theSpace <= bounds.w) {
				// If so, count it up and continue going.
				
				// Minimum possible width.
				lineWidth += theSpace + elem.getWidth(*this, style);
				// Element-only width (used for justification).
				elementWidth += elem.getWidth(*this, style);
				
				// Line ascent.
				float asc = elem.getAscent(*this, style);
				if (asc > lineAscent) lineAscent = asc;
				// Line descent.
				float desc = elem.getDescent(*this, style);
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
			InlineElement &elem = *pair.second;
			style = pair.first;
			elem.draw(to, *this, style);
			float width = elem.getWidth(*this, style);
			
			// Draw underline, strikethrough and overline.
			float decorHeight = 1;
			if (elem.isText()) {
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
			
			// Determine next style, if any, then draw the lines again.
			if (elem.isText() && i < endIndex - 1) {
				TextStyle &nextStyle = list[i+1].first;
				if (nextStyle.underline && style.underline) {
					// Draw underline.
					to.drawRect(style.color, width, 0, spacing, decorHeight);
				}
				if (nextStyle.overline && style.overline) {
					// Draw overline.
					to.drawRect(style.color, width, -elem.getAscent(*this, style), spacing, decorHeight);
				}
				if (nextStyle.strikethrough && style.strikethrough) {
					// Draw strikethrough.
					to.drawRect(style.color, width, -elem.getAscent(*this, style)/2, spacing, decorHeight);
				}
			}
			
			// Move to the right a bit.
			to.translate(width + spacing, 0);
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
