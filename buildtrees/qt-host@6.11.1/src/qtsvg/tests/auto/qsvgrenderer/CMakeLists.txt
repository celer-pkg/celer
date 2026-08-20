# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qsvgrenderer Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qsvgrenderer LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qsvgrenderer
    SOURCES
        tst_qsvgrenderer.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::Svg
)

# Resources:
set(resources_resource_files
    "heart.svgz"
    "large.svg"
    "large.svgz"
)

qt_internal_add_resource(tst_qsvgrenderer "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)
