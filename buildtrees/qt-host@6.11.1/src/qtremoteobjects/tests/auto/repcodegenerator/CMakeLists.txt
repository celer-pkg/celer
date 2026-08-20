# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_repcodegenerator Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_repcodegenerator LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_repcodegenerator
    SOURCES
        tst_repcodegenerator.cpp
    LIBRARIES
        Qt::RemoteObjects
)
qt6_add_repc_merged(tst_repcodegenerator
    classwithsignalonlytest.rep
    preprocessortest.rep
    classwithreadonlypropertytest.rep
    classwithattributetest.rep
    classinnamespace.rep
)

## Scopes:
#####################################################################
