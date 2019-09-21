#include "JetStory.h"

namespace Upp {
extern GLProgram gl_image, gl_image_colored;
extern GLProgram gl_rect;
extern int u_color;
};

void DrawImage(int x, int y, float alpha, const Image& img, dword flags)
{
	static GLProgram gl_alpha;
	ONCELOCK {
		gl_alpha.Create(R"(
			attribute vec4 a_position;
			attribute vec2 a_texCoord;
			attribute float a_alpha;
			uniform mat4 u_projection;
			varying vec2 v_texCoord;
			varying float v_alpha;
			void main()
			{
			   gl_Position = u_projection * a_position;
			   v_texCoord = a_texCoord;
			   v_alpha = a_alpha;
			}
			)", R"(
		#ifdef GL_ES
			precision mediump float;
			precision mediump int;
		#endif
			varying vec2 v_texCoord;
			varying float v_alpha;
			uniform sampler2D s_texture;
			void main()
			{
			   gl_FragColor = texture2D(s_texture, v_texCoord);
			   gl_FragColor[0] = v_alpha * gl_FragColor[0];
			   gl_FragColor[1] = v_alpha * gl_FragColor[1];
			   gl_FragColor[2] = v_alpha * gl_FragColor[2];
			   gl_FragColor[3] = v_alpha * gl_FragColor[3];
			}
			)",
			ATTRIB_VERTEX, "a_position",
			ATTRIB_TEXPOS, "a_texCoord",
			ATTRIB_ALPHA, "a_alpha"
		);
	
		glUniform1i(gl_alpha.GetUniform("s_texture"), 0);

		gl_alpha.Use();
		GLOrtho(0, (float)window_size.cx, (float)window_size.cy, 0, 0.0f, 1.0f, gl_alpha.GetUniform("u_projection"));
	}
	
	Size isz = img.GetSize();
	Rect r(Point(x, y), isz);
	
	if(flags & SWAPX)
		Swap(r.left, r.right);
	if(flags & SWAPY)
		Swap(r.top, r.bottom);
	
	GLshort vertex[8];
	
	if(flags & ROTATE) {
		vertex[0] = (GLshort)r.left;
		vertex[1] = (GLshort)r.bottom;
		vertex[2] = (GLshort)r.right;
		vertex[3] = (GLshort)r.bottom;
		vertex[4] = (GLshort)r.right;
		vertex[5] = (GLshort)r.top;
		vertex[6] = (GLshort)r.left;
		vertex[7] = (GLshort)r.top;
	}
	else {
		vertex[0] = (GLshort)r.left;
		vertex[1] = (GLshort)r.top;
		vertex[2] = (GLshort)r.left;
		vertex[3] = (GLshort)r.bottom;
		vertex[4] = (GLshort)r.right;
		vertex[5] = (GLshort)r.bottom;
		vertex[6] = (GLshort)r.right;
		vertex[7] = (GLshort)r.top;
	};

	static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	static float fixed[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
	};

	GLfloat alphas[] = {
		alpha,
		alpha,
		alpha,
		alpha,
	};
	
	gl_alpha.Use();
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_TEXPOS);
	glEnableVertexAttribArray(ATTRIB_ALPHA);
	glVertexAttribPointer(ATTRIB_TEXPOS, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), fixed);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);
	glVertexAttribPointer(ATTRIB_ALPHA, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), alphas);
	glBindTexture(GL_TEXTURE_2D, GetTextureForImage(img));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(ATTRIB_ALPHA);
	glDisableVertexAttribArray(ATTRIB_TEXPOS);
}

void DrawImage(int x, int y, const Image& img, dword flags)
{
	Size isz = img.GetSize();
	Rect r(Point(x, y), isz);
	
	if(flags & SWAPX)
		Swap(r.left, r.right);
	if(flags & SWAPY)
		Swap(r.top, r.bottom);
	
	GLshort vertex[8];
	
	if(flags & ROTATE) {
		vertex[0] = (GLshort)r.left;
		vertex[1] = (GLshort)r.bottom;
		vertex[2] = (GLshort)r.right;
		vertex[3] = (GLshort)r.bottom;
		vertex[4] = (GLshort)r.right;
		vertex[5] = (GLshort)r.top;
		vertex[6] = (GLshort)r.left;
		vertex[7] = (GLshort)r.top;
	}
	else {
		vertex[0] = (GLshort)r.left;
		vertex[1] = (GLshort)r.top;
		vertex[2] = (GLshort)r.left;
		vertex[3] = (GLshort)r.bottom;
		vertex[4] = (GLshort)r.right;
		vertex[5] = (GLshort)r.bottom;
		vertex[6] = (GLshort)r.right;
		vertex[7] = (GLshort)r.top;
	};

	static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	static float fixed[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
	};

	gl_image.Use();
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_TEXPOS);
	glVertexAttribPointer(ATTRIB_TEXPOS, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), fixed);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);
	glBindTexture(GL_TEXTURE_2D, GetTextureForImage(img));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(ATTRIB_TEXPOS);
}

