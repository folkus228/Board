#pragma once

#include "glad/glad.h"
#include <vector>
#include "../object/Component.h"

class Mesh : public Component
{
public:
    Mesh(Object* obj, GLenum drawType, int elementCount) 
        : Component(obj), _drawType(drawType), _elementCount(elementCount)
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

    void AllocateData(const void* data, int byteSize, GLenum usage = GL_STATIC_DRAW)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, usage);
    }
    void AllocateIndices(const void* elementsData, GLenum usage = GL_STATIC_DRAW)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _elementCount * sizeof(unsigned int), elementsData, usage);
    }

    void UpdateData(const void* data, GLsizeiptr byteSize, GLintptr offset)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
    }
    void UpdateIndices(const void* data, GLsizeiptr byteSize, GLintptr offset)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
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
        glDrawElements(_drawType, _elementCount, GL_UNSIGNED_INT, 0);
    }
    inline void SetElentsCount(int elementsCount) { _elementCount = elementsCount; }
private:
    GLenum _drawType;
    int _elementCount;
    GLuint _VAO, _VBO, _EBO;
};