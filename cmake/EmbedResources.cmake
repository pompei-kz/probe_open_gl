if (NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif ()
if (NOT DEFINED RESOURCE_DIRECTORY)
    message(FATAL_ERROR "RESOURCE_DIRECTORY is required")
endif ()

file(GLOB_RECURSE RESOURCE_FILES
        LIST_DIRECTORIES false
        "${RESOURCE_DIRECTORY}/*"
)
list(SORT RESOURCE_FILES)

set(GENERATED_CONTENT "")
string(APPEND GENERATED_CONTENT "#pragma once\n\n")
string(APPEND GENERATED_CONTENT "#include <array>\n")
string(APPEND GENERATED_CONTENT "#include <string_view>\n\n")
string(APPEND GENERATED_CONTENT "namespace resources {\n\n")

set(TEXT_RESOURCE_EXTENSIONS
        ".frag"
        ".vert"
        ".txt"
)

set(RESOURCE_NAMES "")
foreach (RESOURCE_FILE IN LISTS RESOURCE_FILES)
    file(RELATIVE_PATH RESOURCE_FILE_NAME "${RESOURCE_DIRECTORY}" "${RESOURCE_FILE}")
    get_filename_component(RESOURCE_FILE_EXTENSION "${RESOURCE_FILE}" EXT)
    string(TOLOWER "${RESOURCE_FILE_EXTENSION}" RESOURCE_FILE_EXTENSION)

    set(RESOURCE_NAME "${RESOURCE_FILE_NAME}")
    string(REPLACE "." "_" RESOURCE_NAME "${RESOURCE_NAME}")
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" RESOURCE_NAME "${RESOURCE_NAME}")
    if (RESOURCE_NAME MATCHES "^[0-9]")
        set(RESOURCE_NAME "_${RESOURCE_NAME}")
    endif ()

    list(FIND RESOURCE_NAMES "${RESOURCE_NAME}" RESOURCE_NAME_INDEX)
    if (NOT RESOURCE_NAME_INDEX EQUAL -1)
        message(FATAL_ERROR "Duplicate resource name after sanitizing: ${RESOURCE_NAME}")
    endif ()
    list(APPEND RESOURCE_NAMES "${RESOURCE_NAME}")

    list(FIND TEXT_RESOURCE_EXTENSIONS "${RESOURCE_FILE_EXTENSION}" TEXT_RESOURCE_EXTENSION_INDEX)
    if (NOT TEXT_RESOURCE_EXTENSION_INDEX EQUAL -1)
        file(READ "${RESOURCE_FILE}" RESOURCE_SOURCE)

        string(APPEND GENERATED_CONTENT
                "inline constexpr std::string_view ${RESOURCE_NAME} = R\"RESOURCE(")
        string(APPEND GENERATED_CONTENT "${RESOURCE_SOURCE}")
        string(APPEND GENERATED_CONTENT ")RESOURCE\";\n\n")
    else ()
        file(READ "${RESOURCE_FILE}" RESOURCE_SOURCE_HEX HEX)
        string(LENGTH "${RESOURCE_SOURCE_HEX}" RESOURCE_SOURCE_HEX_LENGTH)
        math(EXPR RESOURCE_SIZE "${RESOURCE_SOURCE_HEX_LENGTH} / 2")

        string(APPEND GENERATED_CONTENT
                "inline constexpr std::array<unsigned char, ${RESOURCE_SIZE}> ${RESOURCE_NAME} = {")

        if (RESOURCE_SIZE GREATER 0)
            string(REGEX MATCHALL ".." RESOURCE_BYTES "${RESOURCE_SOURCE_HEX}")
            set(RESOURCE_SEPARATOR "")
            foreach (RESOURCE_BYTE IN LISTS RESOURCE_BYTES)
                string(APPEND GENERATED_CONTENT "${RESOURCE_SEPARATOR}0x${RESOURCE_BYTE}")
                set(RESOURCE_SEPARATOR ", ")
            endforeach ()
        endif ()

        string(APPEND GENERATED_CONTENT "};\n\n")
    endif ()
endforeach ()

string(APPEND GENERATED_CONTENT "}  // namespace resources\n")

get_filename_component(OUTPUT_DIRECTORY "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIRECTORY}")
file(WRITE "${OUTPUT}" "${GENERATED_CONTENT}")
