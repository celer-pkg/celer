# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qquick3dgeometry.pro.

#####################################################################
## qquick3dgeometry Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qquick3dgeometry LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qquick3dgeometry
    SOURCES
        tst_qquick3dgeometry.cpp
    LIBRARIES
        Qt::Quick3D
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DUtilsPrivate
)

#### Keys ignored in scope 1:.:.:qquick3dgeometry.pro:<TRUE>:
# TEMPLATE = "app"
