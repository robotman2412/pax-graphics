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

#ifndef PAX_GUI_COLPICKER_HPP
#define PAX_GUI_COLPICKER_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>
#include <functional>

#include <pax_gui_base.hpp>
#include <pax_gui_container.hpp>

namespace pax::gui {

// A colored box made for use with ColorPicker.
class GradientBox: public Element {
	public:
		enum class Type {
			// Fades between two colors horizontally.
			HORIZONTAL_FADE,
			// Fades between two colors vertically.
			VERTICAL_FADE,
			// Hue (horizontal) / saturation (vertical) spectrum.
			HUE_SAT_SPECTRUM,
			// Alpha slider (with the grey and dark grey boxes).
			HORIZONTAL_ALPHA,
		};
		
	private:
		// Shader being used when drawing.
		Shader shader;
		
		// Type of gradient to show.
		Type type;
		
	public:
		// Color information.
		union {
			// Used to force initialisation to happen.
			uint64_t dummy;
			struct {
				// Left-hand color (type = HORIZONTAL_FADE).
				Color left;
				// Right-hand color (type = HORIZONTAL_FADE).
				Color right;
			};
			struct {
				// Top color (type = VERTICAL_FADE).
				Color top;
				// Bottom color (type = VERITCAL_FADE).
				Color bottom;
			};
			// Current brightness (type = HUE_SAT_SPECTRUM).
			uint8_t bri;
			// Base color without alpha (type = HORIZONTAL_ALPHA).
			Color baseColor;
		};
		// Exact value of current color.
		Color color;
		
		// Horizontal line position.
		int16_t hLine;
		// Vertical line position
		int16_t vLine;
		
		// Make a box with given bounds and type.
		GradientBox(Rectf bounds, Type type);
		
		// ???
		~GradientBox() final = default;
		
		// Set the type of gradient.
		void setType(Type type);
		// Get the type of gradient.
		Type getType() { return type; }
		
		// Draw this element to `buf`.
		// When selected by user interaction, `selected` is true.
		void draw(Buffer &buf) final;
};

// A simple button with some centered text on it.
class ColorPicker: public Container {
	private:
		union {
			// Current ARGB color value.
			Color rgb;
			struct {
				// Current blue value.
				uint8_t blue;
				// Current green value.
				uint8_t green;
				// Current red value.
				uint8_t red;
				// Current alpha value, if applicable.
				uint8_t alpha;
			};
		};
		// Current HSV, if applicable.
		uint8_t hue, sat, bri;
		// Is this an HSV-type color picker?
		bool isHSV;
		// Does the color currently have an HSV value?
		bool hasHSV;
		// Does this color picker have an alpha slider?
		bool hasAlpha;
		// Used to track changing of the values.
		uint64_t lastTime;
		
		// Gradient box for hue/sat.
		std::shared_ptr<GradientBox> hueBox;
		// Gradient box for brightness.
		std::shared_ptr<GradientBox> briBox;
		// Gradient box for alpha.
		std::shared_ptr<GradientBox> alphaBox;
		// Gradient box for red.
		std::shared_ptr<GradientBox> redBox;
		// Gradient box for green.
		std::shared_ptr<GradientBox> greenBox;
		// Gradient box for blue.
		std::shared_ptr<GradientBox> blueBox;
		
		// Currently held direction input.
		InputButton held;
		
		// Called every time the color is updated.
		void onChangeInt();
		
	public:
		// Type used for color changed callbacks.
		using Callback = std::function<void(ColorPicker&)>;
		
		// The function to call when the color is changed.
		Callback    onChange;
		// Decorative: Whether the button is currently pressed.
		bool        pressed;
		
		// Make a new color picker.
		ColorPicker(Rectf _bounds = {0, 0, 200, 200},
			bool _isHSV=true, bool _hasAlpha=false, Callback _onChange = {});
		
		// Set color from RGB (updates alpha).
		void setARGB(Color rgb);
		// Set color from HSV (updates alpha).
		void setAHSV(uint8_t alpha, uint8_t hue, uint8_t sat, uint8_t bri);
		// Set color from RGB (preserves alpha).
		void setRGB(Color rgb);
		// Set color from HSV (preserves alpha).
		void setHSV(uint8_t hue, uint8_t sat, uint8_t bri);
		// Set alpha.
		void setAlpha(uint8_t alpha);
		
		// Get HSV values.
		void getHSV(uint8_t &hue_out, uint8_t &sat_out, uint8_t &bri_out);
		// Get RGB value (alpha ignored).
		Color getRGB() { return rgb | 0xff000000; }
		// Get ARGB value.
		Color getColor() { return rgb; }
		// Get alpha.
		uint8_t getAlpha() { return alpha; }
		
		// Button pressed event.
		void buttonDown(InputButton which) final;
		// Button released event.
		void buttonUp(InputButton which) final;
		
		// Callback to run every so often.
		// Returns true if the object has to be redrawn.
		bool tick(uint64_t millis, uint64_t deltaMillis) final;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_COLPICKER_HPP
