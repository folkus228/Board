#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../object/Component.h"

class Camera2D : public Component
{
public:
    Camera2D(Object* obj, float orthoSize = 10.0f, float nearPlane = -1.0f, float farPlane = 100.0f, float aspectRatio = 16.0f / 9.0f)
        : Component(obj), _transform(obj->GetT()), _orthoSize(orthoSize), _nearPlane(nearPlane), _farPlane(farPlane), _aspectRatio(aspectRatio)
    {
    }
    glm::mat4 GetViewMatrix() const {
        glm::vec3 pos = _transform->GetPosition();
        float angle = _transform->GetRotation();

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, -angle, glm::vec3(0, 0, 1));
        view = glm::translate(view, -pos);

        return view;
    }
    glm::mat4 GetProjectionMatrix() const {
        if (_isProjectionDirty) {
            float halfHeight = _orthoSize * 0.5f;
            float halfWidth = halfHeight * _aspectRatio;

            _projectionMatrix = glm::ortho(
                -halfWidth, halfWidth,    // left, right
                -halfHeight, halfHeight,  // bottom, top
                _nearPlane, _farPlane     // near, far
            );
            _isProjectionDirty = false;
        }
        return _projectionMatrix;
    }

    inline void SetOrthoSize(float orthoSize)           { _orthoSize = orthoSize; _isProjectionDirty = true; }
    inline void SetAspectRatio(float aspectRatio)       { _aspectRatio = aspectRatio; _isProjectionDirty = true; }
    inline float GetNearPlane() const                   { return _nearPlane; _isProjectionDirty = true; }
    inline float GetFarPlane() const                    { return _farPlane; _isProjectionDirty = true; }
    inline float GetAspectRatio() const                 { return _aspectRatio; }
    inline float GetOrthoSize() const                   { return _orthoSize; }
    inline glm::mat4 GetViewProjectionMatrix() const    { return GetProjectionMatrix() * GetViewMatrix(); }

    inline void SetNearPlane(float nearPlane)           { _nearPlane = nearPlane; }
    inline void SetFarPlane(float farPlane)             { _farPlane = farPlane; }

private:
    Transform2D* _transform;

    float _orthoSize;
    float _aspectRatio;
    float _nearPlane;
    float _farPlane;

    mutable glm::mat4 _projectionMatrix{ 1.0f };
    mutable bool _isProjectionDirty = true;
};