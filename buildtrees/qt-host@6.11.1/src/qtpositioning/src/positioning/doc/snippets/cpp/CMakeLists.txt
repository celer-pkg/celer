# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(positioning_cppsnippet LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Positioning)

qt_add_executable(positioning_cppsnippet
    cppqml.cpp
    main.cpp
)
set_target_properties(positioning_cppsnippet PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(positioning_cppsnippet PRIVATE
    Qt::Core
    Qt::Positioning
)
