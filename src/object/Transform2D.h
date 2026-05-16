#pragma once
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "../basetools/EventSystem.h"

constexpr float PI2 = 6.28318530718f;

class Transform2D : public Component
{
public:

    Transform2D(Object* obj, 
        glm::vec3 position = { 0.0f, 0.0f, 0.0f },
        float rotation = 0.0f,
        glm::vec2 scale = { 1.0f, 1.0f })
        : Component(obj), _localPosition(position), _localRotation(glm::radians(rotation)), _localScale(scale) { }   

    // Локальные данные
    glm::vec3 GetLocalPosition() const { return _localPosition; }
    float GetLocalRotation() const { return glm::degrees(_localRotation); }
    glm::vec2 GetLocalScale() const { return _localScale; }

    void SetLocalPosition(glm::vec3 newPosition)
    {
        _localPosition = newPosition;
        _eventSetPos.emit();
    }
    void SetLocalRotation(float newRotation)
    {
        _localRotation = std::fmod(glm::radians(newRotation), PI2);
        _eventSetRot.emit();
    }
    void SetLocalScale(glm::vec2 newScale)
    {
        _localScale = newScale;
        _eventSetScale.emit();
    }

    float GetLocalPX() const { return _localPosition.x; }
    float GetLocalPY() const { return _localPosition.y; }
    float GetLocalPZ() const { return _localPosition.z; }

    float GetLocalSX() const { return _localScale.x; }
    float GetLocalSY() const { return _localScale.y; }

    void SetLocalPX(float newX)
    {
        _localPosition.x = newX;
        _eventSetPos.emit();
    }
    void SetLocalPY(float newY)
    {
        _localPosition.y = newY;
        _eventSetPos.emit();
    }
    void SetLocalPZ(float newZ)
    {
        _localPosition.z = newZ;
        _eventSetPos.emit();
    }

    void SetLocalPXY(float newX, float newY)
    {
        _localPosition.x = newX;
        _localPosition.y = newY;
        _eventSetPos.emit();
    }
    void SetLocalPXY(glm::vec2 newPos)
    {
        _localPosition.x = newPos.x;
        _localPosition.y = newPos.y;
        _eventSetPos.emit();
    }
    glm::vec2 GetLocalPXY()
    {
        return { _localPosition.x, _localPosition.y };
    }

    void SetLocalR(float newR)
    {
        float currentX = glm::degrees(_localRotation);
        _localRotation = glm::radians(newR - currentX);
        _eventSetRot.emit();
    }

    void SetLocalSX(float newX)
    {
        _localScale.x = newX;
        _eventSetScale.emit();
    }
    void SetLocalSY(float newY)
    {
        _localScale.y = newY;
        _eventSetScale.emit();
    }

    // Глобальные данные
    float GetRotation() const {
        if (_parent) return _parent->GetRotation() + _localRotation;
        return _localRotation;
    }
    glm::vec2 GetScale() const {
        if (_parent) return _parent->GetScale() * _localScale;
        return _localScale;
    }
    glm::vec3 GetPosition() const {
        if (_parent) {
            float pAngle = _parent->GetRotation();
            float s = std::sin(pAngle);
            float c = std::cos(pAngle);

            // Ручной поворот вектора _localPosition.xy на угол pAngle
            float rx = _localPosition.x * c - _localPosition.y * s;
            float ry = _localPosition.x * s + _localPosition.y * c;

            return _parent->GetPosition() + glm::vec3(rx, ry, _localPosition.z);
        }
        return _localPosition;
    }

    void SetWorldRotation(float degrees) {
        float worldRad = glm::radians(degrees);
        if (_parent) {
            _localRotation = worldRad - _parent->GetRotation();
        }
        else {
            _localRotation = worldRad;
        }
    }
    void SetWorldScale(glm::vec2 scale) {
        if (_parent) {
            glm::vec2 pScale = _parent->GetScale();
            _localScale.x = (pScale.x != 0) ? scale.x / pScale.x : 0;
            _localScale.y = (pScale.y != 0) ? scale.y / pScale.y : 0;
        }
        else {
            _localScale = scale;
        }
    }
    void SetWorldPosition(glm::vec3 pos) {
        if (_parent) {
            glm::vec3 worldOffset = pos - _parent->GetPosition();

            float invAngle = -_parent->GetRotation();
            float s = std::sin(invAngle);
            float c = std::cos(invAngle);

            float lx = worldOffset.x * c - worldOffset.y * s;
            float ly = worldOffset.x * s + worldOffset.y * c;

            _localPosition = glm::vec3(lx, ly, worldOffset.z);
        }
        else {
            _localPosition = pos;
        }
    }

    // Вложенность
    void SetParent(Transform2D* parent)
    {
        // Убираем из дочерних родителя
        if (_parent)
            _parent->DelChildren(this);

        if (parent)
        {
            _parent = parent;
            parent->AddChildren(this);
        }
        else
            parent = nullptr;
    }

    // Вспомогательные 
    inline void Rotate(float angleDegrees) {
        SetLocalRotation(_localRotation + glm::radians(angleDegrees));
    }
    void LookAt(const glm::vec2& target) {
        glm::vec3 worldPos = GetPosition();

        float dx = target.x - worldPos.x;
        float dy = target.y - worldPos.y;

        // Проверяем, не слишком ли близко цель, чтобы избежать дергания
        if (std::abs(dx) < 0.0001f && std::abs(dy) < 0.0001f) {
            return;
        }

        SetWorldRotation(glm::degrees(std::atan2(dy, dx)));
    }
    void LookAt(const glm::vec2& target, float angleOffsetDegrees) {
        glm::vec3 worldPos = GetPosition();

        float dx = target.x - worldPos.x;
        float dy = target.y - worldPos.y;

        // Проверяем, не слишком ли близко цель, чтобы избежать дергания
        if (std::abs(dx) < 0.0001f && std::abs(dy) < 0.0001f) {
            return;
        }

        float finalAngle = std::atan2(dy, dx) + glm::radians(angleOffsetDegrees);
        SetWorldRotation(glm::degrees(finalAngle));
    }

    Event<> _eventSetPos;
    Event<> _eventSetRot;
    Event<> _eventSetScale;
private:

    void DelChildren(Transform2D* children)
    {
        auto it = std::find(_childrens.begin(), _childrens.end(), children);
#ifdef _DEBUG
        if (it == _childrens.end())
        {
            std::cerr << "Children not found" << std::endl;
            throw std::out_of_range("Children not found");
        }
#endif
        _childrens.erase(it);
    }
    void AddChildren(Transform2D* children)
    {
#ifdef _DEBUG
        auto it = std::find(_childrens.begin(), _childrens.end(), children);
        if (it != _childrens.end())
        {
            std::cerr << "Children is already in childrens" << std::endl;
            throw std::out_of_range("Children is already in childrens");
        }
#endif
        _childrens.push_back(children);
    }

    glm::vec3 _localPosition;
    float _localRotation;
    glm::vec2 _localScale;

    Transform2D* _parent = nullptr;
    std::vector<Transform2D*> _childrens;
};