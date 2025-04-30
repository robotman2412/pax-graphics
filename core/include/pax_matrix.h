
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <math.h>

#ifdef __cplusplus
    #define PAX_CXX_Vecf_union union
#else
    #define PAX_CXX_Vecf_union struct
#endif

/* Backwards compat typedefs. */
typedef PAX_CXX_Vecf_union struct_pax_1vec2f pax_vec1_t;
typedef PAX_CXX_Vecf_union struct_pax_2vec2f pax_vec2_t;
typedef PAX_CXX_Vecf_union struct_pax_3vec2f pax_vec3_t;
typedef PAX_CXX_Vecf_union struct_pax_4vec2f pax_vec4_t;
typedef PAX_CXX_Vecf_union struct_pax_2vec2f pax_line_t;
typedef PAX_CXX_Vecf_union struct_pax_3vec2f pax_tri_t;
typedef PAX_CXX_Vecf_union struct_pax_4vec2f pax_quad_t;
typedef PAX_CXX_Vecf_union struct_pax_rectf  pax_rect_t;

/* Integer vectors. */
typedef PAX_CXX_Vecf_union struct_pax_1vec2i pax_vec2i;
typedef PAX_CXX_Vecf_union struct_pax_1vec2i pax_1vec2i;
typedef PAX_CXX_Vecf_union struct_pax_recti  pax_recti;

/* Float vectors. */
typedef PAX_CXX_Vecf_union struct_pax_1vec2f pax_vec2f;
typedef PAX_CXX_Vecf_union struct_pax_1vec2f pax_1vec2f;
typedef PAX_CXX_Vecf_union struct_pax_2vec2f pax_2vec2f;
typedef PAX_CXX_Vecf_union struct_pax_3vec2f pax_3vec2f;
typedef PAX_CXX_Vecf_union struct_pax_4vec2f pax_4vec2f;
typedef PAX_CXX_Vecf_union struct_pax_2vec2f pax_linef;
typedef PAX_CXX_Vecf_union struct_pax_3vec2f pax_trif;
typedef PAX_CXX_Vecf_union struct_pax_4vec2f pax_quadf;
typedef PAX_CXX_Vecf_union struct_pax_rectf  pax_rectf;

/* Shape things. */

/* Matrix stuff. */
typedef union matrix_2d        matrix_2d_t;
typedef struct matrix_stack_2d matrix_stack_2d_t;

#ifdef __cplusplus
    #include <assert.h>
    #include <initializer_list>
    #include <memory>

