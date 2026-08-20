# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-serverconfiguration LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET serverconfiguration-server-go)
    message(WARNING "No serverconfiguration-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-serverconfiguration
    SOURCES
        tst_server_configuration.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(ServerConfigurationClient)
qt_add_openapi_client(ServerConfigurationClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/server_configuration.yaml
)

target_compile_definitions(cpp-qt-serverconfiguration
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:serverconfiguration-server-go>"
)
qt_autogen_tools_initial_setup(ServerConfigurationClient)
target_link_libraries(cpp-qt-serverconfiguration PRIVATE ServerConfigurationClient)
add_dependencies(cpp-qt-serverconfiguration serverconfiguration-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-serverconfiguration POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ServerConfigurationClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
