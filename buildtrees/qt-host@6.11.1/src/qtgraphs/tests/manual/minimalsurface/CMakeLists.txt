# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_minimalsurface LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_minimalsurface
    GUI
    SOURCES
        ../../../src/graphs3d/widget/doc/snippets/doc_src_q3dsurface_construction.cpp
    )
target_link_libraries(tst_minimalsurface PUBLIC
    Qt::Gui
    Qt::Graphs
    Qt::GraphsWidgets
    )
