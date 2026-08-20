# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generates a C++ namespace for a type library and adds generated sources to the target. Arguments:
#
# LIBRARIES: List of type libraries. A type library (.tlb) is a binary file that stores information
#   about a COM or DCOM object's properties and methods in a form that is accessible to other
#   applications at runtime. The list may contain either the path to the library or the name
#   of the library.
#   If the library name is specified, the function will search for the library according to the
#   CMake find_file function rules. See https://cmake.org/cmake/help/latest/command/find_file.html
#   for details.
#   Note: The library name must include a file suffix, e.g "ieframe.dll".
#   LIBRARIES also may contain the library UUID in the following format:
#       <generated_files_basename>:<{00000000-0000-0000-0000-000000000000}>
#   The 'generated_files_basename' may contain ASCII letters, numbers and underscores and will be
#    used as the base name for the generated files.
#
# OUTPUT_DIRECTORY: Custom location of the generated source files.
#   ${CMAKE_CURRENT_BINARY_DIR} is the default location if not specified. (OPTIONAL)
#
# COMPAT: Adds compatibility flag to the dumpcpp call, that generates namespace with
#   dynamicCall-compatible API. (OPTIONAL)
#
# This function is currently in Technology Preview.
# Its signature and behavior might change.
function(qt6_target_typelibs target)
    cmake_parse_arguments(arg "COMPAT" "OUTPUT_DIRECTORY" "LIBRARIES" ${ARGN})
    if(NOT arg_LIBRARIES)
        message(FATAL_ERROR "qt6_target_typelibs: LIBRARIES are not specified")
    endif()

    set(output_directory "${CMAKE_CURRENT_BINARY_DIR}")
    if(arg_OUTPUT_DIRECTORY)
        set(output_directory "${arg_OUTPUT_DIRECTORY}")
    endif()

    set(extra_args "")
    if(arg_COMPAT)
        list(APPEND extra_args "-compat")
    endif()

    set(out_sources "")

    # CMake doesn't support quantifiers.
    set(hex_num "[a-fA-F0-9][a-fA-F0-9]")
    set(hex_two "${hex_num}${hex_num}")
    set(hex_four "${hex_two}${hex_two}")
    set(hex_six "${hex_two}${hex_two}${hex_two}")

    set(ident_ "[a-zA-Z0-9_]")

    set(typelibFilePath "") # Only set if type library is a file and not a UUID

    foreach(lib IN LISTS arg_LIBRARIES)
        unset(libpath CACHE)
        if(lib MATCHES "^(${ident_}+):({${hex_four}-${hex_two}-${hex_two}-${hex_two}-${hex_six}})$")
            set(libpath "${CMAKE_MATCH_2}")
            set(out_basename "${CMAKE_MATCH_1}")
            string(MAKE_C_IDENTIFIER "${out_basename}" out_basename_valid)
            if(NOT "${out_basename_valid}" STREQUAL "${out_basename}")
                message("The specified generated files basename ${out_basename} is not valid\
C indentifier")
            endif()
        else()
            # If lib exists on the filesystem, we assume the user provided the path.
            get_filename_component(lib_abspath "${lib}" ABSOLUTE)
            if(EXISTS "${lib_abspath}")
                set(libpath "${lib_abspath}")
            else()
                find_file(libpath NAMES "${lib}")
                if(NOT libpath)
                    message(FATAL_ERROR "qt6_target_typelibs: Unable to find type lib with name ${lib}")
                endif()
            endif()

            set(typelibFilePath ${libpath})
            get_filename_component(out_basename "${libpath}" NAME_WE)
        endif()

        set(out_filebasepath "${output_directory}/${out_basename}")
        set(out_header "${out_filebasepath}.h")
        set_source_files_properties("${out_header}" PROPERTIES HEADER_FILE_ONLY TRUE)
        set(out_source "${out_filebasepath}.cpp")
        list(APPEND out_sources "${out_header}" "${out_source}")

        _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
        set(dumpcpp_bin "${tool_wrapper}" "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::dumpcpp>")

        add_custom_command(OUTPUT "${out_header}" "${out_source}"
            COMMAND ${dumpcpp_bin}
                "${libpath}" -o "${out_filebasepath}"
                ${extra_args}
            DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::dumpcpp
            MAIN_DEPENDENCY ${typelibFilePath}
            WORKING_DIRECTORY "${output_directory}"
            COMMENT "Generate type lib sources ${out_header} ${out_source}..."
        )
    endforeach()

    set_source_files_properties("${out_sources}" PROPERTIES SKIP_AUTOGEN TRUE)

    target_sources(${target} PRIVATE "${out_sources}")
    target_include_directories(${target} PRIVATE "${output_directory}")
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_target_typelibs)
        qt6_target_typelibs(${ARGV})
    endfunction()
endif()
