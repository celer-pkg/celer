# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlmultiaxis3d LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlmultiaxis3d
    GUI
    SOURCES
    main.cpp
)

target_link_libraries(tst_qmlmultiaxis3d PUBLIC
    Qt::Quick
    Qt::Graphs
)

set(qmlmultiaxis3d_resource_files
    "qml/main.qml"
    "qml/SingleAxis.qml"
    "qml/AxisSettings.qml"
    "qml/CategorySingleAxis.qml"
    "qml/BarAxisSettings.qml"
    "heightmap.jpg"
)

qt_internal_add_resource(tst_qmlmultiaxis3d "qmlmultiaxis3d"
    PREFIX
    "/"
    FILES
    ${qmlmultiaxis3d_resource_files}
)

qt_add_qml_module(tst_qmlmultiaxis3d
    URI qmlmultiaxis3d
    VERSION 1.0
    QML_FILES qml/main.qml
)
