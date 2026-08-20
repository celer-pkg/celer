# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_itemmodeltest LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_itemmodeltest
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(tst_itemmodeltest PUBLIC
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
    )
