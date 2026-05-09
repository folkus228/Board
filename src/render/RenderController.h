//#pragma once
//#include <vector>
//#include <unordered_map>
//#include "Shader.h"
//#include "Mesh.h"
//#include "Camera2D.h"
//#include "../object/object.h"
//using namespace std;
//
//class RenderController
//{
//public:
//	
//	void AddRenderUnit(Object* obj, Mesh* mesh, Shader* shader)
//	{
//		_shaders[shader].push_back({ obj, mesh });
//	}
//	void Render(Camera2D camera)
//	{
//		glm::mat4 camera_matrix = camera.GetViewProjectionMatrix();
//		for ( auto& [shader, vec] : _shaders)
//		{
//			shader->Use();
//			shader->SetUniform("uViewProj", camera_matrix);
//			for (auto& [obj, mesh] : vec)
//			{
//				shader->SetUniform("uPosition", obj->t.GetPosition());
//				shader->SetUniform("uRotation", obj->t.GetRotation());
//				shader->SetUniform("uScale", obj->t.GetScale());
//				shader->ApplyUniforms();
//
//				mesh->Use();
//				mesh->Draw();
//			}
//		}
//	}
//
//
//private:
//
//	// для каждого шейдера своя пачка мешей
//	unordered_map<Shader*, vector<pair<Object*, Mesh*>>> _shaders;
//};
//
//
//