void DrawImage(Point vert[4], const Image& img)
{
	Size isz = img.GetSize();
	
	GLshort vertex[8];
	
	vertex[0] = (GLshort)vert[0].x;
	vertex[1] = (GLshort)vert[0].y;
	vertex[2] = (GLshort)vert[2].x;
	vertex[3] = (GLshort)vert[2].y;
	vertex[4] = (GLshort)vert[3].x;
	vertex[5] = (GLshort)vert[3].y;
	vertex[6] = (GLshort)vert[1].x;
	vertex[7] = (GLshort)vert[1].y;

	static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	static float fixed[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
	};

	gl_image.Use();
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_TEXPOS);
	glVertexAttribPointer(ATTRIB_TEXPOS, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), fixed);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);
	glBindTexture(GL_TEXTURE_2D, GetTextureForImage(img));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(ATTRIB_TEXPOS);
}

void DrawImage(Point p, const Image& img, Color color)
{
	gl_image_colored.Use();

	Size isz = img.GetSize();
	Rect rect(p, isz);

	GLshort vertex[] = {
	    (GLshort)rect.left, (GLshort)rect.top,
	    (GLshort)rect.left, (GLshort)rect.bottom,
	    (GLshort)rect.right, (GLshort)rect.bottom,
	    (GLshort)rect.right, (GLshort)rect.top,
	};

	GLubyte r = color.GetR();
	GLubyte g = color.GetG();
	GLubyte b = color.GetB();

	GLubyte colors[] = {
		r, g, b,
		r, g, b,
		r, g, b,
		r, g, b,
	};

	static GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	const float *tc;

	static float fixed[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
	};
	tc = fixed;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_TEXPOS);
	glVertexAttribPointer(ATTRIB_TEXPOS, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), tc);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(ATTRIB_COLOR, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3 * sizeof(GLubyte), colors);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);
	glBindTexture(GL_TEXTURE_2D, GetTextureForImage(img));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_TEXPOS);
}

void DrawTriangle(Point p1, Point p2, Point p3, Color c1)
{
	gl_rect.Use();

	GLshort vertex[] = {
	    (GLshort)p1.x, (GLshort)p1.y,
	    (GLshort)p2.x, (GLshort)p2.y,
	    (GLshort)p3.x, (GLshort)p3.y,
	};

	static GLushort indices[] = { 0, 1, 2 };

	GLubyte r = c1.GetR();
	GLubyte g = c1.GetG();
	GLubyte b = c1.GetB();
	GLubyte color[] = {
		r, g, b,
		r, g, b,
		r, g, b,
	};

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(ATTRIB_COLOR, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3 * sizeof(GLubyte), color);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);

	glDisable(GL_BLEND);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices);
	glEnable(GL_BLEND);

	glDisableVertexAttribArray(ATTRIB_COLOR);
}

void DrawRect(const Rect& r, Color c1)
{
	DrawTriangle(r.TopLeft(), r.TopRight(), r.BottomLeft(), c1);
	DrawTriangle(r.BottomLeft(), r.TopRight(), r.BottomRight(), c1);
}

void DrawTriangle(Point p1, Point p2, Point p3, float alpha, Color c1)
{
	static GLProgram gl_rect;
	ONCELOCK {
		gl_rect.Create(R"(
			attribute vec4 a_position;
			attribute vec4 a_color;
			attribute float a_alpha;
			uniform mat4 u_projection;
			varying vec4 v_color;
			varying float v_alpha;
			void main()
			{
			 gl_Position = u_projection * a_position;
			 v_color = a_color * vec4((1.0 / 255.0), (1.0 / 255.0), (1.0 / 255.0), 1);
			 v_alpha = a_alpha;
			}
		)", R"(
		#ifdef GL_ES
			precision mediump float;
			precision mediump int;
		#endif
			varying float v_alpha;
			varying vec4 v_color;
			void main()
			{
			    gl_FragColor = v_color;
			    gl_FragColor[3] = v_alpha * gl_FragColor[3];
			}
			)",
			ATTRIB_VERTEX, "a_position",
			ATTRIB_COLOR, "a_color",
			ATTRIB_ALPHA, "a_alpha"
		);
		gl_rect.Use();
		GLOrtho(0, (float)window_size.cx, (float)window_size.cy, 0, 0.0f, 1.0f, gl_rect.GetUniform("u_projection"));
	}
	gl_rect.Use();
	
	GLshort vertex[] = {
	    (GLshort)p1.x, (GLshort)p1.y,
	    (GLshort)p2.x, (GLshort)p2.y,
	    (GLshort)p3.x, (GLshort)p3.y,
	};

	static GLushort indices[] = { 0, 1, 2 };

	GLubyte r = c1.GetR();
	GLubyte g = c1.GetG();
	GLubyte b = c1.GetB();
	GLubyte color[] = {
		r, g, b,
		r, g, b,
		r, g, b,
	};

	GLfloat alphas[] = {
		alpha,
		alpha,
		alpha,
	};
	
	glBlendFunc(GL_ONE/*SRC_ALPHA*/, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glEnableVertexAttribArray(ATTRIB_ALPHA);
	glVertexAttribPointer(ATTRIB_COLOR, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3 * sizeof(GLubyte), color);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 2 * sizeof(GLshort), vertex);
	glVertexAttribPointer(ATTRIB_ALPHA, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), alphas);

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(ATTRIB_COLOR);
	glDisableVertexAttribArray(ATTRIB_ALPHA);
}

void DrawRect(const Rect& r, float alpha, Color c1)
{
	DrawTriangle(r.TopLeft(), r.TopRight(), r.BottomLeft(), alpha, c1);
	DrawTriangle(r.BottomLeft(), r.TopRight(), r.BottomRight(), alpha, c1);
}
