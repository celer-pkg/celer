# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# The variable is ON for macOS, needs to be OFF for Golang search.
if(DEFINED QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH)
    set(_qt_openapi_no_look_in_path_backup "${QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH}")
    set(QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH "OFF")
endif()

# Values to show in summary.
set(java_runtime_value "<not found>")
set(java_compiler_value "<not found>")
set(golang_value "<not found>")
set(maven_value "<not found>")
set(openapi_generator_value "<not found>")
set(doxygen_value "<not found>")
set(build_all_tests_value "no")
set(zlib_value "no")

# Helper variables to track found dependencies.
set(java_runtime_found FALSE)
set(java_compiler_found FALSE)
set(golang_found FALSE)
set(maven_found FALSE)
set(openapi_generator_found FALSE)
set(zlib_found FALSE)
set(doxygen_found FALSE)

# Look for zlib, which is a required dependency for the soon-to-be generated OpenAPI Common
# library.
# Handle the conditional finding of either system zlib or qt zlib.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 COMPONENTS ZlibPrivate)
elseif(NOT TARGET WrapZLIB::WrapZLIB)
    qt_find_package(WrapZLIB PROVIDED_TARGETS WrapZLIB::WrapZLIB)
endif()

if(NOT QT_CONFIGURE_RUNNING)
    # Check which zlib was found.
    if(TARGET Qt6::ZlibPrivate)
        set(zlib_value "Qt zlib")
        set(zlib_found TRUE)
    elseif(TARGET WrapZLIB::WrapZLIB)
        set(zlib_path "")
        if(ZLIB_LIBRARY)
            set(zlib_path "${ZLIB_LIBRARY}")
        else()
            set(zlib_path "system zlib")
        endif()
        set(zlib_value "${zlib_path} version (${ZLIB_VERSION})")
        set(zlib_found TRUE)
    endif()

    # Look for the java compiler, used to build the qt openapi generator plugin.
    find_package(Java COMPONENTS Development)
    if(Java_JAVAC_EXECUTABLE)
        set(java_compiler_value "${Java_JAVAC_EXECUTABLE}")
        set(java_compiler_found TRUE)
    endif()

    # Look for maven, used to build the qt openapi generator plugin.
    find_program(OPENAPI_MAVEN_EXECUTABLE NAMES mvn)
    if(OPENAPI_MAVEN_EXECUTABLE)
        set(maven_value "${OPENAPI_MAVEN_EXECUTABLE}")
        set(maven_found TRUE)
    endif()

    # Look for go, used in tests only.
    find_program(OPENAPI_GO_EXECUTABLE NAMES go)
    if(OPENAPI_GO_EXECUTABLE)
        set(golang_value "${OPENAPI_GO_EXECUTABLE}")
        set(golang_found TRUE)
    endif()

    # Look for doxygen, used to create documentation for the generated code
    find_package(Doxygen)
    if (DOXYGEN_EXECUTABLE)
        set(doxygen_value "${DOXYGEN_EXECUTABLE}")
        set(doxygen_found TRUE)
    endif()

    # We need to find the upstream OpenAPI generator in order to build the OpenApiCommon
    # library and to be able to generate any client library
    qt_find_package(WrapOpenAPIGenerator PROVIDED_TARGETS WrapOpenAPIGenerator::WrapOpenAPIGenerator)
    if(TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
        get_target_property(openapi_generator_cli_jar_path
            WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE_OPENAPI_GENERATOR_CLI_JAR)
        if(openapi_generator_cli_jar_path)
            set(openapi_generator_value "${openapi_generator_cli_jar_path}")
            set(openapi_generator_found TRUE)
        else() # only get executable property if the jar wasn't found
            get_target_property(openapi_generator_cli_executable_path
                WrapOpenAPIGenerator::WrapOpenAPIGenerator
                INTERFACE_OPENAPI_GENERATOR_CLI_EXECUTABLE)
            if(openapi_generator_cli_executable_path)
                set(openapi_generator_value "${openapi_generator_cli_executable_path}")
                set(openapi_generator_found TRUE)
            endif()
        endif()
    endif()

    # Same with the java runtime.
    qt_find_package(WrapOpenAPIJava PROVIDED_TARGETS WrapOpenAPIJava::WrapOpenAPIJava)
    if(TARGET WrapOpenAPIJava::WrapOpenAPIJava)
        get_target_property(java_runtime_path
            WrapOpenAPIJava::WrapOpenAPIJava INTERFACE_OPENAPI_JAVA_RUNTIME_PATH)
        if(java_runtime_path)
            set(java_runtime_value "${java_runtime_path}")
            set(java_runtime_found TRUE)
        endif()
    endif()

    # Building all tests requires all dependencies to be found.
    if(java_runtime_found
            AND java_compiler_found
            AND golang_found
            AND maven_found
            AND openapi_generator_found
            AND zlib_found
        )
        set(build_all_tests_value "yes")
    endif()
endif()

if(DEFINED _qt_openapi_no_look_in_path_backup)
    set(QT_NO_USE_FIND_PACKAGE_SYSTEM_ENVIRONMENT_PATH ${_qt_openapi_no_look_in_path_backup})
    unset(_qt_openapi_no_look_in_path_backup)
endif()

qt_feature("openapi_generator" PRIVATE
    LABEL "Build Qt OpenAPI generator"
    CONDITION
        Java_JAVAC_EXECUTABLE
        AND OPENAPI_MAVEN_EXECUTABLE
        AND TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator
)

qt_feature("openapi_common_library" PRIVATE
    LABEL "Build Qt OpenAPI common library"
    CONDITION
        QT_FEATURE_openapi_generator
        AND zlib_found
)

qt_configure_add_summary_section(NAME "Qt OpenAPI dependencies")
qt_configure_add_summary_entry(ARGS "java" TYPE "message" MESSAGE "${java_runtime_value}")
qt_configure_add_summary_entry(ARGS "javac" TYPE "message" MESSAGE "${java_compiler_value}")
qt_configure_add_summary_entry(ARGS "go (for tests)" TYPE "message" MESSAGE "${golang_value}")
qt_configure_add_summary_entry(ARGS "maven" TYPE "message" MESSAGE "${maven_value}")
qt_configure_add_summary_entry(ARGS "Upstream OpenAPI generator CLI" TYPE "message"
    MESSAGE "${openapi_generator_value}")
qt_configure_add_summary_entry(ARGS "zlib" TYPE "message" MESSAGE "${zlib_value}")
qt_configure_end_summary_section()

qt_configure_add_summary_section(NAME "Qt OpenAPI")
qt_configure_add_summary_entry(ARGS "openapi_generator")
qt_configure_add_summary_entry(ARGS "All test dependencies found" TYPE "message"
    MESSAGE "${build_all_tests_value}")
qt_configure_add_summary_entry(ARGS "doxygen" TYPE "message" MESSAGE "${doxygen_value}")
qt_configure_end_summary_section()

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The java runtime was not found."
    CONDITION NOT Java_JAVA_EXECUTABLE
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The java compiler was not found."
    CONDITION NOT Java_JAVAC_EXECUTABLE
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The maven build tool was not found."
    CONDITION NOT OPENAPI_MAVEN_EXECUTABLE
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Not all build dependencies were found. Skipping building qtopenapi."
    CONDITION NOT QT_FEATURE_openapi_generator
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "The golang compiler was not found. Skipping building qtopenapi tests."
    CONDITION NOT OPENAPI_GO_EXECUTABLE AND QT_BUILD_TESTS
)

if(NOT openapi_generator_found)
    string(CONCAT openapi_generator_check_msg
        "The upstream OpenAPI generator CLI tool was not found. "
        "This doesn't affect the build of the generator plugin itself, "
        "but examples and tests will fail to build."
    )
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "${openapi_generator_check_msg}"
    )
endif()

if(NOT zlib_found)
    string(CONCAT zlib_check_msg
        "The zlib library was not found. Skipping building the OpenAPI common library. "
        "Note that all generated clients depend on this library, so building clients for "
        "examples / tests will fail. "
    )
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "${zlib_check_msg}"
    )
endif()

if(NOT doxygen_found)
    string(CONCAT doxygen_msg
        "The doxygen was not found. The GEN_DOXYGEN_DOCS option of the qt6_add_openapi_client "
        "function will be ignored. It means no doxygen documentation will be generated."
    )
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "${doxygen_msg}"
    )
endif()
