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

#ifndef PAX_GFX_C
#ifndef ARDUINO
#pragma message "This file should not be compiled on it's own."
#endif
#else

#include "pax_internal.h"


/* ===== MULTI-CORE RENDERING ==== */

// If multi-core rendering is enabled, wait for the other core.
void pax_join() {
	// No MCR, no wait.
}

// Enable multi-core rendering.
void pax_enable_multicore(int core) {
	PAX_LOGE(TAG, "Failed to enable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
}

// Disable multi-core rendering.
void pax_disable_multicore() {
	PAX_LOGE(TAG, "No need to disable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
}

#endif
