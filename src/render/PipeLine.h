#pragma once

#include <glad/glad.h>
#include "glm/glm.hpp"

#include "../object/Scene.h"
#include "Camera2D.h"
#include "Mesh.h"
#include "Shader.h"
#include "ParticleManager.h"

#include <vector>
#include <unordered_map>
#include <algorithm>

class PipeLine;

class RenderPass
{
protected:
	virtual void Init(RenderContext& ctx) = 0;
	friend PipeLine;
public:
	virtual ~RenderPass() = default;

	virtual void Execute(RenderContext& ctx, Scene* scene, Camera2D* camera) = 0;
};

class RenderContext
{
public:

	RenderContext(GLbitfield clearMask) : _clearMask(clearMask)
	{
	}

	void SetClearFlags(GLbitfield newClearMask)
	{
		_clearMask = newClearMask;
	}
	void AddClearFlags(GLbitfield clearMask)
	{
		_clearMask |= clearMask;
	}
	void DelClearFlags(GLbitfield clearMask)
	{
		_clearMask &= ~clearMask;
	}
	void Clear()
	{
		glClear(_clearMask);
	}

	void ClearColor(float& r, float& g, float& b, float& a)
	{
		glClearColor(r, g, b, a);
	}
	void ClearColor(float&& r, float&& g, float&& b, float&& a)
	{
		glClearColor(r, g, b, a);
	}

