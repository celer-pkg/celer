# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(build_go_executable target sources)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    if(CMAKE_HOST_APPLE AND ("${CMAKE_OSX_ARCHITECTURES}" MATCHES "(x86_64;arm64|arm64;x64_64)"))
        add_custom_command(
            OUTPUT ${target}
            COMMAND go mod tidy
            COMMAND GOOS=darwin GOARCH=arm64 go build -o "${OUTPUT_DIR}/${target}_arm64" ${sources}
            COMMAND GOOS=darwin GOARCH=amd64 go build -o "${OUTPUT_DIR}/${target}_amd64" ${sources}
            COMMAND lipo -create -output "${OUTPUT_DIR}/${target}" "${OUTPUT_DIR}/${target}_arm64"
                    "${OUTPUT_DIR}/${target}_amd64"
            COMMAND rm -f "${OUTPUT_DIR}/${target}_arm64" "${OUTPUT_DIR}/${target}_amd64"
            DEPENDS ${sources}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating the golang server: ${target}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT ${target}
            COMMAND go mod tidy
            COMMAND go build -o "${OUTPUT_DIR}/${target}" ${sources}
            DEPENDS ${sources}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating the golang server: ${target}"
            VERBATIM
        )
    endif()
endfunction()
