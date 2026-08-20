# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from assetimport.pro.

#####################################################################
## assetimport Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qquick3dassetimport LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    resources/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qquick3dassetimport
    SOURCES
        tst_assetimport.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick3DAssetImportPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:assetimport.pro:<TRUE>:
# TEMPLATE = "app"
