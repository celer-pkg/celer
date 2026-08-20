# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qquickgeocoordinateanimation
    SOURCES
        tst_qquickgeocoordinateanimation.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::PositioningQuickPrivate
        Qt::TestPrivate
)
