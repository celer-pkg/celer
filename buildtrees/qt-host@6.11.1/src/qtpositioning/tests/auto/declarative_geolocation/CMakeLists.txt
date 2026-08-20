# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin

# Collect test data

qt_internal_add_test(tst_declarative_geolocation
    QMLTEST
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Positioning
        Qt::Quick
    TESTDATA
        tst_declarativegeolocation.qml
)

# special case end
