# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-responses LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET responses-server-go)
    message(WARNING "No responses-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-responses
    SOURCES
        tst_responses.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(ResponsesClient)
qt_add_openapi_client(ResponsesClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/responses.yaml
)

target_compile_definitions(cpp-qt-responses
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:responses-server-go>"
        SERVER_DIR="$<TARGET_FILE_DIR:responses-server-go>"
)
qt_autogen_tools_initial_setup(ResponsesClient)
target_link_libraries(cpp-qt-responses PRIVATE ResponsesClient)
add_dependencies(cpp-qt-responses responses-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-responses POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ResponsesClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
