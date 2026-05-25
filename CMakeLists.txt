cmake_minimum_required(VERSION 4.2)
project(probe_open_gl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(probe_open_gl main.cpp)

set(SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag
)
set(GENERATED_SHADER_HEADER
        ${CMAKE_CURRENT_BINARY_DIR}/generated/shader_resources.hpp
)

add_custom_command(
        OUTPUT ${GENERATED_SHADER_HEADER}
        COMMAND ${CMAKE_COMMAND}
                "-DOUTPUT=${GENERATED_SHADER_HEADER}"
                "-DVERTEX_SHADER=${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.vert"
                "-DFRAGMENT_SHADER=${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag"
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
        DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
                ${SHADER_SOURCES}
        COMMENT "Embedding shader resources"
        VERBATIM
)

add_custom_target(shader_resources DEPENDS ${GENERATED_SHADER_HEADER})
add_dependencies(probe_open_gl shader_resources)

target_sources(probe_open_gl
        PRIVATE
        ${SHADER_SOURCES}
)

target_include_directories(probe_open_gl
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

target_link_libraries(probe_open_gl
        PRIVATE
        OpenGL::GL
        PkgConfig::SDL2
        PkgConfig::EPOXY
)
