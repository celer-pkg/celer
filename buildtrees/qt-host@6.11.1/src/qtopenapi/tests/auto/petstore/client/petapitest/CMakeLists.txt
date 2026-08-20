# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-petstore-petapi LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET petstore-server-go)
    message(WARNING "No petstore-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-petstore-petapi
    SOURCES
        tst_petapitests.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Test
)

target_compile_definitions(cpp-qt-petstore-petapi
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:petstore-server-go>"
        SERVER_FOLDER="$<TARGET_FILE_DIR:petstore-server-go>"
)

# Resources:
set(cpp_qt_petstore_resource_files
    "file-for-uploading.txt"
)

qt_add_resources(cpp-qt-petstore-petapi "cpp-qt-petstore-petapi"
    PREFIX
        "/"
    FILES
        ${cpp_qt_petstore_resource_files}
)

target_link_libraries(cpp-qt-petstore-petapi PRIVATE PetStoreClient)
add_dependencies(cpp-qt-petstore-petapi petstore-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-petstore-petapi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:PetStoreClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
