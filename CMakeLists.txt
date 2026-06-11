# CMakeList.txt: проект CMake для Board; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_INSTALLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg_libs") # куда класть либы
set(HB_HAVE_FREETYPE ON CACHE BOOL "" FORCE)
set(HB_HAVE_ICU ON CACHE BOOL "" FORCE)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
project ("Board")

find_package(ICU REQUIRED COMPONENTS uc i18n dt)
find_package(FFMPEG REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(libs/glad)
add_subdirectory(src/render)
add_subdirectory(src/object)
add_subdirectory(src/helpers)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Board "src/Board.cpp" "src/font/FontManager.h"  "src/object/Component.h" "src/font/FontComponent.h" "src/render/VideoManager.h" "src/ModuleTime/Time.h")

target_link_libraries(Board PRIVATE 
    glfw 
    glad 
    imgui::imgui
    glm::glm
    freetype    
    ${FFMPEG_INCLUDE_DIRS}
    ${FFMPEG_LIBRARIES}
    ${ICU_INCLUDE_DIRS}
    ICU::uc
    ICU::i18n
    ICU::dt
    harfbuzz
    module_render		
)

target_include_directories(module_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})