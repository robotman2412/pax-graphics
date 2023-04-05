# PAX Docs: `pax::TextBox`

The `pax::TextBox` class exists to solve a relatively common problem:
Advanced options for text including inline images and word-wrap,
which `pax::Buffer::drawString` does not provide.

*Note: The rendering code isn't as clean as I'd like and rendering behaviour is subject to change.*

A `TextBox` is designed to provide:
- Word-wrap
- Common text alignment options
- Inline elements such as images
- Per-character text styling

In this document:
- [Example usage](#example-usage)
- [`pax::TextBox`](#paxtextbox)
	- [`pax::TextStyle`](#paxtextstyle)
	- [`pax::TextAlign`](#paxtextalign)
- [`pax::InlineElement`](#paxinlineelement)
  - [`pax::TextElement`](#paxtextelement)
  - [`pax::SpaceElement`](#paxspaceelement)
  - [`pax::NewlineElement`](#paxnewlineelement)
  - [`pax::InlineImage`](#paxinlineimage)



# Example usage
The following example draws some normal text followed by red italic text.
Note the leading space in the second `appendText` call; without it there would be no space between "18pt." and "Red".
```cpp
void myTextFunction(pax::Buffer &myBuffer) {
	// Make a simple box with Saira Regular in 18pt.
	pax::TextBox box({0, 0, 150, 100}, pax_font_saira_regular, 18);
	
	// Add a normal bit of text.
	box.appendText("Saira regular 18pt.");
	
	// Add a red bit of text.
	auto style   = box.getStyle();
	style.italic = true;
	style.color  = 0xffff0000;
	box.appendStyle(style);
	
	box.appendText(" Red italics.");
	
	// Draw the thing.
	gfx.background(0xff000000);
	box.draw(myBuffer);
}
```



# `pax::TextBox`
## `pax::TextStyle`
## `pax::TextAlign`



# `pax::InlineElement`
## `pax::TextElement`
## `pax::SpaceElement`
## `pax::NewlineElement`
## `pax::InlineImage`
