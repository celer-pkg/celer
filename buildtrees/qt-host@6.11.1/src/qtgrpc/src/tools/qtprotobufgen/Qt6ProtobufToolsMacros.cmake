# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(__qt_protobuf_macros_module_base_dir "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

# List of the common protoc generator options.
macro(_qt_internal_get_protoc_common_options option_args single_args multi_args)
    set(${option_args}
        COPY_COMMENTS
        GENERATE_PACKAGE_SUBFOLDERS
        QML
        ALLOW_MUTABLE_GETTER_CONFLICTS
        GENERATE_NON_FINAL_MESSAGES
    )
    set(${single_args}
        EXTRA_NAMESPACE
        HEADER_GUARD
    )

    set(${multi_args} "")
endmacro()

# List of arguments common for the protoc generating functions.
macro(_qt_internal_get_protoc_generate_arguments option_args single_args multi_args)
    set(${option_args} "")
    set(${single_args}
        OUTPUT_DIRECTORY
        PROTO_FILES_BASE_DIR
        OUTPUT_HEADERS
        OUTPUT_TARGETS
        EXPORT_MACRO
        QML_URI
    )
    set(${multi_args}
        PROTO_FILES
        PROTO_INCLUDES
    )
endmacro()

# The macro collects options in protoc compatible format. Options are written into out_var.
# All input arguments are names of the lists containing the corresponding options.
macro(_qt_internal_get_protoc_options out_var prefix option single multi)
    set(${out_var} "")
    foreach(opt IN LISTS ${option})
        if(${prefix}_${opt})
            list(APPEND ${out_var} ${opt})
        endif()
    endforeach()

    foreach(opt IN LISTS ${single} ${multi})
        if(${prefix}_${opt})
            list(APPEND ${out_var} "${opt}=${${prefix}_${opt}}")
        endif()
    endforeach()

    if(${prefix}_QML_URI)
        list(APPEND ${out_var} "QML=true")
    endif()
endmacro()

# Returns the generator target name according to the pre-defined pattern
function(_qt_internal_get_generator_dep_target_name out_var target generator dep_index)
    set(${out_var} "${target}_${generator}_deps_${dep_index}" PARENT_SCOPE)
endfunction()

