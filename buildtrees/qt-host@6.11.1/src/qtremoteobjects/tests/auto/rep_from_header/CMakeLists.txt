# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_rep_from_header Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_rep_from_header LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_rep_from_header
    SOURCES
        tst_rep_from_header.cpp
    LIBRARIES
        Qt::RemoteObjects
)
qt6_reps_from_headers(tst_rep_from_header pods.h)
