# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
cmake_minimum_required(VERSION 3.16)

project(qtabstractlistmodel_kotlin VERSION 0.1 LANGUAGES CXX)

install(DIRECTORY
    gradle
    app
    DESTINATION .
)
install(FILES
    build.gradle.kts
    CMakeLists.txt
    gradle.properties
    settings.gradle.kts
    DESTINATION .
)
