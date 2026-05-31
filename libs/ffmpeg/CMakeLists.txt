cmake_minimum_required(VERSION 3.19)
project(FFmpegShared)

set(FFMPEG_ROOT        "${CMAKE_CURRENT_SOURCE_DIR}")
set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")
set(FFMPEG_LIB_DIR     "${FFMPEG_ROOT}/lib")
set(FFMPEG_BIN_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/bin" CACHE INTERNAL "")

set(FFMPEG_LIBS
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    postproc
    swresample
    swscale
)

add_library(ffmpeg INTERFACE)
add_library(ffmpeg::ffmpeg ALIAS ffmpeg)

target_include_directories(ffmpeg SYSTEM INTERFACE "${FFMPEG_INCLUDE_DIR}")

foreach(LIB ${FFMPEG_LIBS})
    set(LIB_PATH "${FFMPEG_LIB_DIR}/${LIB}.lib")
    if(EXISTS "${LIB_PATH}")
        target_link_libraries(ffmpeg INTERFACE "${LIB_PATH}")
    else()
        message(WARNING "FFmpeg: не найдена библиотека ${LIB_PATH}")
    endif()
endforeach()

function(ffmpeg_copy_dlls_deferred)
    get_property(ALL_TARGETS DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TARGET ${ALL_TARGETS})
        get_target_property(TYPE ${TARGET} TYPE)
        if(TYPE STREQUAL "EXECUTABLE")
            file(GLOB FFMPEG_DLLS "${FFMPEG_BIN_DIR}/*.dll")
            # message(STATUS "FFmpeg DLLs found: ${FFMPEG_DLLS}")
            foreach(DLL ${FFMPEG_DLLS})
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL}"
                        "$<TARGET_FILE_DIR:${TARGET}>"
                )
            endforeach()
        endif()
    endforeach()
endfunction()

cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL ffmpeg_copy_dlls_deferred)