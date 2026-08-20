# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_*qml)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_sensors_qmlquick
    QMLTEST
    SOURCES
        ../../common/test_backends.cpp ../../common/test_backends.h
        tst_sensors_qmlquick.cpp
    LIBRARIES
        Qt::Quick
        Qt::Sensors
        Qt::SensorsQuickPrivate
    TESTDATA ${test_data}
)