# The base function that generates rules to call the protoc executable with the custom generator
# plugin.
# Multi-value Arguments:
#   PROTO_FILES - list of the .proto file paths. Paths should be absolute for the correct work of
#       this function.
#   PROTO_INCLUDES - list of the protobuf include paths.
#   GENERATED_FILES - list of files that are expected
#                     to be genreated by the custom generator plugin.
#   OPTIONS - list of the generator-specific options.
function(_qt_internal_protoc_generate target generator output_directory)
    cmake_parse_arguments(arg "" "" "PROTO_FILES;PROTO_INCLUDES;GENERATED_FILES;OPTIONS" ${ARGN})

    if(NOT arg_PROTO_FILES)
        message(FATAL_ERROR "PROTO_FILES list is empty.")
    endif()

    set(proto_includes "")
    if(arg_PROTO_INCLUDES)
        set(proto_includes "${arg_PROTO_INCLUDES}")
    endif()

    if(NOT arg_GENERATED_FILES)
        message(FATAL_ERROR
            "List of generated sources for target '${target}' is empty")
    endif()

    set(generated_files "${arg_GENERATED_FILES}")

    get_filename_component(output_directory "${output_directory}" REALPATH)
    get_target_property(is_generator_imported ${QT_CMAKE_EXPORT_NAMESPACE}::${generator} IMPORTED)
    if(QT_INTERNAL_AVOID_USING_PROTOBUF_TMP_OUTPUT_DIR OR is_generator_imported
        OR NOT CMAKE_GENERATOR MATCHES "^Ninja")
        set(tmp_output_directory "${output_directory}")
    else()
        set(tmp_output_directory "${output_directory}/.tmp")
    endif()
    file(MAKE_DIRECTORY ${tmp_output_directory})

    unset(num_deps)
    if(TARGET ${target})
        get_target_property(num_deps ${target} _qt_${generator}_deps_num)
    endif()
    if(NOT num_deps)
        set(num_deps 0)
    endif()
    _qt_internal_get_generator_dep_target_name(deps_target ${target} ${generator} ${num_deps})
    math(EXPR num_deps "${num_deps} + 1")

    set(generator_file $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::${generator}>)

    set(proto_includes_string "")
    if(proto_includes)
        list(JOIN proto_includes "$<SEMICOLON>" proto_includes)
        set(proto_includes_genex "$<GENEX_EVAL:${proto_includes}>")
        set(proto_includes_condition "$<BOOL:${proto_includes_genex}>")
        string(JOIN "" proto_includes_string
            "$<${proto_includes_condition}:"
                "-I$<JOIN:${proto_includes_genex},\\$<SEMICOLON>-I>"
            ">"
        )
    endif()
    list(JOIN arg_PROTO_FILES "\\$<SEMICOLON>"  proto_files_string)
    if(arg_OPTIONS)
        list(JOIN arg_OPTIONS "\\\\$<SEMICOLON>" generation_options_string)
    else()
        set(generation_options_string "")
    endif()

    set(extra_protoc_args "")
    get_target_property(protoc_version WrapProtoc::WrapProtoc _qt_internal_protobuf_version)
    if(protoc_version VERSION_GREATER_EQUAL "3.12" AND protoc_version VERSION_LESS "3.15")
        list(APPEND extra_protoc_args "--experimental_allow_proto3_optional")
    endif()

    string(JOIN "\\$<SEMICOLON>" protoc_arguments
        ${extra_protoc_args}
        "--plugin=protoc-gen-${generator}=${generator_file}"
        "--${generator}_out=${tmp_output_directory}"
        "--${generator}_opt=${generation_options_string}"
        "${proto_files_string}"
        "${proto_includes_string}"
    )

    set(extra_copy_commands "")
    set(temporary_files "")
    if(NOT tmp_output_directory STREQUAL output_directory)
        foreach(f IN LISTS generated_files)
            get_filename_component(filename "${f}" NAME)
            if(IS_ABSOLUTE "${f}")
                file(RELATIVE_PATH f_rel "${output_directory}" "${f}")
            else()
                message(AUTHOR_WARNING
                    "Path to the generated file ${f} should be absolute, when \
                    calling _qt_internal_protoc_generate"
                )
            endif()
            list(APPEND temporary_files "${tmp_output_directory}/${f_rel}")
            list(APPEND extra_copy_commands COMMAND
                ${CMAKE_COMMAND} -E copy_if_different "${tmp_output_directory}/${f_rel}" "${f}")
        endforeach()
    endif()

    set(byproducts "")
    if(temporary_files)
        set(byproducts BYPRODUCTS ${temporary_files})
    endif()

    add_custom_command(OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND}
            -DPROTOC_EXECUTABLE=$<TARGET_FILE:WrapProtoc::WrapProtoc>
            "-DPROTOC_ARGS=${protoc_arguments}"
            -DWORKING_DIRECTORY=${output_directory}
            -DGENERATOR_NAME=${generator}
            -P
            ${__qt_protobuf_macros_module_base_dir}/QtProtocCommandWrapper.cmake
        ${extra_copy_commands}
        ${byproducts}
        WORKING_DIRECTORY ${output_directory}
        DEPENDS
            ${QT_CMAKE_EXPORT_NAMESPACE}::${generator}
            ${proto_files}
            $<TARGET_FILE:WrapProtoc::WrapProtoc>
        COMMENT "Generating QtProtobuf ${target} sources for ${generator}..."
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
    add_custom_target(${deps_target} DEPENDS ${generated_files})
    set_property(TARGET ${target} APPEND PROPERTY
        AUTOGEN_TARGET_DEPENDS "${deps_target}")
    set_property(TARGET ${target} APPEND PROPERTY AUTOMOC_MACRO_NAMES "Q_PROTOBUF_OBJECT")
    set_property(TARGET ${target} PROPERTY _qt_${generator}_deps_num "${num_deps}")
    _qt_internal_set_source_file_generated(SOURCES ${generated_files})

    get_target_property(proto_files ${target} _qt_internal_proto_files)
    if(NOT proto_files)
        set(proto_files "")
    endif()
    list(APPEND proto_files "${arg_PROTO_FILES}")
    list(REMOVE_DUPLICATES proto_files)
    set_target_properties(${target} PROPERTIES _qt_internal_proto_files "${proto_files}")

    target_include_directories(${target} PUBLIC "$<BUILD_INTERFACE:${output_directory}>")
endfunction()

