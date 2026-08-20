# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## conversion Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(conversion LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(conversion
    SOURCES
        tst_conversion.cpp
    INCLUDE_DIRECTORIES
        ../../../src/activeqt/shared/
    LIBRARIES
        Qt::AxContainer
        Qt::Gui
        Qt::AxBasePrivate
        Qt::CorePrivate
)
