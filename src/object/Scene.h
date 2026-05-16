#pragma once
#include <vector>
#include "object.h"
#include "../render/Mesh.h"
#include "../render/Material.h"

class RenderPass;
class RenderContext;

struct RenderData
{
	Object* obj;
	Mesh* mesh; 
	Material* material;
};

class Scene
{
public:
	void AddRenderUnit(Object* obj, Mesh* mesh, Material* material)
	{
		_shaders[material->_shader].push_back({ obj, mesh, material });
	}
	void AddObj(Object* obj)
	{
		_objects.push_back(obj);
		if (obj->HasComponent<Mesh>())
		{
			Material* mat = obj->GetComponent<Material>();
			Mesh* mesh = obj->GetComponent<Mesh>();
			_shaders[mat->_shader].push_back({ obj, mesh, mat });
		}
	}
	void DelObj(Object* obj)
	{
		auto it = std::find(_objects.begin(), _objects.end(), obj);
		if (it != _objects.end()) {
			_objects.erase(it);
		}
	}

	void Start()
	{
		for (Object* obj : _objects)
			obj->Start();
	}
	void Update()
	{
		for (Object* obj : _objects)
			obj->Update();
	}

	std::unordered_map<Shader*, std::vector<RenderData>> _shaders;
private:
	std::vector<Object*> _objects;
	friend RenderPass;
	friend RenderContext;
};