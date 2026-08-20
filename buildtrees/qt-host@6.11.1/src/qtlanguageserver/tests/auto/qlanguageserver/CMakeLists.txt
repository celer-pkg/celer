# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qlanguageserver Test:
#####################################################################

qt_internal_add_test(tst_qlanguageserver
    SOURCES
        tst_qlanguageserver.cpp
        qiopipe.h qiopipe.cpp
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::CorePrivate
        Qt::LanguageServerPrivate
        Qt::Test
    TESTDATA ${test_data}
)
