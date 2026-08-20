# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

find_program(OPENAPI_GO_EXECUTABLE NAMES go)

if (NOT OPENAPI_GO_EXECUTABLE)
    message(WARNING "Go (golang) is not installed or not in the PATH.")
    return()
endif()

include(../../common/BuildGoExecutable.cmake)

set(GO_BINARY serverconfiguration-server-app)
set(GO_SOURCE main.go)

build_go_executable(${GO_BINARY} ${GO_SOURCE})

# Custom target that depends on the output binary
add_custom_target(serverconfiguration-server-build ALL
    DEPENDS ${GO_BINARY}
)

add_executable(serverconfiguration-server-go IMPORTED)
add_dependencies(serverconfiguration-server-go serverconfiguration-server-build)
set_target_properties(serverconfiguration-server-go PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/serverconfiguration-server-app"
    IMPORTED_GLOBAL TRUE
)
