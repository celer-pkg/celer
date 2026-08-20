# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#quick3d version of qrc to resource
function(_qt_internal_quick3d_generate_resource_from_qrc target resource_name)
    set(generatedResourceFile "${CMAKE_CURRENT_BINARY_DIR}/.rcc/generated_${resource_name}.qrc")
    set(generatedSourceCode "${CMAKE_CURRENT_BINARY_DIR}/.rcc/qrc_${resource_name}.cpp")
    set(rccArgs --name "${resource_name}"
                --output "${generatedSourceCode}" "${generatedResourceFile}")

    if(NOT QT_FEATURE_zstd)
        list(APPEND rccArgs "--no-zstd")
    endif()

    add_custom_command(
        OUTPUT
            "${generatedSourceCode}"
        COMMAND
            ${QT_CMAKE_EXPORT_NAMESPACE}::rcc
        ARGS
            ${rccArgs}
        DEPENDS
            "${generatedResourceFile}"
            $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::rcc>
            $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::shadergen>
        COMMENT
            "RCC ${resource_name}"
        VERBATIM
    )

    get_target_property(type ${target} TYPE)

    target_sources(${target} PRIVATE "${generatedSourceCode}")
endfunction()

#generate shaders from the given files
function(qt6_add_materials target resource_name)

    cmake_parse_arguments(arg
        "" "PREFIX" "FILES" ${ARGN}
    )

    set(output_qrc "generated_${resource_name}.qrc")
    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/.rcc")

    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(shadergen_command
        COMMAND
            "${tool_wrapper}"
            "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::shadergen>"
            -C "${PROJECT_SOURCE_DIR}"
            -o "${output_dir}"
            -r "${output_qrc}"
            \""${arg_FILES}"\"
    )

    add_custom_command(
        OUTPUT "${output_dir}/${output_qrc}"
        ${shadergen_command}
        DEPENDS
            $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::shadergen>
    )

    _qt_internal_quick3d_generate_resource_from_qrc(${target} ${resource_name})
endfunction()

# .hdr -> .ktx baker. Quite similar to qt6_add_shaders().
#
# For example, the following autogenerates the lightprobe map at build time and
# includes it in the executable under :/maps/OpenfootageNET_garage-1024.ktx:
#
# qt6_add_lightprobe_images(principledmaterial "ibl_assets"
#    PREFIX
#        "/"
#    FILES
#        "maps/OpenfootageNET_garage-1024.hdr" )
#
# In addition to PREFIX, BASE is available as well, and works like in qt6_add_resources:
# adding BASE "maps" in the above example would lead to getting :/OpenfootageNET_garage-1024.ktx
#
# OUTPUTS works like in qt6_add_shaders, allowing to specify an alternative name
# for the resource system for each entry in FILES. For example, adding
# OUTPUTS "alt/er/native/image.abc" to the above example would generate :/alt/er/native/image.abc
#
# In short, the actual file name in the resource system is
# either :/PREFIX/FILES[i]-BASE-".hdr"+".ktx" or :/PREFIX/OUTPUTS[i]
#
function(qt6_add_lightprobe_images target resource_name)
    cmake_parse_arguments(arg
        ""
        "PREFIX;BASE;_QT_INTERNAL"
        "FILES;OUTPUTS"
        ${ARGN}
    )

    math(EXPR file_index "0")
    foreach(file IN LISTS arg_FILES)
        get_filename_component(file_name_wo_ext ${file} NAME_WLE)
        get_filename_component(file_ext ${file} EXT)
        set(supported_formats ".hdr" ".exr")
        if(NOT file_ext IN_LIST supported_formats)
            message(FATAL_ERROR "Light probe HDRI maps must have the extensions .hdr or .exr, whereas ${file} has something else")
        endif()

        get_filename_component(file_dir "${file}" DIRECTORY)
        string(JOIN "/" res_name "${file_dir}" "${file_name_wo_ext}")
        string(JOIN "." res_name "${res_name}" "ktx")

        if(arg_OUTPUTS)
            list(GET arg_OUTPUTS ${file_index} res_name)
        elseif(arg_BASE)
            get_filename_component(abs_base "${arg_BASE}" ABSOLUTE)
            get_filename_component(abs_res_name "${res_name}" ABSOLUTE)
            file(RELATIVE_PATH res_name "${abs_base}" "${abs_res_name}")
        endif()

        # This should be .../.ibl/${res_name} but balsam strips the path so no choice but
        # to follow suit here and hope that the names of the files won't clash.
        set(ktx_result "${CMAKE_CURRENT_BINARY_DIR}/.ibl/${file_name_wo_ext}.ktx")

        get_filename_component(file_absolute ${file} ABSOLUTE)

        list(APPEND balsam_args "--no-plugins")
        list(APPEND balsam_args "-o")
        list(APPEND balsam_args "${CMAKE_CURRENT_BINARY_DIR}/.ibl")
        list(APPEND balsam_args "${file_absolute}")

        add_custom_command(
            OUTPUT
                ${ktx_result}
            COMMAND
                ${QT_CMAKE_EXPORT_NAMESPACE}::balsam ${balsam_args}
            DEPENDS
                "${file_absolute}"
                $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::balsam>
            VERBATIM
        )

        list(APPEND ktx_files "${ktx_result}")
        set_source_files_properties("${ktx_result}" PROPERTIES QT_RESOURCE_ALIAS "${res_name}")

        math(EXPR file_index "${file_index}+1")
    endforeach()

    if(arg__QT_INTERNAL)
        qt_internal_add_resources(${target} ${resource_name}
            PREFIX
                "${arg_PREFIX}"
            FILES
                "${ktx_files}"
        )
    else()
        qt6_add_resources(${target} ${resource_name}
            PREFIX
                "${arg_PREFIX}"
            FILES
                "${ktx_files}"
        )
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_materials)
        qt6_add_materials(${ARGV})
    endfunction()
    function(qt_add_lightprobe_images)
        qt6_add_lightprobe_images(${ARGV})
    endfunction()
endif()

# for use by Qt modules that need qt_internal_add_resource
function(qt_internal_add_lightprobe_images)
    qt6_add_lightprobe_images(${ARGV} _QT_INTERNAL)
endfunction()

# Qt 6.1 compatibility
function(qt6_quick3d_generate_materials)
    message(AUTHOR_WARNING "qt6_quick3d_generate_materials is deprecated, use qt6_add_materials instead")
    qt6_add_materials(${ARGV})
endfunction()
function(qt6_quick3d_bake_lightprobe_hdri)
    message(AUTHOR_WARNING "qt6_quick3d_bake_lightprobe_hdri is deprecated, use qt6_add_lightprobe_images instead")
    qt6_add_lightprobe_images(${ARGV})
endfunction()
