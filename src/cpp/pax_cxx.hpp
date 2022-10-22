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

#include <pax_types.h>

#include <functional>

// This serves as a C++ wrapper which is fully compatible with the normal C version of PAX graphics.
namespace pax {

// A C++ ShaderFunc thing for use with lambdas and such.
typedef std::function<pax_col_t(pax_col_t tint, int x, int y, float u, float v, void *args)> ShaderFunc;

// A helper data structure for C++ shaders.
typedef struct {
	ShaderFunc *callback;
	void *args;
} ShaderContent;

// Shader class stub.
class Shader {
	protected:
		// The internal shader object to run.
		pax_shader_t internal;
		// Optional C++ shader function.
		ShaderContent cxxShaderCtx;
		// Wehther this is a C++ type shader.
		bool isCxx;
		// Whether this shader actually does anything.
		bool active;
		
	public:
		// Make an empty shader.
		// This shader won't change anything.
		Shader();
		// Make a shader using an existing PAX shader.
		Shader(pax_shader_t *existing);
		// Make a shader a C++ version of shader callback.
		Shader(ShaderFunc callback, void *context);
		
		// Deletion operator.
		~Shader();
		
		// Apply this shader to a pixel.
		pax_col_t apply(pax_col_t tint, int x, int y, float u, float v);
		// Get a shader object for using in C PAX APIs.
		pax_shader_t *getInternal();
};

class Buffer {
	protected:
		// The PAX buffer this wrapper is of.
		pax_buf_t *internal;
		// Whether to also delete the internal buffer.
		bool deleteInternal;
		
	public:
		// Default color to use for drawing filled shapes.
		pax_col_t fillColor;
		// Default color to use for drawing outlined shapes.
		pax_col_t lineColor;
		
		// Make an empty wrapper.
		Buffer();
		// Make a wrapper using an existing PAX buffer.
		Buffer(pax_buf_t *existing);
		// Make a wrapper of a new buffer.
		Buffer(int width, int height, pax_buf_type_t type);
		// Make a wrapper of a new buffer, with preallocated memory.
		// This memory will not be free()d by default.
		Buffer(void *preallocated, int width, int height, pax_buf_type_t type);
		
		// Deletion operator.
		~Buffer();
		
		// Draws a rectangle with the default color.
		void drawRect(float x, float y, float width, float height);
		// Draws a rectangle with a custom color.
		void drawRect(pax_col_t color, float x, float y, float width, float height);
		// Draws a rectangle with a custom shader.
		// Shader is ignored if NULL.
		void drawRect(Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Draws a rectangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawRect(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float width, float height);
		// Outlines a rectangle with the default outline color.
		void outlineRect(float x, float y, float width, float height);
		// Outlines a rectangle with a custom outline color.
		void outlineRect(pax_col_t color, float x, float y, float width, float height);
		
		// Draws a triangle with the default color.
		void drawTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color.
		void drawTri(pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom shader.
		// Shader is ignored if NULL.
		void drawTri(Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Draws a triangle with a custom color and shader.
		// Shader is ignored if NULL.
		void drawTri(pax_col_t color, Shader *shader, pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with the default outline color.
		void outlineTri(float x0, float y0, float x1, float y1, float x2, float y2);
		// Outlines a triangle with a custom outline color.
		void outlineTri(pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2);
		
		// Draws a circle around the given point with the default color.
		void drawCircle(float x, float y, float radius);
		// Draws a circle around the given point with a custom color.
		void drawCircle(pax_col_t color, float x, float y, float radius);
		// Draws a circle around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawCircle(Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Draws a circle around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawCircle(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius);
		// Outlines a circle with the default outline color.
		void outlineCircle(float x, float y, float radius);
		// Outlines a circle with a custom outline color.
		void outlineCircle(pax_col_t color, float x, float y, float radius);
		
		// Draws an arc around the given point with the default color.
		void drawArc(float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color.
		void drawArc(pax_col_t color, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom shader.
		// Shader is ignored if NULL.
		void drawArc(Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Draws an arc around the given point with a custom color and shader.
		// Shader is ignored if NULL.
		void drawArc(pax_col_t color, Shader *shader, pax_quad_t *uvs, float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with the default outline color.
		void outlineArc(float x, float y, float radius, float startangle, float endangle);
		// Outlines an arc with a custom outline color.
		void outlineArc(pax_col_t color, float x, float y, float radius, float startangle, float endangle);
		
		// Draws a line with the default outline color.
		void drawLine(float x0, float y0, float x1, float y1);
		// Draws a line with a custom outline color.
		void drawLine(pax_col_t color, float x0, float y0, float x1, float y1);
};

}
