#pragma once
#include "Shader.h"
#include "sys/object/Component.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <cstdint>

class Material;
class Draw3DSystem;
class RenderContext;

class MaterialController {
private:
    struct ShaderData {
        Shader* shader;
        std::unordered_set<Material*> materials;
    };
    //                          ID   shader + meterials
    inline static std::unordered_map<uint32_t, ShaderData> _shaders;
    inline static uint32_t _nextID = 1;

    static uint32_t RegisterShader(Shader* shader, Material* material)
    {
#ifdef _DEBUG
        if (!shader) return 0;
#endif

        for (auto& [id, data] : _shaders) {
            if (data.shader == shader) {
                data.materials.insert(material);
                return id;
            }
        }

        uint32_t newID = _nextID++;
        _shaders[newID].shader = shader;
        _shaders[newID].materials.insert(material);
        return newID;
    }
    static void RemoveReference(uint32_t shaderID, Material* material)
    {
        auto it = _shaders.find(shaderID);
        if (it != _shaders.end())
            it->second.materials.erase(material);

    }

    friend class Material;

public:
    static void CheckShaderUsage(uint32_t shaderID)
    {
        auto it = _shaders.find(shaderID);
        if (it != _shaders.end() && it->second.materials.size() == 0) {
            delete it->second.shader;
            _shaders.erase(it);
        }
    }

    static void CheckAllShadersUsage()
    {
        std::vector<uint32_t> toRemove;

        for (auto& [id, data] : _shaders) {
            if (data.materials.size() == 0) {
                delete data.shader;
                toRemove.push_back(id);
            }
        }

        for (uint32_t id : toRemove) {
            _shaders.erase(id);
        }
    }

    static Shader* GetShader(uint32_t shaderID)
    {
#ifdef _DEBUG
        auto it = _shaders.find(shaderID);
        return (it != _shaders.end()) ? it->second.shader : nullptr;
#else
        return _shaders.find(shaderID)->second.shader;
#endif
    }

    static uint32_t GetShaderRefCount(uint32_t shaderID)
    {
#ifdef _DEBUG
        auto it = _shaders.find(shaderID);
        return (it != _shaders.end()) ? static_cast<uint32_t>(it->second.materials.size()) : 0;
#else
        return static_cast<uint32_t>(_shaders.find(shaderID)->second.materials.size());
#endif
    }
    static size_t GetTotalShadersCount() { return _shaders.size(); }
};