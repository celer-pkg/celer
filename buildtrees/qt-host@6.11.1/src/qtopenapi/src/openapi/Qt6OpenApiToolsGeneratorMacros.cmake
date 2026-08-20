# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(__qt_openapi_macros_module_base_dir "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

function(_qt_internal_openapi_detect_target_type target
    out_is_shared out_is_static out_is_executable)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "SHARED_LIBRARY"
            OR target_type STREQUAL "MODULE_LIBRARY")
        set(is_shared TRUE)
    elseif(target_type STREQUAL "STATIC_LIBRARY")
        set(is_static TRUE)
    elseif(target_type STREQUAL "EXECUTABLE")
        set(is_executable TRUE)
    else()
        message(FATAL_ERROR "Unsupported target type '${target_type}'.")
    endif()

    set(${out_is_shared} "${is_shared}" PARENT_SCOPE)
    set(${out_is_static} "${is_static}" PARENT_SCOPE)
    set(${out_is_executable} "${is_executable}" PARENT_SCOPE)
endfunction()

function(qt6_add_openapi_client target)
    set(options
        GENERATE_DOCUMENTATION
        __QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET
    )
    set(oneValueArgs
        SPEC_FILE
        OUTPUT_DIRECTORY
        DOCUMENTATION_OUTPUT_DIRECTORY
        OUTPUT_PUBLIC_HEADERS_DIR
        OUTPUT_PRIVATE_HEADERS_DIR
    )
    set(multiValueArgs
        ADDITIONAL_PROPERTIES
        GENERATE_OPTIONS
        JAVA_OPTIONS
    )
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    if(NOT arg_SPEC_FILE)
        message(FATAL_ERROR "qt6_add_openapi_client: SPEC_FILE is required.")
    endif()

    if(NOT arg_OUTPUT_DIRECTORY)
        set(arg_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if(NOT TARGET "${target}")
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "Target '${target}' does not exist. "
            "Please create it before calling qt6_add_openapi_client. "
            "You can use qt_add_library or qt_add_executable to create the target.")
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET AND NOT QT_BUILDING_QT)
        message(FATAL_ERROR "qt6_add_openapi_client: "
            "__QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET is an internal option, and shouldn't be "
            "used outside of the Qt build.")
    endif()

    if (NOT TARGET "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar")
        message(FATAL_ERROR
            "Can't find ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar target, "
            "please report an issue on the Qt bugtracker")
        return()
    endif()

    get_target_property(openapi_generator_cli_jar_file
        WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE_OPENAPI_GENERATOR_CLI_JAR)
    if(NOT openapi_generator_cli_jar_file)
        get_target_property(openapi_generator_cli_exec_file
            WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE_OPENAPI_GENERATOR_CLI_EXECUTABLE)
        if (NOT openapi_generator_cli_exec_file)
            message(FATAL_ERROR "qt6_add_openapi_client: "
                "Java OpenAPI generator is not installed or not added in PATH.")
        endif()
    endif()

    file(MAKE_DIRECTORY "${arg_OUTPUT_DIRECTORY}")
    if(NOT arg_DOCUMENTATION_OUTPUT_DIRECTORY)
        set(arg_DOCUMENTATION_OUTPUT_DIRECTORY "${arg_OUTPUT_DIRECTORY}")
    endif()
    set(generator_name ${QT_OPENAPI_GENERATOR_NAME})
    get_target_property(generator_jar_path
        "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" IMPORTED_LOCATION)

    # Qt pre-generated common library always uses QOAI
    # prefix, for generating different common library,
    # please call the generator manually.
    set(qt_commonlib_prefix "QOAI")
    set(qt_common_file_prefix "qoai")
    # Qt pre-generated common library always uses QtOpenApiCommon namespace
    set(cpp_common_namespace "QtOpenApiCommon")

    # The default commonLibGenerationType is defined in CppQt6ClientGenerator.java:
    # String commonLibrary = GENERATION_TYPE.COMMON_LIB.value;
    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(common_lib_generation_type "Gen-Common-Lib")
    else()
        set(common_lib_generation_type "Gen-Client-Lib")
    endif()

    set(common_lib_name "CommonLibrary")

    # The common library always needs compression support.
    set(compression_required "true")

    string(JOIN "," fixed_additional_properties
        "--additional-properties=useCmakeMacro=true"
        "packageName=${target}"
        "cppCommonNamespace=${cpp_common_namespace}"
        "prefix=${qt_commonlib_prefix}"
        "commonLibrary=${common_lib_generation_type}"
        "commonLibraryName=${common_lib_name}"
        "contentCompression=${compression_required}"
    )

    if(arg_ADDITIONAL_PROPERTIES)
        string(JOIN "," users_additional_properties ${arg_ADDITIONAL_PROPERTIES})
        string(PREPEND users_additional_properties "--additional-properties=")
    else()
        set(users_additional_properties "")
    endif()

    set(generating_sources "")
    set(common_sources "")
    set(client_sources "")
    set(common_dir "common")
    set(client_dir "client")
    set(target_lower_case ${target})
    string(TOLOWER ${target} target_lower_case)
    set(client_dir_path "${arg_OUTPUT_DIRECTORY}/${client_dir}/")
    set(common_dir_path "${arg_OUTPUT_DIRECTORY}/${common_dir}/")

    set(extra_dependencies
        DEPENDS
            "${generator_jar_path}"
            "${arg_SPEC_FILE}"
    )
    if(TARGET QtOpenAPIGenerator)
        # otherwise add_custom_command tries to run generator, that is not created yet
        list(APPEND extra_dependencies
            QtOpenAPIGenerator
        )
    endif()

    # if target is already created, no need to use package_name option for generator
    if(TARGET ${target})
        set(target_lib_name "")
    else()
        set(target_lib_name "--package-name=${target}")
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(comment "Generating the Qt6 Common Library code with the generator: ${generator_name}")
    else()
        string(CONCAT comment
            "Generating the Qt6 Client code for ${target} with the generator: ${generator_name}."
            "\nThe specification file is: ${arg_SPEC_FILE}"
        )
    endif()

    if(openapi_generator_cli_jar_file)
        if(CMAKE_HOST_WIN32)
            set(path_separator "\\;")
        else()
            set(path_separator ":")
        endif()

        set(openapi_cli_entrypoint_class "org.openapitools.codegen.OpenAPIGenerator")
        set(run_client_cmd
            "${generator_jar_path}${path_separator}${openapi_generator_cli_jar_file}")
        set(generation_command java ${arg_JAVA_OPTIONS} -cp "${run_client_cmd}" "${openapi_cli_entrypoint_class}")
        list(APPEND extra_dependencies "${openapi_generator_cli_jar_file}")
    elseif(openapi_generator_cli_exec_file)
        # When using the jar, logback.xml is included in it and picked up automatically.
        # When using the cli executable, there is no jar on the classpath, so we pass
        # the logback config explicitly via JAVA_OPTS. The user's JAVA_OPTIONS come
        # after, so they can still override it.
        get_filename_component(generator_logback_config
            "${__qt_openapi_macros_module_base_dir}/../tools/qtopenapi-generator/src/main/resources/logback.xml"
            REALPATH)
        set(backup_java_opts "$ENV{JAVA_OPTS}")
        list(PREPEND arg_JAVA_OPTIONS "-Dlogback.configurationFile=${generator_logback_config}")
        list(JOIN arg_JAVA_OPTIONS " " java_options_str)
        set(ENV{JAVA_OPTS}
            "${backup_java_opts} ${java_options_str}")

        set(generation_command
            "${openapi_generator_cli_exec_file}" --custom-generator="${generator_jar_path}")
        list(APPEND extra_dependencies "${openapi_generator_cli_exec_file}")
    endif()

    # We *do not* want to regenerate the sources if the user changes something
    # manually in the generated files, but we want to regenerate the sources if
    # the files got removed.
    set(need_call_generator FALSE)

    # The generator writes the list of generated files into
    # ${arg_OUTPUT_DIRECTORY}/.openapi-generator/FILES
    # Check if this file exists, and if it does, read its
    # content and verify that all the listed files exist.
    set(generated_files_file "${arg_OUTPUT_DIRECTORY}/.openapi-generator/FILES")
    if(NOT EXISTS "${generated_files_file}")
        message(DEBUG "File does not exist: ${generated_files_file}")
        # the file does not exist
        set(need_call_generator TRUE)
    else()
        # add this file to the dependencies, so that CMake automatically
        # reconfigures if it's gone
        set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${generated_files_file}")
        # get the timestamp of the file - we'll use it later
        file(TIMESTAMP "${generated_files_file}" generated_files_file_timestamp)
        # read the file and get the list of generated files
        file(STRINGS "${generated_files_file}" generated_file_list)
        foreach(generated_file IN LISTS generated_file_list)
            if(NOT EXISTS "${arg_OUTPUT_DIRECTORY}/${generated_file}")
                message(DEBUG "File not found: ${arg_OUTPUT_DIRECTORY}/${generated_file}")
                set(need_call_generator TRUE)
                break()
            endif()
        endforeach()
    endif()

    # We depend on the following things:
    # * the yaml file;
    # * the Qt generator plugin (because newer version may do smth differently).
    # * the upstream OpenAPI generator

    # Handle changes in the yaml file. We need to check both the path
    # and the timestamp.
    if(NOT DEFINED QT_INTERNAL_OPENAPI_${target}_YAML_PATH)
        set(QT_INTERNAL_OPENAPI_${target}_YAML_PATH ""
            CACHE INTERNAL "Path to the yaml file for ${target}")
    endif()
    set(yaml_depfile "${arg_OUTPUT_DIRECTORY}/.openapi-generator/yaml_deps.txt")
    if(NOT ("${arg_SPEC_FILE}" STREQUAL "${QT_INTERNAL_OPENAPI_${target}_YAML_PATH}"))
        # Yaml file path has changed
        message(DEBUG "Yaml file path has changed"
                "\nOld path: ${QT_INTERNAL_OPENAPI_${target}_YAML_PATH}"
                "\nNew path: ${arg_SPEC_FILE}")
        set(need_call_generator TRUE)
        set(QT_INTERNAL_OPENAPI_${target}_YAML_PATH "${arg_SPEC_FILE}"
            CACHE INTERNAL "Path to the yaml file for ${target}" FORCE)
    elseif(generated_files_file_timestamp)
        # read the file with yaml deps. It also includes ${arg_SPEC_FILE}
        message(DEBUG "Reading ${yaml_depfile} to get the list of all yaml dependencies.")
        if(EXISTS "${yaml_depfile}")
            file(STRINGS "${yaml_depfile}" yaml_depfile_entries)
            foreach(yaml_file IN LISTS yaml_depfile_entries)
                if(EXISTS "${yaml_file}")
                    file(TIMESTAMP "${yaml_file}" curr_yaml_timestamp)
                    if("${generated_files_file_timestamp}" STRLESS "${curr_yaml_timestamp}")
                        message(DEBUG "${yaml_file} was modified after last check"
                        "\nModification time is: ${curr_yaml_timestamp}")
                        set(need_call_generator TRUE)
                        break() # we do not need to check all
                    endif()
                endif()
            endforeach()
        else()
            # At least check the timestamp of the ${arg_SPEC_FILE} yaml file
            message(DEBUG "Could not find ${yaml_depfile}."
                    "\nChecking the timestamp of ${arg_SPEC_FILE}")
            file(TIMESTAMP "${arg_SPEC_FILE}" curr_yaml_timestamp)
            if("${generated_files_file_timestamp}" STRLESS "${curr_yaml_timestamp}")
                message(DEBUG "Yaml file was modified after last check"
                        "\nModification time is: ${curr_yaml_timestamp}")
                set(need_call_generator TRUE)
            endif()
        endif()
    endif()

    # For the Qt6 generator, only check its timestamp.
    if(generated_files_file_timestamp)
        file(TIMESTAMP "${generator_jar_path}" qt_generator_timestamp)
        if("${generated_files_file_timestamp}" STRLESS "${qt_generator_timestamp}")
            message(DEBUG "Qt6 Generator was modified after last check")
            set(need_call_generator TRUE)
        endif()
    endif()

    # For the upstream generator, also check its timestamp only.
    # This covers the case when the upstream generator is updated
    # by some package manager.
    set(upstream_generator_path "${openapi_generator_cli_jar_file}")
    if(NOT upstream_generator_path)
        set(upstream_generator_path "${openapi_generator_cli_exec_file}")
    endif()
    if(generated_files_file_timestamp)
        file(TIMESTAMP "${upstream_generator_path}" upstream_generator_timestamp)
        if("${generated_files_file_timestamp}" STRLESS "${upstream_generator_timestamp}")
            message(DEBUG "Upstream OpenAPI generator was modified after last check")
            set(need_call_generator TRUE)
        endif()
    endif()
    set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${upstream_generator_path}")

    if(need_call_generator)
        message(STATUS "${comment}")
        set(extra_args "")
        if(NOT QT_FEATURE_developer_build)
            list(APPEND extra_args
                ERROR_VARIABLE generate_error
                OUTPUT_VARIABLE generate_output
            )
        endif()
        execute_process(
            COMMAND ${generation_command}
                generate -g ${generator_name}
                ${target_lib_name}
                ${users_additional_properties}
                ${arg_GENERATE_OPTIONS}
                -i "${arg_SPEC_FILE}"
                -o "${arg_OUTPUT_DIRECTORY}"
                ${fixed_additional_properties}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RESULT_VARIABLE generate_result
            ${extra_args}
        )
        if(openapi_generator_cli_exec_file)
            set(ENV{JAVA_OPTS} ${backup_java_opts})
        endif()
        if(NOT generate_result)
            # now we have a list of all yaml files - add them as dependencies
            if(EXISTS "${yaml_depfile}")
                # ${arg_SPEC_FILE} is also in the list
                file(STRINGS "${yaml_depfile}" yaml_depfile_entries)
                foreach(yaml_file IN LISTS yaml_depfile_entries)
                    set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${yaml_file}")
                endforeach()
            else()
                # at least add ${arg_SPEC_FILE} as a dependency
                set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${arg_SPEC_FILE}")
            endif()
            message(STATUS "Generation completed successfully for ${target}.")
        else()
            set(error_message "Generation failed. Exit code: ${generate_result}.")
            if(generate_output OR generate_error)
                string(APPEND error_message "\n stdout: '${generate_output}' "
                    "\n stderr: '${generate_error}'")
            endif()
            message(FATAL_ERROR "${error_message}")
        endif()
    else()
        message(STATUS "Skipping the generation step for ${target}, because nothing has changed.")
    endif() # need_call_generator

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}${target_lower_case}commonexports.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}baseapi.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}baseapi.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}commonglobal.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}helpers.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}helpers.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httprequest.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httprequest.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httpfileelement.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}httpfileelement.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}object.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}object.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}enum.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}enum.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}serverconfiguration.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}serverconfiguration.cpp")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}servervariable.h")
        list(APPEND common_sources "${common_dir_path}${qt_common_file_prefix}servervariable.cpp")
        list(APPEND generating_sources ${common_sources})
    else()
        # Re-read the full list of generated files from .openapi-generator/FILES
        file(STRINGS "${generated_files_file}" generated_file_list)
        list(TRANSFORM generated_file_list PREPEND "${arg_OUTPUT_DIRECTORY}/"
            OUTPUT_VARIABLE client_sources)
        list(APPEND generating_sources ${client_sources})
    endif()

    set(is_shared FALSE)
    set(is_static FALSE)
    set(is_executable FALSE)

    if(NOT arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set_property(TARGET ${target} PROPERTY AUTOMOC "ON")
        target_link_libraries(${target} PRIVATE
            ${QT_CMAKE_EXPORT_NAMESPACE}::Core
            ${QT_CMAKE_EXPORT_NAMESPACE}::Network
        )

        # This is PUBLIC, because the consumer of the client library
        # needs to get also the common library includes.
        target_link_libraries(${target} PUBLIC Qt6::OpenApiCommon)
    endif()

    _qt_internal_openapi_detect_target_type(${target}
        is_shared is_static is_executable)

    set(define_infix "")
    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        set(define_infix "_COMMON")
    endif()

    set(target_export ${target})
    string(TOUPPER ${target} target_export)
    if(is_shared)
        target_compile_definitions(${target} PRIVATE
            ${target_export}${define_infix}_LIB_SHARED)
    elseif(is_static OR is_executable)
        target_compile_definitions(${target} PRIVATE
            ${target_export}${define_infix}_LIB_STATIC)
    endif()
    if(NOT is_executable)
        target_compile_definitions(${target} PRIVATE
            ${target_export}_BUILD${define_infix}_LIB)
    endif()

    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}>")
    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${client_dir}>")

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        target_include_directories(${target} PRIVATE
            "$<BUILD_INTERFACE:${arg_OUTPUT_DIRECTORY}/${common_dir}>")
    endif()

    if(TARGET QtOpenAPIGenerator)
        add_dependencies(${target} QtOpenAPIGenerator)
    endif()

    if(arg___QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET)
        target_sources(${target} PRIVATE ${common_sources})
        _qt_internal_set_source_file_generated(
            SOURCES ${common_sources}
        )
    else()
        target_sources(${target} PRIVATE ${client_sources})
        _qt_internal_set_source_file_generated(
            SOURCES ${client_sources}
        )
    endif()

    # Generate Doxygen documentation for the Client.
    if(arg_GENERATE_DOCUMENTATION)
        # If Doxygen is not installed, then just skipping.
        if(TARGET Doxygen::doxygen)
            file(MAKE_DIRECTORY "${arg_DOCUMENTATION_OUTPUT_DIRECTORY}")
            set(configure_doxygen_script
            "${__qt_openapi_macros_module_base_dir}/Qt6OpenApiConfigureDoxygenScript.cmake")
            set(MAIN_GEN_DOC_FILE
                "${arg_DOCUMENTATION_OUTPUT_DIRECTORY}/doc/html/index.html")
            set(DOXYGEN_FILE "${client_dir_path}doc/Doxyfile")
            add_custom_command(
                OUTPUT "${MAIN_GEN_DOC_FILE}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DDOXYGEN_IN_FILE_PATH=${DOXYGEN_FILE}.in"
                    "-DDOXYGEN_OUT_FILE_PATH=${DOXYGEN_FILE}"
                    "-DINPUT_DIR=${client_dir_path}"
                    "-DOUTPUT_DIR=${arg_DOCUMENTATION_OUTPUT_DIRECTORY}/doc"
                    -P "${configure_doxygen_script}"
                COMMAND $<TARGET_FILE:Doxygen::doxygen> "${DOXYGEN_FILE}"
                ${extra_dependencies}
                ${generating_sources}
                COMMENT "Generating doxygen documentation for target '${target}'"
                VERBATIM
                COMMAND_EXPAND_LISTS
            )
            add_custom_target(${target}_openapi_docs ALL
                DEPENDS "${MAIN_GEN_DOC_FILE}"
            )
        else()
            message(WARNING "Could not find doxygen to generate documentation. "
                    "Either install it or make sure it's added to PATH and then call "
                    "find_package(Doxygen) in your project."
                    "Skipping documentation generation.")
        endif()
    endif()

    if(arg_OUTPUT_PUBLIC_HEADERS_DIR OR arg_OUTPUT_PRIVATE_HEADERS_DIR)
        set(client_public_headers_dir "${arg_OUTPUT_DIRECTORY}/${client_dir}_public_headers")
        set(client_private_headers_dir "${arg_OUTPUT_DIRECTORY}/${client_dir}_private_headers")
        set(copied_headers_timestamp_file
            "${arg_OUTPUT_DIRECTORY}/openapi_client_copied_headers_timestamp.txt")
        set(copy_headers_script
            "${__qt_openapi_macros_module_base_dir}/Qt6OpenApiToolsCopyHeadersScript.cmake")

        add_custom_command(
            OUTPUT "${copied_headers_timestamp_file}"
            DEPENDS
                "${copy_headers_script}"
                ${generating_sources}
            COMMAND "${CMAKE_COMMAND}"
                "-DTIMESTAMP_PATH=${copied_headers_timestamp_file}"
                "-DCLIENT_DIR=${client_dir_path}"
                "-DOUTPUT_PUBLIC_HEADERS_DIR=${client_public_headers_dir}"
                "-DOUTPUT_PRIVATE_HEADERS_DIR=${client_private_headers_dir}"
                -P "${copy_headers_script}"
            COMMENT "Copying generated OpenApi headers for target '${target}'"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )
        target_sources(${target} PRIVATE "${copied_headers_timestamp_file}")
    endif()

    if(arg_OUTPUT_PUBLIC_HEADERS_DIR)
        set(${arg_OUTPUT_PUBLIC_HEADERS_DIR} "${client_public_headers_dir}" PARENT_SCOPE)
    endif()
    if(arg_OUTPUT_PRIVATE_HEADERS_DIR)
        set(${arg_OUTPUT_PRIVATE_HEADERS_DIR} "${client_private_headers_dir}" PARENT_SCOPE)
    endif()
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_openapi_client)
         qt6_add_openapi_client(${ARGV})
    endfunction()
endif()
