# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.21)

project(qmlsingletonthemetest VERSION 0.1 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS QuickTest Qml Graphs Gui)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(tst_qmltestsingletontheme
    main.cpp
)
add_test(NAME tst_qmltestsingletontheme COMMAND $<TARGET_FILE:tst_qmltestsingletontheme>)

# no need to copy around qml test files for shadow builds - just set the respective define
add_definitions(-DQUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE QML_FILES *.qml)
set_source_files_properties(SingletonTheme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
add_custom_target(singletontheme_qmlfiles SOURCES ${QML_FILES})

qt_add_qml_module(tst_qmltestsingletontheme
    URI SingletonThemeTest
    QML_FILES
        SingletonTheme.qml
)

set_target_properties(tst_qmltestsingletontheme PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)

target_link_libraries(tst_qmltestsingletontheme
    PRIVATE Qt6::Qml
    PRIVATE Qt6::Gui
    PRIVATE Qt6::Graphs
    PRIVATE Qt6::QuickTest
)
