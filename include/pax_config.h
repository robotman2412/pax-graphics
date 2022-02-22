/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

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

#ifndef PAX_CONFIG_H
#define PAX_CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Log errors instead of just setting pax_last_error.
#define PAX_AUTOREPORT

// Compile in bezier curves.
#define PAX_COMPILE_BEZIER

// Use the more expensive, but more accurate algorithm in pax_draw_bezier.
// #define PAX_USE_EXPENSIVE_BEZIER

// Compile in triangulation (filling the outline of a shape).
#define PAX_COMPILE_TRIANGULATE

// Compile in multi-core rendering.
#define PAX_COMPILE_MCR

// Queue size to use for multi-core rendering.
#define PAX_QUEUE_SIZE 30

#endif //PAX_CONFIG_H
