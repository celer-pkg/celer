# Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(qt6_add_statecharts target_or_outfiles)
    set(options)
    set(oneValueArgs OUTPUT_DIR OUTPUT_DIRECTORY NAMESPACE)
    set(multiValueArgs QSCXMLC_ARGUMENTS OPTIONS)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(scxml_files ${ARGS_UNPARSED_ARGUMENTS})
    set(outfiles)

    if (ARGS_NAMESPACE)
        set(namespace "--namespace" ${ARGS_NAMESPACE})
    endif()

    if (ARGS_OUTPUT_DIR)
        message(AUTHOR_WARNING
            "OUTPUT_DIR is deprecated. Please use OUTPUT_DIRECTORY instead.")
        set(ARGS_OUTPUT_DIRECTORY ${ARGS_OUTPUT_DIR})
    endif()

    if (ARGS_QSCXMLC_ARGUMENTS)
        message(AUTHOR_WARNING
            "QSCXMLC_ARGUMENTS is deprecated. Please use OPTIONS instead.")
        set(ARGS_OPTIONS ${ARGS_QSCXMLC_ARGUMENTS})
    endif()

    set(qscxmlcOutputDir ${CMAKE_CURRENT_BINARY_DIR})
    if (ARGS_OUTPUT_DIRECTORY)
        set(qscxmlcOutputDir ${ARGS_OUTPUT_DIRECTORY})
        if (NOT EXISTS "${qscxmlcOutputDir}" OR NOT IS_DIRECTORY "${qscxmlcOutputDir}")
            message(WARNING
                "qt6_add_statecharts: output dir does not exist: \"" ${qscxmlcOutputDir} "\". "
                "Statechart code generation may fail on some platforms." )
        endif()
    endif()

    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(qscxmlc_bin "${tool_wrapper}" "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qscxmlc>")

    set(outfiles)
    foreach(it ${scxml_files})
        get_filename_component(outfilename ${it} NAME_WE)
        get_filename_component(infile ${it} ABSOLUTE)
        set(outfile ${qscxmlcOutputDir}/${outfilename})
        set(outfile_cpp ${outfile}.cpp)
        set(outfile_h ${outfile}.h)

        add_custom_command(OUTPUT ${outfile_cpp} ${outfile_h}
                           COMMAND
                               ${qscxmlc_bin} ${namespace} ${ARGS_OPTIONS}
                               --output ${outfile} ${infile}
                           DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::qscxmlc
                           MAIN_DEPENDENCY ${infile}
                           VERBATIM)
        set_source_files_properties(${outfile_cpp} ${outfile_h} PROPERTIES SKIP_AUTOGEN TRUE)
        list(APPEND outfiles ${outfile_cpp})
    endforeach()
    if (TARGET ${target_or_outfiles})
        target_include_directories(${target_or_outfiles} PRIVATE ${qscxmlcOutputDir})
        target_sources(${target_or_outfiles} PRIVATE ${outfiles})
    else()
        set(${target_or_outfiles} ${outfiles} PARENT_SCOPE)
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_statecharts outfiles)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
            qt5_add_statecharts("${outfiles}" ${ARGN})
        elseif(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_add_statecharts("${outfiles}" ${ARGN})
        endif()
        set("${outfiles}" "${${outfiles}}" PARENT_SCOPE)
    endfunction()
endif()
