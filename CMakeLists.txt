cmake_minimum_required(VERSION 4.2)
project(probe_open_gl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(probe_open_gl main.cpp)

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
