# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-mediatype LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET mediatype-server-go)
    message(WARNING "No mediatype-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-mediatype
    SOURCES
        tst_mediatype.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Test
)

qt_add_library(MediaTypeClient)
qt_add_openapi_client(MediaTypeClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/mediatype.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=MediaNamespace
        modelNamePrefix=Media
        packageName=random # Should be ignored
        cppCommonNamespace=NotQtOpenApiCommon # Should be ignored
    GENERATE_DOCUMENTATION
)

# Resources:
set(cpp_qt_mediatype_resource_files
    "file-for-uploading.txt"
    "usericon.png"
)

qt_internal_add_resource(cpp-qt-mediatype "cpp-qt-mediatype"
    PREFIX
        "/"
    FILES
        ${cpp_qt_mediatype_resource_files}
)

target_compile_definitions(cpp-qt-mediatype
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:mediatype-server-go>"
)
qt_autogen_tools_initial_setup(MediaTypeClient)
target_link_libraries(cpp-qt-mediatype PRIVATE MediaTypeClient)
add_dependencies(cpp-qt-mediatype mediatype-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-mediatype POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:MediaTypeClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
