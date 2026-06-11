
project("module_render")


set(CMAKE_CXX_STANDARD 20)
add_library(module_render STATIC
			"Shader.cpp"
			"ShaderProgram.cpp"
    "Material.h" "TextureManager.h" "ParticleManager.h" "WindowsManager.h")
target_link_libraries(module_render PRIVATE glad)
target_link_libraries(module_render PRIVATE glm::glm)
target_link_libraries(module_render PRIVATE module_helpers)