namespace pax {

typedef union struct_pax_1vec2f Vec2f;
typedef union struct_pax_2vec2f BiVec2f;
typedef union struct_pax_3vec2f TriVec2f;
typedef union struct_pax_4vec2f QuadVec2f;
typedef union struct_pax_1vec2f Pointf;
typedef union struct_pax_2vec2f Linef;
typedef union struct_pax_3vec2f Trif;
typedef union struct_pax_4vec2f Quadf;
typedef union struct_pax_rectf  Rectf;

typedef union struct_pax_1vec2i Vec2i;
typedef union struct_pax_recti  Recti;

typedef union matrix_2d        Matrix2f;
typedef struct matrix_stack_2d Matrix2fStack;

} // namespace pax

    #define PAX_CXX_Vec2f_INDEX()                                                                                      \
        pax::Vec2f &operator[](int index) {                                                                            \
            return ((pax::Vec2f *)arr)[index];                                                                         \
        }                                                                                                              \
        const pax::Vec2f &operator[](int index) const {                                                                \
            return ((const pax::Vec2f *)arr)[index];                                                                   \
        }

    #define PAX_CXX_Vecf_AVERAGE()                                                                                     \
        pax::Vec2f average() const {                                                                                   \
            pax::Vec2f   avg(0, 0);                                                                                    \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                avg += arr[i];                                                                                         \
            }                                                                                                          \
            return avg / _size;                                                                                        \
        }

    #define PAX_CXX_Vecf_OPERATOR(_type, _oper)                                                                        \
        _type operator _oper(_type rhs) const {                                                                        \
            _type        out;                                                                                          \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                out.arr[i] = arr[i] _oper rhs.arr[i];                                                                  \
            }                                                                                                          \
            return out;                                                                                                \
        }                                                                                                              \
        _type operator _oper(float rhs) const {                                                                        \
            _type        out;                                                                                          \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                out.arr[i] = arr[i] _oper rhs;                                                                         \
            }                                                                                                          \
            return out;                                                                                                \
        }

    #define PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, _oper)                                                                 \
        _type &operator _oper(_type rhs) {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                arr[i] _oper rhs.arr[i];                                                                               \
            }                                                                                                          \
            return *this;                                                                                              \
        }                                                                                                              \
        _type &operator _oper(float rhs) {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                arr[i] _oper rhs;                                                                                      \
            }                                                                                                          \
            return *this;                                                                                              \
        }

    #define PAX_CXX_Vecf_OPERATORS(_type)                                                                              \
        _type round() const {                                                                                          \
            _type        out;                                                                                          \
            const size_t _size = sizeof(arr) / sizeof(float);                                                          \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                out.arr[i] = roundf(arr[i]);                                                                           \
            }                                                                                                          \
            return out;                                                                                                \
        }                                                                                                              \
        bool operator==(_type rhs) const {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                if (arr[i] != rhs.arr[i])                                                                              \
                    return false;                                                                                      \
            }                                                                                                          \
            return true;                                                                                               \
        }                                                                                                              \
        bool operator!=(_type rhs) const {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(float);                                                      \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                if (arr[i] == rhs.arr[i])                                                                              \
                    return false;                                                                                      \
            }                                                                                                          \
            return true;                                                                                               \
        }                                                                                                              \
        PAX_CXX_Vecf_OPERATOR(_type, +) PAX_CXX_Vecf_OPERATOR(_type, -) PAX_CXX_Vecf_OPERATOR(_type, *)                \
            PAX_CXX_Vecf_OPERATOR(_type, /) PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, +=)                                    \
                PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, -=) PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, *=)                        \
                    PAX_CXX_Vecf_OPERATOR_ASSIGN(_type, /=)

    #define PAX_CXX_Vec2i_INDEX()                                                                                      \
        pax::Vec2i &operator[](int index) {                                                                            \
            return ((pax::Vec2i *)arr)[index];                                                                         \
        }                                                                                                              \
        const pax::Vec2i &operator[](int index) const {                                                                \
            return ((const pax::Vec2i *)arr)[index];                                                                   \
        }

    #define PAX_CXX_Veci_AVERAGE()                                                                                     \
        pax::Vec2i average() const {                                                                                   \
            int64_t      avgX, avgY;                                                                                   \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                avgX += arr[i].x;                                                                                      \
                avgY += arr[i].y;                                                                                      \
            }                                                                                                          \
            return {(avgX + _size / 2) / _size, (avgY + _size / 2) / _size};                                           \
        }

    #define PAX_CXX_Veci_OPERATOR(_type, _oper)                                                                        \
        _type operator _oper(_type rhs) const {                                                                        \
            _type        out;                                                                                          \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                out.arr[i] = arr[i] _oper rhs.arr[i];                                                                  \
            }                                                                                                          \
            return out;                                                                                                \
        }                                                                                                              \
        _type operator _oper(int rhs) const {                                                                          \
            _type        out;                                                                                          \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                out.arr[i] = arr[i] _oper rhs;                                                                         \
            }                                                                                                          \
            return out;                                                                                                \
        }

    #define PAX_CXX_Veci_OPERATOR_ASSIGN(_type, _oper)                                                                 \
        _type &operator _oper(_type rhs) {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                arr[i] _oper rhs.arr[i];                                                                               \
            }                                                                                                          \
            return *this;                                                                                              \
        }                                                                                                              \
        _type &operator _oper(int rhs) {                                                                               \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                arr[i] _oper rhs;                                                                                      \
            }                                                                                                          \
            return *this;                                                                                              \
        }

    #define PAX_CXX_Veci_OPERATORS(_type)                                                                              \
        _type round() const {                                                                                          \
            return *this;                                                                                              \
        }                                                                                                              \
        bool operator==(_type rhs) const {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                if (arr[i] != rhs.arr[i])                                                                              \
                    return false;                                                                                      \
            }                                                                                                          \
            return true;                                                                                               \
        }                                                                                                              \
        bool operator!=(_type rhs) const {                                                                             \
            const size_t _size = sizeof(arr) / 2 / sizeof(int);                                                        \
            for (size_t i = 0; i < _size; i++) {                                                                       \
                if (arr[i] == rhs.arr[i])                                                                              \
                    return false;                                                                                      \
            }                                                                                                          \
            return true;                                                                                               \
        }                                                                                                              \
        PAX_CXX_Veci_OPERATOR(_type, +) PAX_CXX_Veci_OPERATOR(_type, -) PAX_CXX_Veci_OPERATOR(_type, *)                \
            PAX_CXX_Veci_OPERATOR(_type, /) PAX_CXX_Veci_OPERATOR_ASSIGN(_type, +=)                                    \
                PAX_CXX_Veci_OPERATOR_ASSIGN(_type, -=) PAX_CXX_Veci_OPERATOR_ASSIGN(_type, *=)                        \
                    PAX_CXX_Veci_OPERATOR_ASSIGN(_type, /=)

