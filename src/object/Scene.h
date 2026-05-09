#pragma once
#include <vector>
#include "object.h"
#include "../render/Mesh.h"
#include "../render/Shader.h"

class RenderPass;
class RenderContext;

class Scene
{
public:
	void AddRenderUnit(Object* obj, Mesh* mesh, Shader* shader)
	{
		_shaders[shader].push_back({ obj, mesh });
	}
	void AddObj(Object* obj)
	{
		_objects.push_back(obj);
	}
	void DelObj(Object* obj)
	{
		auto it = std::find(_objects.begin(), _objects.end(), obj);
		if (it != _objects.end()) {
			_objects.erase(it);
		}
	}

	std::unordered_map<Shader*, std::vector<std::pair<Object*, Mesh*>>> _shaders;
private:
	std::vector<Object*> _objects;
	friend RenderPass;
	friend RenderContext;
};