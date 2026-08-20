# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qmldomerrormessage LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qmldomfilelocations
    SOURCES
        tst_qmldomfilelocations.cpp tst_qmldomfilelocations.h
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::QmlDomPrivate
)