#endif //__cplusplus

PAX_CXX_Vecf_union struct_pax_1vec2i {
#ifdef __cplusplus
    struct {
#endif
        // Single point.
        int x, y;
#ifdef __cplusplus
    };
    int arr[2];
#endif

#ifdef __cplusplus
    // Initialise to zero.
    struct_pax_1vec2i() {
        x = y = 0;
    }
    // Initialise with value.
    struct_pax_1vec2i(int _x, int _y) {
        x = _x;
        y = _y;
    }
    // Initialise from initialiser list.
    struct_pax_1vec2i(std::initializer_list<int> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }
    // Initialise as copy.
    inline struct_pax_1vec2i(pax::Vec2f const &other);

    PAX_CXX_Veci_OPERATORS(pax::Vec2i)
#endif //__cplusplus
};

PAX_CXX_Vecf_union struct_pax_1vec2f {
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
    struct_pax_1vec2f() {
        x = y = 0;
    }
    // Initialise with value.
    struct_pax_1vec2f(float _x, float _y) {
        x = _x;
        y = _y;
    }
    // Initialise from initialiser list.
    struct_pax_1vec2f(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }

    PAX_CXX_Vecf_OPERATORS(pax::Vec2f)

        // Unifies a this vector (it's magnitude will be 1).
        // Does not work for vectors with all zero.
        pax::Vec2f   &
        unify();
    // Calculate magnitude of vector.
    float magnitude();
    // Calculate magnitude squared of vector.
    float squareMagnitude();
#endif //__cplusplus
};

#ifdef __cplusplus
pax::Vec2i::struct_pax_1vec2i(pax::Vec2f const &other) {
    x = (int)other.x;
    y = (int)other.y;
}
#endif

