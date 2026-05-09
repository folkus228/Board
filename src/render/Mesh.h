#pragma once

#include "glad/glad.h"
#include <vector>

class Mesh
{
public:


    Mesh(GLenum drawType, int elementCount) 
        : _drawType(drawType), _elementCount(elementCount)
    {
        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);
    }
    ~Mesh() 
    {
        std::cout << "Mesh Destructor is call" << std::endl;
        glDeleteVertexArrays(1, &_VAO);
        glDeleteBuffers(1, &_EBO);
        glDeleteBuffers(1, &_VBO);
    }

    void AddData(const void* data, int byteSize)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
    }

    void AddIndices(const void* elementsData)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _elementCount * sizeof(unsigned int), elementsData, GL_STATIC_DRAW);
    }

    template<typename... Args>
    void Compile(GLuint shaderIdx, GLint elementSize, GLenum valueType, GLboolean normalized, GLsizei stride, const void* offset, Args... params)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glVertexAttribPointer(shaderIdx, elementSize, valueType, normalized, stride, offset);
        glEnableVertexAttribArray(shaderIdx);
        if constexpr (sizeof...(params) > 0)
            Compile(params...);
    }

    inline void Use()
    {
        glBindVertexArray(_VAO);
    }

    inline void Draw()
    {
        // for check the operation of EBO
        //GLint currentEBO;
        //glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentEBO);
        //std::cout << "Active EBO: " << currentEBO << " (Expected: " << _EBO << ")" << std::endl;
        glDrawElements(_drawType, _elementCount, GL_UNSIGNED_INT, 0);
    }

private:
    GLenum _drawType;
    int _elementCount;
    GLuint _VAO, _VBO, _EBO;
};