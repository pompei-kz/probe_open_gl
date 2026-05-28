cmake_minimum_required(VERSION 4.2)
project(probe_open_gl)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RESOURCE_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
)
set(FILES_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/files
)

file(GLOB RESOURCE_SOURCES
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    ${RESOURCE_DIRECTORY}/*
)
file(GLOB_RECURSE RUNTIME_FILES
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    ${FILES_DIRECTORY}/*
)

add_executable(probe_open_gl src/main.cpp)

target_sources(probe_open_gl
    PRIVATE
    FILE_SET CXX_MODULES
    FILES
    src/application.cppm
    src/arguments.cppm
    src/deferred_tasks.cppm
    src/main_window.cppm
    src/render.cppm
    src/scene.cppm
    src/sync_map.cppm
    src/utils.cppm
)

set(GENERATED_RESOURCE_HEADER
    ${CMAKE_CURRENT_BINARY_DIR}/generated/resources.hpp
)

add_custom_command(
    OUTPUT ${GENERATED_RESOURCE_HEADER}
    COMMAND ${CMAKE_COMMAND}
    "-DOUTPUT=${GENERATED_RESOURCE_HEADER}"
    "-DRESOURCE_DIRECTORY=${RESOURCE_DIRECTORY}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake
    ${RESOURCE_SOURCES}
    COMMENT "Embedding resources"
    VERBATIM
)

add_custom_target(embedded_resources DEPENDS ${GENERATED_RESOURCE_HEADER})
add_dependencies(probe_open_gl embedded_resources)

target_sources(probe_open_gl
    PRIVATE
    ${RESOURCE_SOURCES}
)

target_include_directories(probe_open_gl
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

add_custom_target(copy_runtime_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${FILES_DIRECTORY}
    $<TARGET_FILE_DIR:probe_open_gl>
    DEPENDS ${RUNTIME_FILES}
    COMMENT "Copying runtime files"
    VERBATIM
)
add_dependencies(probe_open_gl copy_runtime_files)

find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)

target_link_libraries(probe_open_gl
    PRIVATE
    OpenGL::GL
    glm::glm
    yaml-cpp::yaml-cpp
    PkgConfig::SDL2
    PkgConfig::EPOXY
)

add_executable(scene_tests tests/scene_tests.cpp)

target_sources(scene_tests
    PRIVATE
    FILE_SET CXX_MODULES
    FILES
    src/scene.cppm
)

target_link_libraries(scene_tests
    PRIVATE
    yaml-cpp::yaml-cpp
    PkgConfig::EPOXY
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scene_tests)

add_executable(arguments_tests tests/arguments_tests.cpp)

target_sources(arguments_tests
    PRIVATE
    FILE_SET CXX_MODULES
    FILES
    src/arguments.cppm
)

target_link_libraries(arguments_tests
    PRIVATE
    GTest::gtest_main
)

gtest_discover_tests(arguments_tests)

add_executable(deferred_tasks_tests tests/deferred_tasks_tests.cpp)

target_sources(deferred_tasks_tests
    PRIVATE
    FILE_SET CXX_MODULES
    FILES
    src/sync_map.cppm
    src/deferred_tasks.cppm
)

target_link_libraries(deferred_tasks_tests
    PRIVATE
    GTest::gtest_main
)

gtest_discover_tests(deferred_tasks_tests)

add_executable(sync_map_tests tests/sync_map_tests.cpp)

target_sources(sync_map_tests
    PRIVATE
    FILE_SET CXX_MODULES
    FILES
    src/sync_map.cppm
)

target_link_libraries(sync_map_tests
    PRIVATE
    GTest::gtest_main
)

gtest_discover_tests(sync_map_tests)
