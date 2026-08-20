# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(_qc_internal_add_shaders_impl target resourcename)
    cmake_parse_arguments(
        arg
        "_QT_INTERNAL"
        "PREFIX;BASE;OUTPUT_TARGETS"
        "FILES;OUTPUTS;DEFINES"
        ${ARGN}
    )

    set(qsb_outputs ${arg_OUTPUTS})
    set(processed_files "")

    math(EXPR file_index "0")
    foreach(file IN LISTS arg_FILES)
        get_filename_component(input_file_absolute ${file} ABSOLUTE)
        get_filename_component(input_file_ext ${file} LAST_EXT)

        # Mirror Qt6ShaderToolsMacros.cmake logic, but use a .qctempshaders
        # directory. OUTPUTS, if present, matters in particular for the temporary
        # filename, since just generating a filename based on the input (in FILES)
        # would lead to clashes if the same source file was used to generate
        # multiple variants (like with different DEFINES) in qc_add_shaders.

        set(output_file "${file}.qsb")
        if(arg_OUTPUTS)
            list(GET arg_OUTPUTS ${file_index} output_file)
        else()
            if(arg_BASE)
                get_filename_component(abs_base "${arg_BASE}" ABSOLUTE)
                get_filename_component(abs_output_file "${output_file}" ABSOLUTE)
                file(RELATIVE_PATH output_file "${abs_base}" "${abs_output_file}")
            endif()
            list(APPEND qsb_outputs ${output_file})
        endif()

        # Now output_file ends in .qsb, but it must end with the original .vert
        # or .frag extension since that is required by qsb.
        set(output_file "${output_file}.qcpre${input_file_ext}")
        set(processed_file "${CMAKE_CURRENT_BINARY_DIR}/.qctempshaders/${output_file}")
        list(APPEND processed_files "${processed_file}")

        set(shadergen_args "")

        list(APPEND shadergen_args "-o")
        list(APPEND shadergen_args "${processed_file}")

        list(APPEND shadergen_args "${input_file_absolute}")

        _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
        set(qcshadergen_executable "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qcshadergen>")
        if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config"
                AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")
            set(qcshadergen_executable "$<COMMAND_CONFIG:${qcshadergen_executable}>")
        endif()
        add_custom_command(
            OUTPUT
                ${processed_file}
            COMMAND
                ${tool_wrapper}
                ${qcshadergen_executable}
                ${shadergen_args}
            DEPENDS
                ${qcshadergen_executable}
                "${file}"
            VERBATIM
        )

        math(EXPR file_index "${file_index}+1")
    endforeach()

    add_custom_target(${target}_${resourcename}_qcpre DEPENDS "${processed_files}")
    add_dependencies(${target} ${target}_${resourcename}_qcpre)

    # OpenGL ES 3.0 or newer, OpenGL 3.2 or newer. 130 (OpenGL 3.0) is the
    # minimum (e.g. due to textureSize), it is here for old llvmpipe versions
    # that some CI might still use.
    set(opengl_glsl_versions "300es,150,130")

    if (arg__QT_INTERNAL)
        qt_internal_add_shaders(${target} ${resourcename}
            GLSL
                ${opengl_glsl_versions}
            PREFIX
                ${arg_PREFIX}
            FILES
                ${processed_files}
            ORIGINAL_FILES
                ${arg_FILES}
            OUTPUTS
                ${qsb_outputs}
            DEFINES
                ${arg_DEFINES}
            OUTPUT_TARGETS
                output_targets
        )
    else()
        qt_add_shaders(${target} ${resourcename}
            GLSL
                ${opengl_glsl_versions}
            PREFIX
                ${arg_PREFIX}
            FILES
                ${processed_files}
            ORIGINAL_FILES
                ${arg_FILES}
            OUTPUTS
                ${qsb_outputs}
            DEFINES
                ${arg_DEFINES}
            OUTPUT_TARGETS
                output_targets
        )
    endif()

    if(arg_OUTPUT_TARGETS)
        set(${arg_OUTPUT_TARGETS} "${output_targets}" PARENT_SCOPE)
    endif()
endfunction()

function(qc_add_shaders)
    _qc_internal_add_shaders_impl(${ARGV})
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "OUTPUT_TARGETS" "")
    if (arg_OUTPUT_TARGETS)
        set(${arg_OUTPUT_TARGETS} ${${arg_OUTPUT_TARGETS}} PARENT_SCOPE)
    endif()
endfunction()

# for use by Qt modules that need qt_internal_add_resource
function(qc_internal_add_shaders)
    _qc_internal_add_shaders_impl(${ARGV} _QT_INTERNAL)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "OUTPUT_TARGETS" "")
    if (arg_OUTPUT_TARGETS)
        set(${arg_OUTPUT_TARGETS} ${${arg_OUTPUT_TARGETS}} PARENT_SCOPE)
    endif()
endfunction()
