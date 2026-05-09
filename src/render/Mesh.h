#pragma once

#include "glad/glad.h"
#include <vector>


struct MeshData
{
    MeshData(GLenum _drawType, int _elementCount) 
        :drawType(_drawType), elementCount(_elementCount)
    {
        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);
    }
    ~MeshData()
    {
        glDeleteVertexArrays(1, &_VAO);
        glDeleteBuffers(1, &_EBO);
        glDeleteBuffers(1, &_VBO);
    }
    GLenum drawType;
    int elementCount;
    GLuint _VAO, _VBO, _EBO;
};

class Mesh
{
public:


    Mesh(GLenum drawType, int elementCount, const void* elementsData) 
        : meshData(drawType, elementCount) 
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData._EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementCount * sizeof(unsigned int), elementsData, drawType);
    }
    ~Mesh() { }

    void AddData(const void* data, int byteSize)
    {
        glBindBuffer(GL_ARRAY_BUFFER, meshData._VBO);
        glBufferData(GL_ARRAY_BUFFER, byteSize, data, meshData.drawType);
    }

    inline void PreCompile()
    {
        glBindBuffer(GL_ARRAY_BUFFER, meshData._VBO);
    }
    template<typename... Args>
    void Compile(GLuint shaderIdx, GLint elementSize, GLenum valueType, GLboolean normalized, GLsizei stride, const void* offset, Args... params)
    {

        glVertexAttribPointer(shaderIdx, elementSize, valueType, normalized, stride, offset);
        glEnableVertexAttribArray(shaderIdx);
        if constexpr (sizeof...(params) > 0)
            Compile(params...);
    }

    inline void Use()
    {
        glBindVertexArray(meshData._VAO);
    }

    inline void Draw()
    {
        glDrawElements(meshData.drawType, meshData.elementCount, GL_UNSIGNED_INT, 0);
    }

private:
    MeshData meshData;
};