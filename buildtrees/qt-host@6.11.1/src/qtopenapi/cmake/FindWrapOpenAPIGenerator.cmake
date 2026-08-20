# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include("FindWrapOpenAPIGeneratorHelpers")

if(TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
    set(WrapOpenAPIGenerator_FOUND TRUE)
    return()
endif()

# Option 1: Try to find the jar file "openapi-generator-cli.jar"
# We don't use find_jar, because that passes NO_DEFAULT_PATH to find_path, and we want to search
# inside PATH env.
find_file(OPENAPI_GENERATOR_CLI_JAR
    NAMES
        openapi-generator-cli.jar
    PATHS
        # These are the paths that find_jar would search by default
        /usr/share/java
        /usr/local/share/java
        ${Java_JAR_PATHS}
)

# Option 2: - Try to find openapi-generator-cli executable
if(NOT OPENAPI_GENERATOR_CLI_JAR)
    message(DEBUG "Could NOT find openapi-generator-cli.jar")

    find_program(OPENAPI_GENERATOR_CLI_EXECUTABLE NAMES openapi-generator-cli openapi-generator)
    if(OPENAPI_GENERATOR_CLI_EXECUTABLE)
        message(DEBUG "Found openapi-generator-cli executable: ${OPENAPI_GENERATOR_CLI_EXECUTABLE}")

        _qt_internal_check_custom_generator_support(__qt_openapi_supports_custom_generator)
        if(NOT __qt_openapi_supports_custom_generator)
            _qt_internal_openapi_find_cli_jar(OPENAPI_GENERATOR_CLI_JAR)
        endif()
    else()
        message(DEBUG "Could NOT find openapi-generator-cli executable")
    endif()
endif()

if(OPENAPI_GENERATOR_CLI_JAR)
    set(__qt_openapi_required_vars "OPENAPI_GENERATOR_CLI_JAR")
elseif(OPENAPI_GENERATOR_CLI_EXECUTABLE AND __qt_openapi_supports_custom_generator)
    set(__qt_openapi_required_vars "OPENAPI_GENERATOR_CLI_EXECUTABLE")
else()
    # Either nothing found, or cli file found but doesn't support custom generators.
    # In either case, it's not usable for qtopenapi.
    set(__qt_openapi_required_vars
        OPENAPI_GENERATOR_CLI_JAR
        OPENAPI_GENERATOR_CLI_EXECUTABLE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapOpenAPIGenerator
    REQUIRED_VARS
       ${__qt_openapi_required_vars}
)

unset(__qt_openapi_required_vars)

if(WrapOpenAPIGenerator_FOUND)
    add_library(WrapOpenAPIGenerator::WrapOpenAPIGenerator INTERFACE IMPORTED)
    if(OPENAPI_GENERATOR_CLI_JAR)
        set_target_properties(WrapOpenAPIGenerator::WrapOpenAPIGenerator PROPERTIES
            INTERFACE_OPENAPI_GENERATOR_CLI_JAR "${OPENAPI_GENERATOR_CLI_JAR}"
        )
    elseif(OPENAPI_GENERATOR_CLI_EXECUTABLE AND __qt_openapi_supports_custom_generator)
        set_target_properties(WrapOpenAPIGenerator::WrapOpenAPIGenerator PROPERTIES
            INTERFACE_OPENAPI_GENERATOR_CLI_EXECUTABLE "${OPENAPI_GENERATOR_CLI_EXECUTABLE}"
        )
    endif()
endif()
