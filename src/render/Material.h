#pragma once
#include <unordered_map>
#include <vector>
#include "Shader.h"
#include "TextureManager.h"
#include "../object/Component.h"

class Scene;

class Material : public Component {
public:
    Material(Object* obj, Shader* shader) 
        : Component(obj), _shader(shader) { }
       
    template<typename T>
    void SetUniformValue(const std::string& name, T value)
    {
        _uniforms[name] = value;
    }
    void SetTextureValue(const std::string& name, unsigned int texID)
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
   
    Shader* _shader;
private:
    std::unordered_map<std::string, UniformValue> _uniforms;
    std::unordered_map<std::string, unsigned int> _textures;
    friend Scene;
};