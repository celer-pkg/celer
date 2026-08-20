# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT Java_JAVAC_EXECUTABLE)
    message(FATAL_ERROR "Missing Java_JAVAC_EXECUTABLE to be set.")
endif()

if(NOT OPENAPI_MAVEN_EXECUTABLE)
     message(FATAL_ERROR "Missing OPENAPI_MAVEN_EXECUTABLE to be set.")
 endif()

set(generator_jar_name "cpp-qt6-client-openapi-generator.jar")

set(QT6_OPENAPI_GENERATOR_CLI "${CMAKE_CURRENT_BINARY_DIR}/target/${generator_jar_name}")

# We only need to build our generator plugin when one of these files changes
# or when it was not yet built. Note that the change of .mustache templates
# should *also* lead to rebuilding the generator, because the files are
# embedded into the jar file.
set(need_call_maven FALSE)

set(source_files
    src/main/java/org/qtproject/qt/codegen/CppQt6AbstractCodegen.java
    src/main/java/org/qtproject/qt/codegen/CppQt6ClientGenerator.java
    src/main/java/org/qtproject/qt/codegen/YamlDependencyBuilder.java
    src/main/resources/logback.xml
    src/main/resources/META-INF/services/org.openapitools.codegen.CodegenConfig
    src/main/resources/cpp-qt6-client/CMakeConfig.mustache
    src/main/resources/cpp-qt6-client/CMakeLists.txt.mustache
    src/main/resources/cpp-qt6-client/README.mustache
    src/main/resources/cpp-qt6-client/api-body.mustache
    src/main/resources/cpp-qt6-client/api-header.mustache
    src/main/resources/cpp-qt6-client/common/CMakeConfig.mustache
    src/main/resources/cpp-qt6-client/common/CMakeLists.txt.mustache
    src/main/resources/cpp-qt6-client/common/httpfileelement.cpp.mustache
    src/main/resources/cpp-qt6-client/common/httpfileelement.h.mustache
    src/main/resources/cpp-qt6-client/common/httprequest.cpp.mustache
    src/main/resources/cpp-qt6-client/common/httprequest.h.mustache
    src/main/resources/cpp-qt6-client/common/serverconfiguration.cpp.mustache
    src/main/resources/cpp-qt6-client/common/serverconfiguration.h.mustache
    src/main/resources/cpp-qt6-client/common/servervariable.cpp.mustache
    src/main/resources/cpp-qt6-client/common/servervariable.h.mustache
    src/main/resources/cpp-qt6-client/common/api-base-body.mustache
    src/main/resources/cpp-qt6-client/common/api-base-header.mustache
    src/main/resources/cpp-qt6-client/common/common-exports.mustache
    src/main/resources/cpp-qt6-client/common/common-global.mustache
    src/main/resources/cpp-qt6-client/common/enum-body.mustache
    src/main/resources/cpp-qt6-client/common/enum-header.mustache
    src/main/resources/cpp-qt6-client/common/helpers-body.mustache
    src/main/resources/cpp-qt6-client/common/helpers-header.mustache
    src/main/resources/cpp-qt6-client/common/object-body.mustache
    src/main/resources/cpp-qt6-client/common/object-header.mustache
    src/main/resources/cpp-qt6-client/doc/Doxyfile.in.mustache
    src/main/resources/cpp-qt6-client/exports.mustache
    src/main/resources/cpp-qt6-client/licenseInfo.mustache
    src/main/resources/cpp-qt6-client/model-body.mustache
    src/main/resources/cpp-qt6-client/model-header.mustache
    src/test/java/org/qtproject/qt/codegen/CppQt6ClientGeneratorTest.java
    pom.xml
)

# add the files to configure-time dependencies
set(full_file_paths "")
foreach(source_file IN LISTS source_files)
    list(APPEND full_file_paths "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
endforeach()
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${full_file_paths}")

if(NOT EXISTS "${QT6_OPENAPI_GENERATOR_CLI}")
    set(need_call_maven TRUE)
endif()

if(NOT need_call_maven)
    # Check if the files changed after the generator plugin was
    # built last time
    file(TIMESTAMP "${QT6_OPENAPI_GENERATOR_CLI}" plugin_timestamp)
    foreach(source_file IN LISTS source_files)
        file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}" file_timestamp)
        if(plugin_timestamp STRLESS file_timestamp)
            set(need_call_maven TRUE)
            break()
        endif()
    endforeach()
endif()

if(need_call_maven)
    message(STATUS "Building the Qt6 Client Code Generator with Maven")
    set(cmd_suffix "")
    if(CMAKE_HOST_WIN32)
        set(cmd_suffix ".cmd")
    endif()
    set(extra_args "")
    if(NOT QT_FEATURE_developer_build)
        list(APPEND extra_args
            ERROR_VARIABLE build_plugin_error
            OUTPUT_VARIABLE build_plugin_output
        )
    endif()
    execute_process(
        COMMAND "${OPENAPI_MAVEN_EXECUTABLE}${cmd_suffix}"
                -f "${CMAKE_CURRENT_SOURCE_DIR}/pom.xml" package
                "-Dqt.build.dir=${CMAKE_CURRENT_BINARY_DIR}/target"
                "-Drevision=${QT_OPENAPI_GENERATOR_VERSION}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RESULT_VARIABLE build_plugin_result
        ${extra_args}
    )
    message(STATUS "Finished building the Qt6 Client Code Generator")
endif() # need_call_maven

# if we didn't run maven, build_plugin_result is undefined,
# so if(NOT) is still true
if(NOT build_plugin_result)
    message(STATUS "Copying the Qt6 Client Code Generator to libexec build directory.")
    execute_process(
        # Copy the built generator to libexec build dir, so that standalone tests work
        # with no-prefix builds as well.
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${QT6_OPENAPI_GENERATOR_CLI}"
                "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/${generator_jar_name}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RESULT_VARIABLE copy_plugin_result
    )

    if(copy_plugin_result)
        message(FATAL_ERROR
            "Failed to copy the Qt6 Client Code Generator to the installation directory")
    endif()
else()
    set(error_message "Failed to build the Qt6 Client Code Generator."
        "Exit code: ${build_plugin_result}")
    if(build_plugin_output OR build_plugin_error)
        string(APPEND error_message "\n stdout: '${build_plugin_output}' "
            "\n stderr: '${build_plugin_error}'")
    endif()
    message(FATAL_ERROR "${error_message}")
endif()

add_executable(QtOpenAPIGeneratorJar IMPORTED GLOBAL)
add_executable("${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" ALIAS QtOpenAPIGeneratorJar)
set_target_properties(QtOpenAPIGeneratorJar PROPERTIES
    IMPORTED_LOCATION
        "${QT6_OPENAPI_GENERATOR_CLI}"
)

if(INSTALL_LIBEXECDIR)
    set(qt_install_libexec_dir "${INSTALL_LIBEXECDIR}")
elseif(QT6_INSTALL_LIBEXECS)
    set(qt_install_libexec_dir "${QT6_INSTALL_LIBEXECS}")
endif()

qt_install(FILES "${QT6_OPENAPI_GENERATOR_CLI}" DESTINATION "${qt_install_libexec_dir}")
