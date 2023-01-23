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

#include <math.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#define PAX_CXX_Vecf_union union
#else
#define PAX_CXX_Vecf_union struct
#endif

typedef PAX_CXX_Vecf_union  pax_vec1  pax_vec1_t;
typedef PAX_CXX_Vecf_union  pax_vec2  pax_vec2_t;
typedef PAX_CXX_Vecf_union  pax_vec3  pax_vec3_t;
typedef PAX_CXX_Vecf_union  pax_vec4  pax_vec4_t;
typedef PAX_CXX_Vecf_union  pax_vec2  pax_line_t;
typedef PAX_CXX_Vecf_union  pax_vec3  pax_tri_t;
typedef PAX_CXX_Vecf_union  pax_vec4  pax_quad_t;
typedef PAX_CXX_Vecf_union  pax_rect  pax_rect_t;
typedef union  matrix_2d matrix_2d_t;
typedef struct matrix_stack_2d matrix_stack_2d_t;

#ifdef __cplusplus
#include <pax_cxx_ref.hpp>
#include <assert.h>

namespace pax {

typedef union  pax_vec1 Vec2f;
typedef union  pax_vec2 BiVec2f;
typedef union  pax_vec3 TriVec2f;
typedef union  pax_vec4 QuadVec2f;
typedef union  pax_vec1 Pointf;
typedef union  pax_vec2 Linef;
typedef union  pax_vec3 Trif;
typedef union  pax_vec4 Quadf;
typedef union  pax_rect Rectf;
typedef union  matrix_2d Matrix2f;
typedef struct matrix_stack_2d Matrix2fStack;

#define PAX_CXX_Vec2f_INDEX() \
	pax::Vec2f &operator[](ssize_t index) { \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		if (index < 0 || index >= _size) { \
			fprintf(stderr, "Error: Index out of bounds: %zd (not in range 0-%zu)\n", index, _size); \
			abort(); \
		} \
		return ((pax::Vec2f*) arr)[index]; \
	}
#define PAX_CXX_Vecf_AVERAGE() \
	pax::Vec2f average() { \
		pax::Vec2f avg(0, 0); \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			avg += arr[i]; \
		} \
		return avg / _size; \
	}

#define PAX_CXX_Vecf_OPERATOR(_type, _oper) \
	_type operator _oper(pax::Ref<_type> &rhs) { \
		_type out; \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			out.arr[i] = arr[i] _oper rhs->arr[i]; \
		} \
		return out; \
	} \
	_type operator _oper(const _type &rhs) { \
		_type out; \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			out.arr[i] = arr[i] _oper rhs.arr[i]; \
		} \
		return out; \
	} \
	_type operator _oper(float rhs) { \
		_type out; \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			out.arr[i] = arr[i] _oper rhs; \
		} \
		return out; \
	}

#define PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, _oper) \
	_type &operator _oper(pax::Ref<_type> &rhs) { \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			arr[i] _oper rhs->arr[i]; \
		} \
		return *this; \
	} \
	_type &operator _oper(const _type &rhs) { \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			arr[i] _oper rhs.arr[i]; \
		} \
		return *this; \
	} \
	_type &operator _oper(float rhs) { \
		const size_t _size = sizeof(arr) / 2 / sizeof(float); \
		for (size_t i = 0; i < _size; i++) { \
			arr[i] _oper rhs; \
		} \
		return *this; \
	}

#define PAX_CXX_Vecf_OPERATORS(_type) \
	PAX_CXX_Vecf_OPERATOR(_type, +) \
	PAX_CXX_Vecf_OPERATOR(_type, -) \
	PAX_CXX_Vecf_OPERATOR(_type, *) \
	PAX_CXX_Vecf_OPERATOR(_type, /) \
	PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, +=) \
	PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, -=) \
	PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, *=) \
	PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, /=) \
	PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, =)

} //namespace pax
#endif //__cplusplus

