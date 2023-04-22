# PAX Docs: Matrix and vector

PAX has helper types for performing *vector math* and *matrix math*, both of which are useful in computer graphics.

The purpose for *vector types* is mostly to represent shapes and/or dimensions,
while *matrix types* represent
*[affine transformations](https://en.wikipedia.org/wiki/Affine_transformation)
([video explanation](https://www.youtube.com/watch?v=kYB8IZa5AuE))*.

In this document:
- Vector and related types:
	- [`pax::Vec2f` and `pax::Pointf`](#paxvec2f)
	- [`pax::BiVec2f` and `pax::Linef`](#paxbivec2f)
	- [`pax::TriVec2f` and `pax::Trif`](#paxtrivec2f)
	- [`pax::QuadVec2f` and `pax::Quadf`](#paxquadvec2f)
	- [`pax::Rectf`](#paxrectf)
	- [`pax::Vec2i`](#paxvec2i)
	- [`pax::Recti`](#paxrecti)
- Matrix and transformations:
	- [`pax::Matrix2f`](#paxmatrix2f)
	- [Transformations](#transformations)



# `pax::Vec2f`
A struct with `float x, y`.

Has vector math operators `+`, `-`, `*` and `/`.



# `pax::BiVec2f`
A struct with `float x0, y0, x1, y1`.

Has vector math operators `+`, `-`, `*` and `/`.
Has index operator returning `pax::Vec2f &`.



# `pax::TriVec2f`
A struct with `float x0, y0, x1, y1, x2, y2`.

Has vector math operators `+`, `-`, `*` and `/`.
Has index operator returning `pax::Vec2f &`.



# `pax::QuadVec2f`
A struct with `float x0, y0, x1, y1, x2, y2, x3, y3`.

Has vector math operators `+`, `-`, `*` and `/`.
Has index operator returning `pax::Vec2f &`.



# `pax::Rectf`
A struct with `float x, y, w, h` used to represent bounds.



# `pax::Vec2i`
A struct with `int x, y`.



Integer counterpart of `pax::Vec2f`.

# `pax::Recti`
A struct with `int x, y, w, h`.

Integer counterpart of `pax::Rectf`.



# `pax::Matrix2f`
Partial representation of 3x3 matrix, used for matrix transformations.


## Usage
This type is mostly used internally to achieve the transformations in `pax::Buffer`.

The multiply (`*`) operator performs *matrix multiplication* between two `pax::Matrix2f`.
For example, when matrix A is multiplied by B, the resulting matrix represents performing the transformation encoded by A followed by that of B.

**Note: Matrix multiplication is not *commutative*; `A*B != B*A`.**


## Layout
To save memory, the bottom row is *implicit* and can't be changed; this row is identical to the same in the *3x3 identity matrix*.
```
a0 a1 a2
b0 b1 b2
 0  0  1
```


## Creation

### `static Matrix2f identity()`
2D identity matrix: represents no transformation.
```
 1  0  0
 0  1  0
 0  0  1
```

---
### `static Matrix2f scale(float x, float y)`
2D scale matrix: represents a 2D scaling.
```
 x  0  0
 0  y  0
 0  0  1
```

---
### `static Matrix2f translate(float x, float y)`
2D translation matrix: represents a 2D movement of the camera.
```
 1  0  x
 0  1  y
 0  0  1
```

---
### `static Matrix2f shear(float x, float y)`
2D shear matrix: represents a 2D shearing.
```
 1  y  0
 x  1  0
 0  0  1
```

---
### `static Matrix2f rotate(float angle)`
2D rotation matrix: represents a 2D rotation.
```
 cos  -sin  0
 sin   cos  0
 0     0    1
```

