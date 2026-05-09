
project("module_helpers")

add_library(module_helpers STATIC
            tools.cpp
)

set(CMAKE_CXX_STANDARD 20)

target_include_directories(module_helpers PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(module_helpers PRIVATE glad) # только ради путей
target_link_libraries(module_helpers PRIVATE glfw) # только ради путей


