# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

include(../../common/BuildGoExecutable.cmake)

set(GO_BINARY compression-server-app)
set(GO_SOURCE main.go)

build_go_executable(${GO_BINARY} ${GO_SOURCE})

# Custom target that depends on the output binary
add_custom_target(compression-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(compression-server-go IMPORTED)
add_dependencies(compression-server-go compression-server-build)
set_target_properties(compression-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/compression-server-app"
    IMPORTED_GLOBAL TRUE
)
