# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlbarscatter LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlbarscatter
    GUI
    SOURCES
    main.cpp
    processorinfo.h processorinfo.cpp
    cpuusageupdater.h cpuusageupdater.cpp
    barseries.h barseries.cpp
    scatterseries.h scatterseries.cpp
)


target_link_libraries(tst_qmlbarscatter PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
    Pdh
)

set(qmlbarscatter_resource_files
    "qml/main.qml"
    "images/img.png"
)

qt_internal_add_resource(tst_qmlbarscatter "qmlbarscatter"
    PREFIX
    "/"
    FILES
    ${qmlbarscatter_resource_files}
)

qt_add_qml_module(tst_qmlbarscatter
    URI qmlbarscatter
    VERSION 1.0
    QML_FILES qml/main.qml
)
