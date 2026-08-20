# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-petstore-storeapi LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET petstore-server-go)
    message(WARNING "No petstore-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-petstore-storeapi
    SOURCES
        tst_storeapitests.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

target_compile_definitions(cpp-qt-petstore-storeapi
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:petstore-server-go>"
)

target_link_libraries(cpp-qt-petstore-storeapi PRIVATE PetStoreClient)
add_dependencies(cpp-qt-petstore-storeapi petstore-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-petstore-storeapi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:PetStoreClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
