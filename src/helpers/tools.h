#pragma once

#include<string>
#include<glad/glad.h>

#ifdef TARGET_OS_OSX
#include<mach-o/dyld.h>
std::string PathOnMac();
#define Path(path) (PathOnMac() + path)
#else
#define Path(path) path
#endif

namespace tools
{
	namespace file_tools
	{
		const char* loadTextFromFile(const std::string& filename);
	}
	namespace gl_tools
	{
		bool loadTexture(const char* filename, GLuint& texture, GLenum tex_type, GLuint filter, GLuint wrap, GLint aligment = 4,
			bool auto_type = true, GLenum format = GL_RGBA);
		bool loadFlipTexture(unsigned char* data, int& width, int& height,
			GLuint& texture, GLenum tex_type, GLuint filter, GLuint wrap, GLint aligment = 4, GLenum format = GL_RED);
		//void createShaderProgram(GLuint& shader_program, const char* vertexShaderSource, const char* fragmentShaderSource);
		void createBufferObject(GLuint& glBO, GLenum BOtype, GLsizeiptr size_of_data, const void* data, GLenum draw_type);
		void createVertexArrayOnElements(GLuint& VAO, GLuint& VBO, GLuint& EBO, GLenum draw_type, GLboolean normalized, GLsizeiptr size_of_vertices, const void* vertices, GLsizeiptr size_of_indices, const void* indices, GLsizeiptr size_of_attributes, int* attributes);
	}
}