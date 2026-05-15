#pragma once
#include <unordered_map>
#include <vector>
#include "Shader.h"
#include "TextureManager.h"

class Material {
public:
    Material(Shader* shader) 
        : _shader(shader) { }
       
    void SetUniformValue(const std::string& name, UniformValue value)
    {
        _uniforms[name] = value;
    }
    void SetTextureValue(const std::string& name, GLuint texID)
    {
        _textures[name] = texID;
    }

    void Bind()
    {
        for (auto& [name, value] : _uniforms)
            _shader->SetUniform(name, value);
        int slot = 0;
        for (auto& [name, texID] : _textures) {
            GLuint tex = TextureManager::GetTexture(texID);
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, tex);
            _shader->SetUniform(name, slot);
            slot++;
        }
        _shader->Use();
        _shader->ApplyUniforms();
    }
   
private:
    Shader* _shader;
    std::unordered_map<std::string, UniformValue> _uniforms;
    std::unordered_map<std::string, GLuint> _textures;
};