# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

include(../../common/BuildGoExecutable.cmake)

set(GO_BINARY responses-server-app)
set(GO_SOURCE main.go)

build_go_executable(${GO_BINARY} ${GO_SOURCE})

# Custom target that depends on the output binary
add_custom_target(responses-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(responses-server-go IMPORTED)
add_dependencies(responses-server-go responses-server-build)
set_target_properties(responses-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/responses-server-app"
    IMPORTED_GLOBAL TRUE
)

add_custom_command(
    TARGET responses-server-build
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
            ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/testImage.jpg
            ${CMAKE_CURRENT_BINARY_DIR}/testImage.jpg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/testImage.png
            ${CMAKE_CURRENT_BINARY_DIR}/testImage.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/test.bin
            ${CMAKE_CURRENT_BINARY_DIR}/test.bin
    COMMENT "Copying test files to ${CMAKE_CURRENT_BINARY_DIR}"
)
