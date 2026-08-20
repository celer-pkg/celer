# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE test_data_glob
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     data/*)
list(APPEND test_data ${test_data_glob})

get_filename_component(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)

qt_internal_add_test(tst_qcandbcfileparser
    SOURCES
        tst_qcandbcfileparser.cpp
    INCLUDE_DIRECTORIES
        ${SHARED_DIR}
    LIBRARIES
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
    TESTDATA
        ${test_data}
)

qt_internal_extend_target(tst_qcandbcfileparser CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

qt_internal_extend_target(tst_qcandbcfileparser CONDITION ANDROID OR IOS
    DEFINES
        QT_TEST_DATADIR=":/data"
)
