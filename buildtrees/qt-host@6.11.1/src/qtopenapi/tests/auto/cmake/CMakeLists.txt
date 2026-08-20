# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtopenapi_cmake_tests)
enable_testing()

# Add all tests using CMake's RunCMake test module
add_subdirectory(RunCMake)
