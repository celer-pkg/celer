# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.21)

if(NOT DOXYGEN_IN_FILE_PATH)
    message(FATAL_ERROR "Doxyfile input file is not set")
endif()

if(NOT DOXYGEN_OUT_FILE_PATH)
    message(FATAL_ERROR "Doxyfile output file is not set")
endif()

if(NOT INPUT_DIR)
    message(FATAL_ERROR "INPUT_DIR is not set")
endif()

if(NOT OUTPUT_DIR)
    message(FATAL_ERROR "OUTPUT_DIR is not set")
endif()

set(DOXY_OUT_DIR "${OUTPUT_DIR}")
set(DOXY_IN_DIR "${INPUT_DIR}")

configure_file(
    "${DOXYGEN_IN_FILE_PATH}"
    "${DOXYGEN_OUT_FILE_PATH}"
    @ONLY
)
