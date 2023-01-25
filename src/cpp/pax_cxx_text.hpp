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

#pragma once

#include <pax_types.h>

#ifdef __cplusplus

namespace pax {

// An element that can be drawn in a `TextBox`.
// This serves merely as an interface for other classes to implement.
class InlineElement;
// Text, the most common type of `InlineElement` to draw.
// Supports fonts, color and certain effects like strikethrough.
class TextElement;
// Style to apply to text.
class TextStyle;
// A box intended to draw text and inline elements.
// Has word-wrap and justification support.
class TextBox;

// Method by which to align or justify text.
enum TextAlign {
	// Left-aligned text, the usual.
	LEFT,
	// Center-aligned text, usually for titles.
	CENTER,
	// Right-aligned text, sometimes used for page numbers.
	RIGHT,
	// Justified "fill space" text. Will fill all available horizontal space.
	JUSTIFY,
};

}

#include "pax_cxx.hpp"
#include <string>
#include <vector>
#include <memory>

namespace pax {

// An element that can be drawn in a `TextBox`.
// This serves merely as an interface for other classes to implement.
class InlineElement {
	public:
		enum Type {
			// Generic inline element.
			GENERIC,
			// Text element.
			TEXT,
			// Space character of some kind.
			SPACE,
			// Line break.
			NEWLINE,
		};
		
		// This class is quite virtual, and so must be the destructor.
		virtual ~InlineElement() = default;
		
		// Get ascent above baseline.
		virtual float getAscent(TextBox &ctx, TextStyle &style) { return 0; };
		// Get descent below baseline.
		virtual float getDescent(TextBox &ctx, TextStyle &style) { return 0; };
		// Compute and get dimensions.
		// This is called once after the start of drawing or when this element's style changes.
		virtual void calcSize(TextBox &ctx, TextStyle &style) {}
		// Get width after computation.
		virtual float getWidth(TextBox &ctx, TextStyle &style) { return 0; }
		// Get the type of element this is. Different types cause different treatment.
		virtual Type type() { return GENERIC; }
		// Draw the element.
		virtual void draw(Buffer &to, TextBox &ctx, TextStyle &style) {}
};

// Text, the most common type of `InlineElement` to draw.
class TextElement: public InlineElement {
	protected:
		// Text string to draw.
		// Everything is treated as a single word.
		std::string text;
		// Cached text width.
		float textWidth;
		
	public:
		// Wow very complicated.
		TextElement(std::string str);
		// Update text.
		void updateText(std::string str);
		
		// This class is quite virtual, and so must be the destructor.
		virtual ~TextElement() override = default;
		
		// Get ascent above baseline.
		virtual float getAscent(TextBox &ctx, TextStyle &style) override;
		// Get descent below baseline.
		virtual float getDescent(TextBox &ctx, TextStyle &style) override;
		// Compute and get dimensions.
		// This is called once after the start of drawing or when this element's style changes.
		virtual void calcSize(TextBox &ctx, TextStyle &style) override;
		// Get width after computation.
		virtual float getWidth(TextBox &ctx, TextStyle &style) override;
		// Get the type of element this is. Different types cause different treatment.
		virtual Type type() override { return TEXT; }
		// Draw the element.
		virtual void draw(Buffer &to, TextBox &ctx, TextStyle &style) override;
};

// A space character.
class SpaceElement: public InlineElement {
	protected:
		// Cached text width.
		float width;
		
	public:
		// This class is quite virtual, and so must be the destructor.
		virtual ~SpaceElement() override = default;
		// Compute and get dimensions.
		// This is called once after the start of drawing or when this element's style changes.
		virtual void calcSize(TextBox &ctx, TextStyle &style) override;
		// Get width after computation.
		virtual float getWidth(TextBox &ctx, TextStyle &style) override;
		// Get the type of element this is. Different types cause different treatment.
		virtual Type type() override { return SPACE; }
};

// A line break.
class NewlineElement: public InlineElement {
	public:
		// This class is quite virtual, and so must be the destructor.
		virtual ~NewlineElement() override = default;
		// Get the type of element this is. Different types cause different treatment.
		virtual Type type() override { return NEWLINE; }
};

// An inline image made of a pax::Buffer.
class ImageElement: public InlineElement {
	protected:
		// What teh hel is actually drawn.
		pax_buf_t *image = NULL;
		
