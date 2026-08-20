# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlqtquick3dintegration LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlqtquick3dintegration
    GUI
    SOURCES
        videodata.cpp videodata.h
        main.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Multimedia MultimediaWidgets)

target_link_libraries(tst_qmlqtquick3dintegration PUBLIC
    Qt::Gui
    Qt::Graphs
    Qt::Multimedia
    Qt::MultimediaWidgets
)

set(qmlqtquick3dintegration_resource_files
    "qml/qmlqtquick3dintegration/main.qml"
    "qml/qmlqtquick3dintegration/Axes.qml"
    "qml/qmlqtquick3dintegration/Data.qml"
    "video/video.mp4"
)

qt_internal_add_resource(tst_qmlqtquick3dintegration "qmlqtquick3dintegration"
    PREFIX
    "/"
    FILES
    ${qmlqtquick3dintegration_resource_files}
)

# Install video.mp4 so that it can be loaded from file system
install(FILES video/video.mp4
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/video
)
