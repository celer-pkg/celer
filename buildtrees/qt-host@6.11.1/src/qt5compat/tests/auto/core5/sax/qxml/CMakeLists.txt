# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qxml.pro.

#####################################################################
## tst_qxml Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qxml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Collect test data
list(APPEND test_data "0x010D.xml")

qt_internal_add_test(tst_qxml
    SOURCES
        tst_qxml.cpp
    LIBRARIES
        Qt::Core5Compat
    TESTDATA ${test_data}
)
