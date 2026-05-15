#pragma once
#include <unordered_map>
#include <string>
#include <variant>
#include <vector>
#include <iostream>

#include "ShaderProgram.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

// Вся логика и обработка инофрмации из ShaderProgram

class Material;

// Enum для типов uniform'ов
enum class UniformType {
    FLOAT,
    INT,
    BOOL,
    VEC2,
    VEC3,
    VEC4,
    MAT3,
    MAT4,
    SAMPLER2D,
    SAMPLERCUBE
};

// Variant для хранения значений
using UniformValue = std::variant<
    float,
    int,
    bool,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat3,
    glm::mat4,
    GLuint  // для текстур
>;

struct LocationAndDirty
{
    UniformType type;
    UniformValue value;
    GLuint location;
    bool isDirty;
};

class Shader
{
private:
    ShaderProgram _shaderProgram;
    std::unordered_map<std::string, LocationAndDirty> _uniformData;

public:
    Shader();
    ~Shader();

    bool LoadShader(GLenum shaderType, const char* source);
    bool LoadShaderFromFile(GLenum shaderType, const std::string& filepath);
    bool CompileProgram();

    void ParseUniforms();

    template<typename T>
    void SetUniform(const std::string& name, const T& value);
    template<typename T>
    void SetUniform(const std::string& name, const T&& value);
    template<typename T>
    void SetUniform(const std::string&& name, const T&& value);

    void ApplyUniforms();

    void Use();

    //UniformType GetUniformType(std::string name);
    //UniformValue GetUniformValue(const std::string& name) const;

    UniformType GLTypeToUniformType(GLenum glType);
    friend Material;
};

template<typename T>
void Shader::SetUniform(const std::string& name, const T& value) {

#ifdef _DEBUG
    auto typeIt = _uniformData.find(name);
    if (typeIt == _uniformData.end()) return;

    UniformType expectedType = typeIt->second.type;

    if constexpr (std::is_same_v<T, float>) {
        if (expectedType != UniformType::FLOAT) return;
    }
    else if constexpr (std::is_same_v<T, int>) {
        if (expectedType != UniformType::INT) return;
    }
    else if constexpr (std::is_same_v<T, bool>) {
        if (expectedType != UniformType::BOOL) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec2>) {
        if (expectedType != UniformType::VEC2) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec3>) {
        if (expectedType != UniformType::VEC3) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec4>) {
        if (expectedType != UniformType::VEC4) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat3>) {
        if (expectedType != UniformType::MAT3) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat4>) {
        if (expectedType != UniformType::MAT4) return;
    }
    else if constexpr (std::is_same_v<T, GLuint>) {
        if (expectedType != UniformType::SAMPLER2D &&
            expectedType != UniformType::SAMPLERCUBE) return;
    }
    else
    {
        std::cout << "\033[90m" << "parameter \"" << typeIt->first << "\" passes an " 
            << "\033[91m" << "unsupported" << "\033[90m" 
            << " type" << "\033[0m" << std::endl;
        return;
    }
#endif
    _uniformData[name].value = value;
    _uniformData[name].isDirty = true;
}

// только эту оставить
template<typename T>
void Shader::SetUniform(const std::string& name, const T&& value) {

#ifdef _DEBUG
    auto typeIt = _uniformData.find(name);
    if (typeIt == _uniformData.end()) return;

    UniformType expectedType = typeIt->second.type;

    if constexpr (std::is_same_v<T, float>) {
        if (expectedType != UniformType::FLOAT) return;
    }
    else if constexpr (std::is_same_v<T, int>) {
        if (expectedType != UniformType::INT) return;
    }
    else if constexpr (std::is_same_v<T, bool>) {
        if (expectedType != UniformType::BOOL) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec2>) {
        if (expectedType != UniformType::VEC2) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec3>) {
        if (expectedType != UniformType::VEC3) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec4>) {
        if (expectedType != UniformType::VEC4) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat3>) {
        if (expectedType != UniformType::MAT3) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat4>) {
        if (expectedType != UniformType::MAT4) return;
    }
    else if constexpr (std::is_same_v<T, GLuint>) {
        if (expectedType != UniformType::SAMPLER2D &&
            expectedType != UniformType::SAMPLERCUBE) return;
    }
    else
    {
        std::cout << "\033[90m" << "parameter \"" << typeIt->first << "\" passes an "
            << "\033[91m" << "unsupported" << "\033[90m"
            << " type" << "\033[0m" << std::endl;
        return;
    }
#endif
    _uniformData[name].value = value;
    _uniformData[name].isDirty = true;
}

template<typename T>
void Shader::SetUniform(const std::string&& name, const T&& value) {

#ifdef _DEBUG
    auto typeIt = _uniformData.find(name);
    if (typeIt == _uniformData.end()) return;

    UniformType expectedType = typeIt->second.type;

    if constexpr (std::is_same_v<T, float>) {
        if (expectedType != UniformType::FLOAT) return;
    }
    else if constexpr (std::is_same_v<T, int>) {
        if (expectedType != UniformType::INT) return;
    }
    else if constexpr (std::is_same_v<T, bool>) {
        if (expectedType != UniformType::BOOL) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec2>) {
        if (expectedType != UniformType::VEC2) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec3>) {
        if (expectedType != UniformType::VEC3) return;
    }
    else if constexpr (std::is_same_v<T, glm::vec4>) {
        if (expectedType != UniformType::VEC4) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat3>) {
        if (expectedType != UniformType::MAT3) return;
    }
    else if constexpr (std::is_same_v<T, glm::mat4>) {
        if (expectedType != UniformType::MAT4) return;
    }
    else if constexpr (std::is_same_v<T, GLuint>) {
        if (expectedType != UniformType::SAMPLER2D &&
            expectedType != UniformType::SAMPLERCUBE) return;
    }
    else
    {
        std::cout << "\033[90m" << "parameter \"" << typeIt->first << "\" passes an "
            << "\033[91m" << "unsupported" << "\033[90m"
            << " type" << "\033[0m" << std::endl;
        return;
    }
#endif
    _uniformData[name].value = value;
    _uniformData[name].isDirty = true;
}
