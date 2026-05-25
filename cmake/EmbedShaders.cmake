if (NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif ()
if (NOT DEFINED RESOURCE_DIRECTORY)
    message(FATAL_ERROR "RESOURCE_DIRECTORY is required")
endif ()

file(GLOB RESOURCE_FILES
        LIST_DIRECTORIES false
        "${RESOURCE_DIRECTORY}/*"
)
list(SORT RESOURCE_FILES)

set(GENERATED_CONTENT "")
string(APPEND GENERATED_CONTENT "#pragma once\n\n")
string(APPEND GENERATED_CONTENT "#include <string_view>\n\n")
string(APPEND GENERATED_CONTENT "namespace shader_resources {\n\n")

set(RESOURCE_NAMES "")
foreach (RESOURCE_FILE IN LISTS RESOURCE_FILES)
    get_filename_component(RESOURCE_FILE_NAME "${RESOURCE_FILE}" NAME)
    set(RESOURCE_NAME "${RESOURCE_FILE_NAME}")
    string(REPLACE "." "_" RESOURCE_NAME "${RESOURCE_NAME}")
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" RESOURCE_NAME "${RESOURCE_NAME}")
    if (RESOURCE_NAME MATCHES "^[0-9]")
        set(RESOURCE_NAME "_${RESOURCE_NAME}")
    endif ()

    list(FIND RESOURCE_NAMES "${RESOURCE_NAME}" RESOURCE_NAME_INDEX)
    if (NOT RESOURCE_NAME_INDEX EQUAL -1)
        message(FATAL_ERROR "Duplicate shader resource name after sanitizing: ${RESOURCE_NAME}")
    endif ()
    list(APPEND RESOURCE_NAMES "${RESOURCE_NAME}")

    file(READ "${RESOURCE_FILE}" RESOURCE_SOURCE)

    string(APPEND GENERATED_CONTENT
            "inline constexpr std::string_view ${RESOURCE_NAME} = R\"SHADER(")
    string(APPEND GENERATED_CONTENT "${RESOURCE_SOURCE}")
    string(APPEND GENERATED_CONTENT ")SHADER\";\n\n")
endforeach ()

string(APPEND GENERATED_CONTENT "}  // namespace shader_resources\n")

get_filename_component(OUTPUT_DIRECTORY "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
file(WRITE "${OUTPUT}" "${GENERATED_CONTENT}")
