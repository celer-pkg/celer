# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgeocoordinateobject
    SOURCES
        tst_qgeocoordinateobject.cpp
    LIBRARIES
        Qt::Core
        Qt::PositioningPrivate
        Qt::TestPrivate
)
