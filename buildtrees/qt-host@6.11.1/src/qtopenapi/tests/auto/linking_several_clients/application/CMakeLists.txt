# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-test-config LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network Test)

qt_internal_add_test(cpp-qt-test-config1-app
    SOURCES
        LinkageTest.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

# client1 generation
qt_add_library(TestClient1)
qt_add_openapi_client(TestClient1
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/linking_several_clients/client1.yaml
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client1
)

# client2 generation
qt_add_library(TestClient2)
qt_add_openapi_client(TestClient2
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/linking_several_clients/client2.yaml
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/client2
)

qt_autogen_tools_initial_setup(TestClient1)
qt_autogen_tools_initial_setup(TestClient2)

# Test is checking there is no names clashing in case several clients were linked.
target_link_libraries(cpp-qt-test-config1-app PRIVATE TestClient1 TestClient2)

if(WIN32)
    add_custom_command(TARGET cpp-qt-test-config1-app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestClient1>
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestClient2>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
