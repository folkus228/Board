#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "../object/Component.h"
#include "Shader.h"
#include "../ModuleTime/Time.h"

struct Particle
{
	glm::vec2 position;
	glm::vec2 velocity;
	float life;
	float maxLife;
};

class ParticleSystem : public Component
{
public:
	ParticleSystem(Object* obj, int maxParticles, float maxLifeTime, glm::vec2 velocity, std::string pathToComputeShader)
		: Component(obj),
		_templateParticle({
			{0.f, 0.f}, 
			velocity,
			0.f,
			maxLifeTime
		}),
		_maxParticles(maxParticles)
	{	
		float quad[] = {
				-0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f,  0.0f, 1.0f
		};
		GLuint indices[] = { 0,1,2, 0,2,3 };

		glGenVertexArrays(1, &_VAO);
		glGenBuffers(1, &_VBO);
		glGenBuffers(1, &_EBO);

		glBindVertexArray(_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glBufferData(GL_ARRAY_BUFFER, 64, quad, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24, indices, GL_STATIC_DRAW);
		glGenBuffers(1, &_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO); // binding point 0
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		_computeShader.LoadShaderFromFile(GL_COMPUTE_SHADER, pathToComputeShader);
		_computeShader.CompileProgram();
		_computeShader.SetUniform("gravity", glm::vec2{0.0f, -1.5f});
		_computeShader.SetUniform("turbulence", 10.0f);
	}

	void Update() override
	{
		if (_systemIsLive) {
			_computeShader.Use();
			std::string dt = "dt";
			_computeShader.SetUniform(dt, (float)Time::GetDeltaTime());
			_computeShader.ApplyUniforms();
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
			glDispatchCompute(ceil(_maxParticles / 256.0f), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // vait
		}
	}

	void Emit()
	{
		_particles.assign(_maxParticles, _templateParticle);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, _SSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _maxParticles * sizeof(Particle), _particles.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		_systemIsLive = true;
	}

	void Draw() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _SSBO);
		glBindVertexArray(_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, _maxParticles);
	}

private:
	std::vector<Particle> _particles;
	Shader _computeShader;
	Particle _templateParticle;
	GLuint _VAO, _VBO, _EBO, _SSBO;
	int _maxParticles;
	bool _systemIsLive = false;
};
