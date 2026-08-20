# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-standard-models LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(cpp-qt-standard-models
    SOURCES
        tst_standardmodels.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(basicSchemas)
qt_add_openapi_client(basicSchemas
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/basic-schemas.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=StandardSchemasModels
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/basicSchemas
)

qt_autogen_tools_initial_setup(basicSchemas)
target_link_libraries(cpp-qt-standard-models PRIVATE basicSchemas)

if(WIN32)
    add_custom_command(TARGET cpp-qt-standard-models POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:basicSchemas>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
