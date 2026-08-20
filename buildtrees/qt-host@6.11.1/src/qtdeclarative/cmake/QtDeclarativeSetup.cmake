# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generate a header file containing a regular expression jit table.
function(qt_declarative_generate_reg_exp_jit_tables consuming_target)
    set(generate_dir "${CMAKE_CURRENT_BINARY_DIR}/.generated")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        string(APPEND generate_dir "/debug")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        string(APPEND generate_dir "/release")
    endif()

    set(output_file "${generate_dir}/RegExpJitTables.h")
    set(retgen_script_file "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/masm/yarr/create_regex_tables")

    add_custom_command(
        OUTPUT "${output_file}"
        COMMAND "${QT_INTERNAL_DECLARATIVE_PYTHON}" ${retgen_script_file} ${output_file}
        MAIN_DEPENDENCY ${retgen_script_file}
        VERBATIM
    )
    target_sources(${consuming_target} PRIVATE ${output_file})
    target_include_directories(${consuming_target} PRIVATE $<BUILD_INTERFACE:${generate_dir}>)
    _qt_internal_set_source_file_generated(SOURCES "${output_file}")
    set_source_files_properties(${output_file} PROPERTIES
        _qt_non_module_header TRUE)
endfunction()