# The function looks for the enum and message definitions inside provided proto files and returns
# list of the absolute .proto file paths, protobuf include paths and packages found here.
function(_qt_internal_protobuf_preparse_proto_files target base_dir
    out_proto_files out_proto_includes out_proto_packages)

    set(proto_files "")
    set(proto_includes "")
    set(proto_packages "")
    foreach(f IN LISTS ARGN)
        if(NOT IS_ABSOLUTE "${f}")
            set(f "${base_dir}/${f}")
            get_filename_component(f "${f}" ABSOLUTE)
        endif()
        get_filename_component(f "${f}" REALPATH)

        if("${f}" IN_LIST proto_files)
            message(WARNING "The .proto file ${f} is listed twice for ${target}."
                " Skip processing for the second time.")
            continue()
        endif()

        _qt_internal_preparse_proto_file_common(result proto_package "${f}" "message;enum")
        if(NOT result)
            message(STATUS "No messages or enums found in ${f}. Skipping.")
            continue()
        endif()

        get_filename_component(proto_file_base_dir "${f}" DIRECTORY)
        list(APPEND proto_files "${f}")
        list(APPEND proto_includes "${proto_file_base_dir}")
        list(APPEND proto_packages "${proto_package}")
    endforeach()

    set(${out_proto_files} "${proto_files}" PARENT_SCOPE)
    set(${out_proto_includes} "${proto_includes}" PARENT_SCOPE)
    set(${out_proto_packages} "${proto_packages}" PARENT_SCOPE)
endfunction()

function(_qt_internal_protobuf_package_qml_uri out_uri)
    list(REMOVE_DUPLICATES ARGN)
    list(LENGTH ARGN length)
    if(NOT length EQUAL 1)
        string(JOIN "\n" proto_packages_string "${ARGN}")
        message(FATAL_ERROR "All *.proto files must have single package name,"
            " that will be used for QML plugin registration."
            "\nThe following packages found in the .proto files for ${target}:"
            "\n${proto_packages_string}."
            " Please split the ${target} target per package."
        )
    endif()
    list(GET ARGN 0 qml_uri)
    set(${out_uri} ${qml_uri} PARENT_SCOPE)
endfunction()

function(_qt_internal_protoc_get_export_macro_filename out_filename target)
    # Export filename is always based on target name.
    string(TOLOWER "${target}" target_lower)
    set(${out_filename} "${target_lower}_exports.qpb.h" PARENT_SCOPE)
endfunction()

function(_qt_internal_protoc_generate_cpp_exports out_generated_file out_generation_options target
    export_macro)

    # Add EXPORT_MACRO if the target is a shared library
    string(TOUPPER "${target}" target_upper)
    get_target_property(export_macro_previous ${target} _qt_internal_protobuf_export_macro)

    # This is not the first time we enter this function for the target.
    if(export_macro_previous)
        if(export_macro AND NOT "${export_macro}" STREQUAL "${export_macro_previous}")
            message(FATAL_ERROR "EXPORT_MACRO argument doesn't match the one that already"
                "used for ${target}.\n"
                "Previous: ${export_macro_previous}\n"
                "New: ${export_macro}"
            )
        endif()
        set(export_macro "${export_macro_previous}")
        set(skip_generating TRUE)
    else()
        set(skip_generating FALSE)
    endif()

    if(NOT export_macro)
        string(MAKE_C_IDENTIFIER "${target_upper}" target_sanitized)
        set(export_macro "${target_sanitized}")
    endif()

    string(MAKE_C_IDENTIFIER "${export_macro}" export_macro_sanitized)
    if(NOT "${export_macro}" STREQUAL "${export_macro_sanitized}")
        message(FATAL_ERROR "EXPORT_MACRO should be a valid C identifier.")
    endif()

    _qt_internal_protoc_get_export_macro_filename(export_macro_filename ${target})

    if(skip_generating)
        # Tell the generator that we have export macro but we don't want to generate exports,
        # since they were generated in previous qt_add_<protobuf|grpc> call.
        set(${out_generation_options}
            "EXPORT_MACRO=${export_macro}:${export_macro_filename}:false")

        # Avoid scheduling the file generating twice
        set(export_macro_filename "")
    else()
        set(${out_generation_options}
            "EXPORT_MACRO=${export_macro}:${export_macro_filename}:true")

        set_target_properties(${target} PROPERTIES
            _qt_internal_protobuf_export_macro "${export_macro}")

            # Define this so we can conditionally set the export macro behavior
        target_compile_definitions(${target}
            PRIVATE "QT_BUILD_${export_macro}_LIB")
    endif()

    set(${out_generated_file} "${export_macro_filename}" PARENT_SCOPE)
    set(${out_generation_options} "${${out_generation_options}}" PARENT_SCOPE)
