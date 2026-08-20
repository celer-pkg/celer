# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qqmlparser.pro.

#####################################################################
## tst_qqmlparser Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qqmlparser_doc LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qqmlparser_doc # special case
    SOURCES
        tst_qqmlparser.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES
        Qt6::QmlPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qqmlparser_doc CONDITION CMAKE_CROSSCOMPILING # special case
    DEFINES
        QTEST_CROSS_COMPILED
)