PAX_CXX_Vecf_union  pax_vec1 {
#ifdef __cplusplus
	struct {
#endif
		// Single point.
		float x, y;
#ifdef __cplusplus
	};
	float arr[2];
#endif
	
#ifdef __cplusplus
	// Initialise to zero.
	pax_vec1() {x=y=0;}
	// Initialise with value.
	pax_vec1(float _x, float _y) {x=_x; y=_y;}
	// Initialise from initialiser list.
	pax_vec1(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise as copy.
	pax_vec1(const pax::Vec2f &) = default;
	
	PAX_CXX_Vecf_OPERATORS(pax::Vec2f)
	
	// Unifies a this vector (it's magnitude will be 1).
	// Does not work for vectors with all zero.
	pax::Vec2f &unify();
	// Calculate magnitude of vector.
	float magnitude();
	// Calculate magnitude squared of vector.
	float squareMagnitude();
#endif //__cplusplus
};

PAX_CXX_Vecf_union  pax_vec2 {
#ifdef __cplusplus
	struct {
#endif
		// Line points.
		float x0, y0, x1, y1;
#ifdef __cplusplus
	};
	float arr[4];
#endif
	
#ifdef __cplusplus
	// Initialise to zero.
	pax_vec2() {x0=y0=x1=y1=0;}
	// Initialise with value.
	pax_vec2(float _x0, float _y0, float _x1, float _y1) {x0=_x0; y0=_y0; x1=_x1; y1=_y1;}
	// Initialise from initialiser list.
	pax_vec2(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise as copy.
	pax_vec2(const pax_vec2 &) = default;
	
	PAX_CXX_Vec2f_INDEX()
	PAX_CXX_Vecf_AVERAGE()
	PAX_CXX_Vecf_OPERATORS(pax::BiVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union  pax_vec3 {
#ifdef __cplusplus
	struct {
#endif
		// Triangle points.
		float x0, y0, x1, y1, x2, y2;
#ifdef __cplusplus
	};
	float arr[6];
#endif
	
#ifdef __cplusplus
	// Initialise to zero.
	pax_vec3() {x0=y0=x1=y1=x2=y2=0;}
	// Initialise with value.
	pax_vec3(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2) {x0=_x0; y0=_y0; x1=_x1; y1=_y1; x2=_x2; y2=_y2;}
	// Initialise from initialiser list.
	pax_vec3(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise as copy.
	pax_vec3(const pax_vec3 &) = default;
	
	// Operator []
	PAX_CXX_Vec2f_INDEX()
	// Operators + - * / += -= *= /= =
	PAX_CXX_Vecf_OPERATORS(pax::TriVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union  pax_vec4 {
#ifdef __cplusplus
	struct {
#endif
		// Quad points.
		float x0, y0, x1, y1, x2, y2, x3, y3;
#ifdef __cplusplus
	};
	float arr[8];
#endif
	
#ifdef __cplusplus
	// Initialise to zero.
	pax_vec4() {x0=y0=x1=y1=x2=y2=x3=y3=0;}
	// Initialise with value.
	pax_vec4(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2, float _x3, float _y3) {x0=_x0; y0=_y0; x1=_x1; y1=_y1; x2=_x2; y2=_y2; x3=_x3; y3=_y3;}
	// Initialise from initialiser list.
	pax_vec4(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise as copy.
	pax_vec4(const pax_vec4 &) = default;
	
	// Operator []
	PAX_CXX_Vec2f_INDEX()
	// Operators + - * / += -= *= /= =
	PAX_CXX_Vecf_OPERATORS(pax::QuadVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union  pax_rect {
#ifdef __cplusplus
	struct {
#endif
		// Rectangle points.
		float x, y, w, h;
#ifdef __cplusplus
	};
	float arr[4];
#endif
	
#ifdef __cplusplus
	// Initialise to zero.
	pax_rect() {x=y=w=h=0;}
	// Initialise with value.
	pax_rect(float _x, float _y, float _w, float _h) {x=_x; y=_y; w=_w; h=_h;}
	// Initialise from initialiser list.
	pax_rect(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise from initialiser list.
	pax_rect(std::initializer_list<pax::Vec2f> list) { assert(list.size()==2); position()=list.begin()[0]; size()=list.begin()[1]; }
	// Initialise as copy.
	pax_rect(const pax_rect &) = default;
	
	// Operator []
	PAX_CXX_Vec2f_INDEX()
	
	// Get average position, i.e. center, of the rectangle.
	pax::Vec2f average() {
		return pax::Vec2f(x+w/2.0f, y+h/2.0f);
	}
	// Get X/Y component.
	pax::Vec2f &position() {
		return *(pax::Vec2f*) &x;
	}
	// Get width/height component.
	pax::Vec2f &size() {
		return *(pax::Vec2f*) &w;
	}
	// Create an equivalent quad.
	pax::Quadf toQuad() {
		return pax::Quadf(x, y,  x+w, y,  x+w, y+h,  x, y+h);
	}
	// Get a copy which gaurantees nonnegative dimensions.
	pax::Rectf fixSize() {
		pax::Rectf out = *this;
		if (out.w < 0) { out.x += out.w; out.w = -out.w; }
		if (out.h < 0) { out.y += out.h; out.h = -out.h; }
		return out;
	}
#endif //__cplusplus
};

// Simplified representation of a 2D matrix.
// Excludes the bottom row, which is implicit.
// The matrix looks like this:
//   a0, a1, a2,
//   b0, b1, b2,
//   0,  0,  1
union matrix_2d {
	// Named members of the matrix.
	struct {
		float a0, a1, a2;
		float b0, b1, b2;
	};
	// Array members of the matrix.
	float arr[6];
	
#ifdef __cplusplus
	// Initialise to identity.
	matrix_2d() {a0=1; a1=a2=b0=0; b1=1; b2=0;}
	// Initialise with value.
	matrix_2d(float _a0, float _a1, float _a2, float _b0, float _b1, float _b2) {a0=_a0; a1=_a1; a2=_a2; b0=_b0; b1=_b1; b2=_b2;}
	// Initialise from initialiser list.
	matrix_2d(std::initializer_list<float> list) { assert(list.size()==sizeof(arr)/sizeof(float)); std::copy(list.begin(), list.end(), arr); }
	// Initialise as copy.
	matrix_2d(const matrix_2d &) = default;

	// 2D identity matrix: represents no transformation.
	static matrix_2d identity();
	// 2D scale matrix: represents a 2D scaling.
	static matrix_2d scale(float x, float y);
	// 2D translation matrix: represents a 2D movement of the camera.
	static matrix_2d translate(float x, float y);
	// 2D shear matrix: represents a 2D shearing.
	static matrix_2d shear(float x, float y);
	// 2D rotation matrix: represents a 2D rotation.
	static matrix_2d rotate(float angle);
	
	// Matrix multiplication. Note that A*B != B*A in matrices.
	matrix_2d operator*(const matrix_2d &rhs);
	// Matrix multiplication. Note that A*B != B*A in matrices.
	matrix_2d operator*(pax::Ref<matrix_2d> &rhs);
#endif //__cplusplus
};



#ifdef __cplusplus
extern "C" {
#endif

// Check whether the matrix exactly equals the identity matrix.
static inline bool matrix_2d_is_identity(matrix_2d_t m) {
	return m.a0 == 1 && m.a1 == 0 && m.a2 == 0
		&& m.b0 == 0 && m.b1 == 1 && m.b2 == 0;
}
// Check whether the matrix represents no more than a translation.
static inline bool matrix_2d_is_identity1(matrix_2d_t m) {
	return m.a0 == 1 && m.a1 == 0 && m.b0 == 0 && m.b1 == 1;
}
// Check whether the matrix represents no more than a translation and/or scale.
static inline bool matrix_2d_is_identity2(matrix_2d_t m) {
	return m.a1 == 0 && m.b0 == 0;
}

// 2D identity matrix: represents no transformation.
static inline matrix_2d_t matrix_2d_identity() {
#ifdef __cplusplus
	return pax::Matrix2f(1, 0, 0,  0, 1, 0);
#else
	return (matrix_2d_t) {.arr = {1, 0, 0,  0, 1, 0}};
#endif
}
// 2D scale matrix: represents a 2D scaling.
static inline matrix_2d_t matrix_2d_scale(float x, float y) {
#ifdef __cplusplus
	return pax::Matrix2f(x, 0, 0,  0, y, 0);
#else
	return (matrix_2d_t) {.arr = {x, 0, 0,  0, y, 0}};
#endif
}
// 2D translation matrix: represents a 2D movement of the camera.
static inline matrix_2d_t matrix_2d_translate(float x, float y) {
#ifdef __cplusplus
	return pax::Matrix2f(1, 0, x,  0, 1, y);
#else
	return (matrix_2d_t) {.arr = {1, 0, x,  0, 1, y}};
#endif
}
// 2D shear matrix: represents a 2D shearing.
static inline matrix_2d_t matrix_2d_shear(float x, float y) {
#ifdef __cplusplus
	return pax::Matrix2f(1, y, 0,  x, 1, 0);
#else
	return (matrix_2d_t) {.arr = {1, y, 0,  x, 1, 0}};
#endif
}
// 2D rotation matrix: represents a 2D rotation.
matrix_2d_t matrix_2d_rotate      (float angle);

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply    (matrix_2d_t a, matrix_2d_t b);
// 2D matrix: applies the transformation that a represents on to a point.
void        matrix_2d_transform   (matrix_2d_t a, float *x, float *y);

// 2D vector: unifies a given vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
pax_vec1_t  vec1_unify            (pax_vec1_t vec);

#ifdef __cplusplus
} // extern "C"
#endif



#ifdef __cplusplus

// Unifies a this vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
inline pax::Vec2f &pax::Vec2f::unify() {
	float mag = magnitude();
	x /= mag; y /= mag;
	return *this;
}

// Calculate magnitude of vector.
inline float pax::Vec2f::magnitude() {
	return sqrtf(x*x+y*y);
}

// Calculate magnitude squared of vector.
inline float pax::Vec2f::squareMagnitude() {
	return x*x+y*y;
}

// 2D identity matrix: represents no transformation.
inline pax::Matrix2f pax::Matrix2f::identity() {
	return matrix_2d_identity();
}
// 2D scale matrix: represents a 2D scaling.
inline pax::Matrix2f pax::Matrix2f::scale(float x, float y) {
	return matrix_2d_scale(x, y);
}
// 2D translation matrix: represents a 2D movement of the camera.
inline pax::Matrix2f pax::Matrix2f::translate(float x, float y) {
	return matrix_2d_translate(x, y);
}
// 2D shear matrix: represents a 2D shearing.
inline pax::Matrix2f pax::Matrix2f::shear(float x, float y) {
	return matrix_2d_shear(x, y);
}
// 2D rotation matrix: represents a 2D rotation.
inline pax::Matrix2f pax::Matrix2f::rotate(float angle) {
	return matrix_2d_rotate(angle);
}

// Matrix multiplication. Note that A*B != B*A in matrices.
inline pax::Matrix2f pax::Matrix2f::operator*(const pax::Matrix2f &rhs) {
	return matrix_2d_multiply(*this, rhs);
}
// Matrix multiplication. Note that A*B != B*A in matrices.
inline pax::Matrix2f pax::Matrix2f::operator*(pax::Ref<pax::Matrix2f> &rhs) {
	return matrix_2d_multiply(*this, *rhs);
}

#endif //__cplusplus