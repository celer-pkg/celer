# Copyright (C) 2019 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
# framework style includes like #include <QtCore/qobject.h>
# Extract the location of the Qt frameworks by querying the imported location of QtRemoteObjects
# framework parent directory.
function(_qt_internal_get_remote_objects_framework_path out_var)
    set(value "")
    if(APPLE AND QT_FEATURE_framework)
        get_target_property(ro_path ${QT_CMAKE_EXPORT_NAMESPACE}::RemoteObjects IMPORTED_LOCATION)
        string(REGEX REPLACE "(.*)/Qt[^/]+\\.framework.*" "\\1" ro_fw_path "${ro_path}")
        if(ro_fw_path)
            set(value "${ro_fw_path}")
        endif()
    endif()
    set(${out_var} "${value}" PARENT_SCOPE)
endfunction()

function(_qt_internal_get_remote_objects_framework_path_moc_options out_var)
    _qt_internal_get_remote_objects_framework_path(ro_fw_path)
    if(ro_fw_path)
        set(${out_var} "OPTIONS" "-F${ro_fw_path}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(_qt_internal_add_repc_files type target)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(outfiles)
    if(QT_REPC_DEBUG_MODE)
        set(debug "-d")
    else()
        set(debug "")
    endif()
    set(repc_incpath) ########### TODO

    _qt_internal_get_remote_objects_framework_path_moc_options(extra_moc_options)
    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(repc_command
        COMMAND
            "${tool_wrapper}"
            "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::repc>"
    )
    foreach(it ${ARGS_SOURCES})
        get_filename_component(outfilename ${it} NAME_WE)
        get_filename_component(extension ${it} EXT)
        if ("${extension}" STREQUAL ".h" OR "${extension}" STREQUAL ".hpp")
            # This calls moc on an existing header file to extract metatypes json information
            # which is then passed to the tool to generate another header.
            qt6_wrap_cpp(qtro_moc_files "${it}"
                         __QT_INTERNAL_OUTPUT_MOC_JSON_FILES json_list
                         TARGET "${target}"
                         ${extra_moc_options})

            # Pass the generated metatypes .json file to the tool.
            set(infile ${json_list})
            set_source_files_properties(${qtro_moc_files} PROPERTIES HEADER_FILE_ONLY ON)
            list(APPEND outfiles ${qtro_moc_files})
        else()
            # Pass the .rep file to the tool.
            get_filename_component(infile ${it} ABSOLUTE)
        endif()
        set(outfile ${CMAKE_CURRENT_BINARY_DIR}/rep_${outfilename}_${type}.h)

        add_custom_command(
            OUTPUT ${outfile}
            ${repc_command} ${debug} -o ${type} ${repc_incpath} ${infile} ${outfile}
            MAIN_DEPENDENCY ${infile}
            DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::repc
            VERBATIM
        )
        list(APPEND outfiles ${outfile})

        # The generated header file needs to be manually moc'ed (without using AUTOMOC) and then
        # added as source to compile into the target.
        qt6_wrap_cpp(qtro_moc_files "${outfile}" TARGET "${target}" ${extra_moc_options})
        list(APPEND outfiles ${qtro_moc_files})
    endforeach()
    _qt_internal_set_source_file_generated(
        SOURCES ${outfiles}
        SKIP_AUTOGEN
    )
    target_sources(${target} PRIVATE ${outfiles})
endfunction()

# Add .rep source files to a target to generate source header files
function(qt6_add_repc_sources target)
    list(POP_FRONT ARGV)
    _qt_internal_add_repc_files(source ${target} SOURCES ${ARGV})
endfunction()

# Add .rep source files to a target to generate replica header files
function(qt6_add_repc_replicas target)
    list(POP_FRONT ARGV)
    _qt_internal_add_repc_files(replica ${target} SOURCES ${ARGV})
endfunction()

# Add .rep source files to a target to generate combined (source and replica) header files
function(qt6_add_repc_merged target)
    list(POP_FRONT ARGV)
    _qt_internal_add_repc_files(merged ${target} SOURCES ${ARGV})
endfunction()

# Create .rep interface file from QObject header
function(qt6_reps_from_headers target)
    list(POP_FRONT ARGV)
    _qt_internal_get_remote_objects_framework_path_moc_options(extra_moc_options)
    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(repc_command
        COMMAND
            "${tool_wrapper}"
            "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::repc>"
    )

    foreach(it ${ARGV})
        get_filename_component(outfilename ${it} NAME_WE)
        # This calls moc on an existing header file to extract metatypes json information
        # which is then passed to the tool to generate a .rep file.
        qt6_wrap_cpp(qtro_moc_files "${it}"
                     __QT_INTERNAL_OUTPUT_MOC_JSON_FILES json_list
                     TARGET "${target}"
                     ${extra_moc_options})
        set(infile ${json_list})
        set_source_files_properties(${qtro_moc_files} PROPERTIES HEADER_FILE_ONLY ON)
        list(APPEND outfiles ${qtro_moc_files})
        set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfilename}.rep)
        add_custom_command(OUTPUT ${outfile}
                           ${repc_command} -o rep ${infile} ${outfile}
                           MAIN_DEPENDENCY ${infile}
                           DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::repc
                           VERBATIM)
        list(APPEND outfiles ${outfile})
    endforeach()
    target_sources(${target} PRIVATE ${outfiles})
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_repc_sources)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_add_repc_sources(${ARGN})
        else()
            message(FATAL_ERROR "qt_add_repc_sources() is only available in Qt 6. "
                                "Please check the repc documentation for alternatives.")
        endif()
    endfunction()
    function(qt_add_repc_replicas)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_add_repc_replicas(${ARGN})
        else()
            message(FATAL_ERROR "qt_add_repc_replicas() is only available in Qt 6. "
                                "Please check the repc documentation for alternatives.")
        endif()
    endfunction()
    function(qt_add_repc_merged)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_add_repc_merged(${ARGN})
        else()
            message(FATAL_ERROR "qt_add_repc_merged() is only available in Qt 6. "
                                "Please check the repc documentation for alternatives.")
        endif()
    endfunction()
    function(qt_reps_from_headers)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_reps_from_headers(${ARGN})
        else()
            message(FATAL_ERROR "qt_reps_from_headers() is only available in Qt 6. "
                                "Please check the repc documentation for alternatives.")
        endif()
    endfunction()
endif()
