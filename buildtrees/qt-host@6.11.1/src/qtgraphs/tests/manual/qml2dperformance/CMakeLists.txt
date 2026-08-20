# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    projecf(tst_qml2dperformance LANGUAGES C CXX ASM)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qml2dperformance
    GUI
    SOURCES
        main.cpp
        randomModel.h randomModel.cpp
        randomGenerator.h randomGenerator.cpp
    )

target_link_libraries(tst_qml2dperformance PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set(qml2dperformance_resource_files
    "qml/qml2dperformance/main.qml"
    )

qt_internal_add_resource(tst_qml2dperformance "qml2dperformance"
    PREFIX
    "/"
    FILES
    ${qml2dperformance_resource_files}
    )
