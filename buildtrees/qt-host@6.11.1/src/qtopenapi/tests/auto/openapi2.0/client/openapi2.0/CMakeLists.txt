# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-operations-parameters-backport LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET backport-server-go)
    message(WARNING "No backport-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-operations-parameters-backport
    SOURCES
        tst_openapi2_0.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(OpenapiBackportClient)
qt_add_openapi_client(OpenapiBackportClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/openapi2.0.yaml
    GENERATE_DOCUMENTATION # test documentation generation for older format
)

target_compile_definitions(cpp-qt-operations-parameters-backport
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:backport-server-go>"
)

add_dependencies(cpp-qt-operations-parameters-backport backport-server-go)
qt_autogen_tools_initial_setup(OpenapiBackportClient)
target_link_libraries(cpp-qt-operations-parameters-backport
    PRIVATE
        OpenapiBackportClient
)

if(WIN32)
    add_custom_command(TARGET cpp-qt-operations-parameters-backport POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:OpenapiBackportClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
