# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qshaderbaker.pro.

#####################################################################
## tst_qshaderbaker Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qshaderbaker LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qshaderbaker
    SOURCES
        tst_qshaderbaker.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::ShaderToolsPrivate
)

# Resources:
set(qshaderbaker_resource_files
    "data"
)

qt_internal_add_resource(tst_qshaderbaker "qshaderbaker"
    PREFIX
        "/"
    FILES
        ${qshaderbaker_resource_files}
)

