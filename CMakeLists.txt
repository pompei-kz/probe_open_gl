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

set(TEXT_RESOURCE_EXTENSIONS
    ".frag"
    ".vert"
    ".txt"
    ".yaml"
)

file(GLOB_RECURSE RESOURCE_SOURCES
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

find_program(OBJCOPY_EXECUTABLE objcopy REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    set(OBJCOPY_OUTPUT_TARGET elf64-x86-64)
    set(OBJCOPY_BINARY_ARCHITECTURE i386:x86-64)
else ()
    message(FATAL_ERROR "Binary resource embedding with objcopy is configured only for Linux x86_64")
endif ()

set(BINARY_RESOURCE_OBJECTS "")
foreach (RESOURCE_SOURCE IN LISTS RESOURCE_SOURCES)
    get_filename_component(RESOURCE_SOURCE_EXTENSION "${RESOURCE_SOURCE}" EXT)
    string(TOLOWER "${RESOURCE_SOURCE_EXTENSION}" RESOURCE_SOURCE_EXTENSION)
    list(FIND TEXT_RESOURCE_EXTENSIONS "${RESOURCE_SOURCE_EXTENSION}" TEXT_RESOURCE_EXTENSION_INDEX)

    if (TEXT_RESOURCE_EXTENSION_INDEX EQUAL -1)
        file(RELATIVE_PATH RESOURCE_RELATIVE_PATH "${RESOURCE_DIRECTORY}" "${RESOURCE_SOURCE}")
        set(RESOURCE_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/generated/resources/${RESOURCE_RELATIVE_PATH}.o")
        set(RESOURCE_NOTE "${CMAKE_CURRENT_BINARY_DIR}/generated/resources/${RESOURCE_RELATIVE_PATH}.note.GNU-stack")
        get_filename_component(RESOURCE_OBJECT_DIRECTORY "${RESOURCE_OBJECT}" DIRECTORY)

        add_custom_command(
            OUTPUT ${RESOURCE_OBJECT}
            BYPRODUCTS ${RESOURCE_NOTE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RESOURCE_OBJECT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E touch ${RESOURCE_NOTE}
            COMMAND ${OBJCOPY_EXECUTABLE}
            --input-target=binary
            --output-target=${OBJCOPY_OUTPUT_TARGET}
            --binary-architecture=${OBJCOPY_BINARY_ARCHITECTURE}
            --rename-section .data=.rodata,alloc,load,readonly,data,contents
            --add-section .note.GNU-stack=${RESOURCE_NOTE}
            --set-section-flags .note.GNU-stack=contents,readonly
            ${RESOURCE_RELATIVE_PATH}
            ${RESOURCE_OBJECT}
            WORKING_DIRECTORY ${RESOURCE_DIRECTORY}
            DEPENDS ${RESOURCE_SOURCE}
            COMMENT "Embedding binary resource ${RESOURCE_RELATIVE_PATH}"
            VERBATIM
        )

        list(APPEND BINARY_RESOURCE_OBJECTS "${RESOURCE_OBJECT}")
    endif ()
endforeach ()

add_custom_command(
    OUTPUT ${GENERATED_RESOURCE_HEADER}
    COMMAND ${CMAKE_COMMAND}
    "-DOUTPUT=${GENERATED_RESOURCE_HEADER}"
    "-DRESOURCE_DIRECTORY=${RESOURCE_DIRECTORY}"
    "-DTEXT_RESOURCE_EXTENSIONS=${TEXT_RESOURCE_EXTENSIONS}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedResources.cmake
    ${RESOURCE_SOURCES}
    COMMENT "Embedding resources"
    VERBATIM
)

add_custom_target(embedded_resources DEPENDS ${GENERATED_RESOURCE_HEADER} ${BINARY_RESOURCE_OBJECTS})
add_dependencies(probe_open_gl embedded_resources)

target_sources(probe_open_gl
    PRIVATE
    ${BINARY_RESOURCE_OBJECTS}
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