endfunction()

# TODO Qt6:
#     - Collect PROTO_INCLUDES from the LINK_LIBRARIES property of TARGET
#     - Collect proto files from the source files of the ${TARGET}

function(qt6_add_protobuf target)
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

    _qt_internal_get_protoc_options(generation_options arg
        protoc_option_opt protoc_single_opt protoc_multi_opt)

    if(arg_ALLOW_MUTABLE_GETTER_CONFLICTS)
        set(moc_target ${QT_CMAKE_EXPORT_NAMESPACE}::moc)
        if(TARGET ${moc_target})
        _qt_internal_dealias_target(moc_target)
            get_target_property(moc_version ${moc_target} _qt_package_version)
        else()
            message(AUTHOR_WARNING "Unable to determine the moc version. Qt installation might be"
                " incomplete.")
            set(moc_version 0)
        endif()

        # TODO: The Q_PROPERTY related issue QTBUG-119912 is not fixed, so we prohibit the use of
        # ALLOW_MUTABLE_GETTER_CONFLICTS until then. The property will remain undocumented and for
        # the internal use only. Replace '99' with the proper Qt version once the issue in moc is
        # fixed.
        if(moc_version VERSION_LESS 99 AND NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
            message(FATAL_ERROR "The use of 'ALLOW_MUTABLE_GETTER_CONFLICTS' is prohibited.")
        endif()
    endif()

    if(arg_QML_URI AND NOT arg_QML)
        message(FATAL_ERROR "QML_URI requires the QML option set, "
            "but the QML argument is not provided.")
    endif()

    if(arg_PROTO_FILES_BASE_DIR)
        set(base_dir "${arg_PROTO_FILES_BASE_DIR}")
    else()
        set(base_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(arg_HEADER_GUARD)
        if(NOT arg_HEADER_GUARD MATCHES "^(pragma|filename)$")
            message(FATAL_ERROR "Invalid HEADER_GUARD type specified ${arg_HEADER_GUARD}."
                "Supported types: pragma, filename.")
        endif()
    endif()

    _qt_internal_protobuf_preparse_proto_files(${target}
        "${base_dir}"
        proto_files proto_includes proto_packages
        ${arg_PROTO_FILES}
    )

    if(NOT proto_files AND arg_PROTO_FILES)
        _qt_internal_protobuf_missing_definitions_warning(${target} protobuf "${arg_PROTO_FILES}")
        return()
    endif()

    set(output_directory "${CMAKE_CURRENT_BINARY_DIR}")
    if(DEFINED arg_OUTPUT_DIRECTORY)
        set(output_directory "${arg_OUTPUT_DIRECTORY}")
    endif()

    if(TARGET ${target})
        get_target_property(target_protos ${target} QT_PROTOBUF_PROTO_FILES)
        if(NOT target_protos)
            set(target_protos "")
        endif()
    else()
        set(target_protos "")
    endif()

    set(extra_include_directories "")
    set(cpp_sources "")
    set(idx 0)
    foreach(f IN LISTS proto_files)
        if(arg_GENERATE_PACKAGE_SUBFOLDERS)
            if(proto_packages)
                list(GET proto_packages ${idx} package)
            else()
                set(package "")
            endif()
            string(REPLACE "." "/" package_full_path "${package}/")
            math(EXPR idx "${idx} + 1")
            list(APPEND extra_include_directories
                "$<BUILD_INTERFACE:${output_directory}/${package_full_path}>")
        else()
            list(APPEND extra_include_directories
                "$<BUILD_INTERFACE:${output_directory}>")
            set(package_full_path "")
        endif()

        get_filename_component(proto_file_name "${f}" NAME)
        foreach(proto_file_in_list IN LISTS target_protos)
            if("${proto_file_in_list}" MATCHES "(^|/)${proto_file_name}($|;)")
                message(FATAL_ERROR "The file name ${proto_file_name} is"
                        " added more than once in the ${target}."
                        " This is not supported by protoc."
                        " Please, add a separate protobuf target to generate code from ${f}."
                )
            endif()
        endforeach()
        list(APPEND target_protos "${f}")

        get_filename_component(basename "${f}" NAME_WLE)
        list(APPEND cpp_sources
            "${output_directory}/${package_full_path}${basename}.qpb.h"
            "${output_directory}/${package_full_path}${basename}.qpb.cpp"
        )

        list(APPEND type_registrations
            "${output_directory}/${package_full_path}${basename}_qtprotoreg.cpp")
    endforeach()

    if(TARGET ${target})
        get_target_property(existing_proto_packages ${target} QT_PROTOBUF_PACKAGES)
        if(NOT existing_proto_packages)
            set(existing_proto_packages "")
        endif()
    else()
        set(existing_proto_packages "")
    endif()

    list(APPEND existing_proto_packages ${proto_packages})
    list(REMOVE_DUPLICATES existing_proto_packages)

    set(qml_sources "")
    if(arg_QML)
        if(TARGET ${target})
            get_target_property(existing_uri ${target} QT_QML_MODULE_URI)
        else()
            set(existing_uri "")
        endif()

        if(existing_uri)
            if(arg_QML_URI AND NOT "${existing_uri}" STREQUAL "${arg_QML_URI}")
                message(WARNING "qt_add_protobuf is called with QML argument and for the existing"
                " QML module target ${target}.The Protobuf generator will use the URI provided"
                " by the QML module target: ${existing_uri}, instead of the manually specified"
                " QML_URI: ${arg_QML_URIs}")
            endif()

            # Prevent mixing multiple protobuf packages in single QML module
            if(existing_proto_packages)
                _qt_internal_protobuf_package_qml_uri(dummy ${existing_proto_packages})
            endif()

            set(qml_uri "${existing_uri}")
        elseif(arg_QML_URI)
            set(qml_uri "${arg_QML_URI}")
        elseif(existing_proto_packages)
            _qt_internal_protobuf_package_qml_uri(qml_uri ${existing_proto_packages})
        else()
            message(FATAL_ERROR ".proto files of ${target} don't specify a package."
                " Please, set QML_URI when using .proto without package name."
            )
        endif()
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

    set_target_properties(${target} PROPERTIES QT_PROTOBUF_PACKAGES "${existing_proto_packages}")

    set_target_properties(${target} PROPERTIES QT_PROTOBUF_PROTO_FILES "${target_protos}")

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
            list(APPEND cpp_sources "${output_directory}/${generated_export}")
        endif()
        list(APPEND generation_options "${generated_export_options}")
    endif()

    _qt_internal_protoc_generate(${target} qtprotobufgen "${output_directory}"
        PROTO_FILES ${proto_files}
        PROTO_INCLUDES ${proto_includes}
        GENERATED_FILES ${cpp_sources} ${qml_sources} ${type_registrations}
        OPTIONS ${generation_options}
    )

    # Filter generated headers
    unset(generated_headers)
    foreach(generated_file IN LISTS cpp_sources)
        get_filename_component(extension "${generated_file}" LAST_EXT)
        if(extension STREQUAL ".h")
            list(APPEND generated_headers "${generated_file}")
        endif()
    endforeach()

    target_sources(${target} PRIVATE ${cpp_sources} ${qml_sources})

    get_property(is_use_protobuf_list_aliases_set TARGET ${target}
        PROPERTY QT_USE_PROTOBUF_LIST_ALIASES SET)
    if(NOT is_use_protobuf_list_aliases_set)
        set_target_properties(${target}
            PROPERTIES
                QT_USE_PROTOBUF_LIST_ALIASES TRUE
        )
    endif()

    set_target_properties(${target}
        PROPERTIES
            AUTOMOC ON
    )

    target_compile_definitions(${target} PUBLIC
        $<$<BOOL:$<TARGET_PROPERTY:QT_USE_PROTOBUF_LIST_ALIASES>>:QT_USE_PROTOBUF_LIST_ALIASES>)

    if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(${target}
                PRIVATE "/Zc:__cplusplus" "/permissive-" "/bigobj")
        elseif(MINGW)
            target_compile_options(${target}
                PRIVATE "-Wa,-mbig-obj")
        endif()
    endif()

    # TODO: adding these include paths might cause the ambiguous include handling if
    # two different packages contain messages with the same name. This should be fixed
    # in moc and qmltypesregistar, see QTBUG-115499.
    target_include_directories(${target} PRIVATE ${extra_include_directories})
    target_link_libraries(${target} PRIVATE
        ${QT_CMAKE_EXPORT_NAMESPACE}::Protobuf
    )

    if(is_shared)
        _qt_internal_protoc_get_export_macro_filename(export_macro_filename ${target})
        set(export_macro_file "${output_directory}/${export_macro_filename}")
    endif()

    set_source_files_properties(${type_registrations} PROPERTIES SKIP_AUTOGEN ON)
    if(is_static OR (WIN32 AND NOT is_executable))
        if(TARGET ${target}_qtprotoreg)
            target_sources(${target}_qtprotoreg PRIVATE ${type_registrations})
        else()
            add_library(${target}_qtprotoreg OBJECT ${type_registrations})
            if(export_macro_file)
                target_sources(${target}_qtprotoreg PRIVATE ${export_macro_file})
            endif()

            target_link_libraries(${target}
                INTERFACE "$<TARGET_OBJECTS:$<TARGET_NAME:${target}_qtprotoreg>>")
            add_dependencies(${target} ${target}_qtprotoreg)

            get_target_property(num_deps ${target} _qt_qtprotobufgen_deps_num)
            if(num_deps)
                # foreach includes the last element in the RANGE
                math(EXPR num_deps "${num_deps} - 1")
                foreach(i RANGE 0 ${num_deps})
                    _qt_internal_get_generator_dep_target_name(deps_target ${target}
                        qtprotobufgen ${i})
                    add_dependencies(${target}_qtprotoreg ${deps_target})
                endforeach()
            endif()

            target_include_directories(${target}_qtprotoreg
                PRIVATE "$<GENEX_EVAL:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>>")
            target_link_libraries(${target}_qtprotoreg
                PRIVATE
                    ${QT_CMAKE_EXPORT_NAMESPACE}::Platform
                    ${QT_CMAKE_EXPORT_NAMESPACE}::Protobuf
                    $<GENEX_EVAL:$<TARGET_PROPERTY:${target},LINK_LIBRARIES>>
            )

            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                target_compile_options(${target}_qtprotoreg
                    PRIVATE "/Zc:__cplusplus" "/permissive-" "/bigobj")
            endif()
        endif()
        if(DEFINED arg_OUTPUT_TARGETS)
            list(APPEND ${arg_OUTPUT_TARGETS}
                "${target}_qtprotoreg")
        endif()
    else()
        target_sources(${target} PRIVATE ${type_registrations})
        if(export_macro_file)
            target_sources(${target} PRIVATE ${export_macro_file})
        endif()
    endif()

    if(arg_QML AND NOT existing_uri)
        string(REPLACE "." "/" qml_module_output_path "${qml_uri}")
        set(qml_module_output_full_path "${CMAKE_CURRENT_BINARY_DIR}/${qml_module_output_path}")

        if(NOT is_executable)
            set(plugin_options PLUGIN_TARGET "${target}plugin")
        endif()

        qt_policy(SET QTP0001 NEW)
        qt6_add_qml_module(${target}
            URI ${qml_uri}
            ${plugin_options}
            VERSION 1.0
            OUTPUT_DIRECTORY "${qml_module_output_full_path}"
            DEPENDENCIES QtProtobuf
            OUTPUT_TARGETS qml_output_targets
        )

        if(TARGET ${target}plugin)
            set_target_properties(${target}plugin
                PROPERTIES
                    AUTOMOC ON
            )
            target_link_libraries(${target}plugin PRIVATE
                ${QT_CMAKE_EXPORT_NAMESPACE}::Protobuf
            )
        endif()

        if(NOT TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::ProtobufQuick)
            message(FATAL_ERROR "QML option of the qt_add_protobuf command requires"
                " ${QT_CMAKE_EXPORT_NAMESPACE}::ProtobufQuick target. Please make sure that you"
                " have the respective Qt component found by adding it to the find_package call:"
                "   find_package(${QT_CMAKE_EXPORT_NAMESPACE} COMPONENTS ProtobufQuick)")
        endif()
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::ProtobufQuick
        )

        if(DEFINED arg_OUTPUT_TARGETS)
            if(qml_output_targets)
                list(APPEND ${arg_OUTPUT_TARGETS} ${qml_output_targets})
            endif()
            if(TARGET ${target}plugin)
                list(APPEND ${arg_OUTPUT_TARGETS} "${target}plugin")
            endif()
        endif()
    elseif(existing_uri)
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::ProtobufQuick
        )
    endif()

    if(DEFINED arg_OUTPUT_HEADERS)
        set(${arg_OUTPUT_HEADERS} "${generated_headers}" PARENT_SCOPE)
    endif()

    if(DEFINED arg_OUTPUT_TARGETS)
        list(REMOVE_DUPLICATES ${arg_OUTPUT_TARGETS})
        set(${arg_OUTPUT_TARGETS} "${${arg_OUTPUT_TARGETS}}" PARENT_SCOPE)
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_protobuf)
        if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
            set(single_out_args OUTPUT_HEADERS OUTPUT_TARGETS)

            cmake_parse_arguments(PARSE_ARGV 1 arg "" "${single_out_args}" "")
            qt6_add_protobuf(${ARGN})
            foreach(out_arg IN LISTS single_out_args)
                if(arg_${out_arg})
                    set(${arg_${out_arg}} "${${arg_${out_arg}}}" PARENT_SCOPE)
                endif()
            endforeach()
        else()
            message(FATAL_ERROR "qt6_add_protobuf() is only available in Qt 6. "
                                "Please check the protobuf documentation for alternatives.")
        endif()
    endfunction()
