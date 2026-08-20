# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_modelview Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_modelview LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_modelview
    SOURCES
        tst_modelview.cpp
        ../shared/model_utilities.h
    LIBRARIES
        Qt::Gui
        Qt::RemoteObjects
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_modelview CONDITION boot2qt
    DEFINES
        SLOW_MODELTEST
)

qt_internal_extend_target(tst_modelview CONDITION MINGW
    DEFINES
        SLOW_MODELTEST
)
