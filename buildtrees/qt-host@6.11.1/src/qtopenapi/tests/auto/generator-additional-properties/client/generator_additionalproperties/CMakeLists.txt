# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-generator-additional-properties LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET generator-additional-properties-server-go)
    message(WARNING "No generator-additional-properties-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-generator-additional-properties
    SOURCES
        tst_generator_additionalproperties.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(GeneratorAdditionalPropertiesClient)
qt_add_openapi_client(GeneratorAdditionalPropertiesClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/generator-additional-properties.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=AddPropNamespace
        modelNamePrefix=AddProp
        addDownloadProgress=true
        ensureUniqueParams=true # default value
        enumUnknownDefaultCase=false # default value
        allowUnicodeIdentifiers=false # default value
        reservedWordPrefix=r_ # default value
        sortParamsByRequiredFlag=true # default value
        sortModelPropertiesByRequiredFlag=true # default value
        prependFormOrBodyParameters=false # default value
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client1
)

qt_add_library(GeneratorAdditionalPropertiesClient2)
qt_add_openapi_client(GeneratorAdditionalPropertiesClient2
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/generator-additional-properties.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=AddPropNamespace2
        modelNamePrefix=AddProp2
        enumUnknownDefaultCase=true
        allowUnicodeIdentifiers=true
        licenseName=MyLicense2 # the option shoud be ignored
        reservedWordPrefix=reserved- # This should be sanitized to reserved_
        sortParamsByRequiredFlag=false # Ignored: parameters must be sorted to keep defaulted
                                       # parameters at the end (C++ requirement).
        sortModelPropertiesByRequiredFlag=false
        prependFormOrBodyParameters=true
    JAVA_OPTIONS
        -Dlog.level=warn
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client2
)

target_compile_definitions(cpp-qt-generator-additional-properties
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:generator-additional-properties-server-go>"
)
qt_autogen_tools_initial_setup(GeneratorAdditionalPropertiesClient)
qt_autogen_tools_initial_setup(GeneratorAdditionalPropertiesClient2)
target_link_libraries(cpp-qt-generator-additional-properties PRIVATE
    GeneratorAdditionalPropertiesClient GeneratorAdditionalPropertiesClient2)
add_dependencies(cpp-qt-generator-additional-properties
    generator-additional-properties-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-generator-additional-properties POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:GeneratorAdditionalPropertiesClient>
            $<TARGET_FILE:GeneratorAdditionalPropertiesClient2>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
