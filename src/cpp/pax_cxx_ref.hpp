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

#include <memory>
#include <string>
#include <assert.h>

namespace pax {

// Reference helper for consolidating pointer types.
template<typename Type> class Ref {
	protected:
		// Internal value storage when obtained from non-pointer reference.
		std::shared_ptr<Type> shared;
		// Internal value storage when obtained from pointer.
		Type *ptr;
		// Does this box have a value?
		bool hasValue;
		
	public:
		// An empty box.
		Ref() {
			hasValue = false;
			ptr      = nullptr;
		}
		// Construction from direct pass of copy.
		Ref(Type byCopy) {
			hasValue = false;
			ptr      = nullptr;
			shared   = std::make_shared<Type>(byCopy);
		}
		// Construction from reference.
		Ref(Type &byRef) {
			hasValue = false;
			ptr      = &byRef;
		}
		// Construction from move.
		Ref(Type &&byMove) {
			hasValue = false;
			ptr      = nullptr;
			shared   = std::make_shared<Type>(byMove);
		}
		// Construction from raw pointer.
		Ref(Type *rawPtr) {
			hasValue = false;
			ptr      = rawPtr;
		}
		
		// Set value of referenced.
		void operator=(Type newValue) {
			*get() = newValue;
		}
		// Get pointer to referenced.
		Type *get() {
			assert(hasValue && "Get value performed on empty pax::Ref");
			if (ptr) {
				return ptr;
			} else {
				return shared.get();
			}
		}
		// Get pointer to referenced.
		Type *operator->() {
			return get();
		}
		// Get value of referenced.
		Type *operator&() {
			return get();
		}
		// Access by reference.
		Type &operator*() {
			assert(hasValue && "Get value performed on empty pax::Ref");
			if (ptr) {
				return *ptr;
			} else {
				return *shared.get();
			}
		}
};

} // namespace pax

#endif //__cplusplus
