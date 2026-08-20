# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlrenderslicetoimage LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_qmlrenderslicetoimage
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(tst_qmlrenderslicetoimage PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set(qmlrenderslicetoimage_resource_files
    "qml/qmlrenderslicetoimage/main.qml"
    "qml/qmlrenderslicetoimage/SurfaceGraph.qml"
    "qml/qmlrenderslicetoimage/BarGraph.qml"
    )

qt_internal_add_resource(tst_qmlrenderslicetoimage "qmlgrabsliceview"
    PREFIX
    "/"
    FILES
    ${qmlrenderslicetoimage_resource_files}
    )

set(qmlrenderslicetoimage_image_resource_files
    "layer_1.png"
    "layer_2.png"
    "layer_3.png"
)

qt6_add_resources(tst_qmlrenderslicetoimage "qmlsurfacelayers1"
    PREFIX
        "/heightmaps"
    FILES
        ${qmlrenderslicetoimage_image_resource_files}
)
