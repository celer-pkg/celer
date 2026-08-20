# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qwebmercator Test:
#####################################################################

qt_internal_add_test(tst_qwebmercator
    SOURCES
        tst_qwebmercator.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::PositioningPrivate
)