PAX_CXX_Vecf_union struct_pax_2vec2f {
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
    struct_pax_2vec2f() {
        x0 = y0 = x1 = y1 = 0;
    }
    // Initialise with value.
    struct_pax_2vec2f(float _x0, float _y0, float _x1, float _y1) {
        x0 = _x0;
        y0 = _y0;
        x1 = _x1;
        y1 = _y1;
    }
    // Initialise from initialiser list.
    struct_pax_2vec2f(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }

    // Operator []
    PAX_CXX_Vec2f_INDEX()
        // Function average()
        PAX_CXX_Vecf_AVERAGE()
        // Operators + - * / += -= *= /= =
        PAX_CXX_Vecf_OPERATORS(pax::BiVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union struct_pax_3vec2f {
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
    struct_pax_3vec2f() {
        x0 = y0 = x1 = y1 = x2 = y2 = 0;
    }
    // Initialise with value.
    struct_pax_3vec2f(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2) {
        x0 = _x0;
        y0 = _y0;
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
    }
    // Initialise from initialiser list.
    struct_pax_3vec2f(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }

    // Operator []
    PAX_CXX_Vec2f_INDEX()
        // Operators + - * / += -= *= /= =
        PAX_CXX_Vecf_OPERATORS(pax::TriVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union struct_pax_4vec2f {
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
    struct_pax_4vec2f() {
        x0 = y0 = x1 = y1 = x2 = y2 = x3 = y3 = 0;
    }
    // Initialise with value.
    struct_pax_4vec2f(float _x0, float _y0, float _x1, float _y1, float _x2, float _y2, float _x3, float _y3) {
        x0 = _x0;
        y0 = _y0;
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
        x3 = _x3;
        y3 = _y3;
    }
    // Initialise from initialiser list.
    struct_pax_4vec2f(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }

    // Operator []
    PAX_CXX_Vec2f_INDEX()
        // Operators + - * / += -= *= /= =
        PAX_CXX_Vecf_OPERATORS(pax::QuadVec2f)
#endif //__cplusplus
};

PAX_CXX_Vecf_union struct_pax_recti {
#ifdef __cplusplus
    struct {
#endif
        // Rectangle points.
        int x, y, w, h;
#ifdef __cplusplus
    };
    int arr[4];
#endif

#ifdef __cplusplus
    // Initialise to zero.
    struct_pax_recti() {
        x = y = w = h = 0;
    }
    // Initialise with value.
    struct_pax_recti(int _x, int _y, int _w, int _h) {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }
    // Initialise from initialiser list.
    struct_pax_recti(std::initializer_list<int> list) {
        assert(list.size() == sizeof(arr) / sizeof(int));
        std::copy(list.begin(), list.end(), arr);
    }
    // Initialise from initialiser list.
    struct_pax_recti(std::initializer_list<pax::Vec2i> list) {
        assert(list.size() == 2);
        position() = list.begin()[0];
        size()     = list.begin()[1];
    }

    // Operator []
    PAX_CXX_Vec2i_INDEX()

        // Comparator.
        bool
        operator==(pax::Recti const &other) const {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }
    // Comparator.
    bool operator!=(pax::Recti const &other) const {
        return !operator==(other);
    }

    // Get average position, i.e. center, of the rectangle.
    pax::Vec2i average() const {
        return pax::Vec2f(x + w / 2.0f, y + h / 2.0f);
    }
    // Get X/Y component.
    pax::Vec2i &position() {
        return *(pax::Vec2i *)&x;
    }
    // Get X/Y component.
    pax::Vec2i const &position() const {
        return *(pax::Vec2i *)&x;
    }
    // Get width/height component.
    pax::Vec2i &size() {
        return *(pax::Vec2i *)&w;
    }
    // Get width/height component.
    pax::Vec2i const &size() const {
        return *(pax::Vec2i *)&w;
    }
    // Create an equivalent quad.
    pax::Quadf toQuad() const {
        return pax::Quadf(x, y, x + w, y, x + w, y + h, x, y + h);
    }
    // Get a copy which gaurantees nonnegative dimensions.
    pax::Recti fixSize() const {
        pax::Recti out = *this;
        if (out.w < 0) {
            out.x += out.w;
            out.w  = -out.w;
        }
        if (out.h < 0) {
            out.y += out.h;
            out.h  = -out.h;
        }
        return out;
    }
#endif //__cplusplus
};

PAX_CXX_Vecf_union struct_pax_rectf {
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
    struct_pax_rectf() {
        x = y = w = h = 0;
    }
    // Initialise with value.
    struct_pax_rectf(float _x, float _y, float _w, float _h) {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }
    // Initialise from initialiser list.
    struct_pax_rectf(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }
    // Initialise from initialiser list.
    struct_pax_rectf(std::initializer_list<pax::Vec2f> list) {
        assert(list.size() == 2);
        position() = list.begin()[0];
        size()     = list.begin()[1];
    }

    // Operator []
    PAX_CXX_Vec2f_INDEX()

        // Comparator.
        bool
        operator==(pax::Rectf const &other) const {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }
    // Comparator.
    bool operator!=(pax::Rectf const &other) const {
        return !operator==(other);
    }

    pax::Rectf round() const {
        return pax::Rectf{floorf(x), floorf(y), floorf(w), floorf(h)};
    }

    // Get average position, i.e. center, of the rectangle.
    pax::Vec2f average() {
        return pax::Vec2f(x + w / 2.0f, y + h / 2.0f);
    }
    // Get X/Y component.
    pax::Vec2f &position() {
        return *(pax::Vec2f *)&x;
    }
    // Get width/height component.
    pax::Vec2f &size() {
        return *(pax::Vec2f *)&w;
    }
    // Create an equivalent quad.
    pax::Quadf toQuad() {
        return pax::Quadf(x, y, x + w, y, x + w, y + h, x, y + h);
    }
    // Get a copy which gaurantees nonnegative dimensions.
    pax::Rectf fixSize() {
        pax::Rectf out = *this;
        if (out.w < 0) {
            out.x += out.w;
            out.w  = -out.w;
        }
        if (out.h < 0) {
            out.y += out.h;
            out.h  = -out.h;
        }
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
    matrix_2d() {
        a0 = 1;
        a1 = a2 = b0 = 0;
        b1           = 1;
        b2           = 0;
    }
    // Initialise with value.
    matrix_2d(float _a0, float _a1, float _a2, float _b0, float _b1, float _b2) {
        a0 = _a0;
        a1 = _a1;
        a2 = _a2;
        b0 = _b0;
        b1 = _b1;
        b2 = _b2;
    }
    // Initialise from initialiser list.
    matrix_2d(std::initializer_list<float> list) {
        assert(list.size() == sizeof(arr) / sizeof(float));
        std::copy(list.begin(), list.end(), arr);
    }

    // Comparator.
    bool operator==(matrix_2d const &other) const {
        for (auto i = 0; i < 6; i++) {
            if (arr[i] != other.arr[i])
                return false;
        }
        return true;
    }
    // Comparator.
    bool operator!=(matrix_2d const &other) const {
        return !operator==(other);
    }

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
    matrix_2d operator*(matrix_2d rhs);
    // Matrix multiplication. Note that A*B != B*A in matrices.
    matrix_2d operator*(matrix_2d const &rhs);
#endif //__cplusplus
};



#ifdef __cplusplus
extern "C" {
#endif

// Check whether the matrix exactly equals the identity matrix.
static inline bool matrix_2d_is_identity(matrix_2d_t m) {
    return m.a0 == 1 && m.a1 == 0 && m.a2 == 0 && m.b0 == 0 && m.b1 == 1 && m.b2 == 0;
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
    return pax::Matrix2f(1, 0, 0, 0, 1, 0);
#else
    return (matrix_2d_t){.arr = {1, 0, 0, 0, 1, 0}};
#endif
}
// 2D scale matrix: represents a 2D scaling.
static inline matrix_2d_t matrix_2d_scale(float x, float y) {
#ifdef __cplusplus
    return pax::Matrix2f(x, 0, 0, 0, y, 0);
#else
    return (matrix_2d_t){.arr = {x, 0, 0, 0, y, 0}};
#endif
}
// 2D translation matrix: represents a 2D movement of the camera.
static inline matrix_2d_t matrix_2d_translate(float x, float y) {
#ifdef __cplusplus
    return pax::Matrix2f(1, 0, x, 0, 1, y);
#else
    return (matrix_2d_t){.arr = {1, 0, x, 0, 1, y}};
#endif
}
// 2D shear matrix: represents a 2D shearing.
static inline matrix_2d_t matrix_2d_shear(float x, float y) {
#ifdef __cplusplus
    return pax::Matrix2f(1, y, 0, x, 1, 0);
#else
    return (matrix_2d_t){.arr = {1, y, 0, x, 1, 0}};
#endif
}
// 2D rotation matrix: represents a 2D rotation.
matrix_2d_t matrix_2d_rotate(float angle) __attribute__((const));

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) __attribute__((const));
// 2D matrix: applies the transformation that a represents on to a point.
void        matrix_2d_transform(matrix_2d_t a, float *x, float *y);
// 2D matrix: applies the transformation that a represents on to a point.
pax_vec2f   matrix_2d_transform_alt(matrix_2d_t a, pax_vec2f b) __attribute__((const));

// Convert the rectangle to one that covers the same area but with positive size.
pax_recti pax_recti_abs(pax_recti a) __attribute__((const));
// Convert the rectangle to one that covers the same area but with positive size.
pax_rectf pax_rectf_abs(pax_rectf a) __attribute__((const));
// Get the intersection between two rectangles.
// Returns {0, 0, 0, 0} if there is no intersection.
pax_recti pax_recti_intersect(pax_recti a, pax_recti b) __attribute__((const));
// Get the intersection between two rectangles.
// Returns {0, 0, 0, 0} if there is no intersection.
pax_rectf pax_rectf_intersect(pax_rectf a, pax_rectf b) __attribute__((const));

// 2D vector: unifies a given vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
pax_vec2f pax_vec2f_unify(pax_vec2f vec);

#ifdef __cplusplus
} // extern "C"
#endif



#ifdef __cplusplus

// Unifies a this vector (it's magnitude will be 1).
// Does not work for vectors with all zero.
inline pax::Vec2f &pax::Vec2f::unify() {
    float mag  = magnitude();
    x         /= mag;
    y         /= mag;
    return *this;
}

// Calculate magnitude of vector.
inline float pax::Vec2f::magnitude() {
    return sqrtf(x * x + y * y);
}

// Calculate magnitude squared of vector.
inline float pax::Vec2f::squareMagnitude() {
    return x * x + y * y;
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
inline pax::Matrix2f pax::Matrix2f::operator*(pax::Matrix2f rhs) {
    return matrix_2d_multiply(*this, rhs);
}
// Matrix multiplication. Note that A*B != B*A in matrices.
inline pax::Matrix2f pax::Matrix2f::operator*(pax::Matrix2f const &rhs) {
    return matrix_2d_multiply(*this, rhs);
}

#endif //__cplusplus