	void Enable3DFilter()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}
	void Disable3DFilter()
	{
		glDisable(GL_DEPTH_TEST);
	}
	void EnableBlend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	void DisableBlend()
	{
		glDisable(GL_BLEND);
	}

	void ApplyTestFiches()
	{
		glEnable(GL_POINT_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_BLEND);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST); // Просим максимальное качество
		//glEnable(GL_BLEND); // Сглаживание требует прозрачности
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(5.0f);
		glPointSize(20.0f);
	}

	void Render(Scene& scene, Camera2D& camera)
	{
		glm::mat4 camera_matrix = camera.GetViewProjectionMatrix();
		std::string uniformNameBuffer = "uViewProj";
		for (auto& [shader, vec] : scene._shaders)
		{
			uniformNameBuffer = "uViewProj";
			shader->Use();
			shader->SetUniform(uniformNameBuffer, camera_matrix);
			for (auto& [obj, mesh, material, subm] : vec)
			{
				Transform2D* t = obj->GetT();
				uniformNameBuffer = "uPosition";
				shader->SetUniform("uPosition", t->GetPosition());
				uniformNameBuffer = "uRotation";
				shader->SetUniform("uRotation", t->GetRotation());
				uniformNameBuffer = "uScale";
				shader->SetUniform("uScale", t->GetScale());

				material->Bind();

				mesh->Use();
				
				mesh->Draw(subm->start, subm->count);
			}
		}
		for (auto& [shader, pd] : scene._particles)
		{
			uniformNameBuffer = "uViewProj";
			shader->Use();
			shader->SetUniform(uniformNameBuffer, camera_matrix);
			for (auto& [ps, material] : pd)
			{
				Object* obj_test = ps->GetObject();
				Transform2D* t = obj_test->GetT();
				uniformNameBuffer = "uPosition";
				shader->SetUniform("uPosition", t->GetPosition());
				uniformNameBuffer = "uRotation";
				shader->SetUniform("uRotation", t->GetRotation());
				uniformNameBuffer = "uScale";
				shader->SetUniform("uScale", t->GetScale());

				material->Bind();
				ps->Draw();
			}
		}
	}
	//void Render(Scene& scene, Camera2D& camera)
	//{
	//	glm::mat4 camera_matrix = camera.GetViewProjectionMatrix();
	//	std::string u_ModelMatrix = "u_ModelMatrix";
	//	std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";
	//	for (Object* object : scene._objects)
	//	{
	//		Material* material = object->GetComponent<Material>();
	//		if (!material) continue;
	//		Mesh* mesh = object->GetComponent<Mesh>();
	//		if (!mesh) continue;
	//
	//		material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
	//		material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
	//		material->_shader->Use();
	//		material->_shader->ApplyUniforms();
	//
	//		mesh->Use();
	//		mesh->Draw();
	//	}
	//}
	/*void Render(Scene* scene, Camera2D* camera, GLenum drawMode)
	{
		glm::mat4 camera_matrix = camera->GetViewProjectionMatrix();
		std::string u_ModelMatrix = "u_ModelMatrix";
		std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";
		for (auto& object : scene->_objects)
		{
			Material* material = object->GetComponent<Material>();
			if (!material) continue;
			Mesh* mesh = object->GetComponent<Mesh>();
			if (!mesh) continue;

			material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
			material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
			material->_shader->Use();
			material->_shader->ApplyUniforms();

			mesh->Use();
			mesh->Draw(drawMode);
		}
	}
	void Render(Scene* scene, Camera2D* camera, bool (*chek)(Object*& object))
	{
		glm::mat4 camera_matrix = camera->GetViewProjectionMatrix();
		std::string u_ModelMatrix = "u_ModelMatrix";
		std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";
		for (auto& object : scene->_objects)
		{
			if (chek(object))
			{
				Material* material = object->GetComponent<Material>();
				if (!material) continue;
				Mesh* mesh = object->GetComponent<Mesh>();
				if (!mesh) continue;

				material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
				material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
				material->_shader->Use();
				material->_shader->ApplyUniforms();

				mesh->Use();
				mesh->Draw();
			}
		}
	}
	void Render(Scene* scene, Camera2D* camera, GLenum drawMode, bool (*chek)(Object*& object))
	{
		glm::mat4 camera_matrix = camera->GetViewProjectionMatrix();
		std::string u_ModelMatrix = "u_ModelMatrix";
		std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";
		for (auto& object : scene->_objects)
		{
			if (chek(object))
			{
				Material* material = object->GetComponent<Material>();
				if (!material) continue;
				Mesh* mesh = object->GetComponent<Mesh>();
				if (!mesh) continue;

				material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
				material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
				material->_shader->Use();
				material->_shader->ApplyUniforms();

				mesh->Use();
				mesh->Draw(drawMode);
			}
		}
	}
	void Render(Object* object, Camera2D* camera)
	{
		glm::mat4 camera_matrix = camera->GetViewProjectionMatrix();
		std::string u_ModelMatrix = "u_ModelMatrix";
		std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";

		Material* material = object->GetComponent<Material>();
		if (!material) return;
		Mesh* mesh = object->GetComponent<Mesh>();
		if (!mesh) return;

		material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
		material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
		material->_shader->Use();
		material->_shader->ApplyUniforms();

		mesh->Use();
		mesh->Draw();
	}
	void Render(Object* object, Camera2D* camera, GLenum drawMode)
	{
		glm::mat4 camera_matrix = camera->GetViewProjectionMatrix();
		std::string u_ModelMatrix = "u_ModelMatrix";
		std::string u_ViewProjectionMatrix = "u_ViewProjectionMatrix";

		Material* material = object->GetComponent<Material>();
		if (!material) return;
		Mesh* mesh = object->GetComponent<Mesh>();
		if (!mesh) return;

		material->_shader->SetUniform(u_ModelMatrix, object->GetComponent<Transform>()->GetModelMatrix());
		material->_shader->SetUniform(u_ViewProjectionMatrix, camera_matrix);
		material->_shader->Use();
		material->_shader->ApplyUniforms();

		mesh->Use();
		mesh->Draw(drawMode);
	}*/

private:
	
	GLbitfield _clearMask;
};

class PipeLine
{
public:
	PipeLine(GLbitfield clearMask, RenderPass* pass = nullptr)
		: ctx(clearMask), _pass(pass) { }
	inline void SetRenderPass(RenderPass* pass) { _pass = pass; }
	inline void Start() { _pass->Init(ctx); }
	void ExecuteRender(Scene* scene, Camera2D* camera) { _pass->Execute(ctx, scene, camera); }

private:
	RenderContext ctx;
	RenderPass* _pass;
};