#pragma once
#include "Transform2D.h"


class Object
{
public:
    Object(Transform2D transform) : t(transform) {}
    virtual ~Object() {};

    virtual void Start() {};
    virtual void Update() {};
    
    Transform2D t;
private:
};