#include <iostream>
#include <string>
#include <fstream>
#include "ShaderProgram.h"

// ЭТАП 1

ShaderProgram::ShaderProgram()
	: _programID(glCreateProgram())
{}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(_programID);
}

bool ShaderProgram::AddShader(GLenum shaderType, const char* source)
{
	GLuint shader = glCreateShader(shaderType);
	_shaders[shaderType] = shader;
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
#ifdef _DEBUG
	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[1024]; glGetShaderInfoLog(shader, 1024, nullptr, log);
		std::cerr << "\033[90m" << "shader " << "\033[91m" << "compile error"
			<< "\033[90m" << ": " << log << "\033[0m" << std::endl;
	}
#endif
	glAttachShader(_programID, shader);
	return true;
}

bool ShaderProgram::AddShaderFromFile(GLenum shaderType, const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::binary);

#ifdef _DEBUG
	if (!file.is_open()) {
		std::cerr << "\033[90m" << "shader file \"" << filepath << "\" is "
			<< "\033[91m" << "not" << "\033[90m" 
			<< " opened" << std::endl;
		return false;
	}
#endif

	// Получаем размер файла
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Выделяем память под содержимое файла + нуль-терминатор
	char* buffer = new char[size + 1];

	// Читаем файл
	file.read(buffer, size);
	buffer[size] = '\0'; // Добавляем нуль-терминатор

	file.close();

	AddShader(shaderType, buffer);
	
	return true;
}

bool ShaderProgram::Link()
{
	glLinkProgram(_programID);
	
#ifdef _DEBUG
	GLint ok = 0; glGetProgramiv(_programID, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[1024]; glGetProgramInfoLog(_programID, 1024, nullptr, log);
		std::cerr << "\033[90m" << "shader program link "
			<< "\033[91m" << "error" << "\033[90m"
			<<": " << log << "\033[0m" << std::endl;
	}
#endif

	for (std::pair<GLenum, GLuint> value : _shaders)
		glDeleteShader(value.second);

	return true;
}

void ShaderProgram::Use() const
{
	glUseProgram(_programID);
}