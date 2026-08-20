# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20)

project(test_plugin_class_name_${PLUGIN_CLASS_NAME} LANGUAGES CXX)

find_package(Qt6Core REQUIRED)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/plugin.cpp" @ONLY)

qt_add_plugin(test_plugin CLASS_NAME ${PLUGIN_CLASS_NAME} "${CMAKE_CURRENT_BINARY_DIR}/plugin.cpp")
