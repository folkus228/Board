
project("module_io")


set(CMAKE_CXX_STANDARD 20)
add_library(module_io STATIC
			"IOManager.cpp")
target_link_libraries(module_io PRIVATE glfw)