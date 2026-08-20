# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WASM)
    return()
endif()

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_scxmlqmlcpp
    SOURCES
        ../../shared/util.cpp ../../shared/util.h
        tst_scxmlqmlcpp.cpp
    LIBRARIES
        Qt::Scxml
        Qt::ScxmlQmlPrivate
        Qt::CorePrivate
        Qt::TestPrivate
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_scxmlqmlcpp CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=":/data"
)

qt_internal_extend_target(tst_scxmlqmlcpp CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

qt6_add_statecharts(tst_scxmlqmlcpp
    "data/topmachine.scxml"
)
