# Copyright (C) 2025 Intel Corporatin
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qatomicwait LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qatomicwait
    SOURCES
        tst_qatomicwait.cpp
        ../../../../../src/corelib/thread/qatomicwait.cpp
    DEFINES
        QATOMICWAIT_USE_FALLBACK
    LIBRARIES
        Qt::CorePrivate
)
