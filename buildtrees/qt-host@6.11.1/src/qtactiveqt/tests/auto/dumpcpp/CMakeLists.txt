# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_dumpcpp Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_dumpcpp LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_dumpcpp
    SOURCES
        tst_dumpcpp.cpp
    LIBRARIES
        Qt::AxContainer
        Qt::Gui
        Qt::Widgets
)

qt6_target_typelibs(tst_dumpcpp LIBRARIES "ieframe.dll")
qt6_target_typelibs(tst_dumpcpp LIBRARIES "msxml6:{F5078F18-C551-11D3-89B9-0000F81FE221}")
