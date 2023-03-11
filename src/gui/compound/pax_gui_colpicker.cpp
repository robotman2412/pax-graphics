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

#include "pax_gui_colpicker.hpp"

namespace pax::gui {

// Make a box with given bounds and type.
GradientBox::GradientBox(Rectf bounds, Type type):
	Element(bounds.round()), dummy(0) {
	
	setType(type);
	hLine = -100;
	vLine = -100;
}

Color shaderFunc(Color existing, Color tint, int x, int y, float u, float v, void *args) {
	return rgb(u*255, v*255, 0);
}

// Set the type of gradient.
void GradientBox::setType(Type type) {
	this->type = type;
	
	// Update shader edition.
	switch (type) {
		case Type::HORIZONTAL_FADE:
			shader = Shader([](Color existing, Color tint, int x, int y, float u, float v, void *args) -> Color {
				auto ptr = (GradientBox *) args;
				// Left-to-right color fade.
				return lerp(u*255, ptr->left, ptr->right);
			}, nullptr);
			break;
			
		case Type::VERTICAL_FADE:
			shader = Shader([](Color existing, Color tint, int x, int y, float u, float v, void *args) -> Color {
				auto ptr = (GradientBox *) args;
				// Top-to-bottom color fade.
				return lerp(v*255, ptr->top, ptr->bottom);
			}, nullptr);
			break;
			
		case Type::HUE_SAT_SPECTRUM:
			shader = Shader([](Color existing, Color tint, int x, int y, float u, float v, void *args) -> Color {
				auto ptr = (GradientBox *) args;
				// Hue-saturation spectrum.
				return hsv(u*255, v*255, ptr->bri);
			}, nullptr);
			break;
			
		case Type::HORIZONTAL_ALPHA:
			shader = Shader([](Color existing, Color tint, int x, int y, float u, float v, void *args) -> Color {
				auto ptr = (GradientBox *) args;
				// Left-to-right alpha device.
				bool  which   = (x & 2) ^ (y & 2);
				Color checker = which ? 0xff7f7f7f : 0xffafafaf;
				return lerp(u*255, checker, ptr->baseColor);
			}, nullptr);
			break;
	}
	
	// Set basic promises.
	shader.getInternal()->alpha_promise_0   = false;
	shader.getInternal()->alpha_promise_255 = true;
	shader.getInternal()->promise_callback  = (void *) (pax_promise_func_t)
		+[](pax_buf_t *buf, pax_col_t tint, void *args) -> uint64_t {
		return PAX_PROMISE_IGNORE_BASE + PAX_PROMISE_OPAQUE;
	};
}


// Draw this element to `buf`.
void GradientBox::draw(Buffer &buf) {
	
	// Delegate the rectangle to shaders.
	shader.setContext((void *) this);
	buf.drawRect(&shader, nullptr, bounds.x, bounds.y, bounds.w, bounds.h);
	
	// Draw outline.
	Color col;
	switch (focus) {
		default:
			col = 0x00000000;
			break;
			
		case FocusState::HIGHLIGHTED:
		case FocusState::FOCUSSED:
			col = getTheme()->outlineColor;
			break;
			
		case FocusState::CAPTURED:
			col = getTheme()->highlightColor;
	}
	buf.outlineRect(col, bounds.x-1, bounds.y-1, bounds.w+1, bounds.h+1);
	
	// Draw lines indicating color selection.
	if (hLine >= 0 && vLine >= 0 && type == Type::HUE_SAT_SPECTRUM) {
		// Get a contrasting color.
		uint16_t grey = (uint8_t) color
				+ (uint8_t) (color >> 8)
				+ (uint8_t) (color >> 16);
		Color contrasting = grey > 3*127 ? 0xff000000 : 0xffffffff;
		
		// Draw a little outline.
		buf.outlineRect(contrasting, bounds.x+hLine-5, bounds.y+vLine-5, 10, 10);
		
	} else if (vLine >= 0 && type == Type::VERTICAL_FADE) {
		// Get a contrasting color.
		uint16_t grey = (uint8_t) color
				+ (uint8_t) (color >> 8)
				+ (uint8_t) (color >> 16);
		Color contrasting = grey > 3*127 ? 0xff000000 : 0xffffffff;
		
		// Draw a vertical line.
		buf.drawLine(contrasting, bounds.x, bounds.y+vLine, bounds.x+bounds.w-1, bounds.y+vLine);
		
	} else if (hLine >= 0) {
		// Get a contrasting color.
		uint16_t grey = (uint8_t) color
				+ (uint8_t) (color >> 8)
				+ (uint8_t) (color >> 16);
		Color contrasting = grey > 3*127 ? 0xff000000 : 0xffffffff;
		
		// Draw a horizontal line.
		buf.drawLine(contrasting, bounds.x+hLine, bounds.y, bounds.x+hLine, bounds.y+bounds.h-1);
	}
}



// Called every time the color is updated.
void ColorPicker::onChangeInt() {
	if (isHSV) {
		hueBox->bri   = bri;
		hueBox->hLine = hueBox->bounds.w * hue / 255;
		hueBox->vLine = hueBox->bounds.h * sat / 255;
		hueBox->color = rgb;
		
		briBox->top   = pax_col_hsv(hue, sat, 255);
		briBox->vLine = briBox->bounds.h * (255-bri) / 255;
		briBox->color = rgb;
		
	} else {
		redBox->left    = (rgb & 0x0000ffff) | 0xff000000;
		redBox->right   = rgb | 0xffff0000;
		redBox->hLine   = redBox->bounds.w * red / 255;
		redBox->color   = rgb;
		
		greenBox->left  = (rgb & 0x00ff00ff) | 0xff000000;
		greenBox->right = rgb | 0xff00ff00;
		greenBox->hLine = greenBox->bounds.w * green / 255;
		greenBox->color = rgb;
		
		blueBox->left   = (rgb & 0x00ffff00) | 0xff000000;
		blueBox->right  = rgb | 0xff0000ff;
		blueBox->hLine  = blueBox->bounds.w * blue / 255;
		blueBox->color  = rgb;
	}
	
	if (hasAlpha) {
		alphaBox->baseColor = rgb | 0xff000000;
		alphaBox->hLine     = alphaBox->bounds.w * alpha / 255;
		alphaBox->color     = rgb;
	}
}


// Make a new color picker.
ColorPicker::ColorPicker(Rectf _bounds, bool _isHSV, bool _hasAlpha, Callback _onChange):
	Container(_bounds.round()), isHSV(_isHSV), hasAlpha(_hasAlpha), onChange(_onChange) {
	background = 0x00000000;
	
	// Set default color (opaque white).
	alpha = 255;
	rgb = 0xffffffff;
	hue = 0;
	sat = 0;
	bri = 255;
	
	// Create child elements.
	if (isHSV) {
		float boxSizeX = hasAlpha ? bounds.h - 30 : bounds.h;
		float boxSizeY = bounds.w - 30;
		float boxSize  = boxSizeX < boxSizeY ? boxSizeX : boxSizeY;
		
		// Hue/saturation box.
		hueBox = appendChildT(GradientBox(
			{0, 0, boxSize, boxSize},
			GradientBox::Type::HUE_SAT_SPECTRUM
		));
		
		// Brightness box.
		briBox = appendChildT(GradientBox(
			{boxSize + 10, 0, 20, boxSize},
			GradientBox::Type::VERTICAL_FADE
		));
		briBox->bottom = 0;
		
		// Alpha box.
		if (hasAlpha) alphaBox = appendChildT(GradientBox(
			{0, boxSize + 10, boxSize, 20},
			GradientBox::Type::HORIZONTAL_ALPHA
		));
		
	} else {
		float boxHeight = hasAlpha
			? (bounds.h - 30) / 4
			: (bounds.h - 20) / 3;
		
		// Red box.
		redBox = appendChildT(GradientBox(
			{0, 0, bounds.w, boxHeight},
			GradientBox::Type::HORIZONTAL_FADE
		));
		
		// Green box.
		greenBox = appendChildT(GradientBox(
			{0, 10 + boxHeight, bounds.w, boxHeight},
			GradientBox::Type::HORIZONTAL_FADE
		));
		
		// Blue box.
		blueBox = appendChildT(GradientBox(
			{0, 20 + 2*boxHeight, bounds.w, boxHeight},
			GradientBox::Type::HORIZONTAL_FADE
		));
		
		// Alpha box.
		if (hasAlpha) alphaBox = appendChildT(GradientBox(
			{0, 30 + 3*boxHeight, bounds.w, boxHeight},
			GradientBox::Type::HORIZONTAL_ALPHA
		));
	}
	
	// Update all the boxes and sliders.
	onChangeInt();
}


// Set color from RGB (updates alpha).
void ColorPicker::setARGB(Color _rgb) {
	if (!hasAlpha) rgb = _rgb | 0xff000000;
	else rgb = _rgb;
	undo_hsv(rgb, hue, sat, bri);
	onChangeInt();
}

// Set color from HSV (updates alpha).
void ColorPicker::setAHSV(uint8_t _alpha, uint8_t _hue, uint8_t _sat, uint8_t _bri) {
	if (hasAlpha) alpha = _alpha;
	hue = _hue;
	sat = _sat;
	bri = _bri;
	rgb = pax_col_ahsv(alpha, hue, sat, bri);
	onChangeInt();
}

// Set color from RGB (preserves alpha).
void ColorPicker::setRGB(Color _rgb) {
	rgb = (_rgb & 0x00ffffff) | (alpha << 24);
	undo_hsv(rgb, hue, sat, bri);
	onChangeInt();
}

// Set color from HSV (preserves alpha).
void ColorPicker::setHSV(uint8_t _hue, uint8_t _sat, uint8_t _bri) {
	hue = _hue;
	sat = _sat;
	bri = _bri;
	rgb = pax_col_ahsv(alpha, hue, sat, bri);
	onChangeInt();
}

// Set alpha.
void ColorPicker::setAlpha(uint8_t _alpha) {
	if (hasAlpha) {
		alpha = _alpha;
		onChangeInt();
	}
}

// Get HSV values.
void ColorPicker::getHSV(uint8_t &hue_out, uint8_t &sat_out, uint8_t &bri_out) {
	hue_out = hue;
	sat_out = sat;
	bri_out = bri;
	onChangeInt();
}

		
// Button pressed event.
void ColorPicker::buttonDown(InputButton which) {
	lastTime = 0;
	held = which;
	auto selected = selectedElement();
	if (!selected) {
		// Initial selection logic.
		switch (which) {
			default: break;
			case InputButton::UP:
			case InputButton::DOWN:
			case InputButton::LEFT:
			case InputButton::RIGHT:
			case InputButton::ACCEPT:
				focus = FocusState::CAPTURED;
				selectChild(isHSV ? hueBox : redBox);
				break;
		}
		
	} else if (selected && selected->focus >= FocusState::CAPTURED) {
		// Un-select box logic.
		if (which == InputButton::ACCEPT || which == InputButton::BACK) {
			selected->focus = FocusState::FOCUSSED;
			focus = FocusState::CAPTURED;
		}
		
	} else if (selected && which == InputButton::ACCEPT) {
		// Select box logic.
		selected->focus = FocusState::CAPTURED;
		focus = FocusState::DELEGATED;
		
	} else if (which == InputButton::BACK) {
		// Release control logic.
		focus = FocusState::FOCUSSED;
		unselect();
		
	} else if (which == InputButton::UP) {
		// Navigation logic 1/4.
		if (selected == alphaBox) {
			selectChild(isHSV ? hueBox : blueBox);
		} else if (selected == blueBox) {
			selectChild(greenBox);
		} else if (selected == greenBox) {
			selectChild(redBox);
		}
	} else if (which == InputButton::DOWN) {
		// Navigation logic 2/4.
		if (isHSV && hasAlpha) {
			selectChild(alphaBox);
		} else if (selected == redBox) {
			selectChild(greenBox);
		} else if (selected == greenBox) {
			selectChild(blueBox);
		} else if (selected == blueBox && hasAlpha) {
			selectChild(alphaBox);
		}
	} else if (which == InputButton::LEFT) {
		// Navigation logic 3/4.
		if (selected == briBox) {
			selectChild(hueBox);
		}
	} else if (which == InputButton::RIGHT) {
		// Navigation logic 4/4.
		if (isHSV) {
			selectChild(briBox);
		}
	}
}

// Button released event.
void ColorPicker::buttonUp(InputButton which) {
	held = InputButton::UNKNOWN;
}


// Callback to run every so often.
// Returns true if the object has to be redrawn.
bool ColorPicker::tick(uint64_t millis, uint64_t deltaMillis) {
	auto selected  = selectedElement();
	if (!selected || selected->focus < FocusState::CAPTURED) return false;
	
	// Compute amount of change.
	if (lastTime == 0) {
		lastTime = millis;
		return false;
	}
	uint8_t delta = 0;
	while (lastTime < millis - 20) {
		lastTime += 20;
		delta ++;
	}
	if (!delta) return false;
	
	if (selected == alphaBox) {
		// Directions check.
		if (held == InputButton::LEFT) {
			// Decrease alpha.
			if (alpha < delta) alpha = 0;
			else alpha -= delta;
		} else if (held == InputButton::RIGHT) {
			// Increase alpha.
			if (255-alpha < delta) alpha = 255;
			else alpha += delta;
		} else {
			return false;
		}
		onChangeInt();
		return true;
	}
	
	if (isHSV) {
		// HSV changing.
		if (selected == hueBox && hueBox->focus >= FocusState::CAPTURED) {
			// Directions check.
			switch (held) {
				case InputButton::UP:
					// Decrease saturation.
					if (sat < delta) sat = 0;
					else sat -= delta;
					break;
					
				case InputButton::DOWN:
					// Increase saturation.
					if (255-sat < delta) sat = 255;
					else sat += delta;
					break;
					
				case InputButton::LEFT:
					// Decrease hue.
					if (hue < delta) hue = 0;
					else hue -= delta;
					break;
					
				case InputButton::RIGHT:
					// Increase hue.
					if (255-hue < delta) hue = 255;
					else hue += delta;
					break;
					
				default: return false;
			}
			rgb = ahsv(alpha, hue, sat, bri);
			onChangeInt();
			return true;
			
		} else if (selected == briBox && briBox->focus >= FocusState::CAPTURED) {
			// Directions check.
			if (held == InputButton::DOWN) {
				// Decrease brightness.
				if (bri < delta) bri = 0;
				else bri -= delta;
				rgb = ahsv(alpha, hue, sat, bri);
				onChangeInt();
				return true;
				
			} else if (held == InputButton::UP) {
				// Increase brightness.
				if (255-bri < delta) bri = 255;
				else bri += delta;
				rgb = ahsv(alpha, hue, sat, bri);
				onChangeInt();
				return true;
			}
			return false;
		}
	} else {
		// RGB changing.
		uint8_t *val;
		if (selected == redBox) {
			val = &red;
		} else if (selected == greenBox) {
			val = &green;
		} else if (selected == blueBox) {
			val = &blue;
		} else {
			return false;
		}
		
		// Directions check.
		if (held == InputButton::LEFT) {
			// Decrease value.
			if (*val < delta) *val = 0;
			else *val -= delta;
		} else if (held == InputButton::RIGHT) {
			// Increase value.
			if (255-*val < delta) *val = 255;
			else *val += delta;
		} else {
			return false;
		}
		onChangeInt();
		return true;
	}
	
	return false;
}

} // namespace pax::gui
