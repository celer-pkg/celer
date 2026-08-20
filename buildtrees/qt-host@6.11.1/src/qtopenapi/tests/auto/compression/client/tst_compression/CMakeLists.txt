# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-compression LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET compression-server-go)
    message(WARNING "No compression-server-go server in the build.")
    return()
endif()

find_package(Doxygen)

qt_internal_add_test(cpp-qt-compression
    SOURCES
        tst_compression.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(CompressionClient)
qt_add_openapi_client(CompressionClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/compression.yaml
    GENERATE_OPTIONS
        -i ${CMAKE_CURRENT_BINARY_DIR}/random.yaml # Should be ignored
        -o ${CMAKE_CURRENT_BINARY_DIR}/client # Should be ignored
        -p cppCommonNamespace=NotQtOpenApiCommon # Should be ignored
        --additional-properties=modelNamePrefix=Compression
        --api-name-suffix MyApiSuffix
    ADDITIONAL_PROPERTIES
        cppNamespace=CompressionNamespace
    JAVA_OPTIONS
        -DmaxYamlCodePoints=99999999
        -Dlog.level=warn
    GENERATE_DOCUMENTATION
)

target_compile_definitions(cpp-qt-compression
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:compression-server-go>"
)
qt_autogen_tools_initial_setup(CompressionClient)
target_link_libraries(cpp-qt-compression PRIVATE CompressionClient)
add_dependencies(cpp-qt-compression compression-server-go)

add_custom_command(TARGET cpp-qt-compression POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
            ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMENT "Copying test files to ${CMAKE_CURRENT_BINARY_DIR}"
)

if(WIN32)
    add_custom_command(TARGET cpp-qt-compression POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CompressionClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
