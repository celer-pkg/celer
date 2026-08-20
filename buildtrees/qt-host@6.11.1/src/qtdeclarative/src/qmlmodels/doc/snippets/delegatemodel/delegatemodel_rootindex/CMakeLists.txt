# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(delegatemodel_rootindex LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(delegatemodel_rootindex
    main.cpp
)

qt_add_qml_module(delegatemodel_rootindex
    URI FileSystemModule
    VERSION 1.0
)

target_link_libraries(delegatemodel_rootindex
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
)

