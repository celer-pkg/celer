# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qmlwebchannel
    QMLTEST
    QML_IMPORTPATH
        "${CMAKE_CURRENT_BINARY_DIR}/../../../qml"
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
    SOURCES
        qml.cpp
        testobject.cpp testobject.h
        testtransport.cpp testtransport.h
        testwebchannel.cpp testwebchannel.h
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
        Qt::WebChannelQuick
    TESTDATA ${test_data}
)

