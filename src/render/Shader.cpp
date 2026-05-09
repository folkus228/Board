#include "Shader.h"

Shader::Shader() {}

Shader::~Shader() {}

bool Shader::LoadShader(GLenum shaderType, const char* source)
{
    _shaderProgram.AddShader(shaderType, source);
    return true;
}

bool Shader::LoadShaderFromFile(GLenum shaderType, const std::string& filepath)
{
    _shaderProgram.AddShaderFromFile(shaderType, filepath);
    return true;
}

bool Shader::CompileProgram()
{
    _shaderProgram.Link();
    ParseUniforms();
    return true;
}

void Shader::ParseUniforms() {
    if (_shaderProgram.GetProgramID() == 0) return;

    GLint uniformCount;
    glGetProgramiv(_shaderProgram.GetProgramID(), GL_ACTIVE_UNIFORMS, &uniformCount);

    GLint maxNameLength;
    glGetProgramiv(_shaderProgram.GetProgramID(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

    std::vector<GLchar> nameBuffer(maxNameLength);

    for (GLint i = 0; i < uniformCount; ++i) {
        GLsizei nameLength;
        GLint size;
        GLenum type;

        glGetActiveUniform(_shaderProgram.GetProgramID(), i, maxNameLength,
            &nameLength, &size, &type, nameBuffer.data());

        std::string uniformName(nameBuffer.data(), nameLength);
        GLuint location = glGetUniformLocation(_shaderProgram.GetProgramID(), uniformName.c_str());

        UniformType uniformType = GLTypeToUniformType(type);

        _uniformData[uniformName].type = uniformType;
        _uniformData[uniformName].location = location;
        _uniformData[uniformName].isDirty = false;
    }
}

UniformType Shader::GLTypeToUniformType(GLenum glType) {
    switch (glType) {
    case GL_FLOAT: return UniformType::FLOAT;
    case GL_INT: return UniformType::INT;
    case GL_BOOL: return UniformType::BOOL;
    case GL_FLOAT_VEC2: return UniformType::VEC2;
    case GL_FLOAT_VEC3: return UniformType::VEC3;
    case GL_FLOAT_VEC4: return UniformType::VEC4;
    case GL_FLOAT_MAT3: return UniformType::MAT3;
    case GL_FLOAT_MAT4: return UniformType::MAT4;
    case GL_SAMPLER_2D: return UniformType::SAMPLER2D;
    case GL_SAMPLER_CUBE: return UniformType::SAMPLERCUBE;
    default: return UniformType::FLOAT; // fallback
    }
}

void Shader::ApplyUniforms() {
    for (const auto& [name, LAD] : _uniformData) {
        if (!LAD.isDirty) continue;

        auto dataIt = _uniformData.find(name);

        if (dataIt == _uniformData.end()) continue;

        GLuint location = dataIt->second.location;
        const UniformValue& value = dataIt->second.value;

        std::visit([location](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, float>)
                glUniform1f(location, arg);
            else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, GLuint>)
                glUniform1i(location, arg);
            else if constexpr (std::is_same_v<T, glm::vec2>)
                glUniform2f(location, arg.x, arg.y);
            else if constexpr (std::is_same_v<T, glm::vec3>)
                glUniform3f(location, arg.x, arg.y, arg.z);
            else if constexpr (std::is_same_v<T, glm::vec4>)
                glUniform4f(location, arg.x, arg.y, arg.z, arg.w);
            else if constexpr (std::is_same_v<T, glm::mat3>)
                glUniformMatrix3fv(location, 1, GL_FALSE, &arg[0][0]);
            else if constexpr (std::is_same_v<T, glm::mat4>)
                glUniformMatrix4fv(location, 1, GL_FALSE, &arg[0][0]);
        }, value);

        dataIt->second.isDirty = false;
    }
}

inline void Shader::Use()
{
    _shaderProgram.Use();
}