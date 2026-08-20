# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    projecf(tst_qmlinjection LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlinjection
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlinjection PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set(qmlinjection_resource_files
    "qml/qmlinjection/main.qml"
    "qml/qmlinjection/GraphInjection.qml"
    "qml/qmlinjection/GraphNodeInjection.qml"
    "qml/qmlinjection/MultiView.qml"
    "qml/qmlinjection/MultiGraph.qml"
    "qml/qmlinjection/australia.png"
    "qml/qmlinjection/australiaSatellite.jpg"
    "qml/qmlinjection/plane.mesh"
    "qml/qmlinjection/layer_1.png"
    )

qt_internal_add_resource(tst_qmlinjection "qmlinjection"
    PREFIX
    "/"
    FILES
    ${qmlinjection_resource_files}
    )
