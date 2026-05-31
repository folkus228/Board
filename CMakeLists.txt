# CMakeList.txt: проект CMake для Board; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()



project ("Board")

add_subdirectory(libs/glad)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
add_subdirectory(libs/imgui)
add_subdirectory(libs/freetype)
add_subdirectory("libs/ffmpeg" "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg")
add_subdirectory(src/render)
add_subdirectory(src/object)
add_subdirectory(src/helpers)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Board "src/Board.cpp" "src/font/FontManager.h"  "src/object/Component.h" "src/font/FontComponent.h" "src/render/VideoManager.h" "src/ModuleTime/Time.h")

target_link_libraries(Board PRIVATE 
					glfw 
					glm 
					glad 
					imgui
					freetype
					ffmpeg::ffmpeg
					module_render		
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(module_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
