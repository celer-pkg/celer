# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# The function looks for the service definitions inside provided PROTO_FILES and returns list of
# the absolute .proto file paths, protobuf include paths and files that are expected to be generated
# by qtgrpcgen.
# Optional arguments:
#   GENERATE_PACKAGE_SUBFOLDERS - generated files will be located in package-base subdirectories.
#
# Multi-value arguments:
#   PROTO_FILES - input list of the proto files. May contain either absolute or relative paths.
function(_qt_internal_grpc_preparse_proto_files type
    out_proto_files out_proto_includes out_generated_files base_dir)

    cmake_parse_arguments(arg "GENERATE_PACKAGE_SUBFOLDERS" "" "PROTO_FILES" ${ARGN})

    unset(proto_files)
    unset(proto_includes)
    unset(output_files)
    foreach(f IN LISTS arg_PROTO_FILES)
        if(NOT IS_ABSOLUTE "${f}")
            set(f "${base_dir}/${f}")
            get_filename_component(f "${f}" ABSOLUTE)
        endif()
        get_filename_component(f "${f}" REALPATH)
        list(APPEND proto_files "${f}")

        _qt_internal_preparse_proto_file_common(result proto_package "${f}" "service")
        if(NOT result)
            message(NOTICE "No services found in ${f}. Skipping.")
            continue()
        endif()

        get_filename_component(proto_file_base_dir "${f}" DIRECTORY)
        list(PREPEND proto_includes "${proto_file_base_dir}")

        string(REPLACE "." "/" package_full_path "${proto_package}")
        set(folder_path "")
        if(arg_GENERATE_PACKAGE_SUBFOLDERS)
            set(folder_path "${package_full_path}/")
        endif()

        get_filename_component(basename "${f}" NAME_WLE)
        if(type STREQUAL "SERVER")
            list(APPEND output_files
                "${folder_path}${basename}_service.grpc.qpb.h")
        elseif(type STREQUAL "CLIENT")
            list(APPEND output_files
                "${folder_path}${basename}_client.grpc.qpb.h"
                "${folder_path}${basename}_client.grpc.qpb.cpp")
            if(arg_QML)
                list(APPEND output_files
                    "${folder_path}qml${basename}_client.grpc.qpb.h"
                    "${folder_path}qml${basename}_client.grpc.qpb.cpp")
            endif()
        else()
            message(FATAL_ERROR "Unknown gRPC target type: '${type}'.\n"
                "Supported types: CLIENT.")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES proto_files)
    list(REMOVE_DUPLICATES proto_includes)
    list(REMOVE_DUPLICATES output_files)
    set(${out_proto_files} "${proto_files}" PARENT_SCOPE)
    set(${out_proto_includes} "${proto_includes}" PARENT_SCOPE)
    set(${out_generated_files} "${output_files}" PARENT_SCOPE)
endfunction()

# TODO Qt6:
#     - Collect PROTO_INCLUDES from the LINK_LIBRARIES property of TARGET
#     - Collect proto files from the source files of the ${TARGET}