endif()

# The common parsing function looking for the 'lookup_keys' definitions inside the 'proto_file'.
# The function sets the 'out_result' variable to true if one of 'lookup_keys' is found. Also the
# function writes to the 'out_package' variable the package name that the .proto file belongs to.
function(_qt_internal_preparse_proto_file_common out_result out_package proto_file lookup_keys)
    if(NOT proto_file OR NOT EXISTS "${proto_file}")
        message(FATAL_ERROR "Unable to scan '${proto_file}': file doesn't exist.")
    endif()

    file(READ "${proto_file}" file_content)
    if(NOT file_content)
        message(FATAL_ERROR "Unable to read ${proto_file}, or file is empty.")
    endif()

    string(REPLACE "[" "" file_content "${file_content}")
    string(REPLACE "]" "" file_content "${file_content}")
    string(REPLACE ";" "[[;]]" file_content "${file_content}")
    string(REGEX REPLACE "([^\t \n]+[\t ]*)}" "\\1;}" file_content "${file_content}")
    string(REGEX REPLACE "{([\t ]*[^\t \n]+)" "{;\\1" file_content "${file_content}")
    string(REPLACE "\n" ";" file_content "${file_content}")
    set(proto_key_common_regex "[\t ]+([a-zA-Z0-9_]+)")

    set(unclosed_braces 0)
    set(in_message_scope FALSE)

    set(found_key FALSE)
    list(JOIN lookup_keys "|" lookup_keys_regex)
    foreach(item IN LISTS file_content)
        if(item MATCHES "^[\t ]*package[\t ]+([a-zA-Z0-9_.-]+)")
            set(proto_package "${CMAKE_MATCH_1}")
        elseif(item MATCHES "^[\t ]*(${lookup_keys_regex})${proto_key_common_regex}")
            set(found_key TRUE)
            break()
        endif()
        if(in_message_scope)
            if(item MATCHES "[^/]*\\{")
                math(EXPR unclosed_braces "${unclosed_braces} + 1")
            endif()
            if(item MATCHES "[^/]*\\}")
                math(EXPR unclosed_braces "${unclosed_braces} - 1")
                if(unclosed_braces EQUAL 0)
                    set(in_message_scope FALSE)
                endif()
            endif()
        endif()
    endforeach()

    set(${out_package} "${proto_package}" PARENT_SCOPE)
    set(${out_result} "${found_key}" PARENT_SCOPE)
endfunction()

function(_qt_internal_protobuf_missing_definitions_warning target generator_type proto_files)
    if(TARGET ${target})
        set(warning_action "adding")
    else()
        set(warning_action "extending")
    endif()

    if(NOT QT_SKIP_PROTOBUF_MISSING_DEFINITIONS_WARNING)
        message(WARNING "PROTO_FILES ${proto_files} do not contain code for ${generator_type}"
            " generator. Skipping ${warning_action} the ${target} target.")
    endif()
endfunction()
