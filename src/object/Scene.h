#pragma once
#include <vector>
#include "object.h"
#include "../render/Mesh.h"
#include "../render/Material.h"
#include "../render/ParticleManager.h"

class RenderPass;
class RenderContext;

struct RenderData
{
	Object* obj;
	Mesh* mesh; 
	Material* material;
	SubMeshData* subm;
};

struct ParticleData
{
	ParticleSystem* ps;
	Material* material;
};


class Scene
{
public:
	void AddRenderUnit(Object* obj, Mesh* mesh, Material* material, SubMeshData* subm)
	{
		_shaders[material->_shader].push_back({ obj, mesh, material, subm });
	}
	void AddObj(Object* obj)
	{
		_objects.push_back(obj);
		if (obj->HasComponent<Mesh>())
		{
			Mesh* mesh = obj->GetComponent<Mesh>();
			for (auto& submd : mesh->_subMeshes)
			{
				AddRenderUnit(obj, mesh, submd.mat, &submd);
			}
		}
		if (obj->HasComponent<ParticleSystem>())
		{
			ParticleSystem* ps = obj->GetComponent<ParticleSystem>();
			Material* mat = obj->GetComponent<Material>();
			_particles[mat->_shader].push_back({ ps, mat });
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
	std::unordered_map<Shader*, std::vector<ParticleData>> _particles;
private:
	std::vector<Object*> _objects;
	friend RenderPass;
	friend RenderContext;
};