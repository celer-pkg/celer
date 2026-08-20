# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(mediaplayerscxml_snippets LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Scxml)

qt_standard_project_setup(REQUIRES 6.7)

qt_add_library(mediaplayerscxml_snippets
    thedatamodel.h thedatamodel.cpp
)

target_link_libraries(mediaplayerscxml_snippets PRIVATE
    Qt::Core
    Qt::Scxml
)

qt_add_statecharts(mediaplayerscxml_snippets
    mediaplayer.scxml
)
