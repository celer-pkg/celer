# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlheightmap LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlheightmap
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(tst_qmlheightmap PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set_source_files_properties("gradientGRAY8.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapGRAY8"
    )
set_source_files_properties("gradientGRAY16.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapGRAY16"
    )
set_source_files_properties("gradientRGB8.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapRGB8"
    )
set_source_files_properties("gradientRGB16.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapRGB16"
    )

set(qmlheightmap_resource_files
    "qml/qmlheightmap/main.qml"
    "gradientGRAY8.png"
    "gradientGRAY16.png"
    "gradientRGB8.png"
    "gradientRGB16.png"
    )
qt_internal_add_resource(tst_qmlheightmap "qmlheightmap"
    PREFIX
    "/"
    FILES
    ${qmlheightmap_resource_files}
    )
