# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(custom-converter LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS HttpServer)

qt_standard_project_setup()

qt_add_executable(custom-converter
    main.cpp
)

target_link_libraries(custom-converter PRIVATE
    Qt6::HttpServer
)
