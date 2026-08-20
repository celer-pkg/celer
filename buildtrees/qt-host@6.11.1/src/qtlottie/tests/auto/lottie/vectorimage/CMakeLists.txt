# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_vectorimage LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_vectorimage
    SOURCES
        tst_vectorimage.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTestUtilsPrivate
    TESTDATA
        ${test_data}
    DEFINES
        QT_QMLTEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

file(GLOB_RECURSE test_data_glob_lottie_original
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../baseline/data/original/*)
list(APPEND lottie_data ${test_data_glob_lottie_original})

qt_add_resources(tst_vectorimage lottieoriginal
    PREFIX /json
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../baseline/data/original/
    FILES
        ${lottie_data}
)

file(GLOB_RECURSE test_data_glob_lottie_docsamples
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../baseline/data/docsamples/*)
list(APPEND lottie_data ${test_data_glob_lottie_docsamples})

qt_add_resources(tst_vectorimage lottiedocsamples
    PREFIX /json
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../baseline/data/docsamples/
    FILES
        ${lottie_data}
)

qt_internal_extend_target(tst_vectorimage CONDITION ANDROID OR IOS
    OR QT_FORCE_BUILTIN_TESTDATA
    DEFINES
    QT_QMLTEST_DATADIR=":/data"
)