function(qt6_add_grpc target type)
    _qt_internal_get_protoc_common_options(protoc_option_opt protoc_single_opt protoc_multi_opt)
    _qt_internal_get_protoc_generate_arguments(protoc_option_arg protoc_single_arg protoc_multi_arg)

    set(option_args
        ${protoc_option_opt}
        ${protoc_option_arg}
    )

    set(single_args
        ${protoc_single_opt}
        ${protoc_single_arg}
    )

    set(multi_args
        ${protoc_multi_opt}
        ${protoc_multi_arg}
    )
    cmake_parse_arguments(arg "${option_args}" "${single_args}" "${multi_args}" ${ARGN})

    # TODO: Add the gRPC 'SERVER' target type support when implemented.
    set(supported_grpc_targets CLIENT)
    if(NOT type IN_LIST supported_grpc_targets)
        message(FATAL_ERROR "Unknown gRPC target type: '${type}'.\n"
            "Supported types: CLIENT.")
    endif()

    _qt_internal_get_protoc_options(generation_options arg
        protoc_option_opt protoc_single_opt protoc_multi_opt)

    if(arg_PROTO_FILES_BASE_DIR)
        set(base_dir "${arg_PROTO_FILES_BASE_DIR}")
    else()
        set(base_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    unset(extra_pre_parse_options)
    if(arg_GENERATE_PACKAGE_SUBFOLDERS)
        list(APPEND extra_pre_parse_options "GENERATE_PACKAGE_SUBFOLDERS")
    endif()

    _qt_internal_grpc_preparse_proto_files(${type} proto_files proto_includes generated_files
        "${base_dir}"
        ${extra_pre_parse_options}
        PROTO_FILES
            ${arg_PROTO_FILES}
    )

    if(NOT proto_files AND arg_PROTO_FILES)
        _qt_internal_protobuf_missing_definitions_warning(${target} grpc "${arg_PROTO_FILES}")
        return()
    endif()

    if(arg_PROTO_INCLUDES)
        list(APPEND proto_includes ${arg_PROTO_INCLUDES})
    endif()

    if(NOT TARGET ${target})
        _qt_internal_add_library(${target})
        if(DEFINED arg_OUTPUT_TARGETS)
            list(APPEND ${arg_OUTPUT_TARGETS} "${target}")
        endif()
    endif()

    foreach(f ${proto_files})
        _qt_internal_expose_source_file_to_ide(${target} ${f})
    endforeach()

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
        set(is_shared TRUE)
    elseif(target_type STREQUAL "STATIC_LIBRARY")
        set(is_static TRUE)
    elseif(target_type STREQUAL "EXECUTABLE")
        set(is_executable TRUE)
    else()
        message(FATAL_ERROR "Unsupported target type '${target_type}'.")
    endif()

    if(is_shared)
        set(generated_export "")
        set(generated_export_options "")
        _qt_internal_protoc_generate_cpp_exports(generated_export generated_export_options
            ${target} "${arg_EXPORT_MACRO}")
        if(generated_export)
            list(APPEND generated_files "${generated_export}")
        endif()
        list(APPEND generation_options "${generated_export_options}")
    endif()

    set(output_directory "${CMAKE_CURRENT_BINARY_DIR}")
    if(DEFINED arg_OUTPUT_DIRECTORY)
        set(output_directory "${arg_OUTPUT_DIRECTORY}")
    endif()

    list(TRANSFORM generated_files PREPEND "${output_directory}/")

    _qt_internal_protoc_generate(${target} qtgrpcgen "${output_directory}"
        PROTO_FILES ${proto_files}
        PROTO_INCLUDES ${proto_includes}
        GENERATED_FILES ${generated_files}
        OPTIONS ${generation_options}
    )

    target_sources(${target} PRIVATE ${generated_files})
    if(is_shared)
        _qt_internal_protoc_get_export_macro_filename(export_macro_filename ${target})
        target_sources(${target} PRIVATE "${output_directory}/${export_macro_filename}")
    endif()

    # Filter generated headers
    set(generated_headers "${generated_files}")
    list(FILTER generated_headers INCLUDE REGEX ".+\\.h$")

    set_target_properties(${target}
        PROPERTIES
            AUTOMOC ON
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target}
            PRIVATE "/Zc:__cplusplus" "/permissive-" "/bigobj")
    endif()

    target_link_libraries(${target} PRIVATE
        ${QT_CMAKE_EXPORT_NAMESPACE}::Grpc
    )
    if(arg_QML)
        if(NOT TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::GrpcQuick)
            message(FATAL_ERROR "QML option of the qt_add_grpc command requires"
                " ${QT_CMAKE_EXPORT_NAMESPACE}::GrpcQuick target. Please make sure that you"
                " have the respective Qt component found by adding it to the find_package call:"
                "   find_package(${QT_CMAKE_EXPORT_NAMESPACE} COMPONENTS GrpcQuick)")
        endif()
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::GrpcQuick)
    endif()

    if(DEFINED arg_OUTPUT_HEADERS)
        set(${arg_OUTPUT_HEADERS} "${generated_headers}" PARENT_SCOPE)
    endif()

    if(DEFINED arg_OUTPUT_TARGETS)
        set(${arg_OUTPUT_TARGETS} "${${arg_OUTPUT_TARGETS}}" PARENT_SCOPE)
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_grpc)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            qt6_add_grpc(${ARGN})
        else()
            message(FATAL_ERROR "qt6_add_grpc() is only available in Qt 6. "
                                "Please check the protobuf documentation for alternatives.")
        endif()
    endfunction()
endif()
