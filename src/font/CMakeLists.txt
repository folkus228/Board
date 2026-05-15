

project("module_font")


set(CMAKE_CXX_STANDARD 20)

# add_library(module_font STATIC
#             imgui.cpp
# )

#target_include_directories(imgui PUBLIC 
#    ${CMAKE_CURRENT_SOURCE_DIR}
#    ${CMAKE_CURRENT_SOURCE_DIR}/backends
#)

target_link_libraries(module_font PRIVATE freetype) # только ради путей

