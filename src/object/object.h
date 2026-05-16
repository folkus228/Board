#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <span>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <assert.h>
#include "Component.h"
#include "Transform2D.h"


class Object
{
public:

    Object()
    {
        _transform = AddComponent<Transform2D>();
    }
    virtual ~Object()
    {
        for (auto& [type, component] : _components) {
            component->OnDestroy();
        }
        for (auto& [type, component] : _components) {
            delete component;
        }
    }

    template<typename T>
    T* GetComponent() const
    {
        auto it = _components.find(std::type_index(typeid(T)));
        if (it != _components.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    //template<typename T>
    //T* GetComponent(unsigned int id) const
    //{
    //    auto [begin, end] = _components.equal_range(std::type_index(typeid(T)));
    //    for (auto it = begin; it != end; ++it)
    //    {
    //        if (it->second->GetID() == id) {
    //            return static_cast<T*>(it->second);
    //        }
    //    }
    //    return nullptr;
    //}

    template<typename T>
    void GetComponents(std::vector<T*>& components) const
    {
        auto [begin, end] = _components.equal_range(std::type_index(typeid(T)));
        for (auto it = begin; it != end; ++it) {
            components.push_back(static_cast<T*>(it->second));
        }
    }

    template<typename T, typename Func>
    void ForEachComponent(Func callback) const
    {
        auto [begin, end] = _components.equal_range(std::type_index(typeid(T)));
        for (auto it = begin; it != end; ++it) {
            callback(static_cast<T*>(it->second));
        }
    }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
#ifdef _DEBUG
        if (std::is_same_v<T, Transform2D> && hasTransform)
        {
            assert(false && "Cannot add multiple Transform2D components!");
        }
        if (std::is_same_v<T, Transform2D>)
        {
            hasTransform = true;
        }
#endif

        auto component = new T(this, std::forward<Args>(args)...);
        _components.emplace(std::type_index(typeid(T)), component);
        return component;
    }

    template<typename T>
    void AddReadyComponent(T* newComponent)
    {
#ifdef _DEBUG
        if (std::is_same_v<T, Transform2D> && hasTransform)
        {
            assert(false && "Cannot add multiple Transform2D components!");
        }
        if (std::is_same_v<T, Transform2D>)
        {
            hasTransform = true;
        }
#endif

        _components.emplace(std::type_index(typeid(T)), newComponent);
    }

    template<typename T>
    bool DelComponent()
    {
#ifdef _DEBUG
        if constexpr (std::is_same_v<T, Transform2D>)
        {
            assert(false && "Cannot delete Transform2D component!");
        }
#endif
        auto it = _components.find(std::type_index(typeid(T)));
        if (it != _components.end()) {
            delete it->second;
            _components.erase(it);
            return true;
        }
        return false;
    }
//    template<typename T>
//    bool DelComponent(unsigned int id)
//    {
//#ifdef _DEBUG
//        if constexpr (std::is_same_v<T, Transform2D>)
//        {
//            assert(false && "Cannot delete Transform2D component!");
//        }
//#endif
//        auto [begin, end] = _components.equal_range(std::type_index(typeid(T)));
//        for (auto it = begin; it != end; ++it) {
//            if (it->second->GetID() == id) {
//                delete it->second;
//                _components.erase(it);
//                return true;
//            }
//        }
//        return false;
//    }

    template<typename T>
    void DelComponents()
    {
#ifdef _DEBUG
        if constexpr (std::is_same_v<T, Transform2D>)
        {
            assert(false && "Cannot delete Transform2D component!");
        }
#endif
        auto [begin, end] = _components.equal_range(std::type_index(typeid(T)));
        for (auto it = begin; it != end; ++it) {
            delete it->second;
        }
        _components.erase(std::type_index(typeid(T)));
    }

    template<typename T>
    bool HasComponent() const
    {
        return _components.find(std::type_index(typeid(T))) != _components.end();
    }

    template<typename T>
    size_t GetComponentCount() const
    {
        return _components.count(std::type_index(typeid(T)));
    }


    void Start()
    {
        for(auto& [tidx, component] : _components)
            component->Start();
    }
    void Update()
    {
        for (auto& [tidx, component] : _components)
            component->Update();
    }
    void OnDestroy()
    {
        for (auto& [tidx, component] : _components)
            component->OnDestroy();
    }

    Transform2D* GetT() { return _transform; }

private:
    Transform2D* _transform;
    std::unordered_multimap<std::type_index, Component*> _components;
#ifdef _DEBUG
    bool hasTransform = false;
#endif
};