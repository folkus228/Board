# CMakeList.txt: проект CMake для Board; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Board")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Board "src/Board.cpp")


add_subdirectory(libs/glad)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
add_subdirectory(libs/imgui)
add_subdirectory(src/render)
add_subdirectory(src/object)
add_subdirectory(src/helpers)


target_link_libraries(Board PRIVATE glfw glm glad imgui module_render)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(module_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
