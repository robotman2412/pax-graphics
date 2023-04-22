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

#ifndef PAX_GUI_HPP
#define PAX_GUI_HPP

#include <pax_gfx.h>

#ifndef __cplusplus
#error "pax_gui is a C++ library, included from a C translation unit"
#endif // __cplusplus

#include <pax_gui_base.hpp>
#include <pax_gui_button.hpp>
#include <pax_gui_container.hpp>
#include <pax_gui_dropdown.hpp>
#include <pax_gui_image.hpp>
#include <pax_gui_label.hpp>
#include <pax_gui_progress.hpp>

#include <pax_gui_colpicker.hpp>
#include <pax_gui_filepicker.hpp>
#include <pax_gui_keyboard.hpp>

#endif // PAX_GUI_HPP
