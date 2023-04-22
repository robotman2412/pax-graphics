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

#ifndef PAX_GUI_PROGRESS_HPP
#define PAX_GUI_PROGRESS_HPP

#include <pax_gfx.h>

#ifdef __cplusplus

#include <memory>
#include <vector>
#include <functional>

#include <pax_gui_base.hpp>

namespace pax::gui {

// A simple progress bar.
class ProgressBar: public Element {
	public:
		// Style of progress bar.
		enum class Style {
			// Rounded horizontal.
			ROUNDED_RECT,
			// Rectangular horizontal.
			RECT,
			// Rounded ring.
			ROUNDED_RING,
			// Simple ring.
			RING,
			// Pie chart.
			PIE_CHART,
		};
		
		// Shape of this progress bar.
		Style style;
		// Color of this progress bar.
		Color color;
		// Radius of rounding or thickness of ring.
		float radius;
		// Display as "unknown progress" type.
		// Uses `progress` to animate the shape.
		bool unknownProgress;
		// Progress/animation coefficient (0-1).
		float progress;
		
		// Make a new progress bar with unknown progress.
		ProgressBar(Rectf _bounds = {0, 0, 50, 50}, Style _style = Style::ROUNDED_RING);
		// Make a new progress bar with known progress.
		ProgressBar(Rectf _bounds, float _progress, Style _style = Style::ROUNDED_RECT);
		// This is required to allow subclasses with virtuals.
		virtual ~ProgressBar() = default;
		
		// Draw this element to `buf`.
		virtual void draw(Buffer &buf) override;
		// Callback to run every so often.
		// Returns true if the object has to be redrawn.
		virtual bool tick(uint64_t millis, uint64_t deltaMillis) override;
};

} // namespace pax::gui

#endif // __cplusplus

#endif // PAX_GUI_PROGRESS_HPP
