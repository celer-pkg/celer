# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.21)

if(NOT CLIENT_DIR OR NOT EXISTS "${CLIENT_DIR}")
    message(FATAL_ERROR "Client dir '${CLIENT_DIR}' does not exist")
endif()

if(NOT OUTPUT_PUBLIC_HEADERS_DIR)
    message(FATAL_ERROR "OUTPUT_PUBLIC_HEADERS_DIR is not set")
endif()

if(NOT OUTPUT_PRIVATE_HEADERS_DIR)
    message(FATAL_ERROR "OUTPUT_PRIVATE_HEADERS_DIR is not set")
endif()

if(NOT TIMESTAMP_PATH)
    message(FATAL_ERROR "TIMESTAMP_PATH is not set")
endif()

set(public_globs "${CLIENT_DIR}/*.h")
set(private_globs "${CLIENT_DIR}/*_p.h")

file(GLOB_RECURSE public_header_files
    ${public_globs}
)

file(GLOB_RECURSE private_header_files
    ${private_globs}
)

file(MAKE_DIRECTORY "${OUTPUT_PUBLIC_HEADERS_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_PRIVATE_HEADERS_DIR}")

foreach(header_file IN LISTS public_header_files)
    get_filename_component(header_name "${header_file}" NAME)
    set(destination_path "${OUTPUT_PUBLIC_HEADERS_DIR}/${header_name}")
    file(COPY_FILE "${header_file}" "${destination_path}" ONLY_IF_DIFFERENT)
endforeach()

foreach(header_file IN LISTS private_header_files)
    get_filename_component(header_name "${header_file}" NAME)
    set(destination_path "${OUTPUT_PRIVATE_HEADERS_DIR}/${header_name}")
    file(COPY_FILE "${header_file}" "${destination_path}" ONLY_IF_DIFFERENT)
endforeach()

file(TOUCH "${TIMESTAMP_PATH}")
