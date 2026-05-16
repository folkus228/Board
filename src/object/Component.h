#pragma once

class Object;

class Component
{
public:
	Component(Object* obj)
	{
	}
	virtual ~Component() { }
	
	inline Object* GetObject() { return _obj; }

	virtual void Start() {}
	virtual void Update() {}
	virtual void OnDestroy() {}

private:
	Object* _obj;
};
