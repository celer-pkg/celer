# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlmultitestwithsingletontheme LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_qmlmultitestwithsingletontheme
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlmultitestwithsingletontheme PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set_source_files_properties(qml/qmlmultitestwithsingletontheme/SingletonTheme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(tst_qmlmultitestwithsingletontheme
    URI SingletonTheme
    QML_FILES
        Main.qml
        qml/qmlmultitestwithsingletontheme/SingletonTheme.qml
)
