# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# The function generates Qt classes from the .proto schema of the well-know types passed in 'ARGN'
# and adds the generated sources to the 'target'
function(qt_internal_add_protobuf_wellknown_types target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "TYPES")

    if(NOT arg_TYPES)
        message("The TYPES argument is missing.")
    endif()

    set(lookup_dirs "")
    if(TARGET protobuf::libprotobuf)
        get_target_property(lookup_dirs protobuf::libprotobuf
            INTERFACE_INCLUDE_DIRECTORIES)
    endif()

    if(QT_PROTOBUF_WELL_KNOWN_TYPES_PROTO_DIR)
        list(PREPEND lookup_dirs "${QT_PROTOBUF_WELL_KNOWN_TYPES_PROTO_DIR}")
        set(extra_find_path_args NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH)
    else()
        set(extra_find_path_args "")
    endif()
    # It's not possible to generate all the well-known types in one chunk, because of complicated
    # dependencies between the .proto files. So we use separate qt_add_protobuf(protoc) calls for
    # them.
    set(generated_headers "")
    set(generated_targets "")
    set(types_found "")
    foreach(type IN LISTS arg_TYPES)
        set(proto_file_name "google/protobuf/${type}.proto")
        unset(proto_file_dir)
        find_path(proto_file_dir
            NAMES "${proto_file_name}"
            PATHS ${lookup_dirs}
            NO_CACHE
            ${extra_find_path_args}
        )
        if(NOT proto_file_dir)
            message(AUTHOR_WARNING "The .proto schema ${proto_file_name} is not found. This can"
                " happen because an incompatible protobuf version is used. Please file a bug on"
                " https://bugreports.qt.io/, so we can adjust the implementation accordingly.")
            continue()
        endif()
        list(APPEND types_found "${proto_file_name}")

        message(DEBUG "Adding well-known type ${proto_file_name}")

        set(proto_file "${proto_file_dir}/${proto_file_name}")
        qt6_add_protobuf(${target}
            PROTO_FILES "${proto_file}"
            PROTO_INCLUDES "${proto_file_dir}"
            OUTPUT_HEADERS type_generated_headers
            OUTPUT_TARGETS type_generated_targets
            GENERATE_PACKAGE_SUBFOLDERS
            GENERATE_NON_FINAL_MESSAGES
        )
        list(APPEND generated_headers ${type_generated_headers})
        list(APPEND generated_targets ${type_generated_targets})

        get_target_property(known_proto_includes ${target} QT_PROTO_INCLUDES)
        if(NOT known_proto_includes)
            set(known_proto_includes "")
        endif()
        list(APPEND known_proto_includes "${proto_file_dir}")
        list(REMOVE_DUPLICATES known_proto_includes)
        set_target_properties(${target} PROPERTIES
            QT_PROTO_INCLUDES "${known_proto_includes}")
        # Assume that ProtobufWellKnownTypes is the Qt module that should have _sync_headers.
        foreach(generated_target IN LISTS generated_targets)
            add_dependencies(${generated_target} ${target}_sync_headers)
        endforeach()
    endforeach()

    if(generated_headers)
        # The generated header files need to be accessible by both the ${target} module include
        # path and the expected protobuf import path 'google/protobuf'. Instead of making copies,
        # let's create the simple aliases.
        qt_internal_module_info(module ${target})

        get_target_property(is_fw ${target} FRAMEWORK)
        if(is_fw)
            qt_internal_get_framework_info(fw ${target})
        endif()

        set(alias_headers "")
        foreach(header IN LISTS generated_headers)
            get_filename_component(file_name "${header}" NAME)
            set(alias_header "${module_build_interface_include_dir}/google/protobuf/${file_name}")
            qt_internal_generate_wellknown_header_alias(
                "${module_build_interface_include_dir}/${file_name}" "${alias_header}")
            if(is_fw)
                qt_internal_get_framework_info(fw ${target})
                get_target_property(output_dir ${target} LIBRARY_OUTPUT_DIRECTORY)
                set(output_dir "${output_dir}/${fw_versioned_header_dir}")
                set(fw_alias_dir "${output_dir}/google/protobuf")
                set(fw_alias_header "${fw_alias_dir}/${file_name}")
                add_custom_command(
                    OUTPUT "${fw_alias_header}"
                    DEPENDS ${in_file_path}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${fw_alias_dir}"
                    COMMAND ${CMAKE_COMMAND} -E copy
                        "${alias_header}"
                        "${fw_alias_dir}"
                    VERBATIM
                )
                set_property(TARGET ${target} APPEND PROPERTY
                    QT_COPIED_FRAMEWORK_HEADERS "${fw_alias_header}")
            else()
                list(APPEND alias_headers "${alias_header}")
            endif()
        endforeach()

        if(NOT is_fw)
            qt_install(FILES ${alias_headers}
                DESTINATION "${module_install_interface_include_dir}/google/protobuf")
        endif()
    endif()

    if(generated_targets)
        list(REMOVE_DUPLICATES generated_targets)
        qt_install(TARGETS ${generated_targets}
            EXPORT "${INSTALL_CMAKE_NAMESPACE}${target}Targets"
            DESTINATION "${INSTALL_LIBDIR}"
        )

        qt_internal_add_targets_to_additional_targets_export_file(
            TARGETS ${generated_targets}
            EXPORT_NAME_PREFIX "${INSTALL_CMAKE_NAMESPACE}${target}"
        )
    endif()
    set_property(TARGET ${target} APPEND PROPERTY
        _qt_internal_protobuf_wellknown_types ${types_found})
    set_property(TARGET ${target} APPEND PROPERTY
        EXPORT_PROPERTIES _qt_internal_protobuf_wellknown_types)

    set(export_name "${INSTALL_CMAKE_NAMESPACE}${target}")
    set(wellknown_types_extras "${export_name}Extras.cmake")

    qt_path_join(config_build_dir ${QT_CONFIG_BUILD_DIR} ${export_name})
    qt_path_join(config_install_dir ${QT_CONFIG_INSTALL_DIR} ${export_name})
    set(module_extra_properties_file "${config_build_dir}/${wellknown_types_extras}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/QtProtobufWellKnownTypesExtras.cmake.in"
        "${module_extra_properties_file}" @ONLY)

    qt_install(FILES "${module_extra_properties_file}" DESTINATION "${config_install_dir}")

    qt_internal_protobuf_generate_properties(${target})
endfunction()

# The function generates the header 'alias_file' containing the include of the original
# 'header_file' by the relative file path.
function(qt_internal_generate_wellknown_header_alias header_file alias_file)
    get_filename_component(alias_directory "${alias_file}" DIRECTORY)
    get_filename_component(header_name "${header_file}" NAME)
    file(RELATIVE_PATH rel_path ${alias_directory} "${header_file}")
    set(content "#include \"${rel_path}\"")
    qt_configure_file(OUTPUT "${alias_file}" CONTENT "${content}")
endfunction()
