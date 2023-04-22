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

#include "pax_gui_progress.hpp"
#include <math.h>

namespace pax::gui {

// Make a new progress bar with unknown progress.
ProgressBar::ProgressBar(Rectf _bounds, Style _style):
	Element(_bounds), style(_style), color(0xff007fff), unknownProgress(1), progress(0) {}

// Make a new progress bar with known progress.
ProgressBar::ProgressBar(Rectf _bounds, float _progress, Style _style):
	Element(_bounds), style(_style), color(0xff007fff), unknownProgress(0), progress(_progress) {}


// Draw this element to `buf`.
void ProgressBar::draw(Buffer &buf) {
	// Determine angles.
	float angle, width;
	if (unknownProgress) {
		// Raw angles.
		width = progress < 0.5 ? progress*2 : progress*2-2;
		angle = progress;
	
		// Bring angles within [0, 1].
		if (width < 0) {
			angle +=  width;
			width  = -width;
		}
		angle = fmodf(angle + 1, 1);
	} else {
		angle = 0;
		width = progress;
	}
	
	switch (style) {
		default:
		case Style::ROUNDED_RECT:
		case Style::RECT: {
			if (angle+width > 1) {
				buf.drawRect(
					color,
					bounds.x+bounds.w*angle, bounds.y,
					bounds.w*(1-angle), bounds.h
				);
				buf.drawRect(
					color,
					bounds.x, bounds.y,
					bounds.w*(width-1+angle), bounds.h
				);
			} else {
				buf.drawRect(
					color,
					bounds.x+bounds.w*angle, bounds.y,
					bounds.w*width, bounds.h
				);
			}
		} break;
		
		case Style::ROUNDED_RING:
		case Style::RING:
		case Style::PIE_CHART: {
			buf.drawArc(
				color,
				bounds.x+bounds.w/2, bounds.y+bounds.h/2,
				fminf(bounds.w, bounds.h)/2,
				M_PI/2-angle*2*M_PI, M_PI/2-(angle+width)*2*M_PI
			);
		} break;
	}
}

// Callback to run every so often.
// Returns true if the object has to be redrawn.
bool ProgressBar::tick(uint64_t millis, uint64_t deltaMillis) {
	if (unknownProgress) {
		progress = millis % 3000 / 3000.0f;
	}
	return unknownProgress;
}

} // namespace pax::gui
