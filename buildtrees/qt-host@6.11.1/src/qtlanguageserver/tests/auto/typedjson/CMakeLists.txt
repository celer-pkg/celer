# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_typedjson Test:
#####################################################################
# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_typedjson
    SOURCES
        tst_typedjson.cpp tst_typedjson.h
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::JsonRpcPrivate
        Qt::Test
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_typedjson CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_TYPEDJSON_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

qt_internal_extend_target(tst_typedjson CONDITION ANDROID OR IOS
    DEFINES
        QT_TYPEDJSON_DATADIR=":/data"
)