	public:
		// Wow very complicated.
		ImageElement(pax_buf_t *image);
		// Wow very complicated.
		ImageElement(Buffer *image);
		
		// This class is quite virtual, and so must be the destructor.
		virtual ~ImageElement() override = default;
		
		// Get ascent above baseline.
		virtual float getAscent(TextBox &ctx, TextStyle &style) override;
		// Get descent below baseline.
		virtual float getDescent(TextBox &ctx, TextStyle &style) override;
		// Compute and get dimensions.
		// This is called once after the start of drawing or when this element's style changes.
		virtual void calcSize(TextBox &ctx, TextStyle &style) override;
		// Get width after computation.
		virtual float getWidth(TextBox &ctx, TextStyle &style) override;
		// Get the type of element this is. Different types cause different treatment.
		virtual Type type() override { return GENERIC; }
		// Draw the element.
		virtual void draw(Buffer &to, TextBox &ctx, TextStyle &style) override;
};

// Style to apply to text.
class TextStyle {
	public:
		// Font to use.
		const pax_font_t *font = NULL;
		// Font size.
		float fontSize = 0;
		// Text color.
		Color color = 0xffffffff;
		// Italicise text?
		bool italic = false;
		// Strikethrough text?
		bool strikethrough = false;
		// Underline text?
		bool underline = false;
		// Overline text?
		bool overline = false;
		
		TextStyle() = default;
		TextStyle(
			const pax_font_t *_font, float _fontSize, Color _color = 0xffffffff,
			bool _italic = false, bool _strikethrough = false,
			bool _underline = false, bool _overline = false
		): font(_font), fontSize(_fontSize), color(_color), italic(_italic), strikethrough(_strikethrough), underline(_underline), overline(_overline) {}
		
		bool operator==(const TextStyle &other) {
			if (this == &other) return true;
			return font          == other.font
				&& fontSize      == other.fontSize
				&& color         == other.color
				&& italic        == other.italic
				&& strikethrough == other.strikethrough
				&& underline     == other.underline
				&& overline      == other.overline;
		}
		bool operator!=(const TextStyle &other) { return !(*this == other); }
};

// A box intended to draw text and inline elements.
// Has word-wrap and justification support.
class TextBox {
	protected:
		// Current text style.
		TextStyle textStyle;
		// Pair of stail and text for im lizt.
		typedef std::pair<TextStyle, std::shared_ptr<InlineElement>> Entry;
		// List ov text alamand ant teirh styles.
		std::vector<Entry> list;
		
	public:
		// On-screen drawing boundaries.
		Rectf bounds;
		// Method by which to align text.
		TextAlign alignment;
		
		// Append a string of text to the grand list.
		// Words are broken at whitespace, to override use the non-breaking space character.
		void appendText(std::string text);
		// Append a text element of any type.
		// Keep in mind that `element` is treated as a single word.
		template <typename Type>
		void append(Type element) {
			// Pack it into an entry to preserve subclasses.
			std::shared_ptr<InlineElement> ptr = std::make_shared<Type>(element);
			Entry entry(textStyle, ptr);
			// Have dimensions computed on newly generated copy.
			entry.second->calcSize(*this, textStyle);
			// Add the the list of thingies.
			list.push_back(entry);
		}
		// Append a style change before the next element.
		void appendStyle(TextStyle newStyle);
		// Get a copy of the current text style.
		const TextStyle &getStyle() { return textStyle; }
		
		// Draw the `TextBox` and all it's contents.
		void draw(Buffer &to);
		
		// Test whether a character is treated as whitespace by a `TextBox`.
		static bool isWhitespace(uint32_t codepoint) { return codepoint <= 0x20; }
};

} // namespace pax

#endif //__cplusplus
