# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# To see both QtCharts and QtGraphs benchmarks in one graph, first build and run this with
# set(USE_CHARTS true) and the again with set(USE_CHARTS false)
set(USE_CHARTS false)

if (USE_CHARTS)
    project(tst_qmlbenchmark2d LANGUAGES C CXX)
    find_package(Qt6 REQUIRED COMPONENTS Charts Core Gui Qml Quick)

    add_compile_definitions(USE_CHARTS)

    qt_standard_project_setup()

    qt_add_executable(tst_qmlbenchmark2d
        main.cpp
        chartsdatasource.h chartsdatasource.cpp
        resultsio.h resultsio.cpp
    )

    target_link_libraries(tst_qmlbenchmark2d PUBLIC
        Qt::Charts
        Qt::Widgets
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
    )
else()
    if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
        project(tst_qmlbenchmark2d LANGUAGES C CXX)
        find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
    endif()

    qt_internal_add_manual_test(tst_qmlbenchmark2d
        GUI
        SOURCES
            main.cpp
            datasource.h datasource.cpp
            resultsio.h resultsio.cpp
    )

    target_link_libraries(tst_qmlbenchmark2d PUBLIC
        Qt::Graphs
        Qt::Widgets
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
    )
endif()

# Resources:
set(qmlbenchmark2d_resource_files
    "Main.qml"
    "Results.qml"
    "ChartsLineTest.qml"
    "ChartsAreaTest.qml"
    "ChartsBarTest.qml"
    "ChartsOpenGLLineTest.qml"
    "ChartsSplineTest.qml"
    "GraphsLineTest.qml"
    "GraphsAreaTest.qml"
    "GraphsBarTest.qml"
    "GraphsSplineTest.qml"
)

qt_internal_add_resource(tst_qmlbenchmark2d "qmlbenchmark2d"
    PREFIX
        "/"
    FILES
        ${qmlbenchmark2d_resource_files}
)
