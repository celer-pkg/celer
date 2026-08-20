# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(contextinfo LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS ShaderTools)

qt6_add_executable(qmlappshadermacro
    main.cpp
)
target_link_libraries(qmlappshadermacro PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
)

set(qmlappshadermacro_resource_files
    "main.qml"
)

qt6_add_resources(qmlappshadermacro "qmlappshadermacro"
    PREFIX
        "/"
    FILES
        ${qmlappshadermacro_resource_files}
)

qt6_add_shaders(qmlappshadermacro "qmlappshadermacro_shaders"
    PREFIX
        "/"
    FILES
        "wobble.frag"
)
