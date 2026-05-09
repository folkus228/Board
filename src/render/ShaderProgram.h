#pragma once

#include <unordered_map>
#include <string>
#include "glad/glad.h"

// Хранилище данных

class ShaderProgram {
private:
    GLuint _programID;
    std::unordered_map<GLenum, GLuint> _shaders; // тип шейдера - сам шейдер // заменить на вектор

public:
    ShaderProgram();
    ~ShaderProgram();

    // Добавление шейдеров разных типов
    bool AddShader(GLenum shaderType, const char* source);
    bool AddShaderFromFile(GLenum shaderType, const std::string& filepath);

    // Линковка программы
    bool Link();

    void Use() const;
    inline GLuint GetProgramID() const { return _programID; }
};