# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_qtscxml_module)

find_package(Qt6Scxml REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(mainapp main.cpp)

qt6_add_statecharts(mainapp
    ../../compiled/connection.scxml

    # unused, just for testing whether it's possible to pass multiple files
    ../../compiled/topmachine.scxml
)

target_link_libraries(mainapp Qt::Scxml)
