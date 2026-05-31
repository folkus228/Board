#include "GLFW/glfw3.h"
#pragma once

class Time
{
public:

	static double GetDeltaTime() { return _dtime; }
	static double GetTime() { return _lastTime; }
	static void Update() 
	{
		_dtime = _lastTime;
		_lastTime = glfwGetTime();
		_dtime = _lastTime - _dtime;
	}
private:
	static inline double _dtime = 0;
	static inline double _lastTime = 0;
};
