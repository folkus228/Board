

#include <glad/glad.h>
#include "PipeLine.h"
#include "Camera2D.h"
#include <iostream>
#include <vector>



class WindowsManager
{
public:
	struct WindowsRender
	{
		PipeLine* pipeline;
		Scene* scene;
		Camera2D* camera;
	};

	WindowsManager(GLFWwindow*& window, int width, int height, const char* title, int versionMajor = 3, int versionMinor = 3, int profile = GLFW_OPENGL_CORE_PROFILE)
	{
		if (!glfwInit()) {
			std::cerr << "Не удалось инициализировать GLFW" << std::endl;
			return;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, versionMajor);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, versionMinor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, profile);

		window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (!window) {
			std::cerr << "Не удалось создать первое окно" << std::endl;
			glfwTerminate();
			return;
		}

		// 3. Делаем контекст первого окна текущим
		glfwMakeContextCurrent(window);

		// 4. ИНИЦИАЛИЗАЦИЯ GLAD (Строго после glfwMakeContextCurrent!)
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cerr << "Не удалось инициализировать GLAD" << std::endl;
			glfwDestroyWindow(window);
			glfwTerminate();
			return;
		}
		_windowses.push_back(window);
	}

	~WindowsManager()
	{
		for (auto& window : _windowses)
			glfwDestroyWindow(window);
		glfwTerminate();
	}

	void AddWindow(GLFWwindow*& window, int width, int height, const char* title, GLFWwindow* shaderWindow)
	{
		window = glfwCreateWindow(width, height, title, NULL, shaderWindow);
		if (!window) {
			std::cerr << "Не удалось создать второе окно" << std::endl;
			glfwDestroyWindow(shaderWindow);
			glfwTerminate();
			return;
		}
		_windowses.push_back(window);
	}

	inline void SetPos(GLFWwindow* window, int x, int y) const {
		glfwSetWindowPos(window, x, y);
	}

	inline void AddUpdateFunc(void(*func)()) { _updates.push_back(func); }

	
	inline void AddRenderData(GLFWwindow* window, PipeLine* pipeline, Scene* scene, Camera2D* camera)
	{
		_renderData[window] = { pipeline, scene, camera };
		_scens.push_back(scene);
	}

	void Run()
	{
		while (std::all_of(_windowses.begin(), _windowses.end(),
			[](GLFWwindow* w) { return !glfwWindowShouldClose(w); })) {

			glfwPollEvents();
			for (auto& func : _updates)
				func();
			for (auto& scene : _scens)
				scene->Update();
			for (int i = 0; i < _windowses.size(); i++)
			{
				GLFWwindow* window = _windowses[i];
				glfwMakeContextCurrent(window);
				WindowsRender* wr = &_renderData[window];
				wr->pipeline->ExecuteRender(wr->scene, wr->camera);
				glfwSwapBuffers(window);
			}
		}
	}

private:

	std::unordered_map<GLFWwindow*, WindowsRender> _renderData;
	std::vector<Scene*> _scens;
	std::vector<GLFWwindow*> _windowses;
	std::vector<void(*)()> _updates;
};
